#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include <iostream>

using namespace std;

#define VERSION_NUMBER "1.0"

static int verbose_flag;

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

  int c;

  while (1) {
    static struct option long_options[] = {
               {"verbose", no_argument, &verbose_flag, 1},
               {"quiet",   no_argument, &verbose_flag, 0},
               {"login",     required_argument, 0, 'l'},
               {"password", required_argument, 0, 'p'},
               {"friend",  required_argument, 0, 'f'},
               {0, 0, 0, 0}
             };

    int option_index = 0;
    c = getopt_long (argc, argv, "pfl:::", long_options, &option_index);
    if (c == -1) {
      break;
    };
    switch (c) {

      case 0:
         /* flag */
        if (long_options[option_index].flag != 0)
          break;
        printf ("option %s", long_options[option_index].name);
        if (optarg)
          rintf (" with arg %s", optarg);
         printf ("\n");

         break;

      case 'l':
        puts ("option -l %s\n", optarg);
        break;

      case 'b':
        puts ("option -b\n");
        break;

      case 'c':
        printf ("option -c val `%s'\n", optarg);
        break;

      case 'd':
        printf ("option -d val `%s'\n", optarg);
        break;

      case 'f':
        printf ("option -f with value `%s'\n", optarg);
        break;

      case '?':
        break;

      default:
        abort ();
    };
  };
};
