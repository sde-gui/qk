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
#include "mooterm/mootermline-private.h"
#include <string.h>


#define EMPTY_CHAR MOO_TERM_EMPTY_CHAR
static MooTermTextAttr ZERO_ATTR = MOO_TERM_ZERO_ATTR;
static MooTermCell EMPTY_CELL = {EMPTY_CHAR, MOO_TERM_ZERO_ATTR};

#define CELLMOVE(dest__,src__,n__) memmove (dest__, src__, (n__) * sizeof (MooTermCell))
#define CELLCPY(dest__,src__,n__)  memcpy (dest__, src__, (n__) * sizeof (MooTermCell))
#define PTRCPY(dest__,src__,n__) memcpy (dest__, src__, (n__) * sizeof (gpointer))
#define PTRSET(s__,c__,n__) memset (s__, c__, (n__) * sizeof (gpointer))

static GMemChunk *LINE_MEM_CHUNK__ = NULL;


static gboolean line_has_tag_in_range   (MooTermLine    *line,
                                         MooTermTag     *tag,
                                         guint           start,
                                         guint           len);
static gboolean line_has_tag            (MooTermLine    *line,
                                         MooTermTag     *tag);

static int ptr_cmp (gconstpointer a,
                    gconstpointer b)
{
    return a < b ? -1 : (a > b ? 1 : 0);
}


void
_moo_term_line_mem_init (void)
{
    if (!LINE_MEM_CHUNK__)
        LINE_MEM_CHUNK__ = g_mem_chunk_create (MooTermLine, 512, G_ALLOC_AND_FREE);
}


MooTermLine*
_moo_term_line_new (guint            width,
                    MooTermTextAttr  attr)
{
    MooTermLine *line;
    guint i;

    g_assert (width != 0);
    g_assert (width <= MOO_TERM_LINE_MAX_LEN);

    line = g_chunk_new0 (MooTermLine, LINE_MEM_CHUNK__);
    line->cells = g_new (MooTermCell, width);
    line->width = line->width_allocd__ = width;

    for (i = 0; i < width; ++i)
    {
        line->cells[i].ch = EMPTY_CHAR;
        line->cells[i].attr = attr;
    }

    return line;
}


void
_moo_term_line_free (MooTermLine *line,
                     gboolean     remove_tags)
{
    if (line)
    {
        if (line->tags)
        {
            guint i;

            if (remove_tags)
            {
                GSList *tags = NULL, *l;
                MooTermTag *tag;

                for (i = 0; i < line->width; ++i)
                {
                    if (line->tags[i])
                    {
                        for (l = line->tags[i]; l != NULL; l = l->next)
                            tags = g_slist_prepend (tags, l->data);
                        g_slist_free (line->tags[i]);
                    }
                }

                tags = g_slist_sort (tags, ptr_cmp);

                for (tag = NULL, l = tags; l != NULL; l = l->next)
                {
                    if (tag != l->data)
                    {
                        tag = l->data;
                        _moo_term_tag_remove_line (tag, line);
                    }
                }

                g_slist_free (tags);
            }
            else
            {
                for (i = 0; i < line->width; ++i)
                    g_slist_free (line->tags[i]);
            }

            g_free (line->tags);
        }

        g_free (line->cells);
        g_chunk_free (line, LINE_MEM_CHUNK__);
    }
}


