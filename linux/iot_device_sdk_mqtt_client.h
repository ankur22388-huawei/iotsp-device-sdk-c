/**************************************************************************
  copyright (C) Copyright 2015-2016 Cisco Systems Inc.; All Rights Reserved
  file iot_device_sdk_curl.h
  (Internal File)
  History:
    02/05/2016 - Created.
 ******************************************************************************/

#ifndef _CISCO_IOT_DEVICE_SDK_MQTT_C_H
#define _CISCO_IOT_DEVICE_SDK_MQTT_C_H

#include "MQTTAsync.h"

typedef struct _mqtt_client {
  MQTTAsync client;
  char clientId[24];  //clientId passed to server
  char* mqtt_url;

  char* mqtt_pub_topic;
  char* mqtt_sub_topic;

  int mqtt_msg_arrived;
  int mqtt_data_sent;

  int connect_failed;
  int connect_success;
  int subscribe_failed;
  int subscribe_success;
  int unsubscribe_failed;
  int unsubscribe_success;
} Mqtt_Client;

extern int mqtt_connectionStatus(Mqtt_Client *mqttClient);

/*mqtt client abstraction API's*/
extern int iot_mqtt_client_init(IotDeviceSdkClient handle);
extern int iot_mqtt_client_connect(IotDeviceSdkClient handle);
extern int iot_mqtt_client_publish(IotDeviceSdkClient handle, char* topic, char* payload, int payload_len, int QOS);
extern int iot_mqtt_client_subscribe(IotDeviceSdkClient handle, char* topic, int QOS);
extern int iot_mqtt_client_unsubscribe(IotDeviceSdkClient handle, char* unsubscribe_topic);
extern int iot_mqtt_client_disconnect(IotDeviceSdkClient handle);
extern int iot_mqtt_client_destroy(IotDeviceSdkClient handle);
#endif // _CISCO_IOT_DEVICE_SDK_MQTT_C_H
