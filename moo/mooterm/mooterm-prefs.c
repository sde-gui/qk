/*
 *   mooterm/mooterm-prefs.c
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

#ifdef __WIN32__
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "mooterm/mooterm-private.h"
#include "mooterm/mooterm-prefs.h"
#include "mooterm/mootermprefs-glade.h"
#include "mooterm/mooterm.h"
#include "mooutils/mooprefs.h"
#include "mooutils/mooprefsdialog.h"
#include "mooutils/moostock.h"
#include "mooutils/mooglade.h"
#include <string.h>


#define MOO_TERM_PREFS_PREFIX "Terminal"

#define NEW_KEY_BOOL(s,v)   moo_prefs_new_key_bool (MOO_TERM_PREFS_PREFIX "/" s, v)
#define NEW_KEY_INT(s,v)    moo_prefs_new_key_int (MOO_TERM_PREFS_PREFIX "/" s, v)
#define NEW_KEY_STRING(s,v) moo_prefs_new_key_string (MOO_TERM_PREFS_PREFIX "/" s, v)
#define NEW_KEY_COLOR(s,v)  moo_prefs_new_key_color (MOO_TERM_PREFS_PREFIX "/" s, v)

#define GET_STRING(s)   moo_prefs_get_string (MOO_TERM_PREFS_PREFIX "/" s)
#define GET_INT(s)      moo_prefs_get_int (MOO_TERM_PREFS_PREFIX "/" s)
#define GET_BOOL(s)     moo_prefs_get_bool (MOO_TERM_PREFS_PREFIX "/" s)
#define GET_COLOR(s)    moo_prefs_get_color (MOO_TERM_PREFS_PREFIX "/" s)


#ifdef __WIN32__
#define DEFAULT_BLINK_TIME ((int)GetCaretBlinkTime())
#else /* ! __WIN32__ */
#define DEFAULT_BLINK_TIME ((int)530)
#endif /* ! __WIN32__ */


static void set_defaults (void)
{
    GtkSettings *settings;
    GtkStyle *style;

    NEW_KEY_STRING (MOO_TERM_PREFS_FONT, DEFAULT_MONOSPACE_FONT);
    NEW_KEY_BOOL (MOO_TERM_PREFS_CURSOR_BLINKS, FALSE);
    NEW_KEY_INT (MOO_TERM_PREFS_CURSOR_BLINK_TIME, DEFAULT_BLINK_TIME);

    settings = gtk_settings_get_default ();
    style = gtk_rc_get_style_by_paths (settings, "MooTerm", "MooTerm", MOO_TYPE_TERM);

    if (!style)
        style = gtk_style_new ();
    else
        g_object_ref (G_OBJECT (style));

    g_return_if_fail (style != NULL);

    NEW_KEY_COLOR (MOO_TERM_PREFS_FOREGROUND, &(style->text[GTK_STATE_NORMAL]));
    NEW_KEY_COLOR (MOO_TERM_PREFS_BACKGROUND, &(style->base[GTK_STATE_NORMAL]));

    g_object_unref (G_OBJECT (style));
}


void moo_term_apply_settings (MooTerm *term)
{
    set_defaults ();

    moo_term_set_font_from_string (term, GET_STRING (MOO_TERM_PREFS_FONT));

    if (GET_BOOL (MOO_TERM_PREFS_CURSOR_BLINKS))
    {
        int t = GET_INT (MOO_TERM_PREFS_CURSOR_BLINK_TIME);
        if (t <= 0)
        {
            moo_prefs_set_bool (moo_term_setting (MOO_TERM_PREFS_CURSOR_BLINKS),
                                FALSE);
            moo_prefs_set_int (moo_term_setting (MOO_TERM_PREFS_CURSOR_BLINK_TIME),
                               DEFAULT_BLINK_TIME);
            moo_term_set_cursor_blink_time (term, 0);
        }
        else
        {
            moo_term_set_cursor_blink_time (term, (guint)t);
        }
    }
    else
    {
        moo_term_set_cursor_blink_time (term, 0);
    }
}


