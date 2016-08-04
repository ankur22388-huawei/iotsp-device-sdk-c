/**************************************************************************
  copyright (C) Copyright 2015-2016 Cisco Systems Inc.; All Rights Reserved
  History:
    01/12/2015 - Created.
 ******************************************************************************/
 #include <stdio.h>
 #include <string.h>
 #include <assert.h>
 #include <iot_device_sdk.h>
 #include "_iot_device_sdk_init.h"
 #include "iot_device_sdk_storage.h"
 #include "iot_device_sdk_json.h"
 #include <jansson.h>

#define SDK_TOKEN_DELIMITER "."
#define DELIM SDK_TOKEN_DELIMITER // short form
#define SZ_DELIM  strlen(DELIM)


extern void set_sdk_config_value ( SDKDeviceInternal* clientP, char* tag, char *value);

static int clean_up_json(SDKDeviceInternal* clientHandle)
{
  if(clientHandle->json_root) {
    json_decref(clientHandle->json_root);
    clientHandle->json_root = NULL;
  }
  return 0;
}

char* get_json_PayLoad(SDKDeviceInternal* clientHandle)
{
  assert(clientHandle->json_root);
  return json_dumps(clientHandle->json_root,JSON_INDENT(1));
}


static const char *GetNextToken(const char *inputstr, char *token)
{
  char *tokenend = strstr(inputstr, DELIM);
  const char *outputptr = NULL;
  int numbytes = 0;
  if (tokenend) {
    numbytes = tokenend-inputstr;
    memcpy(token, inputstr, numbytes);
    token[numbytes]='\0';
    outputptr = inputstr + (numbytes + SZ_DELIM);
  } else {
    strcpy(token, inputstr);
  }
  return outputptr;
}


json_t *FindJsonNode(SDKDeviceInternal* clientHandle, const char* key)
{
  const char *keyjson;
  json_t *json_node=NULL, *json_curnt_node;
  void *iter;
  int i = 0;
  char token[512] = "";
  const char *nexttokenstart = key;

  assert(clientHandle->json_root);
  json_curnt_node = clientHandle->json_root;

  do {
    nexttokenstart = GetNextToken(nexttokenstart , token);
    //    token, nexttokenstart, nexttokenstart);

    iter = json_object_iter(json_curnt_node);
    while(iter){
      keyjson = json_object_iter_key(iter);
      if(!keyjson) {
        printf("keyjson is NULL");
        break;
      }
      json_node = json_object_iter_value(iter);
      if(!strcmp(keyjson,token)){
        json_curnt_node = json_node;
        break;
      } else {
        keyjson = NULL; json_node = NULL;
      }
      i++;
      iter = json_object_iter_next(json_curnt_node, iter);
    }
  }while(NULL != nexttokenstart);

  /* For debug
  if(json_node == NULL)
    printf("FindJsonNode() failed to find node = %s\n", key);
  else
    printf("FindJsonNode() found Json node = %s\n", key);
  */
  return json_node ;
}

/* key_obj - object node in the json data,
   key - key to be deleted on the object */
int del_json_object_node(SDKDeviceInternal* clientHandle, char* key_obj, char* key) {
  SDKDeviceInternal *clientHdle = NULL;
  json_t* jsonObject;

  assert(clientHandle);
  clientHdle=(SDKDeviceInternal*)clientHandle;
  assert(clientHdle->json_root);
  jsonObject = FindJsonNode(clientHdle, key_obj);
  if(jsonObject && key){
    json_object_del(jsonObject,(const char*)key);
  }
  return 0;
}

int set_json_string_value(SDKDeviceInternal* clientHandle, char* key, char* value) {
  SDKDeviceInternal *clientHdle = NULL;
  json_t* jsonString;

  assert(clientHandle);
  clientHdle=(SDKDeviceInternal*)clientHandle;
  assert(clientHdle->json_root);
  jsonString = FindJsonNode(clientHdle, key);

  if(jsonString){
    json_string_set(jsonString,value);
  }
  return 0;
}

