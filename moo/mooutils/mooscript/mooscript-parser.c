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

#include "mooscript-parser-priv.h"
#include "mooscript-parser.h"
#include "mooscript-yacc.h"
#include <string.h>


typedef struct {
    const char *string;
    guint len;
    int token;
} Keyword;

static Keyword keywords[] = {
    { "if",       2, IF },
    { "then",     4, THEN },
    { "else",     4, ELSE },
    { "fi",       2, FI },
    { "while",    5, WHILE },
    { "for",      3, FOR },
    { "in",       2, IN },
    { "do",       2, DO },
    { "od",       2, OD },
    { "not",      3, NOT },
    { "return",   6, RETURN },
    { "continue", 8, CONTINUE },
    { "break",    5, BREAK }
};


typedef struct {
    const guchar *input;
    guint len;
    guint ptr;
    GHashTable *hash;
} MSLex;

struct _MSParser {
    MSLex *lex;
    gboolean failed;

    GSList *nodes;
    MSNode *script;
};


#define IS_QUOTE(c__)   (c__ == '"' || c__ == '\'')
#define IS_EOL(c__)     (c__ == '\r' || c__ == '\n')
#define IS_SPACE(c__)   (c__ == ' ' || c__ == '\t' || IS_EOL (c__))
#define IS_DIGIT(c__)   ('0' <= c__ && c__ <= '9')
#define IS_LETTER(c__)  (('a' <= c__ && c__ <= 'z') || ('A' <= c__ && c__ <= 'Z'))
#define IS_WORD(c__)    (IS_LETTER (c__) || IS_DIGIT (c__) || c__ == '_')


static int
ms_lex_error (MSParser *parser)
{
    parser->failed = TRUE;
    return -1;
}


static char *
ms_lex_add_string (MSLex      *lex,
                   const char *string,
                   int         len)
{
    gpointer orig, dummy;
    char *copy;

    if (len < 0)
        len = strlen (string);

    copy = g_strndup (string, len);

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
ms_lex_parse_string (MSLex    *lex,
                     MSParser *parser)
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
            _ms_script_yylval.str = ms_lex_add_string (lex, string->str, string->len);
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
    token = ms_lex_error (parser);

out:
    g_string_free (string, TRUE);
    return token;
}


static int
ms_lex_parse_number (MSLex    *lex,
                     MSParser *parser)
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
                g_print ("syntax error, number is too big\n");
                return ms_lex_error (parser);
            }

            value = (value * 10) + (c - '0');
            lex->ptr++;
        }
        else if (IS_WORD (c))
        {
            g_print ("syntax error, number followed by word char\n");
            return ms_lex_error (parser);
        }
        else
        {
            _ms_script_yylval.ival = value;
            return NUMBER;
        }
    }

    _ms_script_yylval.ival = value;
    return NUMBER;
}


static int
ms_lex_parse_word (MSLex    *lex,
                   G_GNUC_UNUSED MSParser *parser)
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

    _ms_script_yylval.str = ms_lex_add_string (lex, string, last - lex->ptr);
    lex->ptr = last;

    return IDENTIFIER;
}


static int
ms_lex_parse_python (MSLex *lex)
{
    guint ptr = lex->ptr;
    const char *input = (const char *) lex->input;

    g_assert (input[ptr] == input[ptr+1] &&
              input[ptr+1] == input[ptr+2] &&
              input[ptr] == '=');

    while (TRUE)
    {
        while (ptr < lex->len && !IS_EOL(input[ptr]))
            ptr++;

        if (ptr == lex->len)
        {
            _ms_script_yylval.str = ms_lex_add_string (lex, input + lex->ptr + 3, -1);
            lex->ptr = lex->len;
            return PYTHON;
        }

        while (IS_EOL (input[ptr]))
            ptr++;

        if (input[ptr] == '=' && input[ptr+1] == '=' && input[ptr+2] == '=')
        {
            _ms_script_yylval.str = ms_lex_add_string (lex, input + lex->ptr + 3,
                                                       ptr - lex->ptr - 3);
            lex->ptr = ptr + 3;
            return PYTHON;
        }
    }
}


#define THIS            (lex->input[lex->ptr])
#define NEXT            (lex->input[lex->ptr+1])
#define NEXT2           (lex->input[lex->ptr+2])

#define RETURN1(what)               \
G_STMT_START {                      \
    lex->ptr += 1;                  \
    return what;                    \
} G_STMT_END

#define CHECK1(c_, what_)           \
G_STMT_START {                      \
    if (THIS == c_)                 \
        RETURN1 (what_);            \
} G_STMT_END

#define RETURN2(what)               \
G_STMT_START {                      \
    lex->ptr += 2;                  \
    return what;                    \
} G_STMT_END

#define CHECK2(c1_, c2_, what_)     \
G_STMT_START {                      \
    if (THIS == c1_ && NEXT == c2_) \
        RETURN2 (what_);            \
} G_STMT_END


