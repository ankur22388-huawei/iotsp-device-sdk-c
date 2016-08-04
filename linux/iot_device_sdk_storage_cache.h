/**************************************************************************
copyright (C) Copyright 2015-2016 Cisco Systems Inc.; All Rights Reserved
History:
        03/18/2016 - Created.
******************************************************************************/

#ifndef _IOT_STORAGE_CACHE_H_
#define _IOT_STORAGE_CACHE_H_

#include "_iot_device_sdk_init.h"

#ifdef  __cplusplus
extern "C"{
#endif

/*****************************************************************************************************************
* Function Name : init_device_storage_cache
* Description   : initilizes the device storage cache
* return        : 0  - on Sucess
*                 -1 - on Failure
******************************************************************************************************************/
int init_device_storage_cache();

/*****************************************************************************************************************
* Function Name : destroy_device_storage_cache
* Description   : destory all the device entries in the cache and the device entry list
* return        : None
******************************************************************************************************************/
void destroy_device_storage_cache();

/*****************************************************************************************************************
* Function Name : flush_all_device_entries_from_cache
* Description   : flush all device entries in cache
* return        : None
******************************************************************************************************************/
void flush_all_device_entries_from_cache();

/*****************************************************************************************************************
* Function Name : add_device_Entry_to_cache
* Description   : add a new device entry to cache
* return        : 0  - on Success
*                 -1 - on Failure
******************************************************************************************************************/
int add_device_Entry_to_cache (SDKDeviceInternal* deviceEntry);

/*****************************************************************************************************************
* Function Name : delete_device_entry_from_cache
* Description   : Removes a device entry from the device list cache
* return        : 0  - on Success
*                 -1 - on Failure
******************************************************************************************************************/
int delete_device_entry_from_cache (char* deviceUid);

/*****************************************************************************************************************
* Function Name : get_device_Entry_from_cache
* Description   : get a device entry from cache
* return        : 0  - on Success
*                 -1 - on Failure
******************************************************************************************************************/
SDKDeviceInternal*  get_device_Entry_from_cache (char* deviceUid);

#ifdef  __cplusplus
}
#endif

#endif /* _IOT_STORAGE_CACHE_H_ */
