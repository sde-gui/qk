/*
 *   mootermtag.c
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

#define MOOTERM_COMPILATION
#include "mooterm/mootermbuffer-private.h"
#include "mooterm/mooterm-private.h"
#include "mooterm/mootermline-private.h"
#include "mooterm/mooterm-text.h"


static void     moo_term_tag_finalize       (GObject        *object);
static void     moo_term_tag_set_property   (GObject        *object,
                                             guint           prop_id,
                                             const GValue   *value,
                                             GParamSpec     *pspec);
static void     moo_term_tag_get_property   (GObject        *object,
                                             guint           prop_id,
                                             GValue         *value,
                                             GParamSpec     *pspec);

static int
ptr_cmp (gconstpointer a,
         gconstpointer b)
{
    return a < b ? -1 : (a > b ? 1 : 0);
}


/* MOO_TYPE_TERM_TAG */
G_DEFINE_TYPE (MooTermTag, moo_term_tag, G_TYPE_OBJECT)

enum {
    PROP_0,
    PROP_HAS_ATTR,
    PROP_TAG_TABLE,
    PROP_NAME
};


static void
moo_term_tag_class_init (MooTermTagClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->set_property = moo_term_tag_set_property;
    gobject_class->get_property = moo_term_tag_get_property;
    gobject_class->finalize = moo_term_tag_finalize;

    g_object_class_install_property (gobject_class,
                                     PROP_HAS_ATTR,
                                     g_param_spec_boolean ("has-attr",
                                             "has-attr",
                                             "has-attr",
                                             FALSE,
                                             G_PARAM_READABLE));

    g_object_class_install_property (gobject_class,
                                     PROP_TAG_TABLE,
                                     g_param_spec_pointer ("tag-table",
                                             "tag-table",
                                             "tag-table",
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

    g_object_class_install_property (gobject_class,
                                     PROP_NAME,
                                     g_param_spec_string ("name",
                                             "name",
                                             "name",
                                             NULL,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
}


static void
moo_term_tag_init (MooTermTag *tag)
{
    tag->table = NULL;
    tag->name = NULL;
    tag->attr.mask = 0;
    tag->lines = NULL;
}


static void
moo_term_tag_finalize (GObject *object)
{
    MooTermTag *tag = MOO_TERM_TAG (object);
    g_free (tag->name);
    G_OBJECT_CLASS(moo_term_tag_parent_class)->finalize (object);
}


static void
moo_term_tag_set_property (GObject        *object,
                           guint           prop_id,
                           const GValue   *value,
                           GParamSpec     *pspec)
{
    MooTermTag *tag = MOO_TERM_TAG (object);

    switch (prop_id)
    {
        case PROP_TAG_TABLE:
            tag->table = g_value_get_pointer (value);
            break;

        case PROP_NAME:
            g_free (tag->name);
            tag->name = g_strdup (g_value_get_string (value));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}


static void
moo_term_tag_get_property (GObject        *object,
                           guint           prop_id,
                           GValue         *value,
                           GParamSpec     *pspec)
{
    MooTermTag *tag = MOO_TERM_TAG (object);

    switch (prop_id)
    {
        case PROP_TAG_TABLE:
            g_value_set_pointer (value, tag->table);
            break;

        case PROP_NAME:
            g_value_set_string (value, tag->name);
            break;

        case PROP_HAS_ATTR:
            g_value_set_boolean (value, tag->attr.mask ? TRUE : FALSE);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}


void
_moo_term_tag_add_line (MooTermTag *tag,
                        gpointer    line)
{
    g_assert (MOO_IS_TERM_TAG (tag));
    g_assert (line != NULL);

    if (!g_slist_find (tag->lines, line))
        tag->lines = g_slist_insert_sorted (tag->lines, line, ptr_cmp);
}


void
_moo_term_tag_remove_line (MooTermTag *tag,
                           gpointer    line)
{
    g_assert (MOO_IS_TERM_TAG (tag));
    g_assert (line != NULL);

    tag->lines = g_slist_remove (tag->lines, line);
}


MooTermTagTable*
_moo_term_tag_table_new (void)
{
    MooTermTagTable *table = g_new0 (MooTermTagTable, 1);

    table->named_tags = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

    return table;
}


void
_moo_term_tag_table_free (MooTermTagTable *table)
{
    if (table)
    {
        GSList *l;

        for (l = table->tags; l != NULL; l = l->next)
        {
            MooTermTag *tag = l->data;
            g_slist_free (tag->lines);
            tag->lines = NULL;
            tag->table = NULL;
            g_object_unref (tag);
        }

        g_slist_free (table->tags);
        g_hash_table_destroy (table->named_tags);
        g_free (table);
    }
}


MooTermTag*
moo_term_create_tag (MooTerm            *term,
                     const char         *name)
{
    MooTermTagTable *table;
    MooTermTag *tag;

    g_return_val_if_fail (MOO_IS_TERM (term), NULL);

    table = moo_term_get_tag_table (term);
    tag = g_object_new (MOO_TYPE_TERM_TAG,
                        "tag-table", table,
                        "name", name,
                        NULL);

    if (name)
        g_hash_table_insert (table->named_tags, g_strdup (name), tag);
    table->tags = g_slist_append (table->tags, tag);

    return tag;
}


MooTermTag*
moo_term_get_tag (MooTerm            *term,
                  const char         *name)
{
    MooTermTagTable *table;

    g_return_val_if_fail (MOO_IS_TERM (term), NULL);
    g_return_val_if_fail (name != NULL, NULL);

    table = moo_term_get_tag_table (term);
    return g_hash_table_lookup (table->named_tags, name);
}


void
moo_term_delete_tag (MooTerm            *term,
                     MooTermTag         *tag)
{
    MooTermTagTable *table;
    GSList *lines, *l;

    g_return_if_fail (MOO_IS_TERM (term));
    g_return_if_fail (MOO_IS_TERM_TAG (tag));

    table = moo_term_get_tag_table (term);
    g_return_if_fail (tag->table == table);

    lines = tag->lines;
    tag->lines = NULL;

    for (l = lines; l != NULL; l = l->next)
    {
        MooTermLine *line = l->data;
        _moo_term_line_remove_tag (line, tag, 0,
                                   __moo_term_line_width (line));
    }

    if (tag->name)
        g_hash_table_remove (table->named_tags, tag->name);
    table->tags = g_slist_remove (table->tags, tag);
    tag->table = NULL;

    g_object_unref (tag);
    g_slist_free (lines);
}


MooTermTagTable*
moo_term_get_tag_table (MooTerm *term)
{
    g_return_val_if_fail (MOO_IS_TERM (term), NULL);
    return term->priv->primary_buffer->priv->tag_table;
}


void
moo_term_tag_set_attr (MooTermTag         *tag,
                       MooTermTextAttr     attr)
{
    /* TODO */
    g_return_if_fail (MOO_IS_TERM_TAG (tag));
    tag->attr = attr;
}


GType
moo_term_text_attr_mask_get_type (void)
{
    static GType type = 0;

    if (!type)
    {
        static const GFlagsValue values[] = {
            { MOO_TERM_TEXT_REVERSE, (char*)"MOO_TERM_TEXT_REVERSE", (char*)"reverse" },
            { MOO_TERM_TEXT_BLINK, (char*)"MOO_TERM_TEXT_BLINK", (char*)"blink" },
            { MOO_TERM_TEXT_FOREGROUND, (char*)"MOO_TERM_TEXT_FOREGROUND", (char*)"foreground" },
            { MOO_TERM_TEXT_BACKGROUND, (char*)"MOO_TERM_TEXT_BACKGROUND", (char*)"background" },
            { MOO_TERM_TEXT_BOLD, (char*)"MOO_TERM_TEXT_BOLD", (char*)"bold" },
            { MOO_TERM_TEXT_UNDERLINE, (char*)"MOO_TERM_TEXT_UNDERLINE", (char*)"underline" },
            { 0, NULL, NULL }
        };

        type = g_flags_register_static ("MooTermTextAttrMask", values);
    }

    return type;
}


GType
moo_term_text_color_get_type (void)
{
    static GType type = 0;

    if (!type)
    {
        static const GEnumValue values[] = {
            { MOO_TERM_BLACK, (char*)"MOO_TERM_BLACK", (char*)"black" },
            { MOO_TERM_RED, (char*)"MOO_TERM_RED", (char*)"red" },
            { MOO_TERM_GREEN, (char*)"MOO_TERM_GREEN", (char*)"green" },
            { MOO_TERM_YELLOW, (char*)"MOO_TERM_YELLOW", (char*)"yellow" },
            { MOO_TERM_BLUE, (char*)"MOO_TERM_BLUE", (char*)"blue" },
            { MOO_TERM_MAGENTA, (char*)"MOO_TERM_MAGENTA", (char*)"magenta" },
            { MOO_TERM_CYAN, (char*)"MOO_TERM_CYAN", (char*)"cyan" },
            { MOO_TERM_WHITE, (char*)"MOO_TERM_WHITE", (char*)"white" },
            { 0, NULL, NULL }
        };

        type = g_enum_register_static ("MooTermTextColor", values);
    }

    return type;
}
