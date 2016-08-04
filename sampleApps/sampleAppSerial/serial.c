/**************************************************************************
  copyright (C) Copyright 2015-2016 Cisco Systems Inc.; All Rights Reserved
  Author : Xiangqin Wen 
           wxiangqi@cisco.com
  History:
          01/12/2016 Xiangqin Wen - Created.
 ******************************************************************************/

#include <stdio.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "serial.h"

#define BAUDRATE                 B9600
#define UART_DEVICE              "/dev/ttyS1"
#define COUTINV                  000100000000     /* RTS inversion*/ 
#define FALSE                    0
#define TRUE                     1

#define DEV_ADDRESS              2
#define RETRY_MAX                5

#define OP_WRITE                 1
#define OP_READ                  2
       

//#define __DEBUG_THIS__
#ifdef __DEBUG_THIS__
#define LOG(fmt, args...)  fprintf(stderr, fmt,##args)
#else
#define LOG(args...)       
#endif


#define true 1
#define false 0 

#if 0

#define SYNC_DATA 0xfefefefe

static volatile int              wait_updates = true;
static volatile int              config_change = 0;
static struct sigaction          sigact;
static pthread_t                 serial_thread_id = 0;  
char global_ip[64] = "171.68.114.116";


typedef struct mbed_input_t
{
    double temperature;
    double Potentiometer_1;
    double Potentiometer_2;
    char   orientation[8];
    char   rotation[8];
    int    pad[5];
}mbed_Sensor_Data;


static void signal_handler(int signum, siginfo_t *info, void *ptr)
{
    if(signum == SIGUSR1)
    {
        config_change = 1;
    }
    else {
	wait_updates = false;
    }
}

static int get_mbed_SensorData(Sensor_Data *SensorDataPtr)
{
    if (SensorDataPtr == NULL)
		return -1;
    mbed_Sensor_Data *tmp = (mbed_Sensor_Data*)SensorDataPtr; 
    LOG("temperature %f\n", tmp->temperature);
    LOG("Potentiometer_1 %f\n", tmp->Potentiometer_1);
    LOG("Potentiometer_2 %f\n", tmp->Potentiometer_2);
    LOG("orientation %s\n", tmp->orientation);
    LOG("rotation %s\n", tmp->rotation);
}


static int publish_mbed_SensorData(Sensor_Data *SensorDataPtr)
{
    if (SensorDataPtr == NULL)
		return -1;
    mbed_Sensor_Data *tmp = (mbed_Sensor_Data*)SensorDataPtr;
    char cmd[512] = {0};
    sprintf(cmd, "export LD_LIBRARY_PATH=%s:$LD_LIBRARY_PATH; %s/mosquitto_pub -h %s -t mbed\
    -m \"{\\\"temperature\\\":%f,\\\"Potentiometer_1\\\":%f,\\\"Potentiometer_2\\\":%f,\
\\\"orientation\\\":\\\"%s\\\",\\\"rotation\\\":\\\"%s\\\"}\"\n", 
    getenv("CAF_APP_PATH"), getenv("CAF_APP_PATH"), global_ip, tmp->temperature, 
    tmp->Potentiometer_1, tmp->Potentiometer_2, tmp->orientation, tmp->rotation);
    LOG("%s\n", cmd);
    system(cmd);
}

static int publish_dfRobot_SensorData(Sensor_Data *SensorDataPtr)
{
    if (SensorDataPtr == NULL)
		return -1;
    df_Sensor_Data *tmp = (df_Sensor_Data*)SensorDataPtr;
    char cmd[512] = {0};
    sprintf(cmd, "export LD_LIBRARY_PATH=%s:$LD_LIBRARY_PATH; %s/mosquitto_pub -h %s -t dfRobot\
    -m \"{\\\"temperature\\\":%f,\\\"humidity\\\":%f}\"\n", 
    getenv("CAF_APP_PATH"), getenv("CAF_APP_PATH"), global_ip, tmp->temperature, tmp->humidity);
    LOG("%s\n", cmd);
    system(cmd);
}
#endif