int load_json_config_file(SDKDeviceInternal* clientHandle, char* jsonFile) {
  json_error_t error;
  const char *key = NULL, *string_value = NULL;
  json_t *value = NULL;
  void *iter = NULL;

  if(clientHandle){
    if(jsonFile) {
      if(clientHandle->json_root) {
        json_decref(clientHandle->json_root);
        iot_device_sdk_log(SDK_LOG_WARN,"load_json_config_file load_json_file cleaning previous JSON root\n" );
      }
      clientHandle->json_root = json_load_file(jsonFile, 0, &error);
      if (clientHandle->json_root) {
        iter = json_object_iter(clientHandle->json_root);
        while(iter){
          key = json_object_iter_key(iter);
          value = json_object_iter_value(iter);
          string_value = NULL;
          string_value = json_string_value(value);
          if(string_value && string_value[0])
            set_sdk_config_value(clientHandle, (char*)key, (char*)string_value);
          iter = json_object_iter_next(clientHandle->json_root, iter);
        }
      } else {
        iot_device_sdk_log(SDK_LOG_ERROR,"load_sdk_config failed, json error on line  %d: %s\n", error.line, error.text );
        return -1;
      }
    }
  }
  return 0;
}

int json_release_config_data(SDKDeviceInternal* clientHandle) {
  if(!clientHandle) {
    iot_device_sdk_log(SDK_LOG_ERROR, "json_release_config_data() Client handle NULL \n");
    return -1;
  } else {
    clean_up_json(clientHandle);
  }
  return 0;
}

/**
 * @brief json_loadPrimaryRegistrationMessage
 * json_loadPrimaryRegistrationMessage ( ) Function to serialize Primary server registration data into json
 * @param[in]  : Handle to DeviceSdkClient
 * @return     :  0 on Success.
 *               -1 on Error.
*/
int json_loadRegistrationMessage (SDKDeviceInternal* clientHandle, char* reg_msg)
{
  json_error_t error;

  SDKDeviceInternal *clientHdle = NULL;
  iot_device_sdk_log(SDK_LOG_INFO, "json_loadRegistrationMessage() ++ \n");
  if(!clientHandle) {
    iot_device_sdk_log(SDK_LOG_ERROR, "json_loadRegistrationMessage() Client handle NULL \n");
    return -1;
  }
  clientHdle=(SDKDeviceInternal*)clientHandle;
  if(reg_msg) {
    if(clientHdle->json_root) {
      json_decref(clientHdle->json_root);
      iot_device_sdk_log(SDK_LOG_WARN,"json_loadRegistrationMessage load_json_file cleaning previous JSON root\n" );
    }
    //Load into json format
    clientHdle->json_root = json_loads(reg_msg, 0, &error);

    if (clientHdle->json_root) {
      iot_device_sdk_log(SDK_LOG_INFO,"json_loadRegistrationMessage load default reg msg Success \n" );
    } else {
      iot_device_sdk_log(SDK_LOG_ERROR,"json_loadRegistrationMessage load default reg msg failed, json error on line  %d: %s\n", error.line, error.text );
      return -1;
    }
  }

  iot_device_sdk_log(SDK_LOG_INFO, "json_loadRegistrationMessage() -- \n");
  return 0;
}


/**
 * @brief json_releaseRegistrationMessageData
 * json_releaseRegistrationMessageData ( ) Function to release registration response data
 * @param[in]  : Handle to DeviceSdkClient
 * @return     :  0 on Success.
 *               -1 on Error.
*/
int json_releaseRegistrationMessageData(SDKDeviceInternal* clientHdle){
  iot_device_sdk_log(SDK_LOG_INFO, "json_releaseRegistrationMessageData() ++\n");

  if(!clientHdle) {
    iot_device_sdk_log(SDK_LOG_ERROR, "json_releaseRegistrationMessageData() Client handle NULL \n");
    return -1;
  } else {
    clean_up_json(clientHdle);
  }
  iot_device_sdk_log(SDK_LOG_INFO, "json_releaseRegistrationMessageData() -- \n");
  return 0;
}

