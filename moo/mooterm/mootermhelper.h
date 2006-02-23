/*
 *   mootermhelper.h
 *
 *   Copyright (C) 2004-2006 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef MOOTERM_COMPILATION
#error "This file may not be included"
#endif

#ifndef __MOO_TERM_HELPER_H__
#define __MOO_TERM_HELPER_H__


#define MOO_TERM_HELPER_ENV "MOO_TERM_HELPER_WORKING_DIR"

typedef enum {
    HELPER_CMD_CHAR = 0,
    HELPER_SET_SIZE = 1,
    HELPER_GOODBYE  = 2,
    HELPER_OK       = 3
} HelperCmdCode;


typedef union {
    char        chars[4];
    unsigned    num;
} CharsToUInt;


inline static char UINT_TO_CHAR_FIRST (unsigned a)
{
    CharsToUInt c;
    c.num = a;
    return c.chars[1];
}

inline static char UINT_TO_CHAR_SECOND (unsigned a)
{
    CharsToUInt c;
    c.num = a;
    return c.chars[0];
}

inline static void UINT_TO_CHARS (unsigned a, char *v)
{
    CharsToUInt c;
    c.num = a;
    v[0] = c.chars[1];
    v[1] = c.chars[0];
}

inline static unsigned CHARS_TO_UINT (char first, char second)
{
    CharsToUInt c;
    c.chars[2] = c.chars[3] = 0;
    c.chars[0] = second;
    c.chars[1] = first;
    return c.num;
}

inline static unsigned CHARS_TO_UINT_S (char *v)
{
    CharsToUInt c;
    c.chars[2] = c.chars[3] = 0;
    c.chars[0] = v[1];
    c.chars[1] = v[0];
    return c.num;
}


#define SIZE_CMD_LEN 6
inline static const char *set_size_cmd (unsigned width, unsigned height)
{
    static char cmd[SIZE_CMD_LEN] = {
        HELPER_CMD_CHAR,
        HELPER_SET_SIZE,
        0, 0, 0, 0
    };
    cmd[2] = UINT_TO_CHAR_FIRST (width);
    cmd[3] = UINT_TO_CHAR_SECOND (width);
    cmd[4] = UINT_TO_CHAR_FIRST (height);
    cmd[5] = UINT_TO_CHAR_SECOND (height);
    return cmd;
}


#endif /* __MOO_TERM_HELPER_H__ */
