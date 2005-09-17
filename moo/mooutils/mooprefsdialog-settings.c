/*
 *   mooutils/mooprefsdialog-settings.c
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
#define __INSIDE_MOO_PREFS_DIALOG_SETTINGS_C__
#include "mooutils/mooprefsdialog-settings.h"
#include "mooutils/mooprefs.h"
#include "mooutils/mooutils-gobject.h"
#include <string.h>


void _moo_prefs_dialog_page_init_sig      (MooPrefsDialogPage *page)
{
    settings_init_sig (get_settings (page));
}


void _moo_prefs_dialog_page_apply         (MooPrefsDialogPage *page)
{
    settings_apply_sig (get_settings (page));
}


void        moo_prefs_dialog_page_bind_setting  (MooPrefsDialogPage *page,
                                                 GtkWidget          *widget,
                                                 const char         *setting_name,
                                                 GtkToggleButton    *set_or_not)
{
    g_return_if_fail (MOO_IS_PREFS_DIALOG_PAGE (page));
    g_return_if_fail (widget != NULL && setting_name != NULL);
    settings_bind (get_settings (page), widget, setting_name, set_or_not);
}


void        moo_prefs_dialog_page_bind_radio_setting
                                                (MooPrefsDialogPage *page,
                                                 const char         *setting_name,
                                                 GtkToggleButton   **btns,
                                                 const char        **cvals)
{
    g_return_if_fail (MOO_IS_PREFS_DIALOG_PAGE (page));
    g_return_if_fail (setting_name != NULL);
    settings_bind_radio (get_settings (page), setting_name, btns, cvals);
}


void        moo_prefs_dialog_page_bind_radio    (MooPrefsDialogPage *page,
                                                 const char         *setting_name,
                                                 GtkToggleButton    *btn,
                                                 const char         *cval)
{
    g_return_if_fail (MOO_IS_PREFS_DIALOG_PAGE (page));
    g_return_if_fail (setting_name != NULL && btn != NULL && cval != NULL);
    settings_add_radio  (get_settings (page), setting_name, btn, cval);
}



/**************************************************************************/
/* Aux stuff
 */
static void     string_list_free (GSList *list)
{
    g_slist_foreach (list, (GFunc) g_free, NULL);
    g_slist_free (list);
}



/**************************************************************************/
/* Settings
 */

static GQuark settings_quark (void)
{
    static GQuark q = 0;
    if (!q) q = g_quark_from_static_string ("moo_prefs_dialog_settings");
    return q;
}


static Settings    *get_settings        (MooPrefsDialogPage *page)
{
    Settings *s = g_object_get_qdata (G_OBJECT (page), MOO_PREFS_DIALOG_SETTINGS_QUARK);
    if (!s) {
        s = settings_new ();
        g_object_set_qdata_full (G_OBJECT (page),
                                 MOO_PREFS_DIALOG_SETTINGS_QUARK,
                                 s,
                                 (GDestroyNotify) settings_free);
        g_signal_connect_swapped (page, "init",
                                  G_CALLBACK (settings_init_sig), s);
        g_signal_connect_swapped (page, "apply",
                                  G_CALLBACK (settings_apply_sig), s);
    }

    return s;
}


static Settings    *settings_new        (void)
{
    return g_new0 (Settings, 1);
}


static void         settings_free       (Settings   *settings)
{
    g_return_if_fail (settings != NULL);

    g_slist_foreach (settings->settings, (GFunc) setting_free, NULL);
    g_slist_free (settings->settings);
    g_slist_free (settings->radio_settings);
}


static void         settings_bind       (Settings   *settings,
                                         GtkWidget  *widget,
                                         const char *setting_name,
                                         GtkToggleButton *set_or_not)
{
    Setting *s = NULL;

    if (GTK_IS_SPIN_BUTTON (widget))
        s = double_setting_new (GTK_SPIN_BUTTON (widget), setting_name, set_or_not);
    else if (GTK_IS_ENTRY (widget))
        s = string_setting_new (GTK_ENTRY (widget), setting_name, set_or_not);
    else if (GTK_IS_FONT_BUTTON (widget))
        s = font_setting_new (GTK_FONT_BUTTON (widget), setting_name, set_or_not);
    else if (GTK_IS_COLOR_BUTTON (widget))
        s = color_setting_new (GTK_COLOR_BUTTON (widget), setting_name, set_or_not);
    else if (GTK_IS_TOGGLE_BUTTON (widget))
        s = bool_setting_new (GTK_TOGGLE_BUTTON (widget), setting_name, set_or_not);
    else {
        g_critical ("%s: unsupported widget type %s", G_STRLOC,
                    g_type_name (G_TYPE_FROM_INSTANCE(widget)));
        return;
    }

    settings->settings = g_slist_prepend (settings->settings, s);
    s->set_or_not = set_or_not;
}

