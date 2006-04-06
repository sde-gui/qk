/*
 *   mooutils/mooprefsdialogpage.c
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

#define MOOPREFS_COMPILATION
#include "mooutils/moomarshals.h"
#include "mooutils/moostock.h"
#include "mooutils/moocompat.h"
#include "mooutils/mooprefs.h"
#include "mooutils/mooprefsdialogpage.h"
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

static void moo_prefs_dialog_page_init_sig      (MooPrefsDialogPage *page);
static void moo_prefs_dialog_page_apply         (MooPrefsDialogPage *page);


enum {
    PROP_0,
    PROP_LABEL,
    PROP_ICON,
    PROP_ICON_STOCK_ID,
    PROP_AUTO_APPLY
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

    klass->init = moo_prefs_dialog_page_init_sig;
    klass->apply = moo_prefs_dialog_page_apply;
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

    g_object_class_install_property (gobject_class,
                                     PROP_AUTO_APPLY,
                                     g_param_spec_boolean
                                             ("auto-apply",
                                              "auto-apply",
                                              "auto-apply",
                                              TRUE,
                                              G_PARAM_READWRITE));

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
    page->widgets = NULL;
    page->auto_apply = TRUE;
}


static void moo_prefs_dialog_page_finalize       (GObject      *object)
{
    MooPrefsDialogPage *page = MOO_PREFS_DIALOG_PAGE (object);
    g_free (page->label);
    g_free (page->icon_stock_id);
    if (page->icon)
        g_object_unref (page->icon);
    if (page->xml)
        g_object_unref (page->xml);
    g_slist_free (page->widgets);

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

            if (g_value_get_string (value))
                page->label = g_markup_printf_escaped ("<b>%s</b>", g_value_get_string (value));
            else
                page->label = NULL;

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

        case PROP_AUTO_APPLY:
            page->auto_apply = g_value_get_boolean (value) != 0;
            g_object_notify (object, "auto-apply");
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

        case PROP_AUTO_APPLY:
            g_value_set_boolean (value, page->auto_apply);
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
                                    MooGladeXML        *xml,
                                    const char         *buffer,
                                    int                 buffer_size,
                                    const char         *page_id,
                                    const char         *prefs_root)
{
    MooPrefsDialogPage *page;
    struct {
        const char *prefs_root;
        const char *page_id;
    } data = {prefs_root, page_id};

    g_return_val_if_fail (buffer != NULL && page_id != NULL, NULL);
    g_return_val_if_fail (!xml || MOO_IS_GLADE_XML (xml), NULL);

    if (!xml)
        xml = moo_glade_xml_new_empty ();
    else
        g_object_ref (xml);

    moo_glade_xml_map_id (xml, page_id, MOO_TYPE_PREFS_DIALOG_PAGE);
    moo_glade_xml_map_signal (xml, connect_signals, &data);

    if (!moo_glade_xml_parse_memory (xml, buffer, buffer_size, page_id))
    {
        g_critical ("%s: could not parse xml", G_STRLOC);
        g_object_unref (xml);
        return NULL;
    }

    /*XXX*/
    page = moo_glade_xml_get_widget (xml, page_id);
    page->xml = xml;

    g_object_set (page,
                  "label", label,
                  "icon-stock-id", icon_stock_id,
                  NULL);

    return GTK_WIDGET (page);
}


/**************************************************************************/
/* Settings
 */

static void     setting_init        (GtkWidget      *widget);
static void     setting_apply       (GtkWidget      *widget);
static gboolean setting_get_value   (GtkWidget      *widget,
                                     GValue         *value);
static void     setting_set_value   (GtkWidget      *widget,
                                     const GValue   *value);


static void
moo_prefs_dialog_page_init_sig (MooPrefsDialogPage *page)
{
    g_slist_foreach (page->widgets, (GFunc) setting_init, NULL);
}


static void
moo_prefs_dialog_page_apply (MooPrefsDialogPage *page)
{
    g_slist_foreach (page->widgets, (GFunc) setting_apply, NULL);
}


static void
setting_init (GtkWidget *widget)
{
    const GValue *value;
    const char *prefs_key = g_object_get_data (G_OBJECT (widget), "moo-prefs-key");

    g_return_if_fail (prefs_key != NULL);

    value = moo_prefs_get (prefs_key);
    g_return_if_fail (value != NULL);

    setting_set_value (widget, value);
}


static void
setting_apply (GtkWidget *widget)
{
    const char *prefs_key = g_object_get_data (G_OBJECT (widget), "moo-prefs-key");
    GtkWidget *set_or_not = g_object_get_data (G_OBJECT (widget), "moo-prefs-set-or-not");
    GValue value;
    GType type;

    g_return_if_fail (prefs_key != NULL);

    if (!GTK_WIDGET_SENSITIVE (widget))
        return;

    if (set_or_not)
    {
        gboolean unset;

        if (!GTK_WIDGET_SENSITIVE (set_or_not))
            return;

        unset = !gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (set_or_not));

        if (unset)
            return;
    }

    type = moo_prefs_get_key_type (prefs_key);
    g_return_if_fail (type != G_TYPE_NONE);

    value.g_type = 0;
    g_value_init (&value, type);
    if (setting_get_value (widget, &value))
        moo_prefs_set (prefs_key, &value);
    g_value_unset (&value);
}


