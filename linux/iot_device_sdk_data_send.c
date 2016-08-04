/**************************************************************************
  copyright (C) Copyright 2015-2016 Cisco Systems Inc.; All Rights Reserved
  History:
    02/10/2016 - Created.
 ******************************************************************************/
#define _GNU_SOURCE
#include <stdlib.h>
#include <assert.h>

#include <iot_device_sdk.h>
#include "_iot_device_sdk_init.h"
#include "iot_device_sdk_registration.h"
#include "iot_device_sdk_access_token.h"
#include "iot_device_sdk_json.h"
#include "iot_device_sdk_curl.h"

static int dsdk_send_device_data_https (SDKDeviceInternal *clientHandle, char* jsonData, char* tags)
{
  if(clientHandle) {
    if(clientHandle->registrationData.observationURL) {
      //Get Access Token (Token Refresh?)
      if(iot_dsdk_getAccessToken((IotDeviceSdkClient)clientHandle, &(clientHandle->registrationData.accessTokenDetails)) < 0 ){
        iot_device_sdk_log(SDK_LOG_INFO,"IOT_SDK dsdk_send_device_data_https  Get Access token Failed \n");
        return -1;
      }
      // Check for access token
      if (clientHandle->registrationData.accessTokenDetails.access_token){
        if (send_device_data_curl(clientHandle, (const char*)clientHandle->registrationData.observationURL,
            (const char*)jsonData, (const char*) tags, &(clientHandle->registrationData.dataPostResponse)) < 0) {
            iot_device_sdk_log(SDK_LOG_ERROR,"IOT_SDK dsdk_send_device_data_https Send data Failed \n");
            //parse the responseData
            if(deserializeDataPostResponse_json((IotDeviceSdkClient)clientHandle,
              clientHandle->registrationData.dataPostResponse.response, &(clientHandle->registrationData.dataPostResponse)) < 0)
              iot_device_sdk_log(SDK_LOG_ERROR, "IOT_SDK dsdk_send_device_data_https deserializeDataPostResponse_json Failed \n");
            return -1;
         } else {
            if(deserializeDataPostResponse_json((IotDeviceSdkClient)clientHandle,
               clientHandle->registrationData.dataPostResponse.response, &(clientHandle->registrationData.dataPostResponse)) < 0){
               iot_device_sdk_log(SDK_LOG_ERROR, "IOT_SDK dsdk_send_device_data_https  deserializeDataPostResponse_json Failed \n");
            } else {
              if(clientHandle->registrationData.dataPostResponse.responseCode == SDK_DATA_SEND_SUCCESS) {
                iot_device_sdk_log(SDK_LOG_INFO,"IOT_SDK dsdk_send_device_data_https Send data Success http_code = %d \n",
                                                  clientHandle->registrationData.dataPostResponse.responseCode);
              } else {
                iot_device_sdk_log(SDK_LOG_INFO,"IOT_SDK dsdk_send_device_data_https Send data Failed http_code = %d \n",
                                                  clientHandle->registrationData.dataPostResponse.responseCode);
                return -1;
              }
            }
         }
      } else {
        iot_device_sdk_log(SDK_LOG_ERROR,"IOT_SDK dsdk_send_device_data_https NO access_token Set \n");
        return -1;
      }
    } else {
      iot_device_sdk_log(SDK_LOG_ERROR,"IOT_SDK dsdk_send_device_data_https observationURL is NULL \n");
      return -1;
    }
  }
  return 0;
}

