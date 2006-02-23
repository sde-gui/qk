/*
 *   mootermparser.h
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

#ifndef MOOTERM_COMPILATION
#error "This file may not be included"
#endif

#ifndef __MOO_TERM_PARSER_H__
#define __MOO_TERM_PARSER_H__

#include "mooterm/mooterm-private.h"

G_BEGIN_DECLS


#define MAX_PARAMS_LEN      4095
#define MAX_PARAMS_NUM      16
#define ERROR_CHAR          '?'

struct _MooTermParser;

typedef struct {
    GString         *old_data;
    const guchar    *data;
    guint            data_len;
} Input;

typedef struct {
    struct _MooTermParser *parser;
    gboolean    old;
    guint       offset;
} InputIter;


typedef enum {
    PART_START,
    PART_INTERMEDIATE,
    PART_PARAMETERS,
    PART_FINAL,
    PART_DATA,
    PART_ST,
    PART_DONE
} SequencePartType;

typedef enum {
    LEX_ESCAPE,
    LEX_CONTROL,
    LEX_DCS
} LexType;

typedef struct {
    LexType             lex;
    SequencePartType    part;
    guint               offset;
} Lexer;


typedef struct _MooTermParser {
    MooTerm    *term;

    Input       input;
    gboolean    save;

    InputIter  *current;
    InputIter  *cmd_start;

    Lexer       lex;

    GString    *character;

    GString    *intermediate;
    GString    *parameters;
    GString    *data;
    guchar      final;

    GArray     *numbers;
} MooTermParser;


MooTermParser  *_moo_term_parser_new    (MooTerm        *term);
void            _moo_term_parser_free   (MooTermParser  *parser);

void            _moo_term_parser_parse  (MooTermParser  *parser,
                                         const char     *string,
                                         guint           len);
void            _moo_term_parser_reset  (MooTermParser  *parser);

int             _moo_term_yylex         (MooTermParser  *parser);
void            _moo_term_yyerror       (MooTermParser  *parser,
                                         const char     *string);

char           *_moo_term_current_ctl   (MooTermParser  *parser);
char           *_moo_term_nice_char     (guchar          c);
char           *_moo_term_nice_bytes    (const char     *string,
                                         int             len);

/* defined in generated mootermparser-yacc.c */
int             _moo_term_yyparse       (MooTermParser  *parser);


G_END_DECLS

#endif /* __MOO_TERM_PARSER_H__ */
