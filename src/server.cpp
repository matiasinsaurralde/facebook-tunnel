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

#include "server.h"
#include "client.h"
#include "config.h"
#include "utility.h"

#include <string.h>
#include <arpa/inet.h>
#include <syslog.h>

using namespace std;

#define FIRST_ASSIGNED_IP_OFFSET 100

const Worker::TunnelHeader::Magic Server::magic("hans");

Server::Server(int tunnelMtu, const char *deviceName, const char *passphrase, uint32_t network, bool answerEcho, uid_t uid, gid_t gid, int pollTimeout)
    : Worker(tunnelMtu, deviceName, answerEcho, uid, gid), auth(passphrase)
{
    this->network = network & 0xffffff00;
    this->pollTimeout = pollTimeout;
    this->latestAssignedIpOffset = FIRST_ASSIGNED_IP_OFFSET - 1;

    tun->setIp(this->network + 1, this->network + 2, true);

    dropPrivileges();
}

Server::~Server()
{

}

void Server::handleUnknownClient(const TunnelHeader &header, int dataLength, uint32_t realIp, uint16_t echoId, uint16_t echoSeq)
{
    ClientData client;
    client.realIp = realIp;
    client.maxPolls = 1;

    pollReceived(&client, echoId, echoSeq);

    if (header.type != TunnelHeader::TYPE_CONNECTION_REQUEST || dataLength != sizeof(ClientConnectData))
    {
        syslog(LOG_DEBUG, "invalid request %s", Utility::formatIp(realIp).c_str());
        sendReset(&client);
        return;
    }

    ClientConnectData *connectData = (ClientConnectData *)echoReceivePayloadBuffer();

    client.maxPolls = connectData->maxPolls;
    client.state = ClientData::STATE_NEW;
    client.tunnelIp = reserveTunnelIp(connectData->desiredIp);

    syslog(LOG_DEBUG, "new client: %s (%s)\n", Utility::formatIp(client.realIp).c_str(), Utility::formatIp(client.tunnelIp).c_str());

    if (client.tunnelIp != 0)
    {
        client.challenge = auth.generateChallenge(CHALLENGE_SIZE);
        sendChallenge(&client);

        // add client to list
        clientList.push_back(client);
        clientRealIpMap[realIp] = clientList.size() - 1;
        clientTunnelIpMap[client.tunnelIp] = clientList.size() - 1;
    }
    else
    {
        syslog(LOG_WARNING, "server full");
        sendEchoToClient(&client, TunnelHeader::TYPE_SERVER_FULL, 0);
    }
}

void Server::sendChallenge(ClientData *client)
{
    syslog(LOG_DEBUG, "sending challenge to: %s\n", Utility::formatIp(client->realIp).c_str());

    memcpy(echoSendPayloadBuffer(), &client->challenge[0], client->challenge.size());
    sendEchoToClient(client, TunnelHeader::TYPE_CHALLENGE, client->challenge.size());

    client->state = ClientData::STATE_CHALLENGE_SENT;
}

void Server::removeClient(ClientData *client)
{
    syslog(LOG_DEBUG, "removing client: %s (%s)\n", Utility::formatIp(client->realIp).c_str(), Utility::formatIp(client->tunnelIp).c_str());

    releaseTunnelIp(client->tunnelIp);

    int nr = clientRealIpMap[client->realIp];

    clientRealIpMap.erase(client->realIp);
    clientTunnelIpMap.erase(client->tunnelIp);

    clientList.erase(clientList.begin() + nr);
}

void Server::checkChallenge(ClientData *client, int length)
{
    Auth::Response rightResponse = auth.getResponse(client->challenge);

    if (length != sizeof(Auth::Response) || memcmp(&rightResponse, echoReceivePayloadBuffer(), length) != 0)
    {
        syslog(LOG_DEBUG, "wrong challenge response\n");

        sendEchoToClient(client, TunnelHeader::TYPE_CHALLENGE_ERROR, 0);

        removeClient(client);
        return;
    }

    uint32_t *ip = (uint32_t *)echoSendPayloadBuffer();
    *ip = htonl(client->tunnelIp);

    sendEchoToClient(client, TunnelHeader::TYPE_CONNECTION_ACCEPT, sizeof(uint32_t));

    client->state = ClientData::STATE_ESTABLISHED;

    syslog(LOG_INFO, "connection established: %s", Utility::formatIp(client->realIp).c_str());
}

void Server::sendReset(ClientData *client)
{
    syslog(LOG_DEBUG, "sending reset: %s", Utility::formatIp(client->realIp).c_str());
    sendEchoToClient(client, TunnelHeader::TYPE_RESET_CONNECTION, 0);
}

bool Server::handleEchoData(const TunnelHeader &header, int dataLength, uint32_t realIp, bool reply, uint16_t id, uint16_t seq)
{
    if (reply)
        return false;

    if (header.magic != Client::magic)
        return false;

    ClientData *client = getClientByRealIp(realIp);
    if (client == NULL)
    {
        handleUnknownClient(header, dataLength, realIp, id, seq);
        return true;
    }

    pollReceived(client, id, seq);

    switch (header.type)
    {
        case TunnelHeader::TYPE_CONNECTION_REQUEST:
            if (client->state == ClientData::STATE_CHALLENGE_SENT)
            {
                sendChallenge(client);
                return true;
            }

            while (client->pollIds.size() > 1)
                client->pollIds.pop();

            syslog(LOG_DEBUG, "reconnecting %s", Utility::formatIp(realIp).c_str());
            sendReset(client);
            removeClient(client);
            return true;
        case TunnelHeader::TYPE_CHALLENGE_RESPONSE:
            if (client->state == ClientData::STATE_CHALLENGE_SENT)
            {
                checkChallenge(client, dataLength);
                return true;
            }
            break;
        case TunnelHeader::TYPE_DATA:
            if (client->state == ClientData::STATE_ESTABLISHED)
            {
                if (dataLength == 0)
                {
                    syslog(LOG_WARNING, "received empty data packet");
                    return true;
                }

                sendToTun(dataLength);
                return true;
            }
            break;
        case TunnelHeader::TYPE_POLL:
            return true;
    }

    syslog(LOG_DEBUG, "invalid packet from: %s, type: %d, state: %d", Utility::formatIp(realIp).c_str(), header.type, client->state);

    return true;
}

