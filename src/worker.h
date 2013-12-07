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

#ifndef WORKER_H
#define WORKER_H

#include "time.h"
#include "echo.h"
#include "tun.h"

#include <string>
#include <sys/types.h>

class Worker
{
public:
    Worker(int tunnelMtu, const char *deviceName, bool answerEcho, uid_t uid, gid_t gid);
    virtual ~Worker();

    virtual void run();

    static int headerSize() { return sizeof(TunnelHeader); }

protected:
    struct TunnelHeader
    {
        struct Magic
        {
            Magic() { }
            Magic(const char *magic);

            bool operator==(const Magic &other) const;
            bool operator!=(const Magic &other) const;

            char data[4];
        };

        Magic magic;
        uint8_t type;

        enum Type
        {
            TYPE_RESET_CONNECTION    = 1,
            TYPE_CONNECTION_REQUEST    = 2,
            TYPE_CHALLENGE            = 3,
            TYPE_CHALLENGE_RESPONSE    = 4,
            TYPE_CONNECTION_ACCEPT    = 5,
            TYPE_CHALLENGE_ERROR    = 6,
            TYPE_DATA                = 7,
            TYPE_POLL                = 8,
            TYPE_SERVER_FULL        = 9
        };
    }; // size = 5

    virtual bool handleEchoData(const TunnelHeader &header, int dataLength, uint32_t realIp, bool reply, uint16_t id, uint16_t seq) { return true; }
    virtual void handleTunData(int dataLength, uint32_t sourceIp, uint32_t destIp) { } // to echoSendPayloadBuffer
    virtual void handleTimeout() { }

    void sendEcho(const TunnelHeader::Magic &magic, int type, int length, uint32_t realIp, bool reply, uint16_t id, uint16_t seq);
    void sendToTun(int length); // from echoReceivePayloadBuffer

    void setTimeout(Time delta);

    char *echoSendPayloadBuffer() { return echo->sendPayloadBuffer() + sizeof(TunnelHeader); }
    char *echoReceivePayloadBuffer() { return echo->receivePayloadBuffer() + sizeof(TunnelHeader); }

    int payloadBufferSize() { return tunnelMtu; }

    void dropPrivileges();

    Echo *echo;
    Tun *tun;
    bool alive;
    bool answerEcho;
    int tunnelMtu;
    int maxTunnelHeaderSize;
    uid_t uid;
    gid_t gid;

    bool privilegesDropped;

    Time now;
private:
    int readIcmpData(int *realIp, int *id, int *seq);

    Time nextTimeout;
};

#endif
