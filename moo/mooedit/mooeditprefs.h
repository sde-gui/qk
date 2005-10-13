/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   mooeditprefs.h
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

#ifndef __MOO_EDIT_PREFS_H__
#define __MOO_EDIT_PREFS_H__

#include <mooedit/mooeditor.h>

G_BEGIN_DECLS


#define MOO_EDIT_PREFS_PREFIX "Editor"

GtkWidget  *moo_edit_prefs_page_new         (MooEditor      *editor);
GtkWidget  *moo_edit_colors_prefs_page_new  (MooEditor      *editor);

/* defined in mooeditprefs.c */
const char *moo_edit_setting                (const char     *setting_name);


/* keep in sync with list in mooeditprefspage.c */

#define MOO_EDIT_PREFS_SEARCH_SELECTED              "search/search_selected"

#define MOO_EDIT_PREFS_ON_EXTERNAL_CHANGES          "on_external_changes"
#define MOO_EDIT_PREFS_AUTO_SAVE                    "auto_save"
#define MOO_EDIT_PREFS_AUTO_SAVE_INTERVAL           "auto_save_interval"
#define MOO_EDIT_PREFS_SMART_HOME_END               "smart_home_end"
#define MOO_EDIT_PREFS_TABS_WIDTH                   "tabs_width"
#define MOO_EDIT_PREFS_AUTO_INDENT                  "auto_indent"
#define MOO_EDIT_PREFS_TAB_INDENTS                  "tab_indents"
#define MOO_EDIT_PREFS_BACKSPACE_INDENTS            "backspace_indents"
#define MOO_EDIT_PREFS_SPACES_NO_TABS               "spaces_instead_of_tabs"
#define MOO_EDIT_PREFS_LIMIT_UNDO                   "limit_undo"
#define MOO_EDIT_PREFS_LIMIT_UNDO_NUM               "limit_undo_num"
#define MOO_EDIT_PREFS_WRAP_ENABLE                  "wrapping_enable"
#define MOO_EDIT_PREFS_WRAP_DONT_SPLIT_WORDS        "wrapping_dont_split_words"
#define MOO_EDIT_PREFS_SHOW_LINE_NUMBERS            "show_line_numbers"
#define MOO_EDIT_PREFS_SHOW_MARGIN                  "show_right_margin"
#define MOO_EDIT_PREFS_MARGIN                       "right_margin"
#define MOO_EDIT_PREFS_USE_DEFAULT_FONT             "use_default_font"
#define MOO_EDIT_PREFS_FONT                         "font"
#define MOO_EDIT_PREFS_HIGHLIGHT_CURRENT_LINE       "highlight_current_line"
#define MOO_EDIT_PREFS_USE_SYNTAX_HIGHLIGHTING      "use_syntax_highlighting"

#define MOO_EDIT_STYLE_ALERT                        "Alert"
#define MOO_EDIT_STYLE_BASE_N                       "BaseN"
#define MOO_EDIT_STYLE_CHAR                         "Char"
#define MOO_EDIT_STYLE_COMMENT                      "Comment"
#define MOO_EDIT_STYLE_DATA_TYPE                    "Data Type"
#define MOO_EDIT_STYLE_DECIMAL                      "Decimal"
#define MOO_EDIT_STYLE_ERROR                        "Error"
#define MOO_EDIT_STYLE_FLOAT                        "Float"
#define MOO_EDIT_STYLE_FUNCTION                     "Function"
#define MOO_EDIT_STYLE_KEYWORD                      "Keyword"
#define MOO_EDIT_STYLE_NORMAL                       "Normal"
#define MOO_EDIT_STYLE_OTHERS                       "Others"
#define MOO_EDIT_STYLE_STRING                       "String"

#define MOO_EDIT_PREFS_BOLD                         "bold"
#define MOO_EDIT_PREFS_ITALIC                       "italic"
#define MOO_EDIT_PREFS_UNDERLINE                    "underline"
#define MOO_EDIT_PREFS_STRIKETHROUGH                "strikethrough"

#define MOO_EDIT_PREFS_DIALOGS_SAVE                 "dialogs/save"
#define MOO_EDIT_PREFS_DIALOGS_OPEN                 "dialogs/open"


G_END_DECLS

#endif /* __MOO_EDIT_PREFS_H__ */
