/*
 *   mootermtag-private.h
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef MOOTERM_COMPILATION
#error "This file may not be included"
#endif

#ifndef MOO_TERM_TAG_PRIVATE_H
#define MOO_TERM_TAG_PRIVATE_H

#include <mooterm/mootermtag.h>

G_BEGIN_DECLS


struct _MooTermTag {
    GObject parent;
    MooTermTagTable *table;
    char *name;
    MooTermTextAttr attr;
    GSList *lines;
};

struct _MooTermTagClass {
    GObjectClass  parent_class;

    void (*changed) (MooTermTag *tag);
};


void    _moo_term_tag_add_line              (MooTermTag         *tag,
                                             gpointer            line);
void    _moo_term_tag_remove_line           (MooTermTag         *tag,
                                             gpointer            line);


G_END_DECLS

#endif /* MOO_TERM_TAG_PRIVATE_H */
