/**************************************************************************
  copyright (C) Copyright 2015-2016 Cisco Systems Inc.; All Rights Reserved

  file _iot_device_sdk_init.h
  (Internal File)

  History:
    01/12/2015 - Created.
 ******************************************************************************/

#ifndef CISCO_IOT_DEVICE_SDK_INIT_INTERNAL_H
#define CISCO_IOT_DEVICE_SDK_INIT_INTERNAL_H

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <iot_device_sdk.h>
#include <jansson.h>
#include "iot_device_sdk_defaults.h"
#include "iot_device_sdk_mqtt_client.h"


#define SAFE_FREE(x) if ((x) != NULL) { free(x); x=NULL; }

typedef struct _SDKRegistrationData {
  int  https_flag;            // 1 - https , 0 -http
  char *server_url;           // base server ip address

  char *regUrlPrimarySvr;     // primary server registration URL
  char *regMsgPrimarySvrJson; // registration message in json format

  char *regUrlCPODSvr;        // CPOD server registration URL
  char *regMsgCPODSvrJson;    // CPOD server registration message in json format

  char *regUrlIAMSvr;         // IAM server registration URL

  char *device_username;      // device username
  char *device_password;      // device password

  char *observationURL;       // server url to post device data

  //Token timer
  struct timespec startTokenTime;  //Used to check if token has expired

  SDKRegistrationResponse regResponsePrimarySvr;
  SDKRegistrationResponse regResponseCPODSvr;
  AccessTokenDetails accessTokenDetails;
  DataPostResponse dataPostResponse;
}SDKRegistrationData;

/* Struct to hold the fields from response headers of a curl command
 * Internal for now: TBD
 * */
typedef struct _serverResponseHdrsStruct {
  long curl_res_code;
  char *redirect_url;
}serverResponseHdrsStruct;


typedef struct _SDKDeviceInternal {
  DeviceDetails deviceDetails;
  char *dataPayloadFile;
  char *deviceDataSerializationType;
  char *certAuthorityBundleFile;  //Certificate aganist which
                                  // incoming server cert needs to be verified

  char *regURI; //Registration Uri - received in the reg response

  char *device_profile;
  char *access_token;
  char *deviceCert;
  char *devicePrivKey;
  char *deviceKid;

  char *dataToSend;
  SDKRegistrationData registrationData;
  Mqtt_Client mqttClient;

  json_t *json_root;

}SDKDeviceInternal;

#endif // CISCO_IOT_DEVICE_SDK_INIT_INTERNAL_H
