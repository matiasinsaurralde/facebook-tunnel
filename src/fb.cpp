#include "fb.h"
#include "exception.h"
#include "utility.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include <iostream>
#include <sstream>

#include <arpa/inet.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "curl/curl.h"
#include "curl/easy.h"

#include "base64encode.h"

typedef ip IpHeader;
typedef tcphdr TcpHeader;

using namespace std;

CURL *curl;
CURLcode res;

char *cookies;

Facebook::Facebook() {
};

Facebook::~Facebook()
{
//    tun_close(fd, device);
}

/*
 print_hex_ascii_line y print_payload de tcpdump:
 http://www.tcpdump.org/sniffex.c
*/

void Facebook::print_hex_ascii_line(const char *payload, int len, int offset) {
  int i;
  int gap;
  const char *ch;
  printf("%05d   ", offset);
  ch = payload;

  for(i = 0; i < len; i++) {
    printf("%02x ", *ch);
    ch++;
    if (i == 7)
      printf(" ");
    };
    if (len < 8)
      printf(" ");
    if (len < 16) {
      gap = 16 - len;
      for (i = 0; i < gap; i++) {
        printf("   ");
      }
     }
    printf("   ");
    ch = payload;
    for(i = 0; i < len; i++) {
      if (isprint(*ch))
        printf("%c", *ch);
      else
        printf(".");
        ch++;
    };
  printf("\n");
  return;
};

void Facebook::print_payload(const char *payload, int len) {

  int len_rem = len;
  int line_width = 16;
  int line_len;
  int offset = 0;
  const char *ch = payload;

  if (len <= 0)
  return;

  if (len <= line_width) {
    print_hex_ascii_line(ch, len, offset);
    return;
  }

  for ( ;; ) {
    line_len = line_width % len_rem;
    print_hex_ascii_line(ch, line_len, offset);
    len_rem = len_rem - line_len;
    ch = ch + line_len;
    offset = offset + line_width;
    if (len_rem <= line_width) {
      print_hex_ascii_line(ch, len_rem, offset);
      break;
    }
  }

  return;

};

void Facebook::send_packet( const char *payload, int length ) {

  printf("Facebook::send_packet!\n");

  IpHeader *header = (IpHeader *)payload;
  TcpHeader *tcpheader = (TcpHeader *)( payload + sizeof(struct ip) );

  string source_ip, dest_ip;
  source_ip = Utility::formatIp( ntohl( header->ip_src.s_addr ) );
  dest_ip = Utility::formatIp( ntohl( header->ip_dst.s_addr ) );

//  printf("%s -> %s\n", source_ip.c_str(), dest_ip.c_str() )

  // length = 2 IPs + 2 puertos + 4 separadores
  char serialized_packet[ 30 + 10 + 4 + sizeof( payload ) ];

  char enc[2 * 32768];
  base64_encodestate S;
  base64_encode_init(&S);

  size_t i;
  i = base64_encode_update(&S, (const uint8_t*)payload, sizeof(payload), enc);

  sprintf( serialized_packet, "%s,%d,%s,%d,%s", source_ip.c_str(), htons( tcpheader->source ), dest_ip.c_str(), htons( tcpheader->dest ), enc );

  puts( serialized_packet );

//  fwrite(enc, 1, i, stdout);

/* aca esta la parte del fb,
   primero hice una prueba con ruby, para probarlo rapido
   esta en test/fb_chat_poc.rb.
   una de las cosas que faltaria es decirle al FB que somos Firefox,
   es decir modificar el user agent, con el setopt se puede hacer */

  curl = curl_easy_init();


  curl_easy_setopt( curl, CURLOPT_COOKIEFILE, "cookies.txt");
  curl_easy_setopt( curl, CURLOPT_VERBOSE, 0 );

  curl_easy_setopt(curl, CURLOPT_URL, "https://m.facebook.com/messages/send/?icm=1&refid=12");

  struct curl_httppost *formpost=NULL;
  struct curl_httppost *lastptr=NULL;

  // estas son las boludeces que el FB pone

  curl_formadd(&formpost,
               &lastptr,
               CURLFORM_COPYNAME, "fb_dtsg",
               CURLFORM_COPYCONTENTS, "AQANMR10",
               CURLFORM_END);

  curl_formadd(&formpost,
               &lastptr,
               CURLFORM_COPYNAME, "send",
               CURLFORM_COPYCONTENTS, "Responder",
               CURLFORM_END);

  curl_formadd(&formpost,
               &lastptr,
               CURLFORM_COPYNAME, "wwwupp",
               CURLFORM_COPYCONTENTS, "V3",
               CURLFORM_END);

  // este es el id del chat, no estoy seguro si es el mismo que el user id, es el de un amigo
  curl_formadd(&formpost,
               &lastptr,
               CURLFORM_COPYNAME, "ids[1336031945]",
               CURLFORM_COPYCONTENTS, "1336031945",
               CURLFORM_END);

  curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);

  // en vez de "contenido" deberia ir el paquete en base64

  curl_formadd(&formpost,
               &lastptr,
               CURLFORM_COPYNAME, "body",
               CURLFORM_COPYCONTENTS, serialized_packet,
               CURLFORM_END);

  // con esto envias el mensaje a fb

  res = curl_easy_perform( curl );
  curl_easy_cleanup( curl );

  // esta funcion estaba en tcpdump, sirve para debugging nomas:

  // print_payload( payload, length );

};

void Facebook::printbinchar(char character, int length) {
    char output[ length ];
    printf("%s\n", output);
};
