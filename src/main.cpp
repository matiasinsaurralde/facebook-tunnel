/*
 *  Hans - IP over ICMP
 *  Copyright (C) 2009 Friedrich Schöller <hans@schoeller.se>
 *  
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *  
 */

#include "client.h"
#include "server.h"
#include "exception.h"

#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <stdlib.h>
#include <pwd.h>
#include <netdb.h>
// #include <uuid/uuid.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/socket.h>

void usage()
{
    printf(
        "Hans - IP over ICMP version 0.4.1\n\n"
        "RUN AS SERVER\n"
        "  hans -s network [-fvr] [-p password] [-u unprivileged_user] [-d tun_device] [-m reference_mtu] [-a ip]\n\n"
        "RUN AS CLIENT\n"
        "  hans -c server  [-fv]  [-p password] [-u unprivileged_user] [-d tun_device] [-m reference_mtu] [-w polls]\n\n"
        "ARGUMENTS\n"
        "  -s network    Run as a server with the given network address for the virtual interface.\n"
        "  -c server     Connect to a server.\n"
        "  -f            Run in foreground.\n"
        "  -v            Print debug information.\n"
        "  -r            Respond to ordinary pings. Only in server mode.\n"
        "  -p password   Use a password.\n"
        "  -u username   Set the user under which the program should run.\n"
        "  -d device     Use the given tun device.\n"
        "  -m mtu        Use this mtu to calculate the tunnel mtu.\n"
        "                The generated echo packets will not be bigger than this value.\n"
        "                Has to be the same on client and server. Defaults to 1500.\n"
        "  -w polls      Number of echo requests the client sends to the server for polling.\n"
        "                0 disables polling. Defaults to 10.\n"
        "  -i            Change the echo id for every echo request.\n"
        "  -q            Change the echo sequence number for every echo request.\n"
        "  -a ip         Try to get assigned the given tunnel ip address.\n"
    );
}

int main(int argc, char *argv[])
{
    const char *serverName;
    const char *userName = NULL;
    const char *password = "";
    const char *device = NULL;
    bool isServer = false;
    bool isClient = false;
    bool foreground = false;
    int mtu = 1500;
    int maxPolls = 10;
    uint32_t network = INADDR_NONE;
    uint32_t clientIp = INADDR_NONE;
    bool answerPing = false;
    uid_t uid = 0;
    gid_t gid = 0;
    bool changeEchoId = false;
    bool changeEchoSeq = false;
    bool verbose = false;
    
    openlog(argv[0], LOG_PERROR, LOG_DAEMON);

    int c;
    while ((c = getopt(argc, argv, "fru:d:p:s:c:m:w:qiva:")) != -1)
    {
        switch(c) {
            case 'f':
                foreground = true;
                break;
            case 'u':
                userName = optarg;
                break;
            case 'd':
                device = optarg;
                break;
            case 'p':
                password = strdup(optarg);
                memset(optarg, 0, strlen(optarg)); 
                break;
            case 'c':
                isClient = true;
                serverName = optarg;
                break;
            case 's':
                isServer = true;
                network = ntohl(inet_addr(optarg));
                if (network == INADDR_NONE)
                    printf("invalid network\n");
                break;
            case 'm':
                mtu = atoi(optarg);
                break;
            case 'w':
                maxPolls = atoi(optarg);
                break;
            case 'r':
                answerPing = true;
                break;
            case 'q':
                changeEchoSeq = true;
                break;
            case 'i':
                changeEchoId = true;
                break;
            case 'v':
                verbose = true;
                break;
            case 'a':
                clientIp = ntohl(inet_addr(optarg));
                break;
            default:
                usage();
                return 1;
        }
    }

    mtu -= Echo::headerSize() + Worker::headerSize();

    if (mtu < 68)
    {
        // RFC 791: Every internet module must be able to forward a datagram of 68 octets without further fragmentation.
        printf("mtu too small\n");
        return 1;
    }

    if ((isClient == isServer) ||
        (isServer && network == INADDR_NONE) ||
        (maxPolls < 0 || maxPolls > 255) ||
        (isServer && (changeEchoSeq || changeEchoId)))
    {
        usage();
        return 1;
    }

    if (userName != NULL)
    {
        passwd *pw = getpwnam(userName);

        if (pw != NULL)
        {
            uid = pw->pw_uid;
            gid = pw->pw_gid;
        }
        else
        {
            syslog(LOG_ERR, "user not found");
            return 1;
        }
    }

    if (!verbose)
        setlogmask(LOG_UPTO(LOG_INFO));

    try
    {
        Worker *worker;

        if (isServer)
        {
            worker = new Server(mtu, device, password, network, answerPing, uid, gid, 5000);
        }
        else
        {
            uint32_t serverIp = inet_addr(serverName);
            if (serverIp == INADDR_NONE)
            {
                struct hostent* he = gethostbyname(serverName);
                if (!he)
                {
                    syslog(LOG_ERR, "gethostbyname: %s", hstrerror(h_errno));
                    return 1;
                }

                serverIp = *(uint32_t *)he->h_addr;
            }

            worker = new Client(mtu, device, ntohl(serverIp), maxPolls, password, uid, gid, changeEchoId, changeEchoSeq, clientIp);
        }

        if (!foreground)
        {
            syslog(LOG_INFO, "detaching from terminal");
            daemon(0, 0);
        }

        worker->run();
    }
    catch (Exception e)
    {
        syslog(LOG_ERR, "%s", e.errorMessage());
        return 1;
    }

    return 0;
}
