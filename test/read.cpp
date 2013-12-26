#include <stdio.h>
#include <curl/curl.h>
#include <iostream>

#include "tinyxml2.h"

using namespace tinyxml2;
using namespace std;

string rawBody;

size_t curl_write( void *ptr, size_t size, size_t nmemb, void *stream) {
  rawBody.append( (char*)ptr, size*nmemb );
  return size*nmemb;
};

int main( int argc, char *argv[] ) {

  CURL *curl;
  CURLcode res;
  char *rawHtml;
  char url[60];
  XMLDocument doc;

  sprintf( url, "https://m.facebook.com/messages/thread/%s", argv[1] );

  printf("FB ID: %s\nURL: %s\n\n", argv[1], url );

  curl = curl_easy_init();

  if(curl) {

    curl_easy_setopt(curl, CURLOPT_URL, url );

    curl_easy_setopt( curl, CURLOPT_USERAGENT, "Mozilla Firefox");
    curl_easy_setopt( curl, CURLOPT_COOKIEFILE, "cookies.txt" );
    curl_easy_setopt( curl, CURLOPT_FOLLOWLOCATION, 2L );
    curl_easy_setopt( curl, CURLOPT_VERBOSE, 0 );

    curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, curl_write );

    curl_easy_perform(curl);

    doc.Parse( rawBody.c_str() );


  };

  return 0;
};
