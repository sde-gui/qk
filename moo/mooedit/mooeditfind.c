/*
 *   mooedit/mooeditfind.c
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
#include "mooedit-private.h"
#include "mooeditsearch.h"
#include "mooeditprefs.h"
#include "mooeditdialogs.h"
#include "mooutils/moocompat.h"


/****************************************************************************/
/* Go to line
 */
GtkWidget *_moo_edit_create_go_to_line_dialog   (void); /* in mooeditfind-glade.c */

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


void         moo_edit_goto_line             (MooEdit            *edit,
                                             int                 line)
{
    GtkWidget *dialog;
    GtkTextBuffer *buf;
    int line_count;
    GtkTextIter iter;
    GtkRange *scale;
    GtkSpinButton *spin;
    int response;

    g_return_if_fail (MOO_IS_EDIT (edit));

    buf = edit->priv->text_buffer;
    line_count = gtk_text_buffer_get_line_count (buf);

    if (line < 0 || line >= line_count)
    {
        dialog = _moo_edit_create_go_to_line_dialog ();
#if GTK_CHECK_VERSION(2,6,0)
        gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog),
                                                 GTK_RESPONSE_OK,
                                                 GTK_RESPONSE_CANCEL,
                                                 -1);
#endif /* GTK_CHECK_VERSION(2,6,0) */

        gtk_text_buffer_get_iter_at_mark (buf, &iter, gtk_text_buffer_get_insert (buf));
        line = gtk_text_iter_get_line (&iter);

        scale = GTK_RANGE (g_object_get_data (G_OBJECT (dialog), "scale"));
        gtk_range_set_range (scale, 1, line_count + 1);
        gtk_range_set_value (scale, line + 1);

        spin = GTK_SPIN_BUTTON (g_object_get_data (G_OBJECT (dialog), "spin"));
        gtk_spin_button_set_range (spin, 1, line_count);
        gtk_spin_button_set_value (spin, line + 1);
        gtk_editable_select_region (GTK_EDITABLE (spin), 0, -1);

        g_signal_connect (scale, "value-changed", G_CALLBACK (update_spin_value), spin);
        g_signal_connect (spin, "value-changed", G_CALLBACK (update_scale_value), scale);

        gtk_window_set_transient_for (GTK_WINDOW (dialog),
                                      GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (edit))));
        response = gtk_dialog_run (GTK_DIALOG (dialog));
        if (response != GTK_RESPONSE_OK) {
            gtk_widget_destroy (dialog);
            return;
        }

        line = (int)gtk_spin_button_get_value (spin) - 1;
        gtk_widget_destroy (dialog);
    }

    gtk_text_buffer_get_iter_at_line (buf, &iter, line);
    gtk_text_buffer_place_cursor (buf, &iter);
    gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW (edit),
                                        gtk_text_buffer_get_insert (buf));
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

GtkWidget *_moo_edit_create_find_dialog (gboolean replace);


typedef struct {
    MooEdit *edit;
    GtkWidget *dialog;
    MooEditReplaceResponseType response;
} PromptFuncData;

static MooEditReplaceResponseType prompt_on_replace_func
                                            (const char         *text,
                                             EggRegex           *regex,
                                             const char         *replacement,
                                             GtkTextIter        *to_replace_start,
                                             GtkTextIter        *to_replace_end,
                                             gpointer            data);


