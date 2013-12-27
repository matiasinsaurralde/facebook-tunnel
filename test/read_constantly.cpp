#include <stdio.h>
#include <curl/curl.h>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <string.h>

#include "gumbo.h"

std::list<int> packetIDs;

using namespace std;

string rawBody;

size_t curl_write( void *ptr, size_t size, size_t nmemb, void *stream) {
  rawBody.append( (char*)ptr, size*nmemb );
  return size*nmemb;
};

static std::string cleantext(GumboNode* node) {
  if (node->type == GUMBO_NODE_TEXT) {
    return std::string(node->v.text.text);
  } else if (node->type == GUMBO_NODE_ELEMENT &&
             node->v.element.tag != GUMBO_TAG_SCRIPT &&
             node->v.element.tag != GUMBO_TAG_STYLE) {
    std::string contents = "";
    GumboVector* children = &node->v.element.children;
    for (int i = 0; i < children->length; ++i) {
      const std::string text = cleantext((GumboNode*) children->data[i]);
      if (i != 0 && !text.empty()) {
        contents.append(" ");
      }
      contents.append(text);
    }

   string identChar = "|";
    if( contents.compare( 0, 1, identChar ) == 0 ) {
      return contents;
    } else {
      return "";
    };

  } else {
    return "";
  }
}

static void find_messages(GumboNode* node) {

  if (node->type != GUMBO_NODE_ELEMENT) {
    return;
  }

  GumboAttribute* classN;

  if (node->v.element.tag == GUMBO_TAG_SPAN ) {

    std::string messageBody = cleantext( node );

    if( messageBody.size() > 0 ) {

      messageBody.erase( 0, 1 );
      std::stringstream ss( messageBody );
/*      while( std::getline( ss, messageBody, ',' ) {
      };*/
      std::cout << messageBody << std::endl;
    };
  };

  GumboVector* children = &node->v.element.children;
  for (int i = 0; i < children->length; ++i) {
    find_messages(static_cast<GumboNode*>(children->data[i]) );
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
