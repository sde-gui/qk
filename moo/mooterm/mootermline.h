/*
 *   mootermline.h
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

#ifndef MOO_TERM_LINE_H
#define MOO_TERM_LINE_H

#include <mooterm/mootermtag.h>

G_BEGIN_DECLS


typedef struct _MooTermLine MooTermLine;

#define MOO_TYPE_TERM_LINE (moo_term_line_get_type ())


GType       moo_term_line_get_type      (void) G_GNUC_CONST;

guint       moo_term_line_len           (MooTermLine    *line);
gunichar    moo_term_line_get_char      (MooTermLine    *line,
                                         guint           index_);

char       *moo_term_line_get_text      (MooTermLine    *line,
                                         guint           index_,
                                         guint           len);


G_END_DECLS

#endif /* MOO_TERM_LINE_H */
