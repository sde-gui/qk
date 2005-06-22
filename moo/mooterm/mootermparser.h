/*
 *   mooterm/mootermparser.h
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

#ifndef MOOTERM_MOOTERMPARSER_H
#define MOOTERM_MOOTERMPARSER_H

#include "mooterm/mootermbuffer.h"

G_BEGIN_DECLS


typedef struct _MooTermParser MooTermParser;

MooTermParser  *moo_term_parser_new     (MooTermBuffer  *buf);
void            moo_term_parser_free    (MooTermParser  *parser);

void            moo_term_parser_parse   (MooTermParser  *parser,
                                         const char     *string,
                                         gssize          len);


G_END_DECLS

#endif /* MOOTERM_MOOTERMPARSER_H */
