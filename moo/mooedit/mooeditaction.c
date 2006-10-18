/*
 *   mooeditaction.c
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
#include "mooedit/mooeditaction.h"
#include "mooedit/mooeditaction-factory.h"
#include "mooedit/mooedit-private.h"
#include "mooutils/eggregex.h"
#include <string.h>


typedef enum {
    FILTER_SENSITIVE,
    FILTER_VISIBLE
} FilterType;

#define N_FILTERS 2

struct _MooEditActionPrivate {
    MooEdit *doc;
    GSList *langs;
    EggRegex *filters[N_FILTERS];
};

typedef struct {
    EggRegex *regex;
    guint use_count;
} RegexRef;

typedef struct {
    GHashTable *hash;
} FilterStore;

static FilterStore filter_store;

G_DEFINE_TYPE (MooEditAction, moo_edit_action, MOO_TYPE_ACTION)

enum {
    PROP_0,
    PROP_DOC,
    PROP_LANGS,
    PROP_FILTER_SENSITIVE,
    PROP_FILTER_VISIBLE
};


static EggRegex *get_filter_regex   (const char *pattern);
static void      unuse_filter_regex (EggRegex   *regex);

static gboolean moo_edit_action_check_visible   (MooEditAction  *action);
static gboolean moo_edit_action_check_sensitive (MooEditAction  *action);
static void     moo_edit_action_check_state     (MooEditAction  *action);


static void
moo_edit_action_finalize (GObject *object)
{
    guint i;
    MooEditAction *action = MOO_EDIT_ACTION (object);

    for (i = 0; i < N_FILTERS; ++i)
        if (action->priv->filters[i])
            unuse_filter_regex (action->priv->filters[i]);

    g_slist_foreach (action->priv->langs, (GFunc) g_free, NULL);
    g_slist_free (action->priv->langs);

    G_OBJECT_CLASS(moo_edit_action_parent_class)->finalize (object);
}


static const char *
moo_edit_action_get_filter (MooEditAction *action,
                            FilterType     type)
{
    g_assert (type < N_FILTERS);
    return action->priv->filters[type] ?
            egg_regex_get_pattern (action->priv->filters[type]) : NULL;
}


static void
moo_edit_action_get_property (GObject        *object,
                              guint           prop_id,
                              GValue         *value,
                              GParamSpec     *pspec)
{
    MooEditAction *action = MOO_EDIT_ACTION (object);

    switch (prop_id)
    {
        case PROP_DOC:
            g_value_set_object (value, action->priv->doc);
            break;

        case PROP_FILTER_VISIBLE:
            g_value_set_string (value, moo_edit_action_get_filter (action, FILTER_VISIBLE));
            break;

        case PROP_FILTER_SENSITIVE:
            g_value_set_string (value, moo_edit_action_get_filter (action, FILTER_SENSITIVE));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void
string_slist_free (GSList *list)
{
    g_slist_foreach (list, (GFunc) g_free, NULL);
    g_slist_free (list);
}


static void
moo_edit_action_set_langs (MooEditAction *action,
                           const char    *string)
{
    string_slist_free (action->priv->langs);
    action->priv->langs = _moo_edit_parse_langs (string);
    g_object_notify (G_OBJECT (action), "langs");
}


MooEdit *
moo_edit_action_get_doc (MooEditAction *action)
{
    g_return_val_if_fail (MOO_IS_EDIT_ACTION (action), NULL);
    return action->priv->doc;
}


static void
moo_edit_action_set_filter (MooEditAction *action,
                            const char    *filter,
                            FilterType     type)
{
    EggRegex *tmp;

    g_assert (type < N_FILTERS);

    tmp = action->priv->filters[type];
    action->priv->filters[type] = filter ? get_filter_regex (filter) : NULL;
    unuse_filter_regex (tmp);
}


static void
moo_edit_action_set_property (GObject        *object,
                              guint           prop_id,
                              const GValue   *value,
                              GParamSpec     *pspec)
{
    MooEditAction *action = MOO_EDIT_ACTION (object);

    switch (prop_id)
    {
        case PROP_DOC:
            action->priv->doc = g_value_get_object (value);
            g_object_notify (object, "doc");
            break;

        case PROP_LANGS:
            moo_edit_action_set_langs (action, g_value_get_string (value));
            break;

        case PROP_FILTER_SENSITIVE:
            moo_edit_action_set_filter (action,
                                        g_value_get_string (value),
                                        FILTER_SENSITIVE);
            break;

        case PROP_FILTER_VISIBLE:
            moo_edit_action_set_filter (action,
                                        g_value_get_string (value),
                                        FILTER_VISIBLE);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void
moo_edit_action_init (MooEditAction *action)
{
    action->priv = G_TYPE_INSTANCE_GET_PRIVATE (action, MOO_TYPE_EDIT_ACTION, MooEditActionPrivate);
}


static const char *
get_current_line (MooEdit *doc)
{
    char *line;

    line = g_object_get_data (G_OBJECT (doc), "moo-edit-current-line");

    if (!line)
    {
        GtkTextIter line_start, line_end;
        GtkTextBuffer *buffer;

        buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (doc));
        gtk_text_buffer_get_iter_at_mark (buffer, &line_start,
                                          gtk_text_buffer_get_insert (buffer));
        line_end = line_start;
        gtk_text_iter_set_line_offset (&line_start, 0);
        if (!gtk_text_iter_ends_line (&line_end))
            gtk_text_iter_forward_to_line_end (&line_end);

        line = gtk_text_buffer_get_slice (buffer, &line_start, &line_end, TRUE);

        g_object_set_data_full (G_OBJECT (doc), "moo-edit-current-line", line, g_free);
    }

    return line;
}


static gboolean
moo_edit_action_check_visible_real (MooEditAction *action)
{
    MooLang *lang;
    gboolean visible = TRUE;
    EggRegex *filter = action->priv->filters[FILTER_VISIBLE];

    if (!action->priv->doc)
        return gtk_action_get_visible (GTK_ACTION (action));

    if (!action->priv->langs && !filter)
        return gtk_action_get_visible (GTK_ACTION (action));

    if (visible && action->priv->langs)
    {
        lang = moo_text_view_get_lang (MOO_TEXT_VIEW (action->priv->doc));

        if (!g_slist_find_custom (action->priv->langs,
                                  _moo_lang_id (lang),
                                  (GCompareFunc) strcmp))
            visible = FALSE;
    }

    if (visible && filter)
    {
        const char *line = get_current_line (action->priv->doc);
        if (!egg_regex_match (filter, line, 0))
            visible = FALSE;
    }

    return visible;
}


static gboolean
moo_edit_action_check_sensitive_real (MooEditAction *action)
{
    const char *line;
    EggRegex *filter = action->priv->filters[FILTER_SENSITIVE];

    if (!action->priv->doc || !filter)
        return gtk_action_get_visible (GTK_ACTION (action));

    line = get_current_line (action->priv->doc);
    return egg_regex_match (filter, line, 0);
}


static void
moo_edit_action_check_state_real (MooEditAction *action)
{
    gboolean visible, sensitive;

    visible = moo_edit_action_check_visible (action);
    sensitive = moo_edit_action_check_sensitive (action);

    g_object_set (action, "sensitive", sensitive, "visible", visible, NULL);
}


static void
moo_edit_action_class_init (MooEditActionClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = moo_edit_action_finalize;
    gobject_class->set_property = moo_edit_action_set_property;
    gobject_class->get_property = moo_edit_action_get_property;

    klass->check_state = moo_edit_action_check_state_real;
    klass->check_visible = moo_edit_action_check_visible_real;
    klass->check_sensitive = moo_edit_action_check_sensitive_real;

    g_type_class_add_private (klass, sizeof (MooEditActionPrivate));

    g_object_class_install_property (gobject_class,
                                     PROP_DOC,
                                     g_param_spec_object ("doc",
                                             "doc",
                                             "doc",
                                             MOO_TYPE_EDIT,
                                             G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class,
                                     PROP_LANGS,
                                     g_param_spec_string ("langs",
                                             "langs",
                                             "langs",
                                             NULL,
                                             G_PARAM_WRITABLE));

    g_object_class_install_property (gobject_class,
                                     PROP_FILTER_SENSITIVE,
                                     g_param_spec_string ("filter-sensitive",
                                             "filter-sensitive",
                                             "filter-sensitive",
                                             NULL,
                                             G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class,
                                     PROP_FILTER_VISIBLE,
                                     g_param_spec_string ("filter-visible",
                                             "filter-visible",
                                             "filter-visible",
                                             NULL,
                                             G_PARAM_READWRITE));
}


static gboolean
moo_edit_action_check_visible (MooEditAction *action)
{
    g_return_val_if_fail (MOO_IS_EDIT_ACTION (action), TRUE);

    if (MOO_EDIT_ACTION_GET_CLASS (action)->check_visible)
        return MOO_EDIT_ACTION_GET_CLASS (action)->check_visible (action);
    else
        return gtk_action_get_visible (GTK_ACTION (action));
}

static gboolean
moo_edit_action_check_sensitive (MooEditAction *action)
{
    g_return_val_if_fail (MOO_IS_EDIT_ACTION (action), TRUE);

    if (MOO_EDIT_ACTION_GET_CLASS (action)->check_sensitive)
        return MOO_EDIT_ACTION_GET_CLASS (action)->check_sensitive (action);
    else
        return gtk_action_get_sensitive (GTK_ACTION (action));
}

static void
moo_edit_action_check_state (MooEditAction *action)
{
    g_return_if_fail (MOO_IS_EDIT_ACTION (action));
    g_return_if_fail (MOO_EDIT_ACTION_GET_CLASS (action)->check_state != NULL);
    MOO_EDIT_ACTION_GET_CLASS (action)->check_state (action);
}


void
_moo_edit_check_actions (MooEdit *edit)
{
    GtkActionGroup *group = moo_edit_get_actions (edit);
    GList *actions = gtk_action_group_list_actions (group);

    while (actions)
    {
        if (MOO_IS_EDIT_ACTION (actions->data))
            moo_edit_action_check_state (actions->data);
        actions = g_list_delete_link (actions, actions);
    }
}


static void
regex_ref_free (RegexRef *ref)
{
    if (ref)
    {
        egg_regex_unref (ref->regex);
        g_free (ref);
    }
}

static void
init_filter_store (void)
{
    if (!filter_store.hash)
        filter_store.hash = g_hash_table_new_full (g_str_hash, g_str_equal,  g_free,
                                                   (GDestroyNotify) regex_ref_free);
}


static EggRegex *
get_filter_regex (const char *pattern)
{
    RegexRef *ref;

    g_return_val_if_fail (pattern != NULL, NULL);

    init_filter_store ();

    ref = g_hash_table_lookup (filter_store.hash, pattern);

    if (!ref)
    {
        EggRegex *regex;
        GError *error = NULL;

        regex = egg_regex_new (pattern, 0, 0, &error);

        if (!regex)
        {
            g_warning ("%s: %s", G_STRLOC, error->message);
            g_error_free (error);
            return NULL;
        }

        egg_regex_optimize (regex, NULL);

        ref = g_new0 (RegexRef, 1);
        ref->regex = regex;

        g_hash_table_insert (filter_store.hash, g_strdup (pattern), ref);
    }

    ref->use_count++;
    return ref->regex;
}


static void
unuse_filter_regex (EggRegex *regex)
{
    RegexRef *ref;
    const char *pattern;

    g_return_if_fail (regex != NULL);

    init_filter_store ();

    pattern = egg_regex_get_pattern (regex);
    ref = g_hash_table_lookup (filter_store.hash, pattern);
    g_return_if_fail (ref != NULL);

    if (!--ref->use_count)
        g_hash_table_remove (filter_store.hash, pattern);
}
