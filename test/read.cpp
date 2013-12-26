#include <stdio.h>
#include <curl/curl.h>
#include <iostream>
#include <string.h>

#include "gumbo.h"

using namespace std;

string rawBody;

size_t curl_write( void *ptr, size_t size, size_t nmemb, void *stream) {
  rawBody.append( (char*)ptr, size*nmemb );
  return size*nmemb;
};

static void find_messages(GumboNode* node) {

  if (node->type != GUMBO_NODE_ELEMENT) {
    return;
  }

  GumboAttribute* classN;

  if (node->v.element.tag == GUMBO_TAG_DIV && ( classN = gumbo_get_attribute(&node->v.element.attributes, "class") ) ) {

    if( string( classN->value ).find("msg") != string::npos ) {
      printf("es un msg!\n");
    };

  };

  GumboVector* children = &node->v.element.children;
  for (int i = 0; i < children->length; ++i) {
    find_messages(static_cast<GumboNode*>(children->data[i]));
  }

};

int main( int argc, char *argv[] ) {

  CURL *curl;
  CURLcode res;
  char *rawHtml;
  char url[60];

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

    GumboOutput* html = gumbo_parse( rawBody.c_str() );
    find_messages( html->root );
    gumbo_destroy_output(&kGumboDefaultOptions, html);


  };

  return 0;
};
