/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   moolinebuffer.c
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
#include <string.h>


static void     invalidate_line (LineBuffer *line_buf,
                                 Line       *line,
                                 int         index);


LineBuffer*
moo_line_buffer_new (void)
{
    LineBuffer *buf = g_new0 (LineBuffer, 1);
    buf->tree = moo_text_btree_new ();
    BUF_SET_CLEAN (buf);
    return buf;
}


Line*
moo_line_buffer_insert (LineBuffer     *line_buf,
                        int             index)
{
    Line *line = moo_text_btree_insert (line_buf->tree, index);
    invalidate_line (line_buf, line, index);
    return line;
}


void
moo_line_buffer_clamp_invalid (LineBuffer *line_buf)
{
    if (!BUF_CLEAN (line_buf))
    {
        int size = moo_text_btree_size (line_buf->tree);
        AREA_CLAMP (&line_buf->invalid, size);
    }
}


void
moo_line_buffer_split_line (LineBuffer *line_buf,
                            int         line,
                            int         num_new_lines)
{
    Line *l;
    GSList *tags;

    moo_text_btree_insert_range (line_buf->tree, line + 1, num_new_lines);

    l = moo_line_buffer_get_line (line_buf, line);
    invalidate_line (line_buf, l, line);
    tags = g_slist_copy (l->hl_info->tags);
    g_slist_foreach (tags, (GFunc) g_object_ref, NULL);

    l = moo_line_buffer_get_line (line_buf, line + num_new_lines);
    invalidate_line (line_buf, l, line + num_new_lines);
    g_assert (l->hl_info->tags == NULL);
    l->hl_info->tags = tags;
}


void
moo_line_buffer_delete (LineBuffer *line_buf,
                        int         first,
                        int         num)
{
    Line *line = moo_line_buffer_get_line (line_buf, first + num - 1);
    GSList *old_tags = line->hl_info->tags;
    line->hl_info->tags = NULL;

    moo_text_btree_delete_range (line_buf->tree, first, num);

    if (first > 0)
    {
        line = moo_line_buffer_get_line (line_buf, first - 1);
        line->hl_info->tags = g_slist_concat (line->hl_info->tags, old_tags);
    }
    else
    {
        g_slist_foreach (old_tags, (GFunc) g_object_unref, NULL);
        g_slist_free (old_tags);
    }

    if (first > 0)
        moo_line_buffer_invalidate (line_buf, first - 1);
}


static void
invalidate_line_one (Line *line)
{
    moo_line_erase_segments (line);
    line->hl_info->start_node = NULL;
    line->hl_info->tags_applied = FALSE;
}

void
moo_line_buffer_invalidate (LineBuffer *line_buf,
                            int         index)
{
    invalidate_line (line_buf, moo_text_btree_get_data (line_buf->tree, index), index);
}


static void
invalidate_line (LineBuffer *line_buf,
                 Line       *line,
                 int         index)
{
    invalidate_line_one (line);

    if (line_buf->invalid.empty)
    {
        line_buf->invalid.empty = FALSE;
        line_buf->invalid.first = index;
        line_buf->invalid.last = index;
    }
    else
    {
        line_buf->invalid.first = MIN (line_buf->invalid.first, index);
        line_buf->invalid.last = MAX (line_buf->invalid.last, index);
        moo_line_buffer_clamp_invalid (line_buf);
    }
}


void
moo_line_buffer_invalidate_all (LineBuffer *line_buf)
{
    moo_line_buffer_invalidate (line_buf, 0);
    AREA_SET (&line_buf->invalid, moo_text_btree_size (line_buf->tree));
}


void
moo_line_erase_segments (Line *line)
{
    g_assert (line != NULL);
    line->hl_info->n_segments = 0;
}


void
moo_line_add_segment (Line           *line,
                      int             len,
                      CtxNode        *ctx_node,
                      CtxNode        *match_node,
                      MooRule        *rule)
{
    HLInfo *info = line->hl_info;

    if (info->n_segments == info->n_segments_alloc__)
    {
        info->n_segments_alloc__ = MAX (2, 1.5 * info->n_segments_alloc__);
        info->segments = g_realloc (info->segments, info->n_segments_alloc__ * sizeof (Segment));
    }

    info->segments[info->n_segments].len = len;
    info->segments[info->n_segments].ctx_node = ctx_node;
    info->segments[info->n_segments].match_node = match_node;
    info->segments[info->n_segments].rule = rule;

    info->n_segments++;
}


Line*
moo_line_buffer_get_line (LineBuffer *line_buf,
                          int         index)
{
    return moo_text_btree_get_data (line_buf->tree, index);
}


void
moo_line_buffer_free (LineBuffer *line_buf)
{
    if (line_buf)
    {
        moo_text_btree_free (line_buf->tree);
        g_free (line_buf);
    }
}
