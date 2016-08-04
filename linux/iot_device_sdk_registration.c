/**************************************************************************
  copyright (C) Copyright 2015-2016 Cisco Systems Inc.; All Rights Reserved
  History:
           01/12/2016 - Created.
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
/* platform specific */
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

#include <iot_device_sdk.h>
#include "iot_device_sdk_registration.h"
#include "iot_device_sdk_helper.h"

#define SAFE_FREE(x) if ((x) != NULL) { free(x); x=NULL; }


#define MAX_PRIMARY_REG_RETRIES 10
#define MAX_CPOD_REG_RETRIES 10

#define ENABLE_PRIMARY_REG 1 // Set to 0 to disable Primary server registration step. 1 to enable
#define ENABLE_CPOD_REG    1 // Set to 0 to disable CPOD server registration step. 1 to enable

#define DEFAULT_RETRY_INTERVAL 10
#define DELAY_BETWEEN_RETRY 5

static int cpod_reg_status = 0;
static long keep_alive_interval = 0;

pthread_t ka_thread;


static void PrintPrimarySvrRegistrationResponse(const SDKRegistrationResponse* regResponse)
{
  iot_device_sdk_log(SDK_LOG_INFO," %s ++ \n", __FUNCTION__);

  if (regResponse) {
    iot_device_sdk_log(SDK_LOG_INFO,"IOT_SDK :\n"
    "  ----------------------------\n"
    "  responseCode(%d)\n"
    "  response(%s)\n"
    "  claimed(%d)\n"
    "  registered(%d)\n"
    "  security(%s)\n"
    "  interval(%s)\n"
    "  protocol(%s)\n"
    "  registrationURI(%s)\n"
    "  ----------------------------\n",
    regResponse->responseCode,
    regResponse->response, regResponse->claimed,
    regResponse->registered,regResponse->security,
    regResponse->interval, regResponse->protocol,
    regResponse->registrationURI);
  }

  iot_device_sdk_log(SDK_LOG_INFO," %s -- \n", __FUNCTION__);
}

static void PrintCPODSvrRegistrationResponse(const SDKRegistrationResponse* regResponse)
{
  iot_device_sdk_log(SDK_LOG_INFO," %s ++ \n", __FUNCTION__);
  if (regResponse) {
    iot_device_sdk_log(SDK_LOG_INFO,"IOT_SDK :\n"
    "  ----------------------------\n"
    "  responseCode(%d)\n"
    "  response(%s)\n"
    "  claimed(%d)\n"
    "  registered(%d)\n"
    "  security(%s)\n"
    "  interval(%s)\n"
    "  protocol(%s)\n"
    "  registrationURI(%s)\n"
    "  ----------------------------\n",
    regResponse->responseCode,
    regResponse->response, regResponse->claimed,
    regResponse->registered,regResponse->security,
    regResponse->interval, regResponse->protocol,
    regResponse->registrationURI);
  }
  iot_device_sdk_log(SDK_LOG_INFO," %s -- \n", __FUNCTION__);
}

#if 0
static void PrintAccessTokenDetails(const AccessTokenDetails* accessTokenDetails)
{
  iot_device_sdk_log(SDK_LOG_INFO," %s ++ \n", __FUNCTION__);
  if (accessTokenDetails) {
    iot_device_sdk_log(SDK_LOG_INFO,"IOT_SDK :\n"
    "  ----------------------------\n"
    "  response(%s)\n"
    "  scope(%s)\n"
    "  expires_in(%d)\n"
    "  token_type(%s)\n"
    "  refresh_token(%s)\n"
    "  access_token(%s)\n"
    "  ----------------------------\n",
    accessTokenDetails->response,
    accessTokenDetails->scope,
    accessTokenDetails->expires_in,
    accessTokenDetails->token_type,
    accessTokenDetails->refresh_token,
    accessTokenDetails->access_token);
  }
  iot_device_sdk_log(SDK_LOG_INFO," %s -- \n", __FUNCTION__);
}
#endif

