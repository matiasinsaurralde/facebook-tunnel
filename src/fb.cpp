#include "fb.h"
#include "exception.h"
#include "utility.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include "curl/curl.h"
#include "curl/easy.h"

CURL *curl;
CURLcode res;

char *cookies;

Facebook::Facebook() {

  cookies = read_cookies();

//  res = curl_easy_perform( curl );

//  curl_easy_cleanup( curl );
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

char* Facebook::read_cookies() {
  printf("Facebook::read_cookies()\n");
  FILE *cookie_f;
  int i;
  int line_no = 0;
  char *lines[15];

  cookie_f = fopen("cookies.txt", "r");

  if( cookie_f ) {

    char line[250];
    char *eof;
    int i = 0;

    while(( eof = fgets( line, 101, cookie_f ) ) != NULL) {
      lines[i] = strdup( eof );
      i++; line_no++;
    };
  };

/*  printf("%d\n", line_no);
  for( int i = 0; i < line_no; ++i ) {
    printf("%s", lines[i] );
  };
  printf("\n\n\n\n");*/

  return *lines;
};

void Facebook::send_packet( const char *payload, int length ) {

  printf("Facebook::send_packet!\n");

  curl = curl_easy_init();
//  curl_easy_setopt( curl, CURLOPT_COOKIELIST, cookies );
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
               CURLFORM_COPYNAME, "body",
               CURLFORM_COPYCONTENTS, "prueba desde C",
               CURLFORM_END);

  curl_formadd(&formpost,
               &lastptr,
               CURLFORM_COPYNAME, "ids[100005347350787]",
               CURLFORM_COPYCONTENTS, "100005347350787",
               CURLFORM_END);

  curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);

  res = curl_easy_perform( curl );
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));

  curl_easy_cleanup(curl);

  printf("curl: %d\n", res);

//  print_payload( payload, length );

};
