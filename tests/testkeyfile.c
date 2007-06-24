/*
 *   testkeyfile.c
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   See COPYING file that comes with this distribution.
 */

#include "mooedit/mookeyfile.h"


int main (int argc, char *argv[])
{
    GError *error = NULL;
    MooKeyFile *key_file;
    const char *file;
    char *string;

    if (argc < 2)
        g_error ("usage: %s <file>", argv[0]);

    file = argv[1];
    key_file = moo_key_file_new_from_file (file, &error);

    if (!key_file)
    {
        g_print ("%s\n", error->message);
        g_error_free (error);
        return 1;
    }

    g_print ("Successfully parsed file %s\n", file);

    string = moo_key_file_format (key_file, "A comment", 2);
    g_print ("===========================\n");
    g_print ("%s", string);
    g_print ("===========================\n");
    g_free (string);

    moo_key_file_unref (key_file);

    return 0;
}