static int ParseJsonThingNode(SDKDeviceInternal *clientHdle /*, json_t *json_service */ )
{
  json_t* jsonNode;

  iot_device_sdk_log(SDK_LOG_INFO, "ParseJsonThingNode  ++ \n");

/*
  jsonNode = FindJsonNode(clientHdle, STR_THING_MAKE);
  if(jsonNode) {
    SAFE_FREE(clientHdle->deviceDetails.deviceMake)
    clientHdle->deviceDetails.deviceMake = strdup((char*)json_string_value(jsonNode));
    iot_device_sdk_log(SDK_LOG_INFO, "ParseJsonThingNode() thing.make  = %s\n", clientHdle->deviceDetails.deviceMake);
  } else {
    iot_device_sdk_log(SDK_LOG_ERROR, "ParseJsonThingNode() thing.make node not found - device json file may be corrupted - Check storage DB \n");
  }

  jsonNode = FindJsonNode(clientHdle, STR_THING_MODEL);
  if(jsonNode) {
    SAFE_FREE(clientHdle->deviceDetails.deviceModel)
    clientHdle->deviceDetails.deviceModel = strdup((char*)json_string_value(jsonNode));
    iot_device_sdk_log(SDK_LOG_INFO, "ParseJsonThingNode() thing.model  = %s\n", clientHdle->deviceDetails.deviceModel);
  } else {
    iot_device_sdk_log(SDK_LOG_ERROR, "ParseJsonThingNode() thing.model node not found - device json file may be corrupted - Check storage DB \n");
  }

  jsonNode = FindJsonNode(clientHdle, STR_THING_FW_VER);
  if(jsonNode) {
    SAFE_FREE(clientHdle->deviceDetails.deviceFirmwareVer)
    clientHdle->deviceDetails.deviceFirmwareVer = strdup((char*)json_string_value(jsonNode));
    iot_device_sdk_log(SDK_LOG_INFO, "ParseJsonThingNode() thing.firmwareVersion  = %s\n", clientHdle->deviceDetails.deviceFirmwareVer);
  } else {
    iot_device_sdk_log(SDK_LOG_ERROR, "ParseJsonThingNode() thing.firmwareVersion node not found - device json file may be corrupted - Check storage DB \n");
  }

  jsonNode = FindJsonNode(clientHdle, STR_THING_HW_VER);
  if(jsonNode) {
    SAFE_FREE(clientHdle->deviceDetails.hardwareVer)
    clientHdle->deviceDetails.hardwareVer = strdup((char*)json_string_value(jsonNode));
    iot_device_sdk_log(SDK_LOG_INFO, "ParseJsonThingNode() thing.hardwareVersion  = %s\n", clientHdle->deviceDetails.hardwareVer);
  } else {
    iot_device_sdk_log(SDK_LOG_ERROR, "ParseJsonThingNode() thing.hardwareVersion node not found - device json file may be corrupted - Check storage DB \n");
  }

  jsonNode = FindJsonNode(clientHdle, STR_THING_UI_MAC_ADDR);
  if(jsonNode) {
    SAFE_FREE(clientHdle->deviceDetails.macAddress)
    clientHdle->deviceDetails.macAddress = strdup((char*)json_string_value(jsonNode));
    iot_device_sdk_log(SDK_LOG_INFO, "ParseJsonThingNode() thing.uniqueIdentifiers.macAddress  = %s\n", clientHdle->deviceDetails.macAddress);
  } else {
    iot_device_sdk_log(SDK_LOG_ERROR, "ParseJsonThingNode() thing.uniqueIdentifiers.macAddress node not found - device json file may be corrupted - Check storage DB \n");
  }

  jsonNode = FindJsonNode(clientHdle, STR_THING_UI_SERIAL_NUM);
  if(jsonNode) {
    SAFE_FREE(clientHdle->deviceDetails.deviceSerialNum)
    clientHdle->deviceDetails.deviceSerialNum = strdup((char*)json_string_value(jsonNode));
    iot_device_sdk_log(SDK_LOG_INFO, "ParseJsonThingNode() thing.uniqueIdentifiers.serialNumber  = %s\n", clientHdle->deviceDetails.deviceSerialNum);
  } else {
    iot_device_sdk_log(SDK_LOG_ERROR, "ParseJsonThingNode() thing.uniqueIdentifiers.serialNumber node not found - device json file may be corrupted - Check storage DB \n");
  }
*/
  jsonNode = FindJsonNode(clientHdle, STR_THING_UI_DEVICE_UID);
  if(jsonNode) {
    SAFE_FREE(clientHdle->deviceDetails.manufacturingId)
    clientHdle->deviceDetails.manufacturingId = strdup((char*)json_string_value(jsonNode));
    iot_device_sdk_log(SDK_LOG_INFO, "ParseJsonThingNode() thing.uniqueIdentifiers.uuid  = %s\n", clientHdle->deviceDetails.manufacturingId);
  } else {
    iot_device_sdk_log(SDK_LOG_ERROR, "ParseJsonThingNode() thing.uniqueIdentifiers.uuid node not found - device json file may be corrupted - Check storage DB \n");
  }

  return 0;
}

