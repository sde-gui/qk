/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   mootextbuffer.c
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
#include "mooedit/moolinebuffer.h"
#include "mooedit/mootextiter.h"
#include "mooedit/moohighlighter.h"
#include "mooutils/moomarshals.h"


struct _MooTextBufferPrivate {
    gboolean has_selection;
    gboolean has_text;

    MooHighlighter *hl;
    MooLang *lang;
    gboolean may_apply_tag;
    gboolean do_highlight;

    gboolean check_brackets;
    guint num_brackets;
    gunichar *left_brackets, *right_brackets;
    GtkTextTag *correct_match_tag;
    GtkTextTag *incorrect_match_tag;
    GtkTextMark *bracket_mark[4];
    MooBracketMatchType bracket_found;

    LineBuffer *line_buf;

    int cursor_moved_frozen;
    gboolean cursor_moved;
};


static void     moo_text_buffer_finalize            (GObject        *object);

static void     moo_text_buffer_set_property        (GObject        *object,
                                                     guint           prop_id,
                                                     const GValue   *value,
                                                     GParamSpec     *pspec);
static void     moo_text_buffer_get_property        (GObject        *object,
                                                     guint           prop_id,
                                                     GValue         *value,
                                                     GParamSpec     *pspec);

static void     moo_text_buffer_cursor_moved        (MooTextBuffer      *buffer,
                                                     const GtkTextIter  *iter);
static void     moo_text_buffer_mark_set            (GtkTextBuffer      *buffer,
                                                     const GtkTextIter  *iter,
                                                     GtkTextMark        *mark);
static void     moo_text_buffer_insert_text         (GtkTextBuffer      *buffer,
                                                     GtkTextIter        *pos,
                                                     const gchar        *text,
                                                     gint                length);
static void     moo_text_buffer_apply_tag           (GtkTextBuffer      *buffer,
                                                     GtkTextTag         *tag,
                                                     const GtkTextIter  *start,
                                                     const GtkTextIter  *end);
static void     moo_text_buffer_delete_range        (GtkTextBuffer      *buffer,
                                                     GtkTextIter        *start,
                                                     GtkTextIter        *end);
static void     moo_text_buffer_begin_user_action   (GtkTextBuffer      *buffer);
static void     moo_text_buffer_end_user_action     (GtkTextBuffer      *buffer);

static void     moo_text_buffer_queue_highlight     (MooTextBuffer      *buffer);

static void     cursor_moved                        (MooTextBuffer      *buffer,
                                                     const GtkTextIter  *iter);
static void     freeze_cursor_moved                 (MooTextBuffer      *buffer);
static void     thaw_cursor_moved                   (MooTextBuffer      *buffer);

enum {
    CURSOR_MOVED,
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];


enum {
    PROP_0,
    PROP_CHECK_BRACKETS,
    PROP_BRACKET_MATCH_STYLE,
    PROP_BRACKET_MISMATCH_STYLE,
    PROP_HAS_TEXT,
    PROP_HAS_SELECTION,
    PROP_LANG
};


/* MOO_TYPE_TEXT_BUFFER */
G_DEFINE_TYPE (MooTextBuffer, moo_text_buffer, GTK_TYPE_TEXT_BUFFER)


