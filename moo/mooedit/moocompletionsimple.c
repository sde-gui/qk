/*
 *   moocompletionsimple.c
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   See COPYING file that comes with this distribution.
 */

#include "mooedit/moocompletionsimple.h"
#include "mooedit/mootextpopup.h"
#include "mooedit/mooedit-script.h"
#include "mooutils/moomarshals.h"
#include "mooutils/eggregex.h"
#include "mooscript/mooscript-parser.h"
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <string.h>


#define MOO_COMPLETION_VAR_MATCH "match"
#define MOO_COMPLETION_VAR_COMPLETION "completion"

enum {
    COLUMN_DATA,
    COLUMN_GROUP
};

/* same as GCompletionFunc - must not allocate a new string;
   must allow two simultaneous calls */
typedef char* (*MooCompletionStringFunc)    (gpointer       data);
typedef void  (*MooCompletionFreeFunc)      (gpointer       data);
typedef int   (*MooCompletionCmpFunc)       (gpointer       data1,
                                             gpointer       data2);

struct _MooCompletionSimplePrivate {
    GSList *groups;
    GSList *active_groups;
    gboolean groups_found;
    GList *data;

    MooCompletionStringFunc string_func;
    MooCompletionFreeFunc free_func;
    MooCompletionCmpFunc cmp_func;
};

struct _MooCompletionGroup {
    char *name;

    EggRegex *regex;
    guint *parens;
    guint n_parens;

    GCompletion *cmpl;
    GList *data;
    char *suffix;
    MSNode *script;

    MooCompletionFreeFunc free_func;
};


static void     moo_completion_simple_dispose           (GObject                *object);

static void     moo_completion_simple_populate          (MooTextCompletion      *cmpl,
                                                         GtkTreeModel           *model,
                                                         GtkTextIter            *cursor,
                                                         const char             *text);
static void     moo_completion_simple_complete          (MooTextCompletion      *cmpl,
                                                         GtkTreeModel           *model,
                                                         GtkTreeIter            *iter);

static int      list_sort_func                          (GtkTreeModel           *model,
                                                         GtkTreeIter            *a,
                                                         GtkTreeIter            *b,
                                                         MooCompletionSimple    *cmpl);
static char    *completion_text_func                    (GtkTreeModel           *model,
                                                         GtkTreeIter            *iter,
                                                         gpointer                data);

static MooCompletionSimple *moo_completion_simple_new   (MooCompletionStringFunc string_func,
                                                         MooCompletionFreeFunc free_func,
                                                         MooCompletionCmpFunc cmp_func);

static MooCompletionGroup *moo_completion_group_new     (const char             *name,
                                                         MooCompletionStringFunc string_func,
                                                         MooCompletionFreeFunc free_func);
static void     moo_completion_group_free               (MooCompletionGroup     *group);

static gboolean moo_completion_group_find               (MooCompletionGroup     *group,
                                                         const char             *line,
                                                         int                    *start_pos,
                                                         int                    *end_pos);
static GList   *moo_completion_group_complete           (MooCompletionGroup     *group,
                                                         const char             *text,
                                                         char                  **prefix);
static void     moo_completion_simple_finish            (MooTextCompletion      *cmpl);


G_DEFINE_TYPE (MooCompletionSimple, moo_completion_simple, MOO_TYPE_TEXT_COMPLETION)


static void
moo_completion_simple_dispose (GObject *object)
{
    MooCompletionSimple *cmpl = MOO_COMPLETION_SIMPLE (object);

    if (cmpl->priv)
    {
        g_slist_foreach (cmpl->priv->groups, (GFunc) moo_completion_group_free, NULL);
        g_slist_free (cmpl->priv->groups);
        g_slist_free (cmpl->priv->active_groups);
        g_free (cmpl->priv);
        cmpl->priv = NULL;
    }

    G_OBJECT_CLASS(moo_completion_simple_parent_class)->dispose (object);
}