/* Get dfrobot sensor data from RS485 */
static int ShowRDText(const char *text, int len)
{
    int            i,fd;
    //int            Done,count;
    //int            status;
    struct termios oldtio,newtio;

    if ((text == NULL))
        return -1;
    LOG("=========================================================\n");
    LOG("Retrieve data from sensors RS-485 interface address: %x\n",deviceAddress);

    //initialize serial port

    //fd = open(DEVICE, O_RDWR | O_NOCTTY | O_NONBLOCK); 
    fd = open(UART_DEVICE, O_RDWR | O_NOCTTY); 
    if (fd <0) {perror(UART_DEVICE); exit(-1); }

    tcgetattr(fd,&oldtio); /* save current port settings */

    bzero(&newtio, sizeof(newtio));
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;
    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD | COUTINV;
    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 25;   /* blocking read until 1 bytemperature is received */

    tcflush(fd, TCIOFLUSH);
    tcsetattr(fd,TCSANOW,&newtio);

    for (i=0;i<len;i++)
    {
      //LOG("Sending cmd %x, size %d \n",cmd[i],sizeof(cmd[i]));
      write(fd, &text[i], sizeof(text[i]));
      //LOG("Result: Bytes senti : %d \n",res);
      usleep(150);
    }
    sleep(1); 
    // restore old setting
    tcsetattr(fd,TCSANOW,&oldtio);
    close(fd); 
    
    return 0;
}

int RD_start()
{
    const char *text = "*11\rfalCON\r~~~O\r~~~O\rfalCON\r";
    ShowRDText(text, strlen(text));
    return 0;
}

int RD_stop()
{
    const char *text = "*11\riotsdk\r~~~G\r~~~G\riotsdk\r";
    ShowRDText(text, strlen(text));
    return 0;
}