static void
moo_text_buffer_class_init (MooTextBufferClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GtkTextBufferClass *buffer_class = GTK_TEXT_BUFFER_CLASS (klass);

    gobject_class->set_property = moo_text_buffer_set_property;
    gobject_class->get_property = moo_text_buffer_get_property;
    gobject_class->finalize = moo_text_buffer_finalize;

    buffer_class->mark_set = moo_text_buffer_mark_set;
    buffer_class->insert_text = moo_text_buffer_insert_text;
    buffer_class->delete_range = moo_text_buffer_delete_range;
    buffer_class->apply_tag = moo_text_buffer_apply_tag;
    buffer_class->begin_user_action = moo_text_buffer_begin_user_action;
    buffer_class->end_user_action = moo_text_buffer_end_user_action;

    klass->cursor_moved = moo_text_buffer_cursor_moved;

    g_object_class_install_property (gobject_class,
                                     PROP_CHECK_BRACKETS,
                                     g_param_spec_boolean ("check-brackets",
                                             "check-brackets",
                                             "check-brackets",
                                             TRUE,
                                             G_PARAM_CONSTRUCT | G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class,
                                     PROP_BRACKET_MATCH_STYLE,
                                     g_param_spec_boxed ("bracket-match-style",
                                             "bracket-match-style",
                                             "bracket-match-style",
                                             MOO_TYPE_TEXT_STYLE,
                                             G_PARAM_CONSTRUCT | G_PARAM_WRITABLE));

    g_object_class_install_property (gobject_class,
                                     PROP_BRACKET_MISMATCH_STYLE,
                                     g_param_spec_boxed ("bracket-mismatch-style",
                                             "bracket-mismatch-style",
                                             "bracket-mismatch-style",
                                             MOO_TYPE_TEXT_STYLE,
                                             G_PARAM_CONSTRUCT | G_PARAM_WRITABLE));

    g_object_class_install_property (gobject_class,
                                     PROP_HAS_TEXT,
                                     g_param_spec_boolean ("has-text",
                                             "has-text",
                                             "has-text",
                                             FALSE,
                                             G_PARAM_READABLE));

    g_object_class_install_property (gobject_class,
                                     PROP_HAS_SELECTION,
                                     g_param_spec_boolean ("has-selection",
                                             "has-selection",
                                             "has-selection",
                                             FALSE,
                                             G_PARAM_READABLE));

    g_object_class_install_property (gobject_class,
                                     PROP_LANG,
                                     g_param_spec_boxed ("lang",
                                             "lang",
                                             "lang",
                                             MOO_TYPE_LANG,
                                             G_PARAM_READWRITE));

    signals[CURSOR_MOVED] =
            g_signal_new ("cursor-moved",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooTextBufferClass, cursor_moved),
                          NULL, NULL,
                          _moo_marshal_VOID__BOXED,
                          G_TYPE_NONE, 1,
                          GTK_TYPE_TEXT_ITER);
}


static void
moo_text_buffer_init (MooTextBuffer *buffer)
{
    buffer->priv = g_new0 (MooTextBufferPrivate, 1);
    buffer->priv->line_buf = moo_line_buffer_new ();
    buffer->priv->do_highlight = TRUE;
    buffer->priv->hl = moo_highlighter_new (GTK_TEXT_BUFFER (buffer),
                                            buffer->priv->line_buf, NULL);
    buffer->priv->bracket_found = MOO_BRACKET_MATCH_NONE;
}


static void
moo_text_buffer_finalize (GObject *object)
{
    MooTextBuffer *buffer = MOO_TEXT_BUFFER (object);

    moo_highlighter_destroy (buffer->priv->hl, FALSE);
    moo_line_buffer_free (buffer->priv->line_buf);

    g_free (buffer->priv->left_brackets);
    g_free (buffer->priv->right_brackets);

    g_free (buffer->priv);
    buffer->priv = NULL;

    G_OBJECT_CLASS (moo_text_buffer_parent_class)->finalize (object);
}


GtkTextBuffer*
moo_text_buffer_new (GtkTextTagTable *table)
{
    return g_object_new (MOO_TYPE_TEXT_BUFFER,
                         "tag-table", table, NULL);
}


static void
moo_text_buffer_begin_user_action (GtkTextBuffer *buffer)
{
    moo_text_buffer_freeze (MOO_TEXT_BUFFER (buffer));
}


static void
moo_text_buffer_end_user_action (GtkTextBuffer *buffer)
{
    moo_text_buffer_thaw (MOO_TEXT_BUFFER (buffer));
}


