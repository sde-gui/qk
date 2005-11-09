/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   mooeditfind.c
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

#define MOOEDIT_COMPILATION
#include "mooedit/mootextview-private.h"
#include "mooedit/mooeditsearch.h"
#include "mooedit/mooeditprefs.h"
#include "mooedit/mooeditdialogs.h"
#include "mooedit/mooeditgotoline-glade.h"
#include "mooedit/mooeditfind-glade.h"
#include "mooutils/moohistoryentry.h"
#include "mooutils/moocompat.h"
#include "mooutils/mooglade.h"


static void
scroll_to_mark (MooTextView *view,
                GtkTextMark *mark)
{
    gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (view), mark, 0.2, FALSE, 0, 0);
}


/****************************************************************************/
/* Go to line
 */

static void     update_spin_value   (GtkRange       *scale,
                                     GtkSpinButton  *spin);
static gboolean update_scale_value  (GtkSpinButton  *spin,
                                     GtkRange       *scale);

static void     update_spin_value   (GtkRange       *scale,
                                     GtkSpinButton  *spin)
{
    double value = gtk_range_get_value (scale);
    g_signal_handlers_block_matched (spin, G_SIGNAL_MATCH_FUNC, 0, 0, 0,
                                     (gpointer)update_scale_value, 0);
    gtk_spin_button_set_value (spin, value);
    g_signal_handlers_unblock_matched (spin, G_SIGNAL_MATCH_FUNC, 0, 0, 0,
                                       (gpointer)update_scale_value, 0);
}

static gboolean update_scale_value  (GtkSpinButton  *spin,
                                     GtkRange       *scale)
{
    double value = gtk_spin_button_get_value (spin);
    g_signal_handlers_block_matched (scale, G_SIGNAL_MATCH_FUNC, 0, 0, 0,
                                     (gpointer)update_spin_value, 0);
    gtk_range_set_value (scale, value);
    g_signal_handlers_unblock_matched (scale, G_SIGNAL_MATCH_FUNC, 0, 0, 0,
                                       (gpointer)update_spin_value, 0);
    return FALSE;
}


void
moo_text_view_goto_line (MooTextView *view,
                         int          line)
{
    GtkWidget *dialog;
    GtkTextBuffer *buffer;
    int line_count;
    GtkTextIter iter;
    GtkRange *scale;
    GtkSpinButton *spin;

    g_return_if_fail (MOO_IS_TEXT_VIEW (view));

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));
    line_count = gtk_text_buffer_get_line_count (buffer);

    if (line < 0 || line >= line_count)
    {
        MooGladeXML *xml;

        xml = moo_glade_xml_new_from_buf (MOO_EDIT_GOTO_LINE_GLADE_UI,
                                          -1, NULL, NULL);
        g_return_if_fail (xml != NULL);

        dialog = moo_glade_xml_get_widget (xml, "dialog");

#if GTK_CHECK_VERSION(2,6,0)
        gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog),
                                                 GTK_RESPONSE_OK,
                                                 GTK_RESPONSE_CANCEL,
                                                 -1);
#endif /* GTK_CHECK_VERSION(2,6,0) */

        gtk_text_buffer_get_iter_at_mark (buffer, &iter, gtk_text_buffer_get_insert (buffer));
        line = gtk_text_iter_get_line (&iter);

        scale = moo_glade_xml_get_widget (xml, "scale");
        gtk_range_set_range (scale, 1, line_count + 1);
        gtk_range_set_value (scale, line + 1);

        spin = moo_glade_xml_get_widget (xml, "spin");
        gtk_entry_set_activates_default (GTK_ENTRY (spin), TRUE);
        gtk_spin_button_set_range (spin, 1, line_count);
        gtk_spin_button_set_value (spin, line + 1);
        gtk_editable_select_region (GTK_EDITABLE (spin), 0, -1);

        g_signal_connect (scale, "value-changed", G_CALLBACK (update_spin_value), spin);
        g_signal_connect (spin, "value-changed", G_CALLBACK (update_scale_value), scale);

        gtk_window_set_transient_for (GTK_WINDOW (dialog),
                                      GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (view))));

        moo_glade_xml_unref (xml);

        if (gtk_dialog_run (GTK_DIALOG (dialog)) != GTK_RESPONSE_OK)
        {
            gtk_widget_destroy (dialog);
            return;
        }

        line = (int)gtk_spin_button_get_value (spin) - 1;
        gtk_widget_destroy (dialog);
    }

    gtk_text_buffer_get_iter_at_line (buffer, &iter, line);
    gtk_text_buffer_place_cursor (buffer, &iter);
    scroll_to_mark (view, gtk_text_buffer_get_insert (buffer));
}


