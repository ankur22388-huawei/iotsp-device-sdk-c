/**************************************************************************
  copyright (C) Copyright 2015-2016 Cisco Systems Inc.; All Rights Reserved
  History:
    01/12/2016 - Created.
 ******************************************************************************/
#include <stdlib.h>
#include <assert.h>

#include <iot_device_sdk.h>
#include "_iot_device_sdk_init.h"
#include "iot_device_sdk_curl.h"
#include "iot_device_sdk_storage_cache.h"

#define MAX_PATH 4096 // TODO, come back to this

static SDKDeviceInternal *sdkClientP;

static int first = 1;

extern int json_releaseRegistrationMessageData(SDKDeviceInternal* clientHdle);
extern int logging_set_filepath(const char *logFile);
extern int load_sdk_config( SDKDeviceInternal* clientP, char* sdkConfiPath);


static int checkDeviceDetails(DeviceDetails *deviceDetail){
  //Check for mandatory feilds in deviceDetails Structure
  if(deviceDetail){
    //manufacturingId is the only mandatory Field
    if(!deviceDetail->manufacturingId){
      iot_device_sdk_log(SDK_LOG_ERROR,"checkDeviceDetails ERROR manufacturingId is not Set\n");
      return -1;
    }
  }
  return 0;
}

static int getDeviceDetails(SDKDeviceInternal* clientP, DeviceDetails *deviceDetail){
  if(clientP && deviceDetail) {
    if(deviceDetail->manufacturingId){
      clientP->deviceDetails.manufacturingId = strdup(deviceDetail->manufacturingId);
    }
    if(deviceDetail->deviceType){
      clientP->deviceDetails.deviceType = strdup(deviceDetail->deviceType);
    }
    if(deviceDetail->deviceMake){
      clientP->deviceDetails.deviceMake = strdup(deviceDetail->deviceMake);
    }
    if(deviceDetail->deviceModel){
      clientP->deviceDetails.deviceModel = strdup(deviceDetail->deviceModel);
    }
    if(deviceDetail->deviceFirmwareVer){
      clientP->deviceDetails.deviceFirmwareVer = strdup(deviceDetail->deviceFirmwareVer);
    }
    if(deviceDetail->hardwareVer){
      clientP->deviceDetails.hardwareVer = strdup(deviceDetail->hardwareVer);
    }
    if(deviceDetail->macAddress){
      clientP->deviceDetails.macAddress = strdup(deviceDetail->macAddress);
    }
    if(deviceDetail->deviceSerialNum){
      clientP->deviceDetails.deviceSerialNum = strdup(deviceDetail->deviceSerialNum);
    }
    if(deviceDetail->ipv4){
      clientP->deviceDetails.ipv4 = strdup(deviceDetail->ipv4);
    }
    if(deviceDetail->ipv6){
      clientP->deviceDetails.ipv6 = strdup(deviceDetail->ipv6);
    }
    if(deviceDetail->alt_id1){
      clientP->deviceDetails.alt_id1 = strdup(deviceDetail->alt_id1);
    }
    if(deviceDetail->sdkConfigFile){
      clientP->deviceDetails.sdkConfigFile = strdup(deviceDetail->sdkConfigFile);
    }
    clientP->deviceDetails.skip_registration = deviceDetail->skip_registration;
  } else {
    return -1;
  }
  return 0;
}

