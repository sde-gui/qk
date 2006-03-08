/*
 *   mscript.c
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

#include "mooutils/mooscript/mooscript-parser.h"
#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>


static void usage (const char *prg)
{
    g_print ("usage:\t%s <file>\n\t%s -c <script>\n", prg, prg);
    exit (1);
}


static int run_interactive (void)
{
    MSContext *ctx;

    using_history ();

    ctx = ms_context_new ();

    while (TRUE)
    {
        char *line;
        MSNode *node;
        MSValue *val;

        line = readline (">>> ");

        if (!line)
        {
            g_print ("\n");
            return 0;
        }

        node = ms_script_parse (line);
        add_history (line);
        free (line);

        if (!node)
        {
            g_print ("syntax error\n");
            continue;
        }

        val = ms_top_node_eval (node, ctx);
        ms_node_unref (node);

        if (!val)
        {
            g_print ("%s\n", ms_context_get_error_msg (ctx));
            ms_context_clear_error (ctx);
            continue;
        }

        if (!ms_value_is_none (val))
        {
            char *s = ms_value_print (val);
            g_print ("%s\n", s);
            g_free (s);
        }

        ms_value_unref (val);
    }
}


int main (int argc, char *argv[])
{
    const char *file = NULL;
    char *script = NULL;
    MSNode *node;
    MSValue *val;
    MSContext *ctx;

    g_type_init ();
    ms_type_init ();

    if (argc < 2)
        return run_interactive ();

    if (argc == 2)
        file = argv[1];
    else if (strcmp (argv[1], "-c"))
        usage (argv[0]);
    else
        script = argv[2];

    if (file)
    {
        GError *error = NULL;

        if (!g_file_get_contents (file, &script, NULL, &error))
        {
            g_print ("%s\n", error->message);
            return 1;
        }
    }

    g_assert (script != NULL);
    node = ms_script_parse (script);

    if (!node)
        g_error ("could not parse script");

    ctx = ms_context_new ();
    val = ms_top_node_eval (node, ctx);

    if (!val)
    {
        g_print ("error: %s\n", ms_context_get_error_msg (ctx));
        return 2;
    }
    else
    {
        if (!ms_value_is_none (val))
            g_print ("%s\n", ms_value_print (val));
        return 0;
    }
}
