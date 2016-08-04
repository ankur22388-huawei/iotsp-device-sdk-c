/**************************************************************************
copyright (C) Copyright 2015-2016 Cisco Systems Inc.; All Rights Reserved
History:
        03/18/2016 - Created.
******************************************************************************/

/*****************************************************************************************************************
* System Include Files
******************************************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <pthread.h>
#include <string.h>
#include <dirent.h>

/*****************************************************************************************************************
* Include Files
******************************************************************************************************************/
#include <iot_device_sdk.h>
#include "_iot_device_sdk_init.h"
#include "iot_device_sdk_storage_cache.h"
#include "iot_device_sdk_utils.h"
#include "iot_device_sdk_storage.h"
#include "iot_device_sdk_json.h"

/*****************************************************************************************************************
* Defines
******************************************************************************************************************/
//Device Entry Object List
static POBJLIST pDeviceList = NULL;
static pthread_mutex_t deviceListMutex = PTHREAD_MUTEX_INITIALIZER;

/*****************************************************************************************************************
* Function Name : init_device_storage_cache
* Description   : initilizes the device storage cache
* return        : 0  - on Sucess
*                 -1 - on Failure
******************************************************************************************************************/
int init_device_storage_cache()
{
  DIR *dirHdle = NULL;
  struct dirent *fileEntry;
  SDKDeviceInternal *deviceEntry = NULL;
  int device_count = 0;

  iot_device_sdk_log(SDK_LOG_MSG,"IOT_SDK Initializing device storage cache \n");

  //Create the device Object list (list depth 10, growth rate 5)
  pDeviceList = Utils_ObjListCreate(10, 5);
  if(!pDeviceList) {
    iot_device_sdk_log(SDK_LOG_ERROR,"IOT_SDK device cache list creation FAILED \n");
    return -1;
  } else {
    //Scan the storage folder and add the device entries into cache
    dirHdle = opendir(STORAGE_PREFIX);
    if(dirHdle != NULL) {
      fileEntry = readdir(dirHdle);
      while (fileEntry){
        if(!strcmp(fileEntry->d_name, ".") || !strcmp(fileEntry->d_name, "..")) {
            fileEntry = readdir(dirHdle);
            continue;
        }
        deviceEntry = NULL;
        deviceEntry = iot_load_device_json_into_devicestruct((char*)fileEntry->d_name);
        if(!deviceEntry) {
          iot_device_sdk_log(SDK_LOG_WARN,"IOT_SDK %s loading device with ID [%s] FAILED \n", __FUNCTION__,fileEntry->d_name);
          continue;
        } else {
          //Push the device entry into the device cache List
          if(add_device_Entry_to_cache (deviceEntry) < 0) {
            iot_device_sdk_log(SDK_LOG_WARN,"IOT_SDK %s  adding device [%s] to device cache list FAILED \n", __FUNCTION__,fileEntry->d_name);
          } else {
            device_count++;
            iot_device_sdk_log(SDK_LOG_MSG,"IOT_SDK %s loading device with ID [%s] SUCCESSFULL \n", __FUNCTION__,fileEntry->d_name);
          }
        }
        fileEntry = readdir(dirHdle);
      }
      iot_device_sdk_log(SDK_LOG_MSG,"IOT_SDK Total [%d] devices added to storage cache \n", device_count);
      closedir(dirHdle);
    } else {
      iot_device_sdk_log(SDK_LOG_MSG,"IOT_SDK No device added to storage cache \n");
    }
  }
  return 0;
}

