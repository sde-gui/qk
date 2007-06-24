/*
 *   mooscript-parser.h
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

#ifndef MOO_SCRIPT_PARSER_PRIV_H
#define MOO_SCRIPT_PARSER_PRIV_H

#include "mooscript-node-private.h"

G_BEGIN_DECLS


typedef struct _MSParser MSParser;


int         _ms_script_yyparse          (MSParser   *parser);
int         _ms_script_yylex            (MSParser   *parser);
void        _ms_script_yyerror          (MSParser   *parser,
                                         const char *string);

void        _ms_parser_set_top_node     (MSParser   *parser,
                                         MSNode     *node);
void        _ms_parser_add_node         (MSParser   *parser,
                                         gpointer    node);


G_END_DECLS

#endif /* MOO_SCRIPT_PARSER_PRIV_H */
