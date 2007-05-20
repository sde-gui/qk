/*
 *   mootermhelper.h
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
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

#ifndef MOO_TERM_HELPER_H
#define MOO_TERM_HELPER_H


#define MOO_TERM_HELPER_ENV "MOO_TERM_HELPER_WORKING_DIR"

typedef enum {
    HELPER_CHAR_CMD     = 0,
    HELPER_CMD_SET_SIZE = 1,
    HELPER_CMD_SET_ECHO = 2,
    HELPER_CMD_GOODBYE  = 3,
    HELPER_CMD_OK       = 4
} HelperCmdCode;

#define HELPER_CMD_SIZE 6

#define HELPER_CMD_GET_WIDTH(cmd) ((unsigned char)(cmd)[2] + (((unsigned char)(cmd)[3]) << 2))
#define HELPER_CMD_GET_HEIGHT(cmd) ((unsigned char)(cmd)[4] + (((unsigned char)(cmd)[5]) << 2))
#define HELPER_CMD_GET_ECHO(cmd) ((cmd)[2] != 0)

#define HELPER_SET_SIZE_CMD(buf, width, height)     \
    (buf)[0] = HELPER_CHAR_CMD;                     \
    (buf)[1] = HELPER_CMD_SET_SIZE;                 \
    (buf)[2] = ((unsigned)width) & 0xFF;            \
    (buf)[3] = (((unsigned)width) & 0xFF00) >> 2;   \
    (buf)[4] = ((unsigned)height) & 0xFF;           \
    (buf)[5] = (((unsigned)height) & 0xFF00) >> 2;

#define HELPER_SET_ECHO_CMD(buf, echo)              \
    (buf)[0] = HELPER_CHAR_CMD;                     \
    (buf)[1] = HELPER_CMD_SET_ECHO;                 \
    (buf)[2] = !!(echo);

#define HELPER_OK_CMD(buf)                          \
    (buf)[0] = HELPER_CHAR_CMD;                     \
    (buf)[1] = HELPER_CMD_OK;

#define HELPER_GOODBYE_CMD(buf)                     \
    (buf)[0] = HELPER_CHAR_CMD;                     \
    (buf)[1] = HELPER_CMD_GOODBYE;


#endif /* MOO_TERM_HELPER_H */
