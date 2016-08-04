/**************************************************************************
  copyright (C) Copyright 2015-2016 Cisco Systems Inc.; All Rights Reserved
  Author : Xiangqin Wen 
           wxiangqi@cisco.com
  History:
          01/12/2016 Xiangqin Wen - Created.
 ******************************************************************************/

#ifndef __IOT_SDK_SERIAL_H__
#define __IOT_SDK_SERIAL_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define BUF_SIZE                 128 


typedef struct _Sensor_Data
{
    char     channel_value[BUF_SIZE];
}Sensor_Data;

typedef struct df_input_t
{
    double temperature;
    double humidity;
}df_Sensor_Data;

int getDfSensorData(Sensor_Data *SensorDataPtr);
int RD_start();
int RD_start();

#ifdef __cplusplus
}
#endif

#endif /* __IOT_SDK_SERIAL_H__ */
