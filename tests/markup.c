/*
 *   tests/markup.c
 *
 *   Copyright (C) 2004-2005 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   See COPYING file that comes with this distribution.
 */

#include "mooutils/moomarkup.h"
#include <string.h>
#include <glib.h>


const char *markup =
"<blah>\n"
"<!-- COMMENT -->\n"
"    <bam a=\"ddd\" g=\"ferfer\"> ffff </bam>\n"
"<!-- COMMENT -->\n"
"    <rgrg/>\n"
"</blah>\n"
"<!-- COMMENT -->\n"
"<fffff/>\n"
;

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
        doc = moo_markup_parse_file (file, &err);
    else
        doc = moo_markup_parse_memory (markup, -1, &err);

    if (!doc)
    {
        g_print ("ERROR\n");

        if (err)
        {
            g_print (err->message);
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
}
