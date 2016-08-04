/**************************************************************************//**
  @file iot_device_sdk.h
  @copyright (C) Copyright 2015-2016 Cisco Systems Inc.; All Rights Reserved
  @brief Device SDK API's.
    Version 1 and Version 2 Api's.
  @details Device SDK API's
    - V1 (version 1 api's)
        - initialize device sdk
        - register device to iotsp
        - send data to iotsp device adapter
        - de-initialize device sdk
    - V2 (version 2 api)
        - send device data API (does device sdk initialize, registers device
        and sends data )
    - (Common API's)
        - API's to setup Device SDK Log file and debug levels.

    Version 1 Api's provide more control and flexibility, i.e. Application
    will have more control on the call sequence and call timimg.
    Version 2 Api is an API to accompilish all that version 1 api's provide in
    one single API. Usage of version 2 API simplifies the application code.
  @internal
  History:
    01/12/2016 - Created.
 ******************************************************************************/

#ifndef _CISCO_IOT_DEVICE_SDK_H
#define _CISCO_IOT_DEVICE_SDK_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Various Debug Levels supported by IOT Device SDK
 */
#define SDK_LOG_MSG     0
#define SDK_NO_LOGGING  1
#define SDK_LOG_ERROR   2
#define SDK_LOG_WARN    3
#define SDK_LOG_DEBUG   4
#define SDK_LOG_INFO    5


#define SDK_SERVER_SUCCESS  200
#define SDK_SERVER_FAILURE  500
#define SDK_DATA_SEND_SUCCESS     204


enum ServerType {
  SERVER_PRIMARY = 1,
  SERVER_CPOD,
  SERVER_IAM
};

enum IotErrorTypes {
  IOTERR_NONE = 0,
  IOTERR_CPOD_URL_NULL,
  IOTERR_CPOD_JSON_SERIALIZATION,
  IOTERR_CPOD_CURL_ERR,
  IOTERR_CPOD_UNKNOWN_SERIALIZATION,
  IOTERR_CPOD_SERIALIZATION_NOT_SET,
  IOTERR_CPOD_CLIENT_HANDLE_NOT_SET,
};

enum deviceDataConnectorType {
  CONNECTOR_TYPE_MQTT = 0,  //mqtt
  CONNECTOR_TYPE_HTTPS,     //https
  CONNECTOR_TYPE_WAMP       //wamp
};

#define SDK_TOKEN_DELIMITER "."
#define DELIM SDK_TOKEN_DELIMITER // short form

/**
 * Handle that holds the SDK  context - which represents a device/client
 * instance.
 */
struct IotDeviceSDKClient;
typedef const struct IotDeviceSDKClient *IotDeviceSdkClient;

/**
 * @brief Structure to captures device info details
 *
 * Applicaiton code uses this device details structure to fill out the device
 * information that needs to be passed on to the device SDK. There are few feilds
 * that are mandatory and others are not mandatory.
 *
 * Mandatory feilds
 *  - manufacturingId ; An unique id that the application wants to use.
 *                  it could be a combination of the ramdon numbner + device serial
 *                  or mac id.
 *
 * Optional feilds
 *  - deviceType, deviceMake, deviceModel
 *  - deviceFirmwareVer, hardwareVer,
 *  - macAddress, deviceSerialNum, ipv4, ipv6, alt_id1
 *
 */
typedef struct _DeviceDetails {
  /*! Uniquie device manufacturing ID for the device */
  char *manufacturingId;
  /*! device type - Sensor, Camera, switch ...etc */
  char *deviceType;
  /*! device make - Manufacturer name/Informtion. */
  char *deviceMake;
  /*! device model - Model number of the device. */
  char *deviceModel;
  /*! device firmware version - Firmware version the device is running. */
  char *deviceFirmwareVer;
  /*! device firmware version - Firmware version the device is running. */
  char *hardwareVer;
  /*! device mac address */
  char *macAddress;
  /*! device serial number */
  char *deviceSerialNum;

  /*! device ipv4 address */
  char *ipv4;
  /*! device ipv6 address */
  char *ipv6;

  /*! Any other device Identifiction Number/String */
  char *alt_id1;

  /* Set this to skip registration step */
  int skip_registration;

  /* data connector Types : mqtt, https and wamp */
  enum deviceDataConnectorType dataConnectorType;

  /* to set custom path to sdk config file*/
  char *sdkConfigFile;
} DeviceDetails;


typedef struct _SDKError {
  char *err_tag;
  char *err_value;
  struct _SDKError *next;
} SDKError;

/**
 * @brief Structure that holds the device registration response
 *
 * Device registration details are strored in this structure, like registration
 * response code, device credentials, device connector information etc
 *
 */
typedef struct _SDKRegistrationResponse {
  /*! http response code - 500, 200 ...etc */
  int responseCode;
  /*! response from server in json/xx format */
  char *response;
  /*! claimed */
  int claimed;
  /*! registered */
  int registered;
  /*! thing Uid */
  char *thingUid;
  /*! security */
  char *security;

  /*! device Credentials - name */
  char *cred_name;
  /*! device Credentials - secret*/
  char *cred_secret;

  /*! interval e.g '1s' */
  char *interval;
  /*! protocol */
  char *protocol;
  /*! registrationURI */
  char *registrationURI;

  int numErrors;
  SDKError *errors;

}SDKRegistrationResponse;