void        _moo_edit_find                  (MooEdit            *edit)
{
    GtkWidget *dialog;
    gboolean regex, case_sensitive, whole_words, from_cursor, backwards, selected;
    GtkTextIter sel_start, sel_end;
    int response;
    MooEditSearchOptions options;
    const char *text;
    GtkTextMark *insert;
    GtkTextBuffer *buffer;

    g_return_if_fail (MOO_IS_EDIT (edit));

    regex = _moo_edit_search_params->regex;
    case_sensitive = _moo_edit_search_params->case_sensitive;
    backwards = _moo_edit_search_params->backwards;
    whole_words = _moo_edit_search_params->whole_words;
    from_cursor = _moo_edit_search_params->from_cursor;

    selected = FALSE;
    if (moo_prefs_get_bool (moo_edit_setting (MOO_EDIT_PREFS_SEARCH_SELECTED)) &&
        gtk_text_buffer_get_selection_bounds (edit->priv->text_buffer, &sel_start, &sel_end) &&
        ABS (gtk_text_iter_get_line (&sel_start) - gtk_text_iter_get_line (&sel_end) > 1))
            selected = TRUE;

    dialog = _moo_edit_create_find_dialog (FALSE);
    set (dialog, regex, case_sensitive, whole_words,
         from_cursor, backwards, selected, FALSE);
    if (_moo_edit_search_params->text)
        set_text (dialog, _moo_edit_search_params->text);

    gtk_window_set_transient_for (GTK_WINDOW (dialog),
                                  GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (edit))));
    response = gtk_dialog_run (GTK_DIALOG (dialog));
    if (response != GTK_RESPONSE_OK) {
        gtk_widget_destroy (dialog);
        return;
    }

    get (dialog, &regex, &case_sensitive, &whole_words,
         &from_cursor, &backwards, &selected, NULL);

    if (selected) {
        g_warning ("%s: searching in selected not imlemented\n", G_STRLOC);
        gtk_widget_destroy (dialog);
        return;
    }

    _moo_edit_search_params->regex = regex;
    _moo_edit_search_params->case_sensitive = case_sensitive;
    _moo_edit_search_params->backwards = backwards;
    _moo_edit_search_params->whole_words = whole_words;
    _moo_edit_search_params->from_cursor = from_cursor;

    _moo_edit_search_params->last_search_stamp++;
    edit->priv->last_search_stamp = _moo_edit_search_params->last_search_stamp;

    g_free (_moo_edit_search_params->text);
    _moo_edit_search_params->text = g_strdup (get_text (dialog));
    text = _moo_edit_search_params->text;
    gtk_widget_destroy (dialog);

    options = 0;
    if (regex) options |= MOO_EDIT_SEARCH_REGEX;
    if (backwards) options |= MOO_EDIT_SEARCH_BACKWARDS;
    if (!case_sensitive) options |= MOO_EDIT_SEARCH_CASE_INSENSITIVE;

    buffer = edit->priv->text_buffer;
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

        result = moo_edit_search (&start, &limit, text,
                                  &match_start, &match_end, options,
                                  &err);
        if (!result)
        {
            if (err) {
                moo_edit_regex_error_dialog (edit, err);
                g_error_free (err);
                return;
            }

            if (!from_cursor) {
                moo_edit_nothing_found_dialog (edit, text, regex);
                return;
            }
            if ((backwards && gtk_text_iter_is_end (&start)) ||
                (!backwards && gtk_text_iter_is_start (&start)))
            {
                moo_edit_nothing_found_dialog (edit, text, regex);
                return;
            }

            if (!moo_edit_search_from_beginning_dialog (edit, backwards))
            {
                return;
            }

            if (backwards)
                gtk_text_buffer_get_end_iter (buffer, &start);
            else
                gtk_text_buffer_get_start_iter (buffer, &start);

            limit = search_start;

            result = moo_edit_search (&start, &limit, text,
                                      &match_start, &match_end, options,
                                      NULL);

            if (!result) {
                moo_edit_nothing_found_dialog (edit, text, regex);
                return;
            }
        }

        if (backwards)
            gtk_text_iter_order (&match_end, &match_start);

        if (!edit->priv->last_found_start) {
            edit->priv->last_found_start =
                gtk_text_buffer_create_mark (buffer, NULL, &match_start, TRUE);
            edit->priv->last_found_end =
                gtk_text_buffer_create_mark (buffer, NULL, &match_end, TRUE);
        }
        else {
            gtk_text_buffer_move_mark (buffer, edit->priv->last_found_start,
                                       &match_start);
            gtk_text_buffer_move_mark (buffer, edit->priv->last_found_end,
                                       &match_end);
        }

        gtk_text_buffer_select_range (buffer, &match_end, &match_start);
        gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW (edit), insert);
    }
}


