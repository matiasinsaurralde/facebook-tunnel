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
#include "utils.h"

#include "base64encode.h"

#include <iostream>
#include <thread>

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

/* network, tcp/ip */

#include <arpa/inet.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>

typedef tcphdr TcpHeader;
typedef ip IpHeader;

using namespace std;

int Tun::allocate( char* device ) {

  struct ifreq ifr;
  int fd, err;

  char cldev[32];
  strcpy( cldev, "/dev/net/tun" );

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

  sprintf( this->device, "%s", device);

  return fd;
}

void Tun::setup( char* device ) {

  char cmdl[512];
  snprintf(cmdl, sizeof(cmdl), "/sbin/ifconfig %s mtu %u", this->device, this->mtu);

  if (system(cmdl) != 0)
      syslog(LOG_ERR, "could not set tun device mtu");

};

Tun::Tun( char* device, int mtu, int mode, FacebookClient* facebook ) {

  this->fd = allocate( device );
  this->mtu = mtu;
  this->mode = mode;
  this->facebook = facebook;

  setup( device );

  setIp();

  syslog( LOG_DEBUG, "Tun interface up!" );

};

Tun::~Tun() {
    tun_close(fd, device);
}

void Tun::setIp() {

    // take mode in account

    char cmdline[512];
    snprintf(cmdline, sizeof(cmdline), "/sbin/ifconfig %s 10.1.1.1 netmask 255.255.255.0", this->device);

    if (system(cmdline) != 0)
        syslog(LOG_ERR, "could not set tun device ip address");

    system("/sbin/route add 176.9.168.100 gw 10.1.1.1");

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

void Tun::keepWriting() {

  int alive = 1; // fixme.

  int length = 0;
  char buf[ this->mtu ];

  cout << "Ready and waiting for packets." << endl;

  while( alive ) {

    length = this->read( buf );

    if( length > 0 ) {

      char serialized_packet[ 30 + 10 + 4 + sizeof( buf ) ];

      IpHeader *header = (IpHeader *)(buf + 4  );
      TcpHeader *tcpheader = (TcpHeader *)( buf + 4 + sizeof(struct ip) );

      string source_ip, dest_ip;
      source_ip = Utils::formatIp( ntohl( header->ip_src.s_addr ) );
      dest_ip = Utils::formatIp( ntohl( header->ip_dst.s_addr ) );

      // timestamp
      std::time_t currentTimestamp = std::time(0);

      // payload -> base64
      char enc[2 * 32768];
      base64_encodestate S;
      base64_encode_init(&S);

      size_t i;
      i = base64_encode_update(&S, (const uint8_t*)buf, sizeof(buf), enc);

      sprintf( serialized_packet, "|%lu,%s,%d,%s,%d,%s", currentTimestamp, source_ip.c_str(), htons( tcpheader->source ), dest_ip.c_str(), htons( tcpheader->dest ), enc );

      this->facebook->sendPacketTo( this->facebook->friendID, serialized_packet, sizeof( serialized_packet ) );

      puts( serialized_packet ); puts( "" );

    };

  };
};

void Tun::keepReading() {
  this->facebook->listClients();
  // for ( ;; ) { };
};

void Tun::run() {
  std::thread readTh(  &Tun::keepReading, this );
  std::thread writeTh( &Tun::keepWriting, this );

  readTh.join(); writeTh.join();

};
