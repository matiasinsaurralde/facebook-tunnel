#include <iostream>
#include <curl/curl.h>
#include <syslog.h>
#include <string.h>

#include "gumbo.h"

class FacebookClient {

  public:

    FacebookClient();
    ~FacebookClient();

    bool authenticate( const char* login, const char* password );

  public:
    size_t curl_write( void *ptr, size_t size, size_t nmemb, void *stream);

  protected:

    CURL* curl;

    std::string pageBuffer;

    std::string sendRequest( const char* url );

    void extractFormData(GumboNode*);

    struct curl_httppost *loginForm = NULL;
    struct curl_httppost *loginFormLastPtr = NULL;

    std::string rawCookies;
    struct curl_slist *cookies;

    void fillCSRF();
};
