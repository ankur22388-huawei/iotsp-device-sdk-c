/**************************************************************************
  copyright (C) Copyright 2015-2016 Cisco Systems Inc.; All Rights Reserved
  History:
          02/10/2016 - Created.
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include <iot_device_sdk.h>
#include "iot_device_sdk_helper.h"

void sdk_local_sleep(int type, int sleep_val)
{
  const char *units;
  int usleep_val;
  switch (type) {
    case MILLISECONDS:
      units = "ms";
      usleep_val = sleep_val * 1000;
      break;
    case SECONDS:
      units = "sec";
      usleep_val = sleep_val * 1000 * 1000;
      break;
    default:
      iot_device_sdk_log(SDK_LOG_WARN,"IOT_SDK local_sleep :%s Unknown type %d \n", __FUNCTION__, type);
      assert(0);
      return;
  }
  iot_device_sdk_log(SDK_LOG_INFO,"IOT_SDK local_sleep : %s %d %s...... \n", __FUNCTION__, sleep_val, units );
  usleep(usleep_val); // sleep for specified interval
}