static void         settings_init_sig   (Settings   *settings)
{
    g_slist_foreach (settings->settings, (GFunc) setting_init_sig, NULL);
}


static void         settings_apply_sig  (Settings   *settings)
{
    g_slist_foreach (settings->settings, (GFunc) setting_apply_sig, NULL);
}


static int check_setting_name (Setting *s, const char *setting_name)
{
    return strcmp (s->setting_name, setting_name);
}

static void         settings_bind_radio (Settings          *settings,
                                         const char        *setting_name,
                                         GtkToggleButton  **btns,
                                         const char       **cvals)
{
    GSList *l;
    RadioSetting *s;
    GtkToggleButton **btn;
    const char **val;

    l = g_slist_find_custom (settings->radio_settings,
                             setting_name,
                             (GCompareFunc) check_setting_name);
    if (!l) {
        s = RADIO_SETTING (radio_setting_new (setting_name, NULL));
        settings->settings = g_slist_prepend (settings->settings, s);
        settings->radio_settings = g_slist_prepend (settings->radio_settings, s);
    }
    else
        s = l->data;

    /* TODO: check arguments */
    for (btn = btns; *btn != NULL; ++btn)
        s->buttons = g_slist_append (s->buttons, *btn);
    for (val = cvals; *val != NULL; ++val)
        s->values = g_slist_append (s->values, g_strdup (*val));
}


static void         settings_add_radio  (Settings          *settings,
                                         const char        *setting_name,
                                         GtkToggleButton   *btn,
                                         const char        *sval)
{
    GSList *l;
    RadioSetting *s;

    l = g_slist_find_custom (settings->radio_settings,
                             setting_name,
                             (GCompareFunc) check_setting_name);
    if (!l) {
        s = RADIO_SETTING (radio_setting_new (setting_name, NULL));
        settings->settings = g_slist_prepend (settings->settings, s);
        settings->radio_settings = g_slist_prepend (settings->radio_settings, s);
    }
    else
        s = l->data;

    /* TODO: check arguments */
    s->buttons = g_slist_append (s->buttons, btn);
    s->values = g_slist_append (s->values, g_strdup (sval));
}



/**************************************************************************/
/* Setting
 */
static Setting        *setting_new                 (SettingType     typ,
                                                    GtkWidget      *widget,
                                                    const char     *setting_name,
                                                    GtkToggleButton *set_or_not)
{
    gsize size = sizeof (Setting);
    Setting *s;

    g_return_val_if_fail (setting_name != NULL, NULL);

    switch (typ) {
        case SETTING_STRING:
            size = sizeof (StringSetting);
            break;
        case SETTING_FONT:
            size = sizeof (FontSetting);
            break;
        case SETTING_COLOR:
            size = sizeof (ColorSetting);
            break;
        case SETTING_BOOL:
            size = sizeof (BoolSetting);
            break;
        case SETTING_DOUBLE:
            size = sizeof (DoubleSetting);
            break;
        case SETTING_RADIO:
            size = sizeof (RadioSetting);
            break;
        default:
            g_assert_not_reached ();
    }

    s = (Setting*) g_malloc0 (size);
    s->type = typ;
    s->setting_name = g_strdup (setting_name);
    s->widget = widget;
    s->set_or_not = set_or_not;

    if (widget && set_or_not)
        moo_bind_sensitive (set_or_not, &widget, 1, FALSE);

    return s;
}


static void            setting_free                (Setting        *s)
{
    g_return_if_fail (s != NULL);

    switch (s->type) {
        case SETTING_STRING:
            string_setting_free (STRING_SETTING (s));
            break;
        case SETTING_FONT:
            font_setting_free (FONT_SETTING (s));
            break;
        case SETTING_COLOR:
            color_setting_free (COLOR_SETTING (s));
            break;
        case SETTING_BOOL:
            bool_setting_free (BOOL_SETTING (s));
            break;
        case SETTING_DOUBLE:
            double_setting_free (DOUBLE_SETTING (s));
            break;
        case SETTING_RADIO:
            radio_setting_free (RADIO_SETTING (s));
            break;
        default:
            g_assert_not_reached ();
    }

    g_free (s->setting_name);
    g_free (s);
}


static void            setting_init_sig            (Setting        *s)
{
    g_return_if_fail (s != NULL);

    if (s->set_or_not) {
        gboolean set = moo_prefs_get (SETTING_NAME (s)) ? TRUE : FALSE;
        gtk_toggle_button_set_active (s->set_or_not, set);
        if (!set) return;
    }

    switch (s->type) {
        case SETTING_STRING:
            string_setting_init_sig (STRING_SETTING (s));
            break;
        case SETTING_FONT:
            font_setting_init_sig (FONT_SETTING (s));
            break;
        case SETTING_COLOR:
            color_setting_init_sig (COLOR_SETTING (s));
            break;
        case SETTING_BOOL:
            bool_setting_init_sig (BOOL_SETTING (s));
            break;
        case SETTING_DOUBLE:
            double_setting_init_sig (DOUBLE_SETTING (s));
            break;
        case SETTING_RADIO:
            radio_setting_init_sig (RADIO_SETTING (s));
            break;
        default:
            g_assert_not_reached ();
    }
}

