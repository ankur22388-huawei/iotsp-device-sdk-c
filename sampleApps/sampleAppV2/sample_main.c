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

volatile sig_atomic_t exit_flag = 0;



int check_app_exit_flag()
{
  return (int)exit_flag;
}
void exit_app_function( int sig )
{
  printf("sampleAppV2 exit_app_function sig = %d \n", sig);
  exit_flag = 1;
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
      deviceDetail->manufacturingId = strdup("device-sdk-c-12345");
      deviceDetail->deviceType = strdup("Industrial Integrated Router");
      deviceDetail->deviceMake = strdup("Cisco Systems Inc.");
      deviceDetail->deviceModel = strdup("IR-829-GW");
      deviceDetail->deviceFirmwareVer = strdup("v1.3");
      deviceDetail->hardwareVer = strdup("v0.7");
      deviceDetail->ipv4 = getipv4_Address() ;
    }
  }
  return;
}


int main(int argc, char **argv)
{
  int c=0, ret = -1, test_mode_loop=0,loop_index=0;
  float i = 0;
  float temp =0, hum =0;
  long time_mill_sec;
  char device_data_json[2048];

  char* sdk_conf_File = NULL;
  char* dev_details_conf_File = NULL;
  DeviceDetails deviceDetail;
  struct timeval tv;

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
        printf("sdk confFile1 = %s \n", sdk_conf_File);
        break;
      case 'd':
        dev_details_conf_File = optarg;
        printf("device confFile2 = %s \n", dev_details_conf_File);
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

  printf("## deviceDetail.skip_registration = %d \n", deviceDetail.skip_registration);

  /* Main While Loop */
  printf("sampleApp: Entering Application Main loop ++\n");
  while(1){
    printf("sampleApp: Loop : Accquire data -> serilaize data -> send data \n");
    // DEMO  -3 payload
    //update json payload
    gettimeofday(&tv, NULL);
    time_mill_sec = (tv.tv_sec) * 1000 + (tv.tv_sec) /1000;
    i++;
    if(i > 9.0){ i =  0 ; temp = 0; hum = 0;};
    temp = temp + i;
    hum = hum + (2*i);

    memset(device_data_json, 0, sizeof(device_data_json));
    sprintf(device_data_json, "{\"messages\": [{\"data\": {\"temperature\": %f, \"humidity\": %f }, \"ts\": %ld, \"format\": \"json\"}]}",
           temp, hum, time_mill_sec);

    printf("sampleApp devicedata : %s \n size = %d \n", device_data_json, (int)sizeof(device_data_json));
    ret = iot_device_sdk_data_send_v2 (&deviceDetail, device_data_json , sizeof(device_data_json),"tag1", sizeof("tag1"));
    if( ret < 0)
      printf("sampleApp: iot_device_sdk_data_send_v2 call failed \n");
    else
      printf("sampleApp: iot_device_sdk_data_send_v2 call Successful \n");

    sleep(3);
    //Accquire Device Data

    //Serilize Device Data

    //Submit/Post Device Data

    //Check App exit Flag
    if(exit_flag) {
      break;
    }
    if(test_mode_loop && (loop_index >= test_mode_loop)) {
      break;
    }
    loop_index++;
    //Send keep-alive to CPOD

  }

  printf("sampleApp: Exited Application Main loop -- \n");

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
