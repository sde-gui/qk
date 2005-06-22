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

G_BEGIN_DECLS

#define EMPTY_CHAR ' '

static MooTermTextAttr ZERO_ATTR;

typedef GArray AttrList;

inline static AttrList *attr_list_free (AttrList *attrs)
{
    if (attrs)
        g_array_free (attrs, TRUE);
    return NULL;
}

inline static AttrList *attr_list_new (gulong start,
                                       gulong len,
                                       MooTermTextAttr *attr)
{
    AttrList *a;
    guint i;

    if (!len || !attr || !attr->mask)
        return NULL;

    a = g_array_sized_new (FALSE, FALSE,
                           sizeof (MooTermTextAttr), start + len);
    for (i = 0; i < start; ++i)
        g_array_append_val (a, ZERO_ATTR);
    for (i = start; i < start + len; ++i)
        g_array_append_val (a, *attr);

    return a;
}

inline static MooTermTextAttr *attr_list_get_attr (AttrList *list, gulong i)
{
    if (!list)
        return NULL;
    else if (i >= list->len)
        return NULL;
    else
        return &g_array_index (list, MooTermTextAttr, i);
}

inline static AttrList *attr_list_set_range (AttrList   *list,
                                             gulong      start,
                                             gulong      len,
                                             MooTermTextAttr *attr)
{
    guint i;
    guint old_len;

    if (!len)
        return list;

    if (!list)
        return attr_list_new (start, len, attr);

    if (!attr)
        attr = &ZERO_ATTR;

    old_len = list->len;

    if (start >= old_len)
    {
        for (i = 0; i < start - old_len; ++i)
            g_array_append_val (list, ZERO_ATTR);
        for (i = 0; i < len; ++i)
            g_array_append_val (list, *attr);
    }
    else
    {
        for (i = start; i < MIN (old_len, start + len); ++i)
            g_array_index (list, MooTermTextAttr, i) = *attr;
        if (start + len > old_len)
            for (i = 0; i < start + len - old_len; ++i)
                g_array_append_val (list, *attr);
    }

    return list;
}


/* deletes specified range and shifts back segments following deleted range */
inline static AttrList *attr_list_delete_range  (AttrList   *attrs,
                                                 gulong      start,
                                                 gulong      len)
{
    if (!attrs || !len)
        return attrs;

    if (start > attrs->len)
        return attrs;

    if (start + len >= attrs->len)
    {
        if (!start)
        {
            g_array_free (attrs, TRUE);
            return NULL;
        }
        else
        {
            return g_array_set_size (attrs, start);
        }
    }

    return g_array_remove_range (attrs, start, len);
}


inline static AttrList *attr_list_insert_range  (AttrList   *list,
                                                 gulong      pos,
                                                 gulong      len,
                                                 gulong      line_len,
                                                 MooTermTextAttr *attr)
{
    guint i;

    if (!len)
        return list;

    if (!list)
        return attr_list_new (pos, len, attr);

    if (pos >= list->len)
        return attr_list_set_range (list, pos, len, attr);

    if (!attr)
        attr = &ZERO_ATTR;

    for (i = 0; i < len; ++i)
        g_array_insert_val (list, i, *attr);

    if (list->len > line_len)
        g_array_set_size (list, line_len);

    return list;
}

static AttrList *attr_list_set_line_len (AttrList *attr,
                                         gulong    len)
{
    if (!attr || !len || attr->len <= len)
        return attr;
    else
        return g_array_set_size (attr, len);
}


typedef struct {
    GByteArray  *text;
    AttrList    *attrs;
} MooTermLine;


inline static MooTermLine *term_line_init (MooTermLine *line, gulong len)
{
    line->text = g_byte_array_sized_new (len);
    line->attrs = NULL;
    return line;
}

inline static void term_line_destroy (MooTermLine *line)
{
    if (line)
    {
        g_byte_array_free (line->text, TRUE);
        attr_list_free (line->attrs);
    }
}

inline static char *term_line_chars (MooTermLine *line)
{
    return (char*)line->text->data;
}

inline static char term_line_get_char (MooTermLine *line, gulong i)
{
    if (i >= line->text->len)
        return EMPTY_CHAR;
    else
        return line->text->data[i];
}

inline static gulong term_line_len (MooTermLine *line)
{
    return line->text->len;
}

inline static void term_line_set_len (MooTermLine *line, gulong len)
{
    if (line->text->len > len)
        g_byte_array_set_size (line->text, len);
    line->attrs = attr_list_set_line_len (line->attrs, len);
}

inline static MooTermTextAttr *term_line_get_attr (MooTermLine *line, gulong i)
{
    return attr_list_get_attr (line->attrs, i);
}

inline static void term_line_erase (MooTermLine   *line)
{
    g_byte_array_set_size (line->text, 0);
    line->attrs = attr_list_free (line->attrs);
}


inline static void term_line_erase_range    (MooTermLine   *line,
                                             gulong         start,
                                             gulong         len,
                                             MooTermTextAttr *attr)
{
    if (start >= line->text->len)
        return;

    if (start + len > line->text->len)
        len = line->text->len - start;

    memset (line->text->data + start, EMPTY_CHAR, len);
    line->attrs = attr_list_set_range (line->attrs, start, len, attr);
}


inline static void term_line_delete_range   (MooTermLine   *line,
                                             gulong         start,
                                             gulong         len)
{
    if (start >= line->text->len)
        return;

    if (start + len > line->text->len)
        len = line->text->len - start;

    g_byte_array_remove_range (line->text, start, len);
    line->attrs = attr_list_delete_range (line->attrs, start, len);
}


inline static void term_line_insert_chars   (MooTermLine   *line,
                                             gulong         pos,
                                             char           c,
                                             gulong         num,
                                             gulong         line_len,
                                             MooTermTextAttr *attr)
{
    guint old_len = line->text->len;

    if (pos >= line_len)
        return;
    if (pos + num > line_len)
        num = line_len - pos;

    if (pos >= old_len)
    {
        g_byte_array_set_size (line->text, pos + num);
        memset (line->text->data + old_len, EMPTY_CHAR, pos - old_len);
        memset (line->text->data + pos, c, num);
    }
    else
    {
        g_byte_array_set_size (line->text, old_len + num);
        memmove (line->text->data + (pos + num),
                 line->text->data + pos,
                 old_len - pos);
        memset (line->text->data + pos, c, num);
        if (old_len + num > line_len)
            g_byte_array_set_size (line->text, line_len);
    }

    line->attrs = attr_list_insert_range (line->attrs, pos,
                                          num, line_len, attr);
}


inline static void term_line_insert_char (MooTermLine   *line,
                                          gulong         pos,
                                          char           c,
                                          MooTermTextAttr *attr,
                                          gulong         line_len)
{
    term_line_insert_chars (line, pos, c, 1, line_len, attr);
}

inline static void term_line_set_char (MooTermLine   *line,
                                       gulong         pos,
                                       char           c,
                                       MooTermTextAttr *attr,
                                       gulong         line_len)
{
    if (pos >= line->text->len)
    {
        term_line_insert_char (line, pos, c, attr, line_len);
    }
    else
    {
        line->text->data[pos] = c;
        line->attrs = attr_list_set_range (line->attrs, pos, 1, attr);
    }
}


inline static char *term_line_get_text (MooTermLine   *line)
{
    return g_strndup ((char*)line->text->data, line->text->len);
}


G_END_DECLS

#endif /* MOOTERM_MOOTERMLINE_H */