Server::ClientData *Server::getClientByTunnelIp(uint32_t ip)
{
    ClientIpMap::iterator clientMapIterator = clientTunnelIpMap.find(ip);
    if (clientMapIterator == clientTunnelIpMap.end())
        return NULL;
    
    return &clientList[clientMapIterator->second];
}

Server::ClientData *Server::getClientByRealIp(uint32_t ip)
{
    ClientIpMap::iterator clientMapIterator = clientRealIpMap.find(ip);
    if (clientMapIterator == clientRealIpMap.end())
        return NULL;
    
    return &clientList[clientMapIterator->second];
}

void Server::handleTunData(int dataLength, uint32_t sourceIp, uint32_t destIp)
{
    if (destIp == network + 255) // ignore broadcasts
        return;

    ClientData *client = getClientByTunnelIp(destIp);

    if (client == NULL)
    {
        syslog(LOG_DEBUG, "unknown client: %s\n", Utility::formatIp(destIp).c_str());
        return;
    }

    sendEchoToClient(client, TunnelHeader::TYPE_DATA, dataLength);
}

void Server::pollReceived(ClientData *client, uint16_t echoId, uint16_t echoSeq)
{
    unsigned int maxSavedPolls = client->maxPolls != 0 ? client->maxPolls : 1;

    client->pollIds.push(ClientData::EchoId(echoId, echoSeq));
    if (client->pollIds.size() > maxSavedPolls)
        client->pollIds.pop();
    DEBUG_ONLY(printf("poll -> %d\n", client->pollIds.size()));

    if (client->pendingPackets.size() > 0)
    {
        Packet &packet = client->pendingPackets.front();
        memcpy(echoSendPayloadBuffer(), &packet.data[0], packet.data.size());
        client->pendingPackets.pop();

        DEBUG_ONLY(printf("pending packet: %d bytes\n", packet.data.size()));
        sendEchoToClient(client, packet.type, packet.data.size());
    }

    client->lastActivity = now;
}

void Server::sendEchoToClient(ClientData *client, int type, int dataLength)
{
    if (client->maxPolls == 0)
    {
        sendEcho(magic, type, dataLength, client->realIp, true, client->pollIds.front().id, client->pollIds.front().seq);
        return;
    }

    if (client->pollIds.size() != 0)
    {
        ClientData::EchoId echoId = client->pollIds.front();
        client->pollIds.pop();

        DEBUG_ONLY(printf("sending -> %d\n", client->pollIds.size()));
        sendEcho(magic, type, dataLength, client->realIp, true, echoId.id, echoId.seq);
        return;
    }

    if (client->pendingPackets.size() == MAX_BUFFERED_PACKETS)
    {
        client->pendingPackets.pop();
        syslog(LOG_WARNING, "packet dropped to %s", Utility::formatIp(client->tunnelIp).c_str());
    }

    DEBUG_ONLY(printf("packet queued: %d bytes\n", dataLength));

    client->pendingPackets.push(Packet());
    Packet &packet = client->pendingPackets.back();
    packet.type = type;
    packet.data.resize(dataLength);
    memcpy(&packet.data[0], echoReceivePayloadBuffer(), dataLength);
}

void Server::releaseTunnelIp(uint32_t tunnelIp)
{
    usedIps.erase(tunnelIp);
}

void Server::handleTimeout()
{
    for (int i = 0; i < clientList.size(); i++)
    {
        ClientData *client = &clientList[i];

        if (client->lastActivity + KEEP_ALIVE_INTERVAL * 2 < now)
        {
            syslog(LOG_DEBUG, "client timeout: %s\n", Utility::formatIp(client->realIp).c_str());
            removeClient(client);
            i--;
        }
    }

    setTimeout(KEEP_ALIVE_INTERVAL);
}

uint32_t Server::reserveTunnelIp(uint32_t desiredIp)
{
    if (desiredIp > network + 1 && desiredIp < network + 255 && !usedIps.count(desiredIp))
    {
        usedIps.insert(desiredIp);
        return desiredIp;
    }

    bool ipAvailable = false;

    for (int i = 0; i < 255 - FIRST_ASSIGNED_IP_OFFSET; i++)
    {
        latestAssignedIpOffset++;
        if (latestAssignedIpOffset == 255)
            latestAssignedIpOffset = FIRST_ASSIGNED_IP_OFFSET;

        if (!usedIps.count(network + latestAssignedIpOffset))
        {
            ipAvailable = true;
            break;
        }
    }

    if (!ipAvailable)
        return 0;

    usedIps.insert(network + latestAssignedIpOffset);
    return network + latestAssignedIpOffset;
}

void Server::run()
{
    setTimeout(KEEP_ALIVE_INTERVAL);

    Worker::run();
}
