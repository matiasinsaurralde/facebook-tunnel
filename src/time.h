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

#ifndef TIME_H
#define TIME_H

#include <sys/time.h>

class Time
{
public:
    Time() { tv.tv_sec = 0; tv.tv_usec = 0; };
    Time(int ms);

    timeval &getTimeval() { return tv; }

    Time operator+(const Time &other) const;
    Time operator-(const Time &other) const;

    bool operator!=(const Time &other) const;
    bool operator==(const Time &other) const;
    bool operator<(const Time &other) const;
    bool operator>(const Time &other) const;

    static Time now();

    static const Time ZERO;
protected:
    timeval tv;
};

#endif