static gboolean
connect_prefs (G_GNUC_UNUSED MooGladeXML    *xml,
               G_GNUC_UNUSED const char     *widget_id,
               G_GNUC_UNUSED GtkWidget      *widget,
               const char     *signal,
               G_GNUC_UNUSED const char     *handler,
               G_GNUC_UNUSED const char     *object,
               G_GNUC_UNUSED gpointer        data)
{
    g_warning ("%s: not bound signal '%s'", G_STRLOC, signal);
    return TRUE;
}


static char*
map_prefs (G_GNUC_UNUSED MooGladeXML *xml,
           const char     *key,
           G_GNUC_UNUSED gpointer data)
{
    static GHashTable *map = NULL;
    const char *real_key;

    if (!map)
    {
        map = g_hash_table_new_full (g_str_hash, g_str_equal,
                                     g_free, g_free);

        g_hash_table_insert (map, g_strdup ("MOO_TERM_PREFS_FONT"),
                             g_strdup (MOO_TERM_PREFS_FONT));
        g_hash_table_insert (map, g_strdup ("MOO_TERM_PREFS_FOREGROUND"),
                             g_strdup (MOO_TERM_PREFS_FOREGROUND));
        g_hash_table_insert (map, g_strdup ("MOO_TERM_PREFS_BACKGROUND"),
                             g_strdup (MOO_TERM_PREFS_BACKGROUND));
        g_hash_table_insert (map, g_strdup ("MOO_TERM_PREFS_CURSOR_BLINKS"),
                             g_strdup (MOO_TERM_PREFS_CURSOR_BLINKS));
        g_hash_table_insert (map, g_strdup ("MOO_TERM_PREFS_CURSOR_BLINK_TIME"),
                             g_strdup (MOO_TERM_PREFS_CURSOR_BLINK_TIME));
        g_hash_table_insert (map, g_strdup ("MOO_TERM_PREFS_SAVE_SELECTION_DIR"),
                             g_strdup (MOO_TERM_PREFS_SAVE_SELECTION_DIR));
    }

    real_key = g_hash_table_lookup (map, key);

    if (!real_key)
    {
        g_warning ("%s: uknown key '%s'", G_STRLOC, key);
        real_key = key;
    }

    return moo_prefs_make_key (MOO_TERM_PREFS_PREFIX, real_key, NULL);
}


GtkWidget  *moo_term_prefs_page_new   (void)
{
    MooPrefsDialogPage *page;
    MooGladeXML *xml;

    xml = moo_glade_xml_new_empty ();
    moo_glade_xml_map_id (xml, "page", MOO_TYPE_PREFS_DIALOG_PAGE);
    moo_glade_xml_map_signal (xml, connect_prefs, NULL);
    moo_glade_xml_set_prefs (xml, "page", NULL);
    moo_glade_xml_set_prefs_map (xml, map_prefs, NULL);

    moo_glade_xml_parse_memory (xml, MOO_TERM_PREFS_GLADE_UI,
                                -1, "page");

    page = moo_glade_xml_get_widget (xml, "page");
    g_object_set_data_full (G_OBJECT (page), "moo-glade-xml", xml,
                            (GDestroyNotify) moo_glade_xml_unref);
    g_object_set (page, "label", "Terminal",
                  "icon-stock-id", MOO_STOCK_TERMINAL, NULL);

    set_defaults ();

    return GTK_WIDGET (page);
}


#define STR_STACK_SIZE 4

const char     *moo_term_setting           (const char  *setting_name)
{
    static GString *stack[STR_STACK_SIZE];
    static guint p;

    g_return_val_if_fail (setting_name != NULL, NULL);

    if (!stack[0])
    {
        for (p = 0; p < STR_STACK_SIZE; ++p)
            stack[p] = g_string_new ("");
        p = STR_STACK_SIZE - 1;
    }

    if (p == STR_STACK_SIZE - 1)
        p = 0;
    else
        p++;

    g_string_printf (stack[p], MOO_TERM_PREFS_PREFIX "/%s", setting_name);
    return stack[p]->str;
}
