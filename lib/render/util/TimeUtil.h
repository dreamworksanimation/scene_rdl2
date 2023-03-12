// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
#pragma once

#include <cstring>     // memset()
#include <ctime>       // std::time(), std::localtime(), std::mktime()
#include <sstream>
#include <string>
#include <sys/time.h>  // gettimeofday()
#include <time.h>      // time_t

namespace scene_rdl2 {
namespace time_util {

inline
void init(struct timeval &tv)
{
    std::memset(&tv, 0x0, sizeof(tv));
}

inline
std::string
timeStr(const struct timeval &tv, bool usec = true)
{
    struct tm *time_st = localtime(&tv.tv_sec);

    static const char *month[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    static const char *wday[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

    std::ostringstream ostr;
    ostr << time_st->tm_year + 1900 << "/"
         << month[time_st->tm_mon] << "/"
         << time_st->tm_mday << " "
         << wday[time_st->tm_wday] << " "
         << time_st->tm_hour << ":"
         << time_st->tm_min << ":"
         << time_st->tm_sec;
    if (usec) {
        ostr << ":"
             << tv.tv_usec / 1000;
    }
    return ostr.str();
}

inline
std::string
timeStr(const time_t &t)
{
    struct timeval tv;
    tv.tv_sec = t;
    tv.tv_usec = 0;
    return timeStr(tv, false);
}

inline
float
utcOffsetHours()
{
    std::time_t currTime = std::time(nullptr);

    const time_t timeLocal = std::mktime(std::localtime(&currTime));

    // Greenwich Mean Time (GMT) and Coordinated Universal Time (UTC) share the same current time in practice
    const time_t timeUTC = std::mktime(std::gmtime(&currTime));

    float diffSec = (float)std::difftime(timeLocal, timeUTC);
    return diffSec / (60.0f * 60.0f);
}

inline
struct timeval
getCurrentTime()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv;
}

inline
std::string
currentTimeStr()
{
    return timeStr(getCurrentTime());
}

} // namespace time_util
} // namespace scene_rdl2

