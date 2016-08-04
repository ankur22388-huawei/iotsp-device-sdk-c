/*******************************************************************************
* SampleApp - Sample Application which shows the how to interact with iot device sdk.
* Shows how to :
*   - set device details and initilize iot_device_sdk
*   - set sdk logging level
*   - regiter the device with registration server's and get access token
*   - fill device data in device data structure
*   - serilaize device data and to send data to server
*   History :
*             01/12/2016 - Created.
*
*********************************************************************************/
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>

#include "iot_device_sdk.h"
#include "helper.h"
#include "device_registration.h"

volatile sig_atomic_t exit_flag = 0;
static int test_mode_loop = 0;

int check_app_exit_flag()
{
  if(test_mode_loop || exit_flag)
    return 1;
  else
    return 0;
}
void exit_app_function( int sig )
{
  printf("sampleApp exit_app_function sig = %d \n", sig);
  exit_flag = 1;
  return;
}

void remove_new_line(char* string)
{
  char* ptr = NULL;
  if((ptr = strchr(string, '\n')) != NULL)
    *ptr = '\0';
  return;
}

void populate_device_details(DeviceDetails* deviceDetail, char* filePtr)
{
  FILE *fp=NULL;
  char line[1024];
  const char sep[2] = "=";
  char *token;
  if(deviceDetail){
    //if deviceInfo.txt file present - read data from it

    if(filePtr) {
      fp = fopen(filePtr, "r+");
      if(fp){
        while(fgets(line, sizeof line, fp)!= NULL) {
          token = strtok(line, sep);
          if(!strcmp("manufacturingId", token)){
              token = strtok(NULL, sep);
              deviceDetail->manufacturingId = strdup(token);
              remove_new_line(deviceDetail->manufacturingId);
          } else if(!strcmp("deviceType", token)){
              token = strtok(NULL, sep);
              deviceDetail->deviceType = strdup(token);
              remove_new_line(deviceDetail->deviceType);
          } else if(!strcmp("deviceMake", token)){
              token = strtok(NULL, sep);
              deviceDetail->deviceMake = strdup(token);
              remove_new_line(deviceDetail->deviceMake);
          } else if(!strcmp("deviceModel", token)){
              token = strtok(NULL, sep);
              deviceDetail->deviceModel = strdup(token);
              remove_new_line(deviceDetail->deviceModel);
          } else if(!strcmp("deviceFirmwareVer", token)){
              token = strtok(NULL, sep);
              deviceDetail->deviceFirmwareVer = strdup(token);
              remove_new_line(deviceDetail->deviceFirmwareVer);
          } else if(!strcmp("hardwareVer", token)){
              token = strtok(NULL, sep);
              deviceDetail->hardwareVer = strdup(token);
              remove_new_line(deviceDetail->hardwareVer);
          } else if(!strcmp("macAddress", token)){
              token = strtok(NULL, sep);
              deviceDetail->macAddress = strdup(token);
              remove_new_line(deviceDetail->macAddress);
          } else if(!strcmp("deviceSerialNum", token)){
              token = strtok(NULL, sep);
              deviceDetail->deviceSerialNum = strdup(token);
              remove_new_line(deviceDetail->deviceSerialNum);
          } else if(!strcmp("ipv4", token)){
              token = strtok(NULL, sep);
              deviceDetail->ipv4 = strdup(token);
              remove_new_line(deviceDetail->ipv4);
          }
        }
      }
    } else {
      deviceDetail->manufacturingId = strdup("FTX1942802A");
      deviceDetail->deviceType = strdup("Industrial Integrated Router");
      deviceDetail->deviceMake = strdup("Cisco Systems Inc.");
      deviceDetail->deviceModel = strdup("IR-829-GW");
      deviceDetail->deviceFirmwareVer = strdup("v1.3");
      deviceDetail->hardwareVer = strdup("v0.7");
      deviceDetail->macAddress = getMacAddress();
      deviceDetail->deviceSerialNum = strdup("FTX1942802A");
    }
  }
  return;
}


