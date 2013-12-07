/*  
    VTun - Virtual Tunnel over TCP/IP network.

    Copyright (C) 1998-2000  Maxim Krasnyansky <max_mk@yahoo.com>

    VTun has been derived from VPPP package by Maxim Krasnyansky. 

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
 */

/*
 * tun_dev.c,v 1.3.2.1.2.1 2006/11/16 04:04:52 mtbishop Exp
 */ 

/* #include "config.h" */

#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>

#include <sys/types.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if_tun.h>

/* #include "vtun.h"
#include "lib.h" */

/* 
 * Allocate TUN device, returns opened fd. 
 * Stores dev name in the first arg(must be large enough).
 */  
int tun_open(char *dev)
{
    char tunname[14];
    int i, fd = -1;

    if( *dev ){
       sprintf(tunname, "/dev/%s", dev);
       fd = open(tunname, O_RDWR);
    } else {
       for(i=0; i < 255; i++){
          sprintf(tunname, "/dev/tun%d", i);
          /* Open device */
          if( (fd=open(tunname, O_RDWR)) > 0 ){
             sprintf(dev, "tun%d", i);
             break;
          }
       }
    }
    return fd;
}

int tun_close(int fd, char *dev)
{
    return close(fd);
}

/* Read/write frames from TUN device */
int tun_write(int fd, char *buf, int len)
{
    u_int32_t type = htonl(AF_INET);
    struct iovec iv[2];
    
    iv[0].iov_base = &type;
    iv[0].iov_len = sizeof(type);
    iv[1].iov_base = buf;
    iv[1].iov_len = len;

    return writev(fd, iv, 2);
}

int tun_read(int fd, char *buf, int len)
{
    struct iovec iv[2];
    u_int32_t type;
    register int rlen;

    iv[0].iov_base = &type;
    iv[0].iov_len = sizeof(type);
    iv[1].iov_base = buf;
    iv[1].iov_len = len;

    if( (rlen = readv(fd, iv, 2)) > 0 )
       return rlen - sizeof(type);
    else
       return rlen;
}