/****************************************************************************/
/* Search and replace
 */

static void          set                (GtkWidget  *dialog,
                                         gboolean    regex,
                                         gboolean    case_sensitive,
                                         gboolean    whole_words,
                                         gboolean    from_cursor,
                                         gboolean    backwards,
                                         gboolean    selected,
                                         gboolean    dont_prompt_on_replace);
static void          get                (GtkWidget  *dialog,
                                         gboolean   *regex,
                                         gboolean   *case_sensitive,
                                         gboolean   *whole_words,
                                         gboolean   *from_cursor,
                                         gboolean   *backwards,
                                         gboolean   *selected,
                                         gboolean   *dont_prompt_on_replace);
static const char   *get_text           (GtkWidget  *dialog);
static const char   *get_replace_with   (GtkWidget  *dialog);
static void          set_text           (GtkWidget  *dialog,
                                         const char *text);
static void          set_replace_with   (GtkWidget  *dialog,
                                         const char *text);

static GtkWidget    *create_find_dialog (gboolean    replace);


typedef struct {
    MooTextView *view;
    GtkWidget *dialog;
    MooTextReplaceResponseType response;
} PromptFuncData;

static MooTextReplaceResponseType prompt_on_replace_func
                                            (const char         *text,
                                             EggRegex           *regex,
                                             const char         *replacement,
                                             GtkTextIter        *to_replace_start,
                                             GtkTextIter        *to_replace_end,
                                             gpointer            data);

static GtkWidget*
create_find_dialog (gboolean replace)
{
    MooGladeXML *xml;
    GtkWidget *dialog, *replace_frame, *dont_prompt_on_replace;
    GtkButton *ok_btn;
    MooHistoryEntry *text_to_find, *replacement_text;

    xml = moo_glade_xml_new_empty ();
    moo_glade_xml_map_id (xml, "text_to_find", MOO_TYPE_HISTORY_ENTRY);
    moo_glade_xml_map_id (xml, "replacement_text", MOO_TYPE_HISTORY_ENTRY);
    moo_glade_xml_parse_memory (xml, MOO_EDIT_FIND_GLADE_UI, -1, "dialog");

    dialog = moo_glade_xml_get_widget (xml, "dialog");
    g_return_val_if_fail (dialog != NULL, NULL);

    g_object_set_data_full (G_OBJECT (dialog), "moo-dialog-xml",
                            xml, (GDestroyNotify) moo_glade_xml_unref);

    replace_frame = moo_glade_xml_get_widget (xml, "replace_frame");
    dont_prompt_on_replace = moo_glade_xml_get_widget (xml, "dont_prompt_on_replace");
    ok_btn = moo_glade_xml_get_widget (xml, "ok_btn");

    text_to_find = moo_glade_xml_get_widget (xml, "text_to_find");
    replacement_text = moo_glade_xml_get_widget (xml, "replacement_text");
    moo_history_entry_set_list (text_to_find, _moo_text_search_params->text_to_find_history);
    moo_history_entry_set_list (replacement_text, _moo_text_search_params->replacement_history);

    if (replace)
    {
        gtk_window_set_title (GTK_WINDOW (dialog), "Replace");
        gtk_widget_show (replace_frame);
        gtk_widget_show (dont_prompt_on_replace);
        gtk_button_set_label (ok_btn, GTK_STOCK_FIND_AND_REPLACE);
    }
    else
    {
        gtk_window_set_title (GTK_WINDOW (dialog), "Find");
        gtk_widget_hide (replace_frame);
        gtk_widget_hide (dont_prompt_on_replace);
        gtk_button_set_label (ok_btn, GTK_STOCK_FIND);
    }

    return dialog;
}