/* Get dfrobot sensor data from RS485 */
int getDfSensorData(Sensor_Data *SensorDataPtr)
{
    char           chksum=0;
    //char           rchksum=0;
    char           recbuf[255];
    int            deviceAddress = DEV_ADDRESS;
    unsigned char           humidtyLow = 0,humidtyHigh = 0,temperatureLow = 0,temperatureHigh = 0;
    //char           serialport[16]=UART_DEVICE;
    //int            inkey;
    int            i,fd, res,brcvd;
    int            humidity = 0,temperature = 0;
    //int            Done,count;
    //int            status;
    unsigned char  cmd[7];
    struct termios oldtio,newtio;
    df_Sensor_Data *df_Sensor_DataPtr = (df_Sensor_Data*)SensorDataPtr;

    if ((SensorDataPtr == NULL))
        return -1;
    memset(recbuf, 0, sizeof(recbuf));
    LOG("=========================================================\n");
    LOG("Retrieve data from sensors RS-485 interface address: %x\n",deviceAddress);

    //initialize serial port

    //fd = open(DEVICE, O_RDWR | O_NOCTTY | O_NONBLOCK); 
    fd = open(UART_DEVICE, O_RDWR | O_NOCTTY); 
    if (fd <0) {perror(UART_DEVICE); exit(-1); }

    tcgetattr(fd,&oldtio); /* save current port settings */

    bzero(&newtio, sizeof(newtio));
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;
    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD | COUTINV;
    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 25;   /* blocking read until 1 bytemperature is received */

    tcflush(fd, TCIOFLUSH);
    tcsetattr(fd,TCSANOW,&newtio);

    // assemble the query command
    cmd[0]=0x78;
    cmd[1]=0x55;
    cmd[2]=0xAA;
    cmd[3]=0x00 + deviceAddress;
    cmd[4]=0x00;
    cmd[5]=0x21;

    chksum=0;
    // calculatemperature the checksum
    for(i=1;i<6;i++)
      chksum += cmd[i];
    cmd[6]=chksum;

    // send the query command
    LOG("Sending Command :Header  %x  %x | Device address %x | framelen %x |  commandword %x, checksum %x \n",cmd[1],cmd[2],cmd[3],cmd[4],cmd[5],cmd[6]);
    res=0;
    for (i=1;i<7;i++)
    {
      res=0;
      //LOG("Sending cmd %x, size %d \n",cmd[i],sizeof(cmd[i]));
      res = write(fd, &cmd[i], sizeof(cmd[i]));
      //LOG("Result: Bytes senti : %d \n",res);
      usleep(100);
    }
    LOG("Starting Read..\n");
    brcvd=0;
    int try = 0;
    while(brcvd < 25 && (try < RETRY_MAX))  
    {
       res=0;
       /* FIXME if the cmd is not correct, the system won't return */
       res=read(fd,&recbuf[brcvd],255);
       try++;
       LOG("Read %d bytes\n",res);
       usleep(10000);
// need to fix this, will fail if read returns less than 25
         //for (i=0;(i<res);i++)
           //LOG("0x%x\n", recbuf[i],recbuf[i]);
       brcvd += res;
    }
    //for (i=0;(i<brcvd);i++)
      //LOG("0x%x\n", recbuf[i],recbuf[i]);

    humidtyHigh =recbuf[7];
    humidtyLow =recbuf[8];
    humidity   = humidtyHigh;
    humidity   =(humidity * 256) | humidtyLow ;
    LOG("humidtyLow = %x, humidtyHigh = %x, humidity =%x %d\n", humidtyLow,humidtyHigh,humidity,humidity);
    temperatureHigh =recbuf[9];
    temperatureLow =recbuf[10];
    temperature   = temperatureHigh;
    temperature   =(temperature * 256) | temperatureLow ;
    LOG("temperatureLow = %x, temperatureHigh = %x, temperature =%x %d\n", temperatureLow,temperatureHigh,temperature,temperature);
    LOG("Sensor Data ..\n");
    LOG("Humidity    : %2d\% \n",(humidity/10));
    LOG("Temperature : %2d C / %2d F \n",(temperature/10),((((temperature/10)*9)/5)+32));
    int humidStandard = (humidity/10);
    int tempStandard = ((((temperature/10)*9)/5)+32);
    LOG("HUM:%2d:TEM:%2d\n",humidStandard, tempStandard);
    LOG("=========================================================\n");

    // restore old setting
    tcsetattr(fd,TCSANOW,&oldtio);
    close(fd); 
    
    df_Sensor_DataPtr->temperature = tempStandard;
    df_Sensor_DataPtr->humidity = humidStandard;

    return 0;
}

#if 0

/* Send data to RD6 over RS485 */
static int setRDData(int direction, Sensor_Data *SensorDataPtr)
{
    char           chksum=0;
    char           rchksum=0;
    char           recbuf[32];
    unsigned char           humidtyLow = 0,humidtyHigh = 0,temperatureLow = 0,temperatureHigh = 0;
    char           serialport[16]=UART_DEVICE;
    int            inkey;
    int            i,c,fd, res,brcvd;
    int            humidity = 0,temperature = 0;
    int            Done,count;
    int            status;
    unsigned char  cmd[10];
    struct termios oldtio,newtio;
    df_Sensor_Data *df_Sensor_DataPtr = (df_Sensor_Data*)SensorDataPtr;

    if ((SensorDataPtr == NULL))
        return -1;
    memset(recbuf, 0, sizeof(recbuf));
    LOG("=========================================================\n");

    //initialize serial port

    //fd = open(DEVICE, O_RDWR | O_NOCTTY | O_NONBLOCK); 
    fd = open(UART_DEVICE, O_RDWR | O_NOCTTY); 
    if (fd <0) {perror(UART_DEVICE); exit(-1); }

    tcgetattr(fd,&oldtio); /* save current port settings */

    bzero(&newtio, sizeof(newtio));
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;
    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD | COUTINV;
    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 25;   /* blocking read until 1 bytemperature is received */

    tcflush(fd, TCIOFLUSH);
    tcsetattr(fd,TCSANOW,&newtio);

    // assemble the query command
    cmd[1]='*';
    cmd[2]='1';
    cmd[3]='1';
    cmd[4]='\r';
    cmd[5]='2';
    cmd[6]='3';
    cmd[7]='4';
    cmd[8]='5';
    cmd[9]='\r';

    chksum=0;

    // send the query command
    LOG("Sending Command :%c  %x | Device address %x |  %x  %c %c %c %c %x\n",cmd[1],cmd[2],cmd[3],cmd[4],cmd[5],cmd[6], cmd[7],cmd[8],cmd[9]);
    res=0;
    for (i=1;i<10;i++)
    {
      res=0;
      //LOG("Sending cmd %x, size %d \n",cmd[i],sizeof(cmd[i]));
      res = write(fd, &cmd[i], sizeof(cmd[i]));
      //LOG("Result: Bytes senti : %d \n",res);
      usleep(100);
    }
    LOG("end writing..\n");

    return 0;
}

