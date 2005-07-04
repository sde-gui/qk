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
    CMD_ERROR,
    CMD_IGNORE,

    CMD_DECALN,
    CMD_DECSET,     /* see skip */
    CMD_DECRST,     /* see skip */
    CMD_G0_CHARSET, /* see skip */
    CMD_G1_CHARSET, /* see skip */
    CMD_G2_CHARSET, /* see skip */
    CMD_G3_CHARSET, /* see skip */
    CMD_DECSC,
    CMD_DECRC,
    CMD_IND,
    CMD_NEL,
    CMD_HTS,
    CMD_RI,
    CMD_SS2,
    CMD_SS3,
    CMD_DA,
    CMD_ICH,
    CMD_CUU,
    CMD_CUD,
    CMD_CUF,
    CMD_CUB,
    CMD_CUP,
    CMD_ED,
    CMD_EL,
    CMD_IL,
    CMD_DL,
    CMD_DCH,
    CMD_INIT_HILITE_MOUSE_TRACKING,
    CMD_TBC,
    CMD_SM,
    CMD_RM,
    CMD_SGR,
    CMD_DSR,
    CMD_DECSTBM,
    CMD_DECREQTPARM,
    CMD_RESTORE_DECSET,
    CMD_SAVE_DECSET,
    CMD_SET_TEXT,
    CMD_RIS,
    CMD_LS2,
    CMD_LS3,
    CMD_DECKPAM,
    CMD_DECKPNM,

    CMD_BELL,
    CMD_BACKSPACE,
    CMD_TAB,
    CMD_LINEFEED,
    CMD_VERT_TAB,
    CMD_FORM_FEED,
    CMD_CARRIAGE_RETURN,
    CMD_ALT_CHARSET,
    CMD_NORM_CHARSET,

    CMD_COLUMN_ADDRESS,
    CMD_ROW_ADDRESS,
    CMD_BACK_TAB,
    CMD_RESET_2STRING
} CmdCode;

#define CMD_HVP     CMD_CUP

#define CMD_MC              CMD_IGNORE
#define CMD_DECDHL          CMD_IGNORE
#define CMD_DECDWL          CMD_IGNORE
#define CMD_DECSWL          CMD_IGNORE
#define CMD_DECTST          CMD_IGNORE
#define CMD_DECLL           CMD_IGNORE
#define CMD_DCS             CMD_IGNORE
#define CMD_PM              CMD_IGNORE
#define CMD_APC             CMD_IGNORE
#define CMD_MEMORY_LOCK     CMD_IGNORE
#define CMD_MEMORY_UNLOCK   CMD_IGNORE
#define CMD_LS3R            CMD_IGNORE
#define CMD_LS2R            CMD_IGNORE
#define CMD_LS1R            CMD_IGNORE
#define CMD_TRACK_MOUSE     CMD_IGNORE
#define CMD_PRINT_SCREEN    CMD_IGNORE
#define CMD_PRTR_OFF        CMD_IGNORE
#define CMD_PRTR_ON         CMD_IGNORE
#define CMD_USER6           CMD_IGNORE
#define CMD_USER8           CMD_IGNORE


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
    char        string[MAX_ESC_SEQ_LEN];
    guint       string_len;
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

char           *_moo_term_nice_bytes    (const char     *string,
                                         int             len);


inline static gboolean  iter_is_start   (Iter           *iter)
{
    if (iter->parser->old_data_len)
    {
        return iter->old && !iter->offset;
    }
    else
    {
        g_assert (!iter->old);
        return !iter->offset;
    }
}


inline static void      iter_backward   (Iter           *iter)
{
    g_assert (!iter_is_start (iter));

    if (iter->offset)
    {
        --iter->offset;
    }
    else
    {
        g_assert (!iter->old && iter->parser->old_data_len);
        iter->offset = iter->parser->old_data_len - 1;
    }
}


G_END_DECLS

#endif /* MOOTERM_MOOTERMPARSER_H */
