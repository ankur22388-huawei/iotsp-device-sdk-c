/**************************************************************************
  copyright (C) Copyright 2015-2016 Cisco Systems Inc.; All Rights Reserved
  History:
    01/16/2015 - Created.
 ******************************************************************************/
#define _GNU_SOURCE
#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <assert.h>
#include <iot_device_sdk.h>
#include "_iot_device_sdk_init.h"
#include "iot_device_sdk_defaults.h"

typedef struct _curlResponseStruct {
  char *data;
  size_t size;
  CURL *curlHandle;
  serverResponseHdrsStruct svrRespHdrs;
}curlResponseStruct;

/**
* sdk_init_curl ( ) - Function to initialize libcurl
* Input  :
* return :
*/
void sdk_init_curl()
{
  iot_device_sdk_log(SDK_LOG_INFO, "sdk_init_curl() ++ \n");
  curl_global_init(CURL_GLOBAL_ALL);
  iot_device_sdk_log(SDK_LOG_INFO, "sdk_init_curl() -- \n");
}


/**
* sdk_deinit_curl ( ) - Function to de-initialize libcurl
* Input  :
* return :
*/
void sdk_deinit_curl()
{
  iot_device_sdk_log(SDK_LOG_INFO, "sdk_deinit_curl() ++ \n");
  curl_global_cleanup();
  iot_device_sdk_log(SDK_LOG_INFO, "sdk_deinit_curl() -- \n");
}

#if 0
/* Needed when handling IAM re-direct when device token expires */
/**
* header_callback( ) - Callback function to receive header strings from libcurl
* Input  :
* return : (nmemb * size) for successful processing of callback bytes
*/
static size_t header_callback(char* contentsPtr, size_t size, size_t nmemb, void* userDataP)
{
  curlResponseStruct *res = (curlResponseStruct *)userDataP;
  // If response code had not been previously set, read it
  if (0 == res->svrRespHdrs.curl_res_code) {
    curl_easy_getinfo(res->curlHandle, CURLINFO_RESPONSE_CODE, &(res->svrRespHdrs.curl_res_code));
  }
  // For 301,302 redirect cases: look for the Location: field
  if (301 == res->svrRespHdrs.curl_res_code || 302 == res->svrRespHdrs.curl_res_code) {
    if (0 == strncasecmp(contentsPtr, "Location", 8)) {
      char *ret = strchr(contentsPtr, ':');
      if (ret) {
        ret++;
        if (*ret==' ') ret++; // skip over leading space
        // header lines have at end, 0x0d[carraige return] 0x0a[new line]
        int length = strlen(ret)-1;
        res->svrRespHdrs.redirect_url=(char*)malloc(length);
        strncpy(res->svrRespHdrs.redirect_url, ret, length);
        res->svrRespHdrs.redirect_url[length-1] = '\0';
      }
    }
  }
  return nmemb * size;
}
#endif

/**
* curlWriteCallback( ) - Callback function to receive message data from libcurl
* Input  :
* return : (nmemb * size) for successful processing of callback bytes
*/
static size_t curlWriteCallback(char* contentsPtr, size_t size, size_t nmemb, void* userDataP){
  size_t realsize = size * nmemb;
  long curl_res_code =0;

  curlResponseStruct *res = (curlResponseStruct *)userDataP;
  res->data = (char*)realloc(res->data, res->size + realsize + 1);
  if(res->data == NULL) {
    /* out of memory! */
    iot_device_sdk_log(SDK_LOG_ERROR, "curlWriteCallback() realloc Failed \n");
    return -1;
  }

  memcpy(&(res->data[res->size]), contentsPtr, realsize);
  res->size += realsize;
  res->data[res->size] = 0;

  //Also get the curl reponse code
  if(res->curlHandle){
    curl_easy_getinfo(res->curlHandle, CURLINFO_RESPONSE_CODE, &curl_res_code);
    iot_device_sdk_log(SDK_LOG_INFO, "curlWriteCallback() curl response Code [%d] \n",curl_res_code);
  }
  // What to do with the response - Do we send it back to the Application
  //if(res->data)
  //  free(res->data);
  return realsize;
}



static int last_curl_error = 0;

