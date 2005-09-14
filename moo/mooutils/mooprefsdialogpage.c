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
#include "mooutils/mooprefs.h"
#include <string.h>


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
    page->xml = NULL;
}


static void moo_prefs_dialog_page_finalize       (GObject      *object)
{
    MooPrefsDialogPage *page = MOO_PREFS_DIALOG_PAGE (object);
    g_free (page->label);
    g_free (page->icon_stock_id);
    if (page->icon)
        g_object_unref (page->icon);
    if (page->xml)
        moo_glade_xml_unref (page->xml);

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

GtkWidget*
moo_prefs_dialog_page_new (const char         *label,
                           const char         *icon_stock_id)
{
    return g_object_new (MOO_TYPE_PREFS_DIALOG_PAGE,
                         "label", label,
                         "icon-stock-id", icon_stock_id,
                         NULL);
}


static gboolean
connect_prefs_key (MooGladeXML    *xml,
                   GtkWidget      *widget,
                   const char     *handler,
                   const char     *object,
                   gpointer        user_data)
{
    char *key = NULL;
    GtkToggleButton *set_or_not = NULL;
    MooPrefsDialogPage *page;

    struct {
        const char *prefs_root;
        const char *page_id;
    } *data = user_data;

    page = moo_glade_xml_get_widget (xml, data->page_id);
    g_return_val_if_fail (page != NULL, FALSE);

    key = data->prefs_root ?
            moo_prefs_make_key (data->prefs_root, handler, NULL) :
            g_strdup (handler);
    g_return_val_if_fail (key != NULL, FALSE);

    if (!moo_prefs_key_registered (key))
    {
        g_warning ("%s: key '%s' is not registered", G_STRLOC, key);
        g_free (key);
        return FALSE;
    }

    if (object)
    {
        set_or_not = moo_glade_xml_get_widget (xml, object);
        if (!set_or_not)
        {
            g_warning ("%s: could not find widget '%s'", G_STRLOC, object);
            g_free (key);
            return FALSE;
        }
    }

    moo_prefs_dialog_page_bind_setting (page, widget, key, set_or_not);

    g_free (key);
    return TRUE;
}


static gboolean
connect_signals (MooGladeXML    *xml,
                 G_GNUC_UNUSED const char *widget_id,
                 GtkWidget      *widget,
                 const char     *signal,
                 const char     *handler,
                 const char     *object,
                 gpointer        user_data)
{
    if (!strcmp (signal, "moo-prefs-key"))
        return connect_prefs_key (xml, widget, handler, object, user_data);
    else
        return FALSE;
}


GtkWidget*
moo_prefs_dialog_page_new_from_xml (const char         *label,
                                    const char         *icon_stock_id,
                                    const char         *buffer,
                                    int                 buffer_size,
                                    const char         *page_id,
                                    const char         *prefs_root)
{
    MooPrefsDialogPage *page;
    MooGladeXML *xml;
    struct {
        const char *prefs_root;
        const char *page_id;
    } data = {prefs_root, page_id};

    g_return_val_if_fail (buffer != NULL && page_id != NULL, NULL);

    xml = moo_glade_xml_new_empty ();
    moo_glade_xml_map_id (xml, page_id, MOO_TYPE_PREFS_DIALOG_PAGE);
    moo_glade_xml_map_signal (xml, connect_signals, &data);

    if (!moo_glade_xml_parse_memory (xml, buffer, buffer_size, page_id))
    {
        g_critical ("%s: could not parse xml", G_STRLOC);
        moo_glade_xml_unref (xml);
        return NULL;
    }

    page = moo_glade_xml_get_widget (xml, page_id);
    page->xml = xml;

    g_object_set (page,
                  "label", label,
                  "icon-stock-id", icon_stock_id,
                  NULL);

    return GTK_WIDGET (page);
}