/* Get mbed sensor data from RS232 */
static int SerialSensorData(int direction, Sensor_Data *SensorDataPtr)
{
    char           chksum=0;
    char           rchksum=0;
    char           *tmp = NULL;
    char           humidtyLow,humidtyHigh,temperatureLow,temperatureHigh;
    char           serialport[16]=UART_DEVICE;
    int            inkey;
    int            i,c,fd, res,brcvd;
    int            humidity,temperature;
    int            Done,count;
    int            status;
    unsigned char           buf_size = 0;
    unsigned char           tmp_size = 0;
    struct termios oldtio,newtio;
    Sensor_Data    local_data ;
    int            ret = 0;

    if ((SensorDataPtr == NULL))
        return -1;

    //initialize serial port
    //LOG("Starting %x... \n", direction);

    //fd = open(UART_DEVICE, O_RDWR | O_NOCTTY | O_NONBLOCK); 
    fd = open(UART_DEVICE, O_RDWR | O_NOCTTY); 
    if (fd <0) {
            LOG("Open device error... \n");
	    perror(UART_DEVICE); exit(-1); 
    }

    tcgetattr(fd,&oldtio); /* save current port settings */

    bzero(&newtio, sizeof(newtio));
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;
    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD | COUTINV;
    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 1;   /* blocking read until 1 bytemperature is received */

    tcflush(fd, TCIOFLUSH);
    tcsetattr(fd,TCSANOW,&newtio);

    chksum=0;
    tmp = (char *)&local_data;
    
    // Extra one size byte and one checksum
    buf_size = BUF_SIZE;
    
    int sync = SYNC_DATA;
    char *tmp_ptr = &sync;
    unsigned char send_len = 0;
    int k = 0;
    
    if (direction == OP_WRITE) 
    {
        LOG("Writing... \n");
#if 1    
	for (k = 0; k < sizeof(sync); k++)
        {          
		res = write(fd, &(tmp_ptr[k]), sizeof(tmp_ptr[k]));
                usleep(100000);
                //LOG("%x ", tmp_ptr[k]);
	}
        tmp = (char *)SensorDataPtr;
        send_len = sizeof(mbed_Sensor_Data);
	// calculatemperature the checksum
        for(i = 0;i < send_len;i++)
          chksum += tmp[i];

        res = write(fd, &send_len, sizeof(send_len));
                    LOG("\nlen %x \n", send_len);
        usleep(100000);

        for (i = 0;i < send_len;i++)
        {
          res=0;
          res = write(fd, &tmp[i], sizeof(tmp[i]));
          //LOG("%c ", tmp[i]);
          usleep(100000);
        }
        
        res = write(fd, &chksum, sizeof(chksum));
        usleep(100000);
#endif
	//char test_buf[128] = "this is a small world";
	//write(fd, test_buf, sizeof(test_buf));
        LOG("End sending..., chksum %x \n", chksum);
    }
    else
    {
        LOG("Reading... \n");
#if 1
        char tmp_sync = 0;
        int sync_len = 1;
	do {
	    do {
                res = read(fd, &tmp_sync, sizeof(tmp_sync));
                //LOG("%c ", tmp_sync);
	    }while (tmp_sync != (char)(sync));
            for (k = 1; k < sizeof(sync); k++)
	    {
                res = read(fd, &tmp_sync, sizeof(tmp_sync));
                //LOG("%c ", tmp_sync);
		if (tmp_sync == tmp_ptr[k])
		{
		    sync_len++;
		}
		else
		{
		    sync_len = 1;
		}
	    }
	}while (sync_len < sizeof(sync));
#endif        
        res = read(fd, &tmp_size, sizeof(tmp_size));
	//LOG("Receiving %d... \n", tmp_size);

	if (buf_size < tmp_size)
	{
	    printf("Error: tmp_size %d, bigger than buffer size %d\n",tmp_size, buf_size);
	    ret = -1;
	    goto Serial_Done;
	}

        int payload_len = tmp_size;

        LOG("Receiving %d data... \n", payload_len);

        for (i = 0;i < payload_len;i++)
        {
          res = read(fd, &tmp[i], sizeof(tmp[i]));
        }
        
	//LOG("Receiving chksum \n");
        
        res = read(fd, &chksum, sizeof(chksum));
	
	// calculatemperature the checksum
        for(i = 0;i < payload_len;i++)
	{
	      	rchksum += tmp[i];
		//LOG("%c", tmp[i]);
	}
	
	if (rchksum != chksum)
	{
	    printf("Error: rchksum %d, should be %d\n",rchksum, chksum);
	    ret = -1;
	    goto Serial_Done;
	}

	memset(SensorDataPtr, 0, sizeof(Sensor_Data));
	memcpy(SensorDataPtr, &local_data, sizeof(Sensor_Data));
    
    
    }
    
Serial_Done:    
    
    // restore old setting
    tcsetattr(fd,TCSANOW,&oldtio);
    close(fd); 
   
    return ret;
}


