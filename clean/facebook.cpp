#include "facebook.h"

static void s_curl_write(void *buffer, size_t sz, size_t n, void *f) {
  static_cast<FacebookClient*>(f)->curl_write( buffer, sz, n, f );
};

static void curl_print_cookies(CURL *curl) {
  CURLcode res;
  struct curl_slist *cookies;
  struct curl_slist *nc;
  int i;

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

  curl_slist_free_all(cookies);
};

FacebookClient::FacebookClient() {

  curl = curl_easy_init();

  curl_easy_setopt( curl, CURLOPT_USERAGENT, "Mozilla/5.0 (X11; Ubuntu; Linux i686; rv:26.0) Gecko/20100101 Firefox/26.0");

  curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, s_curl_write );
  curl_easy_setopt( curl, CURLOPT_WRITEDATA, this );
  curl_easy_setopt( curl, CURLOPT_COOKIEFILE, "cookies.txt");

};

FacebookClient::~FacebookClient() {
};

size_t FacebookClient::curl_write( void *ptr, size_t size, size_t nmemb, void *stream) {
  pageBuffer.append( (char*)ptr, size*nmemb );
  return size*nmemb;
};

void FacebookClient::extractLinks(GumboNode* node) {

  if (node->type != GUMBO_NODE_ELEMENT) {
    return;
  }

  if (node->v.element.tag == GUMBO_TAG_A ) {
    GumboAttribute* hrefAttr;
    hrefAttr = gumbo_get_attribute( &node->v.element.attributes, "href" );

    std::string valueStr = std::string( hrefAttr->value );
    std::size_t match = valueStr.find( "messages/thread" );

    if( match == 1 ) {

      std::stringstream ss( valueStr );

      int index = 0;

      while( std::getline( ss, valueStr, '/' ) ) {
        std::istringstream iss( valueStr );
        if( index == 3 ) {
          std::istringstream iss( valueStr );
          iss >> this->friendID;
          break;
        };
        index++;
      };

    };
  };


  GumboVector* children = &node->v.element.children;

  for (int i = 0; i < children->length; ++i) {
    extractLinks(static_cast<GumboNode*>(children->data[i]) );
  };

};

void FacebookClient::extractFormData(GumboNode* node ) {

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

    std::size_t found = std::string( inputType->value ).find( "hidden" );

    if( found == 0 ) {

      curl_formadd(&loginForm,
               &loginFormLastPtr,
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

void FacebookClient::fillCSRF() {

  curl_easy_setopt( curl, CURLOPT_URL, "https://m.facebook.com/login.php" );
  curl_easy_perform( curl );

  GumboOutput* html = gumbo_parse( pageBuffer.c_str() );
  extractFormData( html->root );
  gumbo_destroy_output(&kGumboDefaultOptions, html);

  pageBuffer = "";

};

std::string FacebookClient::sendRequest( const char* url ) {

  return pageBuffer;
};

bool FacebookClient::authenticate( const char* login, const char* password ) {

  syslog( LOG_DEBUG, "authenticate()" );

  bool result = false;

  fillCSRF();

  curl_easy_setopt( curl, CURLOPT_URL, "https://m.facebook.com/login.php" );


  curl_formadd(&loginForm,
               &loginFormLastPtr,
               CURLFORM_COPYNAME, "email",
               CURLFORM_COPYCONTENTS, login,
               CURLFORM_END);

  curl_formadd(&loginForm,
               &loginFormLastPtr,
               CURLFORM_COPYNAME, "pass",
               CURLFORM_COPYCONTENTS, password,
               CURLFORM_END);

  curl_easy_setopt(curl, CURLOPT_HTTPPOST, loginForm);

  curl_easy_perform( curl );

  struct curl_slist *nc;

  int index;

  curl_easy_getinfo(curl, CURLINFO_COOKIELIST, &cookies);

  nc = cookies, index = 1;

  while (nc) {

    std::string cookieData = std::string( nc->data );

    int match = cookieData.find( "c_user" );

    rawCookies.append( cookieData );
    rawCookies.append( "\n" );

    if ( match != -1 ) {
      result = true;
    };
    nc = nc->next;
    index++;
  };

  if( result ) {

    syslog( LOG_DEBUG, "authenticate(): successful!" );

    std::ofstream cookieTempF("cookies.txt");
    cookieTempF << rawCookies;
    cookieTempF.close();

  } else {
    syslog( LOG_DEBUG, "authenticate(): failed." );
  };

  return result;

};

void FacebookClient::cleanup() {

  pageBuffer = "";
  curl_easy_cleanup( curl );

//  curl = NULL;

  curl = curl_easy_init();

  curl_easy_setopt( curl, CURLOPT_USERAGENT, "Mozilla/5.0 (X11; Ubuntu; Linux i686; rv:26.0) Gecko/20100101 Firefox/26.0");
  curl_easy_setopt( curl, CURLOPT_COOKIEFILE, "cookies.txt");

  curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, s_curl_write );
  curl_easy_setopt( curl, CURLOPT_WRITEDATA, this );

  curl_easy_setopt( curl, CURLOPT_FOLLOWLOCATION, 2L );

};

int FacebookClient::getFriendID( const char* name ) {

  cleanup();

  char url[128]; // fixme: is 128 enough?
  snprintf(url, sizeof(url), "https://m.facebook.com/%s", name );

  curl_easy_setopt( curl, CURLOPT_URL, url );

  curl_easy_perform( curl );

  GumboOutput* html;

  html = gumbo_parse( pageBuffer.c_str() );
  extractLinks( html->root );
  gumbo_destroy_output(&kGumboDefaultOptions, html);

  return friendID;

};

void FacebookClient::sendPacketTo( int someFriendID, const char* payload, int payloadLength ) {
  cleanup();

};
