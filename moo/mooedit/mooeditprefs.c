/*
 *   mooeditprefs.c
 *
 *   Copyright (C) 2004-2008 by Yevgen Muntyan <muntyan@tamu.edu>
 *
 *   This file is part of medit.  medit is free software; you can
 *   redistribute it and/or modify it under the terms of the
 *   GNU Lesser General Public License as published by the
 *   Free Software Foundation; either version 2.1 of the License,
 *   or (at your option) any later version.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with medit.  If not, see <http://www.gnu.org/licenses/>.
 */

#define MOOEDIT_COMPILATION
#include "mooedit/mooeditprefs.h"
#include "mooedit/mooedit-impl.h"
#include "mooedit/mooedit-fileops.h"
#include "mooedit/mootextview-private.h"
#include "mooedit/mooedit-enums.h"
#include "mooedit/mootextbuffer.h"
#include "mooedit/moolangmgr.h"
#include "mooutils/mooencodings.h"
#include <string.h>

#ifdef __WIN32__
#define DEFAULT_FONT "Monospace 10"
#else
#define DEFAULT_FONT "Monospace"
#endif

static void _moo_edit_init_prefs (void);


static guint settings[MOO_EDIT_LAST_SETTING];
guint *_moo_edit_settings = settings;


void
_moo_edit_init_config (void)
{
    static gboolean done = FALSE;

    if (done)
        return;
    done = TRUE;

    _moo_edit_init_prefs ();

    _moo_edit_settings[MOO_EDIT_SETTING_LANG] =
        moo_edit_config_install_setting (g_param_spec_string ("lang", "lang", "lang",
                                                              NULL,
                                                              (GParamFlags) G_PARAM_READWRITE));
    _moo_edit_settings[MOO_EDIT_SETTING_INDENT] =
        moo_edit_config_install_setting (g_param_spec_string ("indent", "indent", "indent",
                                                              NULL,
                                                              (GParamFlags) G_PARAM_READWRITE));
    _moo_edit_settings[MOO_EDIT_SETTING_STRIP] =
        moo_edit_config_install_setting (g_param_spec_boolean ("strip", "strip", "strip",
                                                               FALSE,
                                                               (GParamFlags) G_PARAM_READWRITE));
    _moo_edit_settings[MOO_EDIT_SETTING_ADD_NEWLINE] =
        moo_edit_config_install_setting (g_param_spec_boolean ("add-newline", "add-newline", "add-newline",
                                                               FALSE,
                                                               (GParamFlags) G_PARAM_READWRITE));
    _moo_edit_settings[MOO_EDIT_SETTING_WRAP_MODE] =
        moo_edit_config_install_setting (g_param_spec_enum ("wrap-mode", "wrap-mode", "wrap-mode",
                                                            GTK_TYPE_WRAP_MODE, GTK_WRAP_NONE,
                                                            (GParamFlags) G_PARAM_READWRITE));
    _moo_edit_settings[MOO_EDIT_SETTING_SHOW_LINE_NUMBERS] =
        moo_edit_config_install_setting (g_param_spec_boolean ("show-line-numbers", "show-line-numbers", "show-line-numbers",
                                                               FALSE,
                                                               (GParamFlags) G_PARAM_READWRITE));
    _moo_edit_settings[MOO_EDIT_SETTING_TAB_WIDTH] =
        moo_edit_config_install_setting (g_param_spec_uint ("tab-width", "tab-width", "tab-width",
                                                            1, G_MAXUINT, 8,
                                                            (GParamFlags) G_PARAM_READWRITE));
    _moo_edit_settings[MOO_EDIT_SETTING_WORD_CHARS] =
        moo_edit_config_install_setting (g_param_spec_string ("word-chars", "word-chars", "word-chars",
                                                              NULL, (GParamFlags) G_PARAM_READWRITE));
}


#define NEW_KEY_BOOL(s,v)    moo_prefs_new_key_bool (MOO_EDIT_PREFS_PREFIX "/" s, v)
#define NEW_KEY_INT(s,v)     moo_prefs_new_key_int (MOO_EDIT_PREFS_PREFIX "/" s, v)
#define NEW_KEY_STRING(s,v)  moo_prefs_new_key_string (MOO_EDIT_PREFS_PREFIX "/" s, v)

