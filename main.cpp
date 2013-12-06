#include <iostream>

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/ioctl.h>
#include <sys/socket.h>

#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#include <linux/if.h>
#include <linux/if_tun.h>
#include "tun.h"

int main() {

  int mtu = 512;
  char tun_name[IFNAMSIZ];
  strcpy( tun_name, "tun0" );

  int tun_fd = tun_open( tun_name, IFF_TUN );

  tun_setup( tun_name, mtu );

  char tun_buffer[ mtu ];
  int nread;

  while( 1 ) {

    uint32_t sourceIp, destIp;
    nread = tun_read2(tun_fd, tun_buffer, sizeof( tun_buffer ), sourceIp, destIp );

/*    struct sockaddr_in source;
    int source_addr_len = sizeof(struct sockaddr_in);

    int dataLength = recvfrom(tun_fd, tun_buffer, sizeof(tun_buffer), 0, (struct sockaddr *)&source, (socklen_t *)&source_addr_len);
*/


    printf("! Paquete: %d, %d -> %d\n", nread, sourceIp, destIp);
//    printf("%d\n", sourceIp);

  };

};