static int sendKeepAlive_CPODServer(IotDeviceSdkClient handle)
{
  int error_code = 0;

  iot_device_sdk_log(SDK_LOG_INFO,"sendKeepAlive_CPODServer ++\n");
  const SDKRegistrationResponse* regResponse =
    (SDKRegistrationResponse*)iot_dsdk_sendRegistrationMessage_v1(handle, &error_code);
  PrintCPODSvrRegistrationResponse(regResponse);

  if (NULL == regResponse) { // error case
    iot_device_sdk_log(SDK_LOG_INFO,"IOT_SDK : CPOD server Contact Error error_code[%d]\n", error_code);

  }
  return 0;
}


static int device_keep_alive_thread(void *arg)
{

  IotDeviceSdkClient handle = *(IotDeviceSdkClient*)(arg);

  while(1) {
    if(cpod_reg_status){
      iot_device_sdk_log(SDK_LOG_INFO,"IOT_SDK : %s keep_alive_thread - device  registered - keep alive interval = %ld  \n", __FUNCTION__, keep_alive_interval);
      if(keep_alive_interval < 0) {
        iot_device_sdk_log(SDK_LOG_WARN,"IOT_SDK : %s keep_alive_thread - INVALID keep alive interval = %ld  \n", __FUNCTION__, keep_alive_interval);
      } else {
        sdk_local_sleep(SECONDS, keep_alive_interval);
        //Check App Wants to exit
        //if(check_app_exit_flag()) break;
        //send keep alive reg message
        if(handle)
          sendKeepAlive_CPODServer(handle);
      }

    } else {
      iot_device_sdk_log(SDK_LOG_INFO,"IOT_SDK : %s keep_alive_thread - device not yet registered \n", __FUNCTION__);
      sdk_local_sleep(SECONDS, 10);
    }
  }
  return 0;
}

int spawn_keep_alive_loop(IotDeviceSdkClient handle)
{
  /* Create worker thread */
  int ret = pthread_create(&ka_thread, NULL, (void*)&device_keep_alive_thread, &handle);
  if (ret !=0) {
    iot_device_sdk_log(SDK_LOG_ERROR,"IOT_SDK : %s Error : Could not create thread!\n", __FUNCTION__);
  }
  return ret;
}


/**
 * @brief GetRetryInterval
 * GetRetryInterval ( ) Function to read retry interval
 * @param[in]  : Pointer to registration response structure
 * @return     : value in seconds
 *               -1 on Error.
*/
static long GetRetryInterval(const SDKRegistrationResponse* regResponse)
{
  char *ptr;
  long ret;
  /* working off a sample value is '1s'. If format changes, update this function */
  if (NULL == regResponse || NULL == regResponse->interval || 0 == strlen(regResponse->interval)) {
    return -1;
  }
  ret = strtol(regResponse->interval, &ptr, 10);
  iot_device_sdk_log(SDK_LOG_INFO,"IOT_SDK GetRetryInterval regResponse->interval'%s' ret=%ld \n", regResponse->interval, ret);

  return ret;
}



static void PrintDemarcator(int regStep)
{
  switch(regStep) {
  case REG_PRIMARY:
    iot_device_sdk_log(SDK_LOG_INFO,"\nIOT_SDK ==== PRIMARY ======== PRIMARY ========\n");
    break;
  case REG_CPOD:
    iot_device_sdk_log(SDK_LOG_INFO,"\nIOT_SDK ==== CPOD ======== CPOD ========\n");
    break;
  case REG_IAM:
    iot_device_sdk_log(SDK_LOG_INFO,"\nIOT_SDK ==== IAM ======== IAM ========\n");
    break;
  default:
    iot_device_sdk_log(SDK_LOG_INFO,"IOT_SDK :Unknown REG step [%d]\n", regStep);
    assert(0);
  }
}


