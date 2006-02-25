/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   mooscript-parser.h
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

#ifndef __MOO_SCRIPT_PARSER_H__
#define __MOO_SCRIPT_PARSER_H__

#include <glib-object.h>
#include "mooscript-node.h"
#include "mooscript-context.h"

G_BEGIN_DECLS


typedef struct _MSParser MSParser;


MSNode     *ms_script_parse             (const char *string);


int         _ms_script_yyparse          (MSParser   *parser);
int         _ms_script_yylex            (MSParser   *parser);
void        _ms_script_yyerror          (MSParser   *parser,
                                         const char *string);

void        _ms_parser_set_top_node     (MSParser   *parser,
                                         MSNode     *node);

MSNode     *_ms_parser_node_list_add    (MSParser   *parser,
                                         MSNodeList *list,
                                         MSNode     *node);
MSNode     *_ms_parser_node_command     (MSParser   *parser,
                                         const char *name,
                                         MSNodeList *list);
MSNode     *_ms_parser_node_if_else     (MSParser   *parser,
                                         MSNode     *condition,
                                         MSNode     *then_,
                                         MSNode     *else_);

MSNode     *_ms_parser_node_while       (MSParser   *parser,
                                         MSNode     *cond,
                                         MSNode     *what);
MSNode     *_ms_parser_node_do_while    (MSParser   *parser,
                                         MSNode     *cond,
                                         MSNode     *what);
MSNode     *_ms_parser_node_for         (MSParser   *parser,
                                         MSNode     *var,
                                         MSNode     *list,
                                         MSNode     *what);

MSNode     *_ms_parser_node_assignment  (MSParser   *parser,
                                         MSNodeVar  *var,
                                         MSNode     *val);
MSNode     *_ms_parser_node_binary_op   (MSParser   *parser,
                                         MSBinaryOp  op,
                                         MSNode     *lval,
                                         MSNode     *rval);
MSNode     *_ms_parser_node_unary_op    (MSParser   *parser,
                                         MSUnaryOp   op,
                                         MSNode     *val);
MSNode     *_ms_parser_node_int         (MSParser   *parser,
                                         int         n);
MSNode     *_ms_parser_node_string      (MSParser   *parser,
                                         const char *string);
MSNode     *_ms_parser_node_value_list  (MSParser   *parser,
                                         MSNodeList *list);
MSNode     *_ms_parser_node_value_range (MSParser   *parser,
                                         MSNode     *first,
                                         MSNode     *last);
MSNode     *_ms_parser_node_var         (MSParser   *parser,
                                         const char *name);


G_END_DECLS

#endif /* __MOO_SCRIPT_PARSER_H__ */
