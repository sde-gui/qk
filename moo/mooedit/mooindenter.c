/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
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
#include "mooedit/mooedit.h"
#include <string.h>


/* XXX this doesn't take unicode control chars into account */

static GObject *moo_indenter_constructor    (GType           type,
                                             guint           n_construct_properties,
                                             GObjectConstructParam *construct_param);
static void     moo_indenter_finalize       (GObject        *object);
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
static void     set_value_default           (MooIndenter    *indenter,
                                             const char     *var,
                                             const char     *value);
static void     variable_changed            (MooIndenter    *indenter,
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
    PROP_INDENT,
    PROP_DOC
};


/* MOO_TYPE_INDENTER */
G_DEFINE_TYPE (MooIndenter, moo_indenter, G_TYPE_OBJECT)


static void
moo_indenter_class_init (MooIndenterClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->constructor = moo_indenter_constructor;
    gobject_class->finalize = moo_indenter_finalize;
    gobject_class->set_property = moo_indenter_set_property;
    gobject_class->get_property = moo_indenter_get_property;

    klass->character = character_default;
    klass->set_value = set_value_default;

    g_object_class_install_property (gobject_class,
                                     PROP_DOC,
                                     g_param_spec_object ("doc",
                                             "doc",
                                             "doc",
                                             MOO_TYPE_EDIT,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

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


static GObject*
moo_indenter_constructor (GType           type,
                          guint           n_props,
                          GObjectConstructParam *props)
{
    GObject *object;
    MooIndenter *indent;

    object = G_OBJECT_CLASS(moo_indenter_parent_class)->constructor (type, n_props, props);
    indent = MOO_INDENTER (object);

    if (indent->doc)
    {
        guint i;
        static const char *known_vars[] = {
            "tab-width", "indent-tabs-mode",
            "space-indent", "indent-width",
            "c-basic-offset"
        };

        for (i = 0; i < G_N_ELEMENTS (known_vars); ++i)
            moo_indenter_set_value (indent, known_vars[i], NULL);

        for (i = 0; i < G_N_ELEMENTS (known_vars); ++i)
        {
            const char *val = moo_edit_get_var (indent->doc, known_vars[i]);
            if (val)
                moo_indenter_set_value (indent, known_vars[i], val);
        }

        g_signal_connect_swapped (indent->doc, "variable-changed",
                                  G_CALLBACK (variable_changed), indent);
    }

    return object;
}


static void
moo_indenter_finalize (GObject *object)
{
    MooIndenter *indent = MOO_INDENTER (object);

    if (indent->doc)
        g_signal_handlers_disconnect_by_func (indent->doc,
                                              (gpointer) variable_changed,
                                              indent);

    G_OBJECT_CLASS(moo_indenter_parent_class)->finalize (object);
}


static void  moo_indenter_set_property  (GObject        *object,
                                         guint           prop_id,
                                         const GValue   *value,
                                         GParamSpec     *pspec)
{
    MooIndenter *indenter = MOO_INDENTER (object);

    switch (prop_id)
    {
        case PROP_DOC:
            indenter->doc = g_value_get_object (value);
            g_object_notify (object, "doc");
            break;

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
        case PROP_DOC:
            g_value_set_object (value, indenter->doc);
            break;

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


static void
variable_changed (MooIndenter    *indenter,
                  const char     *var,
                  const char     *value)
{
    moo_indenter_set_value (indenter, var, value);
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


void
moo_indenter_set_value (MooIndenter    *indenter,
                        const char     *var,
                        const char     *value)
{
    g_return_if_fail (MOO_IS_INDENTER (indenter));
    g_return_if_fail (var && var[0]);
    MOO_INDENTER_GET_CLASS(indenter)->set_value (indenter, var, value);
}


MooIndenter*
moo_indenter_new (gpointer      doc,
                  G_GNUC_UNUSED const char *name)
{
    g_return_val_if_fail (MOO_IS_EDIT (doc), NULL);
    return g_object_new (MOO_TYPE_INDENTER, "doc", doc, NULL);
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

    if (!indenter->use_tabs)
        return g_strnfill (len, ' ');

    delta = start % tab_width;

    if (!delta)
    {
        tabs = len / tab_width;
        spaces = len % tab_width;
    }
    else if (len < tab_width - delta)
    {
        tabs = 0;
        spaces = len;
    }
    else if (len == tab_width - delta)
    {
        tabs = 1;
        spaces = 0;
    }
    else
    {
        len -= tab_width - delta;
        tabs = len / tab_width + 1;
        spaces = len % tab_width;
    }

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
int
moo_iter_get_blank_offset (const GtkTextIter  *start,
                           guint               tab_width)
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
guint
moo_text_iter_get_prev_stop (const GtkTextIter *start,
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


void
moo_indenter_tab (MooIndenter    *indenter,
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


void
moo_indenter_shift_lines (MooIndenter    *indenter,
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
        guint64 tab_width = value ? g_ascii_strtoull (value, NULL, 10) : 8;

        if (tab_width > 0 && tab_width < G_MAXUINT)
            indenter->tab_width = tab_width;
    }
    else if (!g_ascii_strcasecmp (var, "indent-tabs-mode"))
    {
        indenter->use_tabs = value ? !g_ascii_strcasecmp (value, "t") : TRUE;
    }
    else if (!g_ascii_strcasecmp (var, "space-indent"))
    {
        indenter->use_tabs = value ? !!g_ascii_strcasecmp (value, "on") : FALSE;
    }
    else if (!g_ascii_strcasecmp (var, "c-basic-offset") ||
              !g_ascii_strcasecmp (var, "indent-width"))
    {
        guint64 indent = value ? g_ascii_strtoull (value, NULL, 10) : 8;

        if (indent > 0 && indent < G_MAXUINT)
            indenter->indent = indent;
    }
}
