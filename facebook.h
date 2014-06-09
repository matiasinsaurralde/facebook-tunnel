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

    double getFriendID( const char* );

    size_t curl_write( void *, size_t, size_t, void *);

    void sendPacketTo( double, const char*, int );

    void readPacketsFrom( double );

    void listClients();

    double friendID;

  protected:

    CURL* curl;

    void cleanup();

    std::string pageBuffer;

    std::string sendRequest( const char* );

    void extractFormData(GumboNode*, curl_httppost*, curl_httppost*);
    void extractFormData(GumboNode*, char*, char* );
    void extractLinks(GumboNode* );
    void extractClientList(GumboNode* );
    void extractMessages(GumboNode* );

    struct curl_httppost *loginForm;
    struct curl_httppost *loginFormLastPtr;

    struct curl_httppost *messageForm;
    struct curl_httppost *messageFormLastPtr;

    std::string rawCookies;
    struct curl_slist *cookies;

    char fb_dtsg[64];

    void fillCSRF();
    void fillChatCSRF( double );
};

#endif