void
_moo_text_view_find (MooTextView *view)
{
    GtkWidget *dialog;
    gboolean regex, case_sensitive, whole_words, from_cursor, backwards, selected;
    GtkTextIter sel_start, sel_end;
    int response;
    MooTextSearchOptions options;
    const char *text;
    GtkTextMark *insert;
    GtkTextBuffer *buffer;

    g_return_if_fail (MOO_IS_TEXT_VIEW (view));

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));

    regex = _moo_text_search_params->regex;
    case_sensitive = _moo_text_search_params->case_sensitive;
    backwards = _moo_text_search_params->backwards;
    whole_words = _moo_text_search_params->whole_words;
    from_cursor = _moo_text_search_params->from_cursor;

    selected = FALSE;
    if (moo_prefs_get_bool (moo_edit_setting (MOO_EDIT_PREFS_SEARCH_SELECTED)) &&
        gtk_text_buffer_get_selection_bounds (buffer, &sel_start, &sel_end) &&
        ABS (gtk_text_iter_get_line (&sel_start) - gtk_text_iter_get_line (&sel_end) > 1))
            selected = TRUE;

    dialog = create_find_dialog (FALSE);
    set (dialog, regex, case_sensitive, whole_words,
         from_cursor, backwards, selected, FALSE);

    if (gtk_text_buffer_get_selection_bounds (buffer, &sel_start, &sel_end) &&
        gtk_text_iter_get_line (&sel_start) == gtk_text_iter_get_line (&sel_end))
    {
        char *selection = gtk_text_buffer_get_text (buffer, &sel_start, &sel_end, TRUE);
        set_text (dialog, selection);
        g_free (selection);
    }
    else if (_moo_text_search_params->text)
    {
        set_text (dialog, _moo_text_search_params->text);
    }

    gtk_window_set_transient_for (GTK_WINDOW (dialog),
                                  GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (view))));
    response = gtk_dialog_run (GTK_DIALOG (dialog));

    if (response != GTK_RESPONSE_OK)
    {
        gtk_widget_destroy (dialog);
        return;
    }

    get (dialog, &regex, &case_sensitive, &whole_words,
         &from_cursor, &backwards, &selected, NULL);

    if (selected)
    {
        g_warning ("%s: searching in selected not imlemented\n", G_STRLOC);
        gtk_widget_destroy (dialog);
        return;
    }

    _moo_text_search_params->regex = regex;
    _moo_text_search_params->case_sensitive = case_sensitive;
    _moo_text_search_params->backwards = backwards;
    _moo_text_search_params->whole_words = whole_words;
    _moo_text_search_params->from_cursor = from_cursor;

    _moo_text_search_params->last_search_stamp++;
    view->priv->last_search_stamp = _moo_text_search_params->last_search_stamp;

    g_free (_moo_text_search_params->text);
    _moo_text_search_params->text = g_strdup (get_text (dialog));
    text = _moo_text_search_params->text;
    gtk_widget_destroy (dialog);

    if (text && text[0])
        moo_history_list_add (_moo_text_search_params->text_to_find_history, text);

    options = 0;

    if (regex)
        options |= MOO_TEXT_SEARCH_REGEX;
    if (backwards)
        options |= MOO_TEXT_SEARCH_BACKWARDS;
    if (!case_sensitive)
        options |= MOO_TEXT_SEARCH_CASE_INSENSITIVE;

    insert = gtk_text_buffer_get_insert (buffer);

    {
        GtkTextIter start, search_start, limit, match_start, match_end;
        gboolean result;
        GError *err = NULL;

        if (from_cursor) {
            gtk_text_buffer_get_iter_at_mark (buffer, &start, insert);
        }
        else {
            if (backwards)
                gtk_text_buffer_get_end_iter (buffer, &start);
            else
                gtk_text_buffer_get_start_iter (buffer, &start);
        }

        search_start = start;

        if (backwards)
            gtk_text_buffer_get_start_iter (buffer, &limit);
        else
            gtk_text_buffer_get_end_iter (buffer, &limit);

        result = moo_text_search (&start, &limit, text,
                                  &match_start, &match_end, options,
                                  &err);
        if (!result)
        {
            if (err) {
                moo_text_regex_error_dialog (view, err);
                g_error_free (err);
                return;
            }

            if (!from_cursor) {
                moo_text_nothing_found_dialog (view, text, regex);
                return;
            }
            if ((backwards && gtk_text_iter_is_end (&start)) ||
                (!backwards && gtk_text_iter_is_start (&start)))
            {
                moo_text_nothing_found_dialog (view, text, regex);
                return;
            }

            if (!moo_text_search_from_beginning_dialog (view, backwards))
            {
                return;
            }

            if (backwards)
                gtk_text_buffer_get_end_iter (buffer, &start);
            else
                gtk_text_buffer_get_start_iter (buffer, &start);

            limit = search_start;

            result = moo_text_search (&start, &limit, text,
                                      &match_start, &match_end, options,
                                      NULL);

            if (!result) {
                moo_text_nothing_found_dialog (view, text, regex);
                return;
            }
        }

        if (backwards)
            gtk_text_iter_order (&match_end, &match_start);

        if (!view->priv->last_found_start) {
            view->priv->last_found_start =
                gtk_text_buffer_create_mark (buffer, NULL, &match_start, TRUE);
            view->priv->last_found_end =
                gtk_text_buffer_create_mark (buffer, NULL, &match_end, TRUE);
        }
        else {
            gtk_text_buffer_move_mark (buffer, view->priv->last_found_start,
                                       &match_start);
            gtk_text_buffer_move_mark (buffer, view->priv->last_found_end,
                                       &match_end);
        }

        gtk_text_buffer_select_range (buffer, &match_end, &match_start);
        scroll_to_mark (view, insert);
    }
}