static gboolean
setting_get_value (GtkWidget      *widget,
                   GValue         *value)
{
    if (GTK_IS_SPIN_BUTTON (widget))
    {
        GtkSpinButton *spin = GTK_SPIN_BUTTON (widget);

        if (value->g_type == G_TYPE_INT)
        {
            int val = gtk_spin_button_get_value_as_int (spin);
            g_value_set_int (value, val);
            return TRUE;
        }
        else if (value->g_type == G_TYPE_DOUBLE)
        {
            double val = gtk_spin_button_get_value (spin);
            g_value_set_double (value, val);
            return TRUE;
        }
    }
    else if (GTK_IS_ENTRY (widget))
    {
        if (value->g_type == G_TYPE_STRING)
        {
            const char *val = gtk_entry_get_text (GTK_ENTRY (widget));
            g_value_set_string (value, val);
            return TRUE;
        }
    }
    else if (GTK_IS_FONT_BUTTON (widget))
    {
        if (value->g_type == G_TYPE_STRING)
        {
            const char *val = gtk_font_button_get_font_name (GTK_FONT_BUTTON (widget));
            g_value_set_string (value, val);
            return TRUE;
        }
    }
    else if (GTK_IS_COLOR_BUTTON (widget))
    {
        if (value->g_type == GDK_TYPE_COLOR)
        {
            GdkColor val;
            gtk_color_button_get_color (GTK_COLOR_BUTTON (widget), &val);
            g_value_set_boxed (value, &val);
            return TRUE;
        }
    }
    else if (GTK_IS_TOGGLE_BUTTON (widget))
    {
        if (value->g_type == G_TYPE_BOOLEAN)
        {
            gboolean val = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
            g_value_set_boolean (value, val);
            return TRUE;
        }
    }

    g_warning ("%s: could not get value of type '%s' from widget '%s'",
               G_STRLOC, g_type_name (value->g_type),
               g_type_name (G_OBJECT_TYPE (widget)));
    return FALSE;
}


static void setting_set_value   (GtkWidget      *widget,
                                 const GValue   *value)
{
    if (GTK_IS_SPIN_BUTTON (widget))
    {
        GtkSpinButton *spin = GTK_SPIN_BUTTON (widget);

        if (value->g_type == G_TYPE_INT)
        {
            gtk_spin_button_set_value (spin, g_value_get_int (value));
            return;
        }
        else if (value->g_type == G_TYPE_DOUBLE)
        {
            gtk_spin_button_set_value (spin, g_value_get_double (value));
            return;
        }
    }
    else if (GTK_IS_ENTRY (widget))
    {
        if (value->g_type == G_TYPE_STRING)
        {
            const char *val = g_value_get_string (value);
            if (!val)
                val = "";
            gtk_entry_set_text (GTK_ENTRY (widget), val);
            return;
        }
    }
    else if (GTK_IS_FONT_BUTTON (widget))
    {
        if (value->g_type == G_TYPE_STRING)
        {
            const char *val = g_value_get_string (value);
            if (!val)
                val = "";
            g_object_set (widget, "font-name", val, NULL);
            return;
        }
    }
    else if (GTK_IS_COLOR_BUTTON (widget))
    {
        if (value->g_type == GDK_TYPE_COLOR)
        {
            GdkColor *val = g_value_get_boxed (value);
            gtk_color_button_set_color (GTK_COLOR_BUTTON (widget), val);
            return;
        }
    }
    else if (GTK_IS_TOGGLE_BUTTON (widget))
    {
        if (value->g_type == G_TYPE_BOOLEAN)
        {
            gboolean val = g_value_get_boolean (value);
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget), val);
            return;
        }
    }

    g_warning ("%s: could not set value of type '%s' to widget '%s'",
               G_STRLOC, g_type_name (value->g_type),
               g_type_name (G_OBJECT_TYPE (widget)));
}


void
moo_prefs_dialog_page_bind_setting (MooPrefsDialogPage *page,
                                    GtkWidget          *widget,
                                    const char         *setting,
                                    GtkToggleButton    *set_or_not)
{
    g_return_if_fail (MOO_IS_PREFS_DIALOG_PAGE (page));
    g_return_if_fail (GTK_IS_WIDGET (widget));
    g_return_if_fail (setting != NULL);
    g_return_if_fail (!set_or_not || GTK_IS_TOGGLE_BUTTON (set_or_not));

    g_object_set_data_full (G_OBJECT (widget), "moo-prefs-key",
                            g_strdup (setting), g_free);
    g_object_set_data (G_OBJECT (widget), "moo-prefs-set-or-not", set_or_not);

    page->widgets = g_slist_prepend (page->widgets, widget);
}
