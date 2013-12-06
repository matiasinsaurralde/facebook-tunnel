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

  /* Arguments taken by the function:
   *
   * char *dev: the name of an interface (or '\0'). MUST have enough
   *   space to hold the interface name if '\0' is passed
   * int flags: interface flags (eg, IFF_TUN etc.)
   */

   /* open the clone device */
   if( (fd = open(clonedev, O_RDWR)) < 0 ) {
     return fd;
   }

   /* preparation of the struct ifr, of type "struct ifreq" */
   memset(&ifr, 0, sizeof(ifr));

   ifr.ifr_flags = flags;   /* IFF_TUN or IFF_TAP, plus maybe IFF_NO_PI */

   if (*dev) {
     /* if a device name was specified, put it in the structure; otherwise,
      * the kernel will try to allocate the "next" device of the
      * specified type */
     strncpy(ifr.ifr_name, dev, IFNAMSIZ);
   }

   /* try to create the device */
   if( (err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0 ) {
     close(fd);
     return err;
   }

  /* if the operation was successful, write back the name of the
   * interface to the variable "dev", so the caller can know
   * it. Note that the caller MUST reserve space in *dev (see calling
   * code below) */
  strcpy(dev, ifr.ifr_name);

  /* this is the special file descriptor that the caller will use to talk
   * with the virtual interface */
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