void
_moo_text_view_find_next (MooTextView *view)
{
    gboolean regex, case_sensitive, whole_words, backwards;
    MooTextSearchOptions options;
    const char *text;
    GtkTextMark *insert;
    GtkTextBuffer *buffer;

    g_return_if_fail (MOO_IS_TEXT_VIEW (view));

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));

    if (_moo_text_search_params->last_search_stamp < 0 ||
        !_moo_text_search_params->text)
            return moo_text_view_find_interactive (view);

    regex = _moo_text_search_params->regex;
    case_sensitive = _moo_text_search_params->case_sensitive;
    backwards = _moo_text_search_params->backwards;
    whole_words = _moo_text_search_params->whole_words;

    view->priv->last_search_stamp = _moo_text_search_params->last_search_stamp;

    text = _moo_text_search_params->text;

    options = 0;
    if (regex) options |= MOO_TEXT_SEARCH_REGEX;
    if (backwards) options |= MOO_TEXT_SEARCH_BACKWARDS;
    if (!case_sensitive) options |= MOO_TEXT_SEARCH_CASE_INSENSITIVE;

    insert = gtk_text_buffer_get_insert (buffer);

    {
        GtkTextIter start, search_start, match_start, match_end, limit;
        gboolean result;

        gtk_text_buffer_get_iter_at_mark (buffer, &start, insert);
        search_start = start;

        if (backwards)
            gtk_text_buffer_get_start_iter (buffer, &limit);
        else
            gtk_text_buffer_get_end_iter (buffer, &limit);

        result = moo_text_search (&start, &limit, text,
                                  &match_start, &match_end, options,
                                  NULL);
        if (!result)
        {
            if ((backwards && gtk_text_iter_is_end (&start)) ||
                (!backwards && gtk_text_iter_is_start (&start)))
            {
                moo_text_nothing_found_dialog (view, text, regex);
                return;
            }

            if (!moo_text_search_from_beginning_dialog (view, backwards))
            {
                return;
            }

            if (backwards)
                gtk_text_buffer_get_end_iter (buffer, &start);
            else
                gtk_text_buffer_get_start_iter (buffer, &start);

            limit = search_start;

            result = moo_text_search (&start, &search_start, text,
                                      &match_start, &match_end, options,
                                      NULL);

            if (!result) {
                moo_text_nothing_found_dialog (view, text, regex);
                return;
            }
        }

        if (backwards)
            gtk_text_iter_order (&match_end, &match_start);

        if (!view->priv->last_found_start) {
            view->priv->last_found_start =
                gtk_text_buffer_create_mark (buffer, NULL, &match_start, TRUE);
            view->priv->last_found_end =
                gtk_text_buffer_create_mark (buffer, NULL, &match_end, TRUE);
        }
        else {
            gtk_text_buffer_move_mark (buffer, view->priv->last_found_start,
                                       &match_start);
            gtk_text_buffer_move_mark (buffer, view->priv->last_found_end,
                                       &match_end);
        }

        gtk_text_buffer_select_range (buffer, &match_end, &match_start);
        scroll_to_mark (view, insert);
    }
}


