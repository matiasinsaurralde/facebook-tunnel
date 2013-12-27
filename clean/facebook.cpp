#include "facebook.h"

static void s_curl_write(void *buffer, size_t sz, size_t n, void *f) {
  static_cast<FacebookClient*>(f)->curl_write( buffer, sz, n, f );
};

FacebookClient::FacebookClient() {

  curl = curl_easy_init();

  curl_easy_setopt( curl, CURLOPT_USERAGENT, "Mozilla/5.0 (X11; Ubuntu; Linux i686; rv:26.0) Gecko/20100101 Firefox/26.0");

  curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, s_curl_write );
  curl_easy_setopt( curl, CURLOPT_WRITEDATA, this );

};

FacebookClient::~FacebookClient() {
};

size_t FacebookClient::curl_write( void *ptr, size_t size, size_t nmemb, void *stream) {
  pageBuffer.append( (char*)ptr, size*nmemb );
  return size*nmemb;
};

void FacebookClient::fillCSRF() {
};

std::string FacebookClient::sendRequest( const char* url ) {

  pageBuffer = "";

  curl_easy_setopt( curl, CURLOPT_URL, url );

  curl_easy_perform( curl );

  return pageBuffer;
};

bool FacebookClient::authenticate( const char* login, const char* password ) {

  syslog( LOG_DEBUG, "authenticate()" );

  fillCSRF();

  sendRequest( "https://matias.insaurral.de/" );

  printf("%s\n", pageBuffer.c_str() );

};