void
_moo_term_line_resize (MooTermLine    *line,
                       guint           width,
                       MooTermTextAttr attr)
{
    guint i;

    g_assert (line != NULL);
    g_assert (width != 0);
    g_assert (width < MOO_TERM_LINE_MAX_LEN);

    if (width == line->width)
        return;

    line->wrapped = FALSE;

    if (width <= line->width)
    {
        if (line->tags)
        {
            GSList *removed = NULL, *l;
            MooTermTag *tag;

            for (i = width; i < line->width; ++i)
            {
                if (line->tags[i])
                {
                    for (l = line->tags[i]; l != NULL; l = l->next)
                        removed = g_slist_prepend (removed, l->data);

                    g_slist_free (line->tags[i]);
                }
            }

            removed = g_slist_sort (removed, ptr_cmp);

            for (tag = NULL, l = removed; l != NULL; l = l->next)
            {
                if (tag != l->data)
                {
                    tag = l->data;

                    if (!line_has_tag_in_range (line, tag, 0, width))
                        _moo_term_tag_remove_line (tag, line);
                }
            }

            if (!line_has_tag_in_range (line, NULL, 0, width))
            {
                g_free (line->tags);
                line->tags = NULL;
            }

            g_slist_free (removed);
        }
    }
    else
    {
        if (line->width_allocd__ < width)
        {
            MooTermCell *tmp = g_new (MooTermCell, width);
            CELLCPY (tmp, line->cells, line->width);
            g_free (line->cells);
            line->cells = tmp;

            if (line->tags)
            {
                GSList **tmp2 = g_new (GSList*, width);
                PTRCPY (tmp2, line->tags, line->width);
                g_free (line->tags);
                line->tags = tmp2;
            }

            line->width_allocd__ = width;
        }

        for (i = line->width; i < width; ++i)
        {
            line->cells[i].ch = EMPTY_CHAR;
            line->cells[i].attr = attr;
        }

        if (line->tags)
            PTRSET (&line->tags[line->width], 0,
                     width - line->width);
    }

    line->width = width;
    line->len = MIN (line->len, width);
    line->wrapped = FALSE;
}


void
_moo_term_line_erase_range (MooTermLine    *line,
                            guint           pos,
                            guint           len,
                            MooTermTextAttr attr)
{
    guint i;
    guint last = MIN ((guint) (line->width - 1), pos + len);

    if ((int) last == line->width - 1)
        line->len = MIN (line->len, pos);

    for (i = pos; i < last; ++i)
    {
        line->cells[i].ch = EMPTY_CHAR;
        line->cells[i].attr = attr;
    }
}


void
_moo_term_line_delete_range (MooTermLine    *line,
                             guint           pos,
                             guint           len,
                             MooTermTextAttr attr)
{
    guint i;

    g_assert (line != NULL);
    g_assert (len != 0);
    g_assert (pos < line->width);

    line->wrapped = FALSE;

    if (pos < line->len)
    {
        if (len + pos >= line->len)
            line->len = pos;
        else
            line->len -= pos;
    }

    if (len + pos >= line->width)
    {
        for (i = pos; i < line->width; ++i)
        {
            line->cells[i].ch = EMPTY_CHAR;
            line->cells[i].attr = attr;
        }
    }
    else
    {
        CELLMOVE (&line->cells[pos], &line->cells[pos+len],
                   line->width - (pos + len));
        for (i = line->width - len; i < line->width; ++i)
        {
            line->cells[i].ch = EMPTY_CHAR;
            line->cells[i].attr = attr;
        }
    }
}


void
_moo_term_line_set_unichar (MooTermLine    *line,
                            guint           pos,
                            gunichar        c,
                            guint           num,
                            MooTermTextAttr attr)
{
    guint i;

    g_assert (line != NULL);
    g_assert (num != 0);
    g_assert (pos < line->width);

    if (pos + num > line->width)
        num = line->width - pos;

    if (!c)
        c = EMPTY_CHAR;

    if (c != EMPTY_CHAR)
        line->len = MAX (line->len, pos + num);
    g_assert (line->len <= line->width);

    for (i = pos; i < pos + num; ++i)
    {
        line->cells[i].ch = c;
        line->cells[i].attr = attr;
    }
}


void
_moo_term_line_insert_unichar (MooTermLine    *line,
                               guint           pos,
                               gunichar        c,
                               guint           num,
                               MooTermTextAttr attr)
{
    guint i;

    g_assert (line != NULL);
    g_assert (num != 0);
    g_assert (pos < line->width);

    if (!c)
        c = EMPTY_CHAR;

    line->wrapped = FALSE;

    if (c != EMPTY_CHAR)
    {
        if (pos + num >= line->width)
            line->len = line->width;
        else if (pos >= line->len)
            line->len = pos + num;
        else
            line->len += num;

        g_assert (line->len <= line->width);
    }

    if (pos + num >= line->width)
    {
        for (i = pos; i < line->width; ++i)
        {
            line->cells[i].ch = c;
            line->cells[i].attr = attr;
        }
    }
    else
    {
        CELLMOVE (&line->cells[pos+num], &line->cells[pos],
                   line->width - (pos+num));

        for (i = pos; i < pos+num; ++i)
        {
            line->cells[i].ch = c;
            line->cells[i].attr = attr;
        }
    }
}


