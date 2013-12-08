#include "fb.h"
#include "exception.h"
#include "utility.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include <iostream>
#include <sstream>

#include "curl/curl.h"
#include "curl/easy.h"

#include "b64.h"

#include <msgpack.h>

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

  curl = curl_easy_init();

  curl_easy_setopt( curl, CURLOPT_COOKIEFILE, "cookies.txt");
  curl_easy_setopt( curl, CURLOPT_VERBOSE, 0 );

  curl_easy_setopt(curl, CURLOPT_URL, "https://m.facebook.com/messages/send/?icm=1&refid=12");

  struct curl_httppost *formpost=NULL;
  struct curl_httppost *lastptr=NULL;

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

  curl_formadd(&formpost,
               &lastptr,
               CURLFORM_COPYNAME, "ids[100005347350787]",
               CURLFORM_COPYCONTENTS, "100005347350787",
               CURLFORM_END);

  curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);

/*  msgpack_sbuffer* sbuf = msgpack_sbuffer_new();
  msgpack_packer* pk = msgpack_packer_new(sbuf, msgpack_sbuffer_write);

  msgpack_pack_raw( pk, length );
  msgpack_pack_raw_body( pk, payload, length );

  msgpack_unpacked msg;
  msgpack_unpacked_init(&msg);
  bool success = msgpack_unpack_next(&msg, sbuf->data, sbuf->size, NULL);

  msgpack_object obj = msg.data;
  msgpack_object_print(stdout, obj.via.raw);
  printf("%s\n", obj.via.raw);
  msgpack_serialize();

  msgpack_sbuffer_free(sbuf);
  msgpack_packer_free(pk);*/

  curl_formadd(&formpost,
               &lastptr,
               CURLFORM_COPYNAME, "body",
               CURLFORM_COPYCONTENTS, "contenido",
               CURLFORM_END);

  std::string s;
  std::stringstream out;
  out << payload;
  s = out.str();

  std::cout << s << std::endl;
//  res = curl_easy_perform( curl );
//  curl_easy_cleanup( curl );

//  print_payload( payload, length );

};

void Facebook::printbinchar(char character, int length) {
    char output[ length ];
//    itoa(character, output, 2);
    printf("%s\n", output);
};
