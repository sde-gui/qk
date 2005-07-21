/*
 *   mooterm/mootermline.c
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

#define MOOTERM_COMPILATION
#include "mooterm/mootermline.h"

#define CELLS(ar)               ((MooTermCellArray*) (ar))
#define CELLS_ARRAY(cells)      ((GArray*) (cells))


/* TODO: midnight commander wants erased chars to have current attributes
           but VT510 manual says: "EL clears all character attributes from erased
           character positions. EL works inside or outside the scrolling margins." */
void moo_term_line_erase_range  (MooTermLine     *line,
                                 guint            pos,
                                 guint            len,
                                 MooTermTextAttr *attr)
{
    guint i;

    if (!len || pos >= line->cells->len)
        return;

    for (i = pos; i < pos + len && i < line->cells->len; ++i)
    {
        line->cells->data[i].ch = EMPTY_CHAR;

        if (attr && attr->mask)
            line->cells->data[i].attr = *attr;
        else
            line->cells->data[i].attr.mask = 0;
    }
}


MooTermLine *moo_term_line_new (guint len)
{
    MooTermLine *line = g_new (MooTermLine, 1);

    line->cells = CELLS (g_array_sized_new (FALSE, FALSE,
                                            sizeof (MooTermCell),
                                            len));
    line->wrapped = FALSE;

    return line;
}


void moo_term_line_free (MooTermLine *line)
{
    if (line)
    {
        g_array_free (CELLS_ARRAY (line->cells), TRUE);
        g_free (line);
    }
}


guint moo_term_line_len (MooTermLine *line)
{
    return line->cells->len;
}


void moo_term_line_set_len (MooTermLine *line, guint len)
{
    if (line->cells->len > len)
        g_array_set_size (CELLS_ARRAY (line->cells), len);
}


void moo_term_line_erase (MooTermLine *line)
{
    moo_term_line_set_len (line, 0);
}


void moo_term_line_delete_range (MooTermLine   *line,
                                 guint          pos,
                                 guint          len)
{
    if (pos >= line->cells->len)
        return;
    else if (pos + len >= line->cells->len)
        return moo_term_line_set_len (line, pos);
    else
        g_array_remove_range (CELLS_ARRAY (line->cells), pos, len);
}


gunichar moo_term_line_get_unichar  (MooTermLine   *line,
                                     guint          col)
{
    if (col >= line->cells->len)
        return EMPTY_CHAR;
    else
        return line->cells->data[col].ch;
}


void moo_term_line_set_unichar  (MooTermLine   *line,
                                 guint          pos,
                                 gunichar       c,
                                 guint          num,
                                 MooTermTextAttr *attr,
                                 guint          width)
{
    guint i;

    if (pos >= width)
        return moo_term_line_set_len (line, width);

    if (pos + num >= width)
        num = width - pos;

    if (!attr || !attr->mask)
        attr = &MOO_TERM_ZERO_ATTR;

    if (!c)
        c = EMPTY_CHAR;

    moo_term_line_set_len (line, width);

    if (pos >= line->cells->len)
    {
        MooTermCell cell = {EMPTY_CHAR, MOO_TERM_ZERO_ATTR};
        guint len = line->cells->len;

        for (i = 0; i < pos - len; ++i)
            g_array_append_val (CELLS_ARRAY (line->cells), cell);

        cell.ch = c;
        cell.attr = *attr;

        for (i = 0; i < num; ++i)
            g_array_append_val (CELLS_ARRAY (line->cells), cell);
    }
    else if (pos + num > line->cells->len)
    {
        MooTermCell cell = {c, *attr};
        guint len = line->cells->len;

        for (i = pos; i < len; ++i)
            line->cells->data[i] = cell;
        for (i = 0; i < pos + num - len; ++i)
            g_array_append_val (CELLS_ARRAY (line->cells), cell);
    }
    else
    {
        MooTermCell cell = {c, *attr};

        for (i = pos; i < pos + num; ++i)
            line->cells->data[i] = cell;
    }
}


guint moo_term_line_get_chars   (MooTermLine    *line,
                                 char           *buf,
                                 guint           first,
                                 int             len)
{
    guint i;
    guint res = 0;

    if (!len || first >= line->cells->len)
        return 0;

    if (len < 0 || first + len > line->cells->len)
        len = line->cells->len - first;

    for (i = first; i < first + len; ++i)
    {
        gunichar c = moo_term_line_get_unichar (line, i);
        guint l = g_unichar_to_utf8 (c, buf);
        buf += l;
        res += l;
    }

    return res;
}


void moo_term_line_insert_unichar   (MooTermLine   *line,
                                     guint          pos,
                                     gunichar       c,
                                     guint          num,
                                     MooTermTextAttr *attr,
                                     guint          width)
{
    guint i;

    if (pos >= width)
        return moo_term_line_set_len (line, width);

    if (pos + num >= width)
        return moo_term_line_set_unichar (line, pos, c, num,
                                          attr, width);

    if (!attr || !attr->mask)
        attr = &MOO_TERM_ZERO_ATTR;

    if (!c)
        c = EMPTY_CHAR;

    moo_term_line_set_len (line, width);

    if (pos > line->cells->len)
    {
        guint len = line->cells->len;

        for (i = len; i < pos; ++i)
        {
            MooTermCell cell = {EMPTY_CHAR, MOO_TERM_ZERO_ATTR};
            g_array_append_val (CELLS_ARRAY (line->cells), cell);
        }
    }

    for (i = 0; i < num; ++i)
    {
        MooTermCell cell = {c, *attr};
        g_array_insert_val (CELLS_ARRAY (line->cells), pos, cell);
    }
}


MooTermTextAttr *moo_term_line_attr         (MooTermLine    *line,
                                             guint           index)
{
    g_assert (index < line->cells->len);
    return &line->cells->data[index].attr;
}