static int Get_Sensor_Data()
{
    int ret = 0;
    int i = 0;
    int polling_sensor = 0;

    Sensor_Data SensorData = {0};
    LOG("\n#####################test#################################\n"); 
    //ret = SerialSensorData(OP_READ, &SensorData);
    ret = getDfSensorData(OP_READ, &SensorData);
    //ret = setRDData(OP_READ, &SensorData);
#if 1
    if (ret == 0) 
    {
	    //get_mbed_SensorData(&SensorData);
	    //publish_mbed_SensorData(&SensorData);
	    publish_dfRobot_SensorData(&SensorData);
            //ret = SerialSensorData(OP_WRITE, &SensorData); 
    }
#endif
    return ret; 
}

void* serial_proc(void* thread) 
{
   do {
       Get_Sensor_Data();
       sleep(1);
   } while(wait_updates);
}

int main(int argc, char **argv)
{
	memset(&sigact, 0, sizeof(sigact));
	sigact.sa_sigaction = signal_handler;
	sigaction(SIGTERM, &sigact, NULL);
	sigaction(SIGINT, &sigact, NULL);
	sigaction(SIGABRT, &sigact, NULL);
	sigaction(SIGUSR1, &sigact, NULL);

        if (argc > 1)
	{
	    strncpy(global_ip, argv[1], sizeof(global_ip));
	}

	if (serial_thread_id == 0)
	{
	        pthread_create(&serial_thread_id, 0, &serial_proc, 0);
	}
	else
	{
           pthread_cancel(serial_thread_id);
           serial_thread_id = 0;	   
    	}	
    	
	while (wait_updates){
	    sleep(1);
	}
    	
        if (serial_thread_id != 0)
	{
           pthread_cancel(serial_thread_id);
           serial_thread_id = 0;	   
	}	

Cleanup:
	return 0;
}
#endif
