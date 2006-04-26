/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
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

#define MOOEDIT_COMPILATION
#include "mooedit/mooeditprefs.h"
#include "mooedit/mooedit-private.h"
#include "mooedit/mootextbuffer.h"
#include <string.h>


#define STR_STACK_SIZE 4

const char *moo_edit_setting                (const char     *setting_name)
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

    g_string_printf (stack[p], MOO_EDIT_PREFS_PREFIX "/%s", setting_name);
    return stack[p]->str;
}


#define NEW_KEY_BOOL(s,v)    moo_prefs_new_key_bool (MOO_EDIT_PREFS_PREFIX "/" s, v)
#define NEW_KEY_INT(s,v)     moo_prefs_new_key_int (MOO_EDIT_PREFS_PREFIX "/" s, v)
#define NEW_KEY_STRING(s,v)  moo_prefs_new_key_string (MOO_EDIT_PREFS_PREFIX "/" s, v)
#define NEW_KEY_COLOR(s,v)   moo_prefs_new_key_color (MOO_EDIT_PREFS_PREFIX "/" s, v)
#define NEW_KEY_ENUM(s,t,v)  moo_prefs_new_key_enum (MOO_EDIT_PREFS_PREFIX "/" s, t, v)
#define NEW_KEY_FLAGS(s,t,v) moo_prefs_new_key_flags (MOO_EDIT_PREFS_PREFIX "/" s, t, v)

void
_moo_edit_init_settings (void)
{
    static gboolean done = FALSE;

    if (done)
        return;
    else
        done = TRUE;

    NEW_KEY_BOOL (MOO_EDIT_PREFS_SPACES_NO_TABS, FALSE);
    NEW_KEY_INT (MOO_EDIT_PREFS_INDENT_WIDTH, 8);
    NEW_KEY_ENUM (MOO_EDIT_PREFS_TAB_KEY_ACTION,
                  MOO_TYPE_TEXT_TAB_KEY_ACTION, MOO_TEXT_TAB_KEY_INDENT);
    NEW_KEY_BOOL (MOO_EDIT_PREFS_AUTO_INDENT, TRUE);
    NEW_KEY_BOOL (MOO_EDIT_PREFS_BACKSPACE_INDENTS, FALSE);

    NEW_KEY_BOOL (MOO_EDIT_PREFS_AUTO_SAVE, FALSE);
    NEW_KEY_INT (MOO_EDIT_PREFS_AUTO_SAVE_INTERVAL, 5);
    NEW_KEY_BOOL (MOO_EDIT_PREFS_MAKE_BACKUPS, FALSE);
    NEW_KEY_BOOL (MOO_EDIT_PREFS_STRIP, FALSE);

    NEW_KEY_STRING (MOO_EDIT_PREFS_COLOR_SCHEME, NULL);

    NEW_KEY_BOOL (MOO_EDIT_PREFS_SMART_HOME_END, TRUE); /* XXX implement it? */
    NEW_KEY_BOOL (MOO_EDIT_PREFS_WRAP_ENABLE, FALSE);
    NEW_KEY_BOOL (MOO_EDIT_PREFS_WRAP_WORDS, TRUE);
    NEW_KEY_BOOL (MOO_EDIT_PREFS_ENABLE_HIGHLIGHTING, TRUE);
    NEW_KEY_BOOL (MOO_EDIT_PREFS_HIGHLIGHT_MATCHING, TRUE);
    NEW_KEY_BOOL (MOO_EDIT_PREFS_HIGHLIGHT_MISMATCHING, FALSE);
    NEW_KEY_BOOL (MOO_EDIT_PREFS_HIGHLIGHT_CURRENT_LINE, TRUE);
    NEW_KEY_BOOL (MOO_EDIT_PREFS_SHOW_LINE_NUMBERS, FALSE); /* XXX implement it */
    NEW_KEY_BOOL (MOO_EDIT_PREFS_SHOW_TABS, FALSE); /* XXX does it work? */
    NEW_KEY_BOOL (MOO_EDIT_PREFS_SHOW_TRAILING_SPACES, FALSE); /* XXX does it work? */
    NEW_KEY_BOOL (MOO_EDIT_PREFS_USE_DEFAULT_FONT, TRUE);
    NEW_KEY_STRING (MOO_EDIT_PREFS_FONT, "Monospace 12");
    NEW_KEY_FLAGS (MOO_EDIT_PREFS_QUICK_SEARCH_FLAGS,
                   MOO_TYPE_TEXT_SEARCH_FLAGS,
                   MOO_TEXT_SEARCH_CASELESS);

//     NEW_KEY_BOOL (MOO_EDIT_PREFS_SEARCH_SELECTED, FALSE);
//     NEW_KEY_BOOL (MOO_EDIT_PREFS_AUTO_INDENT, FALSE);
//     NEW_KEY_BOOL (MOO_EDIT_PREFS_LIMIT_UNDO, FALSE);
//     NEW_KEY_INT (MOO_EDIT_PREFS_LIMIT_UNDO_NUM, 25);
//     NEW_KEY_ENUM (MOO_EDIT_PREFS_ON_EXTERNAL_CHANGES,
//                   MOO_TYPE_EDIT_ON_EXTERNAL_CHANGES,
//                   MOO_EDIT_RELOAD_IF_SAFE);
}


