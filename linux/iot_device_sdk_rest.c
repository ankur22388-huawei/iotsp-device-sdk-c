/**************************************************************************
  copyright (C) Copyright 2015-2016 Cisco Systems Inc.; All Rights Reserved
  History:
    01/12/2016 - Created.
 ******************************************************************************/
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <assert.h>
 #include <iot_device_sdk.h>
 #include "_iot_device_sdk_init.h"
 #include "iot_device_sdk_curl.h"
 #include "iot_device_sdk_json.h"
 #include "iot_device_sdk_default_msg.h"
 #include "iot_device_sdk_storage.h"


 static int iot_BuildPrimaryRegistrationMessage(IotDeviceSdkClient handle, char* primary_reg_msg)
 {
   iot_device_sdk_log(SDK_LOG_INFO, "iot_BuildPrimaryRegistrationMessage() ++ \n");
   if(!handle) {
     iot_device_sdk_log(SDK_LOG_ERROR, "iot_BuildPrimaryRegistrationMessage Client handle NULL \n");
     return -1;
   }
   SDKDeviceInternal* clientHdle=(SDKDeviceInternal*)handle;

   if(clientHdle && primary_reg_msg) {
     if(json_loadRegistrationMessage(clientHdle, primary_reg_msg ) < 0) {
        return -1;
     } else {
      //Update the primary reg message with device specific details
        set_json_string_value(clientHdle, STR_THING_MAKE, clientHdle->deviceDetails.deviceMake);
        set_json_string_value(clientHdle, STR_THING_MODEL, clientHdle->deviceDetails.deviceModel);
        set_json_string_value(clientHdle, STR_THING_FW_VER, clientHdle->deviceDetails.deviceFirmwareVer);
        set_json_string_value(clientHdle, STR_THING_HW_VER, clientHdle->deviceDetails.hardwareVer);

        if(clientHdle->deviceDetails.macAddress && clientHdle->deviceDetails.macAddress[0])
          set_json_string_value(clientHdle, STR_THING_UI_MAC_ADDR, clientHdle->deviceDetails.macAddress);
        else
          del_json_object_node(clientHdle, ST_THING_UI,"macAddress");

        if(clientHdle->deviceDetails.deviceSerialNum && clientHdle->deviceDetails.deviceSerialNum[0])
          set_json_string_value(clientHdle, STR_THING_UI_SERIAL_NUM, clientHdle->deviceDetails.deviceSerialNum);
        else
          del_json_object_node(clientHdle, ST_THING_UI, "serialNumber");

        set_json_string_value(clientHdle, STR_THING_UI_DEVICE_UID, clientHdle->deviceDetails.manufacturingId);

        clientHdle->registrationData.regMsgPrimarySvrJson = get_json_PayLoad(clientHdle);
        json_releaseRegistrationMessageData(clientHdle);
     }
   } else {
     iot_device_sdk_log(SDK_LOG_ERROR, "iot_BuildPrimaryRegistrationMessage primary_reg_msg is NULL \n");
     return -1;
   }

   iot_device_sdk_log(SDK_LOG_INFO, "iot_BuildPrimaryRegistrationMessage() -- \n");
   return 0;
 }


 static int iot_BuildCPODRegistrationMessage(IotDeviceSdkClient handle, char* cpod_reg_msg)
 {
   json_t *json_uuid;
   iot_device_sdk_log(SDK_LOG_INFO, "iot_BuildCPODRegistrationMessage() ++ \n");
   if(!handle) {
     iot_device_sdk_log(SDK_LOG_ERROR, "iot_BuildCPODRegistrationMessage Client handle NULL \n");
     return -1;
   }
   SDKDeviceInternal* clientHdle=(SDKDeviceInternal*)handle;

   if(clientHdle && cpod_reg_msg) {
     if(json_loadRegistrationMessage(clientHdle, cpod_reg_msg ) < 0) {
       iot_device_sdk_log(SDK_LOG_ERROR, "iot_BuildCPODRegistrationMessage json_loadRegistrationMessage FAILED \n");
       return -1;
     }
     json_uuid = FindJsonNode(clientHdle, STR_THING_UI_DEVICE_UID);
     if (json_uuid && strcmp(json_string_value(json_uuid),
                   	     clientHdle->deviceDetails.manufacturingId) == 0) {
	     // we already have the device data loaded
	     printf("Found device '%s'\n", json_string_value(json_uuid));
     } else {
       //Update the primary reg message with device specific details
       set_json_string_value(clientHdle, STR_THING_MAKE, clientHdle->deviceDetails.deviceMake);
       set_json_string_value(clientHdle, STR_THING_MODEL, clientHdle->deviceDetails.deviceModel);
       set_json_string_value(clientHdle, STR_THING_FW_VER, clientHdle->deviceDetails.deviceFirmwareVer);
       set_json_string_value(clientHdle, STR_THING_HW_VER, clientHdle->deviceDetails.hardwareVer);

       if(clientHdle->deviceDetails.macAddress && clientHdle->deviceDetails.macAddress[0])
          set_json_string_value(clientHdle, STR_THING_UI_MAC_ADDR, clientHdle->deviceDetails.macAddress);
       else
          del_json_object_node(clientHdle, ST_THING_UI,"macAddress");

       if(clientHdle->deviceDetails.deviceSerialNum && clientHdle->deviceDetails.deviceSerialNum[0])
          set_json_string_value(clientHdle, STR_THING_UI_SERIAL_NUM, clientHdle->deviceDetails.deviceSerialNum);
        else
          del_json_object_node(clientHdle, ST_THING_UI, "serialNumber");

       set_json_string_value(clientHdle, STR_THING_UI_DEVICE_UID, clientHdle->deviceDetails.manufacturingId);

       //Re-Registration now requires name & secret to be sent in the reg messages
       set_json_string_value(clientHdle, STR_SER_CRED_NAME, clientHdle->registrationData.regResponseCPODSvr.cred_name);
       set_json_string_value(clientHdle, STR_SER_CRED_SECRET, clientHdle->registrationData.regResponseCPODSvr.cred_secret);

     }
     clientHdle->registrationData.regMsgCPODSvrJson = get_json_PayLoad(clientHdle);
     json_releaseRegistrationMessageData(clientHdle);
   } else {
     iot_device_sdk_log(SDK_LOG_ERROR, "iot_BuildCPODRegistrationMessage primary_reg_msg is NULL \n");
     return -1;
   }

   iot_device_sdk_log(SDK_LOG_INFO, "iot_BuildCPODRegistrationMessage() -- \n");
   return 0;
 }

