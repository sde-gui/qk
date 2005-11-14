/*
 *   mooentry.c
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

#include MOO_MARSHALS_H
#ifndef __MOO__
#include "mooentry.h"
#include "gtksourceundomanager.h"
#else
#include "mooutils/mooentry.h"
#include "mooutils/mooutils-gobject.h"
#include "mooutils/gtksourceundomanager.h"
#endif
#include <gtk/gtkbindings.h>
#include <gtk/gtkimagemenuitem.h>
#include <gtk/gtkseparatormenuitem.h>
#include <gtk/gtkstock.h>
#include <gdk/gdkkeysyms.h>


struct _MooEntryPrivate {
    GtkSourceUndoManager *undo_mgr;
    gboolean enable_undo;
    gboolean enable_undo_menu;
    guint user_action_stack;
    guint use_ctrl_u : 1;
    guint grab_selection : 1;
    guint fool_entry : 1;
};


static void     moo_entry_class_init        (MooEntryClass      *klass);
static void     moo_entry_editable_init     (GtkEditableClass   *klass);

static void     moo_entry_init              (MooEntry           *entry);
static void     moo_entry_finalize          (GObject            *object);
static void     moo_entry_set_property      (GObject            *object,
                                             guint               prop_id,
                                             const GValue       *value,
                                             GParamSpec         *pspec);
static void     moo_entry_get_property      (GObject            *object,
                                             guint               prop_id,
                                             GValue             *value,
                                             GParamSpec         *pspec);

static gboolean moo_entry_button_release    (GtkWidget          *widget,
                                             GdkEventButton     *event);

static void     moo_entry_delete_to_start   (MooEntry           *entry);

static void     moo_entry_populate_popup    (GtkEntry           *entry,
                                             GtkMenu            *menu);

static void     moo_entry_insert_at_cursor  (GtkEntry           *entry,
                                             const gchar        *str);
static void     moo_entry_delete_from_cursor(GtkEntry           *entry,
                                             GtkDeleteType       type,
                                             gint                count);
static void     moo_entry_backspace         (GtkEntry           *entry);
static void     moo_entry_cut_clipboard     (GtkEntry           *entry);
static void     moo_entry_paste_clipboard   (GtkEntry           *entry);

static void     moo_entry_do_insert_text    (GtkEditable        *editable,
                                             const gchar        *text,
                                             gint                length,
                                             gint               *position);
static void     moo_entry_do_delete_text    (GtkEditable        *editable,
                                             gint                start_pos,
                                             gint                end_pos);
static void     moo_entry_insert_text       (GtkEditable        *editable,
                                             const gchar        *text,
                                             gint                length,
                                             gint               *position);
static void     moo_entry_delete_text       (GtkEditable        *editable,
                                             gint                start_pos,
                                             gint                end_pos);
static void     moo_entry_set_selection_bounds (GtkEditable     *editable,
                                             gint                start_pos,
                                             gint                end_pos);
static gboolean moo_entry_get_selection_bounds (GtkEditable     *editable,
                                             gint               *start_pos,
                                             gint               *end_pos);



GType
moo_entry_get_type (void)
{
    static GType type = 0;

    if (!type)
    {
        static const GTypeInfo info =
        {
            sizeof (MooEntryClass),
            NULL,		/* base_init */
            NULL,		/* base_finalize */
            (GClassInitFunc) moo_entry_class_init,
            NULL,		/* class_finalize */
            NULL,		/* class_data */
            sizeof (MooEntry),
            0,
            (GInstanceInitFunc) moo_entry_init,
            NULL
        };

        static const GInterfaceInfo editable_info =
        {
            (GInterfaceInitFunc) moo_entry_editable_init,
            NULL,
            NULL
        };

        type = g_type_register_static (GTK_TYPE_ENTRY, "MooEntry", &info, 0);
        g_type_add_interface_static (type, GTK_TYPE_EDITABLE, &editable_info);
    }

    return type;
}


enum {
    PROP_0,
    PROP_UNDO_MANAGER,
    PROP_ENABLE_UNDO,
    PROP_ENABLE_UNDO_MENU,
    PROP_GRAB_SELECTION
};

enum {
    UNDO,
    REDO,
    BEGIN_USER_ACTION,
    END_USER_ACTION,
    DELETE_TO_START,
    NUM_SIGNALS
};

static guint signals[NUM_SIGNALS];
static GtkEditableClass *parent_editable_iface;
static gpointer moo_entry_parent_class;