#define get_string(key) moo_prefs_get_string (MOO_EDIT_PREFS_PREFIX "/" key)
#define get_bool(key) moo_prefs_get_bool (MOO_EDIT_PREFS_PREFIX "/" key)
#define get_int(key) moo_prefs_get_int (MOO_EDIT_PREFS_PREFIX "/" key)
#define get_color(key) moo_prefs_get_color (MOO_EDIT_PREFS_PREFIX "/" key)
#define get_enum(key) moo_prefs_get_enum (MOO_EDIT_PREFS_PREFIX "/" key)
#define get_flags(key) moo_prefs_get_flags (MOO_EDIT_PREFS_PREFIX "/" key)

void
_moo_edit_apply_settings (MooEdit *edit)
{
    GtkTextView *text_view;
    MooLangMgr *mgr;

    g_return_if_fail (MOO_IS_EDIT (edit));

    g_object_freeze_notify (G_OBJECT (edit));
    _moo_edit_freeze_config_notify (edit);

    text_view = GTK_TEXT_VIEW (edit);

    g_object_set (edit,
                  "smart-home-end", get_bool (MOO_EDIT_PREFS_SMART_HOME_END),
                  "enable-highlight", get_bool (MOO_EDIT_PREFS_ENABLE_HIGHLIGHTING),
                  "highlight-matching-brackets", get_bool (MOO_EDIT_PREFS_HIGHLIGHT_MATCHING),
                  "highlight-mismatching-brackets", get_bool (MOO_EDIT_PREFS_HIGHLIGHT_MISMATCHING),
                  "highlight-current-line", get_bool (MOO_EDIT_PREFS_HIGHLIGHT_CURRENT_LINE),
                  "show-line-numbers", get_bool (MOO_EDIT_PREFS_SHOW_LINE_NUMBERS),
                  "draw-tabs", get_bool (MOO_EDIT_PREFS_SHOW_TABS),
                  "draw-trailing-spaces", get_bool (MOO_EDIT_PREFS_SHOW_TRAILING_SPACES),
                  "quick-search-flags", get_flags (MOO_EDIT_PREFS_QUICK_SEARCH_FLAGS),
                  "tab-key-action", get_enum (MOO_EDIT_PREFS_TAB_KEY_ACTION),
                  "auto-indent", get_bool (MOO_EDIT_PREFS_AUTO_INDENT),
                  "backspace-indents", get_bool (MOO_EDIT_PREFS_BACKSPACE_INDENTS),
                  NULL);

    moo_edit_config_set (edit->config,
                         "indent-use-tabs", MOO_EDIT_CONFIG_SOURCE_PREFS,
                         !get_bool (MOO_EDIT_PREFS_SPACES_NO_TABS),
                         "indent-width", MOO_EDIT_CONFIG_SOURCE_PREFS,
                         get_int (MOO_EDIT_PREFS_INDENT_WIDTH),
                         NULL);

    if (get_bool (MOO_EDIT_PREFS_WRAP_ENABLE))
    {
        if (get_bool (MOO_EDIT_PREFS_WRAP_WORDS))
            gtk_text_view_set_wrap_mode (text_view, GTK_WRAP_WORD);
        else
            gtk_text_view_set_wrap_mode (text_view, GTK_WRAP_CHAR);
    }
    else
    {
        gtk_text_view_set_wrap_mode (text_view, GTK_WRAP_NONE);
    }

    if (get_bool (MOO_EDIT_PREFS_USE_DEFAULT_FONT))
        moo_text_view_set_font_from_string (MOO_TEXT_VIEW (edit), NULL);
    else
        moo_text_view_set_font_from_string (MOO_TEXT_VIEW (edit),
                                            get_string (MOO_EDIT_PREFS_FONT));

    mgr = moo_editor_get_lang_mgr (edit->priv->editor);
    moo_text_view_set_scheme (MOO_TEXT_VIEW (edit),
                              moo_lang_mgr_get_active_scheme (mgr));

    _moo_edit_thaw_config_notify (edit);
    g_object_thaw_notify (G_OBJECT (edit));
}
