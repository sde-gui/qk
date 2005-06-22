/*
 *   xdgmime/winfuncs.h
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

#ifndef XDGMIME_WINFUNCS_H
#define XDGMIME_WINFUNCS_H

#include <sys/time.h>
#include <stdio.h>


struct timezone {
        int  tz_minuteswest; /* minutes W of Greenwich */
        int  tz_dsttime;     /* type of dst correction */
};

int gettimeofday(struct timeval *tv, struct timezone *tz);
int fnmatch(const char *pattern, const char *string, int flags);
#define getc_unlocked getc


#endif /* XDGMIME_WINFUNCS_H */
