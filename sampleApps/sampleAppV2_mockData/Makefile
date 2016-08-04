CC		:=	$(TOOLS_PREFIX)gcc
SRCS = sample_main.c \
	helper.c

OBJS = $(SRCS:.c=.o)

APP_DIR := $(shell pwd)
API_DIR := $(APP_DIR)/../../api
LIB_DIR := $(APP_DIR)/../../install/lib
LIBS := -l_iot_device_sdk -ljansson -lpaho-mqtt3as -lpthread -lssl -lgcov

#CFLAGS := -O2 -I$(API_DIR) $(APP_CFLAGS)
#CFLAGS := -fprofile-arcs -ftest-coverage --coverage -g -Wall -static $(/ws/aspadman-sjc/INSTALL/bin/curl-config --static-libs --cflags)-Wextra -I$(API_DIR) $(APP_CFLAGS)
CFLAGS := -fprofile-arcs -ftest-coverage --coverage -g -Wall -Wextra -I$(API_DIR) $(APP_CFLAGS)
LFLAGS := -fprofile-arcs --coverage -L$(LIB_DIR) $(LIBS)

.PHONY: all clean
TARGET = sampleAppV2

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -L$(LIB_DIR) $(OBJS) -o $(TARGET) $(LIBS)

.c.o:
	$(Q)$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(APP_DIR)/*.o
	rm -f $(APP_DIR)/sampleAppV2
