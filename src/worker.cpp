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

#include "worker.h"
#include "tun.h"
#include "exception.h"
#include "config.h"

#include <string.h>
#include <syslog.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/select.h>

using namespace std;

Worker::TunnelHeader::Magic::Magic(const char *magic)
{
    memset(data, 0, sizeof(data));
    strncpy(data, magic, sizeof(data));
}

bool Worker::TunnelHeader::Magic::operator==(const Magic &other) const
{
    return memcmp(data, other.data, sizeof(data)) == 0;
}

bool Worker::TunnelHeader::Magic::operator!=(const Magic &other) const
{
    return memcmp(data, other.data, sizeof(data)) != 0;
}

Worker::Worker(int tunnelMtu, const char *deviceName, bool answerEcho, uid_t uid, gid_t gid)
{
    this->tunnelMtu = tunnelMtu;
    this->answerEcho = answerEcho;
    this->uid = uid;
    this->gid = gid;
    this->privilegesDropped = false;

    echo = NULL;
    tun = NULL;

    try
    {
        echo = new Echo(tunnelMtu + sizeof(TunnelHeader));
        tun = new Tun(deviceName, tunnelMtu);
    }
    catch (...)
    {
        delete echo;
        delete tun;

        throw;
    }
}

Worker::~Worker()
{
    delete echo;
    delete tun;
}

void Worker::sendEcho(const TunnelHeader::Magic &magic, int type, int length, uint32_t realIp, bool reply, uint16_t id, uint16_t seq)
{
    if (length > payloadBufferSize())
        throw Exception("packet too big");

    TunnelHeader *header = (TunnelHeader *)echo->sendPayloadBuffer();
    header->magic = magic;
    header->type = type;

    DEBUG_ONLY(printf("sending: type %d, length %d, id %d, seq %d\n", type, length, id, seq));

    echo->send(length + sizeof(TunnelHeader), realIp, reply, id, seq);
}

void Worker::sendToTun(int length)
{
    tun->write(echoReceivePayloadBuffer(), length);
}

void Worker::setTimeout(Time delta)
{
    nextTimeout = now + delta;
}

void Worker::run()
{
    now = Time::now();
    alive = true;

    int maxFd = echo->getFd() > tun->getFd() ? echo->getFd() : tun->getFd();

    while (alive)
    {
        fd_set fs;
        Time timeout;

        FD_ZERO(&fs);
        FD_SET(tun->getFd(), &fs);
        FD_SET(echo->getFd(), &fs);

        if (nextTimeout != Time::ZERO)
        {
            timeout = nextTimeout - now;
            if (timeout < Time::ZERO)
                timeout = Time::ZERO;
        }

        // wait for data or timeout
        int result = select(maxFd + 1 , &fs, NULL, NULL, nextTimeout != Time::ZERO ? &timeout.getTimeval() : NULL);
        if (result == -1)
            throw Exception("select", true);
        now = Time::now();

        // timeout
        if (result == 0)
        {
            nextTimeout = Time::ZERO;
            handleTimeout();
            continue;
        }

        // icmp data
        if (FD_ISSET(echo->getFd(), &fs))
        {
            bool reply;
            uint16_t id, seq;
            uint32_t ip;

            int dataLength = echo->receive(ip, reply, id, seq);
            if (dataLength != -1)
            {
                bool valid = dataLength >= sizeof(TunnelHeader);

                if (valid)
                {
                    TunnelHeader *header = (TunnelHeader *)echo->receivePayloadBuffer();

                    DEBUG_ONLY(printf("received: type %d, length %d, id %d, seq %d\n", header->type, dataLength - sizeof(TunnelHeader), id, seq));

                    valid = handleEchoData(*header, dataLength - sizeof(TunnelHeader), ip, reply, id, seq);
                }

                if (!valid && !reply && answerEcho)
                {
                    memcpy(echo->sendPayloadBuffer(), echo->receivePayloadBuffer(), dataLength);
                    echo->send(dataLength, ip, true, id, seq);
                }
            }
        }

        // data from tun
        if (FD_ISSET(tun->getFd(), &fs))
        {
            uint32_t sourceIp, destIp;

            int dataLength = tun->read(echoSendPayloadBuffer(), sourceIp, destIp);

            if (dataLength == 0)
                throw Exception("tunnel closed");

            if (dataLength != -1)
                handleTunData(dataLength, sourceIp, destIp);
        }
    }
}

void Worker::dropPrivileges()
{
    if (uid <= 0 || privilegesDropped)
        return;

    syslog(LOG_INFO, "dropping privileges");

    if (setgid(gid) == -1)
        throw Exception("setgid", true);

    if (setuid(uid) == -1)
        throw Exception("setuid", true);

    privilegesDropped = true;
}
