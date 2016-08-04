/**************************************************************************
  copyright (C) Copyright 2015-2016 Cisco Systems Inc.; All Rights Reserved
  History:
    07/06/2016 - Created.
 ******************************************************************************/
#define _GNU_SOURCE
#include <stdlib.h>
#include <iot_device_sdk.h>
#include "_iot_device_sdk_init.h"
#include "iot_device_sdk_curl.h"
#include "iot_device_sdk_json.h"

static int dsdk_getAccessToken(SDKDeviceInternal *handle, AccessTokenDetails *accessTokenResponse, int token_refresh ) {
  char *data=NULL;
  int ret = -1;

  if(handle && accessTokenResponse){
    //First try to use refresh token - if that fails use the username/secret
    if(handle->registrationData.accessTokenDetails.refresh_token && token_refresh){
      iot_device_sdk_log(SDK_LOG_INFO, "dsdk_getAccessToken() using refresh_token to request a new Access Toke \n");
      //Construct the data fields
      ret = asprintf(&(data), "grant_type=refresh_token&refresh_token=%s&client_id=iotspdeviceoauth2client&client_secret=iotspdeviceoauth2client"
                      ,handle->registrationData.accessTokenDetails.refresh_token);
      if(ret < 0 || !(data)){
        iot_device_sdk_log(SDK_LOG_ERROR, "dsdk_getAccessToken()  failed to build data payload\n");
        return -1;
      }
      iot_device_sdk_log(SDK_LOG_INFO, "dsdk_getAccessToken post data [%s] \n", data);
    } else {
      //Construct the data fields
      ret = asprintf(&(data), "grant_type=password&username=%s&password=%s&client_id=iotspdeviceoauth2client&client_secret=iotspdeviceoauth2client"
                     ,handle->registrationData.device_username, handle->registrationData.device_password);
      if(ret < 0 || !(data)){
        iot_device_sdk_log(SDK_LOG_ERROR, "dsdk_getAccessToken()  failed to build data payload\n");
        return -1;
      }
      iot_device_sdk_log(SDK_LOG_INFO, "dsdk_getAccessToken post data =%s \n", data);
    }

    if(handle->registrationData.regUrlIAMSvr) {
      if( send_AccessTokenRequest_curl(handle,
        handle->registrationData.regUrlIAMSvr, data, accessTokenResponse) < 0) {
        iot_device_sdk_log(SDK_LOG_ERROR, "dsdk_getAccessToken()  send_AccessTokenRequest_curl Failed \n");
        if (data) free(data);
          return -1;
      }
      if (data) free(data);
        //parse the accessTokenResponse
      if(deserializeAccessTokenResponse_json((IotDeviceSdkClient)handle,
          (const char*)(accessTokenResponse->response), accessTokenResponse) < 0){
        iot_device_sdk_log(SDK_LOG_ERROR, "dsdk_getAccessToken() deserializeAccessTokenResponse_json Failed \n");
      } else {
        if(accessTokenResponse->responseCode == SDK_SERVER_SUCCESS){
          iot_device_sdk_log(SDK_LOG_INFO, "dsdk_getAccessToken() get Access Token Success http_code = %d \n", accessTokenResponse->responseCode);
          if(clock_gettime(CLOCK_MONOTONIC, &(handle->registrationData.startTokenTime)) != 0){
            iot_device_sdk_log(SDK_LOG_INFO, "dsdk_getAccessToken() start Token Expiry timer Failed \n");
          }
        } else {
          iot_device_sdk_log(SDK_LOG_ERROR, "dsdk_getAccessToken() get Access Token Failed http_code = %d \n", accessTokenResponse->responseCode);
          return -1;
        }
      }
    }
  }
  return 0;
}

int iot_dsdk_getAccessToken(IotDeviceSdkClient clientHandle, AccessTokenDetails *accessTokenResponse)
{
  SDKDeviceInternal *clientHdle=NULL;
  struct timespec nowTime;
  long timeElapsed = 0;

  if(!clientHandle || !accessTokenResponse) {
    iot_device_sdk_log(SDK_LOG_ERROR, "iot_dsdk_getAccessToken Client handle NULL or NULL accessTokenResponse \n");
    return -1;
  }
  clientHdle = (SDKDeviceInternal*)clientHandle;

  //Check if the current access token is set
  if(clientHdle->registrationData.accessTokenDetails.access_token) {
    //Check if access token is valid
    if(clock_gettime(CLOCK_MONOTONIC, &(nowTime))!= 0){
      iot_device_sdk_log(SDK_LOG_INFO, "iot_dsdk_getAccessToken() start Token Expiry timer Failed \n");
    }
    timeElapsed = (nowTime.tv_sec - clientHdle->registrationData.startTokenTime.tv_sec);
    iot_device_sdk_log(SDK_LOG_INFO, "iot_dsdk_getAccessToken() ## timeElapsed = %ld Seconds\n", timeElapsed);
    if(timeElapsed > clientHdle->registrationData.accessTokenDetails.access_token_expires_in * 2/3){
      iot_device_sdk_log(SDK_LOG_INFO, "iot_dsdk_getAccessToken() Access Token has expired\n");
      if(dsdk_getAccessToken(clientHdle, accessTokenResponse, 1)<0){
        iot_device_sdk_log(SDK_LOG_ERROR, "iot_dsdk_getAccessToken() dsdk_getAccessToken Field\n");
        return -1;
      }
    } else {
      iot_device_sdk_log(SDK_LOG_INFO, "iot_dsdk_getAccessToken() Token Not expired - reusing it \n");
      return 0;
    }
  } else {
    if(dsdk_getAccessToken(clientHdle, accessTokenResponse, 0)<0){
      iot_device_sdk_log(SDK_LOG_ERROR, "iot_dsdk_getAccessToken() dsdk_getAccessToken Field\n");
      return -1;
    }
  }
  return 0;
}

const AccessTokenDetails* iot_dsdk_getAccessToken_v1(IotDeviceSdkClient clientHandle){

  iot_device_sdk_log(SDK_LOG_INFO, "iot_dsdk_getAccessToken_v1 ++ \n");
  if(!clientHandle) {
    iot_device_sdk_log(SDK_LOG_ERROR, "iot_dsdk_getAccessToken_v1 sdk Client handle NULL \n");
    return NULL;
  }
  SDKDeviceInternal* clientHdle=(SDKDeviceInternal*)clientHandle;

  if(iot_dsdk_getAccessToken(clientHandle, &(clientHdle->registrationData.accessTokenDetails)) < 0){
    iot_device_sdk_log(SDK_LOG_ERROR, "iot_dsdk_getAccessToken_v1() get Access Token Field\n");
    return NULL;
  } else {
    return  &(clientHdle->registrationData.accessTokenDetails);
  }
  return NULL;
}
