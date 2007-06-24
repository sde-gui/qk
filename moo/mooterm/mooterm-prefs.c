/*
 *   mooterm-prefs.c
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
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
#include "mooutils/moofontsel.h"
#include "mooutils/mooglade.h"
#include "mooutils/mooi18n.h"
#include <string.h>


#define NEW_KEY_BOOL(s__,v__)   moo_prefs_new_key_bool (MOO_TERM_PREFS_PREFIX "/" s__, v__)
#define NEW_KEY_INT(s__,v__)    moo_prefs_new_key_int (MOO_TERM_PREFS_PREFIX "/" s__, v__)
#define NEW_KEY_STRING(s__,v__) moo_prefs_new_key_string (MOO_TERM_PREFS_PREFIX "/" s__, v__)
#define NEW_KEY_COLOR(s__,v__)  moo_prefs_new_key_color (MOO_TERM_PREFS_PREFIX "/" s__, v__)

#define GET_STRING(s__)   moo_prefs_get_string (MOO_TERM_PREFS_PREFIX "/" s__)
#define GET_INT(s__)      moo_prefs_get_int (MOO_TERM_PREFS_PREFIX "/" s__)
#define GET_BOOL(s__)     moo_prefs_get_bool (MOO_TERM_PREFS_PREFIX "/" s__)
#define GET_COLOR(s__)    moo_prefs_get_color (MOO_TERM_PREFS_PREFIX "/" s__)


#ifdef __WIN32__
#define DEFAULT_BLINK_TIME ((int)GetCaretBlinkTime())
#else /* ! __WIN32__ */
#define DEFAULT_BLINK_TIME ((int)530)
#endif /* ! __WIN32__ */


void
_moo_term_init_settings (void)
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


void
moo_term_apply_settings (MooTerm *term)
{
    g_return_if_fail (MOO_IS_TERM (term));

    moo_term_set_font_from_string (term, GET_STRING (MOO_TERM_PREFS_FONT));

    gtk_widget_modify_text (GTK_WIDGET (term), GTK_STATE_NORMAL,
                            GET_COLOR (MOO_TERM_PREFS_FOREGROUND));
    gtk_widget_modify_base (GTK_WIDGET (term), GTK_STATE_NORMAL,
                            GET_COLOR (MOO_TERM_PREFS_BACKGROUND));

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


GtkWidget  *moo_term_prefs_page_new   (void)
{
    MooPrefsDialogPage *page;
    MooGladeXML *xml;

    _moo_term_init_settings ();

    xml = moo_glade_xml_new_empty (GETTEXT_PACKAGE);
    moo_glade_xml_map_id (xml, "font", MOO_TYPE_FONT_BUTTON);
    moo_glade_xml_set_property (xml, "font", "monospace", "True");
    moo_glade_xml_set_property (xml, "font", "filter-visible", "False");

    page = moo_prefs_dialog_page_new_from_xml ("Terminal", MOO_STOCK_TERMINAL,
                                               xml, MOO_TERM_PREFS_GLADE_UI,
                                               "page", MOO_TERM_PREFS_PREFIX);

    g_object_unref (xml);
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