/**
 * @brief iot_BuildRegistrationMessage
 * iot_BuildRegistrationMessage ( ) Function to build registration messages for
 * Primary and CPOD servers. It loads the json skeleton registration message,
 * and then makes key value set calls to update the message
 * @param[in]  : Handle to DeviceSdkClient
 * @param[in]  : server_type: Primary or CPOD
 * @return     :  0 on Success.
 *               -1 on Error.
*/
static int iot_BuildRegistrationMessage(IotDeviceSdkClient handle, int server_type)
{
  iot_device_sdk_log(SDK_LOG_INFO, "iot_BuildRegistrationMessage() ++ \n");
  if(!handle) {
    iot_device_sdk_log(SDK_LOG_ERROR, "iot_BuildRegistrationMessage Client handle NULL \n");
    return -1;
  }
  /* Depending on which server's registration message is being constructed,
   * load the default skeleton from buffer , accordingly */
  switch(server_type) {
    case SERVER_PRIMARY:
      if(iot_BuildPrimaryRegistrationMessage(handle, (char*)GetDefaultPrimaryRegMsg()) < 0 )
        return -1;
      break;
    case SERVER_CPOD:
      if(iot_BuildCPODRegistrationMessage(handle, (char*)GetDefaultCPODRegMsg()) < 0 )
        return -1;
      break;
    case SERVER_IAM:
    default:
      assert(0);
  }
  iot_device_sdk_log(SDK_LOG_INFO, "iot_BuildRegistrationMessage() -- \n");
  return 0;
}

