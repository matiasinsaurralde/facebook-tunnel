/*
 *  Hans - IP over ICMP
 *  Copyright (C) 2009 Friedrich Schöller <hans@schoeller.se>
 *  
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *  
 */

#include "tun.h"

#include <iostream>

#include <arpa/inet.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <syslog.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#include <unistd.h>
#include <fcntl.h>


#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_tun.h>

typedef ip IpHeader;

using namespace std;

int Tun::allocate( const char *device ) {

  struct ifreq ifr;
  int fd, err;

  char *cldev = "/dev/net/tun";

  if( ( fd = open( cldev, O_RDWR ) ) < 0 ) {
    return fd;
  };

  memset(&ifr, 0, sizeof(ifr));
  ifr.ifr_flags = IFF_TUN;
  if (*device) {
    strncpy(ifr.ifr_name, device, IFNAMSIZ);
  };

  if( (err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0 ) {
    close(fd);
    printf("err: %d\n",  err );
  };

  return fd;
}

void Tun::setup( const char* device ) {

  char cmdl[512];
  snprintf(cmdl, sizeof(cmdl), "/sbin/ifconfig %s mtu %u", device, this->mtu);

  if (system(cmdl) != 0)
      syslog(LOG_ERR, "could not set tun device mtu");

  this->device = device;
};

Tun::Tun(const char *device, int mtu, int mode ) {

  this->fd = allocate( device );
  this->mtu = mtu;
  this->mode = mode;

  setup( device );

  setIp();

  syslog( LOG_DEBUG, "Tun interface up!" );

};

Tun::~Tun() {
    tun_close(fd, device);
}

void Tun::setIp() {

printf("modeeeee: %d\n", mode);

    char cmdline[512];
    snprintf(cmdline, sizeof(cmdline), "/sbin/ifconfig %s 10.1.1.1 netmask 255.255.255.0", this->device);
    if (system(cmdline) != 0)
        syslog(LOG_ERR, "could not set tun device ip address");

};

void Tun::write(const char *buffer, int length) {
    if (tun_write(fd, (char *)buffer, length) == -1)
        syslog(LOG_ERR, "error writing %d bytes to tun: %s", length, strerror(errno));
}

int Tun::read(char *buffer) {
    int length = tun_read(fd, buffer, mtu);
    if (length == -1)
        syslog(LOG_ERR, "error reading from tun: %s", strerror(errno));
    return length;
}

int Tun::read(char *buffer, uint32_t &sourceIp, uint32_t &destIp) {
    int length = read(buffer);

    IpHeader *header = (IpHeader *)buffer;
    sourceIp = ntohl(header->ip_src.s_addr);
    destIp = ntohl(header->ip_dst.s_addr);

    return length;
}
