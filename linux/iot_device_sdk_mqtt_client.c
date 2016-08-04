/**************************************************************************
  copyright (C) Copyright 2015-2016 Cisco Systems Inc.; All Rights Reserved
  History:
    02/05/2016 - Created.
 ******************************************************************************/
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <assert.h>
 #include <unistd.h>
 #include <iot_device_sdk.h>
 #include "_iot_device_sdk_init.h"
 #include "iot_device_sdk_mqtt_client.h"
 #include "iot_device_sdk_helper.h"
 #include "MQTTAsync.h"

 #define CLIENT_ID        "Device_sdk"
 #define MQTT_PORT        "1883"
 #define MQTT_SECURE_PORT "8883"


int mqtt_connectionStatus(Mqtt_Client *mqttClient)
{
  if(mqttClient){
    if(mqttClient->connect_success)
      return 1;
    else
      return 0;
  } else {
    iot_device_sdk_log(SDK_LOG_ERROR, "mqtt_connectionStatus() invalid mqttClient handle \n");
    return 0;
  }
}

void mqtt_onConnectFailure(void* context, MQTTAsync_failureData* response)
{
  Mqtt_Client *mqttClient = (Mqtt_Client*)context;
  iot_device_sdk_log(SDK_LOG_INFO, "mqtt_onConnect() ++Connect Failed--\n");
  if(mqttClient)
    mqttClient->connect_failed = 1;
  if(response)
    iot_device_sdk_log(SDK_LOG_INFO, "mqtt_onConnect() Connect Failed code = %d message = %s \n", response->code , response->message);
  return;
}

void mqtt_onConnect(void* context, MQTTAsync_successData* response)
{
  Mqtt_Client *mqttClient = (Mqtt_Client*)context;
  iot_device_sdk_log(SDK_LOG_INFO, "mqtt_onConnect() ++Connected-- \n");
  if(mqttClient)
    mqttClient->connect_success = 1;
  if(response)
    iot_device_sdk_log(SDK_LOG_INFO, "mqtt_onConnect() Connect success token = %d \n", response->token);
  return;
}

static void mqtt_ConnectionLost(void *context, char *cause)
{
  SDKDeviceInternal* clientHdle=(SDKDeviceInternal*)context;
  Mqtt_Client *mqttClient =  NULL;
 	MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
  MQTTAsync_SSLOptions ssl_opts = MQTTAsync_SSLOptions_initializer;
 	int rc;

  iot_device_sdk_log(SDK_LOG_WARN, "mqtt_ConnectionLost() ++ cause = %s \n", cause);
  iot_device_sdk_log(SDK_LOG_WARN, "mqtt_ConnectionLost() Reconnecting \n");

  if(clientHdle){
    mqttClient = &(clientHdle->mqttClient);

   	conn_opts.keepAliveInterval = 20;
   	conn_opts.cleansession = 1;
   	conn_opts.onSuccess = mqtt_onConnect;
   	conn_opts.onFailure = mqtt_onConnectFailure;
    conn_opts.context = mqttClient;
   	conn_opts.retryInterval = 1000;

    if(clientHdle->certAuthorityBundleFile){
      iot_device_sdk_log(SDK_LOG_INFO, "mqtt_ConnectionLost() enabling server cert auth ++ \
                        clientHdle->certAuthorityBundleFile = %s\n", clientHdle->certAuthorityBundleFile);
      ssl_opts.trustStore = clientHdle->certAuthorityBundleFile;
      ssl_opts.enableServerCertAuth = 1;
      conn_opts.ssl = &ssl_opts;
    }

   	if ((rc = MQTTAsync_connect(mqttClient->client, &conn_opts)) != MQTTASYNC_SUCCESS)
   	{
   		printf("Failed to start connect, return code %d\n", rc);
      iot_device_sdk_log(SDK_LOG_ERROR, "mqtt_ConnectionLost() Reconnecting FAILED \n");
   	}
  } else {
    iot_device_sdk_log(SDK_LOG_WARN, "mqtt_ConnectionLost() context passed was NULL - reconnect failed \n");
  }
  iot_device_sdk_log(SDK_LOG_INFO, "mqtt_ConnectionLost() -- \n");
  return;
}