void
_moo_text_view_find_previous (MooTextView *view)
{
    gboolean regex, case_sensitive, whole_words, backwards;
    MooTextSearchOptions options;
    const char *text;
    GtkTextMark *insert;
    GtkTextBuffer *buffer;

    g_return_if_fail (MOO_IS_TEXT_VIEW (view));

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));

    if (_moo_text_search_params->last_search_stamp < 0 ||
        !_moo_text_search_params->text)
            return moo_text_view_find_interactive (view);

    regex = _moo_text_search_params->regex;
    case_sensitive = _moo_text_search_params->case_sensitive;
    backwards = _moo_text_search_params->backwards;
    whole_words = _moo_text_search_params->whole_words;

    view->priv->last_search_stamp = _moo_text_search_params->last_search_stamp;

    text = _moo_text_search_params->text;

    options = 0;
    if (regex) options |= MOO_TEXT_SEARCH_REGEX;
    if (!backwards) options |= MOO_TEXT_SEARCH_BACKWARDS;
    if (!case_sensitive) options |= MOO_TEXT_SEARCH_CASE_INSENSITIVE;

    insert = gtk_text_buffer_get_insert (buffer);

    {
        GtkTextIter start, search_start, match_start, match_end, limit;
        gboolean result;

        gtk_text_buffer_get_iter_at_mark (buffer, &start, insert);
        search_start = start;

        if (!backwards)
            gtk_text_buffer_get_start_iter (buffer, &limit);
        else
            gtk_text_buffer_get_end_iter (buffer, &limit);

        result = moo_text_search (&start, &limit, text,
                                  &match_start, &match_end, options,
                                  NULL);

        if (!result)
        {
            if ((!backwards && gtk_text_iter_is_end (&start)) ||
                (backwards && gtk_text_iter_is_start (&start)))
            {
                moo_text_nothing_found_dialog (view, text, regex);
                return;
            }

            if (!moo_text_search_from_beginning_dialog (view, !backwards))
            {
                return;
            }

            if (!backwards)
                gtk_text_buffer_get_end_iter (buffer, &start);
            else
                gtk_text_buffer_get_start_iter (buffer, &start);

            limit = search_start;

            result = moo_text_search (&start, &search_start, text,
                                      &match_start, &match_end, options,
                                      NULL);

            if (!result) {
                moo_text_nothing_found_dialog (view, text, regex);
                return;
            }
        }

        if (!backwards)
            gtk_text_iter_order (&match_end, &match_start);

        if (!view->priv->last_found_start) {
            view->priv->last_found_start =
                gtk_text_buffer_create_mark (buffer, NULL, &match_start, TRUE);
            view->priv->last_found_end =
                gtk_text_buffer_create_mark (buffer, NULL, &match_end, TRUE);
        }
        else {
            gtk_text_buffer_move_mark (buffer, view->priv->last_found_start,
                                       &match_start);
            gtk_text_buffer_move_mark (buffer, view->priv->last_found_end,
                                       &match_end);
        }

        gtk_text_buffer_select_range (buffer, &match_end, &match_start);
        scroll_to_mark (view, insert);
    }
}


