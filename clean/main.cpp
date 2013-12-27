#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <syslog.h>

#include <iostream>

#include "facebook.h"

using namespace std;

#define VERSION_NUMBER "1.0"
#define SERVER_MODE 0
#define CLIENT_MODE 1

static int verboseFlag;

static int mode;

void help() {
  cout << "facebook-tunnel " << VERSION_NUMBER << endl << endl;
  cout << "This program allows you to send TCP packets over Facebook chat. Login credentials can be specified as arguments." << endl;
  cout << "Also when running in client mode, your friend's nickname (your \"server\") is needed." << endl << endl;
  cout << "Running as client (example):" << endl << endl;
  cout << "$ facebook-tunnel -login luke@skywalker.com -password theforce -friend r2d2" << endl << endl;
  cout << "Running as server:" << endl << endl;
  cout << "$ facebook-tunnel -login r2@d2.com -password droidpower" << endl;

  cout << endl;
};

int main( int argc, char **argv ) {

  const char* login = NULL;
  const char* password = NULL;
  const char* friendName = NULL;

  int c;

  if( argc < 4 ) {
    help();
    exit(1);
  };

  while (1) {
    static struct option long_options[] = {
               {"verbose", no_argument, &verboseFlag, 1},
               {"quiet",   no_argument, &verboseFlag, 0},
               {"login",     required_argument, 0, 'l'},
               {"password", required_argument, 0, 'p'},
               {"friend",  required_argument, 0, 'f'},
               {0, 0, 0, 0}
             };

    int option_index = 0;
    c = getopt_long (argc, argv, "f:p:l:", long_options, &option_index);

    if (c == -1 ) {
      break;
    };

    switch (c) {

      case 0:

        if (long_options[option_index].flag != 0)
          break;

        if (optarg)

         break;

      case 'l':
        login = optarg;
        break;

      case 'p':
        password = optarg;
        break;

      case 'f':
        friendName = optarg;
        break;

      case '?':
        break;

      default:
        abort ();
    };

  };

  if( login == NULL || password == NULL ) {
    cout << "Credentials are required!\n" << endl;
    exit(1);
  };

  if( friendName == NULL ) {
    mode = SERVER_MODE;
  } else {
    mode = CLIENT_MODE;
  };

  openlog(argv[0], LOG_PERROR, LOG_DAEMON);

 if (!verboseFlag)
   setlogmask(LOG_UPTO(LOG_INFO));


  syslog( LOG_DEBUG, "Setting mode: %d", mode );

  FacebookClient* facebook;

  facebook->authenticate( login, password );

};