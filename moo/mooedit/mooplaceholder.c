/*
 *   mooplaceholder.c
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
#include "mooedit/mooplaceholder.h"
#include <gtk/gtklabel.h>


G_DEFINE_TYPE(MooTextAnchor, _moo_text_anchor, GTK_TYPE_TEXT_CHILD_ANCHOR)


static void
_moo_text_anchor_class_init (G_GNUC_UNUSED MooTextAnchorClass *klass)
{
}


static void
_moo_text_anchor_init (MooTextAnchor *anchor)
{
    anchor->widget = NULL;
}


/***************************************************************************/
/* MooPlaceholder
 */

G_DEFINE_TYPE(MooPlaceholder, _moo_placeholder, GTK_TYPE_LABEL)


static void
moo_placeholder_update_size (GtkWidget *ph)
{
    GtkWidget *parent = ph->parent;

    if (parent && GTK_WIDGET_REALIZED (parent))
    {
        PangoContext *ctx;
        PangoLayout *layout;
        PangoLayoutLine *line;
        PangoRectangle rect;
        int height, rise;

        ctx = gtk_widget_get_pango_context (parent);
        g_return_if_fail (ctx != NULL);

        layout = pango_layout_new (ctx);
        pango_layout_set_text (layout, "AA", -1);
        line = pango_layout_get_line (layout, 0);

        pango_layout_line_get_extents (line, NULL, &rect);

        height = rect.height / PANGO_SCALE;
        rise = rect.y + rect.height;

        gtk_widget_set_size_request (ph, -1, height);
        gtk_widget_modify_font (ph, parent->style->font_desc);

        if (MOO_PLACEHOLDER(ph)->tag)
            g_object_set (MOO_PLACEHOLDER(ph)->tag, "rise", -rise, NULL);

        g_object_unref (layout);
    }
}


static void
moo_placeholder_parent_set (GtkWidget *widget,
                            GtkWidget *old_parent)
{
    if (GTK_WIDGET_CLASS (_moo_placeholder_parent_class)->parent_set)
        GTK_WIDGET_CLASS (_moo_placeholder_parent_class)->parent_set (widget, old_parent);

    if (old_parent)
        g_signal_handlers_disconnect_by_func (old_parent,
                                              (gpointer) moo_placeholder_update_size,
                                              widget);

    if (widget->parent)
    {
        g_signal_connect_swapped (widget->parent, "style-set",
                                  G_CALLBACK (moo_placeholder_update_size),
                                  widget);
        g_signal_connect_swapped (widget->parent, "realize",
                                  G_CALLBACK (moo_placeholder_update_size),
                                  widget);

        moo_placeholder_update_size (widget);
    }
}


static void
moo_placeholder_destroy (GtkObject *object)
{
    MooPlaceholder *ph = MOO_PLACEHOLDER (object);

    if (ph->tag)
    {
        gtk_text_tag_table_remove (ph->table, ph->tag);
        g_object_unref (ph->tag);
        g_object_unref (ph->table);
        ph->tag = NULL;
        ph->table = NULL;
    }

    GTK_OBJECT_CLASS(_moo_placeholder_parent_class)->destroy (object);
}


static void
_moo_placeholder_class_init (MooPlaceholderClass *klass)
{
    GTK_OBJECT_CLASS(klass)->destroy = moo_placeholder_destroy;
    GTK_WIDGET_CLASS(klass)->parent_set = moo_placeholder_parent_set;
}


static void
_moo_placeholder_init (MooPlaceholder *ph)
{
    gtk_label_set_text (GTK_LABEL (ph), "  ");
}


void
_moo_placeholder_set_tag (MooPlaceholder     *ph,
                          GtkTextTagTable    *table,
                          GtkTextTag         *tag)
{
    g_return_if_fail (ph->tag == NULL);
    ph->tag = g_object_ref (tag);
    ph->table = g_object_ref (table);
    moo_placeholder_update_size (GTK_WIDGET (ph));
}
