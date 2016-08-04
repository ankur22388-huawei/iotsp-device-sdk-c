
/**************************************************************************
  copyright (C) Copyright 2015-2016 Cisco Systems Inc.; All Rights Reserved
  History:
    01/12/2016 - Created.
 ******************************************************************************/
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <time.h>
#include <iot_device_sdk.h>
#include <string.h>
#include "iot_device_sdk_defaults.h"

static int sdk_logLevel = DEFAULT_LOG_LEVEL;
static const char *logLevelStr[6] = {"SDK_MSG","SDK_NONE","SDK_ERROR","SDK_WARN","SDK_DEBUG","SDK_INFO"};
static FILE *logfp = NULL;
static char log_file[MAX_PATH_LEN]={0};

/**
* logging_set_filepath ( ) is used to set the log file path
* Input  : char * to file path
* return :  0 - success
*          -1 - Error
*/
int logging_set_filepath(const char *logFile)
{
  if (NULL == logFile) {
    fprintf(stderr, "%s() NULL logFile supplied\n", __FUNCTION__);
    return -1;
  }

  logfp = NULL;// clear the log stream
  sprintf(log_file, "%s", logFile); // set the logfile path
  return 0;
}


/**
 * logging_set_stream ( ) can be used to set the logging stream
 * Input  : FILE * to stream
 * return :  0 - success
 *          -1 - Error
*/
static int logging_set_stream(FILE *stream)
{
  if (NULL == stream) {
    fprintf(stderr, "%s() NULL stream supplied\n", __FUNCTION__);
    return -1;
  }
  log_file[0] = 0;// clear the logfile path
  logfp = stream; // set the log stream
  return 0;
}


/**
 * iot_device_sdk_setLogLevel ( ) - Function to set the Device SDK log level
 * logging level.
 * Default Logging Level : LOG_ERROR
 * Input  : Logging Level
 * return :  0 - success
 *          -1 - Error
*/
int iot_device_sdk_setLogLevel (int logLevel){
  if(logLevel < SDK_NO_LOGGING || logLevel > SDK_LOG_INFO) {
    iot_device_sdk_log(SDK_LOG_ERROR, "Unable to set the sdk logging Level - Invalid Logging level supplied\n");
    return -1;
  } else {
    sdk_logLevel = logLevel;
    iot_device_sdk_log(SDK_LOG_INFO,"iot_device_sdk_setLogLevel %d:%s\n", logLevel, logLevelStr[logLevel]);
  }
  return 0;
}


/**
 * iot_device_sdk_setLogFile ( ) - Function to set the Device SDK log file.
 * Input  : Log file
 * return :  0 - success
 *          -1 - Error
*/
int iot_device_sdk_setLogFile (const char *logFile)
{
  if (NULL == logFile) {
    iot_device_sdk_log(SDK_LOG_ERROR,"iot_device_sdk_setLogFile logFile passed in NULL \n");
    return -1;
  }

  /* set the logging file path */
  logging_set_filepath(logFile);

  /* if the log filepath is blank, route logging to default stderr stream */
  if (strlen(logFile) == 0) {
    logging_set_stream(stderr);
  }
  iot_device_sdk_log(SDK_LOG_WARN,"iot_device_sdk_setLogFile '%s'\n", logFile);
  return 0;
}


/**
 * iot_device_sdk_log ( ) Function to log debug/log messages
 * Input  : None
 * return :  0 - success
 *          -1 - Error
*/
int iot_device_sdk_log (int logLevel, const char* format, ...){
  if((logLevel < SDK_NO_LOGGING || logLevel > SDK_LOG_INFO) && logLevel != SDK_LOG_MSG) {
    fprintf(stderr, "Unable to log the message - Invalid Logging level supplied\n");
    return -1;
  }
  // if the message loglevel is above the currently set loglevel, don't log it
  if (logLevel > sdk_logLevel && logLevel != SDK_LOG_MSG) {
    return -1;
  }

  /* Add to log */
  time_t nowTime;
  struct tm *timeInfo;
  char timeBuffer[256];
  va_list args;

  time(&nowTime);
  timeInfo = localtime( &nowTime);
  strftime(timeBuffer, sizeof(timeBuffer), "%m-%d-%Y %H:%M:%S", timeInfo);
  FILE *fp = NULL; // points to a stream, or locally opened file
  int file_opened = 0; // flag to indicate if file has been opened here

  // If a valid stream has been set use it
  if(logfp) {
    fp = logfp;
  } else {
    assert(strlen(log_file) > 0);
    fp = fopen(log_file, "at");
    if (NULL == fp) {
      fprintf(stderr, "Unable to open log file[%s]\n", log_file);
      return -1;
    }
    file_opened = 1;
  }
  // we should be having a valid fp handle
  if(fp) {
    fprintf(fp, "[%s]:[%s]:", timeBuffer, logLevelStr[logLevel]);
    va_start(args, format);
    vfprintf(fp,format,args);
    va_end(args);
  }
  /* If a log file was opened here, close it's handle*/
  if (1 == file_opened) {
    fclose(fp);
    file_opened = 0;
  }
  return 0;
}
