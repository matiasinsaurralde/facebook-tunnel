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
 * $Id: tun_dev.c,v 1.4 2006/12/11 10:45:35 mtbishop Exp $
 */ 

/* #include "config.h" */

#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <errno.h>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/if.h>

/* #include "vtun.h"
#include "lib.h" */

/* 
 * Allocate TUN device, returns opened fd. 
 * Stores dev name in the first arg(must be large enough).
 */  
static int tun_open_common0(char *dev, int istun)
{
    char tunname[14];
    int i, fd, err;

    if( *dev ) {
       sprintf(tunname, "/dev/%s", dev);
       return open(tunname, O_RDWR);
    }

    sprintf(tunname, "/dev/%s", istun ? "tun" : "tap");
    err = 0;
    for(i=0; i < 255; i++){
       sprintf(tunname + 8, "%d", i);
       /* Open device */
       if( (fd=open(tunname, O_RDWR)) > 0 ) {
          strcpy(dev, tunname + 5);
          return fd;
       }
       else if (errno != ENOENT)
          err = errno;
       else if (i)    /* don't try all 256 devices */
          break;
    }
    if (err)
    errno = err;
    return -1;
}

#ifdef HAVE_LINUX_IF_TUN_H /* New driver support */
#include <linux/if_tun.h>

#ifndef OTUNSETNOCSUM
/* pre 2.4.6 compatibility */
#define OTUNSETNOCSUM  (('T'<< 8) | 200) 
#define OTUNSETDEBUG   (('T'<< 8) | 201) 
#define OTUNSETIFF     (('T'<< 8) | 202) 
#define OTUNSETPERSIST (('T'<< 8) | 203) 
#define OTUNSETOWNER   (('T'<< 8) | 204)
#endif

static int tun_open_common(char *dev, int istun)
{
    struct ifreq ifr;
    int fd;

    if ((fd = open("/dev/net/tun", O_RDWR)) < 0)
       return tun_open_common0(dev, istun);

    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = (istun ? IFF_TUN : IFF_TAP) | IFF_NO_PI;
    if (*dev)
       strncpy(ifr.ifr_name, dev, IFNAMSIZ);

    if (ioctl(fd, TUNSETIFF, (void *) &ifr) < 0) {
       if (errno == EBADFD) {
      /* Try old ioctl */
       if (ioctl(fd, OTUNSETIFF, (void *) &ifr) < 0) 
         goto failed;
       } else
          goto failed;
    } 

    strcpy(dev, ifr.ifr_name);
    return fd;

failed:
    close(fd);
    return -1;
}

#else

# define tun_open_common(dev, type) tun_open_common0(dev, type)

#endif /* New driver support */

int tun_open(char *dev) { return tun_open_common(dev, 1); }
int tap_open(char *dev) { return tun_open_common(dev, 0); }

int tun_close(int fd, char *dev) { return close(fd); }
int tap_close(int fd, char *dev) { return close(fd); }

/* Read/write frames from TUN device */
int tun_write(int fd, char *buf, int len) { return write(fd, buf, len); }
int tap_write(int fd, char *buf, int len) { return write(fd, buf, len); }

int tun_read(int fd, char *buf, int len) { return read(fd, buf, len); }
int tap_read(int fd, char *buf, int len) { return read(fd, buf, len); }