static void
_moo_edit_init_prefs (void)
{
    static gboolean done = FALSE;

    if (done)
        return;
    else
        done = TRUE;

    NEW_KEY_BOOL (MOO_EDIT_PREFS_SPACES_NO_TABS, FALSE);
    NEW_KEY_INT (MOO_EDIT_PREFS_INDENT_WIDTH, 8);
    NEW_KEY_INT (MOO_EDIT_PREFS_TAB_WIDTH, 8);
    NEW_KEY_BOOL (MOO_EDIT_PREFS_AUTO_INDENT, TRUE);
    NEW_KEY_BOOL (MOO_EDIT_PREFS_TAB_INDENTS, TRUE);
    NEW_KEY_BOOL (MOO_EDIT_PREFS_BACKSPACE_INDENTS, TRUE);

    NEW_KEY_BOOL (MOO_EDIT_PREFS_SAVE_SESSION, TRUE);
    NEW_KEY_BOOL (MOO_EDIT_PREFS_AUTO_SAVE, FALSE);
    NEW_KEY_INT (MOO_EDIT_PREFS_AUTO_SAVE_INTERVAL, 5);
    NEW_KEY_BOOL (MOO_EDIT_PREFS_MAKE_BACKUPS, FALSE);
    NEW_KEY_BOOL (MOO_EDIT_PREFS_STRIP, FALSE);
    NEW_KEY_BOOL (MOO_EDIT_PREFS_ADD_NEWLINE, FALSE);

    NEW_KEY_BOOL (MOO_EDIT_PREFS_USE_TABS, TRUE);
    NEW_KEY_BOOL (MOO_EDIT_PREFS_OPEN_NEW_WINDOW, FALSE);

    NEW_KEY_STRING (MOO_EDIT_PREFS_TITLE_FORMAT, "%a - %f%s");
    NEW_KEY_STRING (MOO_EDIT_PREFS_TITLE_FORMAT_NO_DOC, "%a");

    NEW_KEY_BOOL (MOO_EDIT_PREFS_DIALOGS_OPEN_FOLLOWS_DOC, FALSE);

    NEW_KEY_STRING (MOO_EDIT_PREFS_COLOR_SCHEME, "kate");

    NEW_KEY_BOOL (MOO_EDIT_PREFS_SMART_HOME_END, TRUE);
    NEW_KEY_BOOL (MOO_EDIT_PREFS_WRAP_ENABLE, FALSE);
    NEW_KEY_BOOL (MOO_EDIT_PREFS_WRAP_WORDS, TRUE);
    NEW_KEY_BOOL (MOO_EDIT_PREFS_ENABLE_HIGHLIGHTING, TRUE);
    NEW_KEY_BOOL (MOO_EDIT_PREFS_HIGHLIGHT_MATCHING, TRUE);
    NEW_KEY_BOOL (MOO_EDIT_PREFS_HIGHLIGHT_MISMATCHING, FALSE);
    NEW_KEY_BOOL (MOO_EDIT_PREFS_HIGHLIGHT_CURRENT_LINE, TRUE);
    NEW_KEY_BOOL (MOO_EDIT_PREFS_DRAW_RIGHT_MARGIN, FALSE);
    NEW_KEY_INT (MOO_EDIT_PREFS_RIGHT_MARGIN_OFFSET, 80);
    NEW_KEY_BOOL (MOO_EDIT_PREFS_SHOW_LINE_NUMBERS, FALSE);
    NEW_KEY_BOOL (MOO_EDIT_PREFS_SHOW_TABS, FALSE);
    NEW_KEY_BOOL (MOO_EDIT_PREFS_SHOW_SPACES, FALSE);
    NEW_KEY_BOOL (MOO_EDIT_PREFS_SHOW_TRAILING_SPACES, FALSE);
    NEW_KEY_STRING (MOO_EDIT_PREFS_FONT, DEFAULT_FONT);
    NEW_KEY_INT (MOO_EDIT_PREFS_QUICK_SEARCH_FLAGS, MOO_TEXT_SEARCH_CASELESS);
    NEW_KEY_STRING (MOO_EDIT_PREFS_LINE_NUMBERS_FONT, NULL);

    NEW_KEY_STRING (MOO_EDIT_PREFS_ENCODINGS, _moo_get_default_encodings ());
    NEW_KEY_STRING (MOO_EDIT_PREFS_ENCODING_SAVE, MOO_ENCODING_UTF8);
}


#define get_string(key) moo_prefs_get_string (MOO_EDIT_PREFS_PREFIX "/" key)
#define get_bool(key) moo_prefs_get_bool (MOO_EDIT_PREFS_PREFIX "/" key)
#define get_int(key) moo_prefs_get_int (MOO_EDIT_PREFS_PREFIX "/" key)

