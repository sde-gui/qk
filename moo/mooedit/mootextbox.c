/*
 *   mootextbox.c
 *
 *   Copyright (C) 2004-2008 by Yevgen Muntyan <muntyan@tamu.edu>
 *
 *   This file is part of medit.  medit is free software; you can
 *   redistribute it and/or modify it under the terms of the
 *   GNU Lesser General Public License as published by the
 *   Free Software Foundation; either version 2.1 of the License,
 *   or (at your option) any later version.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with medit.  If not, see <http://www.gnu.org/licenses/>.
 */

#define MOOEDIT_COMPILATION
#include "mooedit/mootextbox.h"
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
/* MooTextBox
 */

G_DEFINE_TYPE(MooTextBox, _moo_text_box, GTK_TYPE_LABEL)


static void
moo_text_box_update_size (GtkWidget *box)
{
    GtkWidget *parent = box->parent;

    if (parent && GTK_WIDGET_REALIZED (parent))
    {
        PangoContext *ctx;
        PangoLayout *layout;
        PangoLayoutLine *line;
        PangoRectangle rect;
        int height;

        ctx = gtk_widget_get_pango_context (parent);
        g_return_if_fail (ctx != NULL);

        layout = pango_layout_new (ctx);
        pango_layout_set_text (layout, "AA", -1);
        line = pango_layout_get_line (layout, 0);

        pango_layout_line_get_extents (line, NULL, &rect);

        height = rect.height / PANGO_SCALE;

        gtk_widget_set_size_request (box, -1, height);
        gtk_widget_modify_font (box, parent->style->font_desc);

        g_object_unref (layout);
    }
}


static void
moo_text_box_parent_set (GtkWidget *widget,
                         GtkWidget *old_parent)
{
    if (GTK_WIDGET_CLASS (_moo_text_box_parent_class)->parent_set)
        GTK_WIDGET_CLASS (_moo_text_box_parent_class)->parent_set (widget, old_parent);

    if (old_parent)
        g_signal_handlers_disconnect_by_func (old_parent,
                                              (gpointer) moo_text_box_update_size,
                                              widget);

    if (widget->parent)
    {
        g_signal_connect_swapped (widget->parent, "style-set",
                                  G_CALLBACK (moo_text_box_update_size),
                                  widget);
        g_signal_connect_swapped (widget->parent, "realize",
                                  G_CALLBACK (moo_text_box_update_size),
                                  widget);

        moo_text_box_update_size (widget);
    }
}


static void
_moo_text_box_class_init (MooTextBoxClass *klass)
{
    GTK_WIDGET_CLASS(klass)->parent_set = moo_text_box_parent_set;
}


static void
_moo_text_box_init (MooTextBox *box)
{
    gtk_label_set_text (GTK_LABEL (box), "  ");
}
