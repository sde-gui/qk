/*
 *   moolang-aux.h
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
/* g_utf8_offset_to_pointer, g_utf8_pointer_to_offset taken from glib
 * Copyright (C) 1999 Tom Tromey
 * Copyright (C) 2000 Red Hat, Inc.
 */

#ifndef MOOEDIT_COMPILATION
#error "This file may not be included"
#endif

#include <glib.h>
#include <string.h>


#define CHAR_IS_ASCII(ch__) ((guint8) ch__ < 128)
#define CHAR_IS_DIGIT(c__) ((c__) >= '0' && (c__) <= '9')
#define CHAR_IS_OCTAL(c__) ((c__) >= '0' && (c__) <= '7')
#define CHAR_IS_HEX(c__) (((c__) >= '0' && (c__) <= '9') || ((c__) >= 'A' && (c__) <= 'F') || ((c__) >= 'a' && (c__) <= 'f'))
#define CHAR_IS_WORD(c__) ((c__) == '_' || g_ascii_isalnum (c__))
#define CHAR_IS_SPACE(c__) ((c__) == ' ' || (c__) == '\t')

#define ASCII_TOLOWER(c__)                              \
    (g_ascii_isupper (c__) ? (c__) - 'A' + 'a' : (c__))


inline static long
utf8_pointer_to_offset (char *str, char *pos)
{
    char *s = str;
    long offset = 0;

    if (pos < str)
    {
        offset = - utf8_pointer_to_offset (pos, str);
    }
    else while (s < pos)
    {
        s = g_utf8_next_char (s);
        offset++;
    }

    return offset;
}


inline static char *
utf8_offset_to_pointer (char *str,
                        long  offset)
{
    char *s = str;

    if (offset > 0)
    {
        while (offset--)
            s = g_utf8_next_char (s);
    }
    else
    {
        char *s1;

        /* This nice technique for fast backwards stepping
         * through a UTF-8 string was dubbed "stutter stepping"
         * by its inventor, Larry Ewing.
         */
        while (offset)
        {
            s1 = s;
            s += offset;

            while ((*s & 0xc0) == 0x80)
                s--;

            offset += utf8_pointer_to_offset (s, s1);
        }
    }

    return s;
}


inline static gboolean
string_is_ascii (const char *string)
{
    while (*string++)
        if (!CHAR_IS_ASCII (*string))
            return FALSE;
    return TRUE;
}


inline static char*
ascii_casestrstr (char *haystack, char *needle, char *limit)
{
    while (haystack <= limit)
    {
        if (g_ascii_strcasecmp (haystack, needle))
            return haystack;
        haystack++;
    }

    return NULL;
}


inline static char*
ascii_lower_strchr (char *string, char ch, char *limit)
{
    while (string <= limit)
    {
        if (ASCII_TOLOWER (*string) == ch)
            return string;
        string++;
    }

    return NULL;
}


inline static char*
ascii_strchr (char *string, char ch, char *limit)
{
    g_assert (limit && string);

    while (string <= limit)
    {
        if (*string == ch)
            return string;
        string++;
    }

    return NULL;
}