// 0 -success 1- failure
static int set_server_cert_verfication_curl_options(IotDeviceSdkClient clientHandle, CURL *curl_handle){

  if(((SDKDeviceInternal*)clientHandle)->certAuthorityBundleFile){
    iot_device_sdk_log(SDK_LOG_WARN, "set_server_cert_verfication_curl_options() server certificate will be verified \n");
    if(curl_easy_setopt(curl_handle, CURLOPT_CAINFO, ((SDKDeviceInternal*)clientHandle)->certAuthorityBundleFile) != CURLE_OK) {
      iot_device_sdk_log(SDK_LOG_ERROR, "set_server_cert_verfication_curl_options() curl CURLOPT_CAINFO failed \n");
      return  -1;
    }
    if(curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 1L) != CURLE_OK) {
      iot_device_sdk_log(SDK_LOG_ERROR, "set_server_cert_verfication_curl_options() curl CURLOPT_SSL_VERIFYPEER failed \n");
      return -1;
    }

    if(curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 2L) != CURLE_OK) {
      iot_device_sdk_log(SDK_LOG_ERROR, "set_server_cert_verfication_curl_options() curl CURLOPT_SSL_VERIFYHOST failed \n");
      return -1;
    }
  } else {
    iot_device_sdk_log(SDK_LOG_WARN, "set_server_cert_verfication_curl_options() server certificate will not be verified \n");
    if(curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0L) != CURLE_OK) {
      iot_device_sdk_log(SDK_LOG_ERROR, "set_server_cert_verfication_curl_options() curl CURLOPT_SSL_VERIFYPEER failed \n");
      return -1;
    }
    if(curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 0L) != CURLE_OK) {
      iot_device_sdk_log(SDK_LOG_ERROR, "set_server_cert_verfication_curl_options() curl CURLOPT_SSL_VERIFYHOST failed \n");
      return -1;
    }
  }
  return 0;
}
/**
* send_RegistrationMessage_curl ( ) - Function to send registration message using curl
* Input  :
* return :  0 - success
*          -1 - Error
*/

int send_RegistrationMessage_curl(IotDeviceSdkClient clientHandle,
  const char *url, const char *data, SDKRegistrationResponse *responseData) {
  CURL *curl_handle = NULL;
  CURLcode ret_val;
  curlResponseStruct writedata;
  struct curl_slist *headers = NULL;
  int ret = 0; // default no error return value

  iot_device_sdk_log(SDK_LOG_INFO, "send_RegistrationMessage_curl() ++ url(%s)data(%s)\n", url, data);

  if(clientHandle && url && data) {
    curl_handle = curl_easy_init();
    if(!curl_handle) {
      iot_device_sdk_log(SDK_LOG_ERROR, "send_RegistrationMessage_curl() curl init failed \n");
      //call callback if callbackhandle set
      //if(clienthandle)
      //  clientHandle->clientCallBackFunction();
      ret = -1;
      goto cleanup;
    } else {
      /* set up header */
      headers = curl_slist_append(headers, "Content-Type: application/json");
      if(curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, headers) != CURLE_OK) {
        iot_device_sdk_log(SDK_LOG_ERROR, "send_RegistrationMessage_curl() curl CURLOPT_HTTPHEADER failed \n");
        ret = -1;
        goto cleanup;
      }

      /* First set the URL that is about to receive our POST. */
      if(curl_easy_setopt(curl_handle, CURLOPT_URL, url) != CURLE_OK) {
        iot_device_sdk_log(SDK_LOG_ERROR, "send_RegistrationMessage_curl() curl CURLOPT_URL failed \n");
        ret = -1;
        goto cleanup;
      }

      if(set_server_cert_verfication_curl_options(clientHandle, curl_handle) < 0){
        ret = -1;
        goto cleanup;
      }

      // initialize the writedata struct
      memset(&writedata, 0, sizeof(curlResponseStruct ));
      writedata.curlHandle = curl_handle;
      /*  specify we want to POST data */
      if(curl_easy_setopt(curl_handle, CURLOPT_POST, 1L)!= CURLE_OK) {
        iot_device_sdk_log(SDK_LOG_ERROR, "send_RegistrationMessage_curl() curl CURLOPT_POST failed \n");
        ret = -1;
        goto cleanup;
      }
      /* size of the POST data */
      if(curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDSIZE, strlen(data))!= CURLE_OK) {
        iot_device_sdk_log(SDK_LOG_ERROR, "send_RegistrationMessage_curl() curl CURLOPT_POSTFIELDSIZE failed \n");
        ret = -1;
        goto cleanup;
      }
      /* pointer to the POST data */
      if(curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, data)!= CURLE_OK) {
        iot_device_sdk_log(SDK_LOG_ERROR, "send_RegistrationMessage_curl() curl CURLOPT_POSTFIELDS failed \n");
        ret = -1;
        goto cleanup;
      }
      /* write callback function */
      if(curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, curlWriteCallback)!= CURLE_OK) {
        iot_device_sdk_log(SDK_LOG_ERROR, "send_RegistrationMessage_curl() curl CURLOPT_WRITEFUNCTION failed \n");
        ret = -1;
        goto cleanup;
      }
      /* pass data pointer to the write callback */
      if(curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void*)(&writedata))!= CURLE_OK) {
        iot_device_sdk_log(SDK_LOG_ERROR, "send_RegistrationMessage_curl() curl CURLOPT_WRITEDATA failed \n");
        ret = -1;
        goto cleanup;
      }
      /* complete within n seconds : to come from config?*/
      if(curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, IOT_DEFAULT_CURL_TIMEOUT)!= CURLE_OK) {
        iot_device_sdk_log(SDK_LOG_ERROR, "send_RegistrationMessage_curl() curl CURLOPT_TIMEOUT failed \n");
        ret = -1;
        goto cleanup;
      }

      ret_val = curl_easy_perform(curl_handle);

      /* check for errors */
      if(ret_val != CURLE_OK) {
        iot_device_sdk_log(SDK_LOG_ERROR, "send_RegistrationMessage_curl()->curl_easy_perform() failed %s \n",curl_easy_strerror(ret_val));
        last_curl_error = ret_val;
        ret = -1;
        goto cleanup;
      } else {
        long httpCode = 0;
        curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &httpCode);
        responseData->responseCode = httpCode;
        SAFE_FREE(responseData->response)
        // use the registration message response in writedata.data instead
        // of freeing
        responseData->response = writedata.data;
        iot_device_sdk_log(SDK_LOG_INFO, "send_RegistrationMessage_curl()->curl_easy_perform()Success \n");
