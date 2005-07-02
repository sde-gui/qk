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

#include "mooterm/mootermbuffer.h"

G_BEGIN_DECLS


typedef enum {
    CMD_NONE = 0,

    CMD_SET_ATTRS,

    CMD_CURSOR_NORMAL,
    CMD_CURSOR_INVISIBLE,
    CMD_BACK_TAB,
    CMD_CLEAR_SCREEN,
    CMD_CHANGE_SCROLL_REGION,

    CMD_SET_WINDOW_TITLE,
    CMD_SET_ICON_NAME,
    CMD_SET_WINDOW_ICON_NAME,

    CMD_PARM_LEFT_CURSOR,
    CMD_PARM_RIGHT_CURSOR,
    CMD_PARM_DOWN_CURSOR,
    CMD_PARM_UP_CURSOR,
    CMD_CURSOR_RIGHT,
    CMD_CURSOR_LEFT,
    CMD_CURSOR_DOWN,
    CMD_CURSOR_UP,
    CMD_CURSOR_HOME,
    CMD_CURSOR_ADDRESS,
    CMD_ROW_ADDRESS,
    CMD_COLUMN_ADDRESS,
    CMD_CARRIAGE_RETURN,

    CMD_PARM_DCH,
    CMD_PARM_DELETE_LINE,
    CMD_DELETE_CHARACTER,
    CMD_DELETE_LINE,
    CMD_ERASE_CHARS,
    
    CMD_CLR_EOS,
    CMD_CLR_EOL,
    CMD_CLR_BOL,
    
    CMD_ENA_ACS,
    CMD_FLASH_SCREEN,
    
    CMD_PARM_ICH,
    CMD_PARM_INSERT_LINE,
    CMD_INSERT_LINE,
    
    CMD_ENTER_SECURE_MODE,
    CMD_INIT_2STRING,
    CMD_RESET_2STRING,
    CMD_RESET_1STRING,
    CMD_PRINT_SCREEN,
    CMD_PRTR_OFF,
    CMD_PRTR_ON,
    
    CMD_ORIG_PAIR,
    CMD_ENTER_REVERSE_MODE,
    CMD_ENTER_AM_MODE,
    CMD_EXIT_AM_MODE,
    CMD_ENTER_CA_MODE,
    CMD_EXIT_CA_MODE,
    CMD_ENTER_INSERT_MODE,
    CMD_EXIT_INSERT_MODE,
    CMD_ENTER_STANDOUT_MODE,
    CMD_EXIT_STANDOUT_MODE,
    CMD_ENTER_UNDERLINE_MODE,
    CMD_EXIT_UNDERLINE_MODE,
    
    CMD_KEYPAD_LOCAL,
    CMD_KEYPAD_XMIT,
    
    CMD_SET_BACKGROUND,
    CMD_SET_FOREGROUND,
    
    CMD_CLEAR_ALL_TABS,
    
    CMD_USER6,
    CMD_USER7,
    CMD_USER8,
    CMD_USER9,
    
    CMD_BELL,
    CMD_ENTER_ALT_CHARSET_MODE,
    CMD_EXIT_ALT_CHARSET_MODE,
    
    CMD_TAB,
    CMD_SCROLL_FORWARD,
    CMD_SCROLL_REVERSE,
    CMD_SET_TAB,
    CMD_ESC_l,
    CMD_ESC_m,

    CMD_RESTORE_CURSOR,
    CMD_SAVE_CURSOR
} CmdCode;
    

#define MAX_NUMS_LEN        128
#define MAX_ESC_SEQ_LEN     1024

typedef struct _MooTermParser MooTermParser;

typedef struct {
    MooTermParser *parser;
    gboolean    old;
    guint       offset;
} Iter;

typedef struct {
    Iter        start;
    Iter        end;
} Block;

struct _MooTermParser {
    MooTermBuffer *term_buffer;

    Iter        tosave;
    char        old_data[MAX_ESC_SEQ_LEN];
    guint       old_data_len;
    const char *data;
    guint       data_len;

    Iter        current;

    Block       chars;

    gboolean    eof_error;

    CmdCode     cmd;
    Block       cmd_string;

    guint       nums[MAX_NUMS_LEN];
    guint       nums_len;
};


MooTermParser  *moo_term_parser_new     (MooTermBuffer  *buf);
void            moo_term_parser_free    (MooTermParser  *parser);

void            moo_term_parser_parse   (MooTermParser  *parser,
                                         const char     *string,
                                         gssize          len);

int             _moo_term_yylex         (MooTermParser  *parser);
void            _moo_term_yyerror       (MooTermParser  *parser,
                                         const char     *string);

/* defined in generated mootermparser-yacc.c */
int             _moo_term_yyparse       (MooTermParser  *parser);

void            _moo_term_print_bytes   (const char     *string,
                                         int             len);


G_END_DECLS

#endif /* MOOTERM_MOOTERMPARSER_H */
