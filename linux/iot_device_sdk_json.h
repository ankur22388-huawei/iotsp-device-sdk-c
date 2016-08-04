/**************************************************************************
  copyright (C) Copyright 2015-2016 Cisco Systems Inc.; All Rights Reserved
  file iot_device_sdk_json.h
  (Internal File)
  History:
    01/14/2016 - Created.
 ******************************************************************************/

#ifndef _CISCO_IOT_DEVICE_SDK_JSON_INTERNAL_H
#define _CISCO_IOT_DEVICE_SDK_JSON_INTERNAL_H

#include "_iot_device_sdk_init.h"

/* strings used in the json message creation/parsing. */
#define STR_ERROR                    "errorCode"
#define STR_ERROR_MSG                "message"
#define STR_THING                    "thing"
#define STR_THING_MAKE               "thing.make"
#define STR_THING_MODEL              "thing.model"
#define STR_THING_FW_VER             "thing.firmwareVersion"
#define STR_THING_HW_VER             "thing.hardwareVersion"
#define ST_THING_UI                  "thing.uniqueIdentifiers"
#define STR_THING_UI_MAC_ADDR        "thing.uniqueIdentifiers.macAddress"
#define STR_THING_UI_SERIAL_NUM      "thing.uniqueIdentifiers.serialNumber"
#define STR_THING_UI_DEVICE_UID      "thing.uniqueIdentifiers.manufacturingId"

#define STR_SERVICE                  "service"
#define STR_SER_CLAIMED              "service.claimed"
#define STR_SER_REGISTERED           "service.registered"
#define STR_SER_REQ_PROTO            "service.request.protocol"
#define STR_SER_REG_INTVL            "service.request.interval"

#define STR_SER_CRED                 "service.credentials"
#define STR_SER_CRED_NAME            "service.credentials.name"
#define STR_SER_CRED_SECRET          "service.credentials.secret"


#define STR_ACCESS_TOKEN_SCOPE            "scope"
#define STR_ACCESS_TOKEN_TYPE             "token_type"
#define STR_ACCESS_TOKEN                  "access_token"
#define STR_ACCESS_TOKEN_EXPIRES_IN       "expires_in"
#define STR_REFRESH_TOKEN                 "refresh_token"
#define STR_REFRESH_TOKEN_EXPIRES_IN      "expires_in"


/* Observation Connection Details*/
#define STR_SER_OBSER_CONN_DET_CONNECTOR        "service.dataConnectorDetails.connector"
#define STR_SER_OBSER_CONN_DET_PROTOCOL         "service.dataConnectorDetails.protocol"
#define STR_SER_OBSER_CONN_DET_HOST             "service.dataConnectorDetails.host"
#define STR_SER_OBSER_CONN_DET_PORT             "service.dataConnectorDetails.port"
#define STR_SER_OBSER_CONN_DET_TOPIC            "service.dataConnectorDetails.upstreamTopics"

#define STR_SER_THING_UID            "service.thingUid"


char* get_json_PayLoad(SDKDeviceInternal* clientHandle);
json_t *FindJsonNode(SDKDeviceInternal* clientHandle, const char* key);
int del_json_object_node(SDKDeviceInternal* clientHandle, char* key_obj, char* key);
int set_json_string_value(SDKDeviceInternal* clientHandle, char* key, char* value);
int load_json_config_file(SDKDeviceInternal* clientHandle, char* jsonFile);
int json_release_config_data(SDKDeviceInternal* clientHandle);
int json_loadRegistrationMessage (SDKDeviceInternal* clientHandle, char* primary_reg_msg);
int json_releaseRegistrationMessageData(SDKDeviceInternal* clientHdle);
int json_deserializeRegistrationResponseMessage (IotDeviceSdkClient clientHandle, int server_type,
    const char *jsondata, SDKRegistrationResponse *regResponse);
int json_parse_device_json_data (IotDeviceSdkClient *clientHandle);
int json_releaseReg_ResponseMessageData(SDKDeviceInternal *clientHandle);

int deserializeAccessTokenResponse_json (IotDeviceSdkClient clientHandle,
    const char *jsondata, AccessTokenDetails *accessTokenDetails);
int deserializeDataPostResponse_json (IotDeviceSdkClient clientHandle,
    const char *jsondata, DataPostResponse *dataPostResponse);

#endif //_CISCO_IOT_DEVICE_SDK_JSON_INTERNAL_H
