/**************************************************************************
  copyright (C) Copyright 2015-2016 Cisco Systems Inc.; All Rights Reserved
  Author :
  History:
 ******************************************************************************/
 #include <stdio.h>
 #include <string.h>

static const char *primary_reg_msg =
"{\n\
      \"thing\":       {\n\
                \"make\":  \"\",\n\
                \"model\": \"\",\n\
                \"firmwareVersion\":   \"\",\n\
                \"hardwareVersion\":   \"\",\n\
                \"uniqueIdentifiers\":  {\n\
                        \"manufacturingId\":   \"\",\n\
                        \"macAddress\":   \"\",\n\
                        \"serialNumber\":  \"\"\n\
                }\n\
       },\n\
      \"service\":      {\n\
                \"claimed\":  false,\n\
                \"registered\": false,\n\
                \"request\":      {\n\
                   },\n\
                \"credentials\":      {\n\
                    \"name\":   \"\",\n\
                    \"secret\":   \"\"\n\
                   },\n\
                \"thingUid\": \"\",\n\
                \"fingerprint\":  \"\"\n\
      }\n\
}\n";

static const char *cpod_reg_msg =
"{\n\
      \"thing\":       {\n\
                \"make\":  \"\",\n\
                \"model\": \"\",\n\
                \"firmwareVersion\":   \"\",\n\
                \"hardwareVersion\":   \"\",\n\
                \"uniqueIdentifiers\":  {\n\
                        \"manufacturingId\":   \"\",\n\
                        \"macAddress\":   \"\",\n\
                        \"serialNumber\":  \"\"\n\
                }\n\
       },\n\
      \"service\":      {\n\
                \"claimed\": false,\n\
                \"registered\": false,\n\
                \"request\":      {\n\
                   },\n\
                \"credentials\":      {\n\
                    \"name\":   \"\",\n\
                    \"secret\":   \"\"\n\
                   },\n\
                \"thingUid\": \"\",\n\
                \"fingerprint\":  \"\"\n\
      }\n\
}\n";

const char *GetDefaultPrimaryRegMsg()
{
  return primary_reg_msg;
}

const char *GetDefaultCPODRegMsg()
{
  return cpod_reg_msg;
}
