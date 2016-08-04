/**************************************************************************
  copyright (C) Copyright 2015-2016 Cisco Systems Inc.; All Rights Reserved
  History:
    01/12/2016 - Created.
 ******************************************************************************/
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "_iot_device_sdk_init.h"
#include "iot_device_sdk_json.h"

/*from limits.h linux*/
#define FILE_PATH_MAX_LEN 4096

void set_device_data_connector_type(SDKDeviceInternal* clientP, char *value) {
  if(clientP){
    if(!strcmp(value, "mqtt")){
      clientP->deviceDetails.dataConnectorType = CONNECTOR_TYPE_MQTT;
    } else if (!strcmp(value, "https")) {
      clientP->deviceDetails.dataConnectorType = CONNECTOR_TYPE_HTTPS;
    } else if (!strcmp(value, "wamp")) {
      clientP->deviceDetails.dataConnectorType = CONNECTOR_TYPE_WAMP;
    } else {
      //Unknown device data connector type specified - default to mqtt
      clientP->deviceDetails.dataConnectorType = CONNECTOR_TYPE_MQTT;
    }
  }
  return;
}

int setup_various_urls(SDKDeviceInternal* clientP) {
  int ret = -1;

  if(clientP) {
    if(clientP->registrationData.https_flag) {
      //https - secure mode
      //Set the registration server url
      SAFE_FREE(clientP->registrationData.regUrlCPODSvr)
      ret = asprintf(&(clientP->registrationData.regUrlCPODSvr), "https://%s/v1/thing-services/things/actions/register"
                      ,clientP->registrationData.server_url);
      if(ret > 0)
        iot_device_sdk_log(SDK_LOG_INFO,"setup_various_urls reg_server url set to [%s] \n", clientP->registrationData.regUrlCPODSvr);

      //Set access token server url
      SAFE_FREE(clientP->registrationData.regUrlIAMSvr)
      ret = asprintf(&(clientP->registrationData.regUrlIAMSvr), "https://%s/v1/user-services/oauth2/token"
                      ,clientP->registrationData.server_url);
      if(ret > 0)
        iot_device_sdk_log(SDK_LOG_INFO,"setup_various_urls access_token_server_url set to [%s] \n", clientP->registrationData.regUrlIAMSvr);

      //Set mqtt url
      SAFE_FREE(clientP->mqttClient.mqtt_url)
      ret = asprintf(&(clientP->mqttClient.mqtt_url), "ssl://%s", clientP->registrationData.server_url);
      if(ret > 0)
        iot_device_sdk_log(SDK_LOG_INFO,"setup_various_urls mqtt url  set to [%s] \n", clientP->mqttClient.mqtt_url);

      //Set observation url
      SAFE_FREE(clientP->registrationData.observationURL)
      ret = asprintf(&(clientP->registrationData.observationURL), "https://%s/v1/observations/publish"
                      ,clientP->registrationData.server_url);
      if(ret > 0)
        iot_device_sdk_log(SDK_LOG_INFO,"setup_various_urls observation_url set to [%s] \n", clientP->registrationData.observationURL);
    } else {
      //http - non secure mode
      //Set the registration server url
      SAFE_FREE(clientP->registrationData.regUrlCPODSvr)
      ret = asprintf(&(clientP->registrationData.regUrlCPODSvr), "http://%s/v1/thing-services/things/actions/register"
                      ,clientP->registrationData.server_url);
      if(ret > 0)
        iot_device_sdk_log(SDK_LOG_INFO,"setup_various_urls reg_server url set to [%s] \n", clientP->registrationData.regUrlCPODSvr);

      //Set access token server url
      SAFE_FREE(clientP->registrationData.regUrlIAMSvr)
      ret = asprintf(&(clientP->registrationData.regUrlIAMSvr), "http://%s/v1/user-services/oauth2/token"
                      ,clientP->registrationData.server_url);
      if(ret > 0)
        iot_device_sdk_log(SDK_LOG_INFO,"setup_various_urls access_token_server_url set to [%s] \n", clientP->registrationData.regUrlIAMSvr);

      //Set mqtt url
      SAFE_FREE(clientP->mqttClient.mqtt_url)
      ret = asprintf(&(clientP->mqttClient.mqtt_url), "tcp://%s",clientP->registrationData.server_url);
      if(ret > 0)
        iot_device_sdk_log(SDK_LOG_INFO,"setup_various_urls mqtt url  set to [%s] \n", clientP->mqttClient.mqtt_url);

      //Set observation url
      SAFE_FREE(clientP->registrationData.observationURL)
      ret = asprintf(&(clientP->registrationData.observationURL), "http://%s/v1/observations/publish"
                      ,clientP->registrationData.server_url);
      if(ret > 0)
        iot_device_sdk_log(SDK_LOG_INFO,"setup_various_urls observation_url set to [%s] \n", clientP->registrationData.observationURL);
    }
  }

  return 0;
}

