/*
 *   mootermline.h
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

#ifndef __MOO_TERM_LINE_H__
#define __MOO_TERM_LINE_H__

#include "mooterm/mootermbuffer.h"

G_BEGIN_DECLS


typedef struct _MooTermCell         MooTermCell;
typedef struct _MooTermCellArray    MooTermCellArray;
typedef struct _MooTermLine         MooTermLine;

#define EMPTY_CHAR  ' '
#define DECALN_CHAR 'S'

extern MooTermTextAttr _MOO_TERM_ZERO_ATTR;


/* FALSE if equal */
#define ATTR_CMP(a1, a2)                                \
    ((a1)->mask != (a2)->mask ||                        \
     (((a1)->mask & MOO_TERM_TEXT_FOREGROUND) &&        \
            ((a1)->foreground != (a2)->foreground)) ||  \
     (((a1)->mask & MOO_TERM_TEXT_BACKGROUND) &&        \
            ((a1)->background != (a2)->background)))


struct _MooTermCell {
    gunichar        ch;
    MooTermTextAttr attr;
};

struct _MooTermCellArray {
    MooTermCell *data;
    guint        len;
};

struct _MooTermLine {
    MooTermCellArray *cells;
};


MooTermLine *_moo_term_line_new             (guint           len);
void         _moo_term_line_free            (MooTermLine    *line);

guint        _moo_term_line_len             (MooTermLine    *line);

MooTermTextAttr *_moo_term_line_attr        (MooTermLine    *line,
                                             guint           index);
MooTermCell *_moo_term_line_cell            (MooTermLine    *line,
                                             guint           index);
gunichar     _moo_term_line_get_unichar     (MooTermLine    *line,
                                             guint           col);

void         _moo_term_line_set_len         (MooTermLine    *line,
                                             guint           len);

void         _moo_term_line_erase           (MooTermLine    *line);
void         _moo_term_line_erase_range     (MooTermLine    *line,
                                             guint           pos,
                                             guint           len,
                                             MooTermTextAttr *attr);
void         _moo_term_line_delete_range    (MooTermLine    *line,
                                             guint           pos,
                                             guint           len);
void         _moo_term_line_set_unichar     (MooTermLine    *line,
                                             guint           pos,
                                             gunichar        c,
                                             guint           num,
                                             MooTermTextAttr *attr,
                                             guint           width);
void         _moo_term_line_insert_unichar  (MooTermLine    *line,
                                             guint           pos,
                                             gunichar        c,
                                             guint           num,
                                             MooTermTextAttr *attr,
                                             guint           width);
guint        _moo_term_line_get_chars       (MooTermLine    *line,
                                             char           *buf,
                                             guint           first,
                                             int             len);


G_END_DECLS

#endif /* __MOO_TERM_LINE_H__ */
