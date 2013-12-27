#include <stdio.h>
#include <curl/curl.h>
#include <sys/time.h>
#include <iostream>
#include <string>

#include "gumbo.h"

using namespace std;

string rawBody;

struct curl_httppost *formpost=NULL;
struct curl_httppost *lastptr=NULL;

static size_t curl_write( void *ptr, size_t size, size_t nmemb, void *stream) {
  rawBody.append( (char*)ptr, size*nmemb );
  return size*nmemb;
};

static void curl_print_cookies(CURL *curl)
{
  CURLcode res;
  struct curl_slist *cookies;
  struct curl_slist *nc;
  int i;

  printf("Cookies, curl knows:\n");
  res = curl_easy_getinfo(curl, CURLINFO_COOKIELIST, &cookies);
  if (res != CURLE_OK) {
    fprintf(stderr, "Curl curl_easy_getinfo failed: %s\n", curl_easy_strerror(res));
  }
  nc = cookies, i = 1;
  while (nc) {
    printf("%s\n", nc->data);
    nc = nc->next;
    i++;
  }
  if (i == 1) {
    printf("(none)\n");
  }
  curl_slist_free_all(cookies);
}

static void extractFormData(GumboNode* node ) {

  if (node->type != GUMBO_NODE_ELEMENT) {
    return;
  }

  if (node->v.element.tag == GUMBO_TAG_INPUT ) {
    GumboAttribute* inputType;
    GumboAttribute* inputName;
    GumboAttribute* inputValue;

    inputType = gumbo_get_attribute( &node->v.element.attributes, "type" );
    inputName = gumbo_get_attribute( &node->v.element.attributes, "name" );
    inputValue = gumbo_get_attribute( &node->v.element.attributes, "value" );

    std::size_t found = string( inputType->value ).find( "hidden" );

    if( found == 0 ) {
      // printf("%s = %s\t\t( type = %s )\n", inputName->value, inputValue->value, inputType->value );
      curl_formadd(&formpost,
               &lastptr,
               CURLFORM_COPYNAME, inputName->name,
               CURLFORM_COPYCONTENTS, inputValue->value,
               CURLFORM_END);
    };

  };

  GumboVector* children = &node->v.element.children;
  for (int i = 0; i < children->length; ++i) {
    extractFormData(static_cast<GumboNode*>(children->data[i]) );
  }

};

void setFormData( ) {

  CURL *tokenCurl;

  tokenCurl = curl_easy_init();

  curl_easy_setopt( tokenCurl, CURLOPT_URL, "https://m.facebook.com/login.php" );

  curl_easy_setopt( tokenCurl, CURLOPT_USERAGENT, "Mozilla/5.0 (X11; Ubuntu; Linux i686; rv:26.0) Gecko/20100101 Firefox/26.0");

  curl_easy_setopt( tokenCurl, CURLOPT_WRITEFUNCTION, curl_write );

  curl_easy_perform( tokenCurl );

  GumboOutput* html = gumbo_parse( rawBody.c_str() );
  extractFormData( html->root );
  gumbo_destroy_output(&kGumboDefaultOptions, html);

};

int main( int argc, char *argv[] ) {

  std::cout << "Logging in w/ credentials: " << argv[1] << " and password ***" << std::endl;

  setFormData();

  CURL *curl;
  CURLcode res;
  char *rawHtml;

  curl = curl_easy_init();

  if(curl) {

    curl_easy_setopt(curl, CURLOPT_URL, "https://m.facebook.com/login.php" );

    curl_easy_setopt( curl, CURLOPT_USERAGENT, "Mozilla/5.0 (X11; Ubuntu; Linux i686; rv:26.0) Gecko/20100101 Firefox/26.0");
    curl_easy_setopt( curl, CURLOPT_FOLLOWLOCATION, 2L );
    curl_easy_setopt( curl, CURLOPT_VERBOSE, 0 );
    curl_easy_setopt( curl, CURLOPT_COOKIEFILE, "");
    curl_easy_setopt( curl, CURLOPT_COOKIEJAR, "yes.txt" );

    curl_formadd(&formpost,
               &lastptr,
               CURLFORM_COPYNAME, "email",
               CURLFORM_COPYCONTENTS, argv[1],
               CURLFORM_END);

    curl_formadd(&formpost,
               &lastptr,
               CURLFORM_COPYNAME, "pass",
               CURLFORM_COPYCONTENTS, argv[2],
               CURLFORM_END);

    curl_formadd(&formpost,
               &lastptr,
               CURLFORM_COPYNAME, "lsd",
               CURLFORM_COPYCONTENTS, "AVo80vaw",
               CURLFORM_END);

    curl_formadd(&formpost,
               &lastptr,
               CURLFORM_COPYNAME, "version",
               CURLFORM_COPYCONTENTS, "1",
               CURLFORM_END);

    curl_formadd(&formpost,
               &lastptr,
               CURLFORM_COPYNAME, "ajax",
               CURLFORM_COPYCONTENTS, "0",
               CURLFORM_END);

    curl_formadd(&formpost,
               &lastptr,
               CURLFORM_COPYNAME, "width",
               CURLFORM_COPYCONTENTS, "0",
               CURLFORM_END);

    curl_formadd(&formpost,
               &lastptr,
               CURLFORM_COPYNAME, "pxr",
               CURLFORM_COPYCONTENTS, "0",
               CURLFORM_END);

    curl_formadd(&formpost,
               &lastptr,
               CURLFORM_COPYNAME, "gps",
               CURLFORM_COPYCONTENTS, "0",
               CURLFORM_END);

    curl_formadd(&formpost,
               &lastptr,
               CURLFORM_COPYNAME, "signup_layout",
               CURLFORM_COPYCONTENTS, "layout|bottom_clean||wider_form||prmnt_btn|special||st|create||header_crt_acct_button||hdbtn_color|green||signupinstr||launched_Mar3",
               CURLFORM_END);

    struct timeval ts;
    gettimeofday( &ts, NULL );
    char timestampStr[30];

    snprintf( timestampStr, 30, "timestamp: %ld", ts.tv_sec  );

    curl_formadd(&formpost,
               &lastptr,
               CURLFORM_COPYNAME, "m_ts",
               CURLFORM_COPYCONTENTS, timestampStr,
               CURLFORM_END);

    curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
    curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, curl_write );

    curl_easy_perform(curl);

    curl_print_cookies( curl );
  };

  return 0;
};
