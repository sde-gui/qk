/*
 *   mooedit/mooeditprefs.c
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define MOOEDIT_COMPILATION
#include "mooedit/mooeditprefs.h"
#include "mooedit/mooedit-private.h"
#include <string.h>


const char *moo_edit_setting                (const char     *setting_name)
{
    static char *s = NULL;
    g_return_val_if_fail (setting_name != NULL, NULL);
    g_free (s);
    s = g_strdup_printf (MOO_EDIT_PREFS_PREFIX "::%s", setting_name);
    return s;
}


static void set_matching_bracket_styles (MooEdit *edit);
static void set_font (MooEdit *edit);
static void set_text_colors (MooEdit *edit);
static void set_highlight_current_line (MooEdit *edit);


#define set_default(key,val) \
    moo_prefs_set_if_not_set_ignore_change (MOO_EDIT_PREFS_PREFIX "::" key, val)
#define set_default_color(key,val) \
    moo_prefs_set_color_if_not_set_ignore_change (MOO_EDIT_PREFS_PREFIX "::" key, val)

void        _moo_edit_set_default_settings      (void)
{
    GtkSettings *settings;
    GtkStyle *style;

    set_default (MOO_EDIT_PREFS_SEARCH_SELECTED, "NO");
    set_default (MOO_EDIT_PREFS_SMART_HOME_END, "YES");
    set_default (MOO_EDIT_PREFS_TABS_WIDTH, "8");
    set_default (MOO_EDIT_PREFS_SPACES_NO_TABS, "NO");
    set_default (MOO_EDIT_PREFS_AUTO_INDENT, "NO");
    set_default (MOO_EDIT_PREFS_LIMIT_UNDO, "NO");
    set_default (MOO_EDIT_PREFS_LIMIT_UNDO_NUM, "25");
    set_default (MOO_EDIT_PREFS_WRAP_ENABLE, "NO");
    set_default (MOO_EDIT_PREFS_WRAP_DONT_SPLIT_WORDS, "YES");
    set_default (MOO_EDIT_PREFS_SHOW_LINE_NUMBERS, "NO");
    set_default (MOO_EDIT_PREFS_SHOW_MARGIN, "NO");
    set_default (MOO_EDIT_PREFS_MARGIN, "80");
    set_default (MOO_EDIT_PREFS_CHECK_BRACKETS, "YES");
    set_default (MOO_EDIT_PREFS_USE_DEFAULT_FONT, "NO");
    set_default (MOO_EDIT_PREFS_FONT, "Courier New 11");
    set_default (MOO_EDIT_PREFS_USE_DEFAULT_COLORS, "YES");
    set_default (MOO_EDIT_PREFS_HIGHLIGHT_CURRENT_LINE, "YES");
    set_default (MOO_EDIT_PREFS_USE_SYNTAX_HIGHLIGHTING, "YES");

    settings = gtk_settings_get_default ();
    style = gtk_rc_get_style_by_paths (settings, "MooEdit", "MooEdit", MOO_TYPE_EDIT);
    if (!style) style = gtk_style_new ();
    else g_object_ref (G_OBJECT (style));
    g_return_if_fail (style != NULL);

    set_default_color (MOO_EDIT_PREFS_FOREGROUND, &(style->text[GTK_STATE_NORMAL]));
    set_default_color (MOO_EDIT_PREFS_BACKGROUND, &(style->base[GTK_STATE_NORMAL]));
    set_default_color (MOO_EDIT_PREFS_SELECTED_FOREGROUND, &(style->text[GTK_STATE_SELECTED]));
    set_default_color (MOO_EDIT_PREFS_SELECTED_BACKGROUND, &(style->base[GTK_STATE_SELECTED]));

    g_object_unref (G_OBJECT (style));

    set_default (MOO_EDIT_PREFS_HIGHLIGHT_CURRENT_LINE_COLOR, "#EEF6FF");
    set_default (MOO_EDIT_MATCHING_BRACKETS_CORRECT "::" MOO_EDIT_PREFS_BACKGROUND, "#FFFF99");
    set_default (MOO_EDIT_MATCHING_BRACKETS_INCORRECT "::" MOO_EDIT_PREFS_BACKGROUND, "red");
}


#define get(key) moo_prefs_get (MOO_EDIT_PREFS_PREFIX "::" key)
#define get_bool(key) moo_prefs_get_bool (MOO_EDIT_PREFS_PREFIX "::" key)
#define get_int(key) moo_prefs_get_int (MOO_EDIT_PREFS_PREFIX "::" key)
#define get_color(key) moo_prefs_get_color (MOO_EDIT_PREFS_PREFIX "::" key)

void        _moo_edit_apply_settings            (MooEdit    *edit)
{
    GtkSourceView *view = GTK_SOURCE_VIEW (edit);
    GtkTextView *text_view = GTK_TEXT_VIEW (edit);
    GtkSourceBuffer *buffer = edit->priv->source_buffer;

    gtk_source_view_set_smart_home_end (view, get_bool (MOO_EDIT_PREFS_SMART_HOME_END));
    gtk_source_view_set_auto_indent (view, get_bool (MOO_EDIT_PREFS_AUTO_INDENT));
    gtk_source_view_set_insert_spaces_instead_of_tabs (view, get_bool (MOO_EDIT_PREFS_SPACES_NO_TABS));

    if (get_bool (MOO_EDIT_PREFS_LIMIT_UNDO))
        gtk_source_buffer_set_max_undo_levels (buffer, 0);
    else
        gtk_source_buffer_set_max_undo_levels (buffer, get_int (MOO_EDIT_PREFS_LIMIT_UNDO_NUM));

    if (get_bool (MOO_EDIT_PREFS_WRAP_ENABLE)) {
        if (get_bool (MOO_EDIT_PREFS_WRAP_DONT_SPLIT_WORDS))
            gtk_text_view_set_wrap_mode (text_view, GTK_WRAP_WORD);
        else
            gtk_text_view_set_wrap_mode (text_view, GTK_WRAP_CHAR);
    }
    else
        gtk_text_view_set_wrap_mode (text_view, GTK_WRAP_NONE);

    gtk_source_view_set_show_line_numbers (view, get_bool (MOO_EDIT_PREFS_SHOW_LINE_NUMBERS));
    gtk_source_view_set_show_margin (view, get_bool (MOO_EDIT_PREFS_SHOW_MARGIN));
    gtk_source_view_set_margin (view, get_int (MOO_EDIT_PREFS_MARGIN));
    gtk_source_buffer_set_check_brackets (buffer, get_bool (MOO_EDIT_PREFS_CHECK_BRACKETS));
    set_matching_bracket_styles (edit);
    set_highlight_current_line (edit);
    gtk_source_buffer_set_highlight (buffer, get_bool (MOO_EDIT_PREFS_USE_SYNTAX_HIGHLIGHTING));
}


void        _moo_edit_apply_style_settings      (MooEdit    *edit)
{
    set_text_colors (edit);
    set_font (edit);
    gtk_source_view_set_tabs_width  (GTK_SOURCE_VIEW (edit),
                                     get_int (MOO_EDIT_PREFS_TABS_WIDTH));
}


void        _moo_edit_settings_changed          (const char *key,
                                                 G_GNUC_UNUSED const char *newval,
                                                 MooEdit    *edit)
{
    GtkSourceBuffer *buffer = edit->priv->source_buffer;

    if (!strcmp (key, MOO_EDIT_PREFS_PREFIX "::" MOO_EDIT_PREFS_LIMIT_UNDO) ||
        !strcmp (key, MOO_EDIT_PREFS_PREFIX "::" MOO_EDIT_PREFS_LIMIT_UNDO_NUM))
    {
        if (get_bool (MOO_EDIT_PREFS_LIMIT_UNDO))
            gtk_source_buffer_set_max_undo_levels (buffer, 0);
        else
            gtk_source_buffer_set_max_undo_levels (buffer, get_int (MOO_EDIT_PREFS_LIMIT_UNDO_NUM));
    }

    else if (!strncmp (key, MOO_EDIT_PREFS_PREFIX "::" MOO_EDIT_MATCHING_BRACKETS_CORRECT,
                       strlen (MOO_EDIT_PREFS_PREFIX "::" MOO_EDIT_MATCHING_BRACKETS_CORRECT)))
        set_matching_bracket_styles (edit);
    else if (!strncmp (key, MOO_EDIT_PREFS_PREFIX "::" MOO_EDIT_MATCHING_BRACKETS_INCORRECT,
                       strlen (MOO_EDIT_PREFS_PREFIX "::" MOO_EDIT_MATCHING_BRACKETS_INCORRECT)))
        set_matching_bracket_styles (edit);

    else if (!strcmp (key, MOO_EDIT_PREFS_PREFIX "::" MOO_EDIT_PREFS_USE_DEFAULT_FONT) ||
             !strcmp (key, MOO_EDIT_PREFS_PREFIX "::" MOO_EDIT_PREFS_FONT))
        set_font (edit);

    else if (!strcmp (key, MOO_EDIT_PREFS_PREFIX "::" MOO_EDIT_PREFS_USE_DEFAULT_COLORS) ||
             !strcmp (key, MOO_EDIT_PREFS_PREFIX "::" MOO_EDIT_PREFS_FOREGROUND) ||
             !strcmp (key, MOO_EDIT_PREFS_PREFIX "::" MOO_EDIT_PREFS_BACKGROUND) ||
             !strcmp (key, MOO_EDIT_PREFS_PREFIX "::" MOO_EDIT_PREFS_SELECTED_FOREGROUND) ||
             !strcmp (key, MOO_EDIT_PREFS_PREFIX "::" MOO_EDIT_PREFS_SELECTED_BACKGROUND) ||
             !strcmp (key, MOO_EDIT_PREFS_PREFIX "::" MOO_EDIT_PREFS_BACKGROUND) ||
             !strcmp (key, MOO_EDIT_PREFS_PREFIX "::" MOO_EDIT_PREFS_BACKGROUND))
        set_text_colors (edit);

    else if (!strcmp (key, MOO_EDIT_PREFS_PREFIX "::" MOO_EDIT_PREFS_HIGHLIGHT_CURRENT_LINE_COLOR) ||
             !strcmp (key, MOO_EDIT_PREFS_PREFIX "::" MOO_EDIT_PREFS_HIGHLIGHT_CURRENT_LINE))
        set_highlight_current_line (edit);

    else if (!strcmp (key, MOO_EDIT_PREFS_PREFIX "::" MOO_EDIT_PREFS_USE_SYNTAX_HIGHLIGHTING))
        gtk_source_buffer_set_highlight (buffer, get_bool (MOO_EDIT_PREFS_USE_SYNTAX_HIGHLIGHTING));
}


static void set_matching_bracket_styles (MooEdit *edit)
{
    GtkSourceBuffer *buffer = edit->priv->source_buffer;
    GtkSourceTagStyle style;
    const GdkColor *color;

    style.is_default = TRUE;

    style.mask = 0;
    style.bold = get_bool (MOO_EDIT_MATCHING_BRACKETS_CORRECT "::" MOO_EDIT_PREFS_BOLD);
    style.italic = get_bool (MOO_EDIT_MATCHING_BRACKETS_CORRECT "::" MOO_EDIT_PREFS_ITALIC);
    style.underline = get_bool (MOO_EDIT_MATCHING_BRACKETS_CORRECT "::" MOO_EDIT_PREFS_UNDERLINE);
    style.strikethrough = get_bool (MOO_EDIT_MATCHING_BRACKETS_CORRECT "::" MOO_EDIT_PREFS_STRIKETHROUGH);
    color = get_color (MOO_EDIT_MATCHING_BRACKETS_CORRECT "::" MOO_EDIT_PREFS_BACKGROUND);
    if (color) {
        style.background = *color;
        style.mask |= GTK_SOURCE_TAG_STYLE_USE_BACKGROUND;
    }
    color = get_color (MOO_EDIT_MATCHING_BRACKETS_CORRECT "::" MOO_EDIT_PREFS_FOREGROUND);
    if (color) {
        style.foreground = *color;
        style.mask |= GTK_SOURCE_TAG_STYLE_USE_FOREGROUND;
    }
    gtk_source_buffer_set_bracket_correct_match_style (buffer, &style);

    style.mask = 0;
    style.bold = get_bool (MOO_EDIT_MATCHING_BRACKETS_INCORRECT "::" MOO_EDIT_PREFS_BOLD);
    style.italic = get_bool (MOO_EDIT_MATCHING_BRACKETS_INCORRECT "::" MOO_EDIT_PREFS_ITALIC);
    style.underline = get_bool (MOO_EDIT_MATCHING_BRACKETS_INCORRECT "::" MOO_EDIT_PREFS_UNDERLINE);
    style.strikethrough = get_bool (MOO_EDIT_MATCHING_BRACKETS_INCORRECT "::" MOO_EDIT_PREFS_STRIKETHROUGH);
    color = get_color (MOO_EDIT_MATCHING_BRACKETS_INCORRECT "::" MOO_EDIT_PREFS_BACKGROUND);
    if (color) {
        style.background = *color;
        style.mask |= GTK_SOURCE_TAG_STYLE_USE_BACKGROUND;
    }
    color = get_color (MOO_EDIT_MATCHING_BRACKETS_INCORRECT "::" MOO_EDIT_PREFS_FOREGROUND);
    if (color) {
        style.foreground = *color;
        style.mask |= GTK_SOURCE_TAG_STYLE_USE_FOREGROUND;
    }
    gtk_source_buffer_set_bracket_incorrect_match_style (buffer, &style);
}


static void set_font (MooEdit *edit)
{
    if (get_bool (MOO_EDIT_PREFS_USE_DEFAULT_FONT))
        moo_edit_set_font_from_string (edit, NULL);
    else
        moo_edit_set_font_from_string (edit, get (MOO_EDIT_PREFS_FONT));
}


static void set_text_colors (MooEdit *edit)
{
    GtkWidget *widget = GTK_WIDGET (edit);

    if (!GTK_WIDGET_REALIZED (widget)) return;

    if (get_bool (MOO_EDIT_PREFS_USE_DEFAULT_COLORS)) {
        gtk_widget_modify_text (widget, GTK_STATE_NORMAL, NULL);
        gtk_widget_modify_base (widget, GTK_STATE_NORMAL, NULL);
        gtk_widget_modify_text (widget, GTK_STATE_SELECTED, NULL);
        gtk_widget_modify_base (widget, GTK_STATE_SELECTED, NULL);
    }
    else {
        gtk_widget_modify_text (widget, GTK_STATE_NORMAL,
                                get_color (MOO_EDIT_PREFS_FOREGROUND));
        gtk_widget_modify_base (widget, GTK_STATE_NORMAL,
                                get_color (MOO_EDIT_PREFS_BACKGROUND));
        gtk_widget_modify_text (widget, GTK_STATE_SELECTED,
                                get_color (MOO_EDIT_PREFS_SELECTED_FOREGROUND));
        gtk_widget_modify_base (widget, GTK_STATE_SELECTED,
                                get_color (MOO_EDIT_PREFS_SELECTED_BACKGROUND));
    }
}


static void set_highlight_current_line (MooEdit *edit)
{
    const GdkColor *color;
    GtkSourceView *view = GTK_SOURCE_VIEW (edit);

    color = get_color (MOO_EDIT_PREFS_HIGHLIGHT_CURRENT_LINE_COLOR);
    if (!color)
        moo_prefs_set_ignore_change (MOO_EDIT_PREFS_PREFIX "::" MOO_EDIT_PREFS_HIGHLIGHT_CURRENT_LINE_COLOR,
                                     "#EEF6FF");
    color = get_color (MOO_EDIT_PREFS_HIGHLIGHT_CURRENT_LINE_COLOR);

    gtk_source_view_set_highlight_current_line (view, get_bool (MOO_EDIT_PREFS_HIGHLIGHT_CURRENT_LINE));
    gtk_source_view_set_highlight_current_line_color (view, color);
}