static void
moo_text_buffer_mark_set (GtkTextBuffer      *text_buffer,
                          const GtkTextIter  *iter,
                          GtkTextMark        *mark)
{
    GtkTextMark *insert = gtk_text_buffer_get_insert (text_buffer);
    GtkTextMark *sel_bound = gtk_text_buffer_get_selection_bound (text_buffer);
    MooTextBuffer *buffer = MOO_TEXT_BUFFER (text_buffer);
    gboolean has_selection = buffer->priv->has_selection;

    if (GTK_TEXT_BUFFER_CLASS(moo_text_buffer_parent_class)->mark_set)
        GTK_TEXT_BUFFER_CLASS(moo_text_buffer_parent_class)->mark_set (text_buffer, iter, mark);

    if (mark == insert)
    {
        GtkTextIter iter2;
        gtk_text_buffer_get_iter_at_mark (text_buffer, &iter2, sel_bound);
        has_selection = gtk_text_iter_compare (iter, &iter2) ? TRUE : FALSE;
    }
    else if (mark == sel_bound)
    {
        GtkTextIter iter2;
        gtk_text_buffer_get_iter_at_mark (text_buffer, &iter2, insert);
        has_selection = gtk_text_iter_compare (iter, &iter2) ? TRUE : FALSE;
    }

    if (has_selection != buffer->priv->has_selection)
    {
        buffer->priv->has_selection = has_selection;
        g_object_notify (G_OBJECT (buffer), "has-selection");
    }

    if (mark == insert)
        cursor_moved (buffer, iter);
}


static void
moo_text_buffer_insert_text (GtkTextBuffer      *text_buffer,
                             GtkTextIter        *pos,
                             const gchar        *text,
                             gint                length)
{
    MooTextBuffer *buffer = MOO_TEXT_BUFFER (text_buffer);
    int first_line, last_line;
    gboolean starts_line, ins_line;

    if (!text[0])
        return;

    first_line = gtk_text_iter_get_line (pos);
    starts_line = gtk_text_iter_starts_line (pos);
    ins_line = (text[0] == '\n' || text[0] == '\r');

    GTK_TEXT_BUFFER_CLASS(moo_text_buffer_parent_class)->insert_text (text_buffer, pos, text, length);

    last_line = gtk_text_iter_get_line (pos);

    if (last_line == first_line)
    {
        moo_line_buffer_invalidate (buffer->priv->line_buf, first_line);
    }
    else
    {
        moo_line_buffer_insert_range (buffer->priv->line_buf,
                                      first_line + 1,
                                      last_line - first_line);
        moo_line_buffer_invalidate (buffer->priv->line_buf, first_line);
        moo_line_buffer_invalidate (buffer->priv->line_buf, last_line);
    }

    moo_text_buffer_queue_highlight (buffer);

    cursor_moved (buffer, pos);

    if (!buffer->priv->has_text)
    {
        buffer->priv->has_text = TRUE;
        g_object_notify (G_OBJECT (buffer), "has-text");
    }
}


static void
moo_text_buffer_delete_range (GtkTextBuffer      *text_buffer,
                              GtkTextIter        *start,
                              GtkTextIter        *end)
{
    MooTextBuffer *buffer = MOO_TEXT_BUFFER (text_buffer);
    int first_line, last_line;

    first_line = gtk_text_iter_get_line (start);
    last_line = gtk_text_iter_get_line (end);

#define MANY_LINES 1000
    if (buffer->priv->lang && buffer->priv->do_highlight &&
        last_line - first_line > MANY_LINES)
    {
        gtk_text_buffer_remove_all_tags (text_buffer, start, end);
    }
#undef MANY_LINES

    GTK_TEXT_BUFFER_CLASS(moo_text_buffer_parent_class)->delete_range (text_buffer, start, end);

    if (first_line < last_line)
        moo_line_buffer_delete_range (buffer->priv->line_buf,
                                      first_line + 1,
                                      last_line - first_line);

    moo_line_buffer_invalidate (buffer->priv->line_buf, first_line);
    moo_text_buffer_queue_highlight (buffer);

    cursor_moved (buffer, start);

    if (buffer->priv->has_text)
    {
        buffer->priv->has_text = moo_text_buffer_has_text (buffer);
        if (!buffer->priv->has_text)
            g_object_notify (G_OBJECT (buffer), "has-text");
    }
}


