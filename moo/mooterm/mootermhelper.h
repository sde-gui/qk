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


inline static unsigned
CHARS_TO_UINT (guchar first, guchar second)
{
    return ((unsigned) second << 2) + first;
}


#define SIZE_CMD_LEN 6
inline static const char *set_size_cmd (guint16 width, guint16 height)
{
    static char cmd[SIZE_CMD_LEN] = {
        HELPER_CMD_CHAR,
        HELPER_SET_SIZE,
        0, 0, 0, 0
    };
    cmd[2] = width & 0xFF;
    cmd[3] = (width & 0xFF00) >> 2;
    cmd[4] = height & 0xFF;
    cmd[5] = (height & 0xFF00) >> 2;
    return cmd;
}


#endif /* __MOO_TERM_HELPER_H__ */
