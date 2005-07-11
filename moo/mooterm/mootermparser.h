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

#ifndef MOOTERM_COMPILATION
#error "Don't include this file"
#endif

#include "mooterm/mooterm-private.h"

G_BEGIN_DECLS


#define MAX_PARAMS_LEN      4095
#define MAX_PARAMS_NUM      16
#define ERROR_CHAR          '?'

typedef struct {
    GString         *old_data;
    const guchar    *data;
    guint            data_len;
} Input;

typedef struct {
    gboolean    old;
    guint       offset;
} InputIter;


typedef enum {
    PART_START,
    PART_INTERMEDIATE,
    PART_PARAMETERS,
    PART_FINAL,
    PART_DONE
} SequencePartType;

typedef enum {
    LEX_ESCAPE,
    LEX_CONTROL
} LexType;

typedef struct {
    LexType             lex;
    SequencePartType    part;
    guint               offset;
} Lexer;


typedef enum {
    INITIAL = 0,
    ESCAPE,
    ESCAPE_INTERMEDIATE,
    DCS,
    CSI,
    OSC,
    PM,
    APC,
    ERROR
} ParserState;


typedef struct _MooTermParser {
    MooTerm    *term;

    Input       input;
    gboolean    save;

    InputIter   current;
    InputIter   cmd_start;
    gboolean    escape_two_bytes;

    ParserState state;

    Lexer       lex;

    GString    *character;

    GString    *intermediate;
    GString    *parameters;
    GString    *data;
    guchar      final;

    GArray     *numbers;
} MooTermParser;


MooTermParser  *moo_term_parser_new     (MooTerm        *term);
void            moo_term_parser_free    (MooTermParser  *parser);

void            moo_term_parser_parse   (MooTermParser  *parser,
                                         const char     *string,
                                         guint           len);
void            moo_term_parser_reset   (MooTermParser  *parser);

int             _moo_term_yylex         (MooTermParser  *parser);
void            _moo_term_yyerror       (MooTermParser  *parser,
                                         const char     *string);

/* defined in generated mootermparser-yacc.c */
int             _moo_term_yyparse       (MooTermParser  *parser);

char           *_moo_term_nice_bytes    (const char     *string,
                                         int             len);


G_END_DECLS

#endif /* MOOTERM_MOOTERMPARSER_H */
