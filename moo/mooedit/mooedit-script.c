#include "mooedit/mooedit-script.h"
#include "mooedit/mootextview.h"
#include "mooutils/mooutils.h"

static void
get_selected_lines_bounds (GtkTextBuffer *buf,
                           GtkTextIter   *start,
                           GtkTextIter   *end,
                           gboolean      *cursor_at_next_line)
{
    if (cursor_at_next_line)
        *cursor_at_next_line = FALSE;

    gtk_text_buffer_get_selection_bounds (buf, start, end);

    gtk_text_iter_set_line_offset (start, 0);

    if (gtk_text_iter_starts_line (end) && !gtk_text_iter_equal (start, end))
    {
        gtk_text_iter_backward_line (end);
        if (cursor_at_next_line)
            *cursor_at_next_line = TRUE;
    }

    if (!gtk_text_iter_ends_line (end))
        gtk_text_iter_forward_to_line_end (end);

}

/**
 * moo_edit_get_selected_lines:
 *
 * Returns selected lines as a list of strings, one string for each line,
 * line terminator characters not included. If nothing is selected, then
 * line at cursor is returned.
 *
 * Returns: (type strv)
 **/
char **
moo_edit_get_selected_lines (MooEdit *doc)
{
    GtkTextIter start, end;
    GtkTextBuffer *buf;
    char *text;
    char **lines;

    moo_return_val_if_fail (MOO_IS_EDIT (doc), NULL);

    buf = moo_edit_get_buffer (doc);
    get_selected_lines_bounds (buf, &start, &end, NULL);
    text = gtk_text_buffer_get_slice (buf, &start, &end, TRUE);
    lines = moo_splitlines (text);
    g_free (text);
    return lines;
}

static char *
join_lines (char **strv)
{
    char **p;
    GString *string = g_string_new (NULL);
    for (p = strv; p && *p; ++p)
    {
        if (p != strv)
            g_string_append_c (string, '\n');
        g_string_append (string, *p);
    }
    return g_string_free (string, FALSE);
}

/**
 * moo_edit_replace_selected_lines:
 *
 * @doc:
 * @replacement: (type strv)
 *
 * replace selected lines with %param{replacement}. Similar to
 * %method{replace_selected_text()}, but selection is extended to include
 * whole lines. If nothing is selected, then line at cursor is replaced.
 **/
void
moo_edit_replace_selected_lines (MooEdit  *doc,
                                 char    **replacement)
{
//     switch (repl.vt())
//     {
//         case VtString:
//             text = repl.value<VtString>();
//             break;
//         case VtArray:
//             {
//                 moo::Vector<String> lines = get_string_list(repl);
//                 text = join(lines, "\n");
//             }
//             break;
//         case VtArgList:
//             {
//                 moo::Vector<String> lines = get_string_list(repl);
//                 text = join(lines, "\n");
//             }
//             break;
//         default:
//             Error::raisef("string or list of strings expected, got %s",
//                           get_argument_type_name(repl.vt()));
//             break;
//     }

    GtkTextBuffer *buf;
    GtkTextIter start, end;
    gboolean cursor_at_next_line;

    moo_return_if_fail (MOO_IS_EDIT (doc));

    buf = moo_edit_get_buffer (doc);
    get_selected_lines_bounds (buf, &start, &end, &cursor_at_next_line);
    gtk_text_buffer_delete (buf, &start, &end);

    if (replacement)
    {
        char *text = join_lines (replacement);
        gtk_text_buffer_insert (buf, &start, text, -1);
        g_free (text);
    }

    if (cursor_at_next_line)
    {
        gtk_text_iter_forward_line (&start);
        gtk_text_buffer_place_cursor (buf, &start);
    }
}

/**
 * moo_edit_get_selected_text:
 *
 * returns selected text.
 **/
char *
moo_edit_get_selected_text (MooEdit *doc)
{
    GtkTextBuffer *buf;
    GtkTextIter start, end;

    moo_return_val_if_fail (MOO_IS_EDIT (doc), NULL);

    buf = moo_edit_get_buffer (doc);
    gtk_text_buffer_get_selection_bounds(buf, &start, &end);
    return gtk_text_buffer_get_slice(buf, &start, &end, TRUE);
}

/**
 * moo_edit_replace_selected_text:
 *
 * replace selected text with %param{replacement}. If nothing is selected,
 * then %param{replacement} is inserted at cursor.
 **/
void
moo_edit_replace_selected_text (MooEdit    *doc,
                                const char *replacement)
{
    GtkTextBuffer *buf;
    GtkTextIter start, end;

    moo_return_if_fail (MOO_IS_EDIT (doc));
    moo_return_if_fail (replacement != NULL);

    buf = moo_edit_get_buffer (doc);
    gtk_text_buffer_get_selection_bounds (buf, &start, &end);
    gtk_text_buffer_delete (buf, &start, &end);
    if (*replacement)
        gtk_text_buffer_insert (buf, &start, replacement, -1);
    gtk_text_buffer_place_cursor (buf, &start);
}

/**
 * moo_edit_has_selection:
 **/
gboolean
moo_edit_has_selection (MooEdit *doc)
{
    MooEditView *view = moo_edit_get_view (doc);
    return moo_text_view_has_selection (MOO_TEXT_VIEW (view));
}

/**
 * moo_edit_get_text:
 **/
char *
moo_edit_get_text (MooEdit *doc)
{
    MooEditView *view = moo_edit_get_view (doc);
    return moo_text_view_get_text (GTK_TEXT_VIEW (view));
}

static void
get_iter (int pos, GtkTextBuffer *buf, GtkTextIter *iter)
{
    if (pos > gtk_text_buffer_get_char_count(buf) || pos < 0)
    {
        moo_critical ("invalid offset");
        pos = 0;
    }

    gtk_text_buffer_get_iter_at_offset (buf, iter, pos);
}

/**
 * moo_edit_set_selection:
 **/
void
moo_edit_set_selection (MooEdit *doc,
                        int      pos_start,
                        int      pos_end)
{
    GtkTextBuffer *buf;
    GtkTextIter start, end;

    moo_return_if_fail (MOO_IS_EDIT (doc));

    buf = moo_edit_get_buffer (doc);

    get_iter (pos_start, buf, &start);
    get_iter (pos_end, buf, &end);

    gtk_text_buffer_select_range(buf, &start, &end);
}

/**
 * moo_edit_get_cursor_pos:
 **/
int
moo_edit_get_cursor_pos (MooEdit *doc)
{
    GtkTextBuffer *buf;
    GtkTextIter iter;

    moo_return_val_if_fail (MOO_IS_EDIT (doc), 0);

    buf = moo_edit_get_buffer (doc);
    gtk_text_buffer_get_iter_at_mark(buf, &iter, gtk_text_buffer_get_insert(buf));
    return gtk_text_iter_get_offset(&iter);
}

/**
 * moo_edit_insert_text:
 **/
void
moo_edit_insert_text (MooEdit    *doc,
                      const char *text)
{
    GtkTextIter iter;
    GtkTextBuffer *buf;

    moo_return_if_fail (MOO_IS_EDIT (doc));
    moo_return_if_fail (text != NULL);

    buf = moo_edit_get_buffer (doc);

    gtk_text_buffer_get_iter_at_mark (buf, &iter, gtk_text_buffer_get_insert(buf));
    gtk_text_buffer_insert (buf, &iter, text, -1);
}
