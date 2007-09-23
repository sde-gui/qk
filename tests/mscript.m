/*
 *   mscript.c
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

#include "mooscript/mooscript-parser.h"
#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>


static void
usage (const char *prg)
{
    g_print ("usage:\t%s <file>\n\t%s -c <script>\n", prg, prg);
    exit (1);
}


static int
run_interactive (void)
{
    MSContext *ctx;

    using_history ();

    ctx = g_object_new (MS_TYPE_CONTEXT, NULL);

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

        if (ms_script_check (line) == MS_SCRIPT_INCOMPLETE)
        {
            GString *input = g_string_new (line);
            free (line);

            while (TRUE)
            {
                line = readline ("... ");

                if (!line)
                {
                    g_print ("\n");
                    return 0;
                }

                g_string_append (input, " ");
                g_string_append (input, line);
                free (line);

                if (ms_script_check (input->str) != MS_SCRIPT_INCOMPLETE)
                {
                    line = g_string_free (input, FALSE);
                    break;
                }
            }
        }

        node = ms_script_parse (line);
        add_history (line);
        free (line);

        if (!node)
            continue;

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

    if (argc > 1 && !strcmp (argv[1], "--g-fatal-warnings"))
    {
        GLogLevelFlags fatal_mask;
        int i;

        fatal_mask = g_log_set_always_fatal (G_LOG_FATAL_MASK);
        fatal_mask |= G_LOG_LEVEL_WARNING | G_LOG_LEVEL_CRITICAL;
        g_log_set_always_fatal (fatal_mask);

        for (i = 1; i < argc - 1; ++i)
            argv[i] = argv[i+1];
        argv[argc-1] = NULL;
        argc -= 1;
    }

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

    ctx = g_object_new (MS_TYPE_CONTEXT, NULL);
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
