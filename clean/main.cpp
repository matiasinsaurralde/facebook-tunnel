#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <syslog.h>

#include <iostream>

#include "facebook.h"
#include "tun.h"
#include "utils.h"

/* tcp/ip */

#include <arpa/inet.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>

typedef ip IpHeader;
typedef tcphdr TcpHeader;

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

  openlog(argv[0], LOG_PERROR, LOG_DAEMON);

  if (!verboseFlag)
    setlogmask(LOG_UPTO(LOG_INFO));

  if( login == NULL || password == NULL ) {
    cout << "Credentials are required!\n" << endl;
    exit(1);
  };

  FacebookClient* facebook = new FacebookClient();

/*  bool authSuccess = facebook->authenticate( login, password );

  if( !authSuccess ) {
    cout << "Authentication error!" << endl;
    exit(1);
  };

  syslog( LOG_DEBUG, "Setting mode: %d", mode );

  if( friendName == NULL ) {
    mode = SERVER_MODE;
  } else {
    mode = CLIENT_MODE;
    syslog( LOG_DEBUG, "Looking up friend's ID..." );
    int friendID = facebook->getFriendID( friendName );
    if( friendID == 0 ) {
      syslog( LOG_ERR, "Invalid friend" );
      exit(1);
    };
  };*/

  int alive = true;

  int mtu = 1500;

  char device[ 8 ];
  strcpy( device, "tun0" );

  Tun* tunnel = new Tun( device, mtu, mode );

  int length = 0;
  char buf[ mtu ];

  cout << "Ready and waiting for packets." << endl;

  while( alive ) {

    length = tunnel->read( buf );

    if( length > 0 ) {

      char serialized_packet[ 30 + 10 + 4 + sizeof( buf ) ];

      IpHeader *header = (IpHeader *)(buf + 4  );
//    TcpHeader *tcpheader = (TcpHeader *)( buf + sizeof(struct ip) );
      TcpHeader *tcpheader = (TcpHeader *)( buf + 4 + sizeof(struct ip) );

      string source_ip, dest_ip;
      source_ip = Utils::formatIp( ntohl( header->ip_src.s_addr ) );
      dest_ip = Utils::formatIp( ntohl( header->ip_dst.s_addr ) );

      sprintf( serialized_packet, "|%s,%d,%s,%d", source_ip.c_str(), htons( tcpheader->source ), dest_ip.c_str(), htons( tcpheader->dest )  );

      puts( serialized_packet );

      printf("paquete! %d\n", length );
    };

  };

  return 0;

};
