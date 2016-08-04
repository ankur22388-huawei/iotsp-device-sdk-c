/**************************************************************************
  copyright (C) Copyright 2015-2016 Cisco Systems Inc.; All Rights Reserved
  History:
           01/12/2016 - Created.
 ******************************************************************************/

#ifndef __IOT_SDK_DEVICE_REG_H__
#define __IOT_SDK_DEVICE_REG_H__

#ifdef __cplusplus
extern "C"
{
#endif

enum RegistrationStep {
  REG_START = 0,
  REG_PRIMARY,
  REG_CPOD,
  REG_IAM,
  REG_END
} ;

/**
* iot_device_register( ) - Function to load configuration data
* or actual registration with the IOT servers
* Input  :
* return :  0 - success
*          -1 - Error
*/
int device_register(IotDeviceSdkClient handle, DeviceDetails* deviceDetail);

#ifdef __cplusplus
}
#endif

#endif /* __IOT_SDK_DEVICE_REG_H__ */