guint
_moo_term_line_get_chars (MooTermLine    *line,
                          char           *buf,
                          guint           first,
                          int             len)
{
    guint i;
    guint res = 0;

    g_assert (line != NULL);

    if (!len || first >= line->width)
        return 0;

    if (len < 0 || first + len > line->width)
        len = line->width - first;

    for (i = first; i < first + len; ++i)
    {
        gunichar c = _moo_term_line_get_char (line, i);
        guint l = g_unichar_to_utf8 (c, buf);
        buf += l;
        res += l;
    }

    return res;
}


gboolean
_moo_term_line_wrapped_chk__ (MooTermLine *line)
{
    g_assert (line != NULL);
    return MOO_TERM_LINE_WRAPPED__ (line);
}


guint
_moo_term_line_len_chk__ (MooTermLine *line)
{
    g_assert (line != NULL);
    return MOO_TERM_LINE_LEN__ (line);
}

guint
moo_term_line_len (MooTermLine    *line)
{
    g_return_val_if_fail (line != NULL, 0);
    return MOO_TERM_LINE_LEN__ (line);
}


guint
_moo_term_line_width_chk__ (MooTermLine *line)
{
    g_assert (line != NULL);
    return MOO_TERM_LINE_WIDTH__ (line);
}

gunichar
_moo_term_line_get_char_chk__ (MooTermLine   *line,
                               guint          index_)
{
    g_assert (line != NULL);
    g_assert (index_ < line->width);
    return MOO_TERM_LINE_CHAR__ (line, index_);
}


gunichar
moo_term_line_get_char (MooTermLine    *line,
                        guint           index_)
{
    g_return_val_if_fail (line != NULL, 0);
    return index_ < line->width ? MOO_TERM_LINE_CHAR__ (line, index_) : EMPTY_CHAR;
}


MooTermTextAttr
_moo_term_line_get_attr (MooTermLine    *line,
                         guint           index_)
{
    MooTermTextAttr attr;
    GSList *tags, *l;

    g_assert (line != NULL);
    g_assert (index_ < line->width);

    if (!line->tags || !line->tags[index_])
        return line->cells[index_].attr;

    attr = line->cells[index_].attr;
    tags = line->tags[index_];

    for (l = tags; l != NULL; l = l->next)
    {
        MooTermTag *tag = l->data;
        MooTermTextAttr tag_attr = tag->attr;

        if (tag_attr.mask & MOO_TERM_TEXT_REVERSE)
        {
            if (attr.mask & MOO_TERM_TEXT_REVERSE)
                attr.mask &= ~MOO_TERM_TEXT_REVERSE;
            else
                attr.mask |= MOO_TERM_TEXT_REVERSE;
        }

        if (tag_attr.mask & MOO_TERM_TEXT_BLINK)
            attr.mask |= MOO_TERM_TEXT_BLINK;
        if (tag_attr.mask & MOO_TERM_TEXT_BOLD)
            attr.mask |= MOO_TERM_TEXT_BOLD;
        if (tag_attr.mask & MOO_TERM_TEXT_UNDERLINE)
            attr.mask |= MOO_TERM_TEXT_UNDERLINE;

        if (tag_attr.mask & MOO_TERM_TEXT_FOREGROUND)
        {
            attr.mask |= MOO_TERM_TEXT_FOREGROUND;
            attr.foreground = tag_attr.foreground;
        }

        if (tag_attr.mask & MOO_TERM_TEXT_BACKGROUND)
        {
            attr.mask |= MOO_TERM_TEXT_BACKGROUND;
            attr.background = tag_attr.background;
        }
    }

    return attr;
}


MooTermCell*
_moo_term_line_get_cell_chk__ (MooTermLine    *line,
                               guint           index_)
{
    g_assert (line != NULL);
    g_assert (index_ < line->width);
    return MOO_TERM_LINE_CELL__ (line, index_);
}


