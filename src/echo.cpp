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

#include "echo.h"
#include "exception.h"

#include <sys/socket.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <syslog.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

typedef ip IpHeader;

Echo::Echo(int maxPayloadSize)
{
    fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (fd == -1)
        throw Exception("creating icmp socket", true);

    bufferSize = maxPayloadSize + headerSize();
    sendBuffer = new char[bufferSize];
    receiveBuffer = new char[bufferSize];
}

Echo::~Echo()
{
    close(fd);

    delete[] sendBuffer;
    delete[] receiveBuffer;
}

int Echo::headerSize()
{
    return sizeof(IpHeader) + sizeof(EchoHeader);
}

void Echo::send(int payloadLength, uint32_t realIp, bool reply, uint16_t id, uint16_t seq)
{
    struct sockaddr_in target;
    target.sin_family = AF_INET;
    target.sin_addr.s_addr = htonl(realIp);

    if (payloadLength + sizeof(IpHeader) + sizeof(EchoHeader) > bufferSize)
        throw Exception("packet too big");

    EchoHeader *header = (EchoHeader *)(sendBuffer + sizeof(IpHeader));
    header->type = reply ? 0: 8;
    header->code = 0;
    header->id = htons(id);
    header->seq = htons(seq);
    header->chksum = 0;
    header->chksum = icmpChecksum(sendBuffer + sizeof(IpHeader), payloadLength + sizeof(EchoHeader));

    int result = sendto(fd, sendBuffer + sizeof(IpHeader), payloadLength + sizeof(EchoHeader), 0, (struct sockaddr *)&target, sizeof(struct sockaddr_in));
    if (result == -1)
        syslog(LOG_ERR, "error sending icmp packet: %s", strerror(errno));
}

int Echo::receive(uint32_t &realIp, bool &reply, uint16_t &id, uint16_t &seq)
{
    struct sockaddr_in source;
    int source_addr_len = sizeof(struct sockaddr_in);

    int dataLength = recvfrom(fd, receiveBuffer, bufferSize, 0, (struct sockaddr *)&source, (socklen_t *)&source_addr_len);
    if (dataLength == -1)
    {
        syslog(LOG_ERR, "error receiving icmp packet: %s", strerror(errno));
        return -1;
    }

    if (dataLength < sizeof(IpHeader) + sizeof(EchoHeader))
        return -1;

    EchoHeader *header = (EchoHeader *)(receiveBuffer + sizeof(IpHeader));
    if ((header->type != 0 && header->type != 8) || header->code != 0)
        return -1;

    realIp = ntohl(source.sin_addr.s_addr);
    reply = header->type == 0;
    id = ntohs(header->id);
    seq = ntohs(header->seq);

    return dataLength - sizeof(IpHeader) - sizeof(EchoHeader);
}

uint16_t Echo::icmpChecksum(const char *data, int length)
{
    uint16_t *data16 = (uint16_t *)data;
    uint32_t sum = 0;
    
    for (sum = 0; length > 1; length -= 2)
        sum += *data16++;
    if (length == 1)
        sum += *(unsigned char *)data16;
    
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return ~sum;
}