/**
 * iot_device_sdk_init ( ) function is the first function to be called to initilize
 * IOT DEVICE SDK library.
 * Input  : None
 * return :  IotDeviceSDKClientP - Instance Object
 *           NULL on Failure
*/
IotDeviceSdkClient iot_device_sdk_init_v1 (DeviceDetails *deviceDetail){
  /* Setup the Device SDK Logging */
  logging_set_filepath (DEFAULT_LOG_FILEPATH);

  iot_device_sdk_log(SDK_LOG_INFO,"iot_device_sdk_init_v1 --Start--\n");

  if(first){
    //Populate device entry storage DB cache
    if(init_device_storage_cache() < 0){
      iot_device_sdk_log(SDK_LOG_ERROR,"iot_device_sdk_init_v1 device BD cache init Failed \n");
      return NULL;
    }
    first = 0;
    //init curl interface
    sdk_init_curl();
  }

  if(deviceDetail) {
    //Check the device details info that the App has passed
    if(checkDeviceDetails(deviceDetail) < 0) {
      iot_device_sdk_log(SDK_LOG_ERROR,"iot_device_sdk_init_v1 Fill the mandatory feilds in DeviceDetails struct\n");
      return NULL;
    }

    //Check if device is already present in device DB cache
    sdkClientP = get_device_Entry_from_cache(deviceDetail->manufacturingId);
    if(sdkClientP) {
      iot_device_sdk_log(SDK_LOG_INFO,"iot_device_sdk_init_v1 Device [%s] found in the device Entry DB cache \n",deviceDetail->manufacturingId);
    }

    /* If Application does not call deinit - it will create a memory leak */
    if(sdkClientP == NULL) {
      sdkClientP = (SDKDeviceInternal*)calloc(1, sizeof(SDKDeviceInternal));
      if(sdkClientP == NULL) {
        iot_device_sdk_log(SDK_LOG_ERROR,"iot_device_sdk_init_v1 calloc Failed\n");
        return NULL;
      } else {

        /* Read the sdk_config.json file to load config Information */
        if(load_sdk_config(sdkClientP, deviceDetail->sdkConfigFile ) < 0 ) {
          iot_device_sdk_log(SDK_LOG_ERROR,"iot_device_sdk_init_v1 Error: Load config from sdk_config.json failed\n");
          return NULL;
        } else {
          iot_device_sdk_log(SDK_LOG_INFO,"iot_device_sdk_init_v1 Load config from sdk_config.json Success\n");
          // Put the skip_registration read from .ini into the passed in deviceDetail
          deviceDetail->skip_registration = sdkClientP->deviceDetails.skip_registration;
        }

        //Get the device details
        if(getDeviceDetails(sdkClientP, deviceDetail) < 0){
          iot_device_sdk_log(SDK_LOG_ERROR,"iot_device_sdk_init_v1 Error: populate deviceDetails Failed \n");
        }

        // Add device to cache
        iot_device_sdk_log(SDK_LOG_INFO,"iot_device_sdk_init_v1 - adding device to cache\n");
        //Add device to the cache
        if(add_device_Entry_to_cache (sdkClientP) < 0) {
          iot_device_sdk_log(SDK_LOG_ERROR, "IOT_SDK iot_device_sdk_data_send_v2 add_device_Entry_to_cache Failed \n");
          return NULL;
        }

        iot_device_sdk_log(SDK_LOG_INFO,"iot_device_sdk_init_v1 --Complete--\n");
        return (IotDeviceSdkClient)sdkClientP;
      }
    } else {

      if(load_sdk_config(sdkClientP, deviceDetail->sdkConfigFile ) < 0 ) {
        iot_device_sdk_log(SDK_LOG_ERROR,"iot_device_sdk_init_v1 Error: Load config from sdk_config.json failed\n");
        return NULL;
      } else {
        iot_device_sdk_log(SDK_LOG_INFO,"iot_device_sdk_init_v1 Load config from sdk_config.json Success\n");
        // Put the skip_registration read from .ini into the passed in deviceDetail
        deviceDetail->skip_registration = sdkClientP->deviceDetails.skip_registration;
      }

      iot_device_sdk_log(SDK_LOG_WARN,"iot_device_sdk_init_v1 INFO: SDK Client Already Initialized for device %s\n", sdkClientP->deviceDetails.manufacturingId);
      return (IotDeviceSdkClient)sdkClientP;;
    }
  } else {
    iot_device_sdk_log(SDK_LOG_ERROR,"iot_device_sdk_init_v1 Error: NULL ptr passed for deviceDetail \n");
    return NULL;
  }
}


/**
 * iot_device_sdk_deinit ( ) fucntion to be called when exiting Client/App Code
 * Input  : None
 * return :  0 - success
 *          -1 - Error
*/
int iot_device_sdk_deinit_v1 (void){

  /* De-allocate Any Resource that was Allocated and notify if anyone
  * needs to be notified */
  if(sdkClientP) {
    if(sdkClientP->deviceDataSerializationType) free(sdkClientP->deviceDataSerializationType);
    if(sdkClientP->certAuthorityBundleFile) free(sdkClientP->certAuthorityBundleFile);
    if(sdkClientP->regURI) free(sdkClientP->regURI);
    json_releaseRegistrationMessageData(sdkClientP);
    sdk_deinit_curl();
    free(sdkClientP);
  }
  iot_device_sdk_log(SDK_LOG_INFO,"iot_device_sdk_deinit_v1 --Complete-- App Can Exit Cleanly\n");
  return 0;
}