/**
* RegisterWithPrimaryServer( ) - Function to communicate with the Primary server
* Input  :
* return :  0 - success
*          -1 - Error
*/
static int RegisterWithPrimaryServer(IotDeviceSdkClient handle)
{
  long retry_interval = 0;
  int regResponseCode = 0;
  int registered = 0; // whether registered with server
  unsigned int reg_attempt =1;

#if ENABLE_PRIMARY_REG
  /* Loop for MAX_PRIMARY_REG_RETRIES times, till a successful registration
   * occurs*/
  //do {
    PrintDemarcator(REG_PRIMARY);
    iot_device_sdk_log(SDK_LOG_INFO,"IOT_SDK Primary server Registration attempt '%d'\n", reg_attempt);
    const SDKRegistrationResponse* regResponse =
        iot_dsdk_sendPrimaryRegistrationMessage_v1(handle);
    if (NULL == regResponse) { // error case
      iot_device_sdk_log(SDK_LOG_INFO,"IOT_SDK Primary server Contact Error \n");
    }
    else {

      assert(regResponse);
      regResponseCode = regResponse->responseCode;
      registered = regResponse->registered;
      iot_device_sdk_log(SDK_LOG_INFO,"IOT_SDK : Primary server registration response code(%d)registered(%d)\n",
        regResponseCode, registered);

      // if registration is successful, break out of loop
      if (1 == registered) {
        iot_device_sdk_log(SDK_LOG_INFO,"IOT_SDK : Primary server Registration Successful !\n");
        // print the primary server registration response
        PrintPrimarySvrRegistrationResponse(regResponse);
        return 0;
        //break;
      } else {
        iot_device_sdk_log(SDK_LOG_ERROR,"IOT_SDK : Primary server Registration Un-Successful !\n");
        // print the primary server registration response
        PrintPrimarySvrRegistrationResponse(regResponse);
      }
      // registration unsuccessful, retry using a provided, or default interval
      int numErrs = regResponse->numErrors;
      if (numErrs > 0) {
        int err_idx = 0;
        SDKError *err_node = regResponse->errors;
        for (; err_idx < numErrs; err_idx++) {
          iot_device_sdk_log(SDK_LOG_INFO,"IOT_SDK : err(%d '%s' '%s') \n",err_idx, err_node->err_tag, err_node->err_value );
          err_node = err_node->next;
        }
      }
      retry_interval = GetRetryInterval(regResponse);
      if (-1 == retry_interval) {
        iot_device_sdk_log(SDK_LOG_INFO,"IOT_SDK : Invalid retry interval '%s'\n", regResponse->interval);
        retry_interval = DEFAULT_RETRY_INTERVAL;
      }
    }

    reg_attempt++;
    if (reg_attempt >= MAX_PRIMARY_REG_RETRIES) {
      iot_device_sdk_log(SDK_LOG_WARN,"IOT_SDK : maximum Primary retries '%d' reached. Exiting\n", MAX_PRIMARY_REG_RETRIES);
      return -1;
    }
    //Check App Wants to exit
    //if(check_app_exit_flag()) break;
    //sdk_local_sleep(SECONDS, DELAY_BETWEEN_RETRY );// delay between retries

  //} while(SDK_SERVER_FAILURE == regResponseCode || 0 == registered);

#endif // ENABLE_PRIMARY_REG

  //  usleep(DEFAULT_RETRY_INTERVAL * 1000 * 1000); // sleep in seconds. for debugging.
  return -1;
}//RegisterWithPrimaryServer