static int ParseJsonServiceNode(SDKDeviceInternal *clientHdle,/* json_t *json_service, const char *jsondata,*/
           int server_type, SDKRegistrationResponse *regResponse)
{
  json_t* jsonNode;

  iot_device_sdk_log(SDK_LOG_INFO, "ParseJsonServiceNode server_type(%d) ++ \n", server_type);
  jsonNode = FindJsonNode(clientHdle, STR_SER_CLAIMED);
  if(jsonNode) {
    regResponse->claimed = json_boolean_value(jsonNode);
  } else {
    iot_device_sdk_log(SDK_LOG_ERROR, "ParseJsonServiceNode() service.claimed node not found - check reg response message\n");
  }

  jsonNode = FindJsonNode(clientHdle, STR_SER_REGISTERED);
  if(jsonNode) {
    regResponse->registered = json_boolean_value(jsonNode);
  } else {
    iot_device_sdk_log(SDK_LOG_ERROR, "ParseJsonServiceNode() service.registered node not found - check reg response message \n");
  }

  if(!regResponse->claimed) iot_device_sdk_log(SDK_LOG_INFO, "ParseJsonServiceNode() DEVICE NOT YET CLAIMED !!\n");
    else  iot_device_sdk_log(SDK_LOG_INFO, "ParseJsonServiceNode() DEVICE CLAIMED SUCCESSFULLY !!\n");
  if(!regResponse->registered) iot_device_sdk_log(SDK_LOG_INFO, "ParseJsonServiceNode() DEVICE NOT YET REGISTERED !!\n");
    else iot_device_sdk_log(SDK_LOG_INFO, "ParseJsonServiceNode() DEVICE REGISTERED SUCCESSFULLY !!\n");

  jsonNode = FindJsonNode(clientHdle, STR_SER_REQ_PROTO);
  if(jsonNode) {
    SAFE_FREE(regResponse->protocol)
    regResponse->protocol = strdup((char*)json_string_value(jsonNode));
    iot_device_sdk_log(SDK_LOG_INFO, "ParseJsonServiceNode() service.request.protocol = %s\n", regResponse->protocol);
  } else {
    iot_device_sdk_log(SDK_LOG_ERROR, "ParseJsonServiceNode() service.request.protocol node not found - check reg response message \n");
  }

  jsonNode = FindJsonNode(clientHdle, STR_SER_REG_INTVL);
  if(jsonNode) {
    SAFE_FREE(regResponse->interval)
    regResponse->interval = strdup((char*)json_string_value(jsonNode));
    iot_device_sdk_log(SDK_LOG_INFO, "ParseJsonServiceNode() service.request.interval = %s\n", regResponse->interval);
  } else {
    iot_device_sdk_log(SDK_LOG_ERROR, "ParseJsonServiceNode() service.request.interval node not found - check reg response message \n");
  }

  if(regResponse->claimed && regResponse->registered) {
    /* parse the device credentials and the device uid */
    jsonNode = FindJsonNode(clientHdle, STR_SER_CRED_NAME);
    if(jsonNode) {
      SAFE_FREE(regResponse->cred_name)
      regResponse->cred_name = strdup((char*)json_string_value(jsonNode));
      iot_device_sdk_log(SDK_LOG_INFO, "ParseJsonCredNode() response.credentials.name Received in response [%s]\n", regResponse->cred_name);
    } else {
      iot_device_sdk_log(SDK_LOG_ERROR, "ParseJsonCredNode() response.credentials.name tag not found - check reg response message \n");
    }

    jsonNode = FindJsonNode(clientHdle, STR_SER_CRED_SECRET);
    if(jsonNode) {
      SAFE_FREE(regResponse->cred_secret)
      regResponse->cred_secret = strdup((char*)json_string_value(jsonNode));
      iot_device_sdk_log(SDK_LOG_INFO, "ParseJsonCredNode() response.credentials.secret Received in response [%s]\n", regResponse->cred_secret);
    } else {
      iot_device_sdk_log(SDK_LOG_ERROR, "ParseJsonCredNode() response.credentials.secret tag not found - check reg response message \n");
    }

    jsonNode = FindJsonNode(clientHdle, STR_SER_THING_UID);
    if(jsonNode) {
      SAFE_FREE(regResponse->thingUid)
      regResponse->thingUid = strdup((char*)json_string_value(jsonNode));
      iot_device_sdk_log(SDK_LOG_INFO, "ParseJsonCredNode() response.thingUid Received in response [%s]\n", regResponse->thingUid);
    } else {
      iot_device_sdk_log(SDK_LOG_ERROR, "ParseJsonCredNode() response.thingUid tag not found - check reg response message \n");
    }

  }
  return 0;
}