/**
 * @brief iot_DestroyRegistrationMessage
 * iot_DestroyRegistrationMessage ( ) Function to destroy the registration message
 * skeleton created by iot_BuildRegistrationMessage.
 * @param[in]  : Handle to DeviceSdkClient
 * @return     :  0 on Success.
 *               -1 on Error.
*/
static int iot_DestroyRegistrationMessage(IotDeviceSdkClient handle)
{
  SDKDeviceInternal *clientHdle = NULL;
  iot_device_sdk_log(SDK_LOG_INFO, "iot_DestroyRegistrationMessage() ++ \n");

  clientHdle=(SDKDeviceInternal*)handle;

  if(clientHdle) {
    if(clientHdle->registrationData.regMsgPrimarySvrJson){
      free(clientHdle->registrationData.regMsgPrimarySvrJson);
      clientHdle->registrationData.regMsgPrimarySvrJson = NULL;
    }
    if(clientHdle->registrationData.regMsgCPODSvrJson){
      free(clientHdle->registrationData.regMsgCPODSvrJson);
      clientHdle->registrationData.regMsgCPODSvrJson = NULL;
    }
  }
  return 0;
}


/**
 * @brief iot_dsdk_sendPrimaryRegistrationMessage
 * iot_dsdk_sendPrimaryRegistrationMessage ( ) Function to send registration message
 * @param[in]  : Handle to DeviceSdkClient
 * @return     : SDKRegistrationResponse* on Success
 *               -1 on Error.
*/
const SDKRegistrationResponse* iot_dsdk_sendPrimaryRegistrationMessage_v1(IotDeviceSdkClient clientHandle)
{
  int ret = -1;
  SDKDeviceInternal *clientHdle = NULL;
  iot_device_sdk_log(SDK_LOG_INFO, "iot_dsdk_sendPrimaryRegistrationMessage_v1() ++ \n");

  if(clientHandle) {
    clientHdle=(SDKDeviceInternal*)clientHandle;

    // load the registration message json tree
    if(iot_BuildRegistrationMessage(clientHandle, SERVER_PRIMARY) < 0) {
      iot_device_sdk_log(SDK_LOG_ERROR, "iot_dsdk_sendPrimaryRegistrationMessage_v1() iot_BuildRegistrationMessage Failed \n");
      return NULL;
    }

    // send the registration message to the URL
    ret = send_RegistrationMessage_curl(clientHdle,
        clientHdle->registrationData.regUrlPrimarySvr,
        clientHdle->registrationData.regMsgPrimarySvrJson,
        &(clientHdle->registrationData.regResponsePrimarySvr));

    // teardown the registration message
    iot_DestroyRegistrationMessage(clientHandle);

    // If curl failed to send message (bad url.. etc), return NULL
    if (-1 == ret) {
      return NULL;
    }

    int responseCode = clientHdle->registrationData.regResponsePrimarySvr.responseCode;
//  printf("responseCode(%d) \n", responseCode );

    // only if a valid response is received by the server, try to deserialize it
    if (SDK_SERVER_FAILURE == responseCode || SDK_SERVER_SUCCESS== responseCode) {
      // parse the registration response json from the Primary server
      json_deserializeRegistrationResponseMessage(clientHandle, SERVER_PRIMARY,
          clientHdle->registrationData.regResponsePrimarySvr.response,
          &(clientHdle->registrationData.regResponsePrimarySvr) );
      json_releaseReg_ResponseMessageData(clientHdle);
    }

  } else {
    iot_device_sdk_log(SDK_LOG_ERROR, "iot_dsdk_sendPrimaryRegistrationMessage_v1() clientHandle passed is NULL\n");
    return NULL;
  }

  iot_device_sdk_log(SDK_LOG_INFO, "iot_dsdk_sendPrimaryRegistrationMessage_v1() -- \n");
  return &(clientHdle->registrationData.regResponsePrimarySvr);
}


