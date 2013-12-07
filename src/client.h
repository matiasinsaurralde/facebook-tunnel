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

#ifndef CLIENT_H
#define CLIENT_H

#include "worker.h"
#include "auth.h"

#include <vector>

class Client : public Worker
{
public:
    Client(int tunnelMtu, const char *deviceName, uint32_t serverIp,
           int maxPolls, const char *passphrase, uid_t uid, gid_t gid,
           bool changeEchoId, bool changeEchoSeq, uint32_t desiredIp);
    virtual ~Client();

    virtual void run();

    static const Worker::TunnelHeader::Magic magic;
protected:
    enum State
    {
        STATE_CLOSED,
        STATE_CONNECTION_REQUEST_SENT,
        STATE_CHALLENGE_RESPONSE_SENT,
        STATE_ESTABLISHED
    };

    virtual bool handleEchoData(const TunnelHeader &header, int dataLength, uint32_t realIp, bool reply, uint16_t id, uint16_t seq);
    virtual void handleTunData(int dataLength, uint32_t sourceIp, uint32_t destIp);
    virtual void handleTimeout();

    void handleDataFromServer(int length);

    void startPolling();

    void sendEchoToServer(int type, int dataLength);
    void sendChallengeResponse(int dataLength);
    void sendConnectionRequest();

    Auth auth;

    uint32_t serverIp;
    uint32_t clientIp;
    uint32_t desiredIp;

    int maxPolls;
    int pollTimeoutNr;

    bool changeEchoId, changeEchoSeq;

    uint16_t nextEchoId;
    uint16_t nextEchoSequence;

    State state;
};

#endif