void
_moo_text_view_replace (MooTextView *view)
{
    GtkWidget *dialog;
    gboolean regex, case_sensitive, whole_words, from_cursor,
             backwards, selected, dont_prompt_on_replace;
    GtkTextIter sel_start, sel_end;
    MooTextSearchOptions options;
    const char *text, *replace_with;
    GtkTextMark *insert;
    GtkTextBuffer *buffer;

    g_return_if_fail (MOO_IS_TEXT_VIEW (view));

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));

    regex = _moo_text_search_params->regex;
    case_sensitive = _moo_text_search_params->case_sensitive;
    backwards = _moo_text_search_params->backwards;
    whole_words = _moo_text_search_params->whole_words;
    from_cursor = _moo_text_search_params->from_cursor;
    dont_prompt_on_replace = _moo_text_search_params->dont_prompt_on_replace;

    selected = FALSE;
    if (moo_prefs_get_bool (moo_edit_setting (MOO_EDIT_PREFS_SEARCH_SELECTED)) &&
        gtk_text_buffer_get_selection_bounds (buffer, &sel_start, &sel_end) &&
        ABS (gtk_text_iter_get_line (&sel_start) - gtk_text_iter_get_line (&sel_end) > 1))
            selected = TRUE;

    dialog = create_find_dialog (TRUE);
    set (dialog, regex, case_sensitive, whole_words,
         from_cursor, backwards, selected, dont_prompt_on_replace);

    if (gtk_text_buffer_get_selection_bounds (buffer, &sel_start, &sel_end) &&
        gtk_text_iter_get_line (&sel_start) == gtk_text_iter_get_line (&sel_end))
    {
        char *selection = gtk_text_buffer_get_text (buffer, &sel_start, &sel_end, TRUE);
        set_text (dialog, selection);
        g_free (selection);
    }
    else if (_moo_text_search_params->text)
    {
        set_text (dialog, _moo_text_search_params->text);
    }

    if (_moo_text_search_params->replace_with)
        set_replace_with (dialog, _moo_text_search_params->replace_with);

    gtk_window_set_transient_for (GTK_WINDOW (dialog),
                                  GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (view))));

    if (gtk_dialog_run (GTK_DIALOG (dialog)) != GTK_RESPONSE_OK)
    {
        gtk_widget_destroy (dialog);
        return;
    }

    get (dialog, &regex, &case_sensitive, &whole_words,
         &from_cursor, &backwards, &selected, &dont_prompt_on_replace);

    if (selected)
    {
        g_warning ("%s: searching in selected not imlemented\n", G_STRLOC);
        return;
    }

    _moo_text_search_params->regex = regex;
    _moo_text_search_params->case_sensitive = case_sensitive;
    _moo_text_search_params->backwards = backwards;
    _moo_text_search_params->whole_words = whole_words;
    _moo_text_search_params->from_cursor = from_cursor;
    _moo_text_search_params->dont_prompt_on_replace = dont_prompt_on_replace;

    _moo_text_search_params->last_search_stamp++;
    view->priv->last_search_stamp = _moo_text_search_params->last_search_stamp;

    g_free (_moo_text_search_params->text);
    _moo_text_search_params->text = g_strdup (get_text (dialog));
    text = _moo_text_search_params->text;
    g_free (_moo_text_search_params->replace_with);
    _moo_text_search_params->replace_with = g_strdup (get_replace_with (dialog));
    replace_with = _moo_text_search_params->replace_with;
    gtk_widget_destroy (dialog);

    if (text && text[0])
        moo_history_list_add (_moo_text_search_params->text_to_find_history, text);
    if (replace_with && replace_with[0])
        moo_history_list_add (_moo_text_search_params->replacement_history, replace_with);

    options = 0;
    if (regex) options |= MOO_TEXT_SEARCH_REGEX;
    if (backwards) options |= MOO_TEXT_SEARCH_BACKWARDS;
    if (!case_sensitive) options |= MOO_TEXT_SEARCH_CASE_INSENSITIVE;

    insert = gtk_text_buffer_get_insert (buffer);

    G_STMT_START {
        GtkTextIter start, limit;
        gboolean result;
        GError *err = NULL;
        PromptFuncData data = {view, NULL, MOO_TEXT_REPLACE};

        if (from_cursor) {
            gtk_text_buffer_get_iter_at_mark (buffer, &start, insert);
        }
        else {
            if (backwards)
                gtk_text_buffer_get_end_iter (buffer, &start);
            else
                gtk_text_buffer_get_start_iter (buffer, &start);
        }

        if (backwards)
            gtk_text_buffer_get_start_iter (buffer, &limit);
        else
            gtk_text_buffer_get_end_iter (buffer, &limit);

        if (!dont_prompt_on_replace) {
            result = moo_text_replace_all_interactive (&start, &limit, text,
                                                       replace_with,
                                                       options, &err,
                                                       prompt_on_replace_func,
                                                       &data);
        }
        else
            result = moo_text_replace_all_interactive (&start, &limit, text,
                                                       replace_with, options,
                                                       &err, moo_text_replace_func_replace_all,
                                                       NULL);

        g_return_if_fail (result != MOO_TEXT_REPLACE_INVALID_ARGS);

        if (result == MOO_TEXT_REPLACE_REGEX_ERROR || err) {
            moo_text_regex_error_dialog (view, err);
            if (err) g_error_free (err);
            return;
        }

        if (!from_cursor || data.response == MOO_TEXT_REPLACE_STOP) {
            if (data.dialog) gtk_widget_destroy (data.dialog);
            moo_text_replaced_n_dialog (view, result);
            return;
        }

        if ((backwards && gtk_text_iter_is_end (&start)) ||
            (!backwards && gtk_text_iter_is_start (&start)))
        {
            if (data.dialog) gtk_widget_destroy (data.dialog);
            moo_text_replaced_n_dialog (view, result);
            return;
        }

        if (!moo_text_search_from_beginning_dialog (view, backwards))
        {
            if (data.dialog) gtk_widget_destroy (data.dialog);
            moo_text_replaced_n_dialog (view, result);
            return;
        }

        {
            int result2;

            if (backwards)
                gtk_text_buffer_get_end_iter (buffer, &start);
            else
                gtk_text_buffer_get_start_iter (buffer, &start);

            gtk_text_buffer_get_iter_at_mark (buffer, &limit, insert);

            if (!dont_prompt_on_replace)
                result2 = moo_text_replace_all_interactive (&start, &limit, text,
                                                            replace_with,
                                                            options, &err,
                                                            prompt_on_replace_func,
                                                            &data);
            else
                result2 = moo_text_replace_all_interactive (&start, &limit, text,
                                                            replace_with, options, &err,
                                                            moo_text_replace_func_replace_all,
                                                            NULL);

            g_return_if_fail (result2 >= 0);

            if (data.dialog) gtk_widget_destroy (data.dialog);
            moo_text_replaced_n_dialog (view, result + result2);
        }
    } G_STMT_END;
}