/*        if(writedata.data) {
          iot_device_sdk_log(SDK_LOG_INFO, "send_RegistrationMessage_curl()->curl_easy_perform() data [%s] \n",writedata.data);
          free(writedata.data);
        }
        */
      }
    }
  } else {
    iot_device_sdk_log(SDK_LOG_ERROR, "send_RegistrationMessage_curl() url or data passed is NULL \n");
    return -1;
  }

cleanup:
  if(headers)
    curl_slist_free_all(headers);
  curl_easy_cleanup(curl_handle);
  iot_device_sdk_log(SDK_LOG_INFO, "send_RegistrationMessage_curl() -- \n");
  return ret;
}

/**
* send_AccessTokenRequest_curl ( ) - Function to send access token request using curl
* Input  :
* return :  0 - success
*          -1 - Error
*/

int send_AccessTokenRequest_curl(IotDeviceSdkClient clientHandle,
  const char *url, const char *data, AccessTokenDetails *responseData) {

  CURL *curl_handle = NULL;
  CURLcode ret_val;
  curlResponseStruct writedata;
  int ret = 0; // default no error return value

  iot_device_sdk_log(SDK_LOG_INFO, "send_AccessTokenRequest_curl() ++ \n");

  if(clientHandle && url && data) {
    curl_handle = curl_easy_init();
    if(!curl_handle) {
      iot_device_sdk_log(SDK_LOG_ERROR, "send_AccessTokenRequest_curl() curl init failed \n");
      ret = -1;
      goto cleanup;
    } else {
      /* First set the URL that is about to receive our POST. */
      if(curl_easy_setopt(curl_handle, CURLOPT_URL, url) != CURLE_OK) {
        iot_device_sdk_log(SDK_LOG_ERROR, "send_AccessTokenRequest_curl() curl CURLOPT_URL failed \n");
        ret = -1;
        goto cleanup;
      }

      if(set_server_cert_verfication_curl_options(clientHandle, curl_handle) < 0){
        ret = -1;
        goto cleanup;
      }

      // initialize the writedata struct
      memset(&writedata, 0, sizeof(curlResponseStruct ));
      writedata.curlHandle = curl_handle;

      /*  specify we want to POST data */
      if(curl_easy_setopt(curl_handle, CURLOPT_POST, 1L)!= CURLE_OK) {
        iot_device_sdk_log(SDK_LOG_ERROR, "send_AccessTokenRequest_curl() curl CURLOPT_POST failed \n");
        ret = -1;
        goto cleanup;
      }

      /* size of the POST data */
      if(curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDSIZE, strlen(data))!= CURLE_OK) {
        iot_device_sdk_log(SDK_LOG_ERROR, "send_RegistrationMessage_curl() curl CURLOPT_POSTFIELDSIZE failed \n");
        ret = -1;
        goto cleanup;
      }
      /* pointer to the POST data */
      if(curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, data)!= CURLE_OK) {
        iot_device_sdk_log(SDK_LOG_ERROR, "send_RegistrationMessage_curl() curl CURLOPT_POSTFIELDS failed \n");
        ret = -1;
        goto cleanup;
      }
      /* write callback function */
      if(curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, curlWriteCallback)!= CURLE_OK) {
        iot_device_sdk_log(SDK_LOG_ERROR, "send_AccessTokenRequest_curl() curl CURLOPT_WRITEFUNCTION failed \n");
        ret = -1;
        goto cleanup;
      }
      /* pass data pointer to the write callback */
      if(curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void*)(&writedata))!= CURLE_OK) {
        iot_device_sdk_log(SDK_LOG_ERROR, "send_AccessTokenRequest_curl() curl CURLOPT_WRITEDATA failed \n");
        ret = -1;
        goto cleanup;
      }

      /* complete within n seconds : to come from config?*/
      if(curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, IOT_DEFAULT_CURL_TIMEOUT)!= CURLE_OK) {
        iot_device_sdk_log(SDK_LOG_ERROR, "send_AccessTokenRequest_curl() curl CURLOPT_TIMEOUT failed \n");
        ret = -1;
        goto cleanup;
      }

      ret_val = curl_easy_perform(curl_handle);
      /* check for errors */
      if(ret_val != CURLE_OK) {
        iot_device_sdk_log(SDK_LOG_ERROR, "send_AccessTokenRequest_curl()->curl_easy_perform() failed %s \n",curl_easy_strerror(ret_val));
        ret = -1;
        goto cleanup;
      } else {
        long http_code = 0;
        curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &http_code);
        responseData->responseCode = http_code;
        SAFE_FREE(responseData->response)
        // use writedata.data instead of freeing
        responseData->response = writedata.data;
      }
    }
  } else {
    iot_device_sdk_log(SDK_LOG_ERROR, "send_AccessTokenRequest_curl() url or data passed is NULL \n");
    return -1;
  }

