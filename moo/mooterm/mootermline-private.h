/*
 *   mootermline-private.h
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

#ifndef MOOTERM_COMPILATION
#error "This file may not be included"
#endif

#ifndef __MOO_TERM_LINE_PRIVATE_H__
#define __MOO_TERM_LINE_PRIVATE_H__

#include "mooterm/mootermline.h"

G_BEGIN_DECLS


#define MOO_TERM_DECALN_CHAR 'S'
#define MOO_TERM_LINE_MAX_LEN (1 << 16)


void         _moo_term_line_mem_init        (void);

MooTermLine *_moo_term_line_new             (guint           len,
                                             MooTermTextAttr fill);
void         _moo_term_line_free            (MooTermLine    *line,
                                             gboolean        remove_tags);

void         _moo_term_line_resize          (MooTermLine    *line,
                                             guint           len,
                                             MooTermTextAttr fill);

void         _moo_term_line_erase_range     (MooTermLine    *line,
                                             guint           pos,
                                             guint           len,
                                             MooTermTextAttr fill);
void         _moo_term_line_delete_range    (MooTermLine    *line,
                                             guint           pos,
                                             guint           len,
                                             MooTermTextAttr fill);
void         _moo_term_line_set_unichar     (MooTermLine    *line,
                                             guint           pos,
                                             gunichar        c,
                                             guint           num,
                                             MooTermTextAttr attr);
void         _moo_term_line_insert_unichar  (MooTermLine    *line,
                                             guint           pos,
                                             gunichar        c,
                                             guint           num,
                                             MooTermTextAttr attr);

guint        _moo_term_line_get_chars       (MooTermLine    *line,
                                             char           *buf,
                                             guint           first,
                                             int             len);

void         _moo_term_line_apply_tag       (MooTermLine    *line,
                                             MooTermTag     *tag,
                                             guint           start,
                                             guint           len);
void         _moo_term_line_remove_tag      (MooTermLine    *line,
                                             MooTermTag     *tag,
                                             guint           start,
                                             guint           len);
gboolean     _moo_term_line_has_tag         (MooTermLine    *line,
                                             MooTermTag     *tag,
                                             guint           index_);
gboolean     _moo_term_line_get_tag_start   (MooTermLine    *line,
                                             MooTermTag     *tag,
                                             guint          *index_);
gboolean     _moo_term_line_get_tag_end     (MooTermLine    *line,
                                             MooTermTag     *tag,
                                             guint          *index_);

MooTermTextAttr _moo_term_line_get_attr     (MooTermLine    *line,
                                             guint           index_);
guint        _moo_term_line_len_chk         (MooTermLine    *line);
MooTermCell *_moo_term_line_get_cell_chk    (MooTermLine    *line,
                                             guint           index_);
gunichar     _moo_term_line_get_char_chk    (MooTermLine    *line,
                                             guint           index_);
GSList      *_moo_term_line_get_tags_chk    (MooTermLine    *line,
                                             guint           index_);

#define _MOO_TERM_LINE_CELL(line__,index__) (&(line__)->cells[index__])
#define _MOO_TERM_LINE_CHAR(line__,index__) ((line__)->cells[index__].ch)
#define _MOO_TERM_LINE_LEN(line__)          ((line__)->n_cells)
#define _MOO_TERM_LINE_TAGS(line__,index__) ((line__)->tags ? (line__)->tags[index__] : NULL)

#if 1
#define _moo_term_line_get_cell(line__,index__) (_moo_term_line_get_cell_chk(line__, index__))
#define _moo_term_line_get_char(line__,index__) (_moo_term_line_get_char_chk(line__, index__))
#define _moo_term_line_get_tags(line__,index__) (_moo_term_line_get_tags_chk(line__, index__))
#define _moo_term_line_len(line__)              (_moo_term_line_len_chk(line__))
#else
#define _moo_term_line_get_cell(line__,index__) (_MOO_TERM_LINE_CELL (line__,index__))
#define _moo_term_line_get_char(line__,index__) (_MOO_TERM_LINE_CHAR (line__,index__))
#define _moo_term_line_get_tags(line__,index__) (_MOO_TERM_LINE_TAGS (line__,index__))
#define _moo_term_line_len(line__)              (_MOO_TERM_LINE_LEN (line__))
#endif


G_END_DECLS

#endif /* __MOO_TERM_LINE_PRIVATE_H__ */
