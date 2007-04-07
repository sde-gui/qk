/*
 *   mootermline-private.h
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
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


#define MOO_TERM_EMPTY_CHAR  ' '
#define MOO_TERM_ZERO_ATTR {0, 0, 0}


#define MOO_TERM_TEXT_ATTR_EQUAL(a1__,a2__)             \
    ((a1__).mask == (a2__).mask &&                      \
    (!((a1__).mask & MOO_TERM_TEXT_FOREGROUND) ||       \
        (a1__).foreground == (a2__).foreground) &&      \
    (!((a1__).mask & MOO_TERM_TEXT_BACKGROUND) ||       \
        (a1__).background == (a2__).background))


typedef struct _MooTermCell MooTermCell;

struct _MooTermCell {
    gunichar        ch;
    MooTermTextAttr attr;
};

struct _MooTermLine {
    MooTermCell *cells;
    guint16 width;
    guint16 len;
    guint16 width_allocd__;
    guint wrapped : 1;
    GSList **tags;
};


#define MOO_TERM_DECALN_CHAR 'S'
#define MOO_TERM_LINE_MAX_LEN (1 << 16)


MooTermLine *_moo_term_line_new             (guint           width,
                                             MooTermTextAttr fill);
void         _moo_term_line_free            (MooTermLine    *line,
                                             gboolean        remove_tags);

void         _moo_term_line_resize          (MooTermLine    *line,
                                             guint           width,
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

guint        _moo_term_line_len_chk__       (MooTermLine    *line);
guint        _moo_term_line_width_chk__     (MooTermLine    *line);
MooTermCell *_moo_term_line_get_cell_chk__  (MooTermLine    *line,
                                             guint           index_);
gunichar     _moo_term_line_get_char_chk__  (MooTermLine    *line,
                                             guint           index_);
GSList      *_moo_term_line_get_tags_chk__  (MooTermLine    *line,
                                             guint           index_);
gboolean     _moo_term_line_wrapped_chk__   (MooTermLine    *line);

#define MOO_TERM_LINE_CELL__(line__,index__) (&(line__)->cells[index__])
#define MOO_TERM_LINE_CHAR__(line__,index__) ((line__)->cells[index__].ch)
#define MOO_TERM_LINE_TAGS__(line__,index__) ((line__)->tags ? (line__)->tags[index__] : NULL)
#define MOO_TERM_LINE_LEN__(line__)          ((line__)->len)
#define MOO_TERM_LINE_WIDTH__(line__)        ((line__)->width)
#define MOO_TERM_LINE_WRAPPED__(line__)      ((line__)->wrapped != 0)

#define _moo_term_line_set_wrapped(line__)  (line__)->wrapped = TRUE

#ifdef MOO_DEBUG
#define _moo_term_line_get_cell(line__,index__) (_moo_term_line_get_cell_chk__(line__, index__))
#define _moo_term_line_get_char(line__,index__) (_moo_term_line_get_char_chk__(line__, index__))
#define _moo_term_line_get_tags(line__,index__) (_moo_term_line_get_tags_chk__(line__, index__))
#define _moo_term_line_len(line__)              (_moo_term_line_len_chk__(line__))
#define _moo_term_line_width(line__)            (_moo_term_line_width_chk__(line__))
#define _moo_term_line_wrapped(line__)          (_moo_term_line_wrapped_chk__(line__))
#else
#define _moo_term_line_get_cell(line__,index__) (MOO_TERM_LINE_CELL__ (line__,index__))
#define _moo_term_line_get_char(line__,index__) (MOO_TERM_LINE_CHAR__ (line__,index__))
#define _moo_term_line_get_tags(line__,index__) (MOO_TERM_LINE_TAGS__ (line__,index__))
#define _moo_term_line_len(line__)              (MOO_TERM_LINE_LEN__ (line__))
#define _moo_term_line_width(line__)            (MOO_TERM_LINE_WIDTH__ (line__))
#define _moo_term_line_wrapped(line__)          (MOO_TERM_LINE_WRAPPED__ (line__))
#endif


G_END_DECLS

#endif /* __MOO_TERM_LINE_PRIVATE_H__ */
