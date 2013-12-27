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

void Tun::setup( const char* device, int mtu ) {

  char cmdl[512];
  snprintf(cmdl, sizeof(cmdl), "/sbin/ifconfig %s mtu %u", device, mtu);

  if (system(cmdl) != 0)
      syslog(LOG_ERR, "could not set tun device mtu");
};

Tun::Tun(const char *device, int mtu ) {

  int fd = allocate( device );
  std::cout << device << std::endl;
  setup( device, mtu );

};

Tun::~Tun()
{
    tun_close(fd, device);
}

void Tun::setIp(uint32_t ip, uint32_t destIp, bool includeSubnet)
{
    char cmdline[512];
/*    string ips = Utility::formatIp(ip);
    string destIps = Utility::formatIp(destIp);

#ifdef LINUX
    snprintf(cmdline, sizeof(cmdline), "/sbin/ifconfig %s %s netmask 255.255.255.0", device, ips.c_str());
#else
    snprintf(cmdline, sizeof(cmdline), "/sbin/ifconfig %s %s %s netmask 255.255.255.255", device, ips.c_str(), destIps.c_str());
#endif
*/
/*
    if (system(cmdline) != 0)
        syslog(LOG_ERR, "could not set tun device ip address");

#ifndef LINUX
    if (includeSubnet)
    {
        snprintf(cmdline, sizeof(cmdline), "/sbin/route add %s/24 %s", destIps.c_str(), destIps.c_str());
        if (system(cmdline) != 0)
            syslog(LOG_ERR, "could not add route");
    }
#endif
*/
}

void Tun::write(const char *buffer, int length)
{
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
