/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   as-plugin-script.c
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

#include "as-plugin-script.h"


ASContext *
_as_plugin_context_new (void)
{
    ASContext *ctx = as_context_new ();
    return ctx;
}


gboolean
_as_plugin_context_exec (ASContext      *ctx,
                         ASNode         *script,
                         MooEdit        *doc,
                         GtkTextIter    *insert,
                         char           *match,
                         char          **parens,
                         guint           n_parens)
{
    guint i;
    ASValue *val;
    gboolean success;

    g_return_val_if_fail (AS_IS_CONTEXT (ctx), FALSE);
    g_return_val_if_fail (AS_IS_NODE (script), FALSE);
    g_return_val_if_fail (MOO_IS_EDIT (doc), FALSE);
    g_return_val_if_fail (insert != NULL, FALSE);
    g_return_val_if_fail (match != NULL, FALSE);
    g_return_val_if_fail (!n_parens || parens, FALSE);

    val = as_value_string (match);
    as_context_assign_positional (ctx, 0, val);
    as_value_unref (val);

    for (i = 0; i < n_parens; ++i)
    {
        val = as_value_string (parens[i]);
        as_context_assign_positional (ctx, i + 1, val);
        as_value_unref (val);
    }

    val = as_node_eval (script, ctx);
    success = val != NULL;

    if (val)
        as_value_unref (val);

    for (i = 0; i < n_parens + 1; ++i)
        as_context_assign_positional (ctx, i, NULL);

    if (!success)
    {
        g_print ("%s\n", as_context_get_error_msg (ctx));
        as_context_clear_error (ctx);
    }

    return success;
}
