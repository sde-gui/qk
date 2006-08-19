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
#include "mooedit/mooedit-private.h"
#include "mooutils/eggregex.h"
#include <string.h>


struct _MooEditActionPrivate {
    MooEdit *doc;
    GSList *langs;
    EggRegex *regex;
};


G_DEFINE_TYPE (MooEditAction, moo_edit_action, MOO_TYPE_ACTION);

enum {
    PROP_0,
    PROP_DOC,
    PROP_LANGS,
    PROP_FILTER_PATTERN
};


static void
moo_edit_action_set_filter (MooEditAction *action,
                            const char    *pattern)
{
    GError *error = NULL;

    egg_regex_unref (action->priv->regex);
    action->priv->regex = NULL;

    if (!pattern)
        return;

    action->priv->regex = egg_regex_new (pattern, 0, EGG_REGEX_MATCH_NOTEMPTY, &error);

    if (!action->priv->regex)
    {
        g_warning ("%s: %s", G_STRLOC, error->message);
        g_error_free (error);
    }
    else
    {
        egg_regex_optimize (action->priv->regex, NULL);
    }
}


static void
moo_edit_action_finalize (GObject *object)
{
    MooEditAction *action = MOO_EDIT_ACTION (object);

    egg_regex_unref (action->priv->regex);
    g_slist_foreach (action->priv->langs, (GFunc) g_free, NULL);
    g_slist_free (action->priv->langs);

    G_OBJECT_CLASS(moo_edit_action_parent_class)->finalize (object);
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

        case PROP_FILTER_PATTERN:
            if (action->priv->regex)
                g_value_set_string (value, egg_regex_get_pattern (action->priv->regex));
            else
                g_value_set_string (value, NULL);
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

        case PROP_FILTER_PATTERN:
            moo_edit_action_set_filter (action, g_value_get_string (value));
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


static void
moo_edit_action_check_state_real (MooEditAction *action)
{
    gboolean sensitive = TRUE, visible = TRUE;

    g_return_if_fail (action->priv->doc != NULL);

    if (action->priv->langs)
    {
        MooLang *lang = moo_text_view_get_lang (MOO_TEXT_VIEW (action->priv->doc));

        if (!g_slist_find_custom (action->priv->langs, moo_lang_id (lang),
                                  (GCompareFunc) strcmp))
        {
            sensitive = FALSE;
            visible = FALSE;
        }
    }

    g_object_set (action, "visible", visible, "sensitive", sensitive, NULL);
}


static void
moo_edit_action_class_init (MooEditActionClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = moo_edit_action_finalize;
    gobject_class->set_property = moo_edit_action_set_property;
    gobject_class->get_property = moo_edit_action_get_property;

    klass->check_state = moo_edit_action_check_state_real;

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
                                     PROP_FILTER_PATTERN,
                                     g_param_spec_string ("filter-pattern",
                                             "filter-pattern",
                                             "filter-pattern",
                                             NULL,
                                             G_PARAM_READWRITE));
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
    GList *actions = moo_action_collection_list_actions (edit->priv->actions);

    while (actions)
    {
        if (MOO_IS_EDIT_ACTION (actions->data))
            moo_edit_action_check_state (actions->data);
        actions = g_list_delete_link (actions, actions);
    }
}
