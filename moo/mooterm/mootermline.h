/*
 *   mooterm/mootermline.h
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

#ifndef MOOTERM_MOOTERMLINE_H
#define MOOTERM_MOOTERMLINE_H

#ifndef MOOTERM_COMPILATION
#error "This file may not be included directly"
#endif

#include <string.h>
#include <glib.h>

G_BEGIN_DECLS


typedef struct _MooTermCell MooTermCell;
typedef struct _MooTermLine MooTermLine;

#define EMPTY_CHAR  ' '

static MooTermTextAttr ZERO_ATTR;

#define TERM_LINE(ar)           ((MooTermLine*) (ar))
#define TERM_LINE_ARRAY(line)   ((GArray*) (line))

struct _MooTermCell {
    gunichar        ch;
    MooTermTextAttr attr;
};

struct _MooTermLine {
    MooTermCell *data;
    guint        len;
};


inline static MooTermLine *term_line_new (guint len)
{
    return TERM_LINE (g_array_sized_new (FALSE, FALSE,
                                         sizeof (MooTermCell),
                                         len));
}

inline static void term_line_free (MooTermLine *line)
{
    if (line)
        g_array_free (TERM_LINE_ARRAY (line), TRUE);
}


inline static guint term_line_len (MooTermLine *line)
{
    return line->len;
}


inline static void term_line_set_len (MooTermLine *line, guint len)
{
    if (line->len > len)
        g_array_set_size (TERM_LINE_ARRAY (line), len);
}


inline static void term_line_erase (MooTermLine *line)
{
    term_line_set_len (line, 0);
}


inline static void term_line_erase_range    (MooTermLine   *line,
                                             guint          pos,
                                             guint          len)
{
    guint i;

    if (!len)
        return;

    if (pos + len >= line->len)
        return term_line_set_len (line, pos);

    for (i = pos; i < pos + len; ++i)
    {
        line->data[pos].ch = EMPTY_CHAR;
        line->data[pos].attr.mask = 0;
    }
}


inline static void term_line_delete_range   (MooTermLine   *line,
                                             guint          pos,
                                             guint          len)
{
    if (pos >= line->len)
        return;
    else if (pos + len >= line->len)
        return term_line_set_len (line, pos);
    else
        g_array_remove_range (TERM_LINE_ARRAY (line), pos, len);
}


inline static void term_line_set_unichar    (MooTermLine   *line,
                                             guint          pos,
                                             gunichar       c,
                                             guint          num,
                                             MooTermTextAttr *attr,
                                             guint          width)
{
    guint i;

    if (pos >= width)
        return term_line_set_len (line, width);

    if (pos + num >= width)
        num = width - pos;

    if (!attr || !attr->mask)
        attr = &ZERO_ATTR;

    if (!c)
        c = EMPTY_CHAR;

    term_line_set_len (line, width);

    if (pos >= line->len)
    {
        MooTermCell cell = {EMPTY_CHAR, ZERO_ATTR};

        for (i = 0; i < pos - line->len; ++i)
            g_array_append_val (TERM_LINE_ARRAY (line), cell);

        cell.ch = c;
        cell.attr = *attr;

        for (i = 0; i < num; ++i)
            g_array_append_val (TERM_LINE_ARRAY (line), cell);
    }
    else if (pos + num > line->len)
    {
        MooTermCell cell = {c, *attr};

        for (i = pos; i < line->len; ++i)
            g_array_index (TERM_LINE_ARRAY (line), MooTermCell, i) = cell;
        for (i = 0; i < pos + num - line->len; ++i)
            g_array_append_val (TERM_LINE_ARRAY (line), cell);
    }
    else
    {
        MooTermCell cell = {c, *attr};

        for (i = pos; i < pos + num; ++i)
            g_array_index (TERM_LINE_ARRAY (line), MooTermCell, i) = cell;
    }
}


inline static void term_line_insert_unichar (MooTermLine   *line,
                                             guint          pos,
                                             gunichar       c,
                                             guint          num,
                                             MooTermTextAttr *attr,
                                             guint          width)
{
    guint i;

    if (pos >= width)
        return term_line_set_len (line, width);

    if (pos + num >= width)
        return term_line_set_unichar (line, pos, c, num,
                                      attr, width);

    if (!attr || !attr->mask)
        attr = &ZERO_ATTR;

    if (!c)
        c = EMPTY_CHAR;

    term_line_set_len (line, width);

    for (i = 0; i < num; ++i)
    {
        MooTermCell cell = {c, *attr};
        g_array_insert_val (TERM_LINE_ARRAY (line), pos, cell);
    }
}


inline static gunichar term_line_get_unichar (MooTermLine   *line,
                                              guint          col)
{
    return g_array_index (TERM_LINE_ARRAY (line), MooTermCell, col).ch;
}


inline static guint term_line_get_chars (MooTermLine    *line,
                                         char           *buf,
                                         guint           first,
                                         int             len)
{
    guint i;
    guint res = 0;

    if (!len || first >= line->len)
        return 0;

    if (len < 0 || first + len > line->len)
        len = line->len - first;

    for (i = first; i < first + len; ++i)
    {
        gunichar c = term_line_get_unichar (line, i);
        guint l = g_unichar_to_utf8 (c, buf);
        buf += l;
        res += l;
    }

    return res;
}


G_END_DECLS

#endif /* MOOTERM_MOOTERMLINE_H */
