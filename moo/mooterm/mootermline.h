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
#define DECALN_CHAR 'S'

extern MooTermTextAttr MOO_TERM_ZERO_ATTR;


/* FALSE if equal */
#define ATTR_CMP(a1, a2)                                \
    ((a1)->mask != (a2)->mask ||                        \
     (((a1)->mask & MOO_TERM_TEXT_FOREGROUND) &&        \
            ((a1)->foreground != (a2)->foreground)) ||  \
     (((a1)->mask & MOO_TERM_TEXT_BACKGROUND) &&        \
            ((a1)->background != (a2)->background)))


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


MooTermLine *moo_term_line_new              (guint           len);
void         moo_term_line_free             (MooTermLine    *line);
guint        moo_term_line_len              (MooTermLine    *line);
void         moo_term_line_set_len          (MooTermLine    *line,
                                             guint           len);
void         moo_term_line_erase            (MooTermLine    *line);
void         moo_term_line_erase_range      (MooTermLine    *line,
                                             guint           pos,
                                             guint           len,
                                             MooTermTextAttr *attr);
void         moo_term_line_delete_range     (MooTermLine    *line,
                                             guint           pos,
                                             guint           len);
void         moo_term_line_set_unichar      (MooTermLine    *line,
                                             guint           pos,
                                             gunichar        c,
                                             guint           num,
                                             MooTermTextAttr *attr,
                                             guint           width);
void         moo_term_line_insert_unichar   (MooTermLine    *line,
                                             guint           pos,
                                             gunichar        c,
                                             guint           num,
                                             MooTermTextAttr *attr,
                                             guint           width);
gunichar     moo_term_line_get_unichar      (MooTermLine    *line,
                                             guint           col);
guint        moo_term_line_get_chars        (MooTermLine    *line,
                                             char           *buf,
                                             guint           first,
                                             int             len);


G_END_DECLS

#endif /* MOOTERM_MOOTERMLINE_H */
