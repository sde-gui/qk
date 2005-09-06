/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *   mooedit.c
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

#define MOOINDENTER_COMPILATION
#include "mooedit/mooindenter.h"
#include <string.h>


/* XXX this doesn't take unicode control chars into account */

static void     moo_indenter_set_property   (GObject        *object,
                                             guint           prop_id,
                                             const GValue   *value,
                                             GParamSpec     *pspec);
static void     moo_indenter_get_property   (GObject        *object,
                                             guint           prop_id,
                                             GValue         *value,
                                             GParamSpec     *pspec);

static void     character_default           (MooIndenter    *indenter,
                                             GtkTextBuffer  *buffer,
                                             gunichar        inserted_char,
                                             GtkTextIter    *where);
static gboolean backspace_default           (MooIndenter    *indenter,
                                             GtkTextBuffer  *buffer);
static void     tab_default                 (MooIndenter    *indenter,
                                             GtkTextBuffer  *buffer);
static void     shift_lines_default         (MooIndenter    *indenter,
                                             GtkTextBuffer  *buffer,
                                             guint           first_line,
                                             guint           last_line,
                                             int             direction);
static void     set_value_default           (MooIndenter    *indenter,
                                             const char     *var,
                                             const char     *value);


enum {
    LAST_SIGNAL
};

// static guint signals[LAST_SIGNAL];

enum {
    PROP_0,
    PROP_TAB_WIDTH,
    PROP_USE_TABS,
    PROP_INDENT
};


/* MOO_TYPE_INDENTER */
G_DEFINE_TYPE (MooIndenter, moo_indenter, G_TYPE_OBJECT)


