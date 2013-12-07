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

#ifndef SERVER_H
#define SERVER_H

#include "worker.h"
#include "auth.h"

#include <map>
#include <queue>
#include <vector>
#include <set>

class Server : public Worker
{
public:
    Server(int tunnelMtu, const char *deviceName, const char *passphrase, uint32_t network, bool answerEcho, uid_t uid, gid_t gid, int pollTimeout);
    virtual ~Server();

    // change some time:
    // struct __attribute__ ((__packed__)) ClientConnectData
    struct ClientConnectData
    {
        uint8_t maxPolls;
        uint32_t desiredIp;
    };

    static const Worker::TunnelHeader::Magic magic;

protected:
    struct Packet
    {
        int type;
        std::vector<char> data;
    };

    struct ClientData
    {
        enum State
        {
            STATE_NEW,
            STATE_CHALLENGE_SENT,
            STATE_ESTABLISHED
        };

        struct EchoId
        {
            EchoId(uint16_t id, uint16_t seq) { this->id = id; this->seq = seq; }

            uint16_t id;
            uint16_t seq;
        };

        uint32_t realIp;
        uint32_t tunnelIp;

        std::queue<Packet> pendingPackets;

        int maxPolls;
        std::queue<EchoId> pollIds;
        Time lastActivity;

        State state;

        Auth::Challenge challenge;
    };

    typedef std::vector<ClientData> ClientList;
    typedef std::map<uint32_t, int> ClientIpMap;

    virtual bool handleEchoData(const TunnelHeader &header, int dataLength, uint32_t realIp, bool reply, uint16_t id, uint16_t seq);
    virtual void handleTunData(int dataLength, uint32_t sourceIp, uint32_t destIp);
    virtual void handleTimeout();

    virtual void run();

    void serveTun(ClientData *client);

    void handleUnknownClient(const TunnelHeader &header, int dataLength, uint32_t realIp, uint16_t echoId, uint16_t echoSeq);
    void removeClient(ClientData *client);

    void sendChallenge(ClientData *client);
    void checkChallenge(ClientData *client, int dataLength);
    void sendReset(ClientData *client);

    void sendEchoToClient(ClientData *client, int type, int dataLength);

    void pollReceived(ClientData *client, uint16_t echoId, uint16_t echoSeq);

    uint32_t reserveTunnelIp(uint32_t desiredIp);
    void releaseTunnelIp(uint32_t tunnelIp);

    ClientData *getClientByTunnelIp(uint32_t ip);
    ClientData *getClientByRealIp(uint32_t ip);

    Auth auth;

    uint32_t network;
    std::set<uint32_t> usedIps;
    uint32_t latestAssignedIpOffset;

    Time pollTimeout;

    ClientList clientList;
    ClientIpMap clientRealIpMap;
    ClientIpMap clientTunnelIpMap;
};

#endif
