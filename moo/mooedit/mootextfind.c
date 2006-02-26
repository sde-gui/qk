/*
 *   mootextfind.c
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

#include "mooedit/mootextfind.h"
#include "mooedit/mootextfind-glade.h"
#include "mooedit/mootextgotoline-glade.h"
#include "mooedit/mootextview.h"
#include "mooedit/mooeditdialogs.h"
#include "mooedit/mootextsearch.h"
#include "mooutils/moohistoryentry.h"
#include "mooutils/mooentry.h"
#include "mooutils/moocompat.h"
#include "mooutils/moodialogs.h"
#include <gtk/gtk.h>


static char *last_search;
static EggRegex *last_regex;
static MooFindFlags last_search_flags;
static MooHistoryList *search_history;
static MooHistoryList *replace_history;


static GObject *moo_find_constructor    (GType           type,
                                         guint           n_props,
                                         GObjectConstructParam *props);
static void     moo_find_set_property   (GObject        *object,
                                         guint           prop_id,
                                         const GValue   *value,
                                         GParamSpec     *pspec);
static void     moo_find_get_property   (GObject        *object,
                                         guint           prop_id,
                                         GValue         *value,
                                         GParamSpec     *pspec);
static void     moo_find_finalize       (GObject        *object);


G_DEFINE_TYPE(MooFind, moo_find, GTK_TYPE_DIALOG)


enum {
    PROP_0,
    PROP_REPLACE
};


static void
moo_find_class_init (MooFindClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->constructor = moo_find_constructor;
    gobject_class->set_property = moo_find_set_property;
    gobject_class->get_property = moo_find_get_property;
    gobject_class->finalize = moo_find_finalize;

    search_history = moo_history_list_new ("MooFind");
    replace_history = moo_history_list_new ("MooReplace");
    last_search_flags = MOO_FIND_CASELESS;

    g_object_class_install_property (gobject_class,
                                     PROP_REPLACE,
                                     g_param_spec_boolean ("replace",
                                             "replace",
                                             "replace",
                                             TRUE,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
}


static void
moo_find_init (MooFind *find)
{
    GtkWidget *vbox;

    find->xml = moo_glade_xml_new_empty ();
    moo_glade_xml_map_id (find->xml, "search_entry", MOO_TYPE_HISTORY_ENTRY);
    moo_glade_xml_map_id (find->xml, "replace_entry", MOO_TYPE_HISTORY_ENTRY);

    if (!moo_glade_xml_parse_memory (find->xml, MOO_TEXT_FIND_GLADE_UI, -1, "vbox"))
    {
        g_object_unref (find->xml);
        find->xml = NULL;
        g_return_if_reached ();
    }

    vbox = moo_glade_xml_get_widget (find->xml, "vbox");

    gtk_container_add (GTK_CONTAINER (GTK_DIALOG(find)->vbox), vbox);
    gtk_dialog_set_has_separator (GTK_DIALOG (find), FALSE);

    moo_history_entry_set_list (moo_glade_xml_get_widget (find->xml, "search_entry"), search_history);
    moo_history_entry_set_list (moo_glade_xml_get_widget (find->xml, "replace_entry"), replace_history);
}


static GObject*
moo_find_constructor (GType           type,
                      guint           n_props,
                      GObjectConstructParam *props)
{
    GObject *object;
    MooFind *find;
    gboolean use_replace;
    const char *stock_id, *title;

    object = G_OBJECT_CLASS (moo_find_parent_class)->constructor (type, n_props, props);
    find = MOO_FIND (object);

    if (find->replace)
    {
        use_replace = TRUE;
        stock_id = GTK_STOCK_FIND_AND_REPLACE;
        title = "Replace";
    }
    else
    {
        use_replace = FALSE;
        stock_id = GTK_STOCK_FIND;
        title = "Find";
    }

    gtk_widget_set_sensitive (moo_glade_xml_get_widget (find->xml, "backwards"), !use_replace);
    g_object_set (moo_glade_xml_get_widget (find->xml, "replace_frame"),
                  "visible", use_replace, NULL);
    g_object_set (moo_glade_xml_get_widget (find->xml, "dont_prompt"),
                  "visible", use_replace, NULL);

    gtk_window_set_title (GTK_WINDOW (find), title);
    gtk_dialog_add_buttons (GTK_DIALOG (find),
                            GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                            stock_id, GTK_RESPONSE_OK,
                            NULL);
    gtk_dialog_set_alternative_button_order (GTK_DIALOG (find), GTK_RESPONSE_OK,
                                             GTK_STOCK_CANCEL, -1);
    gtk_dialog_set_default_response (GTK_DIALOG (find), GTK_RESPONSE_OK);

    return object;
}


static void
moo_find_set_property (GObject        *object,
                       guint           prop_id,
                       const GValue   *value,
                       GParamSpec     *pspec)
{
    MooFind *find = MOO_FIND (object);

    switch (prop_id)
    {
        case PROP_REPLACE:
            find->replace = g_value_get_boolean (value) ? TRUE : FALSE;
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}


static void
moo_find_get_property (GObject        *object,
                       guint           prop_id,
                       GValue         *value,
                       GParamSpec     *pspec)
{
    MooFind *find = MOO_FIND (object);

    switch (prop_id)
    {
        case PROP_REPLACE:
            g_value_set_boolean (value, find->replace ? TRUE : FALSE);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}


static void
moo_find_finalize (GObject *object)
{
    MooFind *find = MOO_FIND (object);

    g_object_unref (find->xml);
    egg_regex_unref (find->regex);

    G_OBJECT_CLASS(moo_find_parent_class)->finalize (object);
}


GtkWidget*
moo_find_new (gboolean replace)
{
    return g_object_new (MOO_TYPE_FIND, "replace", replace, NULL);
}


void
moo_find_setup (MooFind        *find,
                GtkTextView    *view)
{
    GtkTextBuffer *buffer;
    GtkTextIter sel_start, sel_end;
    MooCombo *search_entry;

    g_return_if_fail (MOO_IS_FIND (find));
    g_return_if_fail (GTK_IS_TEXT_VIEW (view));

    moo_position_window (GTK_WIDGET (find), GTK_WIDGET (view), FALSE, FALSE, 0, 0);

    buffer = gtk_text_view_get_buffer (view);
    search_entry = moo_glade_xml_get_widget (find->xml, "search_entry");

    if (gtk_text_buffer_get_selection_bounds (buffer, &sel_start, &sel_end) &&
        gtk_text_iter_get_line (&sel_start) == gtk_text_iter_get_line (&sel_end))
    {
        char *selection = gtk_text_buffer_get_text (buffer, &sel_start, &sel_end, TRUE);
        gtk_entry_set_text (GTK_ENTRY (search_entry->entry), selection);
        g_free (selection);
    }
    else if (last_search)
    {
        gtk_entry_set_text (GTK_ENTRY (search_entry->entry), last_search);
    }

    moo_entry_clear_undo (MOO_ENTRY (search_entry->entry));

    moo_find_set_flags (find, last_search_flags);
}


gboolean
moo_find_run (MooFind *find)
{
    MooCombo *search_entry, *replace_entry;
    g_return_val_if_fail (MOO_IS_FIND (find), FALSE);

    search_entry = moo_glade_xml_get_widget (find->xml, "search_entry");
    replace_entry = moo_glade_xml_get_widget (find->xml, "replace_entry");

    egg_regex_unref (find->regex);
    find->regex = NULL;

    while (TRUE)
    {
        const char *search_for, *replace_with;
        MooFindFlags flags;

        if (gtk_dialog_run (GTK_DIALOG (find)) != GTK_RESPONSE_OK)
            return FALSE;

        search_for = moo_combo_entry_get_text (search_entry);
        replace_with = moo_combo_entry_get_text (replace_entry);

        if (!search_for[0])
            return FALSE;

        flags = moo_find_get_flags (find);

        if (flags & MOO_FIND_REGEX)
        {
            EggRegex *regex;
            EggRegexCompileFlags re_flags = 0;
            GError *error = NULL;

            if (flags & MOO_FIND_CASELESS)
                re_flags |= EGG_REGEX_CASELESS;

            regex = egg_regex_new (search_for, re_flags, 0, &error);

            if (error)
            {
                moo_text_regex_error_dialog (GTK_WIDGET (find), error);
                g_error_free (error);
                egg_regex_unref (regex);
                continue;
            }

            find->regex = regex;

            egg_regex_optimize (regex, &error);

            if (error)
            {
                g_warning ("%s", error->message);
                g_error_free (error);
            }
        }

        if (find->replace && !(flags & MOO_FIND_REPL_LITERAL))
        {
            GError *error = NULL;

            if (!egg_regex_check_replacement (replace_with, NULL, &error))
            {
                moo_text_regex_error_dialog (GTK_WIDGET (find), error);
                g_error_free (error);
                egg_regex_unref (find->regex);
                find->regex = NULL;
                continue;
            }
        }

        last_search_flags = flags;
        g_free (last_search);
        last_search = g_strdup (search_for);
        egg_regex_unref (last_regex);
        last_regex = egg_regex_ref (find->regex);

        moo_history_list_add (search_history, search_for);

        if (find->replace && replace_with[0])
            moo_history_list_add (replace_history, replace_with);

        return TRUE;
    }
}


MooFindFlags
moo_find_get_flags (MooFind *find)
{
    MooFindFlags flags = 0;

    g_return_val_if_fail (MOO_IS_FIND (find), 0);

    if (gtk_toggle_button_get_active (moo_glade_xml_get_widget (find->xml, "regex")))
        flags |= MOO_FIND_REGEX;
    if (gtk_toggle_button_get_active (moo_glade_xml_get_widget (find->xml, "repl_literal")))
        flags |= MOO_FIND_REPL_LITERAL;
    if (gtk_toggle_button_get_active (moo_glade_xml_get_widget (find->xml, "whole_words")))
        flags |= MOO_FIND_WHOLE_WORDS;
    if (gtk_toggle_button_get_active (moo_glade_xml_get_widget (find->xml, "from_cursor")))
        flags |= MOO_FIND_FROM_CURSOR;
    if (gtk_toggle_button_get_active (moo_glade_xml_get_widget (find->xml, "backwards")))
        flags |= MOO_FIND_BACKWARDS;
    if (gtk_toggle_button_get_active (moo_glade_xml_get_widget (find->xml, "selected")))
        flags |= MOO_FIND_IN_SELECTED;
    if (gtk_toggle_button_get_active (moo_glade_xml_get_widget (find->xml, "dont_prompt")))
        flags |= MOO_FIND_DONT_PROMPT;

    if (!gtk_toggle_button_get_active (moo_glade_xml_get_widget (find->xml, "case_sensitive")))
        flags |= MOO_FIND_CASELESS;

    return flags;
}


void
moo_find_set_flags (MooFind        *find,
                    MooFindFlags    flags)
{
    g_return_if_fail (MOO_IS_FIND (find));

    gtk_toggle_button_set_active (moo_glade_xml_get_widget (find->xml, "regex"),
                                  (flags & MOO_FIND_REGEX) ? TRUE : FALSE);
    gtk_toggle_button_set_active (moo_glade_xml_get_widget (find->xml, "repl_literal"),
                                  (flags & MOO_FIND_REPL_LITERAL) ? TRUE : FALSE);
    gtk_toggle_button_set_active (moo_glade_xml_get_widget (find->xml, "whole_words"),
                                  (flags & MOO_FIND_WHOLE_WORDS) ? TRUE : FALSE);
    gtk_toggle_button_set_active (moo_glade_xml_get_widget (find->xml, "from_cursor"),
                                  (flags & MOO_FIND_FROM_CURSOR) ? TRUE : FALSE);
    gtk_toggle_button_set_active (moo_glade_xml_get_widget (find->xml, "backwards"),
                                  (flags & MOO_FIND_BACKWARDS) ? TRUE : FALSE);
    gtk_toggle_button_set_active (moo_glade_xml_get_widget (find->xml, "selected"),
                                  (flags & MOO_FIND_IN_SELECTED) ? TRUE : FALSE);
    gtk_toggle_button_set_active (moo_glade_xml_get_widget (find->xml, "dont_prompt"),
                                  (flags & MOO_FIND_DONT_PROMPT) ? TRUE : FALSE);

    gtk_toggle_button_set_active (moo_glade_xml_get_widget (find->xml, "case_sensitive"),
                                  (flags & MOO_FIND_CASELESS) ? FALSE : TRUE);
}


char*
moo_find_get_text (MooFind *find)
{
    MooCombo *entry;
    g_return_val_if_fail (MOO_IS_FIND (find), NULL);
    entry = moo_glade_xml_get_widget (find->xml, "search_entry");
    return g_strdup (moo_combo_entry_get_text (entry));
}


EggRegex*
moo_find_get_regex (MooFind *find)
{
    g_return_val_if_fail (MOO_IS_FIND (find), NULL);
    return find->regex ? egg_regex_ref (find->regex) : NULL;
}


char*
moo_find_get_replacement (MooFind *find)
{
    MooCombo *entry;
    g_return_val_if_fail (MOO_IS_FIND (find), NULL);
    entry = moo_glade_xml_get_widget (find->xml, "replace_entry");
    return g_strdup (moo_combo_entry_get_text (entry));
}


static gboolean
do_find (const GtkTextIter *start,
         const GtkTextIter *end,
         MooFindFlags       flags,
         EggRegex          *regex,
         const char        *text,
         GtkTextIter       *match_start,
         GtkTextIter       *match_end)
{
    if (regex)
    {
        if (flags & MOO_FIND_BACKWARDS)
            return moo_text_search_regex_backward (start, end, regex, match_start,
                                                   match_end, NULL, NULL, NULL);
        else
            return moo_text_search_regex_forward (start, end, regex, match_start,
                                                  match_end, NULL, NULL, NULL);
    }
    else
    {
        MooTextSearchFlags search_flags = 0;

        if (flags & MOO_FIND_CASELESS)
            search_flags |= MOO_TEXT_SEARCH_CASELESS;
        if (flags & MOO_FIND_WHOLE_WORDS)
            search_flags |= MOO_TEXT_SEARCH_WHOLE_WORDS;

        if (flags & MOO_FIND_BACKWARDS)
            return moo_text_search_backward (start, text, search_flags,
                                             match_start, match_end, end);
        else
            return moo_text_search_forward (start, text, search_flags,
                                            match_start, match_end, end);
    }
}


static void
get_search_bounds (GtkTextBuffer *buffer,
                   MooFindFlags   flags,
                   GtkTextIter   *start,
                   GtkTextIter   *end)
{
    if (flags & MOO_FIND_FROM_CURSOR)
    {
        gtk_text_buffer_get_iter_at_mark (buffer, start,
                                          gtk_text_buffer_get_insert (buffer));
        if (flags & MOO_FIND_BACKWARDS)
            gtk_text_buffer_get_start_iter (buffer, end);
        else
            gtk_text_buffer_get_end_iter (buffer, end);
    }
    else if (flags & MOO_FIND_BACKWARDS)
    {
        gtk_text_buffer_get_end_iter (buffer, start);
        gtk_text_buffer_get_start_iter (buffer, end);
    }
    else
    {
        gtk_text_buffer_get_start_iter (buffer, start);
        gtk_text_buffer_get_end_iter (buffer, end);
    }
}

static gboolean
get_search_bounds2 (GtkTextBuffer *buffer,
                    MooFindFlags   flags,
                    GtkTextIter   *start,
                    GtkTextIter   *end)
{
    if (flags & MOO_FIND_FROM_CURSOR)
    {
        gtk_text_buffer_get_iter_at_mark (buffer, end,
                                          gtk_text_buffer_get_insert (buffer));

        if (flags & MOO_FIND_BACKWARDS)
            gtk_text_buffer_get_end_iter (buffer, start);
        else
            gtk_text_buffer_get_start_iter (buffer, start);

        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


void
moo_text_view_run_find (GtkTextView *view)
{
    GtkWidget *find;
    MooFindFlags flags;
    char *text;
    EggRegex *regex;
    GtkTextIter start, end;
    GtkTextIter match_start, match_end;
    GtkTextBuffer *buffer;
    gboolean found;

    g_return_if_fail (GTK_IS_TEXT_VIEW (view));

    find = moo_find_new (FALSE);
    moo_find_setup (MOO_FIND (find), view);

    if (!moo_find_run (MOO_FIND (find)))
    {
        gtk_widget_destroy (find);
        return;
    }

    flags = moo_find_get_flags (MOO_FIND (find));
    text = moo_find_get_text (MOO_FIND (find));
    regex = moo_find_get_regex (MOO_FIND (find));

    gtk_widget_destroy (find);
    buffer = gtk_text_view_get_buffer (view);

    get_search_bounds (buffer, flags, &start, &end);

    found = do_find (&start, &end, flags, regex, text, &match_start, &match_end);

    if (!found && get_search_bounds2 (buffer, flags, &start, &end))
        found = do_find (&start, &end, flags, regex, text, &match_start, &match_end);

    if (found)
    {
        gtk_text_buffer_select_range (buffer, &match_end, &match_start);
        gtk_text_view_scroll_to_mark (view, gtk_text_buffer_get_insert (buffer),
                                      0, FALSE, 0, 0);
    }
    else
    {
        GtkTextIter insert;
        gtk_text_buffer_get_iter_at_mark (buffer, &insert,
                                          gtk_text_buffer_get_insert (buffer));
        gtk_text_buffer_place_cursor (buffer, &insert);
    }

    g_free (text);
    egg_regex_unref (regex);
}


void
moo_text_view_run_find_next (GtkTextView *view)
{
    GtkTextBuffer *buffer;
    GtkTextIter sel_start, sel_end;
    GtkTextIter start, end;
    GtkTextIter match_start, match_end;
    gboolean found;
    gboolean has_selection;

    g_return_if_fail (GTK_IS_TEXT_VIEW (view));

    if (!last_search)
        return moo_text_view_run_find (view);

    buffer = gtk_text_view_get_buffer (view);

    has_selection = gtk_text_buffer_get_selection_bounds (buffer, &sel_start, &sel_end);

    start = sel_end;
    gtk_text_buffer_get_end_iter (buffer, &end);

    found = do_find (&start, &end, last_search_flags & ~MOO_FIND_BACKWARDS,
                     last_regex, last_search, &match_start, &match_end);

    if (found && !has_selection &&
        gtk_text_iter_equal (&match_start, &match_end) &&
        gtk_text_iter_equal (&match_start, &start))
    {
        if (!gtk_text_iter_forward_char (&start))
        {
            found = FALSE;
        }
        else
        {
            found = do_find (&start, &end, last_search_flags & ~MOO_FIND_BACKWARDS,
                              last_regex, last_search, &match_start, &match_end);;
        }
    }

    if (!found && !gtk_text_iter_is_start (&sel_start))
    {
        gtk_text_buffer_get_start_iter (buffer, &start);
        end = sel_start;
        found = do_find (&start, &end, last_search_flags & ~MOO_FIND_BACKWARDS,
                          last_regex, last_search, &match_start, &match_end);
    }

    if (found)
    {
        gtk_text_buffer_select_range (buffer, &match_end, &match_start);
        gtk_text_view_scroll_to_mark (view, gtk_text_buffer_get_insert (buffer),
                                      0, FALSE, 0, 0);
    }
    else
    {
        GtkTextIter insert;
        gtk_text_buffer_get_iter_at_mark (buffer, &insert,
                                          gtk_text_buffer_get_insert (buffer));
        gtk_text_buffer_place_cursor (buffer, &insert);
    }
}


void
moo_text_view_run_find_prev (GtkTextView *view)
{
    GtkTextBuffer *buffer;
    GtkTextIter sel_start, sel_end;
    GtkTextIter start, end;
    GtkTextIter match_start, match_end;
    gboolean found;
    gboolean has_selection;

    g_return_if_fail (GTK_IS_TEXT_VIEW (view));

    if (!last_search)
        return moo_text_view_run_find (view);

    buffer = gtk_text_view_get_buffer (view);

    has_selection = gtk_text_buffer_get_selection_bounds (buffer, &sel_start, &sel_end);

    start = sel_start;
    gtk_text_buffer_get_start_iter (buffer, &end);

    found = do_find (&start, &end, last_search_flags | MOO_FIND_BACKWARDS,
                     last_regex, last_search, &match_start, &match_end);

    if (found && !has_selection &&
        gtk_text_iter_equal (&match_start, &match_end) &&
        gtk_text_iter_equal (&match_start, &start))
    {
        if (!gtk_text_iter_backward_char (&start))
        {
            found = FALSE;
        }
        else
        {
            found = do_find (&start, &end, last_search_flags | MOO_FIND_BACKWARDS,
                             last_regex, last_search, &match_start, &match_end);;
        }
    }

    if (!found && !gtk_text_iter_is_end (&sel_end))
    {
        gtk_text_buffer_get_end_iter (buffer, &start);
        end = sel_end;
        found = do_find (&start, &end, last_search_flags | MOO_FIND_BACKWARDS,
                         last_regex, last_search, &match_start, &match_end);
    }

    if (found)
    {
        gtk_text_buffer_select_range (buffer, &match_start, &match_end);
        gtk_text_view_scroll_to_mark (view, gtk_text_buffer_get_insert (buffer),
                                      0, FALSE, 0, 0);
    }
    else
    {
        GtkTextIter insert;
        gtk_text_buffer_get_iter_at_mark (buffer, &insert,
                                          gtk_text_buffer_get_insert (buffer));
        gtk_text_buffer_place_cursor (buffer, &insert);
    }
}


static int
do_replace_silent (GtkTextIter       *start,
                   GtkTextIter       *end,
                   MooFindFlags       flags,
                   const char        *text,
                   EggRegex          *regex,
                   const char        *replacement)
{
    MooTextSearchFlags search_flags = 0;

    if (flags & MOO_FIND_CASELESS)
        search_flags |= MOO_TEXT_SEARCH_CASELESS;
    if (flags & MOO_FIND_WHOLE_WORDS)
        search_flags |= MOO_TEXT_SEARCH_WHOLE_WORDS;

    if (regex)
        return moo_text_replace_regex_all (start, end, regex, replacement,
                                           flags & MOO_FIND_REPL_LITERAL);
    else
        return moo_text_replace_all (start, end, text, replacement, search_flags);
}


static void
run_replace_silent (GtkTextView *view,
                    const char  *text,
                    EggRegex    *regex,
                    const char  *replacement,
                    MooFindFlags flags)
{
    GtkTextIter start, end;
    GtkTextBuffer *buffer;
    int replaced;

    buffer = gtk_text_view_get_buffer (view);
    get_search_bounds (buffer, flags, &start, &end);

    replaced = do_replace_silent (&start, &end, flags, text, regex, replacement);

    if (get_search_bounds2 (buffer, flags, &start, &end) &&
        moo_text_search_from_start_dialog (GTK_WIDGET (view), replaced))
    {
        int replaced2 = do_replace_silent (&start, &end, flags, text, regex, replacement);
        moo_text_replaced_n_dialog (GTK_WIDGET (view), replaced2);
    }
    else
    {
        moo_text_replaced_n_dialog (GTK_WIDGET (view), replaced);
    }
}


static MooTextReplaceResponse
replace_func (G_GNUC_UNUSED const char *text,
              G_GNUC_UNUSED EggRegex *regex,
              G_GNUC_UNUSED const char *replacement,
              const GtkTextIter  *to_replace_start,
              const GtkTextIter  *to_replace_end,
              gpointer            user_data)
{
    GtkTextBuffer *buffer;
    int response;

    struct {
        GtkTextView *view;
        GtkWidget *dialog;
        MooTextReplaceResponse response;
    } *data = user_data;

    buffer = gtk_text_view_get_buffer (data->view);
    gtk_text_buffer_select_range (buffer, to_replace_end, to_replace_start);
    gtk_text_view_scroll_to_mark (data->view, gtk_text_buffer_get_insert (buffer),
                                  0, FALSE, 0, 0);

    if (!data->dialog)
        data->dialog = moo_text_prompt_on_replace_dialog (GTK_WIDGET (data->view));

    response = gtk_dialog_run (GTK_DIALOG (data->dialog));

    switch (response)
    {
        case MOO_TEXT_REPLACE_SKIP:
        case MOO_TEXT_REPLACE_DO_REPLACE:
        case MOO_TEXT_REPLACE_ALL:
            data->response = response;
            break;

        default:
            data->response = MOO_TEXT_REPLACE_STOP;
            break;
    }

    return data->response;
}


static MooTextReplaceResponse
do_replace_interactive (GtkTextView       *view,
                        GtkTextIter       *start,
                        GtkTextIter       *end,
                        MooFindFlags       flags,
                        const char        *text,
                        EggRegex          *regex,
                        const char        *replacement,
                        int               *replaced)
{
    MooTextSearchFlags search_flags = 0;

    struct {
        GtkTextView *view;
        GtkWidget *dialog;
        MooTextReplaceResponse response;
    } data = {view, NULL, MOO_TEXT_REPLACE_DO_REPLACE};

    if (flags & MOO_FIND_CASELESS)
        search_flags |= MOO_TEXT_SEARCH_CASELESS;
    if (flags & MOO_FIND_WHOLE_WORDS)
        search_flags |= MOO_TEXT_SEARCH_WHOLE_WORDS;

    if (regex)
        *replaced = moo_text_replace_regex_all_interactive (start, end, regex, replacement,
                                                            flags & MOO_FIND_REPL_LITERAL,
                                                            replace_func, &data);
    else
        *replaced = moo_text_replace_all_interactive (start, end, text, replacement,
                                                      search_flags, replace_func, &data);

    if (data.dialog)
        gtk_widget_destroy (data.dialog);

    return data.response;
}


static void
run_replace_interactive (GtkTextView *view,
                         const char  *text,
                         EggRegex    *regex,
                         const char  *replacement,
                         MooFindFlags flags)
{
    GtkTextIter start, end;
    GtkTextBuffer *buffer;
    MooTextReplaceResponse result;
    int replaced;

    buffer = gtk_text_view_get_buffer (view);
    get_search_bounds (buffer, flags, &start, &end);

    result = do_replace_interactive (view, &start, &end, flags, text, regex, replacement, &replaced);

    if (result != MOO_TEXT_REPLACE_STOP && get_search_bounds2 (buffer, flags, &start, &end))
    {
        int replaced2;

        if (moo_text_search_from_start_dialog (GTK_WIDGET (view), replaced))
            do_replace_interactive (view, &start, &end, flags, text, regex, replacement, &replaced2);

        moo_text_replaced_n_dialog (GTK_WIDGET (view), replaced2);
    }
    else
    {
        moo_text_replaced_n_dialog (GTK_WIDGET (view), replaced);
    }
}


void
moo_text_view_run_replace (GtkTextView *view)
{
    GtkWidget *find;
    MooFindFlags flags;
    char *text, *replacement;
    EggRegex *regex;

    g_return_if_fail (GTK_IS_TEXT_VIEW (view));

    find = moo_find_new (TRUE);
    moo_find_setup (MOO_FIND (find), view);

    if (!moo_find_run (MOO_FIND (find)))
    {
        gtk_widget_destroy (find);
        return;
    }

    flags = moo_find_get_flags (MOO_FIND (find));
    text = moo_find_get_text (MOO_FIND (find));
    regex = moo_find_get_regex (MOO_FIND (find));
    replacement = moo_find_get_replacement (MOO_FIND (find));
    gtk_widget_destroy (find);

    if (flags & MOO_FIND_DONT_PROMPT)
        run_replace_silent (view, text, regex, replacement, flags);
    else
        run_replace_interactive (view, text, regex, replacement, flags);

    g_free (text);
    g_free (replacement);
    egg_regex_unref (regex);
}


/****************************************************************************/
/* goto line
 */

