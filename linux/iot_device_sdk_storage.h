/* ------------------------------------------------------------------
 * Description: header file for using device sdk storage
 *
 *
 * Feburary 2016, 
 *
 * Copyright (c) 2016 by cisco Systems, Inc.
 * All rights reserved.
 * ------------------------------------------------------------------
 */

#ifndef _CISCO_IOT_DEVICE_SDK_STORAGE_H
#define _CISCO_IOT_DEVICE_SDK_STORAGE_H

#include <jansson.h>

#define STORAGE_PREFIX "device_db/"


typedef enum {
    STORAGE_OK,
    STORAGE_NOT_FOUND,
    STORAGE_BUF_TOO_SMALL,
    STORAGE_ERR,
} storage_rc;

/*
 * load json text from file and save into structure
 * Returns: json_t ptr if success, NULL if failure
 * If pointer is returned, it must be freed with json_decref()
 */
json_t *iot_device_load_json(char *dev_id);

/*
 * Save json structure into a file.
 * Note that this API creates the storage directory (STORAGE_PREFIX) if
 * the directory does not already exists
 */
storage_rc iot_device_save_json(char *dev_id, json_t *root);

/*
 * For legacy data structures, save and load device data using ptr and len
 */
storage_rc iot_device_load_data(char *dev_id, void *dd, size_t len);

storage_rc iot_device_save_data(char *dev_id, void *dd, size_t len);

SDKDeviceInternal* iot_load_device_json_into_devicestruct(char *deviceUid);


#endif /* _CISCO_IOT_DEVICE_SDK_STORAGE_H */