/**
 * @brief iot_dsdk_getDeviceRegistrationStatus
 * iot_dsdk_getDeviceRegistrationStatus ( ) - Function to get registration message response
 * @param[in]  : clientHandle
 * @return     : SDKRegistrationResponse* on Success
 *               NULL on Error
*/
const SDKRegistrationResponse* iot_dsdk_getDeviceRegistrationStatus(IotDeviceSdkClient clientHandle)
{
  SDKDeviceInternal *clientHdle = NULL;
  iot_device_sdk_log(SDK_LOG_INFO, "iot_dsdk_getDeviceRegistrationStatus() ++ \n");

  if(!clientHandle) {
    iot_device_sdk_log(SDK_LOG_ERROR, "iot_dsdk_getDeviceRegistrationStatus() clientHandle passed is NULL\n");
    return NULL;
  }

  clientHdle=(SDKDeviceInternal*)clientHandle;
  iot_device_sdk_log(SDK_LOG_INFO, "iot_dsdk_getDeviceRegistrationStatus() -- \n");
  return &(clientHdle->registrationData.regResponsePrimarySvr);
}


/* Need to add code to check if te connector details have been udpated and
 * initiate a re-connect
*/
int update_Device_Connector_Details(SDKDeviceInternal* clientHdle)
{
  if(clientHdle){
    //Update - Reg response does not contain ConnectorType info any more
    // just update username and password
    SAFE_FREE(clientHdle->registrationData.device_username)
    if(clientHdle->registrationData.regResponseCPODSvr.cred_name)
      clientHdle->registrationData.device_username = strdup(clientHdle->registrationData.regResponseCPODSvr.cred_name);

    SAFE_FREE(clientHdle->registrationData.device_password)
    if(clientHdle->registrationData.regResponseCPODSvr.cred_secret)
      clientHdle->registrationData.device_password = strdup(clientHdle->registrationData.regResponseCPODSvr.cred_secret);
  }
  return 0;
}
/**
 * @brief iot_dsdk_sendRegistrationMessage_v1
 * iot_dsdk_sendRegistrationMessage_v1 ( ) Function to send registration message
 * @param[in]  : Handle to DeviceSdkClient
 * @return     :  0 on Success.
 *               -1 on Error.
*/
const SDKRegistrationResponse* iot_dsdk_sendRegistrationMessage_v1(IotDeviceSdkClient clientHandle, int *error_code)
{
  SDKDeviceInternal *clientHdle = NULL;
  int ret = 0;

  iot_device_sdk_log(SDK_LOG_INFO, "iot_dsdk_sendRegistrationMessage_v1() ++ \n");

  if(clientHandle) {
    clientHdle=(SDKDeviceInternal*)clientHandle;

    //Check if device is already registered (case device info loaded from storage BD)
    if( clientHdle->registrationData.regResponseCPODSvr.registered &&
        clientHdle->registrationData.regResponseCPODSvr.claimed ) {
      iot_device_sdk_log(SDK_LOG_INFO, "iot_dsdk_sendRegistrationMessage_v1() device Already Registered \n");
      return &(clientHdle->registrationData.regResponseCPODSvr);
    }

    if (NULL == clientHdle->registrationData.regUrlCPODSvr) {
      iot_device_sdk_log(SDK_LOG_ERROR, "iot_dsdk_sendRegistrationMessage_v1() CPOD Url NULL\n");
      *error_code = IOTERR_CPOD_URL_NULL;
      return NULL;
    }

    // load the registration message json tree
    if(iot_BuildRegistrationMessage(clientHandle, SERVER_CPOD) < 0) {
      iot_device_sdk_log(SDK_LOG_ERROR, "iot_dsdk_sendCPODRegistrationMessage() iot_BuildRegistrationMessage Failed \n");
      return NULL;
    }
    //Memset the reg response structure
    //SAFE_FREE(clientHdle->registrationData.regResponseCPODSvr->response)
    if (clientHdle->registrationData.regResponseCPODSvr.response){
      free(clientHdle->registrationData.regResponseCPODSvr.response);
    }
    memset((char*)(&(clientHdle->registrationData.regResponseCPODSvr)),0, sizeof(SDKRegistrationResponse));

    // send the registration message to the CPOD URL
    ret = send_RegistrationMessage_curl(clientHdle,
        clientHdle->registrationData.regUrlCPODSvr,
        clientHdle->registrationData.regMsgCPODSvrJson,
        &(clientHdle->registrationData.regResponseCPODSvr));

    // teardown the registration message
    iot_DestroyRegistrationMessage(clientHandle);

    // If curl failed to send message (bad url.. etc), return NULL
    if (-1 == ret) {
      *error_code = IOTERR_CPOD_CURL_ERR;
      return NULL;
    }

    int responseCode = clientHdle->registrationData.regResponseCPODSvr.responseCode;

    iot_device_sdk_log(SDK_LOG_INFO, "iot_dsdk_sendRegistrationMessage_v1() response code = %d\n", responseCode);

    // only if a valid response is received by the server, try to deserialize it
    if (SDK_SERVER_FAILURE == responseCode || SDK_SERVER_SUCCESS== responseCode) {
      //Clean up the previous JSON reg response
      //moving it down
      json_releaseReg_ResponseMessageData(clientHdle);

      // parse the registration response json from the CPOD server
      json_deserializeRegistrationResponseMessage(clientHandle, SERVER_CPOD,
          clientHdle->registrationData.regResponseCPODSvr.response,
          &(clientHdle->registrationData.regResponseCPODSvr) );

      //json_releaseReg_ResponseMessageData(clientHandle);

      /*
       * Save the device data on the first successful registration
       *
       * The response is saved into a device file under device_db directory.
       * Since the data is in json format, it should be loaded using
       * iot_device_load_json() API.
       * TODO 1: do better validation. Should parse string into JSON and
       * TODO 2: trim / add data in the response
       * make sure there is indeed real credential section
       */
      if (SDK_SERVER_SUCCESS== responseCode &&
	       clientHdle->registrationData.regResponseCPODSvr.response &&
	       strstr(clientHdle->registrationData.regResponseCPODSvr.response,
		     "credentials")) {
	      iot_device_save_data(clientHdle->deviceDetails.manufacturingId,
	         clientHdle->registrationData.regResponseCPODSvr.response,
	         strlen(clientHdle->registrationData.regResponseCPODSvr.response));
      }


      //store the connector details to appropriate connector struct.
      if (SDK_SERVER_SUCCESS == responseCode &&
	       clientHdle->registrationData.regResponseCPODSvr.registered &&
         clientHdle->registrationData.regResponseCPODSvr.claimed) {
          if( update_Device_Connector_Details(clientHdle) < 0 )
            iot_device_sdk_log(SDK_LOG_ERROR, "iot_dsdk_sendRegistrationMessage_v1() Updating device connector Details Failed\n");
      }
    }
  } else {
    *error_code = IOTERR_CPOD_CLIENT_HANDLE_NOT_SET;
    iot_device_sdk_log(SDK_LOG_ERROR, "iot_dsdk_sendRegistrationMessage_v1() clientHandle passed is NULL\n");
    return NULL;
  }
  iot_device_sdk_log(SDK_LOG_INFO, "iot_dsdk_sendRegistrationMessage_v1() -- \n");
  return &(clientHdle->registrationData.regResponseCPODSvr);
}


/**
 * @brief iot_dsdk_getDeviceCPODRegistrationStatus
 * iot_dsdk_getDeviceCPODRegistrationStatus ( ) - Function to get registration message response
 * @param[in]  : clientHandle
 * @return     : SDKRegistrationResponse* on Success
 *               NULL on Error
*/
const SDKRegistrationResponse* iot_dsdk_getDeviceCPODRegistrationStatus(IotDeviceSdkClient clientHandle)
{
  SDKDeviceInternal *clientHdle = NULL;
  iot_device_sdk_log(SDK_LOG_INFO, "iot_dsdk_getDeviceCPODRegistrationStatus() ++ \n");

  if(!clientHandle) {
    iot_device_sdk_log(SDK_LOG_ERROR, "iot_dsdk_getDeviceCPODRegistrationStatus() clientHandle passed is NULL\n");
    return NULL;
  }

  clientHdle=(SDKDeviceInternal*)clientHandle;
  iot_device_sdk_log(SDK_LOG_INFO, "iot_dsdk_getDeviceCPODRegistrationStatus() -- \n");
  return &(clientHdle->registrationData.regResponseCPODSvr);
}