void set_sdk_config_value ( SDKDeviceInternal* clientP, char* tag, char *value) {
  if(!strcmp(tag, "skip_registration")) {
    iot_device_sdk_log(SDK_LOG_INFO,"set_sdk_config_value skip_registration set to value = %s \n", value);
    clientP->deviceDetails.skip_registration = atoi(value);
  } else if(!strcmp (tag, "server_url") ) {
    iot_device_sdk_log(SDK_LOG_INFO,"set_sdk_config_value server_url set to value = %s \n", value);
    SAFE_FREE(clientP->registrationData.server_url)
    clientP->registrationData.server_url = strdup(value);
  } else if(!strcmp (tag, "https_flag") ) {
    iot_device_sdk_log(SDK_LOG_INFO,"set_sdk_config_value https_flag set to value = %s \n", value);
    clientP->registrationData.https_flag = atoi(value);
  } else if(!strcmp (tag, "manufacturingId") ) {
    iot_device_sdk_log(SDK_LOG_INFO,"set_sdk_config_value manufacturingId set to value = %s \n", value);
    SAFE_FREE(clientP->deviceDetails.manufacturingId)
    clientP->deviceDetails.manufacturingId = strdup(value);
  } else if(!strcmp (tag, "device_data_connector_type") ) {
    iot_device_sdk_log(SDK_LOG_INFO,"set_sdk_config_value device_data_connector_type set to value = %s \n", value);
    set_device_data_connector_type(clientP, value);
  } else if(!strcmp (tag, "dataPayloadFile") ) {
    iot_device_sdk_log(SDK_LOG_INFO,"set_sdk_config_value dataPayloadFile set to value = %s \n", value);
    SAFE_FREE(clientP->dataPayloadFile)
    clientP->dataPayloadFile = strdup(value);
  } else if(!strcmp (tag, "device_data_serilization") ) {
    iot_device_sdk_log(SDK_LOG_INFO,"set_sdk_config_value device_data_serilization set to value = %s \n", value);
    SAFE_FREE(clientP->deviceDataSerializationType)
    clientP->deviceDataSerializationType = strdup(value);
  } else if(!strcmp (tag, "cert_authority_bundle_file") ) {
    iot_device_sdk_log(SDK_LOG_INFO,"set_sdk_config_value cert_authority_bundle_file set to value = %s \n", value);
    SAFE_FREE(clientP->certAuthorityBundleFile)
    clientP->certAuthorityBundleFile = strdup(value);
  } else if(!strcmp (tag, "primary_server_url") ) {
    iot_device_sdk_log(SDK_LOG_INFO,"set_sdk_config_value primary_server_url set to value = %s \n", value);
    SAFE_FREE(clientP->registrationData.regUrlPrimarySvr)
    clientP->registrationData.regUrlPrimarySvr = strdup(value);
  } else if(!strcmp (tag, "access_token") ) {
    iot_device_sdk_log(SDK_LOG_INFO,"set_sdk_config_value access_token set to value = %s \n", value);
    SAFE_FREE(clientP->registrationData.accessTokenDetails.access_token )
    clientP->registrationData.accessTokenDetails.access_token = strdup(value);
  } else if(!strcmp (tag, "access_token_expiration") ) {
    iot_device_sdk_log(SDK_LOG_INFO,"set_sdk_config_value access_token_expiration set to value = %s \n", value);
    clientP->registrationData.accessTokenDetails.access_token_expires_in = atoi(value);
  } else if(!strcmp (tag, "refresh_token") ) {
    iot_device_sdk_log(SDK_LOG_INFO,"set_sdk_config_value refresh_token set to value = %s \n", value);
    SAFE_FREE(clientP->registrationData.accessTokenDetails.refresh_token)
    clientP->registrationData.accessTokenDetails.refresh_token = strdup(value);
  } else if(!strcmp (tag, "refresh_token_expiration") ) {
    iot_device_sdk_log(SDK_LOG_INFO,"set_sdk_config_value refresh_token_expiration set to value = %s \n", value);
    clientP->registrationData.accessTokenDetails.refresh_token_expires_in = atoi(value);
  } else if(!strcmp (tag, "device_username") ) {
    iot_device_sdk_log(SDK_LOG_INFO,"set_sdk_config_value device_username set to value = %s \n", value);
    SAFE_FREE(clientP->registrationData.device_username)
    clientP->registrationData.device_username = strdup(value);
  } else if(!strcmp (tag, "device_password") ) {
    iot_device_sdk_log(SDK_LOG_INFO,"set_sdk_config_value device_password set to value = %s \n", value);
    SAFE_FREE(clientP->registrationData.device_password)
    clientP->registrationData.device_password = strdup(value);
  } else if(!strcmp (tag, "log_file_path") ) {
    iot_device_sdk_log(SDK_LOG_INFO,"set_sdk_config_value log_file_path set to value = %s \n", value);
    iot_device_sdk_setLogFile(value);
  } else if(!strcmp (tag, "logging_level") ) {
    iot_device_sdk_log(SDK_LOG_INFO,"set_sdk_config_value logging_level set to value = %s \n", value);
    iot_device_sdk_setLogLevel(atoi(value));
  } else if(!strcmp (tag, "mqtt_url") ) {
    iot_device_sdk_log(SDK_LOG_INFO,"set_sdk_config_value mqtt url  set to value = %s \n", value);
    SAFE_FREE(clientP->mqttClient.mqtt_url)
    clientP->mqttClient.mqtt_url = strdup(value);
  } else if(!strcmp (tag, "mqtt_pub_topic") ) {
    iot_device_sdk_log(SDK_LOG_INFO,"set_sdk_config_value mqtt pub topic  set to value = %s \n", value);
    SAFE_FREE(clientP->mqttClient.mqtt_pub_topic)
    clientP->mqttClient.mqtt_pub_topic = strdup(value);
  } else if(!strcmp (tag, "mqtt_sub_topic") ) {
    iot_device_sdk_log(SDK_LOG_INFO,"set_sdk_config_value mqtt sub topic  set to value = %s \n", value);
    SAFE_FREE(clientP->mqttClient.mqtt_sub_topic)
    clientP->mqttClient.mqtt_sub_topic = strdup(value);
  } else {
    iot_device_sdk_log(SDK_LOG_WARN,"set_sdk_config_value Unknown config tag = %s \n", value);
  }
}