void
_moo_edit_update_global_config (void)
{
    gboolean use_tabs, strip, show_line_numbers, add_newline;
    int indent_width, tab_width;
    GtkWrapMode wrap_mode;

    use_tabs = !get_bool (MOO_EDIT_PREFS_SPACES_NO_TABS);
    indent_width = get_int (MOO_EDIT_PREFS_INDENT_WIDTH);
    tab_width = get_int (MOO_EDIT_PREFS_TAB_WIDTH);
    strip = get_bool (MOO_EDIT_PREFS_STRIP);
    add_newline = get_bool (MOO_EDIT_PREFS_ADD_NEWLINE);
    show_line_numbers = get_bool (MOO_EDIT_PREFS_SHOW_LINE_NUMBERS);

    if (get_bool (MOO_EDIT_PREFS_WRAP_ENABLE))
    {
        if (get_bool (MOO_EDIT_PREFS_WRAP_WORDS))
            wrap_mode = GTK_WRAP_WORD;
        else
            wrap_mode = GTK_WRAP_CHAR;
    }
    else
    {
        wrap_mode = GTK_WRAP_NONE;
    }

    moo_edit_config_set_global (MOO_EDIT_CONFIG_SOURCE_AUTO,
                                "indent-use-tabs", use_tabs,
                                "indent-width", indent_width,
                                "tab-width", tab_width,
                                "strip", strip,
                                "add-newline", add_newline,
                                "show-line-numbers", show_line_numbers,
                                "wrap-mode", wrap_mode,
                                NULL);
}


void
_moo_edit_apply_prefs (MooEdit *edit)
{
    MooLangMgr *mgr;
    MooTextStyleScheme *scheme;
    MooDrawWhitespaceFlags ws_flags = 0;

    g_return_if_fail (MOO_IS_EDIT (edit));

    g_object_freeze_notify (G_OBJECT (edit));

    g_object_set (edit,
                  "smart-home-end", get_bool (MOO_EDIT_PREFS_SMART_HOME_END),
                  "enable-highlight", get_bool (MOO_EDIT_PREFS_ENABLE_HIGHLIGHTING),
                  "highlight-matching-brackets", get_bool (MOO_EDIT_PREFS_HIGHLIGHT_MATCHING),
                  "highlight-mismatching-brackets", get_bool (MOO_EDIT_PREFS_HIGHLIGHT_MISMATCHING),
                  "highlight-current-line", get_bool (MOO_EDIT_PREFS_HIGHLIGHT_CURRENT_LINE),
                  "draw-right-margin", get_bool (MOO_EDIT_PREFS_DRAW_RIGHT_MARGIN),
                  "right-margin-offset", get_int (MOO_EDIT_PREFS_RIGHT_MARGIN_OFFSET),
                  "quick-search-flags", get_int (MOO_EDIT_PREFS_QUICK_SEARCH_FLAGS),
                  "auto-indent", get_bool (MOO_EDIT_PREFS_AUTO_INDENT),
                  "tab-indents", get_bool (MOO_EDIT_PREFS_TAB_INDENTS),
                  "backspace-indents", get_bool (MOO_EDIT_PREFS_BACKSPACE_INDENTS),
                  NULL);

    if (get_bool (MOO_EDIT_PREFS_SHOW_TABS))
        ws_flags |= MOO_DRAW_TABS;
    if (get_bool (MOO_EDIT_PREFS_SHOW_SPACES))
        ws_flags |= MOO_DRAW_SPACES;
    if (get_bool (MOO_EDIT_PREFS_SHOW_TRAILING_SPACES))
        ws_flags |= MOO_DRAW_TRAILING_SPACES;
    g_object_set (edit, "draw-whitespace", ws_flags, NULL);

    moo_text_view_set_font_from_string (MOO_TEXT_VIEW (edit),
                                        get_string (MOO_EDIT_PREFS_FONT));
    _moo_text_view_set_line_numbers_font (MOO_TEXT_VIEW (edit),
                                          get_string (MOO_EDIT_PREFS_LINE_NUMBERS_FONT));

    mgr = moo_lang_mgr_default ();
    scheme = moo_lang_mgr_get_active_scheme (mgr);

    if (scheme)
        moo_text_view_set_style_scheme (MOO_TEXT_VIEW (edit), scheme);

    g_object_thaw_notify (G_OBJECT (edit));
}


const char *
moo_edit_setting (const char *setting_name)
{
#define STR_STACK_SIZE 4
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
#undef STR_STACK_SIZE
}
