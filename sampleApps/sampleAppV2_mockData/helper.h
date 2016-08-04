/**************************************************************************
  copyright (C) Copyright 2015-2016 Cisco Systems Inc.; All Rights Reserved
  History:
           01/12/2016 - Created.
 ******************************************************************************/

#ifndef __IOT_SDK_HELPER_H__
#define __IOT_SDK_HELPER_H__

#ifdef __cplusplus
extern "C"
{
#endif

char* getMacAddress();
char* getipv4_Address();

enum {
  MILLISECONDS = 0,
  SECONDS
};
void local_sleep(int type, int sleep_val);

#ifdef __cplusplus
}
#endif

#endif /* __IOT_SDK_HELPER_H__ */