static void
moo_completion_simple_class_init (MooCompletionSimpleClass *klass)
{
    MooTextCompletionClass *cmpl_clas = MOO_TEXT_COMPLETION_CLASS(klass);

    G_OBJECT_CLASS(klass)->dispose = moo_completion_simple_dispose;

    cmpl_clas->populate = moo_completion_simple_populate;
    cmpl_clas->finish = moo_completion_simple_finish;
    cmpl_clas->complete = moo_completion_simple_complete;
}


static void
moo_completion_simple_init (MooCompletionSimple *cmpl)
{
    GtkListStore *store;

    cmpl->priv = g_new0 (MooCompletionSimplePrivate, 1);

    store = gtk_list_store_new (2, G_TYPE_POINTER, G_TYPE_POINTER);
    gtk_tree_sortable_set_sort_func (GTK_TREE_SORTABLE (store),
                                     COLUMN_DATA,
                                     (GtkTreeIterCompareFunc) list_sort_func,
                                     cmpl, NULL);
    moo_text_completion_set_model (MOO_TEXT_COMPLETION (cmpl),
                                   GTK_TREE_MODEL (store));
    moo_text_completion_set_text_func (MOO_TEXT_COMPLETION (cmpl),
                                       completion_text_func, cmpl,
                                       NULL);

    g_object_unref (store);
}


static GtkTextBuffer *
get_buffer (MooCompletionSimple *cmpl)
{
    return moo_text_completion_get_buffer (MOO_TEXT_COMPLETION (cmpl));
}


static char *
moo_completion_simple_find_groups (MooCompletionSimple *cmpl,
                                   GtkTextIter         *cursor,
                                   const char          *line)
{
    GSList *l;
    gboolean found = FALSE;
    int start_pos = -1, end_pos = -1;
    char *text = NULL;

    for (l = cmpl->priv->groups; l != NULL; l = l->next)
    {
        int start_pos_here, end_pos_here;
        MooCompletionGroup *grp = l->data;

        if (!moo_completion_group_find (grp, line, &start_pos_here, &end_pos_here))
            continue;

        if (!found)
        {
            found = TRUE;
            start_pos = start_pos_here;
            end_pos = end_pos_here;
        }

        if (start_pos_here == start_pos && end_pos_here == end_pos)
            cmpl->priv->active_groups =
                    g_slist_prepend (cmpl->priv->active_groups, grp);
    }

    if (found)
    {
        GtkTextIter start, end;

        cmpl->priv->active_groups = g_slist_reverse (cmpl->priv->active_groups);

        start = end = *cursor;
        gtk_text_iter_set_line_index (&start, start_pos);
        gtk_text_iter_set_line_index (&end, end_pos);
        text = gtk_text_iter_get_slice (&start, &end);

        moo_text_completion_set_region (MOO_TEXT_COMPLETION (cmpl), &start, &end);
    }

    return text;
}


static int
list_sort_func (GtkTreeModel        *model,
                GtkTreeIter         *a,
                GtkTreeIter         *b,
                MooCompletionSimple *cmpl)
{
    gpointer data1, data2;

    g_assert (MOO_IS_COMPLETION_SIMPLE (cmpl));
    g_assert (cmpl->priv->cmp_func != NULL);

    gtk_tree_model_get (model, a, COLUMN_DATA, &data1, -1);
    gtk_tree_model_get (model, b, COLUMN_DATA, &data2, -1);

    return cmpl->priv->cmp_func (data1, data2);
}


static void
moo_completion_simple_exec_script (MooCompletionSimple *cmpl,
                                   MooCompletionGroup  *group,
                                   GtkTextIter         *start,
                                   GtkTextIter         *end,
                                   const char          *completion)
{
    MSContext *ctx;
    char *match;
    MSValue *result;
    GtkTextView *doc;

    doc = moo_text_completion_get_doc (MOO_TEXT_COMPLETION (cmpl));
    ctx = moo_edit_script_context_new (doc, NULL);
    match = gtk_text_iter_get_slice (start, end);

    ms_context_assign_string (ctx, MOO_COMPLETION_VAR_MATCH, match);
    ms_context_assign_string (ctx, MOO_COMPLETION_VAR_COMPLETION, completion);

    gtk_text_buffer_delete (get_buffer (cmpl), start, end);
    gtk_text_buffer_place_cursor (get_buffer (cmpl), start);
    result = ms_top_node_eval (group->script, ctx);

    if (result)
        ms_value_unref (result);
    else
        g_warning ("%s: %s", G_STRLOC,
                   ms_context_get_error_msg (ctx));

    g_free (match);
    g_object_unref (ctx);
}


