/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   as-script-parser.c
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

#include "as-script-parser.h"
#include "as-script-yacc.h"
#include <string.h>


typedef struct {
    const char *string;
    guint len;
    int token;
} Keyword;

static Keyword keywords[] = {
    { "if",     2, IF },
    { "then",   4, THEN },
    { "else",   4, ELSE },
    { "while",  5, WHILE },
    { "repeat", 6, REPEAT },
    { "not",    3, NOT },
};


typedef struct {
    const guchar *input;
    guint len;
    guint ptr;
    GHashTable *hash;
} ASLex;

struct _ASParser {
    ASLex *lex;
    gboolean failed;

    GSList *nodes;
    ASNode *script;
};


#define IS_QUOTE(c__)   (c__ == '"' || c__ == '\'')
#define IS_SPACE(c__)   (c__ == ' ' || c__ == '\t' || c__ == '\r' || c__ == '\n')
#define IS_DIGIT(c__)   ('0' <= c__ && c__ <= '9')
#define IS_LETTER(c__)  (('a' <= c__ && c__ <= 'z') || ('A' <= c__ && c__ <= 'Z'))
#define IS_WORD(c__)    (IS_LETTER (c__) || IS_DIGIT (c__) || c__ == '_')


static int
as_lex_error (ASParser *parser)
{
    parser->failed = TRUE;
    return -1;
}


static char *
as_lex_add_string (ASLex      *lex,
                   const char *string,
                   guint       len)
{
    gpointer orig, dummy;
    char *copy = g_strndup (string, len);

    if (g_hash_table_lookup_extended (lex->hash, copy, &orig, &dummy))
    {
        g_free (copy);
        return orig;
    }
    else
    {
        g_hash_table_insert (lex->hash, copy, NULL);
        return copy;
    }
}


static int
as_lex_parse_string (ASLex    *lex,
                     ASParser *parser)
{
    guint first, last;
    guchar quote, second;
    GString *string;
    int token = -1;

    g_assert (IS_QUOTE (lex->input[lex->ptr]));

    last = first = lex->ptr + 1;
    quote = lex->input[lex->ptr];
    string = g_string_new (NULL);

    if (quote == '\'')
        second = '"';
    else
        second = '\'';

    while (last < lex->len)
    {
        guchar c = lex->input[last];

        if (c == quote)
        {
            _as_script_yylval.str = as_lex_add_string (lex, string->str, string->len);
            lex->ptr = last + 1;
            token = LITERAL;
            goto out;
        }
        else if (c == '\\')
        {
            guchar next;

            if (last + 1 == lex->len)
                break;

            next = lex->input[last + 1];

            switch (next)
            {
                case 't':
                    g_string_append_c (string, '\t');
                    last += 2;
                    break;
                case 'n':
                    g_string_append_c (string, '\n');
                    last += 2;
                    break;
                case '\'':
                case '"':
                case '\\':
                    g_string_append_c (string, next);
                    last += 2;
                    break;

                default:
                    g_string_append_c (string, '\\');
                    last++;
                    break;
            }
        }
        else
        {
            g_string_append_c (string, c);
            last++;
        }
    }

    g_warning ("unterminated string literal");
    token = as_lex_error (parser);

out:
    g_string_free (string, TRUE);
    return token;
}


static int
as_lex_parse_number (ASLex    *lex,
                     ASParser *parser)
{
    int value = 0;

    g_assert (IS_DIGIT (lex->input[lex->ptr]));

    while (lex->ptr < lex->len)
    {
        guchar c = lex->input[lex->ptr];

        if (IS_DIGIT (c))
        {
            if (value > 1000000)
            {
                g_warning ("number is too big");
                return as_lex_error (parser);
            }

            value = (value * 10) + (c - '0');
            lex->ptr++;
        }
        else if (IS_WORD (c))
        {
            g_warning ("number followed by word char");
            return as_lex_error (parser);
        }
        else
        {
            _as_script_yylval.ival = value;
            return NUMBER;
        }
    }

    g_critical ("oops");
    return as_lex_error (parser);
}


