#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <errno.h>

#include <arpa/inet.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>


#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_tun.h>

#include "tun.h"

typedef ip IpHeader;

int tun_open(char *dev, int flags) {

  struct ifreq ifr;
  int fd, err;
  char *clonedev = "/dev/net/tun";

   if( (fd = open(clonedev, O_RDWR)) < 0 ) {
     return fd;
   }

   memset(&ifr, 0, sizeof(ifr));

   ifr.ifr_flags = flags;   /* IFF_TUN | IFF_TAP | IFF_NO_PI */

   if (*dev) {
     strncpy(ifr.ifr_name, dev, IFNAMSIZ);
   }

   if( (err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0 ) {
     close(fd);
     return err;
   }

  strcpy(dev, ifr.ifr_name);

  return fd;

};

int tun_read( int fd, char *buf, int len ) {

  return read(fd, buf, len);

};

int tun_read2( int fd, char *buf, int len, uint32_t &sourceIp, uint32_t &destIp ) {
  int length = tun_read( fd, buf, len );
  IpHeader *header = (IpHeader *)buf;

  sourceIp = ntohl(header->ip_src.s_addr);
  destIp = ntohl(header->ip_dst.s_addr);

  return length;

};

void tun_setup( char *dev, int mtu ) {

  char cmdline[512];
  snprintf(cmdline, sizeof(cmdline), "/sbin/ifconfig %s 10.10.10.1 netmask 255.255.255.0 mtu %u", dev, mtu);
  if (system(cmdline) != 0)
      printf("ERR!");

};