void        _moo_edit_find_next             (MooEdit            *edit)
{
    gboolean regex, case_sensitive, whole_words, backwards;
    MooEditSearchOptions options;
    const char *text;
    GtkTextMark *insert;
    GtkTextBuffer *buffer;

    g_return_if_fail (MOO_IS_EDIT (edit));

    if (_moo_edit_search_params->last_search_stamp < 0 ||
        !_moo_edit_search_params->text)
            return moo_edit_find (edit);

    regex = _moo_edit_search_params->regex;
    case_sensitive = _moo_edit_search_params->case_sensitive;
    backwards = _moo_edit_search_params->backwards;
    whole_words = _moo_edit_search_params->whole_words;

    edit->priv->last_search_stamp = _moo_edit_search_params->last_search_stamp;

    text = _moo_edit_search_params->text;

    options = 0;
    if (regex) options |= MOO_EDIT_SEARCH_REGEX;
    if (backwards) options |= MOO_EDIT_SEARCH_BACKWARDS;
    if (!case_sensitive) options |= MOO_EDIT_SEARCH_CASE_INSENSITIVE;

    buffer = edit->priv->text_buffer;
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

        result = moo_edit_search (&start, &limit, text,
                                  &match_start, &match_end, options,
                                  NULL);
        if (!result)
        {
            if ((backwards && gtk_text_iter_is_end (&start)) ||
                (!backwards && gtk_text_iter_is_start (&start)))
            {
                moo_edit_nothing_found_dialog (edit, text, regex);
                return;
            }

            if (!moo_edit_search_from_beginning_dialog (edit, backwards))
            {
                return;
            }

            if (backwards)
                gtk_text_buffer_get_end_iter (buffer, &start);
            else
                gtk_text_buffer_get_start_iter (buffer, &start);

            limit = search_start;

            result = moo_edit_search (&start, &search_start, text,
                                      &match_start, &match_end, options,
                                      NULL);

            if (!result) {
                moo_edit_nothing_found_dialog (edit, text, regex);
                return;
            }
        }

        if (backwards)
            gtk_text_iter_order (&match_end, &match_start);

        if (!edit->priv->last_found_start) {
            edit->priv->last_found_start =
                gtk_text_buffer_create_mark (buffer, NULL, &match_start, TRUE);
            edit->priv->last_found_end =
                gtk_text_buffer_create_mark (buffer, NULL, &match_end, TRUE);
        }
        else {
            gtk_text_buffer_move_mark (buffer, edit->priv->last_found_start,
                                       &match_start);
            gtk_text_buffer_move_mark (buffer, edit->priv->last_found_end,
                                       &match_end);
        }

        gtk_text_buffer_select_range (buffer, &match_end, &match_start);
        gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW (edit), insert);
    }
}


void        _moo_edit_find_previous         (MooEdit            *edit)
{
    gboolean regex, case_sensitive, whole_words, backwards;
    MooEditSearchOptions options;
    const char *text;
    GtkTextMark *insert;
    GtkTextBuffer *buffer;

    g_return_if_fail (MOO_IS_EDIT (edit));

    if (_moo_edit_search_params->last_search_stamp < 0 ||
        !_moo_edit_search_params->text)
            return moo_edit_find (edit);

    regex = _moo_edit_search_params->regex;
    case_sensitive = _moo_edit_search_params->case_sensitive;
    backwards = _moo_edit_search_params->backwards;
    whole_words = _moo_edit_search_params->whole_words;

    edit->priv->last_search_stamp = _moo_edit_search_params->last_search_stamp;

    text = _moo_edit_search_params->text;

    options = 0;
    if (regex) options |= MOO_EDIT_SEARCH_REGEX;
    if (!backwards) options |= MOO_EDIT_SEARCH_BACKWARDS;
    if (!case_sensitive) options |= MOO_EDIT_SEARCH_CASE_INSENSITIVE;

    buffer = edit->priv->text_buffer;
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

        result = moo_edit_search (&start, &limit, text,
                                  &match_start, &match_end, options,
                                  NULL);

        if (!result)
        {
            if ((!backwards && gtk_text_iter_is_end (&start)) ||
                (backwards && gtk_text_iter_is_start (&start)))
            {
                moo_edit_nothing_found_dialog (edit, text, regex);
                return;
            }

            if (!moo_edit_search_from_beginning_dialog (edit, !backwards))
            {
                return;
            }

            if (!backwards)
                gtk_text_buffer_get_end_iter (buffer, &start);
            else
                gtk_text_buffer_get_start_iter (buffer, &start);

            limit = search_start;

            result = moo_edit_search (&start, &search_start, text,
                                      &match_start, &match_end, options,
                                      NULL);

            if (!result) {
                moo_edit_nothing_found_dialog (edit, text, regex);
                return;
            }
        }

        if (!backwards)
            gtk_text_iter_order (&match_end, &match_start);

        if (!edit->priv->last_found_start) {
            edit->priv->last_found_start =
                gtk_text_buffer_create_mark (buffer, NULL, &match_start, TRUE);
            edit->priv->last_found_end =
                gtk_text_buffer_create_mark (buffer, NULL, &match_end, TRUE);
        }
        else {
            gtk_text_buffer_move_mark (buffer, edit->priv->last_found_start,
                                       &match_start);
            gtk_text_buffer_move_mark (buffer, edit->priv->last_found_end,
                                       &match_end);
        }

        gtk_text_buffer_select_range (buffer, &match_end, &match_start);
        gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW (edit), insert);
    }
}