static int
as_lex_parse_word (ASLex    *lex,
                   G_GNUC_UNUSED ASParser *parser)
{
    guint last, i;
    const char *string;

    g_assert (IS_WORD (lex->input[lex->ptr]) && !IS_DIGIT (lex->input[lex->ptr]));

    string = (const char *) &lex->input[lex->ptr];

    for (i = 0; i < G_N_ELEMENTS (keywords); ++i)
    {
        Keyword *kw = &keywords[i];

        if (lex->ptr + kw->len <= lex->len &&
            !strncmp (string, kw->string, kw->len) &&
            (lex->ptr + kw->len == lex->len || !IS_WORD (string[kw->len])))
        {
            lex->ptr += keywords[i].len;
            return keywords[i].token;
        }
    }

    for (last = lex->ptr + 1; last < lex->len && IS_WORD (lex->input[last]); ++last) ;

    _as_script_yylval.str = as_lex_add_string (lex, string, last - lex->ptr);
    lex->ptr = last;

    return IDENTIFIER;
}


int
_as_script_yylex (ASParser *parser)
{
    ASLex *lex = parser->lex;
    guchar c;

    while (lex->ptr < lex->len && IS_SPACE(lex->input[lex->ptr]))
        lex->ptr++;

    if (lex->ptr == lex->len)
        return 0;

    c = lex->input[lex->ptr];

    if (c & 0x80)
    {
        g_warning ("got unicode character");
        return as_lex_error (parser);
    }

    if (IS_QUOTE (c))
        return as_lex_parse_string (lex, parser);

    if (IS_DIGIT (c))
        return as_lex_parse_number (lex, parser);

    if (IS_LETTER (c) || c == '_')
        return as_lex_parse_word (lex, parser);

    lex->ptr++;
    return c;
}


void
_as_script_yyerror (ASParser   *parser,
                    const char *string)
{
    g_print ("error: %s\n", string);
    parser->failed = TRUE;
}


static ASLex *
as_lex_new (const char *string,
            int         len)
{
    ASLex *lex = g_new0 (ASLex, 1);

    if (len < 0)
        len = strlen (string);

    lex->input = (const guchar *) string;
    lex->len = len;
    lex->hash = g_hash_table_new (g_str_hash, g_str_equal);

    return lex;
}


static void
as_lex_free (ASLex *lex)
{
    if (lex)
    {
        g_hash_table_destroy (lex->hash);
        g_free (lex);
    }
}


static ASParser *
as_parser_new (void)
{
    ASParser *parser = g_new0 (ASParser, 1);
    return parser;
}


static void
as_parser_cleanup (ASParser *parser)
{
    as_lex_free (parser->lex);
    parser->lex = NULL;
    g_slist_foreach (parser->nodes, (GFunc) g_object_unref, NULL);
    g_slist_free (parser->nodes);
    parser->nodes = NULL;
    parser->script = NULL;
}


static void
as_parser_free (ASParser *parser)
{
    if (parser)
    {
        as_parser_cleanup (parser);
        g_free (parser);
    }
}


static ASNode *
as_parser_parse (ASParser   *parser,
                 const char *string,
                 int         len)
{
    as_parser_cleanup (parser);
    parser->lex = as_lex_new (string, len);

    _as_script_yyparse (parser);

    return parser->failed ? NULL : parser->script;
}


ASNode *
as_script_parse (const char *string)
{
    ASParser *parser;
    ASNode *script;

    g_return_val_if_fail (string != NULL, FALSE);

    if (!string[0])
        return NULL;

    parser = as_parser_new ();
    script = as_parser_parse (parser, string, -1);

    if (script)
        g_object_ref (script);

    as_parser_free (parser);
    return script;
}


static void
parser_add_node (ASParser   *parser,
                 gpointer    node)
{
    g_return_if_fail (AS_IS_NODE (node));
    parser->nodes = g_slist_prepend (parser->nodes, node);
}


void
_as_parser_set_top_node (ASParser   *parser,
                         ASNode     *node)
{
    g_assert (parser != NULL);
    g_assert (parser->script == NULL);

    if (!node)
    {
        node = (ASNode*) as_node_list_new ();
        parser_add_node (parser, node);
    }

    parser->script = node;
}


ASNode *
_as_parser_node_list_add (ASParser   *parser,
                          ASNodeList *list,
                          ASNode     *node)
{
    if (!node)
        return NULL;

    if (!list)
    {
        list = as_node_list_new ();
        parser_add_node (parser, list);
    }

    as_node_list_add (list, node);
    return AS_NODE (list);
}