cleanup:
  curl_easy_cleanup(curl_handle);
  iot_device_sdk_log(SDK_LOG_INFO, "send_AccessTokenRequest_curl() -- \n");
  return ret;
}

/**
* send_device_data_curl ( ) - Function to send device data request using curl
* Input  :
* return :  0 - success
*          -1 - Error
*/

int send_device_data_curl(IotDeviceSdkClient clientHandle,
  const char *url, const char *data, const char *tags, DataPostResponse *responseData) {

  CURL *curl_handle = NULL;
  CURLcode ret_val;
  curlResponseStruct writedata;
  int ret = 0; // default no error return value
  struct curl_slist *headers = NULL;
  char *auth_header =NULL, *route_header = NULL;

  iot_device_sdk_log(SDK_LOG_INFO, "send_device_data_curl() ++ \n");

  if(clientHandle && url && data) {
    curl_handle = curl_easy_init();
    if(!curl_handle) {
      iot_device_sdk_log(SDK_LOG_ERROR, "send_device_data_curl() curl init failed \n");
      ret = -1;
      goto cleanup;
    } else {
      iot_device_sdk_log(SDK_LOG_INFO, "send_device_data_curl() observation post url = %s \n", url);
      /* First set the URL that is about to receive our POST. */
      if(curl_easy_setopt(curl_handle, CURLOPT_URL, url) != CURLE_OK) {
        iot_device_sdk_log(SDK_LOG_ERROR, "send_device_data_curl() curl CURLOPT_URL failed \n");
        ret = -1;
        goto cleanup;
      }
      if(set_server_cert_verfication_curl_options(clientHandle, curl_handle) < 0){
        iot_device_sdk_log(SDK_LOG_ERROR, "send_device_data_curl() set cert verfication options failed \n");
        ret = -1;
        goto cleanup;
      }

      //curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 1L);
      // initialize the writedata struct
      memset(&writedata, 0, sizeof(curlResponseStruct ));
      writedata.curlHandle = curl_handle;

      /* set up headers */
      headers = curl_slist_append(headers, "Content-Type: application/json");
      headers = curl_slist_append(headers, "Accept: application/json");

      if(tags){
        ret = asprintf(&(route_header), "route: /v1/%s/json/dev2app/%s",
                       (((SDKDeviceInternal*)clientHandle)->registrationData.regResponseCPODSvr.thingUid), tags);
      } else {
        ret = asprintf(&(route_header), "route: /v1/%s/json/dev2app/",
                       (((SDKDeviceInternal*)clientHandle)->registrationData.regResponseCPODSvr.thingUid));
      }
      if(ret < 0 || !(route_header)){
        iot_device_sdk_log(SDK_LOG_ERROR, "send_device_data_curl()  failed to build auth header 1 \n");
        goto cleanup;
      }
      headers = curl_slist_append(headers, route_header);

      ret = asprintf(&(auth_header), "Authorization: bearer %s",
                     ((SDKDeviceInternal*)clientHandle)->registrationData.accessTokenDetails.access_token);
      if(ret < 0 || !(auth_header)){
        iot_device_sdk_log(SDK_LOG_ERROR, "send_device_data_curl()  failed to build auth header 2\n");
        goto cleanup;
      }
      headers = curl_slist_append(headers, auth_header);
      if(curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, headers) != CURLE_OK) {
        iot_device_sdk_log(SDK_LOG_ERROR, "send_device_data_curl() curl CURLOPT_HTTPHEADER failed \n");
        ret = -1;
        goto cleanup;
      }

      /*  specify we want to POST data */
      if(curl_easy_setopt(curl_handle, CURLOPT_POST, 1L)!= CURLE_OK) {
        iot_device_sdk_log(SDK_LOG_ERROR, "send_device_data_curl() curl CURLOPT_POST failed \n");
        ret = -1;
        goto cleanup;
      }

      /* size of the POST data */
      if(curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDSIZE, strlen(data))!= CURLE_OK) {
        iot_device_sdk_log(SDK_LOG_ERROR, "send_device_data_curl() curl CURLOPT_POSTFIELDSIZE failed \n");
        ret = -1;
        goto cleanup;
      }
      /* pointer to the POST data */
      if(curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, data)!= CURLE_OK) {
        iot_device_sdk_log(SDK_LOG_ERROR, "send_device_data_curl() curl CURLOPT_POSTFIELDS failed \n");
        ret = -1;
        goto cleanup;
      }
      /* write callback function */
      if(curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, curlWriteCallback)!= CURLE_OK) {
        iot_device_sdk_log(SDK_LOG_ERROR, "send_device_data_curl() curl CURLOPT_WRITEFUNCTION failed \n");
        ret = -1;
        goto cleanup;
      }
      /* pass data pointer to the write callback */
      if(curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void*)(&writedata))!= CURLE_OK) {
        iot_device_sdk_log(SDK_LOG_ERROR, "send_device_data_curl() curl CURLOPT_WRITEDATA failed \n");
        ret = -1;
        goto cleanup;
      }

      /* complete within n seconds : to come from config?*/
      if(curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, IOT_DEFAULT_CURL_TIMEOUT)!= CURLE_OK) {
        iot_device_sdk_log(SDK_LOG_ERROR, "send_device_data_curl() curl CURLOPT_TIMEOUT failed \n");
        ret = -1;
        goto cleanup;
      }

      iot_device_sdk_log(SDK_LOG_INFO, "send_device_data_curl() BEFORE CURL EASY PERFORM \n");
      ret_val = curl_easy_perform(curl_handle);
      /* check for errors */
      if(ret_val != CURLE_OK) {
        iot_device_sdk_log(SDK_LOG_ERROR, "send_device_data_curl()->curl_easy_perform() failed %s \n",curl_easy_strerror(ret_val));
        ret = -1;
        goto cleanup;
      } else {
        long http_code = 0;
        curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &http_code);
        responseData->responseCode = http_code;
        SAFE_FREE(responseData->response)
        // use writedata.data instead of freeing
        responseData->response = writedata.data;
      }
    }
  } else {
    iot_device_sdk_log(SDK_LOG_ERROR, "send_device_data_curl() url or data passed is NULL \n");
    return -1;
  }

cleanup:
  if(auth_header)
    free(auth_header);
  if(route_header)
    free(route_header);
  if(headers)
    curl_slist_free_all(headers);
  curl_easy_cleanup(curl_handle);
  iot_device_sdk_log(SDK_LOG_INFO, "send_device_data_curl() -- \n");
  return ret;
}
