/*
 *   script.c
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
#include "mooutils/mooscript/mooscript-parser.h"
#include <gtk/gtk.h>
#include <string.h>


static void usage (const char *prg)
{
    g_error ("usage:\t%s -f <file>\n\t%s <script>", prg, prg);
}


int main (int argc, char *argv[])
{
    char *script;
    MSNode *node;
    MSValue *val;
    MSContext *ctx;

    gtk_init (&argc, &argv);

    if (argc < 2)
        usage (argv[0]);

    if (argc == 2)
    {
        script = argv[1];
    }
    else if (strcmp (argv[1], "-f"))
    {
        usage (argv[0]);
    }
    else
    {
        GError *error = NULL;
        if (!g_file_get_contents (argv[2], &script, NULL, &error))
            g_error ("%s", error->message);
    }

    node = ms_script_parse (script);

    if (!node)
        g_error ("could not parse script");

    ctx = ms_context_new ();
    val = ms_node_eval (node, ctx);

    if (!val)
        g_print ("error: %s\n", ms_context_get_error_msg (ctx));
    else
        g_print ("success: %s\n", ms_value_print (val));
}