static void
moo_completion_simple_complete (MooTextCompletion *text_cmpl,
                                GtkTreeModel      *model,
                                GtkTreeIter       *iter)
{
    char *text, *old_text;
    GtkTextIter start, end;
    gpointer data = NULL;
    MooCompletionGroup *group = NULL;
    gboolean set_cursor = FALSE;
    MooCompletionSimple *cmpl = MOO_COMPLETION_SIMPLE (text_cmpl);

    g_return_if_fail (cmpl->priv->active_groups != NULL);

    gtk_tree_model_get (model, iter, COLUMN_DATA, &data, COLUMN_GROUP, &group, -1);
    g_assert (group != NULL);

    text = cmpl->priv->string_func ? cmpl->priv->string_func (data) : data;
    g_return_if_fail (text != NULL);

    moo_text_completion_get_region (MOO_TEXT_COMPLETION (cmpl), &start, &end);
    old_text = gtk_text_iter_get_slice (&start, &end);

    gtk_text_buffer_begin_user_action (get_buffer (cmpl));

    if (group->script)
    {
        moo_completion_simple_exec_script (cmpl, group, &start, &end, text);
    }
    else
    {
        if (strcmp (text, old_text))
        {
            gtk_text_buffer_delete (get_buffer (cmpl), &start, &end);
            gtk_text_buffer_insert (get_buffer (cmpl), &start, text, -1);
            set_cursor = TRUE;
        }

        if (group->suffix)
        {
            gboolean do_insert = TRUE;

            if (!gtk_text_iter_ends_line (&start))
            {
                char *old_suffix;
                end = start;
                gtk_text_iter_forward_to_line_end (&end);
                old_suffix = gtk_text_iter_get_slice (&start, &end);

                if (!strncmp (group->suffix, old_suffix, strlen (group->suffix)))
                {
                    do_insert = FALSE;
                    gtk_text_iter_forward_chars (&start, g_utf8_strlen (group->suffix, -1));
                    set_cursor = TRUE;
                }

                g_free (old_suffix);
            }

            if (do_insert)
            {
                gtk_text_buffer_insert (get_buffer (cmpl), &start, group->suffix, -1);
                set_cursor = TRUE;
            }
        }
    }

    if (set_cursor)
        gtk_text_buffer_place_cursor (get_buffer (cmpl), &start);

    gtk_text_buffer_end_user_action (get_buffer (cmpl));

    moo_text_completion_hide (MOO_TEXT_COMPLETION (cmpl));

    g_free (old_text);
}


static void
moo_completion_simple_populate (MooTextCompletion *text_cmpl,
                                GtkTreeModel      *model,
                                GtkTextIter       *cursor,
                                const char        *text)
{
    GSList *l;
    const char *prefix = text;
    char *freeme = NULL;
    MooCompletionSimple *cmpl = MOO_COMPLETION_SIMPLE (text_cmpl);

    if (!cmpl->priv->groups_found)
    {
        freeme = moo_completion_simple_find_groups (cmpl, cursor, text);

        if (!freeme)
            return;

        cmpl->priv->groups_found = TRUE;
        prefix = freeme;
    }

    gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (model),
                                          GTK_TREE_SORTABLE_UNSORTED_SORT_COLUMN_ID,
                                          GTK_SORT_ASCENDING);

    for (l = cmpl->priv->active_groups; l != NULL; l = l->next)
    {
        GList *list;
        MooCompletionGroup *group;

        group = l->data;
        list = moo_completion_group_complete (group, prefix, NULL);

        while (list)
        {
            GtkTreeIter iter;
            gtk_list_store_append (GTK_LIST_STORE (model), &iter);
            gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                                COLUMN_DATA, list->data,
                                COLUMN_GROUP, group,
                                -1);
            list = list->next;
        }
    }

    if (cmpl->priv->cmp_func)
        gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (model),
                                              COLUMN_DATA, GTK_SORT_ASCENDING);

    g_free (freeme);
}