static int ParseJsonErrorNode(SDKDeviceInternal *clientHdle, /*json_t *json_service,*/
    const char *jsondata, int server_type, SDKRegistrationResponse *regResponse)
{
  json_t* jsonNode;

  iot_device_sdk_log(SDK_LOG_INFO, "ParseJsonErrorNode server_type(%d) ++ \n", server_type);
  /* Handle error response case */
  if (SDK_SERVER_FAILURE == regResponse->responseCode) {
    jsonNode = FindJsonNode(clientHdle, STR_ERROR);
    if(NULL == jsonNode ) {
      iot_device_sdk_log(SDK_LOG_ERROR, "ParseJsonErrorNode() could not find \"errorCode \" field."
          "Check Json string for correctness. ----\n%s\n----\n", jsondata);
      return -1;
    } else {
      //get the error code and the error string
      iot_device_sdk_log(SDK_LOG_ERROR, "ParseJsonErrorNode() reg errorCode = %s \n",json_string_value(jsonNode));
      jsonNode = FindJsonNode(clientHdle, STR_ERROR_MSG);
      if(jsonNode)
        iot_device_sdk_log(SDK_LOG_ERROR, "ParseJsonErrorNode() reg error message = %s \n",json_string_value(jsonNode));
    }
  }
  return 0;
}


/**
 * @brief json_releaseRegistrationMessageData
 * json_releaseRegistrationMessageData ( ) Function to release registration response data
 * @param[in]  : Handle to DeviceSdkClient
 * @return     :  0 on Success.
 *               -1 on Error.
*/
int json_releaseReg_ResponseMessageData(SDKDeviceInternal* clientHdle){
  iot_device_sdk_log(SDK_LOG_INFO, "json_releaseReg_ResponseMessageData() ++\n");

  if(!clientHdle) {
    iot_device_sdk_log(SDK_LOG_ERROR, "json_releaseReg_ResponseMessageData() Client handle NULL \n");
    return -1;
  } else {
    clean_up_json(clientHdle);
  }
  iot_device_sdk_log(SDK_LOG_INFO, "json_releaseReg_ResponseMessageData() -- \n");
  return 0;
}

