#ifndef FB_H
#define FB_H
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

    size_t curl_write( void *, size_t, size_t, void *);

    void sendPacketTo( int, const char*, int );

    int friendID;

  protected:

    CURL* curl;

    void cleanup();

    std::string pageBuffer;

    std::string sendRequest( const char* );

    void extractFormData(GumboNode*, curl_httppost*, curl_httppost*);
    void extractFormData(GumboNode*, char*, char* );
    void extractLinks(GumboNode* );

    struct curl_httppost *loginForm = NULL;
    struct curl_httppost *loginFormLastPtr = NULL;

    struct curl_httppost *messageForm = NULL;
    struct curl_httppost *messageFormLastPtr = NULL;

    std::string rawCookies;
    struct curl_slist *cookies;

    char fb_dtsg[4];

    void fillCSRF();
    void fillChatCSRF( int );
};

#endif
