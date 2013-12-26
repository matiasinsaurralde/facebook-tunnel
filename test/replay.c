#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>

#include <arpa/inet.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>

struct ip *iphdr = NULL;

int main() {

  char serialized_packet[] = "176.9.168.100,80,10.1.1.114,52775,RQAAP";

  char* buf;
  buf = strtok( serialized_packet, "," );

  char dest_ip[12];
  int dest_port;

  int i = 0;

  while( buf ) {

    switch( i ) {
      case 1:
        break;
      case 2:
        dest_port = buf;
        break;
    };

    buf = strtok( NULL, "," );

    while( buf && *buf == '\040' )
      buf++; i++;
  };

  printf("dest_ip = %s, dest_port = %d\n", dest_ip, dest_port );

//  iphdr = (struct ip *)(buffer);
//  printf("%d\n", iphdr->ip_dst.s_addr );
};