/**
 * @brief deserializeRegistrationMessage_json
 * deserializeRegistrationMessage_json ( ) Function to deserialize json registration response data
 * @param[in]  : Handle to DeviceSdkClient
 * @return     :  0 on Success.
 *               -1 on Error.
*/
int json_deserializeRegistrationResponseMessage (IotDeviceSdkClient clientHandle, int server_type,
    const char *jsondata, SDKRegistrationResponse *regResponse){
  SDKDeviceInternal *clientHdle = NULL;
  json_error_t error;
  json_t* jsonNode;
  char* jsonDumpPtr=NULL;
  int ret = 0;

  iot_device_sdk_log(SDK_LOG_INFO, "json_deserializeRegistrationMessage() data +++ \n%s\n +++\n", jsondata);

  if(!clientHandle) {
    iot_device_sdk_log(SDK_LOG_ERROR, "json_deserializeRegistrationMessage() Client handle NULL \n");
    return -1;
  }
  clientHdle=(SDKDeviceInternal*)clientHandle;

  if(jsondata && regResponse) {
    if(clientHdle->json_root) {
      json_decref(clientHdle->json_root);
      iot_device_sdk_log(SDK_LOG_WARN,"json_deserializeRegistrationMessage load_json_file cleaning previous JSON root\n" );
    }
    //Load into json format
    clientHdle->json_root = json_loads(jsondata, 0, &error);
    if (clientHdle->json_root) {
      iot_device_sdk_log(SDK_LOG_INFO,"json_deserializeRegistrationMessage load reg response json Success \n" );
      jsonDumpPtr = json_dumps(clientHdle->json_root,JSON_INDENT(1));
      iot_device_sdk_log(SDK_LOG_INFO,"reg response  = %s \n",jsonDumpPtr);
      free(jsonDumpPtr);
      jsonNode = FindJsonNode(clientHdle, STR_SERVICE);

      /* Check if the response payload has the "service" object */
      if(NULL == jsonNode) {
        iot_device_sdk_log(SDK_LOG_INFO, "json_deserializeRegistrationMessage() could not find \"service\" field."
                            "Parsing Error Message. ----\n%s\n----\n", jsondata);
        ParseJsonErrorNode(clientHdle, /*clientHdle->json_root,*/ jsondata, server_type, regResponse);
        return -1;
      } else {
        //Parse the relevant inforamtion from the response JSON
        ParseJsonServiceNode(clientHdle, /*jsonNode, jsondata,*/ server_type, regResponse);
      }
    } else {
      iot_device_sdk_log(SDK_LOG_ERROR,"json_deserializeRegistrationMessage reg response json load failed, json error on line  %d: %s\n", error.line, error.text );
      return -1;
    }
  }

  iot_device_sdk_log(SDK_LOG_INFO, "json_deserializeRegistrationMessage() -- \n");
  return ret;
}

int json_parse_device_json_data (IotDeviceSdkClient *clientHandle/*, json_t *root*/){

  SDKDeviceInternal *clientHdle = NULL;
  char* jsonDumpPtr=NULL;

  iot_device_sdk_log(SDK_LOG_INFO,"json_parse_device_json_data ++ \n" );

  if(!clientHandle) {
    iot_device_sdk_log(SDK_LOG_ERROR, "json_parse_device_json_data() Client handle NULL \n");
    return -1;
  }
  clientHdle=(SDKDeviceInternal*)clientHandle;


  jsonDumpPtr = json_dumps(clientHdle->json_root,JSON_INDENT(1));
  if(jsonDumpPtr){
    iot_device_sdk_log(SDK_LOG_INFO,"json_parse_device_json_data device json data  = %s \n",jsonDumpPtr);
    free(jsonDumpPtr);
  }

  // Parse fucntion takes care of finding THING json Node
  ParseJsonThingNode( clientHdle /*, &(clientHdle->registrationData.regResponseCPODSvr) */);

  // Parse function takes care of finding SERVICE json Node
  ParseJsonServiceNode(clientHdle, /*jsonNode, NULL,*/ 0/*server_type*/, &(clientHdle->registrationData.regResponseCPODSvr) );

  iot_device_sdk_log(SDK_LOG_INFO,"json_parse_device_json_data -- \n" );

  return 0;
}


/*
  int refresh_token_expires_in;
  int access_token_expires_in;
*/

