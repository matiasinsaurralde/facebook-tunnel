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
 * tun_dev.c,v 1.2.2.1.2.1 2006/11/16 04:04:40 mtbishop Exp
 */ 

/* #include "config.h" */

#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>

/* #include "vtun.h"
#include "lib.h" */

/* 
 * Allocate TUN device, returns opened fd. 
 * Stores dev name in the first arg(must be large enough).
 */  
int tun_open(char *dev)
{
    char tunname[14];
    int i, fd;

    if( *dev ) {
       sprintf(tunname, "/dev/%s", dev);
       return open(tunname, O_RDWR);
    }

    for(i=0; i < 255; i++){
       sprintf(tunname, "/dev/tun%d", i);
       /* Open device */
       if( (fd=open(tunname, O_RDWR)) > 0 ){
          sprintf(dev, "tun%d", i);
          return fd;
       }
    }
    return -1;
}

int tun_close(int fd, char *dev)
{
    return close(fd);
}

/* Read/write frames from TUN device */
int tun_write(int fd, char *buf, int len)
{
    return write(fd, buf, len);
}

int tun_read(int fd, char *buf, int len)
{
    return read(fd, buf, len);
}
