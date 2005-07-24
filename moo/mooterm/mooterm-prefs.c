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
#endif /* __WIN32__ */

#include "mooterm/mooterm-private.h"
#include "mooterm/mooterm-prefs.h"
#include "mooterm/mooterm.h"
#include "mooutils/mooprefs.h"
#include "mooutils/mooprefsdialog.h"
#include "mooutils/moostock.h"
#include <string.h>


/* mootermprefs-glade.c */
GtkWidget  *_create_moo_term_prefs_notebook (MooPrefsDialogPage *page);


#define SET_DEFAULT(s, val) \
    moo_prefs_set_if_not_set_ignore_change (moo_term_setting (s), val)
#define SET_DEFAULT_DOUBLE(s, val) \
    moo_prefs_set_double_if_not_set_ignore_change (moo_term_setting (s), val)
#define SET_DEFAULT_BOOL(s, val) \
    moo_prefs_set_bool_if_not_set_ignore_change (moo_term_setting (s), val)
#define SET_DEFAULT_COLOR(s, val) \
    moo_prefs_set_color_if_not_set_ignore_change (moo_term_setting (s), val)

#define GET(s)          moo_prefs_get (moo_term_setting (s))
#define GET_INT(s)      moo_prefs_get_int (moo_term_setting (s))
#define GET_BOOL(s)     moo_prefs_get_bool (moo_term_setting (s))
#define GET_COLOR(s)    moo_prefs_get_color (moo_term_setting (s))


#ifdef __WIN32__
#define DEFAULT_BLINK_TIME ((double)GetCaretBlinkTime())
#else /* ! __WIN32__ */
#define DEFAULT_BLINK_TIME ((double)530)
#endif /* ! __WIN32__ */


static void set_defaults (void)
{
    GtkSettings *settings;
    GtkStyle *style;

    SET_DEFAULT (MOO_TERM_PREFS_FONT, DEFAULT_MONOSPACE_FONT);
    SET_DEFAULT_BOOL (MOO_TERM_PREFS_CURSOR_BLINKS, FALSE);
    SET_DEFAULT_DOUBLE (MOO_TERM_PREFS_CURSOR_BLINK_TIME, DEFAULT_BLINK_TIME);

    settings = gtk_settings_get_default ();
    style = gtk_rc_get_style_by_paths (settings, "MooTerm", "MooTerm", MOO_TYPE_TERM);

    if (!style)
        style = gtk_style_new ();
    else
        g_object_ref (G_OBJECT (style));

    g_return_if_fail (style != NULL);

    SET_DEFAULT_COLOR (MOO_TERM_PREFS_FOREGROUND, &(style->text[GTK_STATE_NORMAL]));
    SET_DEFAULT_COLOR (MOO_TERM_PREFS_BACKGROUND, &(style->base[GTK_STATE_NORMAL]));

    g_object_unref (G_OBJECT (style));
}


void moo_term_apply_settings (MooTerm *term)
{
    set_defaults ();

    moo_term_set_font_from_string (term, GET (MOO_TERM_PREFS_FONT));

    if (GET_BOOL (MOO_TERM_PREFS_CURSOR_BLINKS))
    {
        int t = GET_INT (MOO_TERM_PREFS_CURSOR_BLINK_TIME);
        if (t <= 0)
        {
            moo_prefs_set_bool (moo_term_setting (MOO_TERM_PREFS_CURSOR_BLINKS), FALSE);
            moo_prefs_set_int (moo_term_setting (MOO_TERM_PREFS_CURSOR_BLINK_TIME), DEFAULT_BLINK_TIME);
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
    GtkWidget *page, *notebook;

    page = moo_prefs_dialog_page_new ("Terminal", MOO_STOCK_TERMINAL);
    notebook = _create_moo_term_prefs_notebook (MOO_PREFS_DIALOG_PAGE (page));
    gtk_box_pack_start (GTK_BOX (page), notebook, TRUE, TRUE, 0);

    set_defaults ();

    return page;
}


const char     *moo_term_setting           (const char  *setting_name)
{
    static char *s = NULL;

    g_return_val_if_fail (setting_name != NULL, NULL);

    g_free (s);
    s = g_strdup_printf ("terminal::%s", setting_name);
    return s;
}
