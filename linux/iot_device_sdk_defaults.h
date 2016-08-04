/**************************************************************************
  copyright (C) Copyright 2015-2016 Cisco Systems Inc.; All Rights Reserved

  file _iot_device_sdk_default.h
  (Internal File)

  History:
    01/12/2015 - Created.
 ******************************************************************************/

#define MAX_PATH_LEN                    4096
#define DEFAULT_SDK_CONFIG_FILE        "../../config/sdk_config.json"
#define ALT_PATH_SDK_CONFIG_FILE       "./config/sdk_config.json"
#define DEFAULT_LOG_LEVEL               0
//#define DEFAULT_LOG_FILEPATH            "/var/log/iot_device_sdk.log"
#define DEFAULT_LOG_FILEPATH            "./iot_device_sdk.log"

/* used to timeout in case of bad/noresponse url passed to libcurl */
#define IOT_DEFAULT_CURL_TIMEOUT        10L