GSList*
_moo_term_line_get_tags_chk__ (MooTermLine    *line,
                               guint           index_)
{
    g_assert (line != NULL);
    g_assert (index_ < line->width);
    return MOO_TERM_LINE_TAGS__ (line, index_);
}


void
_moo_term_line_apply_tag (MooTermLine    *line,
                          MooTermTag     *tag,
                          guint           start,
                          guint           len)
{
    guint i;

    g_assert (line != NULL);
    g_assert (start < line->width);
    g_assert (len > 0);
    g_assert (start + len <= line->width);
    g_assert (MOO_IS_TERM_TAG (tag));

    if (!line->tags)
        line->tags = g_new0 (GSList*, line->width_allocd__);

    for (i = start; i < start + len; ++i)
        line->tags[i] = g_slist_append (line->tags[i], tag);

    _moo_term_tag_add_line (tag, line);
}


static gboolean
line_has_tag (MooTermLine    *line,
              MooTermTag     *tag)
{
    guint i;

    if (!line->tags)
        return FALSE;

    if (tag)
    {
        for (i = 0; i < line->width; ++i)
            if (g_slist_find (line->tags[i], tag))
                return TRUE;
    }
    else
    {
        for (i = 0; i < line->width; ++i)
            if (line->tags[i])
                return TRUE;
    }

    return FALSE;
}


static gboolean
line_has_tag_in_range (MooTermLine    *line,
                       MooTermTag     *tag,
                       guint           start,
                       guint           len)
{
    guint i;

    if (!line->tags)
        return FALSE;

    if (tag)
    {
        for (i = start; i < start + len; ++i)
            if (g_slist_find (line->tags[i], tag))
                return TRUE;
    }
    else
    {
        for (i = start; i < start + len; ++i)
            if (line->tags[i])
                return TRUE;
    }

    return FALSE;
}


gboolean
_moo_term_line_has_tag (MooTermLine    *line,
                        MooTermTag     *tag,
                        guint           index_)
{
    if (!line->tags || index_ >= line->width || !line->tags[index_])
        return FALSE;

    if (tag)
        return g_slist_find (line->tags[index_], tag) != NULL;
    else
        return line->tags[index_] != NULL;
}


gboolean
_moo_term_line_get_tag_start (MooTermLine    *line,
                              MooTermTag     *tag,
                              guint          *index_)
{
    if (!_moo_term_line_has_tag (line, tag, *index_))
        return FALSE;

    for ( ; *index_ > 0; (*index_)--)
        if (!_moo_term_line_has_tag (line, tag, *index_ - 1))
            return TRUE;

    return TRUE;
}


gboolean
_moo_term_line_get_tag_end (MooTermLine    *line,
                            MooTermTag     *tag,
                            guint          *index_)
{
    if (*index_ >= line->width || !_moo_term_line_has_tag (line, tag, *index_))
        return FALSE;

    for ( ; *index_ < line->width; (*index_)++)
        if (!_moo_term_line_has_tag (line, tag, *index_))
            return TRUE;

    return TRUE;
}


void
_moo_term_line_remove_tag (MooTermLine    *line,
                           MooTermTag     *tag,
                           guint           start,
                           guint           len)
{
    guint i;

    g_assert (line != NULL);
    g_assert (start < line->width);
    g_assert (len > 0);
    g_assert (start + len <= line->width);
    g_assert (MOO_IS_TERM_TAG (tag));

    if (!line->tags)
        return;

    for (i = start; i < start + len; ++i)
        line->tags[i] = g_slist_remove (line->tags[i], tag);

    if (!line_has_tag (line, tag))
    {
        _moo_term_tag_remove_line (tag, line);

        if (!line_has_tag (line, NULL))
        {
            g_free (line->tags);
            line->tags = NULL;
        }
    }
}


char*
moo_term_line_get_text (MooTermLine    *line,
                        guint           start,
                        guint           len)
{
    GString *text;
    guint i;

    g_return_val_if_fail (line != NULL, NULL);

    if (!len || start >= line->width)
        return g_strdup ("");

    text = g_string_new (NULL);

    if (start + len > line->width)
        len = line->width - start;

    for (i = start; i < start + len; ++i)
        g_string_append_unichar (text, _moo_term_line_get_char (line, i));

    return g_string_free (text, FALSE);
}
