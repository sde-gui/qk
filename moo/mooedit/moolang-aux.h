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

#ifndef MOOEDIT_COMPILATION
#error "This file may not be included"
#endif

#include <glib.h>
#include <string.h>


#define CHAR_IS_ASCII(ch__) ((guint8) ch__ < 128)

static gboolean
string_is_ascii (const char *string)
{
    while (*string++)
        if (!CHAR_IS_ASCII (*string))
            return FALSE;
    return TRUE;
}


static char*
ascii_casestrstr (char *haystack, char *needle)
{
    char *h = haystack;

    while (*h)
    {
        if (g_ascii_strcasecmp (h, needle))
            return h;
        h++;
    }

    return NULL;
}


static char*
ascii_lower_strchr (char *string, char ch)
{
    while (*string)
    {
        if (g_ascii_tolower (*string) == ch)
            return string;
        string++;
    }
    return NULL;
}
