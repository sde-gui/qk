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

#ifndef __MOO_TERM_LINE_H__
#define __MOO_TERM_LINE_H__

#include "mooterm/mootermtag.h"

G_BEGIN_DECLS


typedef struct _MooTermCell MooTermCell;
typedef struct _MooTermLine MooTermLine;


#define MOO_TERM_EMPTY_CHAR  ' '
#define MOO_TERM_ZERO_ATTR {0, 0, 0}

/* FALSE if equal */
#define MOO_TERM_TEXT_ATTR_EQUAL(a1__,a2__)             \
    ((a1__).mask == (a2__).mask &&                      \
    (!((a1__).mask & MOO_TERM_TEXT_FOREGROUND) ||       \
        (a1__).foreground == (a2__).foreground) &&      \
    (!((a1__).mask & MOO_TERM_TEXT_BACKGROUND) ||       \
        (a1__).background == (a2__).background))


guint       moo_term_line_len           (MooTermLine    *line);
gunichar    moo_term_line_get_char      (MooTermLine    *line,
                                         guint           index_);

char       *moo_term_line_get_text      (MooTermLine    *line,
                                         guint           index_,
                                         guint           len);


G_END_DECLS

#endif /* __MOO_TERM_LINE_H__ */