int mqtt_MessageArrived(void *context, char *topicName, int topicLen, MQTTAsync_message *message)
{
  SDKDeviceInternal* clientHdle = NULL;
  int i;
  char* payloadptr;

  printf("mqtt_MessageArrived Message arrived\n");
  printf("mqtt_MessageArrived topic: %s\n", topicName);
  printf("mqtt_MessageArrived topic: %d\n", topicLen);
  printf("mqtt_MessageArrived message: ");

  payloadptr = message->payload;
  for(i=0; i<message->payloadlen; i++)
  {
    putchar(*payloadptr++);
  }
  putchar('\n');

  MQTTAsync_freeMessage(&message);
  MQTTAsync_free(topicName);

  if(context){
    clientHdle = (SDKDeviceInternal*)context;
    //TO-DO pass on the incoming mqtt to message to sdk_config
    //message handler
    clientHdle->mqttClient.mqtt_msg_arrived = 1;
  }

  return 1;
}

 /**
  * @brief iot_mqtt_client_init
  * iot_mqtt_client_init ( ) Function to init the mqtt client
  * @param[in]  : Handle to DeviceSdkClient
  * @return     :  0 on Success.
  *               -1 on Error.
 */
int iot_mqtt_client_init(IotDeviceSdkClient handle)
{
  int ret = -1;
  char mqtt_url_port [2048]; //what size ??

  iot_device_sdk_log(SDK_LOG_INFO, "iot_mqtt_client_init() ++ \n");
  if(!handle) {
    iot_device_sdk_log(SDK_LOG_ERROR, "iot_mqtt_client_init Client handle NULL \n");
    return -1;
  }

  SDKDeviceInternal* clientHdle=(SDKDeviceInternal*)handle;
  //Check which port to use
  if(clientHdle->registrationData.https_flag){
    snprintf(mqtt_url_port, sizeof(mqtt_url_port), "%s:%s",clientHdle->mqttClient.mqtt_url, MQTT_SECURE_PORT );
  } else{
    snprintf(mqtt_url_port, sizeof(mqtt_url_port), "%s:%s",clientHdle->mqttClient.mqtt_url, MQTT_PORT );
  }
  iot_device_sdk_log(SDK_LOG_INFO, "iot_mqtt_client_init mqtt url = %s  \n", mqtt_url_port);

  //set clientId
  strncpy(clientHdle->mqttClient.clientId, clientHdle->deviceDetails.manufacturingId, strlen(clientHdle->deviceDetails.manufacturingId));
  iot_device_sdk_log(SDK_LOG_INFO, "iot_mqtt_client_init mqtt clientid = %s  \n", clientHdle->mqttClient.clientId);
  ret = MQTTAsync_create(&(clientHdle->mqttClient.client), mqtt_url_port, clientHdle->mqttClient.clientId, MQTTCLIENT_PERSISTENCE_NONE, NULL);
  if(ret) {
    iot_device_sdk_log(SDK_LOG_ERROR, "iot_mqtt_client_init MQTTAsync_create Failed ret = %d \n", ret);
    return -1;
  } else {
    iot_device_sdk_log(SDK_LOG_INFO, "iot_mqtt_client_init MQTTAsync_create handle =  0x%x \n", clientHdle->mqttClient.client);
  }

  /* Enable - for debug
  MQTTAsync_setTraceCallback(handleTrace);
  MQTTAsync_setTraceLevel(MQTTASYNC_TRACE_ERROR);
  */

  ret = MQTTAsync_setCallbacks(clientHdle->mqttClient.client, clientHdle, mqtt_ConnectionLost, mqtt_MessageArrived, NULL);
  if(ret) {
    iot_device_sdk_log(SDK_LOG_ERROR, "iot_mqtt_client_init  MQTTAsync_setCallbacks Failed ret = %d \n", ret);
    return -1;
  }

  iot_device_sdk_log(SDK_LOG_INFO, "iot_mqtt_client_init() -- \n");
  return 0;
}


