/**************************************************************************
  copyright (C) Copyright 2015-2016 Cisco Systems Inc.; All Rights Reserved

  History:
           02/10/2016 - Created.
 ******************************************************************************/

#ifndef __IOT_SDK_HELPER_IN_H__
#define __IOT_SDK_HELPER_IN_H__

#ifdef __cplusplus
extern "C"
{
#endif

enum {
  MILLISECONDS = 0,
  SECONDS
};
void sdk_local_sleep(int type, int sleep_val);

#ifdef __cplusplus
}
#endif

#endif /* __IOT_SDK_HELPER_IN_H__ */