static void
moo_completion_simple_finish (MooTextCompletion *text_cmpl)
{
    MooCompletionSimple *cmpl = MOO_COMPLETION_SIMPLE (text_cmpl);
    g_slist_free (cmpl->priv->active_groups);
    cmpl->priv->active_groups = NULL;
    cmpl->priv->groups_found = FALSE;
}


static MooCompletionSimple *
moo_completion_simple_new (MooCompletionStringFunc string_func,
                           MooCompletionFreeFunc free_func,
                           MooCompletionCmpFunc cmp_func)
{
    MooCompletionSimple *cmpl = g_object_new (MOO_TYPE_COMPLETION_SIMPLE, NULL);

    cmpl->priv->string_func = string_func;
    cmpl->priv->free_func = free_func;
    cmpl->priv->cmp_func = cmp_func;

    return cmpl;
}


static void
text_cell_data_func (G_GNUC_UNUSED GtkTreeViewColumn *tree_column,
                     GtkCellRenderer     *cell,
                     GtkTreeModel        *tree_model,
                     GtkTreeIter         *iter,
                     MooCompletionSimple *cmpl)
{
    gpointer data = NULL;
    char *text;

    g_assert (MOO_IS_COMPLETION_SIMPLE (cmpl));

    gtk_tree_model_get (tree_model, iter, 0, &data, -1);
    text = cmpl->priv->string_func ? cmpl->priv->string_func (data) : data;
    g_return_if_fail (text != NULL);

    g_object_set (cell, "text", text, NULL);
}

static char *
completion_text_func (GtkTreeModel *model,
                      GtkTreeIter  *iter,
                      gpointer      user_data)
{
    MooCompletionSimple *cmpl = user_data;
    gpointer data = NULL;
    char *text;

    gtk_tree_model_get (model, iter, 0, &data, -1);
    text = cmpl->priv->string_func ? cmpl->priv->string_func (data) : data;
    g_return_val_if_fail (text != NULL, NULL);

    return g_strdup (text);
}


MooTextCompletion *
moo_completion_simple_new_text (GList *words)
{
    MooCompletionSimple *cmpl;
    GtkCellRenderer *cell;
    MooTextPopup *popup;

    cmpl = moo_completion_simple_new (NULL, g_free, (MooCompletionCmpFunc) strcmp);

    if (words)
    {
        MooCompletionGroup *group = moo_completion_simple_new_group (cmpl, NULL);
        moo_completion_group_add_data (group, words);
        moo_completion_group_set_pattern (group, "\\w*", NULL, 0);
    }

    popup = moo_text_completion_get_popup (MOO_TEXT_COMPLETION (cmpl));
    cell = gtk_cell_renderer_text_new ();
    gtk_cell_layout_clear (GTK_CELL_LAYOUT (popup->column));
    gtk_tree_view_column_pack_start (popup->column, cell, TRUE);
    gtk_tree_view_column_set_cell_data_func (popup->column, cell,
                                             (GtkTreeCellDataFunc) text_cell_data_func,
                                             g_object_ref (cmpl),
                                             g_object_unref);

    return MOO_TEXT_COMPLETION (cmpl);
}


MooCompletionGroup *
moo_completion_simple_new_group (MooCompletionSimple *cmpl,
                                 const char          *name)
{
    MooCompletionGroup *group;

    g_return_val_if_fail (MOO_IS_COMPLETION_SIMPLE (cmpl), NULL);

    group = moo_completion_group_new (name,
                                      cmpl->priv->string_func,
                                      cmpl->priv->free_func);
    cmpl->priv->groups = g_slist_append (cmpl->priv->groups, group);

    return group;
}


/****************************************************************************/
/* MooCompletionGroup
 */

MooCompletionGroup *
moo_completion_group_new (const char *name,
                          MooCompletionStringFunc string_func,
                          MooCompletionFreeFunc free_func)
{
    MooCompletionGroup *group = g_new0 (MooCompletionGroup, 1);
    group->cmpl = g_completion_new (string_func);
    group->free_func = free_func;
    group->name = g_strdup (name);
    return group;
}


