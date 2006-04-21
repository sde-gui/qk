/*
 *   mooutils/mooaccelbutton.c
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

#include "mooutils/mooaccelbutton.h"
#include "mooutils/mooaccelbutton-glade.h"
#include "mooutils/mooglade.h"
#include "mooutils/moocompat.h"
#include "mooutils/moomarshals.h"
#include "mooutils/moodialogs.h"
#include <gtk/gtkaccelgroup.h>
#include <gtk/gtkdialog.h>
#include <gtk/gtklabel.h>
#include <gdk/gdkkeysyms.h>


enum {
    PROP_0,
    PROP_ACCEL,
    PROP_TITLE
};

enum {
    ACCEL_SET,
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = {0};


static void moo_accel_button_class_init    (MooAccelButtonClass *klass);
static void moo_accel_button_init          (MooAccelButton      *button);

static void moo_accel_button_finalize      (GObject             *object);
static void moo_accel_button_set_property  (GObject        *object,
                                            guint           param_id,
                                            const GValue   *value,
                                            GParamSpec     *pspec);
static void moo_accel_button_get_property  (GObject        *object,
                                            guint           param_id,
                                            GValue         *value,
                                            GParamSpec     *pspec);

static void moo_accel_button_clicked       (MooAccelButton *button);


G_DEFINE_TYPE(MooAccelButton, moo_accel_button, GTK_TYPE_BUTTON)


static void moo_accel_button_class_init    (MooAccelButtonClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->get_property = moo_accel_button_get_property;
    gobject_class->set_property = moo_accel_button_set_property;
    gobject_class->finalize = moo_accel_button_finalize;

    klass->accel_set = NULL;

    g_object_class_install_property (gobject_class,
                                     PROP_ACCEL,
                                     g_param_spec_string (
                                        "accel",
                                        "accel",
                                        "accel",
                                        NULL,
                                        G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class,
                                     PROP_TITLE,
                                     g_param_spec_string (
                                        "title",
                                        "title",
                                        "title",
                                        "Choose shortcut",
                                        G_PARAM_READWRITE));

    signals[ACCEL_SET] = g_signal_new ("accel-set",
                                       G_TYPE_FROM_CLASS (gobject_class),
                                       G_SIGNAL_RUN_LAST,
                                       G_STRUCT_OFFSET (MooAccelButtonClass, accel_set),
                                       NULL, NULL,
                                       _moo_marshal_VOID__STRING,
                                       G_TYPE_NONE, 1,
                                       G_TYPE_STRING);
}


static void moo_accel_button_init (MooAccelButton *button)
{
    button->accel = NULL;
    button->title = NULL;
    g_signal_connect (button, "clicked",
                      G_CALLBACK (moo_accel_button_clicked), NULL);
}


static void moo_accel_button_finalize (GObject *object)
{
    MooAccelButton *button = MOO_ACCEL_BUTTON (object);
    g_free (button->title);
    button->title = NULL;
    g_free (button->accel);
    button->accel = NULL;
    G_OBJECT_CLASS (moo_accel_button_parent_class)->finalize (object);
}


static void moo_accel_button_set_property  (GObject        *object,
                                            guint           param_id,
                                            const GValue   *value,
                                            GParamSpec     *pspec)
{
    MooAccelButton *button = MOO_ACCEL_BUTTON (object);

    switch (param_id)
    {
        case PROP_TITLE:
            moo_accel_button_set_title (button, g_value_get_string (value));
            break;

        case PROP_ACCEL:
            moo_accel_button_set_accel (button, g_value_get_string (value));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
            break;
    }
}


static void moo_accel_button_get_property  (GObject        *object,
                                            guint           param_id,
                                            GValue         *value,
                                            GParamSpec     *pspec)
{
    MooAccelButton *button = MOO_ACCEL_BUTTON (object);

    switch (param_id)
    {
        case PROP_TITLE:
            g_value_set_string (value, moo_accel_button_get_title (button));
            break;

        case PROP_ACCEL:
            g_value_set_string (value, moo_accel_button_get_accel (button));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
            break;
    }
}


GtkWidget   *moo_accel_button_new               (const char         *accel)
{
    return GTK_WIDGET (g_object_new (MOO_TYPE_ACCEL_BUTTON,
                                     "accel", accel, NULL));
}


const char  *moo_accel_button_get_title         (MooAccelButton     *button)
{
    g_return_val_if_fail (MOO_IS_ACCEL_BUTTON (button), NULL);
    return button->title;
}


void         moo_accel_button_set_title         (MooAccelButton     *button,
                                                 const char         *title)
{
    g_return_if_fail (MOO_IS_ACCEL_BUTTON (button));
    g_free (button->title);
    button->title = g_strdup (title);
    g_object_notify (G_OBJECT (button), "title");
}


const char  *moo_accel_button_get_accel         (MooAccelButton     *button)
{
    g_return_val_if_fail (MOO_IS_ACCEL_BUTTON (button), NULL);
    return button->accel;
}


gboolean     moo_accel_button_set_accel         (MooAccelButton     *button,
                                                 const char         *accel)
{
    guint accel_key;
    GdkModifierType accel_mods;

    g_return_val_if_fail (MOO_IS_ACCEL_BUTTON (button), FALSE);

    accel_key = 0;
    accel_mods = 0;
    if (accel && accel[0])
    {
        gtk_accelerator_parse (accel, &accel_key, &accel_mods);
        if (!accel_key && !accel_mods)
            return FALSE;
    }

    g_free (button->accel);
    if (accel_key || accel_mods) {
        char *label = gtk_accelerator_get_label (accel_key, accel_mods);
        button->accel = gtk_accelerator_name (accel_key, accel_mods);
        gtk_button_set_label (GTK_BUTTON (button), label);
        g_free (label);
    }
    else {
        button->accel = g_strdup ("");
        gtk_button_set_label (GTK_BUTTON (button), "");
    }

    g_object_notify (G_OBJECT (button), "accel");
    g_signal_emit (button, signals[ACCEL_SET], 0, button->accel);
    return TRUE;
}


typedef struct {
    int mods;
    guint key;
    GtkLabel *label;
} Stuff;


static gboolean key_event (G_GNUC_UNUSED GtkWidget    *widget,
                           GdkEventKey  *event,
                           Stuff        *s)
{
    if (gtk_accelerator_valid (event->keyval, event->state))
    {
        char *label = gtk_accelerator_get_label (event->keyval, event->state);
        gtk_label_set_text (s->label, label);
        g_free (label);
        s->key = event->keyval;
        s->mods = event->state;
    }

    return TRUE;
}


static void moo_accel_button_clicked       (MooAccelButton *button)
{
    MooGladeXML *xml;
    GtkWidget *dialog, *ok_button, *cancel_button, *eventbox, *label;
    Stuff s = {0, 0, NULL};
    int response;

    xml = moo_glade_xml_new_from_buf (MOO_ACCEL_BUTTON_GLADE_UI, -1, "dialog");
    g_return_if_fail (xml != NULL);

    dialog = moo_glade_xml_get_widget (xml, "dialog");

#if GTK_CHECK_VERSION(2,6,0)
    gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog),
                                             GTK_RESPONSE_OK,
                                             GTK_RESPONSE_CANCEL,
                                            -1);
#endif /* GTK_CHECK_VERSION(2,6,0) */

    if (button->title)
        gtk_window_set_title (GTK_WINDOW (dialog), button->title);

    moo_position_window (dialog,
                         gtk_widget_get_toplevel (GTK_WIDGET (button)),
                         FALSE, FALSE, 0, 0);

    ok_button = moo_glade_xml_get_widget (xml, "ok");
    cancel_button = moo_glade_xml_get_widget (xml, "cancel");
    eventbox = moo_glade_xml_get_widget (xml, "eventbox");
    label = moo_glade_xml_get_widget (xml, "label");

    gtk_button_set_use_underline (GTK_BUTTON (ok_button), FALSE);
    gtk_button_set_use_underline (GTK_BUTTON (cancel_button), FALSE);

    s.label = GTK_LABEL (label);

    g_signal_connect (eventbox, "key-press-event", G_CALLBACK (key_event), &s);

    response = gtk_dialog_run (GTK_DIALOG (dialog));

    gtk_widget_destroy (dialog);
    g_object_unref (xml);

    if (response == GTK_RESPONSE_OK)
    {
        if (s.key || s.mods)
        {
            char *accel = gtk_accelerator_name (s.key, s.mods);
            moo_accel_button_set_accel (button, accel);
            g_free (accel);
        }
        else
        {
            moo_accel_button_set_accel (button, "");
        }
    }
}