static void
moo_entry_class_init (MooEntryClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
    GtkEntryClass *entry_class = GTK_ENTRY_CLASS (klass);
    GtkBindingSet *binding_set;

    gobject_class->finalize = moo_entry_finalize;
    gobject_class->set_property = moo_entry_set_property;
    gobject_class->get_property = moo_entry_get_property;

    widget_class->button_release_event = moo_entry_button_release;

    entry_class->populate_popup = moo_entry_populate_popup;
    entry_class->insert_at_cursor = moo_entry_insert_at_cursor;
    entry_class->delete_from_cursor = moo_entry_delete_from_cursor;
    entry_class->backspace = moo_entry_backspace;
    entry_class->cut_clipboard = moo_entry_cut_clipboard;
    entry_class->paste_clipboard = moo_entry_paste_clipboard;

    klass->undo = moo_entry_undo;
    klass->redo = moo_entry_redo;

    moo_entry_parent_class = g_type_class_peek_parent (klass);
    parent_editable_iface = g_type_interface_peek (moo_entry_parent_class, GTK_TYPE_EDITABLE);

    g_object_class_install_property (gobject_class,
                                     PROP_UNDO_MANAGER,
                                     g_param_spec_object ("undo-manager",
                                             "undo-manager",
                                             "undo-manager",
                                             GTK_SOURCE_TYPE_UNDO_MANAGER,
                                             G_PARAM_READABLE));

    g_object_class_install_property (gobject_class,
                                     PROP_ENABLE_UNDO,
                                     g_param_spec_boolean ("enable-undo",
                                             "enable-undo",
                                             "enable-undo",
                                             TRUE,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_ENABLE_UNDO_MENU,
                                     g_param_spec_boolean ("enable-undo-menu",
                                             "enable-undo-menu",
                                             "enable-undo-menu",
                                             TRUE,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_GRAB_SELECTION,
                                     g_param_spec_boolean ("grab-selection",
                                             "grab-selection",
                                             "grab-selection",
                                             FALSE,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    signals[UNDO] =
            g_signal_new ("undo",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                          G_STRUCT_OFFSET (MooEntryClass, undo),
                          NULL, NULL,
                          _moo_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);

    signals[REDO] =
            g_signal_new ("redo",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                          G_STRUCT_OFFSET (MooEntryClass, redo),
                          NULL, NULL,
                          _moo_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);

    signals[BEGIN_USER_ACTION] =
            g_signal_new ("begin-user-action",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                          G_STRUCT_OFFSET (MooEntryClass, begin_user_action),
                          NULL, NULL,
                          _moo_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);

    signals[END_USER_ACTION] =
            g_signal_new ("end-user-action",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                          G_STRUCT_OFFSET (MooEntryClass, end_user_action),
                          NULL, NULL,
                          _moo_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);

    signals[DELETE_TO_START] =
            moo_signal_new_cb ("delete-to-start",
                               G_OBJECT_CLASS_TYPE (klass),
                               G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                               G_CALLBACK (moo_entry_delete_to_start),
                               NULL, NULL,
                               _moo_marshal_VOID__VOID,
                               G_TYPE_NONE, 0);

    binding_set = gtk_binding_set_by_class (klass);
    gtk_binding_entry_add_signal (binding_set, GDK_z,
                                  GDK_CONTROL_MASK,
                                  "undo", 0);
    gtk_binding_entry_add_signal (binding_set, GDK_z,
                                  GDK_CONTROL_MASK | GDK_SHIFT_MASK,
                                  "redo", 0);
    gtk_binding_entry_add_signal (binding_set, GDK_u,
                                  GDK_CONTROL_MASK,
                                  "delete-to-start", 0);
}


static void
moo_entry_editable_init (GtkEditableClass   *klass)
{
    klass->do_insert_text = moo_entry_do_insert_text;
    klass->do_delete_text = moo_entry_do_delete_text;
    klass->insert_text = moo_entry_insert_text;
    klass->delete_text = moo_entry_delete_text;
    klass->set_selection_bounds = moo_entry_set_selection_bounds;
    klass->get_selection_bounds = moo_entry_get_selection_bounds;
}


static void
moo_entry_init (MooEntry *entry)
{
    entry->priv = g_new0 (MooEntryPrivate, 1);
    entry->priv->undo_mgr = gtk_source_undo_manager_new (entry);
    entry->priv->use_ctrl_u = TRUE;
}


static void
moo_entry_set_property (GObject        *object,
                        guint           prop_id,
                        const GValue   *value,
                        GParamSpec     *pspec)
{
    MooEntry *entry = MOO_ENTRY (object);

    switch (prop_id)
    {
        case PROP_ENABLE_UNDO:
            entry->priv->enable_undo = g_value_get_boolean (value);
            g_object_notify (object, "enable-undo");
            break;

        case PROP_ENABLE_UNDO_MENU:
            entry->priv->enable_undo_menu = g_value_get_boolean (value);
            g_object_notify (object, "enable-undo-menu");
            break;

        case PROP_GRAB_SELECTION:
            entry->priv->grab_selection = g_value_get_boolean (value) ? TRUE : FALSE;
            g_object_notify (object, "grab-selection");
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}


static void
moo_entry_get_property (GObject        *object,
                        guint           prop_id,
                        GValue         *value,
                        GParamSpec     *pspec)
{
    MooEntry *entry = MOO_ENTRY (object);

    switch (prop_id)
    {
        case PROP_ENABLE_UNDO:
            g_value_set_boolean (value, entry->priv->enable_undo);
            break;

        case PROP_ENABLE_UNDO_MENU:
            g_value_set_boolean (value, entry->priv->enable_undo_menu);
            break;

        case PROP_UNDO_MANAGER:
            g_value_set_object (value, entry->priv->undo_mgr);
            break;

        case PROP_GRAB_SELECTION:
            g_value_set_boolean (value, entry->priv->grab_selection ? TRUE : FALSE);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}


static void
moo_entry_finalize (GObject *object)
{
    MooEntry *entry = MOO_ENTRY (object);

    g_object_unref (entry->priv->undo_mgr);
    g_free (entry->priv);

    G_OBJECT_CLASS (moo_entry_parent_class)->finalize (object);
}


void
moo_entry_undo (MooEntry       *entry)
{
    g_return_if_fail (MOO_IS_ENTRY (entry));
    if (entry->priv->enable_undo && gtk_source_undo_manager_can_undo (entry->priv->undo_mgr))
        gtk_source_undo_manager_undo (entry->priv->undo_mgr);
}


void
moo_entry_redo (MooEntry       *entry)
{
    g_return_if_fail (MOO_IS_ENTRY (entry));
    if (entry->priv->enable_undo && gtk_source_undo_manager_can_redo (entry->priv->undo_mgr))
        gtk_source_undo_manager_redo (entry->priv->undo_mgr);
}


GtkWidget*
moo_entry_new (void)
{
    return g_object_new (MOO_TYPE_ENTRY, NULL);
}


static void
moo_entry_populate_popup (GtkEntry           *gtkentry,
                          GtkMenu            *menu)
{
    GtkWidget *item;
    MooEntry *entry = MOO_ENTRY (gtkentry);

    if (!entry->priv->enable_undo_menu)
        return;

    item = gtk_separator_menu_item_new ();
    gtk_widget_show (item);
    gtk_menu_shell_prepend (GTK_MENU_SHELL (menu), item);

    item = gtk_image_menu_item_new_from_stock (GTK_STOCK_REDO, NULL);
    gtk_widget_show (item);
    gtk_menu_shell_prepend (GTK_MENU_SHELL (menu), item);
    gtk_widget_set_sensitive (item, entry->priv->enable_undo &&
                                    gtk_source_undo_manager_can_redo (entry->priv->undo_mgr));
    g_signal_connect_swapped (item, "activate", G_CALLBACK (moo_entry_redo), entry);

    item = gtk_image_menu_item_new_from_stock (GTK_STOCK_UNDO, NULL);
    gtk_widget_show (item);
    gtk_menu_shell_prepend (GTK_MENU_SHELL (menu), item);
    gtk_widget_set_sensitive (item, entry->priv->enable_undo &&
                                     gtk_source_undo_manager_can_undo (entry->priv->undo_mgr));
    g_signal_connect_swapped (item, "activate", G_CALLBACK (moo_entry_undo), entry);
}


void
moo_entry_begin_user_action (MooEntry   *entry)
{
    g_return_if_fail (MOO_IS_ENTRY (entry));

    entry->priv->user_action_stack++;

    if (entry->priv->user_action_stack == 1)
        g_signal_emit (entry, signals[BEGIN_USER_ACTION], 0);
}


void
moo_entry_end_user_action (MooEntry   *entry)
{
    g_return_if_fail (MOO_IS_ENTRY (entry));
    g_return_if_fail (entry->priv->user_action_stack > 0);

    entry->priv->user_action_stack--;

    if (!entry->priv->user_action_stack)
        g_signal_emit (entry, signals[END_USER_ACTION], 0);
}


void
moo_entry_begin_not_undoable_action (MooEntry   *entry)
{
    g_return_if_fail (MOO_IS_ENTRY (entry));
    gtk_source_undo_manager_begin_not_undoable_action (entry->priv->undo_mgr);
}


void
moo_entry_end_not_undoable_action (MooEntry   *entry)
{
    g_return_if_fail (MOO_IS_ENTRY (entry));
    gtk_source_undo_manager_end_not_undoable_action (entry->priv->undo_mgr);
}


static void
moo_entry_insert_at_cursor (GtkEntry           *entry,
                            const gchar        *str)
{
    moo_entry_begin_user_action (MOO_ENTRY (entry));
    GTK_ENTRY_CLASS(moo_entry_parent_class)->insert_at_cursor (entry, str);
    moo_entry_end_user_action (MOO_ENTRY (entry));
}

static void
moo_entry_delete_from_cursor (GtkEntry           *entry,
                              GtkDeleteType       type,
                              gint                count)
{
    moo_entry_begin_user_action (MOO_ENTRY (entry));
    GTK_ENTRY_CLASS(moo_entry_parent_class)->delete_from_cursor (entry, type, count);
    moo_entry_end_user_action (MOO_ENTRY (entry));
}

static void
moo_entry_backspace (GtkEntry           *entry)
{
    moo_entry_begin_user_action (MOO_ENTRY (entry));
    GTK_ENTRY_CLASS(moo_entry_parent_class)->backspace (entry);
    moo_entry_end_user_action (MOO_ENTRY (entry));
}

static void
moo_entry_cut_clipboard (GtkEntry           *entry)
{
    moo_entry_begin_user_action (MOO_ENTRY (entry));
    GTK_ENTRY_CLASS(moo_entry_parent_class)->cut_clipboard (entry);
    moo_entry_end_user_action (MOO_ENTRY (entry));
}

static void
moo_entry_paste_clipboard (GtkEntry           *entry)
{
    moo_entry_begin_user_action (MOO_ENTRY (entry));
    GTK_ENTRY_CLASS(moo_entry_parent_class)->paste_clipboard (entry);
    moo_entry_end_user_action (MOO_ENTRY (entry));
}

static void
moo_entry_do_insert_text (GtkEditable        *editable,
                          const gchar        *text,
                          gint                length,
                          gint               *position)
{
    moo_entry_begin_user_action (MOO_ENTRY (editable));
    parent_editable_iface->do_insert_text (editable, text, length, position);
    moo_entry_end_user_action (MOO_ENTRY (editable));
}

static void
moo_entry_do_delete_text (GtkEditable        *editable,
                          gint                start_pos,
                          gint                end_pos)
{
    moo_entry_begin_user_action (MOO_ENTRY (editable));
    parent_editable_iface->do_delete_text (editable, start_pos, end_pos);
    moo_entry_end_user_action (MOO_ENTRY (editable));
}

static void
moo_entry_insert_text (GtkEditable        *editable,
                       const gchar        *text,
                       gint                length,
                       gint               *position)
{
    moo_entry_begin_user_action (MOO_ENTRY (editable));
    parent_editable_iface->insert_text (editable, text, length, position);
    moo_entry_end_user_action (MOO_ENTRY (editable));
}

static void
moo_entry_delete_text (GtkEditable        *editable,
                       gint                start_pos,
                       gint                end_pos)
{
    moo_entry_begin_user_action (MOO_ENTRY (editable));
    parent_editable_iface->delete_text (editable, start_pos, end_pos);
    moo_entry_end_user_action (MOO_ENTRY (editable));
}


static void
moo_entry_delete_to_start (MooEntry *entry)
{
    if (entry->priv->use_ctrl_u)
        gtk_editable_delete_text (GTK_EDITABLE (entry),
                                  0, gtk_editable_get_position (GTK_EDITABLE (entry)));
}


/*********************************************************************/
/* Working around gtk idiotic selection business
 */

/* GtkEdiatble::delete_text and GtkWidget::realize might also require this hack */

static void
moo_entry_set_selection_bounds (GtkEditable *editable,
                                gint         start_pos,
                                gint         end_pos)
{
    if (!MOO_ENTRY(editable)->priv->grab_selection)
        MOO_ENTRY(editable)->priv->fool_entry = TRUE;

    parent_editable_iface->set_selection_bounds (editable, start_pos, end_pos);
    MOO_ENTRY(editable)->priv->fool_entry = FALSE;
}

static gboolean
moo_entry_get_selection_bounds (GtkEditable *editable,
                                gint        *start_pos,
                                gint        *end_pos)
{
    if (MOO_ENTRY(editable)->priv->fool_entry)
        return FALSE;
    else
        return parent_editable_iface->get_selection_bounds (editable, start_pos, end_pos);
}

static gboolean
moo_entry_button_release (GtkWidget          *widget,
                          GdkEventButton     *event)
{
    gboolean result;

    if (!MOO_ENTRY(widget)->priv->grab_selection)
        MOO_ENTRY(widget)->priv->fool_entry = TRUE;

    result = GTK_WIDGET_CLASS(moo_entry_parent_class)->button_release_event (widget, event);
    MOO_ENTRY(widget)->priv->fool_entry = FALSE;

    return result;
}