/**
 * @brief iot_mqtt_client_connect
 * iot_mqtt_client_connect ( ) Function to init the mqtt client
 * @param[in]  : Handle to DeviceSdkClient
 * @return     :  0 on Success.
 *               -1 on Error.
*/
int iot_mqtt_client_connect(IotDeviceSdkClient handle)
{
  int ret = -1;
  MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
  MQTTAsync_SSLOptions ssl_opts = MQTTAsync_SSLOptions_initializer;

  iot_device_sdk_log(SDK_LOG_INFO, "iot_mqtt_client_connect() ++ \n");
  if(!handle) {
    iot_device_sdk_log(SDK_LOG_ERROR, "iot_mqtt_client_connect sdk Client handle NULL \n");
    return -1;
  }
  SDKDeviceInternal* clientHdle=(SDKDeviceInternal*)handle;

  if (clientHdle->mqttClient.client) {
   	conn_opts.cleansession = 1;
    conn_opts.onSuccess = mqtt_onConnect;
    conn_opts.onFailure = mqtt_onConnectFailure;
    conn_opts.context = &(clientHdle->mqttClient);
   	conn_opts.keepAliveInterval = 20;
   	conn_opts.retryInterval = 0;

    if(clientHdle->certAuthorityBundleFile){
      ssl_opts.trustStore = clientHdle->certAuthorityBundleFile;
      iot_device_sdk_log(SDK_LOG_INFO,"iot_mqtt_client_connect enabling server cert verification with trustStore = %s \n", ssl_opts.trustStore  );
      ssl_opts.enableServerCertAuth = 1;
    } else {
      ssl_opts.trustStore = NULL;
      iot_device_sdk_log(SDK_LOG_INFO,"iot_mqtt_client_connect Disabling server cert verification \n");
      ssl_opts.enableServerCertAuth = 0;
    }
    conn_opts.ssl = &ssl_opts;

    //update the username and password
    if(clientHdle->registrationData.device_username && clientHdle->registrationData.device_password){
      conn_opts.username = clientHdle->registrationData.device_username;
      conn_opts.password = clientHdle->registrationData.device_password;

     	if ((ret = MQTTAsync_connect(clientHdle->mqttClient.client, &conn_opts)) != MQTTASYNC_SUCCESS)
     	{
        iot_device_sdk_log(SDK_LOG_ERROR, "iot_mqtt_client_connect MQTTAsync_connect Failed ret = %d \n", ret);
        //clean up - for the create call?
     		return -1;
     	}

     	iot_device_sdk_log(SDK_LOG_INFO, "iot_mqtt_client_connect Waiting for connect\n");
     	while (clientHdle->mqttClient.connect_success == 0 && clientHdle->mqttClient.connect_failed == 0 ) {
        iot_device_sdk_log(SDK_LOG_WARN, "iot_mqtt_client_connect Waiting for mqtt connect: success:%d failed:%d \n", clientHdle->mqttClient.connect_success, clientHdle->mqttClient.connect_failed);
        // Decide whether to wait here for the conenction to complete
        //usleep(100000L);
        sdk_local_sleep(SECONDS,1);
     	}
    } else {
      iot_device_sdk_log(SDK_LOG_ERROR, "iot_mqtt_client_publish mqtt client username:password not set \n");
      return -1;
    }
  } else {
    iot_device_sdk_log(SDK_LOG_ERROR, "iot_mqtt_client_publish mqtt client handle NULL, MQTT client Not initilized \n");
    return -1;
  }
  if(clientHdle->mqttClient.connect_success) {
    iot_device_sdk_log(SDK_LOG_INFO, "iot_mqtt_client_connect() -- Success \n");
    return 0;
  } else
    iot_device_sdk_log(SDK_LOG_INFO, "iot_mqtt_client_connect() -- Failed \n");

  return -1;
}


void mqtt_onDataSend(void* context, MQTTAsync_successData* response)
{
  Mqtt_Client *mqttClient = NULL;
  if(context) {
    mqttClient = (Mqtt_Client*)context;
    mqttClient->mqtt_data_sent = 1;
  }

  if(response)
    iot_device_sdk_log(SDK_LOG_INFO, "mqtt_onDataSend() token value :%d delivery confirmed ++ \n", response->token);

  return ;
}