static int dsdk_send_device_data_mqtt(SDKDeviceInternal *clientHandle, char* jsonData, int jsonData_len, char* tags){
  int ret = -1;

  if(!clientHandle) {
    iot_device_sdk_log(SDK_LOG_ERROR, "dsdk_send_device_data_mqtt Client handle NULL  \n");
    return -1;
  }
  //Setup Mqtt Only Once
  if(!clientHandle->mqttClient.client) {
    ret = iot_mqtt_client_init((IotDeviceSdkClient)clientHandle);
    if(ret < 0 ){
       iot_device_sdk_log(SDK_LOG_ERROR, "dsdk_send_device_data_mqtt iot_mqtt_client_init failed ret =%d \n", ret );
       return -1;
    } else {
      iot_device_sdk_log(SDK_LOG_INFO,"dsdk_send_device_data_mqtt iot_mqtt_client_init Success =%d \n", ret );
      ret = iot_mqtt_client_connect((IotDeviceSdkClient)clientHandle);
      if(ret < 0 ){
        iot_device_sdk_log(SDK_LOG_ERROR, " iot_mqtt_client_connect failed ret =%d \n", ret );
        return -1;
      } else iot_device_sdk_log(SDK_LOG_INFO, "dsdk_send_device_data_mqtt iot_mqtt_client_connect Success =%d \n", ret );
   }
  } else {
   iot_device_sdk_log(SDK_LOG_INFO, "dsdk_send_device_data_mqtt mqtt client aready initilized - checking connection \n");
   //Add an API to check the mqtt connection.
   if(mqtt_connectionStatus(&(clientHandle->mqttClient)) == 1){
     iot_device_sdk_log(SDK_LOG_INFO, "dsdk_send_device_data_mqtt mqtt client Status : Connected \n");
   } else {
     iot_device_sdk_log(SDK_LOG_ERROR, "dsdk_send_device_data_mqtt mqtt client status : NOT Connected \n");
     ret = iot_mqtt_client_connect((IotDeviceSdkClient)clientHandle);
     if(ret < 0 ){
      iot_device_sdk_log(SDK_LOG_ERROR, "dsdk_send_device_data_mqtt iot_mqtt_client_connect 2 failed ret =%d \n", ret );
      return -1;
     } else {
      iot_device_sdk_log(SDK_LOG_INFO, "dsdk_send_device_data_mqtt iot_mqtt_client_connect 2 Success =%d \n", ret );
     }
   }
  }
  //Construct the mqtt topics with tags
  SAFE_FREE(clientHandle->mqttClient.mqtt_pub_topic)
  if(!tags){
    ret = asprintf(&(clientHandle->mqttClient.mqtt_pub_topic), "/v1/%s/json/dev2app"
                    ,clientHandle->registrationData.regResponseCPODSvr.thingUid);
  } else {
    ret = asprintf(&(clientHandle->mqttClient.mqtt_pub_topic), "/v1/%s/json/dev2app/%s"
                    ,clientHandle->registrationData.regResponseCPODSvr.thingUid, tags);
  }
  if(ret > 0) {
    iot_device_sdk_log(SDK_LOG_INFO,"dsdk_send_device_data_mqtt mqtt_pub_topic [%s] \n", clientHandle->mqttClient.mqtt_pub_topic);
  } else  {
    iot_device_sdk_log(SDK_LOG_ERROR,"dsdk_send_device_data_mqtt set mqtt_pub_topic Failed  \n");
    return -1;
  }

  //Send data over mqtt
  ret = iot_mqtt_client_publish((IotDeviceSdkClient)clientHandle, clientHandle->mqttClient.mqtt_pub_topic, jsonData ,jsonData_len, 1);
  if(ret < 0 ) {
   iot_device_sdk_log(SDK_LOG_ERROR,"dsdk_send_device_data_mqtt iot_mqtt_client_publish failed ret =%d \n", ret );
   return -1;
  } else {
   iot_device_sdk_log(SDK_LOG_INFO, "dsdk_send_device_data_mqtt iot_mqtt_client_publish Success =%d \n", ret );
  }
  return 0;
}

/**
 * iot_device_sdk_data_send_v2 ( ) function to send device data
 * IOT DEVICE SDK library.
 * Input  : None
 * return :  IotDeviceSDKClientP - Instance Object
 *           NULL on Failure
*/

