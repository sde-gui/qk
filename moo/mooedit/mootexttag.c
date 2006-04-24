/*
 *   mootexttag.c
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
#include "mooedit/mootexttag.h"


G_DEFINE_TYPE(MooTextTag, _moo_text_tag, GTK_TYPE_TEXT_TAG)

enum {
    PROP_0,
    PROP_TYPE
};


static void
moo_text_tag_get_property (GObject    *object,
                           guint       property_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
    MooTextTag *tag = MOO_TEXT_TAG (object);

    switch (property_id)
    {
        case PROP_TYPE:
            g_value_set_enum (value, tag->type);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}


static void
moo_text_tag_set_property (GObject      *object,
                           guint         property_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
    MooTextTag *tag = MOO_TEXT_TAG (object);

    switch (property_id)
    {
        case PROP_TYPE:
            tag->type = g_value_get_enum (value);
            g_object_notify (object, "type");
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}


static void
_moo_text_tag_class_init (MooTextTagClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->set_property = moo_text_tag_set_property;
    gobject_class->get_property = moo_text_tag_get_property;

    g_object_class_install_property (gobject_class, PROP_TYPE,
                                     g_param_spec_enum ("type", "type", "type",
                                                        MOO_TYPE_TEXT_TAG_TYPE,
                                                        MOO_TEXT_TAG_PLACEHOLDER_START,
                                                        G_PARAM_READWRITE));
}


static void
_moo_text_tag_init (G_GNUC_UNUSED MooTextTag *tag)
{
}


GtkTextTag *
_moo_text_tag_new (MooTextTagType  type,
                   const char     *name)
{
    return g_object_new (MOO_TYPE_TEXT_TAG,
                         "type", type, "name", name,
                         NULL);
}


MooTextTag *
_moo_text_tag_at_iter (const GtkTextIter *iter)
{
    MooTextTag *tag = NULL;
    GSList *tags, *l;

    tags = gtk_text_iter_get_tags (iter);

    for (l = tags; l != NULL; l = l->next)
        if (MOO_IS_TEXT_TAG (l->data))
        {
            tag = l->data;
            break;
        }

    g_slist_free (tags);
    return tag;
}


static gboolean
iter_has_tag (const GtkTextIter *iter,
              MooTextTagType     type)
{
    MooTextTag *tag;

    if (gtk_text_iter_get_char (iter) != MOO_TEXT_UNKNOWN_CHAR)
        return FALSE;

    tag = _moo_text_tag_at_iter (iter);

    if (!tag || tag->type != type)
        return FALSE;

#ifdef MOO_DEBUG
    if (type == MOO_TEXT_TAG_PLACEHOLDER_START)
    {
        GtkTextIter i = *iter;
        g_assert (gtk_text_iter_forward_char (&i));
        g_assert (gtk_text_iter_get_char (&i) == MOO_TEXT_UNKNOWN_CHAR);
        tag = _moo_text_tag_at_iter (&i);
        g_assert (tag && tag->type == MOO_TEXT_TAG_PLACEHOLDER_END);
    }
    else if (type == MOO_TEXT_TAG_PLACEHOLDER_END)
    {
        GtkTextIter i = *iter;
        g_assert (gtk_text_iter_backward_char (&i));
        g_assert (gtk_text_iter_get_char (&i) == MOO_TEXT_UNKNOWN_CHAR);
        tag = _moo_text_tag_at_iter (&i);
        g_assert (tag && tag->type == MOO_TEXT_TAG_PLACEHOLDER_START);
    }
#endif

    return TRUE;
}


gboolean
_moo_text_iter_is_placeholder_start (const GtkTextIter *iter)
{
    return iter_has_tag (iter, MOO_TEXT_TAG_PLACEHOLDER_START);
}


gboolean
_moo_text_iter_is_placeholder_end (const GtkTextIter *iter)
{
    return iter_has_tag (iter, MOO_TEXT_TAG_PLACEHOLDER_END);
}


GType
_moo_text_tag_type_get_type (void)
{
    static GType type;

    if (!type)
    {
        static GEnumValue values[] = {
            { MOO_TEXT_TAG_PLACEHOLDER_START, (char*) "MOO_TEXT_TAG_PLACEHOLDER_START", (char*) "placeholder-start" },
            { MOO_TEXT_TAG_PLACEHOLDER_END, (char*) "MOO_TEXT_TAG_PLACEHOLDER_END", (char*) "placeholder-end" },
            { 0, NULL, NULL}
        };

        type = g_enum_register_static ("MooTextTagType", values);
    }

    return type;
}
