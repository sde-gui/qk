/*
 *   mooutils/mooprefsdialogpage.c
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

#define MOOPREFS_COMPILATION
#include "mooutils/mooprefsdialog-settings.h"
#include "mooutils/moomarshals.h"
#include "mooutils/moostock.h"
#include "mooutils/moocompat.h"


/**************************************************************************/
/* MooPrefsDialogPage class implementation
 */
static void moo_prefs_dialog_page_class_init    (MooPrefsDialogPageClass    *klass);

static void moo_prefs_dialog_page_init          (MooPrefsDialogPage *page);
static void moo_prefs_dialog_page_finalize      (GObject            *object);

static void moo_prefs_dialog_page_set_property  (GObject            *object,
                                                 guint               prop_id,
                                                 const GValue       *value,
                                                 GParamSpec         *pspec);
static void moo_prefs_dialog_page_get_property  (GObject            *object,
                                                 guint               prop_id,
                                                 GValue             *value,
                                                 GParamSpec         *pspec);


enum {
    PROP_0,
    PROP_LABEL,
    PROP_ICON,
    PROP_ICON_STOCK_ID
};

enum {
    APPLY,
    INIT,
    SET_DEFAULTS,
    LAST_SIGNAL
};


static guint prefs_dialog_page_signals[LAST_SIGNAL] = {0};


/* MOO_TYPE_PREFS_DIALOG_PAGE */
G_DEFINE_TYPE (MooPrefsDialogPage, moo_prefs_dialog_page, GTK_TYPE_VBOX)


static void moo_prefs_dialog_page_class_init (MooPrefsDialogPageClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->set_property = moo_prefs_dialog_page_set_property;
    gobject_class->get_property = moo_prefs_dialog_page_get_property;
    gobject_class->finalize = moo_prefs_dialog_page_finalize;

    klass->init = _moo_prefs_dialog_page_init_sig;
    klass->apply = _moo_prefs_dialog_page_apply;
    klass->set_defaults = NULL;

    g_object_class_install_property (gobject_class,
                                     PROP_LABEL,
                                     g_param_spec_string
                                             ("label",
                                              "label",
                                              "Label",
                                              NULL,
                                              G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_ICON_STOCK_ID,
                                     g_param_spec_string
                                             ("icon-stock-id",
                                              "icon-stock-id",
                                              "icon-stock-id",
                                              NULL,
                                              G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_ICON,
                                     g_param_spec_object
                                             ("icon",
                                              "icon",
                                              "Icon",
                                              GDK_TYPE_PIXBUF,
                                              G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    prefs_dialog_page_signals[APPLY] =
        g_signal_new ("apply",
                      G_OBJECT_CLASS_TYPE (klass),
                      (GSignalFlags) (G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION),
                      G_STRUCT_OFFSET (MooPrefsDialogPageClass, apply),
                      NULL, NULL,
                      _moo_marshal_VOID__VOID,
                      G_TYPE_NONE, 0);

    prefs_dialog_page_signals[INIT] =
        g_signal_new ("init",
                      G_OBJECT_CLASS_TYPE (klass),
                      (GSignalFlags) (G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION),
                      G_STRUCT_OFFSET (MooPrefsDialogPageClass, init),
                      NULL, NULL,
                      _moo_marshal_VOID__VOID,
                      G_TYPE_NONE, 0);

    prefs_dialog_page_signals[SET_DEFAULTS] =
        g_signal_new ("set-defaults",
                      G_OBJECT_CLASS_TYPE (klass),
                      (GSignalFlags) (G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION),
                      G_STRUCT_OFFSET (MooPrefsDialogPageClass, set_defaults),
                      NULL, NULL,
                      _moo_marshal_VOID__VOID,
                      G_TYPE_NONE, 0);
}


static void moo_prefs_dialog_page_init (MooPrefsDialogPage *page)
{
    page->label = NULL;
    page->icon = NULL;
    page->icon_stock_id = NULL;
}


static void moo_prefs_dialog_page_finalize       (GObject      *object)
{
    MooPrefsDialogPage *page = MOO_PREFS_DIALOG_PAGE (object);
    g_free (page->label);
    g_free (page->icon_stock_id);
    if (page->icon)
        g_object_unref (page->icon);

    G_OBJECT_CLASS (moo_prefs_dialog_page_parent_class)->finalize (object);
}


static void moo_prefs_dialog_page_set_property  (GObject            *object,
                                                 guint               prop_id,
                                                 const GValue       *value,
                                                 GParamSpec         *pspec)
{
    MooPrefsDialogPage *page = MOO_PREFS_DIALOG_PAGE (object);

    switch (prop_id)
    {
        case PROP_LABEL:
            g_free (page->label);
            page->label = g_strdup_printf ("<b>%s</b>",
                                           g_value_get_string (value));
            g_object_notify (G_OBJECT (page), "label");
            break;

        case PROP_ICON:
            if (page->icon)
                g_object_unref (G_OBJECT (page->icon));
            page->icon = GDK_PIXBUF (g_value_dup_object (value));
            g_object_notify (G_OBJECT (page), "icon");
            break;

        case PROP_ICON_STOCK_ID:
            g_free (page->icon_stock_id);
            page->icon_stock_id = g_strdup (g_value_get_string (value));
            g_object_notify (G_OBJECT (page), "icon-stock-id");
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void moo_prefs_dialog_page_get_property  (GObject            *object,
                                                 guint               prop_id,
                                                 GValue             *value,
                                                 GParamSpec         *pspec)
{
    MooPrefsDialogPage *page = MOO_PREFS_DIALOG_PAGE (object);

    switch (prop_id)
    {
        case PROP_LABEL:
            g_value_set_string (value, page->label);
            break;

        case PROP_ICON:
            g_value_set_object (value, page->icon);
            break;

        case PROP_ICON_STOCK_ID:
            g_value_set_string (value, page->icon_stock_id);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


/**************************************************************************/
/* MooPrefsDialogPage methods
 */

GtkWidget*  moo_prefs_dialog_page_new           (const char         *label,
                                                 const char         *icon_id)
{
    GtkWidget *page = GTK_WIDGET (g_object_new (MOO_TYPE_PREFS_DIALOG_PAGE,
                                                "label", label, NULL));

    g_object_set (page, "icon-stock-id", icon_id, NULL);

    return page;
}
