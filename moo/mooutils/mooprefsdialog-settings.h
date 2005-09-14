/*
 *   mooutils/mooprefsdialog-settings.h
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

#ifndef MOOPREFS_COMPILATION
#error "Do not include this file"
#endif

#ifndef __MOO_PREFS_DIALOG_SETTTING_H__
#define __MOO_PREFS_DIALOG_SETTTING_H__

#include "mooutils/mooprefsdialogpage.h"
#include "mooutils/moocompat.h"
#include <gtk/gtk.h>

G_BEGIN_DECLS


void _moo_prefs_dialog_page_init_sig      (MooPrefsDialogPage *page);
void _moo_prefs_dialog_page_apply         (MooPrefsDialogPage *page);


#ifdef __INSIDE_MOO_PREFS_DIALOG_SETTINGS_C__


#define MOO_PREFS_DIALOG_SETTINGS_QUARK (settings_quark ())
static GQuark settings_quark (void);


typedef struct {
    GSList *settings;       /* Setting* */
    GSList *radio_settings; /* RadioSetting* */
} Settings;

static Settings    *get_settings        (MooPrefsDialogPage *page);

static Settings    *settings_new        (void);
static void         settings_free       (Settings   *settings);

static void         settings_bind       (Settings   *settings,
                                         GtkWidget  *widget,
                                         const char *setting_name,
                                         GtkToggleButton *set_or_not);
static void         settings_init_sig   (Settings   *settings);
static void         settings_apply_sig  (Settings   *settings);
static void         settings_bind_radio (Settings          *settings,
                                         const char        *setting,
                                         GtkToggleButton  **btns,
                                         const char       **cvals);
static void         settings_add_radio  (Settings          *settings,
                                         const char        *setting,
                                         GtkToggleButton   *btn,
                                         const char        *sval);


#define SETTING(s)          ((Setting*)(s))
#define STRING_SETTING(s)   ((s)->type == SETTING_STRING ? (StringSetting*)(s) : NULL)
#define FONT_SETTING(s)     ((s)->type == SETTING_FONT ? (FontSetting*)(s) : NULL)
#define COLOR_SETTING(s)    ((s)->type == SETTING_COLOR ? (ColorSetting*)(s) : NULL)
#define BOOL_SETTING(s)     ((s)->type == SETTING_BOOL ? (BoolSetting*)(s) : NULL)
#define DOUBLE_SETTING(s)   ((s)->type == SETTING_DOUBLE ? (DoubleSetting*)(s) : NULL)
#define RADIO_SETTING(s)    ((s)->type == SETTING_RADIO ? (RadioSetting*)(s) : NULL)

#define SETTING_NAME(s)     (SETTING (s)->setting_name)


typedef enum {
    SETTING_STRING,
    SETTING_FONT,
    SETTING_COLOR,
    SETTING_BOOL,
    SETTING_DOUBLE,
    SETTING_RADIO
} SettingType;


typedef struct {
    SettingType      type;
    char            *setting_name;
    GtkWidget       *widget;
    GtkToggleButton *set_or_not;
} Setting;

static Setting        *setting_new                 (SettingType     type,
                                                    GtkWidget      *widget,
                                                    const char     *setting_name,
                                                    GtkToggleButton *set_or_not);
static void            setting_free                (Setting        *s);
static void            setting_init_sig            (Setting        *s);
static void            setting_apply_sig           (Setting        *s);


typedef struct {
    Setting  setting;
    guint    empty_is_null : 1;
} StringSetting;

static Setting        *string_setting_new          (GtkEntry       *entry,
                                                    const char     *setting_name,
                                                    GtkToggleButton *set_or_not);
static void            string_setting_free         (StringSetting  *s);
static void            string_setting_init_sig     (StringSetting  *s);
static void            string_setting_apply_sig    (StringSetting  *s);


typedef struct {
    Setting  setting;
} FontSetting;

static Setting        *font_setting_new            (GtkFontButton  *btn,
                                                    const char     *setting_name,
                                                    GtkToggleButton *set_or_not);
static void            font_setting_free           (FontSetting    *s);
static void            font_setting_init_sig       (FontSetting    *s);
static void            font_setting_apply_sig      (FontSetting    *s);


typedef struct {
    Setting  setting;
    GdkColor color;
} ColorSetting;

static Setting        *color_setting_new            (GtkColorButton *btn,
                                                     const char     *setting_name,
                                                     GtkToggleButton *set_or_not);
static void            color_setting_free           (ColorSetting   *s);
static void            color_setting_init_sig       (ColorSetting   *s);
static void            color_setting_apply_sig      (ColorSetting   *s);


typedef struct {
    Setting     setting;
} BoolSetting;

static Setting        *bool_setting_new             (GtkToggleButton *btn,
                                                     const char     *setting_name,
                                                     GtkToggleButton *set_or_not);
static void            bool_setting_free            (BoolSetting    *s);
static void            bool_setting_init_sig        (BoolSetting    *s);
static void            bool_setting_apply_sig       (BoolSetting    *s);


typedef struct {
    Setting     setting;
} DoubleSetting;

static Setting        *double_setting_new           (GtkSpinButton  *spin,
                                                     const char     *setting_name,
                                                     GtkToggleButton *set_or_not);
static void            double_setting_free          (DoubleSetting  *s);
static void            double_setting_init_sig      (DoubleSetting  *s);
static void            double_setting_apply_sig     (DoubleSetting  *s);


typedef struct {
    Setting     setting;
    int         value;
    GSList     *buttons;    /* GtkToggleButton* */
    GSList     *values;     /* char* */
} RadioSetting;

static Setting        *radio_setting_new            (const char     *setting_name,
                                                     GtkToggleButton *set_or_not);
static void            radio_setting_free           (RadioSetting   *s);
static void            radio_setting_init_sig       (RadioSetting   *s);
static void            radio_setting_apply_sig      (RadioSetting   *s);


#endif /* __INSIDE_MOO_PREFS_DIALOG_SETTINGS_C__ */


G_END_DECLS

#endif /* __MOO_PREFS_DIALOG_SETTTING_H__ */