static void            setting_apply_sig           (Setting        *s)
{
    g_return_if_fail (s != NULL);

    if (s->set_or_not &&
        GTK_WIDGET_SENSITIVE (s->set_or_not) &&
        !gtk_toggle_button_get_active (s->set_or_not))
    {
        moo_prefs_set (SETTING_NAME (s), NULL);
        return;
    }

    if (s->widget && !GTK_WIDGET_SENSITIVE (s->widget))
        return;

    switch (s->type) {
        case SETTING_STRING:
            string_setting_apply_sig (STRING_SETTING (s));
            break;
        case SETTING_FONT:
            font_setting_apply_sig (FONT_SETTING (s));
            break;
        case SETTING_COLOR:
            color_setting_apply_sig (COLOR_SETTING (s));
            break;
        case SETTING_BOOL:
            bool_setting_apply_sig (BOOL_SETTING (s));
            break;
        case SETTING_DOUBLE:
            double_setting_apply_sig (DOUBLE_SETTING (s));
            break;
        case SETTING_RADIO:
            radio_setting_apply_sig (RADIO_SETTING (s));
            break;
        default:
            g_assert_not_reached ();
    }
}



/**************************************************************************/
/* StringSetting
 */

static Setting        *string_setting_new          (GtkEntry       *entry,
                                                    const char     *setting_name,
                                                    GtkToggleButton *set_or_not)
{
    StringSetting *s = STRING_SETTING (setting_new (SETTING_STRING,
                                                    GTK_WIDGET (entry),
                                                    setting_name,
                                                    set_or_not));

    s->empty_is_null = FALSE;
    return SETTING (s);
}


static void            string_setting_free         (G_GNUC_UNUSED StringSetting  *s)
{
}


static void            string_setting_init_sig     (StringSetting  *s)
{
    const char *val = moo_prefs_get_string (SETTING_NAME (s));
    if (!val) {
        val = "";
        if (!s->empty_is_null)
            g_critical ("%s: '%s' not set", G_STRLOC, SETTING_NAME (s));
    }
    gtk_entry_set_text (GTK_ENTRY (SETTING (s)->widget), val);
}


static void            string_setting_apply_sig    (StringSetting  *s)
{
    const char *old_val = moo_prefs_get_string (SETTING_NAME (s));
    const char *new_val = gtk_entry_get_text (GTK_ENTRY (SETTING (s)->widget));

    if ((old_val && strcmp (old_val, new_val)) ||
        (!old_val && (!s->empty_is_null || new_val[0])))
            moo_prefs_set_string (SETTING_NAME (s), new_val);
}



/**************************************************************************/
/* FontSetting
 */

static Setting        *font_setting_new            (GtkFontButton  *btn,
                                                    const char     *setting_name,
                                                    GtkToggleButton *set_or_not)
{
    return setting_new (SETTING_FONT,
                        GTK_WIDGET (btn),
                        setting_name,
                        set_or_not);
}

static void            font_setting_free           (G_GNUC_UNUSED FontSetting    *s)
{
}


static void            font_setting_init_sig       (FontSetting    *s)
{
    const char *val = moo_prefs_get_string (SETTING_NAME (s));
    if (!val)
        g_critical ("%s: '%s' not set", G_STRLOC, SETTING_NAME (s));
    else
        gtk_font_button_set_font_name (GTK_FONT_BUTTON (SETTING (s)->widget), val);
}


static void            font_setting_apply_sig      (FontSetting    *s)
{
    const char *old_val = moo_prefs_get_string (SETTING_NAME (s));
    const char *new_val = gtk_font_button_get_font_name (GTK_FONT_BUTTON (SETTING (s)->widget));

    if ((old_val && new_val && strcmp (old_val, new_val)) ||
        ((!old_val || !new_val) && old_val != new_val))
            moo_prefs_set_string (SETTING_NAME (s), new_val);
}



/**************************************************************************/
/* ColorSetting
 */

static Setting        *color_setting_new           (GtkColorButton *btn,
                                                    const char     *setting_name,
                                                    GtkToggleButton *set_or_not)
{
    return setting_new (SETTING_COLOR,
                        GTK_WIDGET (btn),
                        setting_name,
                        set_or_not);
}

static void             color_setting_free          (G_GNUC_UNUSED ColorSetting   *s)
{
}


