/*
 *   moofileview-aux.h
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

#ifndef MOO_FILE_VIEW_COMPILATION
#error "This file may not be included"
#endif

#include "moofileview/moofile-private.h"
#include <string.h>
#include <glib.h>

#ifndef __MOO_FILE_VIEW_AUX_H__
#define __MOO_FILE_VIEW_AUX_H__


#define COLUMN_FILE MOO_FOLDER_MODEL_COLUMN_FILE


/* TODO: strncmp should accept char len, not byte len? */
typedef struct {
    int     (*strcmp_func)      (const char   *str,
                                 MooFile      *file);
    int     (*strncmp_func)     (const char   *str,
                                 MooFile      *file,
                                 guint         len);
    char*   (*normalize_func)   (const char   *str,
                                 gssize        len);
} TextFuncs;


static int
strcmp_func (const char *str,
             MooFile    *file)
{
    return strcmp (str, _moo_file_display_name (file));
}

static int
strncmp_func (const char *str,
              MooFile    *file,
              guint       len)
{
    return strncmp (str, _moo_file_display_name (file), len);
}

static char *
normalize_func (const char *str,
                gssize      len)
{
    return g_utf8_normalize (str, len, G_NORMALIZE_ALL);
}


static int
case_strcmp_func (const char *str,
                  MooFile    *file)
{
    return strcmp (str, _moo_file_case_display_name (file));
}

static int
case_strncmp_func (const char *str,
                   MooFile    *file,
                   guint       len)
{
    return strncmp (str, _moo_file_case_display_name (file), len);
}

static char *
case_normalize_func (const char *str,
                     gssize      len)
{
    char *norm = g_utf8_normalize (str, len, G_NORMALIZE_ALL);
    char *res = g_utf8_casefold (norm, -1);
    g_free (norm);
    return res;
}


static gboolean
model_find_next_match (GtkTreeModel   *model,
                       GtkTreeIter    *iter,
                       const char     *text,
                       gssize          len,
                       TextFuncs      *funcs,
                       gboolean        exact_match)
{
    char *normalized_text;
    guint normalized_text_len;

    g_return_val_if_fail (text != NULL, FALSE);

    normalized_text = funcs->normalize_func (text, len);
    normalized_text_len = strlen (normalized_text);

    while (TRUE)
    {
        MooFile *file = NULL;
        gboolean match;

        gtk_tree_model_get (model, iter, COLUMN_FILE, &file, -1);

        if (file)
        {
            if (exact_match)
                match = !funcs->strcmp_func (normalized_text, file);
            else
                match = !funcs->strncmp_func (normalized_text, file,
                                              normalized_text_len);

            _moo_file_unref (file);

            if (match)
            {
                g_free (normalized_text);
                return TRUE;
            }
        }

        if (!gtk_tree_model_iter_next (model, iter))
        {
            g_free (normalized_text);
            return FALSE;
        }
    }
}


static GString *
model_find_max_prefix (GtkTreeModel   *model,
                       const char     *text,
                       TextFuncs      *funcs,
                       gboolean       *unique_p,
                       GtkTreeIter    *unique_iter_p)
{
    GtkTreeIter iter, unique_iter;
    guint text_len;
    GString *prefix = NULL;
    gboolean unique = FALSE;

    g_assert (text && text[0]);

    text_len = strlen (text);

    if (!gtk_tree_model_get_iter_first (model, &iter))
        goto out;

    while (TRUE)
    {
        MooFile *file = NULL;
        const char *name;
        guint i;

        if (!model_find_next_match (model, &iter, text, text_len, funcs, FALSE))
            goto out;

        gtk_tree_model_get (model, &iter,
                            COLUMN_FILE, &file, -1);
        g_assert (file != NULL);

        name = _moo_file_display_name (file);

        if (!prefix)
        {
            prefix = g_string_new (_moo_file_display_name (file));
            unique_iter = iter;
            unique = TRUE;

            /* nothing to look for, just check if it's really unique */
            if (prefix->len == text_len)
            {
                if (gtk_tree_model_iter_next (model, &iter) &&
                    model_find_next_match (model, &iter, text, text_len, funcs, FALSE))
                    unique = FALSE;

                goto out;
            }
        }
        else
        {
            for (i = text_len; i < prefix->len && name[i] == prefix->str[i]; ++i) ;

            prefix->str[i] = 0;
            prefix->len = i;
            unique = FALSE;

            if (prefix->len == text_len)
                goto out;
        }

        if (!gtk_tree_model_iter_next (model, &iter))
            goto out;
    }

out:
    if (unique_p)
        *unique_p = unique;
    if (unique_iter_p)
        *unique_iter_p = unique_iter;
    return prefix;
}


#endif /* __MOO_FILE_VIEW_AUX_H__ */
