/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   moolinebuffer.h
 *
 *   Copyright (C) 2004-2006 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef __MOO_LINE_BUFFER_H__
#define __MOO_LINE_BUFFER_H__

#ifndef MOOEDIT_COMPILATION
#error "This file may not be included"
#endif

#include <gtk/gtktextbuffer.h>
#include "mooedit/mootextbtree.h"
#include "mooedit/mootextbuffer.h"

G_BEGIN_DECLS


typedef struct _CtxNode CtxNode;
typedef struct _LineBuffer LineBuffer;
typedef struct _Segment Segment;
typedef struct _BTData Line;

typedef struct {
    int first;
    int last;
} Interval;

typedef struct {
    int first;
    int last;
    gboolean empty;
} Area;

struct _LineBuffer {
    BTree *tree;
    Area invalid;
};

struct _Segment {
    int len;
    CtxNode *ctx_node;
    CtxNode *match_node;
    MooRule *rule;
};

struct _HLInfo {
    CtxNode *start_node;
    Segment *segments;
    guint n_segments;
    guint n_segments_alloc__;
    GSList *tags; /* tags applied in this line */
    guint tags_applied : 1; /* correct highlighting tags were applied */
};


LineBuffer *moo_line_buffer_new         (void);
void     moo_line_buffer_free           (LineBuffer     *line_buf);

Line    *moo_line_buffer_get_line       (LineBuffer     *line_buf,
                                         int             index);

Line    *moo_line_buffer_insert         (LineBuffer     *line_buf,
                                         int             index);
void     moo_line_buffer_invalidate     (LineBuffer     *line_buf,
                                         int             line);
void     moo_line_buffer_invalidate_all (LineBuffer     *line_buf);
void     moo_line_buffer_clamp_invalid  (LineBuffer     *line_buf);

guint    moo_line_buffer_get_stamp      (LineBuffer     *line_buf);
int      moo_line_buffer_get_line_index (LineBuffer     *line_buf,
                                         Line           *line);

void     moo_line_buffer_add_mark       (LineBuffer     *line_buf,
                                         MooLineMark    *mark,
                                         int             line);
void     moo_line_buffer_remove_mark    (LineBuffer     *line_buf,
                                         MooLineMark    *mark);
void     moo_line_buffer_move_mark      (LineBuffer     *line_buf,
                                         MooLineMark    *mark,
                                         int             line);
GSList  *moo_line_buffer_get_marks_in_range (LineBuffer *line_buf,
                                         int             first_line,
                                         int             last_line);

void     moo_line_buffer_split_line     (LineBuffer     *line_buf,
                                         int             line,
                                         int             num_new_lines,
                                         GtkTextTag     *tag);
void     moo_line_buffer_delete         (LineBuffer     *line_buf,
                                         int             first,
                                         int             num,
                                         int             move_to,
                                         GSList        **moved_marks,
                                         GSList        **deleted_marks);

void     moo_line_erase_segments        (Line           *line);
void     moo_line_add_segment           (Line           *line,
                                         int             len,
                                         CtxNode        *ctx_node,
                                         CtxNode        *match_node,
                                         MooRule        *rule);


#define AREA_SET_EMPTY__(ar__) ((ar__)->empty = TRUE)
#define AREA_IS_EMPTY__(ar__) ((ar__)->empty)

#define AREA_CLAMP(ar__,size__)                                 \
    (ar__)->last = CLAMP ((ar__)->last, 0, size__ - 1);         \
    (ar__)->first = CLAMP ((ar__)->first, 0, (ar__)->last);

#define AREA_SET(ar__,size__)                                   \
    (ar__)->empty = FALSE;                                      \
    (ar__)->first = 0;                                          \
    (ar__)->last = size__ - 1;                                  \

#define LINE_SET_TAGS_APPLIED(line__)                           \
G_STMT_START {                                                  \
    (line__)->hl_info->_tags_applied = TRUE;                    \
    (line__)->hl_info->_dirty = TRUE;                           \
} G_STMT_END

#define LINE_UNSET_TAGS_APPLIED(line__)                         \
G_STMT_START {                                                  \
    (line__)->hl_info->_tags_applied = FALSE;                   \
} G_STMT_END

#define LINE_DIRTY(line__) ((line__)->hl_info->_dirty)
#define LINE_TAGS_APPLIED(line__) ((line__)->hl_info->_tags_applied)

#define BUF_CLEAN(line_buf__) (AREA_IS_EMPTY__ (&(line_buf__)->invalid))
#define BUF_SET_CLEAN(line_buf__) (AREA_SET_EMPTY__ (&(line_buf__)->invalid))


G_END_DECLS

#endif /* __MOO_LINE_BUFFER_H__ */