void
moo_text_buffer_set_lang (MooTextBuffer  *buffer,
                          MooLang        *lang)
{
    MooLang *old_lang;

    g_return_if_fail (MOO_IS_TEXT_BUFFER (buffer));

    if (buffer->priv->lang == lang)
        return;

    old_lang = buffer->priv->lang;

    if (old_lang)
        moo_lang_unref (old_lang);

    moo_highlighter_destroy (buffer->priv->hl, TRUE);

    buffer->priv->lang = lang;

    if (lang)
        moo_lang_ref (lang);

    buffer->priv->hl = moo_highlighter_new (GTK_TEXT_BUFFER (buffer),
                                            buffer->priv->line_buf, lang);

    if (old_lang)
        moo_line_buffer_invalidate_all (buffer->priv->line_buf);

    moo_text_buffer_queue_highlight (buffer);

    moo_text_buffer_set_brackets (buffer, lang ? lang->brackets : NULL);
}


MooLang*
moo_text_buffer_get_lang (MooTextBuffer  *buffer)
{
    g_return_val_if_fail (MOO_IS_TEXT_BUFFER (buffer), NULL);
    return buffer->priv->lang;
}


static void
moo_text_buffer_queue_highlight (MooTextBuffer *buffer)
{
    if (buffer->priv->lang && buffer->priv->do_highlight)
        moo_highlighter_queue_compute (buffer->priv->hl, FALSE);
}


void
moo_text_buffer_set_highlight (MooTextBuffer      *buffer,
                               gboolean            highlight)
{
    g_return_if_fail (MOO_IS_TEXT_BUFFER (buffer));

    if ((highlight && buffer->priv->do_highlight) || (!highlight && !buffer->priv->do_highlight))
        return;

    if (buffer->priv->do_highlight && buffer->priv->lang)
        moo_highlighter_destroy (buffer->priv->hl, TRUE);
    else
        moo_highlighter_destroy (buffer->priv->hl, FALSE);

    buffer->priv->do_highlight = highlight;

    if (!highlight || !buffer->priv->lang)
    {
        buffer->priv->hl = moo_highlighter_new (GTK_TEXT_BUFFER (buffer),
                                                buffer->priv->line_buf, NULL);
    }
    else
    {
        buffer->priv->hl = moo_highlighter_new (GTK_TEXT_BUFFER (buffer),
                                                buffer->priv->line_buf,
                                                buffer->priv->lang);
        moo_line_buffer_invalidate_all (buffer->priv->line_buf);
        moo_text_buffer_queue_highlight (buffer);
    }
}


gboolean
moo_text_buffer_get_highlight (MooTextBuffer      *buffer)
{
    g_return_val_if_fail (MOO_IS_TEXT_BUFFER (buffer), FALSE);
    return buffer->priv->do_highlight ? TRUE : FALSE;
}