static void
moo_indenter_class_init (MooIndenterClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->set_property = moo_indenter_set_property;
    gobject_class->get_property = moo_indenter_get_property;

    klass->character = character_default;
    klass->backspace = backspace_default;
    klass->tab = tab_default;
    klass->shift_lines = shift_lines_default;
    klass->set_value = set_value_default;

    g_object_class_install_property (gobject_class,
                                     PROP_TAB_WIDTH,
                                     g_param_spec_uint ("tab-width",
                                             "tab-width",
                                             "tab-width",
                                             1,
                                             G_MAXUINT,
                                             8,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_USE_TABS,
                                     g_param_spec_boolean ("use-tabs",
                                             "use-tabs",
                                             "use-tabs",
                                             TRUE,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_INDENT,
                                     g_param_spec_uint ("indent",
                                             "indent",
                                             "indent",
                                             1,
                                             G_MAXUINT,
                                             8,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
}


static void
moo_indenter_init (G_GNUC_UNUSED MooIndenter *indent)
{
}


static void  moo_indenter_set_property  (GObject        *object,
                                         guint           prop_id,
                                         const GValue   *value,
                                         GParamSpec     *pspec)
{
    MooIndenter *indenter = MOO_INDENTER (object);

    switch (prop_id)
    {
        case PROP_TAB_WIDTH:
            indenter->tab_width = g_value_get_uint (value);
            g_object_notify (object, "tab-width");
            break;

        case PROP_USE_TABS:
            indenter->use_tabs = g_value_get_boolean (value);
            g_object_notify (object, "use-tabs");
            break;

        case PROP_INDENT:
            indenter->indent = g_value_get_uint (value);
            g_object_notify (object, "indent");
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}


static void  moo_indenter_get_property  (GObject        *object,
                                         guint           prop_id,
                                         GValue         *value,
                                         GParamSpec     *pspec)
{
    MooIndenter *indenter = MOO_INDENTER (object);

    switch (prop_id)
    {
        case PROP_TAB_WIDTH:
            g_value_set_uint (value, indenter->tab_width);
            break;

        case PROP_USE_TABS:
            g_value_set_boolean (value, indenter->use_tabs);
            break;

        case PROP_INDENT:
            g_value_set_uint (value, indenter->indent);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}


void
moo_indenter_character (MooIndenter    *indenter,
                        GtkTextBuffer  *buffer,
                        gunichar        inserted_char,
                        GtkTextIter    *where)
{
    g_return_if_fail (MOO_IS_INDENTER (indenter));
    MOO_INDENTER_GET_CLASS(indenter)->character (indenter, buffer, inserted_char, where);
}


gboolean
moo_indenter_backspace (MooIndenter    *indenter,
                        GtkTextBuffer  *buffer)
{
    g_return_val_if_fail (MOO_IS_INDENTER (indenter), FALSE);
    return MOO_INDENTER_GET_CLASS(indenter)->backspace (indenter, buffer);
}


void
moo_indenter_tab (MooIndenter    *indenter,
                  GtkTextBuffer  *buffer)
{
    g_return_if_fail (MOO_IS_INDENTER (indenter));
    MOO_INDENTER_GET_CLASS(indenter)->tab (indenter, buffer);
}


void
moo_indenter_shift_lines (MooIndenter    *indenter,
                          GtkTextBuffer  *buffer,
                          guint           first_line,
                          guint           last_line,
                          int             direction)
{
    g_return_if_fail (MOO_IS_INDENTER (indenter));
    MOO_INDENTER_GET_CLASS(indenter)->shift_lines (indenter, buffer, first_line, last_line, direction);
}


void
moo_indenter_set_value (MooIndenter    *indenter,
                        const char     *var,
                        const char     *value)
{
    g_return_if_fail (MOO_IS_INDENTER (indenter));
    g_return_if_fail (var && var[0]);
    g_return_if_fail (value && value[0]);
    MOO_INDENTER_GET_CLASS(indenter)->set_value (indenter, var, value);
}


MooIndenter*
moo_indenter_default_new (void)
{
    return g_object_new (MOO_TYPE_INDENTER, NULL);
}


MooIndenter*
moo_indenter_get_for_lang (G_GNUC_UNUSED const char *lang)
{
    return moo_indenter_default_new ();
}


MooIndenter*
moo_indenter_get_for_mode (G_GNUC_UNUSED const char *mode)
{
    return moo_indenter_default_new ();
}


/*****************************************************************************/
/* Default implementation
 */

char*
moo_indenter_make_space (MooIndenter    *indenter,
                         guint           len,
                         guint           start)
{
    guint tabs, spaces, delta;
    guint tab_width = indenter->tab_width;
    char *string;

    g_return_val_if_fail (MOO_IS_INDENTER (indenter), NULL);

    if (!len)
        return NULL;

    if (!indenter->use_tabs || len < tab_width)
        return g_strnfill (len, ' ');

    delta = start % tab_width;
    tabs = (len - delta) / tab_width + (delta ? 1 : 0);
    spaces = (len - delta) % tab_width;

    string = g_new (char, tabs + spaces + 1);
    string[tabs + spaces] = 0;

    if (tabs)
        memset (string, '\t', tabs);
    if (spaces)
        memset (string + tabs, ' ', spaces);

    return string;
}


/* computes amount of leading white space on the given line;
   returns TRUE if line contains some non-whitespace chars;
   if returns TRUE, then iter points to the first non-white-space char */
static gboolean
compute_line_offset (GtkTextIter    *iter,
                     guint           tab_width,
                     guint          *offsetp)
{
    guint offset = 0;

    while (!gtk_text_iter_ends_line (iter))
    {
        gunichar c = gtk_text_iter_get_char (iter);

        if (c == ' ')
        {
            offset += 1;
        }
        else if (c == '\t')
        {
            guint add = tab_width - offset % tab_width;
            offset += add;
        }
        else
        {
            *offsetp = offset;
            return TRUE;
        }

        gtk_text_iter_forward_char (iter);
    }

    return FALSE;
}


static void
character_default (MooIndenter    *indenter,
                   GtkTextBuffer  *buffer,
                   gunichar        inserted_char,
                   GtkTextIter    *where)
{
    int i;
    char *indent_string = NULL;

    if (inserted_char != '\n')
        return;

    for (i = gtk_text_iter_get_line (where) - 1; i >= 0; --i)
    {
        guint offset;
        GtkTextIter iter;

        gtk_text_buffer_get_iter_at_line (buffer, &iter, i);

        if (compute_line_offset (&iter, indenter->tab_width, &offset))
        {
            indent_string = moo_indenter_make_space (indenter, offset, 0);
            break;
        }
    }

    if (indent_string)
    {
        gtk_text_buffer_insert (buffer, where, indent_string, -1);
        g_free (indent_string);
    }
}


/* computes offset of start and returns offset or -1 if there are
   non-whitespace characters before start */
static int
compute_offset (const GtkTextIter *start,
                guint              tab_width)
{
    GtkTextIter iter;
    guint offset;

    if (gtk_text_iter_starts_line (start))
        return 0;

    iter = *start;
    gtk_text_iter_set_line_offset (&iter, 0);
    offset = 0;

    while (gtk_text_iter_compare (&iter, start))
    {
        gunichar c = gtk_text_iter_get_char (&iter);

        if (c == ' ')
        {
            offset += 1;
        }
        else if (c == '\t')
        {
            guint add = tab_width - offset % tab_width;
            offset += add;
        }
        else
        {
            return -1;
        }

        gtk_text_iter_forward_char (&iter);
    }

    return offset;
}


/* computes where cursor should jump when backspace is pressed

<-- result -->
              blah blah blah
                      blah
                     | offset
*/
static guint
compute_next_stop (const GtkTextIter *start,
                   guint              tab_width,
                   guint              offset,
                   gboolean           same_line)
{
    GtkTextIter iter;
    guint indent;

    iter = *start;

    gtk_text_iter_set_line_offset (&iter, 0);
    if (!same_line)
    {
        if (gtk_text_iter_is_start (&iter))
            return 0;
        gtk_text_iter_backward_line (&iter);
    }

    while (TRUE)
    {
        if (compute_line_offset (&iter, tab_width, &indent) &&
            indent && indent <= offset)
            return indent;
        if (!gtk_text_iter_backward_line (&iter))
            return 0;
    }
}


static gboolean
backspace_default (MooIndenter    *indenter,
                   GtkTextBuffer  *buffer)
{
    GtkTextIter start, end;
    int offset;
    guint new_offset;
    guint tab_width;
    char *insert = NULL;

    gtk_text_buffer_get_iter_at_mark (buffer, &end, gtk_text_buffer_get_insert (buffer));

    tab_width = indenter->tab_width;
    offset = compute_offset (&end, tab_width);

    if (offset < 0)
        return FALSE;

    if (!offset)
        new_offset = 0;
    else
        new_offset = compute_next_stop (&end, tab_width, offset - 1, FALSE);

    start = end;
    gtk_text_iter_set_line_offset (&start, 0);
    gtk_text_buffer_delete (buffer, &start, &end);
    insert = moo_indenter_make_space (indenter, new_offset, 0);

    if (insert)
    {
        gtk_text_buffer_insert (buffer, &start, insert, -1);
        g_free (insert);
    }

    return TRUE;
}


/* computes visual offset at iter, amount of white space before the iter,
   and makes iter to point to the beginning of the white space, e.g. for
blah   wefwefw
       |
   it would set offset == 7, white_space == 3, and set iter to the first
   space after 'blah'
*/
static void
iter_get_visual_offset (GtkTextIter *iter,
                        guint        tab_width,
                        int         *offsetp,
                        int         *white_spacep)
{
    GtkTextIter start, white_space_start;
    guint offset, white_space;

    start = *iter;
    gtk_text_iter_set_line_offset (&start, 0);
    offset = 0;
    white_space = 0;

    while (gtk_text_iter_compare (&start, iter))
    {
        gunichar c = gtk_text_iter_get_char (&start);

        if (c == ' ' || c == '\t')
        {
            if (!white_space)
                white_space_start = start;
        }
        else
        {
            white_space = 0;
        }

        if (c == '\t')
        {
            guint add = tab_width - offset % tab_width;
            offset += add;
            white_space += add;
        }
        else if (c == ' ')
        {
            offset += 1;
            white_space += 1;
        }
        else
        {
            offset += 1;
        }

        gtk_text_iter_forward_char (&start);
    }

    *offsetp = offset;
    *white_spacep = white_space;
    if (white_space)
        *iter = white_space_start;
}


static void
tab_default (MooIndenter    *indenter,
             GtkTextBuffer  *buffer)
{
    GtkTextIter insert, start;
    int offset, new_offset, white_space;
    guint tab_width = indenter->tab_width;
    guint indent = indenter->indent;
    char *text = NULL;

    gtk_text_buffer_get_iter_at_mark (buffer, &insert, gtk_text_buffer_get_insert (buffer));

    start = insert;
    iter_get_visual_offset (&start, tab_width, &offset, &white_space);

    new_offset = offset + (indent - offset % indent);
    text = moo_indenter_make_space (indenter,
                                    new_offset - offset + white_space,
                                    offset - white_space);

    gtk_text_buffer_delete (buffer, &start, &insert);
    gtk_text_buffer_insert (buffer, &start, text, -1);

    g_free (text);
}


static void
shift_line_forward (MooIndenter   *indenter,
                    GtkTextBuffer *buffer,
                    GtkTextIter   *iter)
{
    char *text;
    guint offset;
    GtkTextIter start;

    if (!compute_line_offset (iter, indenter->tab_width, &offset))
        return;

    if (offset)
    {
        start = *iter;
        gtk_text_iter_set_line_offset (&start, 0);
        gtk_text_buffer_delete (buffer, &start, iter);
    }

    text = moo_indenter_make_space (indenter, offset + indenter->indent, 0);

    if (text)
        gtk_text_buffer_insert (buffer, iter, text, -1);

    g_free (text);
}


static void
shift_line_backward (MooIndenter   *indenter,
                     GtkTextBuffer *buffer,
                     GtkTextIter   *iter)
{
    GtkTextIter end;
    int deleted;
    gunichar c;

    gtk_text_iter_set_line_offset (iter, 0);
    end = *iter;
    deleted = 0;

    while (TRUE)
    {
        if (gtk_text_iter_ends_line (&end))
            break;

        c = gtk_text_iter_get_char (&end);

        if (c == ' ')
        {
            gtk_text_iter_forward_char (&end);
            deleted += 1;
        }
        else if (c == '\t')
        {
            gtk_text_iter_forward_char (&end);
            deleted += indenter->tab_width;
        }
        else
        {
            break;
        }

        if (deleted >= (int) indenter->indent)
            break;
    }

    gtk_text_buffer_delete (buffer, iter, &end);

    deleted -= indenter->indent;

    if (deleted > 0)
    {
        char *text = moo_indenter_make_space (indenter, deleted, 0);
        gtk_text_buffer_insert (buffer, iter, text, -1);
        g_free (text);
    }
}


static void
shift_lines_default (MooIndenter    *indenter,
                     GtkTextBuffer  *buffer,
                     guint           first_line,
                     guint           last_line,
                     int             direction)
{
    guint i;
    GtkTextIter iter;

    for (i = first_line; i <= last_line; ++i)
    {
        gtk_text_buffer_get_iter_at_line (buffer, &iter, i);
        if (direction > 0)
            shift_line_forward (indenter, buffer, &iter);
        else
            shift_line_backward (indenter, buffer, &iter);
    }
}


static void
set_value_default (MooIndenter    *indenter,
                   const char     *var,
                   const char     *value)
{
    if (!g_ascii_strcasecmp (var, "tab-width"))
    {
        guint64 tab_width = g_ascii_strtoull (value, NULL, 10);

        if (tab_width > 0 && tab_width < G_MAXUINT)
            indenter->tab_width = tab_width;
    }
    else if (!g_ascii_strcasecmp (var, "indent-tabs-mode"))
    {
        if (!g_ascii_strcasecmp (value, "t"))
            indenter->use_tabs = TRUE;
        else
            indenter->use_tabs = FALSE;
    }
    else if (!g_ascii_strcasecmp (var, "c-basic-offset"))
    {
        guint64 indent = g_ascii_strtoull (value, NULL, 10);

        if (indent > 0 && indent < G_MAXUINT)
            indenter->indent = indent;
    }
}
