/*
 *   mooterm/mooterm-prefs.h
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

#ifndef MOOTERM_MOOTREMPREFS_H
#define MOOTERM_MOOTREMPREFS_H

#include <gtk/gtkwidget.h>

G_BEGIN_DECLS


GtkWidget      *moo_term_prefs_page_new     (void);
void            moo_term_set_prefs_prefix   (const char  *prefix);
const char     *moo_term_get_prefs_prefix   (void);
const char     *moo_term_setting            (const char  *setting_name);


#define MOO_TERM_PREFS_FONT                 "font"
#define MOO_TERM_PREFS_FOREGROUND           "foreground"
#define MOO_TERM_PREFS_BACKGROUND           "background"
#define MOO_TERM_PREFS_CURSOR_BLINKS        "cursor_blinks"
#define MOO_TERM_PREFS_CURSOR_BLINK_TIME    "cursor_blink_time"

#define MOO_TERM_PREFS_SAVE_SELECTION_DIR   "save_selection_dir"


G_END_DECLS

#endif /* MOOTERM_MOOTREMPREFS_H */
