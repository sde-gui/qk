/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   as-script-parser.h
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

#ifndef __AS_SCRIPT_PARSER_H__
#define __AS_SCRIPT_PARSER_H__

#include <glib-object.h>
#include "as-script-node.h"
#include "as-script-context.h"

G_BEGIN_DECLS


typedef struct _ASParser ASParser;


ASNode     *as_script_parse             (const char *string);


int         _as_script_yyparse          (ASParser   *parser);
int         _as_script_yylex            (ASParser   *parser);
void        _as_script_yyerror          (ASParser   *parser,
                                         const char *string);

void        _as_parser_set_top_node     (ASParser   *parser,
                                         ASNode     *node);

ASNode     *_as_parser_node_list_add    (ASParser   *parser,
                                         ASNodeList *list,
                                         ASNode     *node);
ASNode     *_as_parser_node_command     (ASParser   *parser,
                                         const char *name,
                                         ASNodeList *list);
ASNode     *_as_parser_node_if_else     (ASParser   *parser,
                                         ASNode     *condition,
                                         ASNode     *then_,
                                         ASNode     *else_);
ASNode     *_as_parser_node_repeat      (ASParser   *parser,
                                         ASNode     *times,
                                         ASNode     *what);
ASNode     *_as_parser_node_while       (ASParser   *parser,
                                         ASNode     *times,
                                         ASNode     *what);
ASNode     *_as_parser_node_assignment  (ASParser   *parser,
                                         ASNodeVar  *var,
                                         ASNode     *val);
ASNode     *_as_parser_node_binary_op   (ASParser   *parser,
                                         ASBinaryOp  op,
                                         ASNode     *lval,
                                         ASNode     *rval);
ASNode     *_as_parser_node_unary_op    (ASParser   *parser,
                                         ASUnaryOp   op,
                                         ASNode     *val);
ASNode     *_as_parser_node_int         (ASParser   *parser,
                                         int         n);
ASNode     *_as_parser_node_string      (ASParser   *parser,
                                         const char *string);
ASNode     *_as_parser_node_value_list  (ASParser   *parser,
                                         ASNodeList *list);
ASNode     *_as_parser_node_var_pos     (ASParser   *parser,
                                         int         n);
ASNode     *_as_parser_node_var_named   (ASParser   *parser,
                                         const char *name);


G_END_DECLS

#endif /* __AS_SCRIPT_PARSER_H__ */
