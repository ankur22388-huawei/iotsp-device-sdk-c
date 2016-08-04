/* ------------------------------------------------------------------
 * Description: interface for storing and retrieving device data
 *
 *
 * Feburary 2016, 
 *
 * Copyright (c) 2016 by cisco Systems, Inc.
 * All rights reserved.
 * ------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>          /* errno constants */
#include <errno.h>          /* errno */
#include <sys/stat.h>       /* fstat() */
#include <unistd.h>         /* read() */
#include <string.h>
#include <jansson.h>
#include <iot_device_sdk.h>
#include "_iot_device_sdk_init.h"
#include "iot_device_sdk_storage.h"
#include "iot_device_sdk_json.h"

#define DEVICE_FILEPATH_LEN 200
#define USRPSWD_LEN 64

extern int update_Device_Connector_Details(SDKDeviceInternal* clientHdle); //rest

/*  helpers  */
SDKDeviceInternal* iot_load_device_json_into_devicestruct(char *deviceUid)
{
    char filepath[DEVICE_FILEPATH_LEN];
    json_error_t json_err;
    json_t *root;
    SDKDeviceInternal *deviceEntry = NULL;

    if(deviceUid) {
      /* TODO: Need SafeC */
      sprintf(filepath, "%s%s", STORAGE_PREFIX, deviceUid);
      iot_device_sdk_log(SDK_LOG_INFO, "iot_load_device_json_into_devicestruct  %s  \n", filepath);
      root = json_load_file(filepath, 0, &json_err);
      if (!root) {
        fprintf(stderr, "IOT_SDK load device json file error loading '%s', line %d: %s \n", deviceUid,
                json_err.line, json_err.text);
        iot_device_sdk_log(SDK_LOG_ERROR,"IOT_SDK iot_load_device_json_into_devicestruct deviceUid [%s] error loding file [%s]\n",
                deviceUid, filepath);
        return NULL;
      } else {

        deviceEntry = (SDKDeviceInternal*)calloc(1, sizeof(SDKDeviceInternal));
        deviceEntry->json_root = root;
        json_parse_device_json_data((IotDeviceSdkClient*)deviceEntry/*, root*/);
        //update device connector Details
        update_Device_Connector_Details(deviceEntry);
        json_decref(deviceEntry->json_root);
        deviceEntry->json_root = NULL;
        //parse the json and fill the
        return deviceEntry;
      }
    } else {
      iot_device_sdk_log(SDK_LOG_ERROR,"IOT_SDK iot_load_device_json_into_devicestruct deviceUid passed is NULL\n" );
      return NULL;
    }
    return NULL;
}



/*
 * load json text from file and save into structure
 * Returns: json_t ptr if success, NULL if failure
 * If pointer is returned, it must be freed with json_decref()
 */
json_t *iot_device_load_json(char *dev_id)
{
    char filepath[DEVICE_FILEPATH_LEN];
    json_error_t json_err;
    json_t *root;
    /* TODO: Need SafeC */
    sprintf(filepath, "%s%s", STORAGE_PREFIX, dev_id);
    root = json_load_file(filepath, 0, &json_err);
    if (!root) {
        fprintf(stderr, "IOT_SDK load device json file error loading '%s', line %d: %s \n", dev_id,
                json_err.line, json_err.text);
        return NULL;
    }
    return root;
}

/*
 * Save json structure into a file.
 * Note that this API does not create the storage directory (STORAGE_PREFIX),
 * the directory needs to be created in advance.
 */
storage_rc iot_device_save_json(char *dev_id, json_t *root)
{
    char filepath[DEVICE_FILEPATH_LEN];
    int first_attempt = 1;
    /* TODO: Need SafeC */
    sprintf(filepath, "%s%s", STORAGE_PREFIX, dev_id);
try_again:
    if (json_dump_file(root, filepath, 0)) {
        struct stat fileinfo;
        if (first_attempt && stat(STORAGE_PREFIX, &fileinfo)) {
    	    /* likely that the directory is not created */
          mkdir(STORAGE_PREFIX, S_IRUSR | S_IWUSR | S_IXUSR);
    	    first_attempt = 0;
    	    goto try_again;
        }
        return STORAGE_ERR;
    }
    return STORAGE_OK;
}


