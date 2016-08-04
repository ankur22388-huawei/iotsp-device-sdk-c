/**************************************************************************
  copyright (C) Copyright 2015-2016 Cisco Systems Inc.; All Rights Reserved
  History:
           02/10/2016 - Created.
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
int iot_device_register(IotDeviceSdkClient handle, DeviceDetails* deviceDetail);

/**
* iot_register_step( ) - Function to perform a sequence of registration steps.
* It takes in a step id, which could be the start of the sequence, or a middle step.
* Input  :
* return :  0 - success
*          -1 - Error
*/
int iot_register_step(IotDeviceSdkClient handle, int regStep);


void iot_handle_send_data_error(IotDeviceSdkClient handle, int response_code);

int iot_init_reverse_path_listener(IotDeviceSdkClient handle);

#ifdef __cplusplus
}
#endif

#endif /* __IOT_SDK_DEVICE_REG_H__ */
