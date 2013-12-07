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

#include "time.h"

const Time Time::ZERO = Time(0);

Time::Time(int ms)
{
    tv.tv_sec = ms / 1000;
    tv.tv_usec = (ms % 1000) * 1000;
}

Time Time::operator-(const Time &other) const
{
    Time result;
    result.tv.tv_sec = tv.tv_sec - other.tv.tv_sec;
    result.tv.tv_usec = tv.tv_usec - other.tv.tv_usec;
    
    if (result.tv.tv_usec < 0)
    {
        result.tv.tv_usec += 1000000;
        result.tv.tv_sec -= 1;
    }
    
    return result;
}

Time Time::operator+(const Time &other) const
{
    Time result;
    result.tv.tv_sec = tv.tv_sec + other.tv.tv_sec;
    result.tv.tv_usec = tv.tv_usec + other.tv.tv_usec;
    
    if (result.tv.tv_usec >= 1000000)
    {
        result.tv.tv_usec -= 1000000;
        result.tv.tv_sec += 1;
    }
    
    return result;
}

bool Time::operator==(const Time &other) const
{
    return tv.tv_sec != other.tv.tv_sec ? false : tv.tv_usec == other.tv.tv_usec;
}

bool Time::operator!=(const Time &other) const
{
    return tv.tv_sec != other.tv.tv_sec ? true : tv.tv_usec != other.tv.tv_usec;
}

bool Time::operator<(const Time &other) const
{
    return tv.tv_sec != other.tv.tv_sec ? tv.tv_sec < other.tv.tv_sec : tv.tv_usec < other.tv.tv_usec;
}

bool Time::operator>(const Time &other) const
{
    return tv.tv_sec != other.tv.tv_sec ? tv.tv_sec > other.tv.tv_sec : tv.tv_usec > other.tv.tv_usec;
}

Time Time::now()
{
    Time result;
    gettimeofday(&result.tv, 0);
    return result;
}