/**
 * @brief ParseIAMResponseNode
 * ParseIAMResponseNode ( ) Function to deserialize json IAM registration response data
 * @param[in]  : Handle to DeviceSdkClient
 * @return     :  0 on Success.
 *               -1 on Error.
*/
static int ParseIAMResponseNode(SDKDeviceInternal *clientHandle,
         AccessTokenDetails *accessTokenDetails)
{
  json_t* jsonNode;

  iot_device_sdk_log(SDK_LOG_INFO,"ParseIAMResponseNode ++ \n" );
  if(!clientHandle) {
    iot_device_sdk_log(SDK_LOG_ERROR, "ParseIAMResponseNode() Client handle NULL \n");
    return -1;
  }

  jsonNode = FindJsonNode(clientHandle, STR_ACCESS_TOKEN);
  if(jsonNode) {
    SAFE_FREE(accessTokenDetails->access_token)
    accessTokenDetails->access_token = strdup((char*)json_string_value(jsonNode));
    iot_device_sdk_log(SDK_LOG_INFO, "ParseIAMResponseNode() access token [%s]\n", accessTokenDetails->access_token);
  } else {
    iot_device_sdk_log(SDK_LOG_ERROR, "ParseIAMResponseNode() access token NOT found - check AccessToken server response \n");
  }

  jsonNode = FindJsonNode(clientHandle, STR_REFRESH_TOKEN);
  if(jsonNode) {
    SAFE_FREE(accessTokenDetails->refresh_token)
    accessTokenDetails->refresh_token = strdup((char*)json_string_value(jsonNode));
    iot_device_sdk_log(SDK_LOG_INFO, "ParseIAMResponseNode() refresh token [%s]\n", accessTokenDetails->refresh_token);
  } else {
    iot_device_sdk_log(SDK_LOG_ERROR, "ParseIAMResponseNode() access token NOT found - check AccessToken server response \n");
  }

  //Both access_token and refresh_token use the same "expires_in" value
  jsonNode = FindJsonNode(clientHandle, STR_ACCESS_TOKEN_EXPIRES_IN);
  if(jsonNode) {
    accessTokenDetails->access_token_expires_in = json_integer_value(jsonNode);
    accessTokenDetails->refresh_token_expires_in = json_integer_value(jsonNode);
    iot_device_sdk_log(SDK_LOG_INFO, "ParseIAMResponseNode() expires_in [%d]\n", accessTokenDetails->access_token_expires_in);
  } else {
    iot_device_sdk_log(SDK_LOG_ERROR, "ParseIAMResponseNode() expires_in NOT found - check AccessToken server response \n");
  }

  jsonNode = FindJsonNode(clientHandle, STR_ACCESS_TOKEN_SCOPE);
  if(jsonNode) {
    SAFE_FREE(accessTokenDetails->scope)
    accessTokenDetails->scope = strdup((char*)json_string_value(jsonNode));
    iot_device_sdk_log(SDK_LOG_INFO, "ParseIAMResponseNode() token scope [%s]\n", accessTokenDetails->scope);
  } else {
    iot_device_sdk_log(SDK_LOG_ERROR, "ParseIAMResponseNode() token scope NOT found - check AccessToken server response \n");
  }

  jsonNode = FindJsonNode(clientHandle, STR_ACCESS_TOKEN_TYPE);
  if(jsonNode) {
    SAFE_FREE(accessTokenDetails->token_type)
    accessTokenDetails->token_type = strdup((char*)json_string_value(jsonNode));
    iot_device_sdk_log(SDK_LOG_INFO, "ParseIAMResponseNode() token type [%s]\n", accessTokenDetails->refresh_token);
  } else {
    iot_device_sdk_log(SDK_LOG_ERROR, "ParseIAMResponseNode() token type NOT found - check AccessToken server response \n");
  }

  iot_device_sdk_log(SDK_LOG_INFO, "ParseIAMResponseNode --\n");
  return 0;
}

/**
 * @brief deserializeAccessTokenResponse_json
 * deserializeAccessTokenResponse_json ( ) Function to deserialize json access token response data
 * @param[in]  : Handle to DeviceSdkClient
 * @return     :  0 on Success.
 *               -1 on Error.
*/
int deserializeAccessTokenResponse_json (IotDeviceSdkClient clientHandle,
    const char *jsondata, AccessTokenDetails *accessTokenDetails)
{
  json_error_t error;
  SDKDeviceInternal *clientHdle = NULL;
  char *jsonDumpPtr=NULL;
  int ret = 0;

  iot_device_sdk_log(SDK_LOG_INFO, "deserializeAccessTokenResponse_json() ++\n");
  if(!clientHandle) {
    iot_device_sdk_log(SDK_LOG_ERROR, "deserializeAccessTokenResponse_json() Client handle NULL \n");
    return -1;
  }
  clientHdle=(SDKDeviceInternal*)clientHandle;

  if(jsondata && accessTokenDetails) {
    if(clientHdle->json_root) {
      json_decref(clientHdle->json_root);
      iot_device_sdk_log(SDK_LOG_WARN,"deserializeAccessTokenResponse_json cleaning previous JSON root\n" );
    }
    //Load into json format
    clientHdle->json_root = json_loads(jsondata, 0, &error);
    if (clientHdle->json_root) {
      iot_device_sdk_log(SDK_LOG_INFO,"json_deserializeRegistrationMessage load reg response json Success \n" );
      //json_dump - pointer free added
      jsonDumpPtr = json_dumps(clientHdle->json_root,JSON_INDENT(1));
      iot_device_sdk_log(SDK_LOG_INFO,"Access Token response  = %s \n",jsonDumpPtr);
      free(jsonDumpPtr);

      ret = ParseIAMResponseNode(clientHdle, accessTokenDetails);
    }
  }
  iot_device_sdk_log(SDK_LOG_INFO, "deserializeAccessTokenResponse_json() -- \n");
  return ret;
}