static void             color_setting_init_sig      (ColorSetting   *s)
{
    const GdkColor *color = moo_prefs_get_color (SETTING_NAME (s));
    if (!color) {
        s->color.red = s->color.green = s->color.blue = 0;
        g_critical ("%s: '%s' not set", G_STRLOC, SETTING_NAME (s));
    }
    else {
        s->color = *color;
        gtk_color_button_set_color (GTK_COLOR_BUTTON (SETTING (s)->widget), color);
    }
}


static void             color_setting_apply_sig     (ColorSetting   *s)
{
    GdkColor color;
    gtk_color_button_get_color (GTK_COLOR_BUTTON (SETTING (s)->widget), &color);
    if (color.red != s->color.red ||
        color.green != s->color.green ||
        color.blue != s->color.blue)
    {
        s->color = color;
        moo_prefs_set_color (SETTING_NAME (s), &color);
    }
}



/**************************************************************************/
/* BoolSetting
 */

static Setting        *bool_setting_new            (GtkToggleButton *btn,
                                                    const char      *setting_name,
                                                    GtkToggleButton *set_or_not)
{
    return setting_new (SETTING_BOOL,
                        GTK_WIDGET (btn),
                        setting_name,
                        set_or_not);
}

static void             bool_setting_free           (G_GNUC_UNUSED BoolSetting    *s)
{
}


static void             bool_setting_init_sig       (BoolSetting    *s)
{
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (SETTING (s)->widget),
                                  moo_prefs_get_bool (SETTING_NAME (s)));
}


static void             bool_setting_apply_sig      (BoolSetting    *s)
{
    gboolean val = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (SETTING (s)->widget));
    if (val != moo_prefs_get_bool (SETTING_NAME (s)))
        moo_prefs_set_bool (SETTING_NAME (s), val);
}



/**************************************************************************/
/* DoubleSetting
 */

static Setting        *double_setting_new          (GtkSpinButton  *spin,
                                                    const char     *setting_name,
                                                    GtkToggleButton *set_or_not)
{
    return setting_new (SETTING_DOUBLE,
                        GTK_WIDGET (spin),
                        setting_name,
                        set_or_not);
}

static void             double_setting_free         (G_GNUC_UNUSED DoubleSetting  *s)
{
}


static void             double_setting_init_sig     (DoubleSetting  *s)
{
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (SETTING (s)->widget),
                               moo_prefs_get_number (SETTING_NAME (s)));
}


static void             double_setting_apply_sig    (DoubleSetting  *s)
{
    double val = gtk_spin_button_get_value (GTK_SPIN_BUTTON (SETTING (s)->widget));
    if (val != moo_prefs_get_number (SETTING_NAME (s)))
        moo_prefs_set_number (SETTING_NAME (s), val);
}



/**************************************************************************/
/* RadioSetting
 */

static Setting        *radio_setting_new           (const char     *setting_name,
                                                    GtkToggleButton *set_or_not)
{
    RadioSetting *s = RADIO_SETTING (setting_new (SETTING_RADIO,
                                                  NULL,
                                                  setting_name,
                                                  set_or_not));

    s->value = -1;
    s->buttons = NULL;
    s->values = NULL;

    return SETTING (s);
}


static void             radio_setting_free          (RadioSetting   *s)
{
    string_list_free (s->values);
}


static void             radio_setting_init_sig      (RadioSetting   *s)
{
    int pos = -1;
    const char *val = moo_prefs_get_string (SETTING_NAME(s));
    GtkToggleButton *btn = NULL;

    if (!val) {
        g_critical ("%s: '%s' not set", G_STRLOC, SETTING_NAME (s));
    }
    else {
        GSList *l = g_slist_find_custom (s->values, val, (GCompareFunc) strcmp);
        if (!l) {
            g_critical ("%s: can't find '%s' in the list of values", G_STRLOC, val);
        }
        else
            pos = g_slist_position (s->values, l);
    }

    if (pos >= 0) {
        btn = g_slist_nth_data (s->buttons, (guint) pos);
        g_assert (btn != NULL);
        gtk_toggle_button_set_active (btn, TRUE);
    }
}


static void             radio_setting_apply_sig     (RadioSetting   *s)
{
    int pos = -1;
    GSList *l = NULL;
    const char *old_val;
    const char *new_val;

    for (l = s->buttons; l != NULL; l = l->next)
        if (gtk_toggle_button_get_active (l->data))
            break;
    g_return_if_fail (l != NULL);

    pos = g_slist_position (s->buttons, l);
    g_return_if_fail (pos > 0);
    new_val = g_slist_nth_data (s->values, (guint) pos);
    g_return_if_fail (new_val != NULL);

    old_val = moo_prefs_get_string (SETTING_NAME (s));
    if (!old_val || strcmp (old_val, new_val))
        moo_prefs_set_string (SETTING_NAME (s), new_val);
}