/**
 * @brief iot_mqtt_client_publish
 * iot_mqtt_client_publish ( ) Function to init the mqtt client
 * @param[in]  : Handle to DeviceSdkClient
 * @return     :  0 on Success.
 *               -1 on Error.
*/
int iot_mqtt_client_publish(IotDeviceSdkClient handle, char* topic, char* payload, int payload_len, int QOS)
{
  int ret = -1;
  MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
	MQTTAsync_message pubmsg = MQTTAsync_message_initializer;


  iot_device_sdk_log(SDK_LOG_INFO, "iot_mqtt_client_publish() ++ \n");
  if(!handle) {
    iot_device_sdk_log(SDK_LOG_ERROR, "iot_mqtt_client_publish sdk Client handle NULL \n");
    return -1;
  }
  SDKDeviceInternal* clientHdle=(SDKDeviceInternal*)handle;

  if (clientHdle->mqttClient.client) {
    opts.onSuccess = mqtt_onDataSend;
    opts.context = clientHdle->mqttClient.client;

    if(payload && payload_len > 0) {
      pubmsg.payload = payload;
      //pubmsg.payloadlen = strlen(payload);
      pubmsg.payloadlen = payload_len;
      pubmsg.qos = QOS;
      pubmsg.retained = 0;
      //deliveredtoken = 0;
    } else {
      iot_device_sdk_log(SDK_LOG_ERROR, "iot_mqtt_client_publish payload len = %d\n", payload_len);
      iot_device_sdk_log(SDK_LOG_ERROR, "iot_mqtt_client_publish payload ptr = ox%x\n", payload);
      iot_device_sdk_log(SDK_LOG_ERROR, "iot_mqtt_client_publish payload passed is NULL or payload len is less than Zero  \n");

      return -1;
    }

    if(topic) {
      if ((ret = MQTTAsync_sendMessage(clientHdle->mqttClient.client, topic, &pubmsg, &opts)) != MQTTASYNC_SUCCESS)
      {
        iot_device_sdk_log(SDK_LOG_ERROR, "iot_mqtt_client_publish Failed to sendMessage to topic: %s ret:%d \n", topic, ret);
        return -1;
      }
    } else {
      iot_device_sdk_log(SDK_LOG_ERROR, "iot_mqtt_client_publish topic passed is NULL \n");
      return -1;
    }
  } else {
    iot_device_sdk_log(SDK_LOG_ERROR, "iot_mqtt_client_publish mqtt client handle NULL, MQTT client Not initilized \n");
    return -1;
  }

  iot_device_sdk_log(SDK_LOG_INFO, "iot_mqtt_client_publish() -- \n");
  return 0;
}


void mqtt_onSubscribe(void* context, MQTTAsync_successData* response)
{
  Mqtt_Client *mqttClient = NULL;
  if(context && response ) {
    mqttClient = (Mqtt_Client*)context;
    //reset mqtt subscribe success status
    mqttClient->subscribe_success = 1;
  }
  iot_device_sdk_log(SDK_LOG_INFO, "mqtt_onSubscribe() Topic subscribe successfull ++-- \n");
	return;
}

void mqtt_onSubscribeFailure(void* context, MQTTAsync_failureData* response)
{
  Mqtt_Client *mqttClient = NULL;
  if(context && response ) {
    mqttClient = (Mqtt_Client*)context;
    //reset mqtt subscribe failure status
    mqttClient->subscribe_failed = 1;
  }
  iot_device_sdk_log(SDK_LOG_INFO, "mqtt_onSubscribeFailure() Topic subscribe Failed, error Code: %d\n", response ? response->code : 0);
  return;
}

