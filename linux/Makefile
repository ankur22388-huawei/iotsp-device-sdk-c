IOT_DEVICE_SDK_VERSION_MAJOR:= 0
IOT_DEVICE_SDK_VERSION_MINOR:= 1
IOT_DEVICE_SDK_VER	:= $(IOT_DEVICE_SDK_VERSION_MAJOR).$(IOT_DEVICE_SDK_VERSION_MINOR)

INSTALL_LIBDIR=../install/lib
TARGET=$(INSTALL_LIBDIR)/lib_iot_device_sdk.so.$(IOT_DEVICE_SDK_VER)

INCLUDE_DIRS	:= -I$(IOT_DEVICE_SDK_DIR)/api  \
									-I$(IOT_DEVICE_SDK_DIR)/install/include \

CFLAGS	+= -fprofile-arcs -ftest-coverage --coverage -g -Wall -Wextra -O3 -Wno-multichar $(INCLUDE_DIRS)
LDFLAGS += -fprofile-arcs --coverage -lgcov


all: $(TARGET)

clean:
	rm -f $(SDKOBJS) $(TARGET)

SDKOBJS:=iot_device_sdk_config.o \
	iot_device_sdk_curl.o \
	iot_device_sdk_default_msg.o \
	iot_device_sdk_init.o \
	iot_device_sdk_json.o \
	iot_device_sdk_logging.o \
	iot_device_sdk_rest.o \
	iot_device_sdk_storage.o \
	iot_device_sdk_registration.o \
	iot_device_sdk_data_send.o \
	iot_device_sdk_helper.o \
	iot_device_sdk_mqtt_client.o \
	iot_device_sdk_utils.o \
	iot_device_sdk_storage_cache.o \
	iot_device_sdk_access_token.o \

# Insert additional obj files above, sorted alphabetically

%.o:%.c
	$(Q)$(CC) -fPIC $(CFLAGS) $^ -c -o $@

$(TARGET) : $(SDKOBJS)
ifeq ($(LIBCURL_STATIC), yes)
	$(Q)$(CC) $(CFLAGS) -shared -Wl,-soname,lib_iot_device_sdk.so -o $@ $^ $(IOT_DEVICE_SDK_DIR)/sampleApps/ioxApp/ir829_lib/libcurl.a
else
	$(Q)$(CC) $(CFLAGS) -shared -Wl,-soname,lib_iot_device_sdk.so -o $@ $^ -L$(IOT_DEVICE_SDK_DIR)/install/lib -lcurl
endif
	cd $(INSTALL_LIBDIR); ln -sf lib_iot_device_sdk.so.$(IOT_DEVICE_SDK_VER) lib_iot_device_sdk.so; cd -
