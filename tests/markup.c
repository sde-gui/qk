/*
 *   tests/markup.c
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

#include "mooutils/moomarkup.h"
#include <string.h>
#include <glib.h>


const char *markup =
"<doc>\n"
"<blah>\n"
"<!-- COMMENT -->\n"
"    <bam a=\"ddd\" g=\"ferfer\"> ffff </bam>\n"
"<!-- COMMENT -->\n"
"<!---->\n"
"    <rgrg/>\n"
"</blah>\n"
"<!-- COMMENT -->\n"
"<fffff/>\n"
"</doc>\n"
;

#define NTIMES 1000

static void
profile (const char *filename)
{
    guint i;
    char *content;
    double time = 0.;

    if (!g_file_get_contents (filename, &content, NULL, NULL))
        g_error ("oops");

    for (i = 0; i < NTIMES; ++i)
    {
        GTimer *timer = g_timer_new ();
        MooMarkupDoc *doc = moo_markup_parse_memory (content, -1, NULL);
        time += g_timer_elapsed (timer, NULL);
        moo_markup_doc_unref (doc);
        g_timer_destroy (timer);
    }

    g_print ("%f\n", time / NTIMES);
}

int main (int argc, char *argv[])
{
    gboolean print = TRUE;
    const char *file = NULL;
    MooMarkupDoc *doc = NULL;
    GError *err = NULL;

    if (argc > 1)
        file = argv[1];

    if (argc > 2)
        print = FALSE;

    if (file)
    {
        profile (file);
        return 0;
        doc = moo_markup_parse_file (file, &err);
    }
    else
        doc = moo_markup_parse_memory (markup, -1, &err);

    if (!doc)
    {
        g_print ("ERROR\n");

        if (err)
        {
            g_print ("%s\n", err->message);
            g_error_free (err);
        }
    }
    else
    {
        g_print ("SUCCESS\n");

        if (print)
        {
            char *text = moo_markup_node_get_string (MOO_MARKUP_NODE (doc));
            g_print ("%s\n", text);
            g_free (text);
            g_print ("------------------------------\n");
        }

        moo_markup_doc_unref (doc);
    }

    return 0;
}