typedef struct _AccessTokenDetails {
  int responseCode;
  /*! response from IAM server in json format */
  char *response;
  /*! scope */
  char *scope;
  /*! token_type */
  char *token_type;
  /*! refresh_token */
  char *refresh_token;
  /*! refresh token expires_in */
  int refresh_token_expires_in;
  /*! access_token */
  char *access_token;
  /*! refresh token expires_in */
  int access_token_expires_in;
}AccessTokenDetails;


typedef struct _DataPostResponse {
  int responseCode;
  /*! response from observation server*/
  char *response;
}DataPostResponse;

typedef struct _ConfigurationData{
  char *buf;
  int buf_len;
//  int type; //json, csv ..
} ConfigurationData;

/**
 * Structure that will hold the data that device wants to send out
 * (name , value , type)
 */
typedef struct _DeviceData {
  char *name;
  char *value;
  char *type;
} DeviceData;

/**
 * Structure that will hold the serialized xml data
 */
typedef struct _SerializedData {
  char *buf;
  int len;
} SerializedData;

/**
 * Structure that will hold the deserialized data, parsed from json/..
 */
typedef struct _DeserializedData{
  DeviceData *data;
  int num;
} DeserializedData;


/**
 * @brief iot_device_sdk_init_v1
 *
 * iot_device_sdk_init_v1 ( ) function is the first function to be called to initilize
 * IOT Device SDK library.
 * @param[in] void None
 * @return     : Returns a Handle to the new client Instance that was created
 *               NULL on Failure (Check log files for exact error condition)
*/
IotDeviceSdkClient iot_device_sdk_init_v1 (DeviceDetails *deviceDetail);

/**
 * @brief iot_device_sdk_deinit_v1
 * iot_device_sdk_deinit_v1 ( ) Function to be called when exiting Client/App Code
 * @param[in]  : None
 * @return     :  0 on Success
 *               -1 on Error
*/
int iot_device_sdk_deinit_v1 ();

/**
 * @brief iot_dsdk_sendRegistrationMessage_v1
 * iot_dsdk_sendRegistrationMessage_v1 ( ) - Function to send registration message to CPOD server
 * @param[in]  : clientHandle
 * @return     : SDKRegistrationResponse* on Success
 *               NULL on Error
*/
const SDKRegistrationResponse* iot_dsdk_sendRegistrationMessage_v1(IotDeviceSdkClient clientHandle, int *error_code);

/**
 * @brief iot_dsdk_sendPrimaryRegistrationMessage
 * iot_dsdk_sendPrimaryRegistrationMessage ( ) Function to send registration message
 * @param[in]  : Handle to DeviceSdkClient
 * @return     : SDKRegistrationResponse* on Success
 *               -1 on Error.
*/
const SDKRegistrationResponse* iot_dsdk_sendPrimaryRegistrationMessage_v1(IotDeviceSdkClient clientHandle);

/**
 * @brief iot_dsdk_getAccessToken_v1
 * iot_dsdk_getAccessToken_v1 ( ) - Function to get an access token
 * @param[in]  : clientHandle
 * @return     : AccessTokenDetails* on success
 *               NULL on Error
*/
const AccessTokenDetails* iot_dsdk_getAccessToken_v1(IotDeviceSdkClient clientHandle);

/**
 * @brief iot_device_sdk_data_send_v1
 * iot_device_sdk_data_send_v1 ( ) - Function which send the device data
 * @param[in]  : clientHandle
 *               jsonData - pointer to the json data payload
 *               jsonData_len - size of json payload
 *               tags - user defined tags "example : "tag1:tag2"
 *               tags_len - size of the tags field
 * @return     : 0 on success and -1 on error
 *
*/
int iot_device_sdk_data_send_v1 (IotDeviceSdkClient clientHandle, char* jsonData, int jsonData_len, char* tags, int tags_len);


/**
* Version 2 API- exposes only api - i.e. the data send API.
* This api is responsible to get the registration done, setup the mqtt init/connection done
* And send the the device data.
*/

/**
 * @brief iot_device_sdk_data_send_v2
 * iot_device_sdk_data_send_v2 ( ) - Function which does registration and sends the device data
 * @param[in]  : Ptr to device Details struct
 *               json payload ptr and size
 *               tags and tag len ( "tag1:tag2" - tags are seperated by collon)
 *               - pass NULL for NO tags
 *               - tags are supported for https data post
 * @return     : 0 on success and -1 on error
 *
*/
int iot_device_sdk_data_send_v2 (DeviceDetails *deviceDetail, char* jsonData, int jsonData_len, char* tags, int tags_len);


/* Logging API's can be used both with v1 and v2 version of sdk apis */
/**
 * @brief iot_device_sdk_setLogLevel
 * iot_device_sdk_setLogLevel ( ) - Function to set the Device SDK log level
 * logging level.
 * @param[in]  int logLevel
 * @return     :  0 on Success
 *               -1 on Error
*/
int iot_device_sdk_setLogLevel (int logLevel);

/**
 * @brief iot_device_sdk_setLogFile_v1
 * iot_device_sdk_setLogFile_v1 ( ) - Function to set the Device SDK log file
 * @param[in]  char *logFile : path to logfile
 * @return     :  0 on Success
 *               -1 on Error
*/
int iot_device_sdk_setLogFile (const char *logFile);

/**
 * @brief iot_device_sdk_log
 * iot_device_sdk_log ( ) - Function to log debug messages
 * @param[in]  int  loglevel
 * @param[in]  char*   format
 * @return     :  0 on Success
 *               -1 on Error
*/
int iot_device_sdk_log (int level, const char* format, ...);

#ifdef __cplusplus
}
#endif

#endif // _CISCO_IOT_DEVICE_SDK_H
