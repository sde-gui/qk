/*
 *   xdgmime/winfuncs.c
 *
 *   Copyright (C) 2004-2005 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   See COPYING file that comes with this distribution.
 */

#include "winfuncs.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <glib.h>


int gettimeofday(struct timeval *tv, struct timezone *tz)
{
    LONGLONG epoch = 0;

    if (!epoch) {
        SYSTEMTIME stEpoch;
        FILETIME ftEpoch;
        LARGE_INTEGER l;

        stEpoch.wYear = 1970;
        stEpoch.wMonth = 1;
        stEpoch.wDay = 1;
        stEpoch.wHour = 0;
        stEpoch.wMinute = 0;
        stEpoch.wSecond = 0;
        stEpoch.wMilliseconds = 0;
        SystemTimeToFileTime (&stEpoch, &ftEpoch);

        l.LowPart = ftEpoch.dwLowDateTime;
        l.HighPart = ftEpoch.dwHighDateTime;
        epoch = l.QuadPart;
    }

    SYSTEMTIME time;
    FILETIME ftime;
    LARGE_INTEGER l;
    LARGE_INTEGER s;

    if (tv) {
        GetLocalTime (&time);
        SystemTimeToFileTime (&time, &ftime);
        l.LowPart = ftime.dwLowDateTime;
        l.HighPart = ftime.dwHighDateTime;
        l.QuadPart -= epoch;
        s.QuadPart = l.QuadPart / 10000;
        tv->tv_sec = s.LowPart;
        s.QuadPart *= 10000;
        s.QuadPart = (l.QuadPart - s.QuadPart) * 10;
        tv->tv_usec = s.LowPart;
    }

    return 0;
}


int fnmatch (const char *pattern, const char *string, int flags)
{
    return g_pattern_match_simple (pattern, string);
}