static void
moo_text_buffer_set_property (GObject        *object,
                              guint           prop_id,
                              const GValue   *value,
                              GParamSpec     *pspec)
{
    MooTextBuffer *buffer = MOO_TEXT_BUFFER (object);

    switch (prop_id)
    {
        case PROP_CHECK_BRACKETS:
            moo_text_buffer_set_check_brackets (buffer, g_value_get_boolean (value));
            break;

        case PROP_BRACKET_MATCH_STYLE:
            moo_text_buffer_set_bracket_match_style (buffer, g_value_get_boxed (value));
            break;

        case PROP_BRACKET_MISMATCH_STYLE:
            moo_text_buffer_set_bracket_mismatch_style (buffer, g_value_get_boxed (value));
            break;

        case PROP_LANG:
            moo_text_buffer_set_lang (buffer, g_value_get_boxed (value));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}


static void
moo_text_buffer_get_property (GObject        *object,
                              guint           prop_id,
                              GValue         *value,
                              GParamSpec     *pspec)
{
    MooTextBuffer *buffer = MOO_TEXT_BUFFER (object);

    switch (prop_id)
    {
        case PROP_CHECK_BRACKETS:
            g_value_set_boolean (value, buffer->priv->check_brackets);
            break;

        case PROP_LANG:
            g_value_set_boxed (value, buffer->priv->lang);
            break;

        case PROP_HAS_TEXT:
            g_value_set_boolean (value, moo_text_buffer_has_text (buffer));
            break;

        case PROP_HAS_SELECTION:
            g_value_set_boolean (value, moo_text_buffer_has_selection (buffer));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}


gboolean
moo_text_buffer_has_text (MooTextBuffer *buffer)
{
    GtkTextIter start, end;

    g_return_val_if_fail (MOO_IS_TEXT_BUFFER (buffer), TRUE);

    gtk_text_buffer_get_bounds (GTK_TEXT_BUFFER (buffer), &start, &end);
    return gtk_text_iter_compare (&start, &end) ? TRUE : FALSE;
}


gboolean
moo_text_buffer_has_selection (MooTextBuffer *buffer)
{
    g_return_val_if_fail (MOO_IS_TEXT_BUFFER (buffer), FALSE);
    return gtk_text_buffer_get_selection_bounds (GTK_TEXT_BUFFER (buffer), NULL, NULL);
}


void
_moo_text_buffer_ensure_highlight (MooTextBuffer      *buffer,
                                   int                 first_line,
                                   int                 last_line)
{
    g_return_if_fail (MOO_IS_TEXT_BUFFER (buffer));
    moo_highlighter_apply_tags (buffer->priv->hl, first_line, last_line);
}


static void
moo_text_buffer_apply_tag (GtkTextBuffer      *buffer,
                           GtkTextTag         *tag,
                           const GtkTextIter  *start,
                           const GtkTextIter  *end)
{
    if (MOO_IS_SYNTAX_TAG (tag) && !MOO_TEXT_BUFFER(buffer)->priv->may_apply_tag)
        return;

    GTK_TEXT_BUFFER_CLASS(moo_text_buffer_parent_class)->apply_tag (buffer, tag, start, end);
}


void
_moo_text_buffer_apply_syntax_tag (MooTextBuffer      *buffer,
                                   GtkTextTag         *tag,
                                   const GtkTextIter  *start,
                                   const GtkTextIter  *end)
{
    buffer->priv->may_apply_tag = TRUE;
    gtk_text_buffer_apply_tag (GTK_TEXT_BUFFER (buffer), tag, start, end);
    buffer->priv->may_apply_tag = FALSE;
}


void
moo_text_buffer_apply_scheme (MooTextBuffer      *buffer,
                              MooTextStyleScheme *scheme)
{
    g_return_if_fail (scheme != NULL);
    g_return_if_fail (MOO_IS_TEXT_BUFFER (buffer));

    moo_text_buffer_set_bracket_match_style (buffer, scheme->bracket_match);
    moo_text_buffer_set_bracket_mismatch_style (buffer, scheme->bracket_mismatch);
    moo_highlighter_apply_scheme (buffer->priv->hl, scheme);
}


void
moo_text_buffer_freeze (MooTextBuffer *buffer)
{
    g_return_if_fail (MOO_IS_TEXT_BUFFER (buffer));
    freeze_cursor_moved (buffer);
}


void
moo_text_buffer_thaw (MooTextBuffer *buffer)
{
    g_return_if_fail (MOO_IS_TEXT_BUFFER (buffer));
    thaw_cursor_moved (buffer);
}


/*****************************************************************************/
/* Matching brackets
 */

#define FIND_BRACKETS_LIMIT 10000

static void
cursor_moved (MooTextBuffer      *buffer,
              const GtkTextIter  *where)
{
    if (!buffer->priv->cursor_moved_frozen)
        g_signal_emit (buffer, signals[CURSOR_MOVED], 0, where);
    else
        buffer->priv->cursor_moved = TRUE;
}

static void
freeze_cursor_moved (MooTextBuffer *buffer)
{
    buffer->priv->cursor_moved_frozen++;
}

static void
thaw_cursor_moved (MooTextBuffer *buffer)
{
    g_return_if_fail (buffer->priv->cursor_moved_frozen > 0);

    if (!--buffer->priv->cursor_moved_frozen && buffer->priv->cursor_moved)
    {
        GtkTextIter iter;
        GtkTextMark *insert = gtk_text_buffer_get_insert (GTK_TEXT_BUFFER (buffer));
        gtk_text_buffer_get_iter_at_mark (GTK_TEXT_BUFFER (buffer), &iter, insert);
        cursor_moved (buffer, &iter);
        buffer->priv->cursor_moved = FALSE;
    }
}


static void
moo_text_buffer_cursor_moved (MooTextBuffer      *buffer,
                              const GtkTextIter  *where)
{
    GtkTextIter iter[4];
    MooBracketMatchType bracket_match;
    GtkTextTag *tag;
    GtkTextBuffer *text_buffer = GTK_TEXT_BUFFER (buffer);
    int i;

    g_return_if_fail (where != NULL);

    switch (buffer->priv->bracket_found)
    {
        case MOO_BRACKET_MATCH_CORRECT:
            tag = buffer->priv->correct_match_tag;
            break;
        case MOO_BRACKET_MATCH_INCORRECT:
            tag = buffer->priv->incorrect_match_tag;
            break;
        default:
            tag = NULL;
    }

    if (tag)
    {
        for (i = 0; i < 4; ++i)
            gtk_text_buffer_get_iter_at_mark (text_buffer, &iter[i],
                                              buffer->priv->bracket_mark[i]);
        gtk_text_buffer_remove_tag (text_buffer, tag, &iter[0], &iter[1]);
        gtk_text_buffer_remove_tag (text_buffer, tag, &iter[2], &iter[3]);
    }

    if (!buffer->priv->check_brackets)
        return;

    iter[0] = *where;

    buffer->priv->bracket_found = MOO_BRACKET_MATCH_NONE;

    if (!moo_text_iter_at_bracket (&iter[0]))
        return;

    iter[2] = iter[0];
    bracket_match = moo_text_iter_find_matching_bracket (&iter[2], FIND_BRACKETS_LIMIT);

    buffer->priv->bracket_found = bracket_match;

    switch (bracket_match)
    {
        case MOO_BRACKET_MATCH_CORRECT:
            tag = buffer->priv->correct_match_tag;
            break;
        case MOO_BRACKET_MATCH_INCORRECT:
            tag = buffer->priv->incorrect_match_tag;
            break;
        default:
            tag = NULL;
    }

    if (tag != NULL)
    {
        iter[1] = iter[0];
        gtk_text_iter_forward_char (&iter[1]);
        iter[3] = iter[2];
        gtk_text_iter_forward_char (&iter[3]);

        if (!buffer->priv->bracket_mark[0])
            for (i = 0; i < 4; ++i)
                buffer->priv->bracket_mark[i] =
                        gtk_text_buffer_create_mark (text_buffer, NULL, &iter[i], FALSE);
        else
            for (i = 0; i < 4; ++i)
                gtk_text_buffer_move_mark (text_buffer, buffer->priv->bracket_mark[i], &iter[i]);

        gtk_text_buffer_apply_tag (text_buffer, tag, &iter[0], &iter[1]);
        gtk_text_buffer_apply_tag (text_buffer, tag, &iter[2], &iter[3]);
    }
}


void
moo_text_buffer_set_bracket_match_style (MooTextBuffer      *buffer,
                                         const MooTextStyle *style)
{
    static MooTextStyle empty;

    g_return_if_fail (MOO_IS_TEXT_BUFFER (buffer));

    if (!style)
        style = &empty;

    if (!buffer->priv->correct_match_tag)
        buffer->priv->correct_match_tag =
                gtk_text_buffer_create_tag (GTK_TEXT_BUFFER (buffer), NULL, NULL);

    _moo_style_set_tag_style (style, buffer->priv->correct_match_tag);
    g_object_notify (G_OBJECT (buffer), "bracket-match-style");
}


void
moo_text_buffer_set_bracket_mismatch_style (MooTextBuffer      *buffer,
                                            const MooTextStyle *style)
{
    static MooTextStyle empty;

    g_return_if_fail (MOO_IS_TEXT_BUFFER (buffer));

    if (!style)
        style = &empty;

    if (!buffer->priv->incorrect_match_tag)
        buffer->priv->incorrect_match_tag =
                gtk_text_buffer_create_tag (GTK_TEXT_BUFFER (buffer), NULL, NULL);

    _moo_style_set_tag_style (style, buffer->priv->incorrect_match_tag);
    g_object_notify (G_OBJECT (buffer), "bracket-mismatch-style");
}


static gboolean
parse_brackets (const char     *string,
                gunichar      **left_brackets,
                gunichar      **right_brackets,
                guint          *num)
{
    long len, i;
    const char *p;
    gunichar *left, *right;

    g_return_val_if_fail (g_utf8_validate (string, -1, NULL), FALSE);
    len = g_utf8_strlen (string, -1);
    g_return_val_if_fail (len > 0 && (len / 2) * 2 == len, FALSE);

    len /= 2;
    p = string;
    left = g_new (gunichar, len);
    right = g_new (gunichar, len);

    for (i = 0; i < len; ++i)
    {
        left[i] = g_utf8_get_char (p);
        p = g_utf8_next_char (p);
        right[i] = g_utf8_get_char (p);
        p = g_utf8_next_char (p);
    }

    *left_brackets = left;
    *right_brackets = right;
    *num = len;

    return TRUE;
}


void
moo_text_buffer_set_brackets (MooTextBuffer *buffer,
                              const gchar   *string)
{
    g_return_if_fail (MOO_IS_TEXT_BUFFER (buffer));

    buffer->priv->num_brackets = 0;
    g_free (buffer->priv->left_brackets);
    buffer->priv->left_brackets = NULL;
    g_free (buffer->priv->right_brackets);
    buffer->priv->right_brackets = NULL;

    if (!string)
    {
        buffer->priv->num_brackets = 3;
        buffer->priv->left_brackets = g_new (gunichar, 3);
        buffer->priv->right_brackets = g_new (gunichar, 3);
        buffer->priv->left_brackets[0] = '(';
        buffer->priv->left_brackets[1] = '{';
        buffer->priv->left_brackets[2] = '[';
        buffer->priv->right_brackets[0] = ')';
        buffer->priv->right_brackets[1] = '}';
        buffer->priv->right_brackets[2] = ']';
        return;
    }
    else if (!string[0])
    {
        return;
    }

    parse_brackets (string,
                    &(buffer->priv->left_brackets),
                    &(buffer->priv->right_brackets),
                    &(buffer->priv->num_brackets));
}


void
moo_text_buffer_set_check_brackets (MooTextBuffer  *buffer,
                                    gboolean        check)
{
    g_return_if_fail (MOO_IS_TEXT_BUFFER (buffer));
    buffer->priv->check_brackets = check ? TRUE : FALSE;
    g_object_notify (G_OBJECT (buffer), "check-brackets");
}


inline static gboolean
find (gunichar *vec, guint size, gunichar c)
{
    guint i;

    for (i = 0; i < size; ++i)
        if (c == vec[i])
            return TRUE;

    return FALSE;
}


gboolean
moo_text_iter_at_bracket (GtkTextIter *iter)
{
    gunichar c;
    GtkTextIter b;
    MooTextBuffer *buffer;

    buffer = MOO_TEXT_BUFFER (gtk_text_iter_get_buffer (iter));
    g_return_val_if_fail (MOO_IS_TEXT_BUFFER (buffer), FALSE);

    c = gtk_text_iter_get_char (iter);

    if (find (buffer->priv->left_brackets, buffer->priv->num_brackets, c) ||
        find (buffer->priv->right_brackets, buffer->priv->num_brackets, c))
    {
        return TRUE;
    }

    b = *iter;

    if (!gtk_text_iter_starts_line (&b) && gtk_text_iter_backward_char (&b))
    {
        c = gtk_text_iter_get_char (&b);

        if (find (buffer->priv->left_brackets, buffer->priv->num_brackets, c) ||
            find (buffer->priv->right_brackets, buffer->priv->num_brackets, c))
        {
            *iter = b;
            return TRUE;
        }
    }

    return FALSE;
}


MooBracketMatchType
moo_text_iter_find_matching_bracket (GtkTextIter *iter,
                                     int          limit)
{
    int addition = 0;
    int count;
    gunichar *same_direction = NULL, *inverse_direction = NULL;
    guint to_find;
    gunichar bracket = gtk_text_iter_get_char (iter);
    gunichar bracket_to_find = 0;
    MooContext *ctx;
    guint stack = 0;
    GtkTextIter b;
    MooTextBuffer *buffer;

    buffer = MOO_TEXT_BUFFER (gtk_text_iter_get_buffer (iter));
    g_return_val_if_fail (MOO_IS_TEXT_BUFFER (buffer), MOO_BRACKET_MATCH_NOT_AT_BRACKET);

    for (to_find = 0; to_find < buffer->priv->num_brackets; ++to_find)
    {
        if (bracket == buffer->priv->left_brackets[to_find])
        {
            bracket_to_find = buffer->priv->right_brackets[to_find];
            addition = 1;
            inverse_direction = buffer->priv->right_brackets;
            same_direction = buffer->priv->left_brackets;
            break;
        }
    }

    if (to_find == buffer->priv->num_brackets)
    {
        for (to_find = 0; to_find < buffer->priv->num_brackets; ++to_find)
        {
            if (bracket == buffer->priv->right_brackets[to_find])
            {
                bracket_to_find = buffer->priv->left_brackets[to_find];
                addition = -1;
                same_direction = buffer->priv->right_brackets;
                inverse_direction = buffer->priv->left_brackets;
                break;
            }
        }
    }

    if (to_find == buffer->priv->num_brackets)
        return MOO_BRACKET_MATCH_NOT_AT_BRACKET;

    ctx = _moo_text_iter_get_context (iter);
    stack = 0;
    b = *iter;

    if (limit < 0)
        limit = G_MAXINT;
    count = 0;

    while (gtk_text_iter_forward_chars (&b, addition) && count++ < limit)
    {
        if (ctx == _moo_text_iter_get_context (&b))
        {
            gunichar c = gtk_text_iter_get_char (&b);

            if (c == bracket_to_find && !stack)
            {
                *iter = b;
                return MOO_BRACKET_MATCH_CORRECT;
            }

            if (find (same_direction, buffer->priv->num_brackets, c))
            {
                ++stack;
            }
            else if (find (inverse_direction, buffer->priv->num_brackets, c))
            {
                if (stack)
                {
                    --stack;
                }
                else
                {
                    *iter = b;
                    return MOO_BRACKET_MATCH_INCORRECT;
                }
            }
        }
    }

    return MOO_BRACKET_MATCH_NONE;
}