static MooTextReplaceResponseType prompt_on_replace_func
                                            (G_GNUC_UNUSED const char         *text,
                                             G_GNUC_UNUSED EggRegex           *regex,
                                             G_GNUC_UNUSED const char         *replacement,
                                             GtkTextIter        *to_replace_start,
                                             GtkTextIter        *to_replace_end,
                                             gpointer            d)
{
    PromptFuncData *data = (PromptFuncData*) d;
    GtkTextBuffer *buffer;
    int response;

    buffer = gtk_text_iter_get_buffer (to_replace_end);
    gtk_text_buffer_select_range (buffer, to_replace_end, to_replace_start);
    scroll_to_mark (data->view, gtk_text_buffer_get_insert (buffer));

    if (!data->dialog)
        data->dialog = moo_text_prompt_on_replace_dialog (data->view);

    response = gtk_dialog_run (GTK_DIALOG (data->dialog));

    if (response == GTK_RESPONSE_DELETE_EVENT ||
        response == GTK_RESPONSE_CANCEL)
            data->response = MOO_TEXT_REPLACE_STOP;
    else
        data->response = (MooTextReplaceResponseType)response;

    return data->response;
}


#define GET_WIDGET(name)                                            \
    GtkWidget *name = moo_glade_xml_get_widget (xml, #name);
#define GET_TOGGLE_BUTTON(name)                                     \
    GtkToggleButton *name = moo_glade_xml_get_widget (xml, #name);

static void          set                (GtkWidget  *dialog,
                                         gboolean    regex,
                                         gboolean    casesensitive,
                                         gboolean    whole_words,
                                         gboolean    fromcursor,
                                         gboolean    backwards,
                                         gboolean    selected,
                                         gboolean    dontpromptonreplace)
{
    MooGladeXML *xml = g_object_get_data (G_OBJECT (dialog), "moo-dialog-xml");

    GET_TOGGLE_BUTTON (regular_expression);
    GET_TOGGLE_BUTTON (case_sensitive);
    GET_TOGGLE_BUTTON (whole_words_only);
    GET_TOGGLE_BUTTON (from_cursor);
    GET_TOGGLE_BUTTON (find_backwards);
    GET_TOGGLE_BUTTON (selected_text);
    GET_TOGGLE_BUTTON (dont_prompt_on_replace);

    gtk_toggle_button_set_active (regular_expression, regex);
    gtk_toggle_button_set_active (case_sensitive, casesensitive);
    gtk_toggle_button_set_active (whole_words_only, whole_words);
    gtk_toggle_button_set_active (from_cursor, fromcursor);
    gtk_toggle_button_set_active (find_backwards, backwards);
    gtk_toggle_button_set_active (selected_text, selected);
    gtk_toggle_button_set_active (dont_prompt_on_replace, dontpromptonreplace);
}

static void          get                (GtkWidget  *dialog,
                                         gboolean   *regex,
                                         gboolean   *casesensitive,
                                         gboolean   *whole_words,
                                         gboolean   *fromcursor,
                                         gboolean   *backwards,
                                         gboolean   *selected,
                                         gboolean   *dontpromptonreplace)
{
    MooGladeXML *xml = g_object_get_data (G_OBJECT (dialog), "moo-dialog-xml");

    GET_TOGGLE_BUTTON (regular_expression);
    GET_TOGGLE_BUTTON (case_sensitive);
    GET_TOGGLE_BUTTON (whole_words_only);
    GET_TOGGLE_BUTTON (from_cursor);
    GET_TOGGLE_BUTTON (find_backwards);
    GET_TOGGLE_BUTTON (selected_text);
    GET_TOGGLE_BUTTON (dont_prompt_on_replace);

    if (regex) *regex = gtk_toggle_button_get_active (regular_expression);
    if (casesensitive) *casesensitive = gtk_toggle_button_get_active (case_sensitive);
    if (whole_words) *whole_words = gtk_toggle_button_get_active (whole_words_only);
    if (fromcursor) *fromcursor = gtk_toggle_button_get_active (from_cursor);
    if (backwards) *backwards = gtk_toggle_button_get_active (find_backwards);
    if (selected) *selected = gtk_toggle_button_get_active (selected_text);
    if (dontpromptonreplace) *dontpromptonreplace = gtk_toggle_button_get_active (dont_prompt_on_replace);
}

static void          set_text           (GtkWidget  *dialog,
                                         const char *text)
{
    MooGladeXML *xml = g_object_get_data (G_OBJECT (dialog), "moo-dialog-xml");
    GET_WIDGET (text_to_find);
    moo_combo_entry_set_text (MOO_COMBO (text_to_find), text);
    moo_combo_select_region (MOO_COMBO (text_to_find), 0, -1);
}

static void          set_replace_with   (GtkWidget  *dialog,
                                         const char *text)
{
    MooGladeXML *xml = g_object_get_data (G_OBJECT (dialog), "moo-dialog-xml");
    GET_WIDGET (replacement_text);
    moo_combo_entry_set_text (MOO_COMBO (replacement_text), text);
}

static const char   *get_text           (GtkWidget *dialog)
{
    MooGladeXML *xml = g_object_get_data (G_OBJECT (dialog), "moo-dialog-xml");
    GET_WIDGET (text_to_find);
    return moo_combo_entry_get_text (MOO_COMBO (text_to_find));
}

static const char   *get_replace_with   (GtkWidget *dialog)
{
    MooGladeXML *xml = g_object_get_data (G_OBJECT (dialog), "moo-dialog-xml");
    GET_WIDGET (replacement_text);
    return moo_combo_entry_get_text (MOO_COMBO (replacement_text));
}