static void     update_spin_value   (GtkRange       *scale,
                                     GtkSpinButton  *spin);
static gboolean update_scale_value  (GtkSpinButton  *spin,
                                     GtkRange       *scale);

static void
update_spin_value (GtkRange       *scale,
                   GtkSpinButton  *spin)
{
    double value = gtk_range_get_value (scale);
    g_signal_handlers_block_matched (spin, G_SIGNAL_MATCH_FUNC, 0, 0, 0,
                                     (gpointer)update_scale_value, 0);
    gtk_spin_button_set_value (spin, value);
    g_signal_handlers_unblock_matched (spin, G_SIGNAL_MATCH_FUNC, 0, 0, 0,
                                       (gpointer)update_scale_value, 0);
}

static gboolean
update_scale_value (GtkSpinButton  *spin,
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
moo_text_view_run_goto_line (GtkTextView *view)
{
    GtkWidget *dialog;
    GtkTextBuffer *buffer;
    int line_count, line;
    GtkTextIter iter;
    GtkRange *scale;
    GtkSpinButton *spin;
    MooGladeXML *xml;

    g_return_if_fail (GTK_IS_TEXT_VIEW (view));

    buffer = gtk_text_view_get_buffer (view);
    line_count = gtk_text_buffer_get_line_count (buffer);

    xml = moo_glade_xml_new_from_buf (MOO_TEXT_GOTO_LINE_GLADE_UI,
                                      -1, NULL, NULL);
    g_return_if_fail (xml != NULL);

    dialog = moo_glade_xml_get_widget (xml, "dialog");

    gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog),
                                             GTK_RESPONSE_OK,
                                             GTK_RESPONSE_CANCEL,
                                             -1);

    gtk_text_buffer_get_iter_at_mark (buffer, &iter,
                                      gtk_text_buffer_get_insert (buffer));
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

    moo_position_window (dialog, GTK_WIDGET (view), FALSE, FALSE, 0, 0);

    g_object_unref (xml);

    if (gtk_dialog_run (GTK_DIALOG (dialog)) != GTK_RESPONSE_OK)
    {
        gtk_widget_destroy (dialog);
        return;
    }

    line = gtk_spin_button_get_value (spin) - 1;
    gtk_widget_destroy (dialog);

    if (MOO_IS_TEXT_VIEW (view))
    {
        moo_text_view_move_cursor (MOO_TEXT_VIEW (view), line, 0, FALSE);
    }
    else
    {
        gtk_text_buffer_get_iter_at_line (buffer, &iter, line);
        gtk_text_buffer_place_cursor (buffer, &iter);
        gtk_text_view_scroll_to_mark (view, gtk_text_buffer_get_insert (buffer),
                                      0, FALSE, 0, 0);
    }
}