/**
* RegisterWithCPODServer( ) - Function to communicate with the CPOD server
* Input  :
* return :  0 - success
*          -1 - Error
*/
static int RegisterWithCPODServer(IotDeviceSdkClient handle)
{
  long retry_interval =0;
  int registered = 0; // whether registered with server
  int regResponseCode = 0;
  unsigned int reg_attempt = 1;
  int error_code = 0;
#if ENABLE_CPOD_REG
  /* Loop for MAX_CPOD_REG_RETRIES times, till a successful registration
   * occurs*/
  //do {
    PrintDemarcator(REG_CPOD);
    iot_device_sdk_log(SDK_LOG_INFO,"IOT_SDK : CPOD server Registration attempt '%d'\n", reg_attempt);
    const SDKRegistrationResponse* regResponse =
      (SDKRegistrationResponse*)iot_dsdk_sendRegistrationMessage_v1(handle, &error_code);
    iot_device_sdk_log(SDK_LOG_INFO,"IOT_SDK : CPOD server regResponse'%p'\n", regResponse);

    if (NULL == regResponse) { // error case
      iot_device_sdk_log(SDK_LOG_ERROR,"IOT_SDK : CPOD server Contact Error error_code[%d]\n", error_code);
      if (error_code == IOTERR_CPOD_URL_NULL) {
        // If CPOD Url is NULL, does not seem there is any use in retrying, just exit here.
        // TODO: should this be changed
        iot_device_sdk_log(SDK_LOG_ERROR,"IOT_SDK : RegisterWithCPODServer error IOT_CPOD_URL_NULL \n");
        return -1;
      }
    }
    else {
      //assert(regResponse);
      regResponseCode = regResponse->responseCode;
      registered = regResponse->registered;
      iot_device_sdk_log(SDK_LOG_INFO,"IOT_SDK : CPOD server registration response code(%d)registered(%d)\n",
          regResponseCode, regResponse->registered);

      if (registered) {
        iot_device_sdk_log(SDK_LOG_INFO,"IOT_SDK : CPOD server Registration Successful !\n");
        // print the CPOD server registration response
        PrintCPODSvrRegistrationResponse(regResponse);
        //Keep-alive
        cpod_reg_status = 1;
        keep_alive_interval = GetRetryInterval(regResponse);
        return 0;
        //break;
      } else {
        iot_device_sdk_log(SDK_LOG_INFO,"IOT_SDK : CPOD server Registration Un-Successful !\n");
        // print the primary server registration response
        PrintCPODSvrRegistrationResponse(regResponse);
      }

      // registration unsuccessful, retry using a provided, or default interval
      retry_interval = GetRetryInterval(regResponse);
      if (-1 == retry_interval) {
        iot_device_sdk_log(SDK_LOG_ERROR,"IOT_SDK : Invalid retry interval '%s'\n", regResponse->interval);
        retry_interval = DEFAULT_RETRY_INTERVAL;
      }
    }

    reg_attempt++;
    if (reg_attempt >= MAX_CPOD_REG_RETRIES) {
      iot_device_sdk_log(SDK_LOG_WARN,"IOT_SDK : maximum CPOD retries '%d' reached. Exiting\n", MAX_CPOD_REG_RETRIES);
      return -1;
    }

    //Check App Wants to exit
    //if(check_app_exit_flag()) break;
    //sdk_local_sleep(SECONDS, 5);// delay between retries

  //} while(SDK_SERVER_FAILURE == regResponseCode || 0 == registered);

  //  usleep(DEFAULT_RETRY_INTERVAL * 1000 * 1000); // sleep in seconds. for debugging.

#endif // ENABLE_CPOD_REG

  return -1;
}


/* This should match the enum RegistrationStep values */
static const char *regStepStrs[] = {
  "REG_START",
  "REG_PRIMARY",
  "REG_CPOD",
  "REG_IAM",
  "REG_END"
};

static const char *GetRegStepStr(int regStep)
{
  const int numStrs = sizeof(regStepStrs)/ sizeof(regStepStrs[0]);
  assert(regStep < numStrs);
  return regStepStrs[regStep];
}


