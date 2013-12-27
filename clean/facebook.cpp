#include "facebook.h"

#include <syslog.h>

FacebookClient::FacebookClient() {
};

FacebookClient::~FacebookClient() {
};

bool FacebookClient::authenticate( const char* login, const char* password ) {
  syslog( LOG_DEBUG, "authenticate()" );
};
