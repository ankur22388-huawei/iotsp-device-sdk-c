#include Makerules
#-include $(OBJS:%.o=%.d)
IOT_DEVICE_SDK_VER := 0.1

# Set the tools prefix for cross-compiling
TOOLS_PREFIX:=
CC		:=	$(TOOLS_PREFIX)gcc
LD		:=	$(TOOLS_PREFIX)ld

IOT_DEVICE_SDK_DIR	:= $(shell pwd)


export TOOLS_PREFIX CC LD LIBCURL_STATIC IOT_DEVICE_SDK_DIR

.PHONY: all clean install doc doxy docs_clean
.SUFFIXES: .c .o

all : linux

linux: libsdk appV1 appV2 appV2_mockData appSerial



libsdk:
	$(MAKE) -C linux/ all


doc:
	./docs/create_doc.sh

doxy: doc
	doxygen docs/doxygen.cfg
	$(MAKE) -C docs/latex
	cp docs/latex/refman.pdf docs/cisco_iot_device_sdk_api_V$(IOT_DEVICE_SDK_VER).pdf

appV1: libsdk
	$(MAKE) -C sampleApps/sampleApp/ all

appV2: libsdk
	$(MAKE) -C sampleApps/sampleAppV2/ all

appV2_mockData: libsdk
	$(MAKE) -C sampleApps/sampleAppV2_mockData/ all

appSerial: libsdk
	$(MAKE) -C sampleApps/sampleAppSerial/ all

appV1_clean:
	rm -rf sampleApps/sampleApp/iot_device_sdk.log
	$(MAKE) -C sampleApps/sampleApp clean

appV2_clean:
	rm -rf sampleApps/sampleAppV2/iot_device_sdk.log
	$(MAKE) -C sampleApps/sampleAppV2 clean

appV2_mockData_clean:
	rm -rf sampleApps/sampleAppV2_mockData/iot_device_sdk.log
	$(MAKE) -C sampleApps/sampleAppV2_mockData clean

appSerial_clean:
		rm -rf sampleApps/sampleAppSerial/iot_device_sdk.log
		$(MAKE) -C sampleApps/sampleAppSerial clean

libsdk_clean:
		rm -f linux/*.o
		rm -f install/lib/lib_iot*.so*

clean_all: appV1_clean appV2_clean appSerial_clean appV2_mockData_clean
	rm -f linux/*.gcda
	rm -f linux/*.gcno
	rm -f linux/*.o
	rm -f install/lib/lib_iot*

version:
	@echo $(IOT_DEVICE_SDK_VER)