void free_deviceEntry_allocted_memory(SDKDeviceInternal *cacheDeviceEntry)
{
  if(cacheDeviceEntry){
    if(cacheDeviceEntry->deviceDetails.manufacturingId) free(cacheDeviceEntry->deviceDetails.manufacturingId);
    if(cacheDeviceEntry->deviceDetails.deviceType) free(cacheDeviceEntry->deviceDetails.deviceType);
    if(cacheDeviceEntry->deviceDetails.deviceMake) free(cacheDeviceEntry->deviceDetails.deviceMake);
    if(cacheDeviceEntry->deviceDetails.deviceModel) free(cacheDeviceEntry->deviceDetails.deviceModel);
    if(cacheDeviceEntry->deviceDetails.deviceFirmwareVer) free(cacheDeviceEntry->deviceDetails.deviceFirmwareVer);
    if(cacheDeviceEntry->deviceDetails.hardwareVer) free(cacheDeviceEntry->deviceDetails.hardwareVer);
    if(cacheDeviceEntry->deviceDetails.macAddress) free(cacheDeviceEntry->deviceDetails.macAddress);
    if(cacheDeviceEntry->deviceDetails.deviceSerialNum) free(cacheDeviceEntry->deviceDetails.deviceSerialNum);
    if(cacheDeviceEntry->deviceDetails.ipv4) free(cacheDeviceEntry->deviceDetails.ipv4);
    if(cacheDeviceEntry->deviceDetails.ipv6) free(cacheDeviceEntry->deviceDetails.ipv6);
    if(cacheDeviceEntry->deviceDetails.alt_id1) free(cacheDeviceEntry->deviceDetails.alt_id1);
  }
  if(cacheDeviceEntry->dataPayloadFile) free(cacheDeviceEntry->dataPayloadFile);
  if(cacheDeviceEntry->deviceDataSerializationType) free(cacheDeviceEntry->deviceDataSerializationType);
  if(cacheDeviceEntry->regURI) free(cacheDeviceEntry->regURI);
  if(cacheDeviceEntry->device_profile) free(cacheDeviceEntry->device_profile);
  if(cacheDeviceEntry->access_token) free(cacheDeviceEntry->access_token);
  if(cacheDeviceEntry->deviceCert) free(cacheDeviceEntry->deviceCert);
  if(cacheDeviceEntry->devicePrivKey) free(cacheDeviceEntry->devicePrivKey);
  if(cacheDeviceEntry->deviceKid) free(cacheDeviceEntry->deviceKid);
  if(cacheDeviceEntry->dataToSend) free(cacheDeviceEntry->dataToSend);

  if(cacheDeviceEntry->registrationData.server_url) free(cacheDeviceEntry->registrationData.server_url);
  if(cacheDeviceEntry->registrationData.regUrlPrimarySvr) free(cacheDeviceEntry->registrationData.regUrlPrimarySvr);
  if(cacheDeviceEntry->registrationData.regMsgPrimarySvrJson) free(cacheDeviceEntry->registrationData.regMsgPrimarySvrJson);
  if(cacheDeviceEntry->registrationData.regUrlCPODSvr) free(cacheDeviceEntry->registrationData.regUrlCPODSvr);
  if(cacheDeviceEntry->registrationData.regMsgCPODSvrJson) free(cacheDeviceEntry->registrationData.regMsgCPODSvrJson);
  if(cacheDeviceEntry->registrationData.regUrlIAMSvr) free(cacheDeviceEntry->registrationData.regUrlIAMSvr);
  if(cacheDeviceEntry->registrationData.observationURL) free(cacheDeviceEntry->registrationData.observationURL);

  if(cacheDeviceEntry->registrationData.regResponseCPODSvr.response) free(cacheDeviceEntry->registrationData.regResponseCPODSvr.response);
  if(cacheDeviceEntry->registrationData.regResponseCPODSvr.thingUid) free(cacheDeviceEntry->registrationData.regResponseCPODSvr.thingUid);
  if(cacheDeviceEntry->registrationData.regResponseCPODSvr.security) free(cacheDeviceEntry->registrationData.regResponseCPODSvr.security);
  if(cacheDeviceEntry->registrationData.regResponseCPODSvr.cred_name) free(cacheDeviceEntry->registrationData.regResponseCPODSvr.cred_name);
  if(cacheDeviceEntry->registrationData.regResponseCPODSvr.cred_secret) free(cacheDeviceEntry->registrationData.regResponseCPODSvr.cred_secret);

  if(cacheDeviceEntry->registrationData.regResponseCPODSvr.interval) free(cacheDeviceEntry->registrationData.regResponseCPODSvr.interval);
  if(cacheDeviceEntry->registrationData.regResponseCPODSvr.protocol) free(cacheDeviceEntry->registrationData.regResponseCPODSvr.protocol);
  if(cacheDeviceEntry->registrationData.regResponseCPODSvr.registrationURI) free(cacheDeviceEntry->registrationData.regResponseCPODSvr.registrationURI);

  return;
}