ASNode *
_as_parser_node_command (ASParser   *parser,
                         const char *name,
                         ASNodeList *list)
{
    ASNodeCommand *cmd;

    g_return_val_if_fail (name != NULL, NULL);
    g_return_val_if_fail (!list || AS_IS_NODE_LIST (list), NULL);

    cmd = as_node_command_new (name, list);
    parser_add_node (parser, cmd);

    return AS_NODE (cmd);
}


ASNode *
_as_parser_node_if_else (ASParser   *parser,
                         ASNode     *condition,
                         ASNode     *then_,
                         ASNode     *else_)
{
    ASNodeIfElse *node;

    g_return_val_if_fail (AS_IS_NODE (condition), NULL);
    g_return_val_if_fail (AS_IS_NODE (then_), NULL);
    g_return_val_if_fail (!else_ || AS_IS_NODE (else_), NULL);

    node = as_node_if_else_new (condition, then_, else_);
    parser_add_node (parser, node);

    return AS_NODE (node);
}


static ASNode *
as_parser_loop (ASParser   *parser,
                ASLoopType  type,
                ASNode     *times,
                ASNode     *what)
{
    ASNodeLoop *loop;

    g_return_val_if_fail (AS_IS_NODE (times), NULL);
    g_return_val_if_fail (AS_IS_NODE (what), NULL);

    loop = as_node_loop_new (type, times, what);
    parser_add_node (parser, loop);

    return AS_NODE (loop);
}

ASNode *
_as_parser_node_repeat (ASParser   *parser,
                        ASNode     *times,
                        ASNode     *what)
{
    return as_parser_loop (parser, AS_LOOP_TIMES, times, what);
}


ASNode *
_as_parser_node_while (ASParser   *parser,
                       ASNode     *cond,
                       ASNode     *what)
{
    return as_parser_loop (parser, AS_LOOP_WHILE, cond, what);
}


ASNode *
_as_parser_node_assignment (ASParser   *parser,
                            ASNodeVar  *var,
                            ASNode     *val)
{
    ASNodeAssign *node;

    g_return_val_if_fail (AS_IS_NODE_VAR (var), NULL);
    g_return_val_if_fail (AS_IS_NODE (val), NULL);

    node = as_node_assign_new (var, val);
    parser_add_node (parser, node);

    return AS_NODE (node);
}


ASNode *
_as_parser_node_binary_op (ASParser   *parser,
                           ASBinaryOp  op,
                           ASNode     *lval,
                           ASNode     *rval)
{
    ASNodeCommand *cmd;

    g_return_val_if_fail (AS_IS_NODE (lval), NULL);
    g_return_val_if_fail (AS_IS_NODE (rval), NULL);

    cmd = as_node_binary_op_new (op, lval, rval);
    parser_add_node (parser, cmd);

    return AS_NODE (cmd);
}


ASNode *
_as_parser_node_unary_op (ASParser   *parser,
                          ASUnaryOp   op,
                          ASNode     *val)
{
    ASNodeCommand *cmd;

    g_return_val_if_fail (AS_IS_NODE (val), NULL);

    cmd = as_node_unary_op_new (op, val);
    parser_add_node (parser, cmd);

    return AS_NODE (cmd);
}


ASNode *
_as_parser_node_int (ASParser   *parser,
                     int         n)
{
    ASNodeValue *node;
    ASValue *value;

    value = as_value_int (n);
    node = as_node_value_new (value);
    as_value_unref (value);
    parser_add_node (parser, node);

    return AS_NODE (node);
}


ASNode *
_as_parser_node_string (ASParser   *parser,
                        const char *string)
{
    ASNodeValue *node;
    ASValue *value;

    value = as_value_string (string);
    node = as_node_value_new (value);
    as_value_unref (value);
    parser_add_node (parser, node);

    return AS_NODE (node);
}


ASNode *
_as_parser_node_var_pos (ASParser   *parser,
                         int         n)
{
    ASNodeVar *node;

    node = as_node_var_new_positional (n);
    parser_add_node (parser, node);

    return AS_NODE (node);
}


ASNode *
_as_parser_node_var_named (ASParser   *parser,
                           const char *name)
{
    ASNodeVar *node;

    node = as_node_var_new_named (name);
    parser_add_node (parser, node);

    return AS_NODE (node);
}


ASNode *
_as_parser_node_value_list (ASParser   *parser,
                            ASNodeList *list)
{
    ASNodeValList *node;

    node = as_node_val_list_new (list);
    parser_add_node (parser, node);

    return AS_NODE (node);
}
