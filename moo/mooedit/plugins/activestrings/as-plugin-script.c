/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   as-plugin-script.c
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

#include "as-plugin-script.h"
#include "mooedit/mooedit-script.h"


static void
as_plugin_context_setup (MSContext       *ctx,
                         MooEdit         *doc,
                         char            *match,
                         char           **parens,
                         guint            n_parens)
{
    guint i;
    MSValue *val;

    val = ms_value_string (match);
    ms_context_assign_positional (ctx, 0, val);
    ms_value_unref (val);

    for (i = 0; i < n_parens; ++i)
    {
        val = ms_value_string (parens[i]);
        ms_context_assign_positional (ctx, i + 1, val);
        ms_value_unref (val);
    }

    moo_edit_context_set_doc (MOO_EDIT_CONTEXT (ctx), doc);
}


static void
as_plugin_context_clear (MSContext *ctx,
                         guint      n_parens)
{
    guint i;

    for (i = 0; i < n_parens + 1; ++i)
        ms_context_assign_positional (ctx, i, NULL);

    moo_edit_context_set_doc (MOO_EDIT_CONTEXT (ctx), NULL);
}


gboolean
_as_plugin_context_exec (MSContext      *ctx,
                         MSNode         *script,
                         MooEdit        *doc,
                         GtkTextIter    *insert,
                         char           *match,
                         char          **parens,
                         guint           n_parens)
{
    MSValue *val;
    gboolean success;

    g_return_val_if_fail (MOO_IS_EDIT_CONTEXT (ctx), FALSE);
    g_return_val_if_fail (script != NULL, FALSE);
    g_return_val_if_fail (MOO_IS_EDIT (doc), FALSE);
    g_return_val_if_fail (insert != NULL, FALSE);
    g_return_val_if_fail (match != NULL, FALSE);
    g_return_val_if_fail (!n_parens || parens, FALSE);

    as_plugin_context_setup (ctx, doc, match, parens, n_parens);

    val = ms_top_node_eval (script, ctx);
    success = val != NULL;
    ms_value_unref (val);

    if (!success)
    {
        g_print ("%s\n", ms_context_get_error_msg (ctx));
        ms_context_clear_error (ctx);
    }

    as_plugin_context_clear (ctx, n_parens);

    return success;
}