/*****************************************************************************************************************
* Function Name : destroy_device_storage_cache
* Description   : destory all the device entries in the cache and the device entry list
* return        : None
******************************************************************************************************************/
void destroy_device_storage_cache()
{
  uint32_t i = 0;
  SDKDeviceInternal *cacheDeviceEntry = NULL;

  //Destroy all the Device Object Entries
  if(pDeviceList) {
    pthread_mutex_lock(&deviceListMutex);

    for(i=0; i < Utils_ObjListGetCount(pDeviceList); i++ ) {
      cacheDeviceEntry = (SDKDeviceInternal*)Utils_ObjListGetAt(pDeviceList,i);
      if(cacheDeviceEntry) {
        //Free all the memory allocated for the device
        free_deviceEntry_allocted_memory(cacheDeviceEntry);
        free(cacheDeviceEntry);
      }
    }
    pthread_mutex_unlock(&deviceListMutex);
  }
  Utils_ObjListDestroy(pDeviceList);
  pDeviceList = NULL;

  pthread_mutex_destroy(&deviceListMutex);
  return;
}

/*****************************************************************************************************************
* Function Name : flush_all_device_entries_from_cache
* Description   : flush all device entries in cache
* return        : None
******************************************************************************************************************/
void flush_all_device_entries_from_cache()
{
  uint32_t i = 0;
  SDKDeviceInternal *cacheDeviceEntry = NULL;

  //Destroy all the Flow/Session Entries
  if(pDeviceList) {
    pthread_mutex_lock(&deviceListMutex);

    for(i=0; i < Utils_ObjListGetCount(pDeviceList);) {
      cacheDeviceEntry = (SDKDeviceInternal*)Utils_ObjListGetAt(pDeviceList,i);
      if(cacheDeviceEntry) {
        //Clean up all the allocated memory
        free_deviceEntry_allocted_memory(cacheDeviceEntry);
        Utils_ObjListRemoveAt(pDeviceList,i);
        free(cacheDeviceEntry);
      }
    }
    pthread_mutex_unlock(&deviceListMutex);
  }
  return;
}

/*****************************************************************************************************************
* Function Name : add_device_Entry_to_cache
* Description   : add a new device entry in cache
* return        : 0  - on Success
*                 -1 - on Failure
******************************************************************************************************************/
int add_device_Entry_to_cache (SDKDeviceInternal* deviceEntry)
{
  uint32_t count = 0;
  uint32_t i = 0, found = 0;
  SDKDeviceInternal *cacheDeviceEntry = NULL;

  if(!deviceEntry) {
    iot_device_sdk_log(SDK_LOG_ERROR,"IOT_SDK deviceEntry passed is NULL \n", __FUNCTION__);
    return -1;
  }

  if(pDeviceList) {
    pthread_mutex_lock(&deviceListMutex);
    count = Utils_ObjListGetCount(pDeviceList);
    if(count > 0){
      //Check device entry already present in the device list cache
      for(i=0; i<count; i++) {
        cacheDeviceEntry = (SDKDeviceInternal*)Utils_ObjListGetAt(pDeviceList,i);
        if(cacheDeviceEntry) {
          if(!strcmp(cacheDeviceEntry->deviceDetails.manufacturingId, deviceEntry->deviceDetails.manufacturingId)) {
            found = 1;
            iot_device_sdk_log(SDK_LOG_INFO,"IOT_SDK device already present in the cache \n", __FUNCTION__);
            pthread_mutex_unlock(&deviceListMutex);
            return -1;
          }
        }
      }
    }
    pthread_mutex_unlock(&deviceListMutex);
  } else {
    iot_device_sdk_log(SDK_LOG_ERROR,"IOT_SDK device cache list is NULL \n", __FUNCTION__);
    return -1;
  }

  if(!found) {
    //Device entry not found - > add the new device to device list cache
    iot_device_sdk_log(SDK_LOG_INFO,"IOT_SDK %s Adding new device [%s] to device cache list \n", __FUNCTION__, deviceEntry->deviceDetails.manufacturingId);
    pthread_mutex_lock(&deviceListMutex);
    if(pDeviceList) Utils_ObjListAdd(pDeviceList,deviceEntry);
    pthread_mutex_unlock(&deviceListMutex);
  }
  return 0;
}