void
moo_completion_group_add_data (MooCompletionGroup *group,
                               GList              *data)
{
    g_return_if_fail (group != NULL);

    if (!data)
        return;

    g_completion_clear_items (group->cmpl);
    group->data = g_list_concat (group->data, data);
    g_completion_add_items (group->cmpl, group->data);
}


static gboolean
moo_completion_group_find (MooCompletionGroup *group,
                           const char         *line,
                           int                *start_pos_p,
                           int                *end_pos_p)
{
    GMatchInfo *match_info;

    g_return_val_if_fail (group != NULL, FALSE);
    g_return_val_if_fail (line != NULL, FALSE);
    g_return_val_if_fail (group->regex != NULL, FALSE);

    if (g_regex_match (group->regex, line, 0, &match_info))
    {
        guint i;

        for (i = 0; i < group->n_parens; ++i)
        {
            int start_pos = -1, end_pos = -1;

            g_regex_fetch_pos (match_info, group->parens[i],
                               &start_pos, &end_pos);

            if (start_pos >= 0 && end_pos >= 0)
            {
                if (start_pos_p)
                    *start_pos_p = start_pos;
                if (end_pos_p)
                    *end_pos_p = end_pos;
                g_match_info_free (match_info);
                return TRUE;
            }
        }
    }

    g_match_info_free (match_info);
    return FALSE;
}


static GList *
moo_completion_group_complete (MooCompletionGroup *group,
                               const char         *text,
                               char              **prefix)
{
    char *dummy = NULL;
    GList *list;

    if (!prefix)
        prefix = &dummy;

    /* g_completion_complete_utf8 wants prefix != NULL */
    list = g_completion_complete_utf8 (group->cmpl, text, prefix);

    g_free (dummy);
    return list;
}


void
moo_completion_group_set_pattern (MooCompletionGroup *group,
                                  const char         *pattern,
                                  const guint        *parens,
                                  guint               n_parens)
{
    EggRegex *regex;
    GError *error = NULL;
    char *real_pattern;

    g_return_if_fail (group != NULL);
    g_return_if_fail (pattern && pattern[0]);
    g_return_if_fail (!parens || n_parens);

    real_pattern = g_strdup_printf ("%s$", pattern);
    regex = g_regex_new (real_pattern, G_REGEX_OPTIMIZE, 0, &error);

    if (!regex)
    {
        g_warning ("%s: %s", G_STRLOC, error->message);
        g_error_free (error);
        goto err;
    }

    g_regex_free (group->regex);
    group->regex = regex;

    g_free (group->parens);

    if (!parens)
    {
        group->parens = g_new0 (guint, 1);
        group->n_parens = 1;
    }
    else
    {
        group->parens = g_memdup (parens, n_parens * sizeof (guint));
        group->n_parens = n_parens;
    }

    g_free (real_pattern);
    return;

err:
    if (error)
        g_error_free (error);
    g_free (real_pattern);
    g_regex_free (regex);
}


void
moo_completion_group_set_suffix (MooCompletionGroup *group,
                                 const char         *suffix)
{
    g_return_if_fail (group != NULL);

    if (group->suffix != suffix)
    {
        g_free (group->suffix);
        group->suffix = (suffix && suffix[0]) ? g_strdup (suffix) : NULL;
    }
}


void
moo_completion_group_set_script (MooCompletionGroup *group,
                                 const char         *script)
{
    g_return_if_fail (group != NULL);

    if (group->script)
    {
        ms_node_unref (group->script);
        group->script = NULL;
    }

    if (script)
        group->script = ms_script_parse (script);
}


static void
moo_completion_group_free (MooCompletionGroup *group)
{
    g_return_if_fail (group != NULL);

    g_regex_free (group->regex);
    g_free (group->parens);
    g_completion_free (group->cmpl);
    g_free (group->suffix);
    g_free (group->name);

    if (group->script)
        ms_node_unref (group->script);

    if (group->free_func)
        g_list_foreach (group->data, (GFunc) group->free_func, NULL);
    g_list_free (group->data);

    g_free (group);
}
