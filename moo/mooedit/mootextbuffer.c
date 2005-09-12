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

#define MOOTEXT_BUFFER_COMPILATION
#include "mooedit/mootextbuffer.h"
#include "mooedit/mootextiter.h"
#include "mooutils/moomarshals.h"


struct _MooTextBufferPrivate {
    MooEditLang           *lang;

    gboolean               has_selection;
    gboolean               has_text;

    gboolean               check_brackets;
    guint                  num_brackets;
    gunichar              *left_brackets, *right_brackets;
    GtkTextTag            *correct_match_tag;
    GtkTextTag            *incorrect_match_tag;
    GtkTextMark           *bracket_mark[4];
    MooBracketMatchType    bracket_found;
};


static void     moo_text_buffer_finalize        (GObject        *object);

static void     moo_text_buffer_set_property    (GObject        *object,
                                                 guint           prop_id,
                                                 const GValue   *value,
                                                 GParamSpec     *pspec);
static void     moo_text_buffer_get_property    (GObject        *object,
                                                 guint           prop_id,
                                                 GValue         *value,
                                                 GParamSpec     *pspec);

static void     moo_text_buffer_cursor_moved    (MooTextBuffer      *buffer,
                                                 const GtkTextIter  *iter);
static void     moo_text_buffer_mark_set        (GtkTextBuffer      *buffer,
                                                 const GtkTextIter  *iter,
                                                 GtkTextMark        *mark);
static void     moo_text_buffer_insert_text     (GtkTextBuffer      *buffer,
                                                 GtkTextIter        *pos,
                                                 const gchar        *text,
                                                 gint                length);
static void     moo_text_buffer_delete_range    (GtkTextBuffer      *buffer,
                                                 GtkTextIter        *start,
                                                 GtkTextIter        *end);


enum {
    CURSOR_MOVED,
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];


enum {
    PROP_0,
    PROP_CHECK_BRACKETS,
    PROP_BRACKET_MATCH_STYLE,
    PROP_HAS_TEXT,
    PROP_HAS_SELECTION
};


/* MOO_TYPE_TEXT_BUFFER */
G_DEFINE_TYPE (MooTextBuffer, moo_text_buffer, GTK_TYPE_SOURCE_BUFFER)


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
                                             GTK_TYPE_SOURCE_TAG_STYLE,
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

    signals[CURSOR_MOVED] =
            g_signal_new ("cursor-moved",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
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
    buffer->priv->check_brackets = TRUE;
    buffer->priv->bracket_found = MOO_BRACKET_MATCH_NONE;
    moo_text_buffer_set_brackets (buffer, NULL);
    gtk_source_buffer_set_check_brackets (GTK_SOURCE_BUFFER (buffer), FALSE);
}


static void
moo_text_buffer_finalize (GObject *object)
{
    MooTextBuffer *buffer = MOO_TEXT_BUFFER (object);

    if (buffer->priv->lang)
        g_object_unref (buffer->priv->lang);

    g_free (buffer->priv->left_brackets);
    g_free (buffer->priv->right_brackets);

    g_free (buffer->priv);
    buffer->priv = NULL;

    G_OBJECT_CLASS (moo_text_buffer_parent_class)->finalize (object);
}


void
moo_text_buffer_set_lang (MooTextBuffer *buffer,
                          MooEditLang   *lang)
{
    GtkTextTagTable *table;

    g_return_if_fail (MOO_IS_TEXT_BUFFER (buffer));

    if (lang == buffer->priv->lang)
        return;

    table = gtk_text_buffer_get_tag_table (GTK_TEXT_BUFFER (buffer));
    gtk_source_tag_table_remove_source_tags (GTK_SOURCE_TAG_TABLE (table));

    if (buffer->priv->lang)
    {
        g_object_unref (buffer->priv->lang);
        buffer->priv->lang = NULL;
    }

    if (lang)
    {
        GSList *tags;
        gunichar escape_char;

        buffer->priv->lang = lang;
        g_object_ref (buffer->priv->lang);

        tags = moo_edit_lang_get_tags (lang);
        gtk_source_tag_table_add_tags (GTK_SOURCE_TAG_TABLE (table), tags);
        g_slist_foreach (tags, (GFunc) g_object_unref, NULL);
        g_slist_free (tags);

        escape_char = moo_edit_lang_get_escape_char (lang);
        gtk_source_buffer_set_escape_char (GTK_SOURCE_BUFFER (buffer),
                                           escape_char);

        moo_text_buffer_set_brackets (buffer, moo_edit_lang_get_brackets (lang));
    }
    else
    {
        gtk_source_buffer_set_escape_char (GTK_SOURCE_BUFFER (buffer), '\\');
        moo_text_buffer_set_brackets (buffer, "{}()[]");
    }
}