void        _moo_edit_replace               (MooEdit            *edit)
{
    GtkWidget *dialog;
    gboolean regex, case_sensitive, whole_words, from_cursor,
             backwards, selected, dont_prompt_on_replace;
    GtkTextIter sel_start, sel_end;
    int response;
    MooEditSearchOptions options;
    const char *text, *replace_with;
    GtkTextMark *insert;
    GtkTextBuffer *buffer;

    g_return_if_fail (MOO_IS_EDIT (edit));

    regex = _moo_edit_search_params->regex;
    case_sensitive = _moo_edit_search_params->case_sensitive;
    backwards = _moo_edit_search_params->backwards;
    whole_words = _moo_edit_search_params->whole_words;
    from_cursor = _moo_edit_search_params->from_cursor;
    dont_prompt_on_replace = _moo_edit_search_params->dont_prompt_on_replace;

    selected = FALSE;
    if (moo_prefs_get_bool (moo_edit_setting (MOO_EDIT_PREFS_SEARCH_SELECTED)) &&
        gtk_text_buffer_get_selection_bounds (edit->priv->text_buffer, &sel_start, &sel_end) &&
        ABS (gtk_text_iter_get_line (&sel_start) - gtk_text_iter_get_line (&sel_end) > 1))
            selected = TRUE;

    dialog = _moo_edit_create_find_dialog (TRUE);
    set (dialog, regex, case_sensitive, whole_words,
         from_cursor, backwards, selected, dont_prompt_on_replace);
    if (_moo_edit_search_params->text)
        set_text (dialog, _moo_edit_search_params->text);
    if (_moo_edit_search_params->replace_with)
        set_replace_with (dialog, _moo_edit_search_params->replace_with);

    gtk_window_set_transient_for (GTK_WINDOW (dialog),
                                  GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (edit))));
    response = gtk_dialog_run (GTK_DIALOG (dialog));
    if (response != GTK_RESPONSE_OK) {
        gtk_widget_destroy (dialog);
        return;
    }

    get (dialog, &regex, &case_sensitive, &whole_words,
         &from_cursor, &backwards, &selected, &dont_prompt_on_replace);

    if (selected) {
        g_warning ("%s: searching in selected not imlemented\n", G_STRLOC);
        return;
    }

    _moo_edit_search_params->regex = regex;
    _moo_edit_search_params->case_sensitive = case_sensitive;
    _moo_edit_search_params->backwards = backwards;
    _moo_edit_search_params->whole_words = whole_words;
    _moo_edit_search_params->from_cursor = from_cursor;
    _moo_edit_search_params->dont_prompt_on_replace = dont_prompt_on_replace;

    _moo_edit_search_params->last_search_stamp++;
    edit->priv->last_search_stamp = _moo_edit_search_params->last_search_stamp;

    g_free (_moo_edit_search_params->text);
    _moo_edit_search_params->text = g_strdup (get_text (dialog));
    text = _moo_edit_search_params->text;
    g_free (_moo_edit_search_params->replace_with);
    _moo_edit_search_params->replace_with = g_strdup (get_replace_with (dialog));
    replace_with = _moo_edit_search_params->replace_with;
    gtk_widget_destroy (dialog);

    options = 0;
    if (regex) options |= MOO_EDIT_SEARCH_REGEX;
    if (backwards) options |= MOO_EDIT_SEARCH_BACKWARDS;
    if (!case_sensitive) options |= MOO_EDIT_SEARCH_CASE_INSENSITIVE;

    buffer = edit->priv->text_buffer;
    insert = gtk_text_buffer_get_insert (buffer);

    {
        GtkTextIter start, limit;
        gboolean result;
        GError *err = NULL;
        PromptFuncData data = {edit, NULL, MOO_EDIT_REPLACE};

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
            result = moo_edit_replace_all_interactive (&start, &limit, text,
                                                       replace_with,
                                                       options, &err,
                                                       prompt_on_replace_func,
                                                       &data);
        }
        else
            result = moo_edit_replace_all_interactive (&start, &limit, text,
                                                       replace_with, options,
                                                       &err, moo_edit_replace_func_replace_all,
                                                       NULL);

        g_return_if_fail (result != MOO_EDIT_REPLACE_INVALID_ARGS);

        if (result == MOO_EDIT_REPLACE_REGEX_ERROR || err) {
            moo_edit_regex_error_dialog (edit, err);
            if (err) g_error_free (err);
            return;
        }

        if (!from_cursor || data.response == MOO_EDIT_REPLACE_STOP) {
            if (data.dialog) gtk_widget_destroy (data.dialog);
            moo_edit_replaced_n_dialog (edit, result);
            return;
        }

        if ((backwards && gtk_text_iter_is_end (&start)) ||
            (!backwards && gtk_text_iter_is_start (&start)))
        {
            if (data.dialog) gtk_widget_destroy (data.dialog);
            moo_edit_replaced_n_dialog (edit, result);
            return;
        }

        if (!moo_edit_search_from_beginning_dialog (edit, backwards))
        {
            if (data.dialog) gtk_widget_destroy (data.dialog);
            moo_edit_replaced_n_dialog (edit, result);
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
                result2 = moo_edit_replace_all_interactive (&start, &limit, text,
                                                            replace_with,
                                                            options, &err,
                                                            prompt_on_replace_func,
                                                            &data);
            else
                result2 = moo_edit_replace_all_interactive (&start, &limit, text,
                                                            replace_with, options, &err,
                                                            moo_edit_replace_func_replace_all,
                                                            NULL);

            g_return_if_fail (result2 >= 0);

            if (data.dialog) gtk_widget_destroy (data.dialog);
            moo_edit_replaced_n_dialog (edit, result + result2);
        }
    }
}