int load_sdk_config( SDKDeviceInternal* clientP, char* sdkConfiPath) {
  int ret = -1;
  char filePath[FILE_PATH_MAX_LEN];

  if(sdkConfiPath) {
      strncpy(filePath, sdkConfiPath, sizeof(filePath));
  } else {
      //Check for the device sdk config file
      if(access(DEFAULT_SDK_CONFIG_FILE, F_OK) != -1 ){
        iot_device_sdk_log(SDK_LOG_INFO,"IOT_SDK load_sdk_config file present in default path\n");
        strncpy(filePath,DEFAULT_SDK_CONFIG_FILE, sizeof(filePath));
      } else {
        iot_device_sdk_log(SDK_LOG_INFO,"IOT_SDK load_sdk_config file NOT present in default path\n");
        strncpy(filePath, ALT_PATH_SDK_CONFIG_FILE, sizeof(filePath));
      }
  }
  ret = load_json_config_file(clientP, filePath);
  if(ret < 0) {
    iot_device_sdk_log(SDK_LOG_ERROR,"load_sdk_config Error: loading sdk config file \n");
  } else {
    json_release_config_data(clientP);
  }

  //Set up the server, access token, observation url and mqtt url
  ret = setup_various_urls(clientP);
  if(ret < 0) {
    iot_device_sdk_log(SDK_LOG_ERROR,"load_sdk_config Error: Setting up various urls failed \n");
    return -1;
  }

  return 0;
}