int
_ms_script_yylex (MSParser *parser)
{
    MSLex *lex = parser->lex;
    guchar c;

    while (lex->ptr < lex->len && IS_SPACE(lex->input[lex->ptr]))
        lex->ptr++;

    if (lex->ptr == lex->len)
        return 0;

    c = lex->input[lex->ptr];

    if (c & 0x80)
    {
        g_warning ("got unicode character");
        return ms_lex_error (parser);
    }

    if (c == '=' && NEXT == '=' && NEXT2 == '=')
        return ms_lex_parse_python (lex);

    if (c == '#')
    {
        while (lex->ptr < lex->len && !IS_EOL(lex->input[lex->ptr]))
            lex->ptr++;

        if (lex->ptr == lex->len)
            return 0;

        lex->ptr++;
        return _ms_script_yylex (parser);
    }

    if (IS_QUOTE (c))
        return ms_lex_parse_string (lex, parser);

    if (IS_DIGIT (c))
        return ms_lex_parse_number (lex, parser);

    if (IS_LETTER (c) || c == '_')
        return ms_lex_parse_word (lex, parser);

    CHECK2 ('.', '.', TWODOTS);
    CHECK2 ('=', '=', EQ);
    CHECK2 ('!', '=', NEQ);
    CHECK2 ('&', '&', AND);
    CHECK2 ('|', '|', OR);
    CHECK2 ('<', '=', LE);
    CHECK2 ('>', '=', GE);

    CHECK1 ('!', NOT);

    lex->ptr++;
    return c;
}


void
_ms_script_yyerror (MSParser   *parser,
                    const char *string)
{
    g_print ("%s\n", string);
    parser->failed = TRUE;
}


static MSLex *
ms_lex_new (const char *string,
            int         len)
{
    MSLex *lex = g_new0 (MSLex, 1);

    if (len < 0)
        len = strlen (string);

    lex->input = (const guchar *) string;
    lex->len = len;
    lex->hash = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

    return lex;
}


static void
ms_lex_free (MSLex *lex)
{
    if (lex)
    {
        g_hash_table_destroy (lex->hash);
        g_free (lex);
    }
}


static MSParser *
ms_parser_new (void)
{
    MSParser *parser = g_new0 (MSParser, 1);
    return parser;
}


static void
ms_parser_cleanup (MSParser *parser)
{
    ms_lex_free (parser->lex);
    parser->lex = NULL;
    g_slist_foreach (parser->nodes, (GFunc) ms_node_unref, NULL);
    g_slist_free (parser->nodes);
    parser->nodes = NULL;
    parser->script = NULL;
}


static void
ms_parser_free (MSParser *parser)
{
    if (parser)
    {
        ms_parser_cleanup (parser);
        g_free (parser);
    }
}


static MSNode *
ms_parser_parse (MSParser   *parser,
                 const char *string,
                 int         len)
{
    ms_parser_cleanup (parser);
    parser->lex = ms_lex_new (string, len);

    _ms_script_yyparse (parser);

    return parser->failed ? NULL : parser->script;
}


MSNode *
ms_script_parse (const char *string)
{
    MSParser *parser;
    MSNode *script;

    g_return_val_if_fail (string != NULL, FALSE);

    if (!string[0])
        return NULL;

    ms_type_init ();

    parser = ms_parser_new ();
    script = ms_parser_parse (parser, string, -1);

    if (script)
        ms_node_ref (script);

    ms_parser_free (parser);
    return script;
}


#define HEAD        (stack ? GPOINTER_TO_INT (stack->data) : 0)
#define PUSH(tok)   stack = g_slist_prepend (stack, GINT_TO_POINTER (tok))
#define POP()       stack = g_slist_delete_link (stack, stack)

MSScriptCheckResult
ms_script_check (const char *string)
{
    MSParser *parser;
    MSScriptCheckResult ret;
    GSList *stack;
    gboolean semicolon;

    g_return_val_if_fail (string != NULL, MS_SCRIPT_ERROR);

    if (!string[0])
        return MS_SCRIPT_INCOMPLETE;

    ms_type_init ();
    parser = ms_parser_new ();
    parser->lex = ms_lex_new (string, -1);

    ret = MS_SCRIPT_ERROR;
    stack = NULL;

    while (TRUE)
    {
        int token = _ms_script_yylex (parser);

        switch (token)
        {
            case -1:
                goto out;

            case 0:
                if (!HEAD && semicolon)
                    ret = MS_SCRIPT_COMPLETE;
                else
                    ret = MS_SCRIPT_INCOMPLETE;
                goto out;

            case IF:
                PUSH (IF);
                semicolon = FALSE;
                break;

            case FI:
                if (HEAD != IF)
                    goto out;
                POP ();
                semicolon = FALSE;
                break;

            case WHILE:
                if (HEAD == DO)
                    POP ();
                else
                    PUSH (WHILE);
                semicolon = FALSE;
                break;

            case DO:
                switch (HEAD)
                {
                    case 0:
                        PUSH (DO);
                        break;
                    case WHILE:
                    case FOR:
                        break;
                    default:
                        goto out;
                }
                semicolon = FALSE;
                break;

            case OD:
                switch (HEAD)
                {
                    case WHILE:
                    case FOR:
                        POP ();
                        break;
                    default:
                        goto out;
                }
                semicolon = FALSE;
                break;

            case ';':
                semicolon = TRUE;
                break;

            default:
                semicolon = FALSE;
        }
    }

out:
    g_slist_free (stack);
    ms_parser_free (parser);
    return ret;
}


void
_ms_parser_add_node (MSParser   *parser,
                     gpointer    node)
{
    g_return_if_fail (node != NULL);
    parser->nodes = g_slist_prepend (parser->nodes, node);
}


void
_ms_parser_set_top_node (MSParser   *parser,
                         MSNode     *node)
{
    g_assert (parser != NULL);
    g_assert (parser->script == NULL);

    if (!node)
    {
        node = (MSNode*) ms_node_list_new ();
        _ms_parser_add_node (parser, node);
    }

    parser->script = node;
}