/*****************************************************************************************************************
* Function Name : delete_device_entry_from_cache
* Description   : Removes a device entry from the device list cache
* return        : 0  - on Success
*                 -1 - on Failure
******************************************************************************************************************/
int delete_device_entry_from_cache (char* deviceUid)
{
  uint32_t count = 0, i = 0, found = 0;
  SDKDeviceInternal *cacheDeviceEntry = NULL;

  if(!deviceUid) {
    iot_device_sdk_log(SDK_LOG_ERROR,"IOT_SDK deviceUid passed is NULL \n", __FUNCTION__);
    return -1;
  }

  if(pDeviceList) {
    pthread_mutex_lock(&deviceListMutex);
    count = Utils_ObjListGetCount(pDeviceList);
    if(count > 0){
      for(i=0; i<count; i++) {
        cacheDeviceEntry = (SDKDeviceInternal*) Utils_ObjListGetAt(pDeviceList,i);
        if(cacheDeviceEntry) {
          if(!strcmp(cacheDeviceEntry->deviceDetails.manufacturingId, deviceUid)) {
            found = 1;
            //Free up all the memory that was allocated for this device entry
            free_deviceEntry_allocted_memory(cacheDeviceEntry);
            Utils_ObjListRemoveAt(pDeviceList,i);
            free(cacheDeviceEntry);
          }
        }
      }
    } else {
      iot_device_sdk_log(SDK_LOG_ERROR,"IOT_SDK device cache list is Empty \n", __FUNCTION__);
      pthread_mutex_unlock(&deviceListMutex);
      return -1;
    }
    pthread_mutex_unlock(&deviceListMutex);
  } else {
    iot_device_sdk_log(SDK_LOG_ERROR,"IOT_SDK device cache list is NULL \n", __FUNCTION__);
    return -1;
  }

  if(!found) {
    iot_device_sdk_log(SDK_LOG_ERROR,"IOT_SDK device not found in the decice cache list \n", __FUNCTION__);
    return -1;
  }
  return 0;
}


/*****************************************************************************************************************
* Function Name : get_device_Entry_from_cache
* Description   : get a device entry from cache
* return        : 0  - on Success
*                 -1 - on Failure
******************************************************************************************************************/
SDKDeviceInternal*  get_device_Entry_from_cache (char* deviceUid)
{
  uint32_t count = 0, i = 0, found = 0;
  SDKDeviceInternal *cacheDeviceEntry = NULL;

  if(!deviceUid) {
    iot_device_sdk_log(SDK_LOG_ERROR,"IOT_SDK deviceUid passed is NULL \n");
    return NULL;
  }
  iot_device_sdk_log(SDK_LOG_INFO,"IOT_SDK device get_device_Entry_from_cache [%s]  ++ \n", deviceUid);

  if(pDeviceList) {
    pthread_mutex_lock(&deviceListMutex);
    count = Utils_ObjListGetCount(pDeviceList);
    if(count > 0){
      //Check device entry already present in the device list cache
      for(i=0; i<count; i++) {
        cacheDeviceEntry = (SDKDeviceInternal*) Utils_ObjListGetAt(pDeviceList,i);
        if(cacheDeviceEntry) {
          if(!strcmp(cacheDeviceEntry->deviceDetails.manufacturingId, deviceUid)) {
            found = 1;
            iot_device_sdk_log(SDK_LOG_INFO,"IOT_SDK device already present in the cache \n", __FUNCTION__);
            pthread_mutex_unlock(&deviceListMutex);
            return cacheDeviceEntry;
          }
        }
      }
    }
    pthread_mutex_unlock(&deviceListMutex);
  } else {
    iot_device_sdk_log(SDK_LOG_ERROR,"IOT_SDK device cache list is NULL \n", __FUNCTION__);
    return NULL;
  }

  if(!found) {
    //Device entry not found - > add the new device to device list cache
    iot_device_sdk_log(SDK_LOG_INFO,"IOT_SDK device [%s] not found in the cache list \n", deviceUid);
    return NULL;
  }
  iot_device_sdk_log(SDK_LOG_INFO,"IOT_SDK device get_device_Entry_from_cache [%s]  -- \n", deviceUid);
  return 0;
}