/**
* iot_register_step( ) - Function to perform a sequence of registration steps.
* It takes in a step id, which could be the start of the sequence, or a middle step.
* Input  :
* return :  0 - success
*          -1 - Error
*/
int iot_register_step(IotDeviceSdkClient handle, int regStep)
{
  iot_device_sdk_log(SDK_LOG_INFO,"IOT_SDK : %s regStep(%d: %s) ++ \n", __FUNCTION__, regStep, GetRegStepStr(regStep));

  int regPrimary = 0, regCPOD = 0, regIAM = 0, ret =-1;
  /* Perform one registration step, and on success, fall through to the next
   * On error cases, retry or roll up to an earlier step */
  int nextRegStep = regStep;
  /* This is the main loop that allows each step on error, to assign the value
   * of nextRegStep and redo an earlier step */
  //do {

    switch(nextRegStep) {
      case REG_START:
      //Priamry server - depricated
      case REG_PRIMARY:
        /* Begin STEP 1 : Attempt to register with Primary server.
         * On registration failure, retry after interval.
        */
        regPrimary = RegisterWithPrimaryServer(handle);
        if (regPrimary == -1) {
          // TODO On several failed Primary registrations, where do we jump to?
          nextRegStep = REG_PRIMARY;
          ret = -1;
          break;
        }
        //nextRegStep = REG_CPOD;
        nextRegStep = REG_END;
        /* End STEP 1 */
      case REG_CPOD:
        /* Begin STEP 2: Attempt to register with CPOD server. */
        regCPOD = RegisterWithCPODServer(handle);
        if (regCPOD == -1) {
          //nextRegStep = REG_PRIMARY;
          nextRegStep = REG_END;
          ret = -1;
          break;
        }
        nextRegStep = REG_END;
        ret = 0;
        //nextRegStep = REG_IAM;
        /* End STEP 2 : CPOD server registration  */
      case REG_IAM:
        /* Begin STEP 3: Attempt to get access token from IAM server. */
        //regIAM = RegisterWithIAMServer(handle);
        if (regIAM == -1) {
          nextRegStep = REG_CPOD;
          ret = -1;
          break;
        }
        nextRegStep = REG_END;
        /* End STEP 3 : IAM server registration  */
      case REG_END:
        // any finalization, cleanup steps
        // At successful registration, save config data to file
        //SaveConfigurationDataToFile(handle);
        iot_device_sdk_log(SDK_LOG_INFO,"IOT_SDK : %s() Registration Success \n", __FUNCTION__);
        break;
      default:
        iot_device_sdk_log(SDK_LOG_INFO,"IOT_SDK : %s: Unknown registration step (%d)\n", __FUNCTION__, regStep);
        assert(0);
    } // switch

    iot_device_sdk_log(SDK_LOG_INFO,"IOT_SDK : %s nextRegStep(%d:%s)\n", __FUNCTION__,
        nextRegStep, GetRegStepStr(nextRegStep));
  //} while (nextRegStep < REG_END); // continue till the END state has not been reached

  iot_device_sdk_log(SDK_LOG_INFO,"IOT_SDK : %s nextRegStep(%d:%s) -- \n", __FUNCTION__, nextRegStep, GetRegStepStr(nextRegStep));
  if(ret  < 0)
    return -1;

  return 0;
}


/**
* iot_loadconfig_or_register( ) - Function to load configuration data
* or actual registration with the IOT servers
* Input  :
* return :  0 - success
*          -1 - Error
*/
int iot_device_register(IotDeviceSdkClient handle, DeviceDetails* deviceDetail)
{
  int ret = -1;

  //App will be made responsbile to do the keep alive
  //spwan keepalive thread
  //spawn_keep_alive_loop(handle);

  iot_device_sdk_log(SDK_LOG_INFO,"IOT_SDK : %s() ++ \n", __FUNCTION__);

  if(deviceDetail) {
    if(!(deviceDetail->skip_registration)) {
      iot_device_sdk_log(SDK_LOG_INFO,"IOT_SDK : %s() Starting Registration Step \n", __FUNCTION__);
      ret = iot_register_step(handle, REG_CPOD);
    } else {
      iot_device_sdk_log(SDK_LOG_INFO,"IOT_SDK : %s() Skipping Device Registration Step \n", __FUNCTION__);
      iot_device_sdk_log(SDK_LOG_INFO,"IOT_SDK : %s() Starting Access Token Request Step \n", __FUNCTION__);
      ret = iot_register_step(handle, REG_IAM);
    }
  } else {
    iot_device_sdk_log(SDK_LOG_ERROR,"IOT_SDK : %s() Error NULL DeviceDetails passed \n", __FUNCTION__);
  }
  iot_device_sdk_log(SDK_LOG_INFO,"IOT_SDK : %s() -- \n", __FUNCTION__);
  return ret;
}