/**
 * @brief iot_mqtt_client_subscribe
 * iot_mqtt_client_subscribe ( ) Function to init the mqtt client
 * @param[in]  : Handle to DeviceSdkClient
 * @return     :  0 on Success.
 *               -1 on Error.
*/
int iot_mqtt_client_subscribe(IotDeviceSdkClient handle, char* topic, int QOS)
{
  int ret = -1;
	MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;

  iot_device_sdk_log(SDK_LOG_INFO, "iot_mqtt_client_subscribe() ++ \n");
  if(!handle) {
    iot_device_sdk_log(SDK_LOG_ERROR, "iot_mqtt_client_subscribe sdk Client handle NULL \n");
    return -1;
  }
  SDKDeviceInternal* clientHdle=(SDKDeviceInternal*)handle;

  if (clientHdle->mqttClient.client) {
    	opts.onSuccess = mqtt_onSubscribe;
    	opts.onFailure = mqtt_onSubscribeFailure;
    	opts.context = clientHdle->mqttClient.client;

      if(topic) {
      	if ((ret = MQTTAsync_subscribe(clientHdle->mqttClient.client, topic, QOS, &opts)) != MQTTASYNC_SUCCESS)
      	{
          iot_device_sdk_log(SDK_LOG_ERROR, "iot_mqtt_client_subscribe Failed to start subscribe topic :%s, return code %d\n", topic, ret);
          return -1;
      	}
      } else {
        iot_device_sdk_log(SDK_LOG_ERROR, "iot_mqtt_client_subscribe topic passed is NULL \n");
        return -1;
      }
  } else {
    iot_device_sdk_log(SDK_LOG_ERROR, "iot_mqtt_client_subscribe mqtt client handle NULL, MQTT client Not initilized \n");
  }

  iot_device_sdk_log(SDK_LOG_INFO, "iot_mqtt_client_subscribe() -- \n");
  return 0;
}


void mqtt_onUnSubscribe(void* context, MQTTAsync_successData* response)
{
  Mqtt_Client *mqttClient = NULL;
  if(context && response ) {
    mqttClient = (Mqtt_Client*)context;
    //reset mqtt unsubscribe success status
    mqttClient->unsubscribe_success = 1;
  }
  iot_device_sdk_log(SDK_LOG_INFO, "mqtt_onUnSubscribe() Topic Unsubscribe successfull ++-- \n");
	return;
}

void mqtt_onUnSubscribeFailure(void* context, MQTTAsync_failureData* response)
{
  Mqtt_Client *mqttClient = NULL;
  if(context && response ) {
    mqttClient = (Mqtt_Client*)context;
    //reset mqtt unsubscribe failure status
    mqttClient->unsubscribe_failed = 1;
  }
  iot_device_sdk_log(SDK_LOG_INFO, "mqtt_onUnSubscribeFailure() Topic UnSubscribe Failed, error Code: %d\n", response ? response->code : 0);
  return;
}

/**
 * @brief iot_mqtt_client_unsubscribe
 * iot_mqtt_client_unsubscribe ( ) Function to init the mqtt client
 * @param[in]  : Handle to DeviceSdkClient
 * @return     :  0 on Success.
 *               -1 on Error.
*/
int iot_mqtt_client_unsubscribe(IotDeviceSdkClient handle, char* unsubscribe_topic)
{
  int ret = -1;
	MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;

  iot_device_sdk_log(SDK_LOG_INFO, "iot_mqtt_client_subscribe() ++ \n");
  if(!handle) {
    iot_device_sdk_log(SDK_LOG_ERROR, "iot_mqtt_client_subscribe sdk Client handle NULL \n");
    return -1;
  }
  SDKDeviceInternal* clientHdle=(SDKDeviceInternal*)handle;

  if (clientHdle->mqttClient.client) {
    	opts.onSuccess = mqtt_onUnSubscribe;
    	opts.onFailure = mqtt_onUnSubscribeFailure;
    	opts.context = clientHdle->mqttClient.client;

      if(unsubscribe_topic) {
      	if ((ret = MQTTAsync_unsubscribe(clientHdle->mqttClient.client, unsubscribe_topic, &opts)) != MQTTASYNC_SUCCESS)
      	{
          iot_device_sdk_log(SDK_LOG_ERROR, "iot_mqtt_client_unsubscribe Failed to start un-subscribe topic:%s , return code %d\n", unsubscribe_topic, ret);
          return -1;
      	}
      } else {
        iot_device_sdk_log(SDK_LOG_ERROR, "iot_mqtt_client_unsubscribe topic passed is NULL \n");
        return -1;
      }
   } else {
     iot_device_sdk_log(SDK_LOG_ERROR, "iot_mqtt_client_unsubscribe mqtt client handle NULL, MQTT client Not initilized \n");
   }
   iot_device_sdk_log(SDK_LOG_INFO, "iot_mqtt_client_unsubscribe() -- \n");
   return 0;
}