static MooEditReplaceResponseType prompt_on_replace_func
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
    gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW (data->edit),
                                        gtk_text_buffer_get_insert (buffer));

    if (!data->dialog)
        data->dialog = moo_edit_prompt_on_replace_dialog (data->edit);

    response = gtk_dialog_run (GTK_DIALOG (data->dialog));

    if (response == GTK_RESPONSE_DELETE_EVENT ||
        response == GTK_RESPONSE_CANCEL)
            data->response = MOO_EDIT_REPLACE_STOP;
    else
        data->response = (MooEditReplaceResponseType)response;

    return data->response;
}


#define GET_WIDGET(name)                                                                        \
    GtkWidget *name = GTK_WIDGET (g_object_get_data (G_OBJECT (dialog), #name));                \
    g_assert (name != NULL);
#define GET_TOGGLE_BUTTON(name)                                                                 \
    GtkToggleButton *name = GTK_TOGGLE_BUTTON (g_object_get_data (G_OBJECT (dialog), #name));   \
    g_assert (name != NULL);

static void          set                (GtkWidget  *dialog,
                                         gboolean    regex,
                                         gboolean    casesensitive,
                                         gboolean    whole_words,
                                         gboolean    fromcursor,
                                         gboolean    backwards,
                                         gboolean    selected,
                                         gboolean    dontpromptonreplace)
{
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
    GET_WIDGET (text_to_find);
    gtk_entry_set_text (GTK_ENTRY (text_to_find), text);
    gtk_editable_select_region (GTK_EDITABLE (text_to_find), 0, -1);
}

static void          set_replace_with   (GtkWidget  *dialog,
                                         const char *text)
{
    GET_WIDGET (replacement_text);
    gtk_entry_set_text (GTK_ENTRY (replacement_text), text);
}

static const char   *get_text           (GtkWidget *dialog)
{
    GET_WIDGET (text_to_find);
    return gtk_entry_get_text (GTK_ENTRY (text_to_find));
}

static const char   *get_replace_with   (GtkWidget *dialog)
{
    GET_WIDGET (replacement_text);
    return gtk_entry_get_text (GTK_ENTRY (replacement_text));
}
