#pragma once

#include <stdio.h>
#include <time.h>

#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#else
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/in6.h>
#include <netdb.h>
#include <sys/socket.h>
#endif

#ifdef WIN32

#define snprintf _snprintf
typedef ADDRESS_FAMILY  sa_family_t;

inline int gettimeofday(struct timeval *tp, void *tzp)
{
    time_t clock;
    struct tm tm;
    SYSTEMTIME wtm;
    GetLocalTime(&wtm);
    tm.tm_year = wtm.wYear - 1900;
    tm.tm_mon = wtm.wMonth - 1;
    tm.tm_mday = wtm.wDay;
    tm.tm_hour = wtm.wHour;
    tm.tm_min = wtm.wMinute;
    tm.tm_sec = wtm.wSecond;
    tm.tm_isdst = -1;
    clock = mktime(&tm);
    tp->tv_sec = clock;
    tp->tv_usec = wtm.wMilliseconds * 1000;
    return (0);
}

inline void bzero(void *s, size_t n)
{
    if (!s || n == 0)
    {
        return;
    }

    memset(&s, 0, n);
}

#endif