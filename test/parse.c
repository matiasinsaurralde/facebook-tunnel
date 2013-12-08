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

  FILE *file;
  size_t size;
  unsigned char *buffer;

  file = fopen("test.txt", "rb");

  fseek(file, 0, SEEK_END);
  size = ftell(file);
  fseek(file, 0, SEEK_SET);

  buffer = (unsigned char *) malloc(size);


  iphdr = (struct ip *)(buffer);
  printf("%d\n", iphdr->ip_dst.s_addr );
};