/**
* iot_handle_send_data_error( ) - Function to handle error
* Input  :
* return :  0 - success
*          -1 - Error
*/
void iot_handle_send_data_error(IotDeviceSdkClient handle, int response_code)
{
  int ret = 0;
  iot_device_sdk_log(SDK_LOG_INFO,"IOT_SDK : %s response_code(%d) ++ \n", __FUNCTION__, response_code);

  // received a redirect response from the server
  if (response_code == 302 || response_code == 301) {
    iot_device_sdk_log(SDK_LOG_INFO,"IOT_SDK : %s response_code(%d) handle Redirect case \n", __FUNCTION__, response_code);
    // If redirect, then call the AccessToken API to hit IAM server
    iot_register_step(handle, REG_IAM);
  }
  else {
    /* could not successfully send data to the DSX server.
       re-try registration with CPOD server */
    ret = iot_register_step(handle, REG_CPOD);
    iot_device_sdk_log(SDK_LOG_INFO,"IOT_SDK : %s register step ret (%d)\n", __FUNCTION__, ret);
  }

  iot_device_sdk_log(SDK_LOG_INFO,"IOT_SDK : %s -- \n", __FUNCTION__);
}

static int lc =0;

void iot_LogCallsTest()
{
  char logfile[128];
  int ret = -1;
  sprintf(logfile, "mylog_%d", lc);
  FILE *fp = fopen("errorlev.txt", "rt");
  if (fp) {
    int loglevel = 0;
    ret = fscanf(fp, "%d", &loglevel);
    if(ret > 0) {
      if (loglevel >0) {
        iot_device_sdk_setLogLevel (loglevel);
      }
    }
    /*
    if (loglevel ==-1) {
    iot_device_sdk_setLogFile ("");
    }
    else {
      iot_device_sdk_setLogFile (logfile);
    }
    */
    fclose(fp);
  }
}

#if 0
static void PrintDeserializedData(const DeserializedData* ds_data)
{
  iot_device_sdk_log(SDK_LOG_INFO,"IOT_SDK :%s ++ \n", __FUNCTION__ );
  assert(ds_data);
  if (NULL == ds_data) {
    iot_device_sdk_log(SDK_LOG_ERROR,"IOT_SDK :%s Error: NULL ds_data passed in \n", __FUNCTION__ );
  }

  int i = 0;
  int num = ds_data->num;
  iot_device_sdk_log(SDK_LOG_INFO,"IOT_SDK :%s num=%d\n"
      "  ----------------------------\n", __FUNCTION__, num);
  for (i =0; i<num; i++) {
  iot_device_sdk_log(SDK_LOG_INFO," %d/%d] name(%s)type(%s)value(%s)\n", i, num,
      ds_data->data[i].name, ds_data->data[i].type, ds_data->data[i].value);
  }
  iot_device_sdk_log(SDK_LOG_INFO,"IOT_SDK : ----------------------------\n");

  iot_device_sdk_log(SDK_LOG_INFO,"IOT_SDK: %s -- \n", __FUNCTION__ );
}

static void ReleaseDeserializedData(DeserializedData* ds_data)
{
  iot_device_sdk_log(SDK_LOG_INFO,"IOT_SDK  :%s ++ \n", __FUNCTION__ );
  assert(ds_data);
  if (NULL == ds_data) {
    iot_device_sdk_log(SDK_LOG_ERROR,"IOT_SDK  :%s Error: NULL ds_data passed in \n", __FUNCTION__ );
  }

  int i = 0;
  int num = ds_data->num;
  iot_device_sdk_log(SDK_LOG_INFO,"IOT_SDK  :%s num=%d\n"
      "  ----------------------------\n", __FUNCTION__, num);
  for (i =0; i<num; i++) {
    SAFE_FREE(ds_data->data[i].name)
    SAFE_FREE(ds_data->data[i].type)
    SAFE_FREE(ds_data->data[i].value)
  }
  SAFE_FREE(ds_data->data)
  ds_data->num = 0;
}
#endif
