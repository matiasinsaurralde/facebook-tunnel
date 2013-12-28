#include <iostream>
#include <fstream>
#include <sstream>
#include <curl/curl.h>
#include <syslog.h>
#include <string.h>

#include "gumbo.h"

class FacebookClient {

  public:

    FacebookClient();
    ~FacebookClient();

    bool authenticate( const char*, const char* );

    int getFriendID( const char* );

  public:
    size_t curl_write( void *, size_t, size_t, void *);

  protected:

    CURL* curl;

    void cleanup();

    std::string pageBuffer;

    std::string sendRequest( const char* );

    void extractFormData(GumboNode*);
    void extractLinks(GumboNode*);

    struct curl_httppost *loginForm = NULL;
    struct curl_httppost *loginFormLastPtr = NULL;

    std::string rawCookies;
    struct curl_slist *cookies;

    int friendID;

    void fillCSRF();
};
