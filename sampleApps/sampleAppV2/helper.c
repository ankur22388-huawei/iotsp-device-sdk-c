/**************************************************************************
  copyright (C) Copyright 2015-2016 Cisco Systems Inc.; All Rights Reserved
  History:
          01/12/2016 - Created.
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include "helper.h"

char* getMacAddress(){
  struct ifreq s;
  int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
  char *mac_address;
  strcpy(s.ifr_name, "eth0");
  if (0 == ioctl(fd, SIOCGIFHWADDR, &s)) {
    mac_address = (char*)calloc(72,sizeof(unsigned char));
    if(mac_address){
      sprintf(mac_address, "%02x:%02x:%02x:%02x:%02x:%02x", (unsigned char) s.ifr_addr.sa_data[0],
      (unsigned char) s.ifr_addr.sa_data[1], (unsigned char) s.ifr_addr.sa_data[2],
      (unsigned char) s.ifr_addr.sa_data[3],(unsigned char) s.ifr_addr.sa_data[4],
      (unsigned char) s.ifr_addr.sa_data[5]);
      close(fd);
      return mac_address;
    } else {
      close(fd);
      return NULL;
    }
  }
  return NULL;
}

char* getipv4_Address(){
  struct ifaddrs *ifaddr, *ifa;
  int family, s,found = 0;
  char host[NI_MAXHOST];
  char *ipv4Addr=NULL;

  if (getifaddrs(&ifaddr) == -1) {
    printf("sampleApp1 getipv4_Address: getifaddrs Failed \n");
    return NULL;
  }
  for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
    if(ifa->ifa_addr == NULL) continue;
    family = ifa->ifa_addr->sa_family;
    if (family == AF_INET) {
      s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in),
                   host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
      if (s != 0) {
        printf("sampleApp1 getipv4_Address: getnameinfo Failed : %s \n", gai_strerror(s));
        return NULL;
      }
      if(!strcmp(ifa->ifa_name,"eth0")) {
        found = 1;
        break;
      }
    }
  }
  freeifaddrs(ifaddr);
  if(found){
    printf("sampleApp1 : ipv4 = %s \n", host);
    ipv4Addr = (char*)malloc(sizeof(char)*NI_MAXHOST);
    strcpy(ipv4Addr,host);
    return ipv4Addr;
  }

  return NULL;
}



void local_sleep(int type, int sleep_val)
{
  const char *units;
  int usleep_val;
  switch (type) {
    case MILLISECONDS:
      units = "ms";
      usleep_val = sleep_val * 1000;
      break;
    case SECONDS:
      units = "sec";
      usleep_val = sleep_val * 1000 * 1000;
      break;
    default:
      printf("sampleAppV2 :%s Unknown type %d \n", __FUNCTION__, type);
      assert(0);
      return;
  }
  printf("sampleAppV2 : %s %d %s...... \n", __FUNCTION__, sleep_val, units );
  usleep(usleep_val); // sleep for specified interval
}