/*
 * For legacy data structures, save and load device data using ptr and len
 */
storage_rc iot_device_load_data(char *dev_id, void *dd, size_t len)
{
    int fd;
    char filepath[DEVICE_FILEPATH_LEN];
    struct stat fileinfo;
    storage_rc rc;
    /* TODO: Need SafeC */
    sprintf(filepath, "%s%s", STORAGE_PREFIX, dev_id);
    fd = open(filepath, O_RDONLY);
    if (fd < 0) {
        if (errno == ENOENT) {
            return STORAGE_NOT_FOUND;
        } else {
            // print errno?
            return STORAGE_ERR;
        }
    }
    if (fstat(fd, &fileinfo)) {
        // print errno?
        rc = STORAGE_ERR;
        goto end;
    }
    // TODO: change from the current C struct
    if ((size_t)fileinfo.st_size > len) {
        rc = STORAGE_BUF_TOO_SMALL;
        goto end;
    }
    if (read(fd, dd, fileinfo.st_size) <= 0) {
        rc = STORAGE_ERR;
        goto end;
    }
    rc = STORAGE_OK;
end:
    close(fd);
    return rc;
}

storage_rc iot_device_save_data(char *dev_id, void *dd, size_t len)
{
    int fd, first_attempt;
    char filepath[DEVICE_FILEPATH_LEN];
    storage_rc rc;
    /* TODO: Need SafeC */
    sprintf(filepath, "%s%s", STORAGE_PREFIX, dev_id);
    first_attempt = 1;
try_again:
    fd = open(filepath, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
    if (fd < 0) {
    	if (ENOENT == errno && first_attempt) {
    	    /* likely that the directory is not created */
          mkdir(STORAGE_PREFIX, S_IRUSR | S_IWUSR | S_IXUSR);
    	    first_attempt = 0;
    	    goto try_again;
        }
        return STORAGE_ERR;
    }
    if ((size_t)write(fd, dd, len) != len) {
        /* maybe went out of space, remove the corrupted file */
        close(fd);
        /* best effort, not checking for return code */
        remove(filepath);
        return STORAGE_ERR;
    }
    rc = STORAGE_OK;
    close(fd);
    return rc;
}

/* Unit Testing

typedef struct {
    int version;
    char username[USRPSWD_LEN];
    char password[USRPSWD_LEN];
    char opaque[16];
} device_data;

int main()
{
    int rc;
    json_t *root;
    char *s;
    root = json_pack("{s:s, s:s, s:s, s:s}",
                     "version", "1.0.1",
                     "username", "foo",
                     "password", "bar",
                     "opaque", "nothing"
                 );
    rc = iot_device_save_json("dfafsaf.json", root);
    printf("save_json rc: %d\n", rc);
    json_decref(root);
    root = iot_device_load_json("dfafsaf.json");
    s = json_dumps(root, 0);
    printf("readback: %s\n", s);
    free(s);

    device_data dd;
    strcpy(dd.username, "foo");
    strcpy(dd.password, "bar");
    strcpy(dd.opaque, "hello, world");
    rc = iot_device_save_data("sdfsadfsafdsf", &dd, sizeof(dd));
    printf("save_data rc: %d\n", rc);
    memset(&dd, 0, sizeof(dd));
    // wrong dev_id
    rc = iot_device_load_data("aaaaadfsafdsf", &dd, sizeof(dd));
    printf("load_data rc: %d\n", rc);
    // success: major, minor version identical
    rc = iot_device_load_data("sdfsadfsafdsf", &dd, sizeof(dd));
    printf("load_data rc: %d\n", rc);
    printf("%s/%s/%s\n", dd.username, dd.password, dd.opaque);

    return 0;
}
*/