/**
 * @brief releaseAccessTokenDetails
 * releaseAccessTokenDetails ( ) Function to release access token details
 * @param[in]  : pointer to AccessTokenDetails
 * @return     :
*/
void releaseAccessTokenDetails(AccessTokenDetails *accessTokenDetails)
{
  iot_device_sdk_log(SDK_LOG_INFO, "releaseAccessTokenDetails() ++\n");
  if(!accessTokenDetails) {
    iot_device_sdk_log(SDK_LOG_ERROR, "releaseAccessTokenDetails() AccessTokenDetails NULL \n");
  }
  else {
    SAFE_FREE(accessTokenDetails->response)
    SAFE_FREE(accessTokenDetails->scope)
    SAFE_FREE(accessTokenDetails->token_type)
    SAFE_FREE(accessTokenDetails->refresh_token)
    SAFE_FREE(accessTokenDetails->access_token)
  }
  iot_device_sdk_log(SDK_LOG_INFO, "releaseAccessTokenDetails() --\n");
}

/**
 * @brief deserializeDataPostResponse_json
 * deserializeDataPostResponse_json ( ) Function to deserialize json data post data
 * @param[in]  : Handle to DeviceSdkClient
 * @return     :  0 on Success.
 *               -1 on Error.
*/
int deserializeDataPostResponse_json (IotDeviceSdkClient clientHandle,
    const char *jsondata, DataPostResponse *dataPostResponse)
{
  json_error_t error;
  SDKDeviceInternal *clientHdle = NULL;
  char *jsonDumpPtr=NULL;
  int ret = 0;

  iot_device_sdk_log(SDK_LOG_INFO, "deserializeDataPostResponse_json() ++\n");
  if(!clientHandle) {
    iot_device_sdk_log(SDK_LOG_ERROR, "deserializeDataPostResponse_json() Client handle NULL \n");
    return -1;
  }
  clientHdle=(SDKDeviceInternal*)clientHandle;

  if(jsondata && dataPostResponse) {
    if(clientHdle->json_root) {
      json_decref(clientHdle->json_root);
      iot_device_sdk_log(SDK_LOG_WARN,"deserializeDataPostResponse_json cleaning previous JSON root\n" );
    }
    //Load into json format
    clientHdle->json_root = json_loads(jsondata, 0, &error);
    if (clientHdle->json_root) {
      iot_device_sdk_log(SDK_LOG_INFO,"deserializeDataPostResponse_json load reg response json Success \n" );
      jsonDumpPtr = json_dumps(clientHdle->json_root,JSON_INDENT(1));
      iot_device_sdk_log(SDK_LOG_INFO,"deserializeDataPostResponse_json Data Post response  = %s \n",jsonDumpPtr);
      free(jsonDumpPtr);

    }
  }
  iot_device_sdk_log(SDK_LOG_INFO, "deserializeDataPostResponse_json() -- \n");
  return ret;
}

/**
 * @brief releaseAccessTokenDetails
 * releaseAccessTokenDetails ( ) Function to release access token details
 * @param[in]  : pointer to AccessTokenDetails
 * @return     :
*/
void releaseDataPostResponse (DataPostResponse *dataPostResponse)
{
  iot_device_sdk_log(SDK_LOG_INFO, "releaseDataPostResponse() ++\n");
  if(!dataPostResponse) {
    iot_device_sdk_log(SDK_LOG_ERROR, "releaseDataPostResponse() dataPostResponse NULL \n");
  }
  else {
    SAFE_FREE(dataPostResponse->response)
    dataPostResponse->responseCode = 0;
  }
  iot_device_sdk_log(SDK_LOG_INFO, "releaseDataPostResponse() --\n");
}
