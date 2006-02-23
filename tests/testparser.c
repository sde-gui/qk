/*
 *   testparser.c
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

#define MOOEDIT_COMPILATION
#include "mooedit/moolang-parser.h"
#include <gtk/gtk.h>


int main (int argc, char *argv[])
{
    gtk_init (&argc, &argv);

    if (argc < 2)
        g_error ("usage: %s <file>", argv[0]);

    if (moo_lang_parse_file (argv[1]))
    {
        g_print ("*** Success ***\n");
        return 0;
    }
    else
    {
        g_print ("*** Error ***\n");
        return 1;
    }
}
