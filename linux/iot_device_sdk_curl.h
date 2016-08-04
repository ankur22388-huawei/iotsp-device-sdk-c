/**************************************************************************
  copyright (C) Copyright 2015-2016 Cisco Systems Inc.; All Rights Reserved
  file iot_device_sdk_curl.h
  (Internal File)
  History:
    01/12/2016 - Created.
 ******************************************************************************/

#ifndef _CISCO_IOT_DEVICE_SDK_CURL_INTERNAL_H
#define _CISCO_IOT_DEVICE_SDK_CURL_INTERNAL_H

/* init/deinit libcurl */
extern void sdk_init_curl();
extern void sdk_deinit_curl();

extern int send_RegistrationMessage_curl(SDKDeviceInternal *clientHandle,
    const char *url, const char *data, SDKRegistrationResponse *responseData);

extern int send_AccessTokenRequest_curl(SDKDeviceInternal *clientHandle,
      const char *url, const char *data, AccessTokenDetails *responseData);

extern int send_device_data_curl(SDKDeviceInternal *clientHandle,
        const char *url, const char *data, const char* tags, DataPostResponse *responseData);
#endif // _CISCO_IOT_DEVICE_SDK_CURL_INTERNAL_H
