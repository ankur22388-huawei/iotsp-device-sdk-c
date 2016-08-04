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
*             02/27/2016 - Added serial port support.
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
#include "serial.h"


volatile sig_atomic_t exit_flag = 0;

extern int RD_stop();

int check_app_exit_flag()
{
  return (int)exit_flag;
}
void exit_app_function( int sig )
{
  printf("sampleAppSerial exit_app_function sig = %d \n", sig);
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
          //printf("#### line = %s \n", line);
          token = strtok(line, sep);
          //printf("#### token1 = %s \n", token);
          if(!strcmp("manufacturingId", token)){
              token = strtok(NULL, sep);
              //printf("#### token2 = %s \n", token);
              deviceDetail->manufacturingId = strdup(token);
              remove_new_line(deviceDetail->manufacturingId);
          } else if(!strcmp("deviceType", token)){
              token = strtok(NULL, sep);
              //printf("#### token2 = %s \n", token);
              deviceDetail->deviceType = strdup(token);
              remove_new_line(deviceDetail->deviceType);
          } else if(!strcmp("deviceMake", token)){
              token = strtok(NULL, sep);
              //printf("#### token2 = %s \n", token);
              deviceDetail->deviceMake = strdup(token);
              remove_new_line(deviceDetail->deviceMake);
          } else if(!strcmp("deviceModel", token)){
              token = strtok(NULL, sep);
              //printf("#### token2 = %s \n", token);
              deviceDetail->deviceModel = strdup(token);
              remove_new_line(deviceDetail->deviceModel);
          } else if(!strcmp("deviceFirmwareVer", token)){
              token = strtok(NULL, sep);
              //printf("#### token2 = %s \n", token);
              deviceDetail->deviceFirmwareVer = strdup(token);
              remove_new_line(deviceDetail->deviceFirmwareVer);
          } else if(!strcmp("hardwareVer", token)){
              token = strtok(NULL, sep);
              //printf("#### token2 = %s \n", token);
              deviceDetail->hardwareVer = strdup(token);
              remove_new_line(deviceDetail->hardwareVer);
          } else if(!strcmp("macAddress", token)){
              token = strtok(NULL, sep);
              //printf("#### token2 = %s \n", token);
              deviceDetail->macAddress = strdup(token);
              remove_new_line(deviceDetail->macAddress);
          } else if(!strcmp("deviceSerialNum", token)){
              token = strtok(NULL, sep);
              //printf("#### token2 = %s \n", token);
              deviceDetail->deviceSerialNum = strdup(token);
              remove_new_line(deviceDetail->deviceSerialNum);
          } else if(!strcmp("ipv4", token)){
              token = strtok(NULL, sep);
              //printf("#### token2 = %s \n", token);
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
      deviceDetail->ipv4 = getipv4_Address() ;
      //deviceDetail->ipv4 = strdup("10.194.30.234") ;
    }
  }
  return;
}


int main(int argc, char **argv)
{
  int c=0, ret = -1;
  //float i = 0;
  //float temp =0, hum =0;
  long time_mill_sec;
  char device_data_json[2048];

  char* sdk_conf_File = NULL;
  char* dev_details_conf_File = NULL;
  DeviceDetails deviceDetail;
  struct timeval tv;

  printf("sampleApp - Starting  ++ \n");
  opterr = 0;
  while ((c = getopt (argc, argv, "c:d:")) != -1) {
    switch (c)
    {
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
 // signal(SIGINT, exit_app_function);
  signal(SIGTERM, exit_app_function);

  //Populate the device information into deviceDetail structure
  memset(&deviceDetail, 0, sizeof(DeviceDetails));
  populate_device_details(&deviceDetail, dev_details_conf_File);

  if(sdk_conf_File)
    deviceDetail.sdkConfigFile = sdk_conf_File;

  printf("## deviceDetail.skip_registration = %d \n", deviceDetail.skip_registration);

  RD_start();

  /* Main While Loop */
  printf("sampleApp: Entering Application Main loop ++\n");
  while(1){
    printf("sampleApp: Loop : Accquire data -> serilaize data -> send data \n");
    // DEMO  -3 payload
    //update json payload
    gettimeofday(&tv, NULL);
    time_mill_sec = (tv.tv_sec) * 1000 + (tv.tv_sec) /1000;
#if 0
    i++;
    if(i > 9.0){ i =  0 ; temp = 0; hum = 0;};
    temp = temp + i;
    hum = hum + (2*i);
#else

    Sensor_Data SensorData = {{0}};
    ret = getDfSensorData(&SensorData);
    if (ret == 0)
#endif
    {
        df_Sensor_Data *tmp = (df_Sensor_Data*)(&SensorData);
        sprintf(device_data_json, "{\"messages\": [{\"data\": {\"temperature\": %f, \"humidity\": %f }, \"ts\": %ld, \"format\": \"json\"}]}",
           tmp->temperature, tmp->humidity, time_mill_sec);

        printf("sampleApp devicedata : %s \n size = %d \n", device_data_json, (int)sizeof(device_data_json));
        ret = iot_device_sdk_data_send_v2 (&deviceDetail, device_data_json , sizeof(device_data_json), NULL, 0);
        if( ret < 0)
          printf("sampleApp: iot_device_sdk_data_send_v2 call failed \n");
        else
          printf("sampleApp: iot_device_sdk_data_send_v2 call Successful \n");
    }
    sleep(3);
    //Accquire Device Data

    //Serilize Device Data

    //Submit/Post Device Data

    //Check App exit Flag
    if(exit_flag) break;

    //Send keep-alive to CPOD

  }

  printf("sampleApp: Exited Application Main loop -- \n");

  RD_stop();
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