static gboolean
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
moo_text_iter_find_matching_bracket (GtkTextIter *iter)
{
    int addition = 0;
    gunichar *same_direction = NULL, *inverse_direction = NULL;
    guint to_find;
    gunichar bracket = gtk_text_iter_get_char (iter);
    gunichar bracket_to_find = 0;
    const GtkTextTag *tag;
    guint stack = 0;
    GtkTextIter b;

    MooTextBuffer *buffer = MOO_TEXT_BUFFER (gtk_text_iter_get_buffer (iter));
    g_return_val_if_fail (MOO_IS_TEXT_BUFFER (buffer), MOO_BRACKET_MATCH_NOT_AT_BRACKET);

    for (to_find = 0; to_find < buffer->priv->num_brackets; ++to_find)
        if (bracket == buffer->priv->left_brackets[to_find])
    {
        bracket_to_find = buffer->priv->right_brackets[to_find];
        addition = 1;
        inverse_direction = buffer->priv->right_brackets;
        same_direction = buffer->priv->left_brackets;
        break;
    }

    if (to_find == buffer->priv->num_brackets)
        for (to_find = 0; to_find < buffer->priv->num_brackets; ++to_find)
            if (bracket == buffer->priv->right_brackets[to_find])
    {
        bracket_to_find = buffer->priv->left_brackets[to_find];
        addition = -1;
        same_direction = buffer->priv->right_brackets;
        inverse_direction = buffer->priv->left_brackets;
        break;
    }

    if (to_find == buffer->priv->num_brackets)
        return MOO_BRACKET_MATCH_NOT_AT_BRACKET;

    tag = gtk_source_iter_has_syntax_tag (iter);
    stack = 0;
    b = *iter;

    while (gtk_text_iter_forward_chars (&b, addition))
    {
        if (tag == gtk_source_iter_has_syntax_tag (&b))
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
    bracket_match = moo_text_iter_find_matching_bracket (&iter[2]); /* TODO: max chars*/

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
        g_signal_emit (buffer, signals[CURSOR_MOVED], 0, iter);
}


static void
moo_text_buffer_insert_text (GtkTextBuffer      *text_buffer,
                             GtkTextIter        *pos,
                             const gchar        *text,
                             gint                length)
{
    MooTextBuffer *buffer = MOO_TEXT_BUFFER (text_buffer);

    if (GTK_TEXT_BUFFER_CLASS(moo_text_buffer_parent_class)->insert_text)
        GTK_TEXT_BUFFER_CLASS(moo_text_buffer_parent_class)->insert_text (text_buffer, pos, text, length);

    g_signal_emit (buffer, signals[CURSOR_MOVED], 0, pos);

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

    if (GTK_TEXT_BUFFER_CLASS(moo_text_buffer_parent_class)->delete_range)
        GTK_TEXT_BUFFER_CLASS(moo_text_buffer_parent_class)->delete_range (text_buffer, start, end);

    g_signal_emit (buffer, signals[CURSOR_MOVED], 0, start);

    if (buffer->priv->has_text)
    {
        buffer->priv->has_text = moo_text_buffer_has_text (buffer);
        if (!buffer->priv->has_text)
            g_object_notify (G_OBJECT (buffer), "has-text");
    }
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
        case PROP_BRACKET_MATCH_STYLE:
            moo_text_buffer_set_bracket_match_style (buffer,
                                                     g_value_get_boxed (value));
            break;

        case PROP_CHECK_BRACKETS:
            moo_text_buffer_set_check_brackets (buffer,
                                                g_value_get_boolean (value));
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


void
moo_text_buffer_set_bracket_match_style (MooTextBuffer           *buffer,
                                         const GtkSourceTagStyle *style)
{
    GtkSourceTagStyle *freeme = NULL;
    const GdkColor *bg = NULL, *fg = NULL;
    int weight = PANGO_WEIGHT_NORMAL;
    PangoUnderline underline = PANGO_UNDERLINE_NONE;
    PangoStyle italic = PANGO_STYLE_NORMAL;

    g_return_if_fail (MOO_IS_TEXT_BUFFER (buffer));

    if (!style)
    {
        freeme = gtk_source_tag_style_new ();
        freeme->is_default = FALSE;
        freeme->mask = GTK_SOURCE_TAG_STYLE_USE_BACKGROUND;
        gdk_color_parse ("yellow", &freeme->background);
        freeme->italic = FALSE;
        freeme->bold = FALSE;
        freeme->underline = FALSE;
        freeme->strikethrough = FALSE;

        style = freeme;
    }

    if (!buffer->priv->correct_match_tag)
        buffer->priv->correct_match_tag =
                gtk_text_buffer_create_tag (GTK_TEXT_BUFFER (buffer), NULL, NULL);

    if (style->mask & GTK_SOURCE_TAG_STYLE_USE_BACKGROUND)
        bg = &style->background;
    if (style->mask & GTK_SOURCE_TAG_STYLE_USE_FOREGROUND)
        bg = &style->foreground;
    if (style->italic)
        italic = PANGO_STYLE_ITALIC;
    if (style->bold)
        weight = PANGO_WEIGHT_BOLD;
    if (style->underline)
        underline = PANGO_UNDERLINE_SINGLE;

    g_object_set (buffer->priv->correct_match_tag,
                  "foreground-gdk", fg,
                  "background-gdk", bg,
                  "style", italic,
                  "weight", weight,
                  "underline", underline,
                  "strikethrough", style->strikethrough,
                  NULL);

    g_object_notify (G_OBJECT (buffer), "bracket-match-style");

    if (freeme)
        gtk_source_tag_style_free (freeme);
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