int iot_device_sdk_data_send_v2 (DeviceDetails *deviceDetail, char* jsonData, int jsonData_len, char* tags, __attribute__((unused)) int tags_len)
{
  SDKDeviceInternal *devicesdkPtr = NULL;

  iot_device_sdk_log(SDK_LOG_INFO,"iot_device_sdk_data_send_v2 --Start--\n");

  if(jsonData_len <= 0 || !jsonData){
    iot_device_sdk_log(SDK_LOG_ERROR,"iot_device_sdk_data_send_v2 Error data length <= ZERO or NULL jsonData \n");
    return -1;
  }

  if(deviceDetail) {
    devicesdkPtr = (SDKDeviceInternal*)iot_device_sdk_init_v1(deviceDetail);
    if(!devicesdkPtr) {
      printf("iot_device_sdk_data_send_v2:  iot_device_sdk_init() call failed - check iot_device_sdk.log File\n");
      iot_device_sdk_log(SDK_LOG_WARN,"iot_device_sdk_data_send_v2  iot_device_sdk_init() call failed\n");
      return -1;
    }

    if(deviceDetail->skip_registration) {
      iot_device_sdk_log(SDK_LOG_INFO,"iot_device_sdk_data_send_v2 skipping registration \n");
    } else {
      //Check if device already registered
      if(!(devicesdkPtr->registrationData.regResponseCPODSvr.registered && devicesdkPtr->registrationData.regResponseCPODSvr.claimed) ) {
        if(iot_device_register((IotDeviceSdkClient)devicesdkPtr, deviceDetail) < 0 ) {
          iot_device_sdk_log(SDK_LOG_ERROR,"iot_device_sdk_data_send_v2  Device Registration Failed\n");
          return -1;
        } else {
          iot_device_sdk_log(SDK_LOG_INFO,"iot_device_sdk_data_send_v2  Device Registration Successful\n");
        }
      }
    }

    if((devicesdkPtr->registrationData.regResponseCPODSvr.registered && devicesdkPtr->registrationData.regResponseCPODSvr.claimed) ||
      deviceDetail->skip_registration){

      iot_device_sdk_log(SDK_LOG_INFO,"iot_device_sdk_data_send_v2  Device data Connector Type = %d [0-mqtt, 1-https]\n",devicesdkPtr->deviceDetails.dataConnectorType);
      // Support Multiple data connectors (mqtt, https, wamp )
      switch (devicesdkPtr->deviceDetails.dataConnectorType) {
        default:
        case CONNECTOR_TYPE_MQTT:
         //Pass the json data len ?
         if(dsdk_send_device_data_mqtt(devicesdkPtr, jsonData, jsonData_len, tags) < 0){
           iot_device_sdk_log(SDK_LOG_ERROR,"IOT_SDK iot_device_sdk_data_send_v2 MQTT Send data Failed \n");
           return -1;
         } else {
           iot_device_sdk_log(SDK_LOG_INFO,"IOT_SDK iot_device_sdk_data_send_v2 MQTT Send data Success \n");
         }
        break;

        case CONNECTOR_TYPE_HTTPS:
          //Pass the json data len ?
          if(dsdk_send_device_data_https(devicesdkPtr, jsonData, tags) < 0){
            iot_device_sdk_log(SDK_LOG_ERROR,"IOT_SDK iot_device_sdk_data_send_v2 HTTPS Send data Failed \n");
            return -1;
          } else {
            iot_device_sdk_log(SDK_LOG_INFO,"IOT_SDK iot_device_sdk_data_send_v2 HTTPS Send data Success \n");
          }
        break;

        case CONNECTOR_TYPE_WAMP:
          // TBD
        break;
      }
    }

  } else {
    iot_device_sdk_log(SDK_LOG_ERROR,"iot_device_sdk_data_send_v2 Error: NULL ptr passed for deviceDetail \n");
    return -1;
  }

  iot_device_sdk_log(SDK_LOG_INFO,"iot_device_sdk_data_send_v2 --End--\n");
  return 0;
}

int iot_device_sdk_data_send_v1 (IotDeviceSdkClient clientHandle, char* jsonData, int jsonData_len, char* tags, __attribute__((unused)) int tags_len)
{
  iot_device_sdk_log(SDK_LOG_INFO, "iot_device_sdk_data_send_v1 ++ \n");
  if(!clientHandle) {
    iot_device_sdk_log(SDK_LOG_ERROR, "iot_device_sdk_data_send_v1 sdk Client handle NULL \n");
    return -1;
  }
  SDKDeviceInternal* clientHdle=(SDKDeviceInternal*)clientHandle;

  if(jsonData_len <= 0 || jsonData == NULL ){
    iot_device_sdk_log(SDK_LOG_INFO,"iot_device_sdk_data_send_v1 Error data length <= ZERO or jsonData is NULL\n");
    return -1;
  }

  if(clientHdle->registrationData.regResponseCPODSvr.registered && clientHdle->registrationData.regResponseCPODSvr.claimed){
    iot_device_sdk_log(SDK_LOG_INFO,"iot_device_sdk_data_send_v1  Device data Connector Type = %d [0-mqtt, 1-https]\n",clientHdle->deviceDetails.dataConnectorType);
    // Support Multiple data connectors (mqtt, https, wamp )
    switch (clientHdle->deviceDetails.dataConnectorType) {
      default:
      case CONNECTOR_TYPE_MQTT:
       //Pass the json data len ?
       if(dsdk_send_device_data_mqtt(clientHdle, jsonData, jsonData_len, tags) < 0){
         iot_device_sdk_log(SDK_LOG_ERROR,"IOT_SDK iot_device_sdk_data_send_v1 MQTT Send data Failed \n");
         return -1;
       } else {
         iot_device_sdk_log(SDK_LOG_INFO,"IOT_SDK iot_device_sdk_data_send_v1 MQTT Send data Success \n");
       }
      break;

      case CONNECTOR_TYPE_HTTPS:
        //Pass the json data len ?
        if(dsdk_send_device_data_https(clientHdle, jsonData, tags) < 0){
          iot_device_sdk_log(SDK_LOG_ERROR,"IOT_SDK iot_device_sdk_data_send_v1 HTTPS Send data Failed \n");
          return -1;
        } else {
          iot_device_sdk_log(SDK_LOG_INFO,"IOT_SDK iot_device_sdk_data_send_v1 HTTPS Send data Success \n");
        }
      break;

      case CONNECTOR_TYPE_WAMP:
        // TBD
      break;
    }
  } else {
    iot_device_sdk_log(SDK_LOG_ERROR,"IOT_SDK iot_device_sdk_data_send_v1 Device Not yet Registered \n");
    return -1;
  }
  return 0;
}
