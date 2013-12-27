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

#define VTUN_DEV_LEN 20

extern "C"
{
    int tun_open(char *dev);
    int tun_close(int fd, char *dev);
    int tun_write(int fd, char *buf, int len);
    int tun_read(int fd, char *buf, int len);
}