void mqtt_onDisconnect(void* context, MQTTAsync_successData* response)
{
  Mqtt_Client *mqttClient = NULL;

  iot_device_sdk_log(SDK_LOG_INFO, "mqtt_onDisconnect() ++-- mqtt client Succesfully Disconnected \n");
  if(context) {
    mqttClient = (Mqtt_Client*)context;
    //reset mqtt status
    mqttClient->connect_failed = 0;
    mqttClient->connect_success = 0;
  }
  if (response)
    iot_device_sdk_log(SDK_LOG_INFO, "mqtt_onDisconnect() mqtt client Succesfully Disconnected \n");

	return;
}

/**
 * @brief iot_mqtt_client_disconnect
 * iot_mqtt_client_disconnect ( ) Function to init the mqtt client
 * @param[in]  : Handle to DeviceSdkClient
 * @return     :  0 on Success.
 *               -1 on Error.
*/
int iot_mqtt_client_disconnect(IotDeviceSdkClient handle)
{
  int ret = -1;
  MQTTAsync_disconnectOptions disc_opts = MQTTAsync_disconnectOptions_initializer;

  iot_device_sdk_log(SDK_LOG_INFO, "iot_mqtt_client_disconnect() ++ \n");
  if(!handle) {
    iot_device_sdk_log(SDK_LOG_ERROR, "iot_mqtt_client_disconnect sdk Client handle NULL \n");
    return -1;
  }
  SDKDeviceInternal* clientHdle=(SDKDeviceInternal*)handle;

  if (clientHdle->mqttClient.client) {
    disc_opts.onSuccess = mqtt_onDisconnect;
    disc_opts.context = clientHdle->mqttClient.client;
    if ((ret = MQTTAsync_disconnect(clientHdle->mqttClient.client, &disc_opts)) != MQTTASYNC_SUCCESS)
    {
      printf("iot_mqtt_client_disconnect Failed to start disconnect, return code:%d\n", ret);
      iot_device_sdk_log(SDK_LOG_ERROR, "iot_mqtt_client_disconnect Failed to start disconnect, return code:%d\n", ret);
      return -1;
    }
  } else {
    printf("iot_mqtt_client_disconnect mqtt client handle NULL, MQTT client not initilized \n");
    iot_device_sdk_log(SDK_LOG_ERROR, "iot_mqtt_client_disconnect mqtt client handle NULL, MQTT client Not initilized \n");
  }
  iot_device_sdk_log(SDK_LOG_INFO, "iot_mqtt_client_disconnect() -- \n");
  return 0;
}

/**
 * @brief iot_mqtt_client_destroy
 * iot_mqtt_client_destroy ( ) Function to init the mqtt client
 * @param[in]  : Handle to DeviceSdkClient
 * @return     :  0 on Success.
 *               -1 on Error.
*/
int iot_mqtt_client_destroy(IotDeviceSdkClient handle)
{
   iot_device_sdk_log(SDK_LOG_INFO, "iot_mqtt_client_destroy() ++ \n");
   if(!handle) {
     iot_device_sdk_log(SDK_LOG_ERROR, "iot_mqtt_client_destroy Client handle NULL \n");
     return -1;
   }

   SDKDeviceInternal* clientHdle=(SDKDeviceInternal*)handle;
   if (clientHdle->mqttClient.client) {
     MQTTAsync_destroy(&(clientHdle->mqttClient.client));
   } else {
     printf("iot_mqtt_client_destroy mqtt client handle NULL, MQTT client not initilized \n");
     iot_device_sdk_log(SDK_LOG_ERROR, "iot_mqtt_client_destroy mqtt client handle NULL, MQTT client Not initilized \n");
   }
   iot_device_sdk_log(SDK_LOG_INFO, "iot_mqtt_client_destroy() -- \n");
   return 0;
}