int main(int argc, char **argv)
{
  int c,loop_index=0,i=0;
  char* sdk_conf_File = NULL;
  char* dev_details_conf_File = NULL;
  IotDeviceSdkClient handle=NULL;
  DeviceDetails deviceDetail;
  char device_data_json[2048];
  struct timeval tv;
  float temp =0, hum =0;
  long time_mill_sec;

  printf("sampleApp - Starting  ++ \n");

  opterr = 0;
  while ((c = getopt (argc, argv, "c:d:t:")) != -1) {
    switch (c)
    {
      case 't':
        test_mode_loop = atoi(optarg);
        break;
      case 'c':
        sdk_conf_File = optarg;
        printf("sdk sdk_conf_File = %s \n", sdk_conf_File);
        break;
      case 'd':
        dev_details_conf_File = optarg;
        printf("device dev_details_conf_File = %s \n", dev_details_conf_File);
        break;
    }
  }

  /* Register for signals*/
  signal(SIGINT, exit_app_function);

  //Populate the device information into deviceDetail structure
  memset(&deviceDetail, 0, sizeof(DeviceDetails));
  populate_device_details(&deviceDetail, dev_details_conf_File);

  if(sdk_conf_File)
    deviceDetail.sdkConfigFile = sdk_conf_File;

  handle = iot_device_sdk_init_v1(&deviceDetail);
  if(!handle) {
    printf("sampleApp: iot_device_sdk_init() call failed \n");
    return 0;
  } else {
    printf("sampleApp: iot_device_sdk_init() call Success \n");

    //Set Log Level to Debug
    iot_device_sdk_setLogLevel(SDK_LOG_INFO);

    printf("## deviceDetail.skip_registration = %d \n", deviceDetail.skip_registration);

    if(deviceDetail.skip_registration) {
      printf("sampleApp: skipping Device Registration \n");
    } else {
      //iot_device_register - func implementation is in app device_registration.c file
      if(device_register(handle, &deviceDetail) < 0 )
        printf("sampleApp: Device Registration Failed\n");
    }

    /* Main While Loop */
    printf("sampleApp: Entering Application Main loop ++\n");
    while(1){
      gettimeofday(&tv, NULL);
      time_mill_sec = (tv.tv_sec) * 1000 + (tv.tv_sec) /1000;
      i++;
      if(i > 9.0){ i =  0 ; temp = 0; hum = 0;};
      temp = temp + i;
      hum = hum + (2*i);

      memset(device_data_json, 0, sizeof(device_data_json));


      sprintf(device_data_json, "{\"messages\": [{\"data\": {\"temperature\": %f, \"humidity\": %f }, \"ts\": %ld, \"format\": \"json\"}]}",
           temp, hum, time_mill_sec);

      printf("sampleApp: Loop : Accquire data -> serilaize data -> send data \n");
      if( iot_device_sdk_data_send_v1 (handle, device_data_json , sizeof(device_data_json), NULL, 0 ) < 0)
       printf("sampleApp: iot_device_sdk_data_send_v1 call failed \n");
      else
       printf("sampleApp: iot_device_sdk_data_send_v1 call Successful \n");

      sleep(3);

      //Check App exit Flag
      if(exit_flag) {
        break;
      }
      if(test_mode_loop && (loop_index >= test_mode_loop)) {
        break;
      }
      loop_index++;
    }
  }
  printf("sampleApp: Exited Application Main loop -- \n");

  iot_device_sdk_deinit_v1();

  //DeviceDetails
  if(deviceDetail.deviceType) free(deviceDetail.deviceType);
  if(deviceDetail.deviceMake) free(deviceDetail.deviceMake);
  if(deviceDetail.deviceModel) free(deviceDetail.deviceModel);
  if(deviceDetail.deviceFirmwareVer) free(deviceDetail.deviceFirmwareVer);
  if(deviceDetail.macAddress) free(deviceDetail.macAddress);
  if(deviceDetail.deviceSerialNum) free(deviceDetail.deviceSerialNum);
  if(deviceDetail.ipv4) free(deviceDetail.ipv4);
  if(deviceDetail.ipv6) free(deviceDetail.ipv6);
  if(deviceDetail.alt_id1) free(deviceDetail.alt_id1);

  printf("sampleAppApp Exiting --\n");
  return 0;
}
