/*
 *   mooterm/mootermparser.c
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

#define MOOTERM_COMPILATION
#include "mooterm/mootermparser-yacc.h"
#include "mooterm/mootermparser.h"
#include "mooterm/mooterm-vt.h"
#include "mooterm/mooterm-vtctls.h"
#include <string.h>


#define INVALID_CHAR        "?"
#define INVALID_CHAR_LEN    1


#if 0
#define debug_one_char(c)                                   \
{                                                           \
    char *s = _moo_term_nice_char (c);                      \
    g_message ("got one-char '%s'", s);                     \
    g_free (s);                                             \
}
#define debug_control()                                     \
{                                                           \
    char *s = _moo_term_current_ctl (parser);               \
    g_message ("got sequence '%s'", s);                     \
    g_free (s);                                             \
}
#else
#define debug_one_char(c)
#define debug_control()
#endif


#define iter_eof(iter)                                      \
    (!(iter).old &&                                         \
        (iter).offset == parser->input.data_len)

#define iter_forward(iter)                                  \
{                                                           \
    g_assert (!iter_eof (iter));                            \
                                                            \
    ++(iter).offset;                                        \
                                                            \
    if ((iter).old)                                         \
    {                                                       \
        if ((iter).offset ==                                \
                parser->input.old_data->len)                \
        {                                                   \
            (iter).offset = 0;                              \
            (iter).old = FALSE;                             \
        }                                                   \
    }                                                       \
}

#define iter_backward(iter)                                 \
{                                                           \
    g_assert ((iter).offset ||                              \
            (!(iter).old &&                                 \
            parser->input.old_data->len));                  \
                                                            \
    if ((iter).offset)                                      \
    {                                                       \
        --(iter).offset;                                    \
    }                                                       \
    else                                                    \
    {                                                       \
        (iter).offset =                                     \
                parser->input.old_data->len - 1;            \
        (iter).old = TRUE;                                  \
    }                                                       \
}

#define iter_set_eof(iter)                                  \
{                                                           \
    (iter).old = FALSE;                                     \
    (iter).offset = parser->input.data_len;                 \
}

#define iter_get_char(iter, c)                              \
{                                                           \
    g_assert (!iter_eof (iter));                            \
                                                            \
    if ((iter).old)                                         \
        c = parser->input.old_data->str[(iter).offset];     \
    else                                                    \
        c = parser->input.data[(iter).offset];              \
}


#define save_cmd()                                          \
{                                                           \
    GString *old = parser->input.old_data;                  \
    guint offset = parser->cmd_start.offset;                \
                                                            \
    g_assert (!iter_eof (parser->cmd_start));               \
                                                            \
    if (parser->cmd_start.old)                              \
    {                                                       \
        g_assert (offset < old->len);                       \
        memmove (old->str, old->str + offset,               \
                 old->len - offset);                        \
        g_string_truncate (old, old->len - offset);         \
                                                            \
        g_string_append_len (old,                           \
                             parser->input.data,            \
                             parser->input.data_len);       \
    }                                                       \
    else                                                    \
    {                                                       \
        const char *data = parser->input.data;              \
        guint data_len = parser->input.data_len;            \
                                                            \
        g_assert (offset < parser->input.data_len);         \
                                                            \
        g_string_truncate (old, 0);                         \
        g_string_append_len (old,                           \
                             data + offset,                 \
                             data_len - offset);            \
    }                                                       \
                                                            \
    parser->save = TRUE;                                    \
}

#define save_char(c)                                        \
{                                                           \
    g_string_truncate (parser->input.old_data, 0);          \
    g_string_append_c (parser->input.old_data, c);          \
    parser->save = TRUE;                                    \
}

#define save_character()                                    \
{                                                           \
    g_string_truncate (parser->input.old_data, 0);          \
    g_string_append_len (parser->input.old_data,            \
                         parser->character->str,            \
                         parser->character->len);           \
    parser->save = TRUE;                                    \
}


#define goto_initial()                                      \
{                                                           \
    parser->state = INITIAL_;                               \
    iter_set_eof (parser->cmd_start);                       \
    goto STATE_INITIAL_;                                    \
}

#define goto_escape(st)                                     \
{                                                           \
    flush_chars ();                                         \
                                                            \
    parser->cmd_start = parser->current;                    \
    iter_backward (parser->cmd_start);                      \
                                                            \
    g_string_truncate (parser->intermediate, 0);            \
    g_string_truncate (parser->parameters, 0);              \
    g_string_truncate (parser->data, 0);                    \
    parser->final = 0;                                      \
                                                            \
    parser->state = ESCAPE_;                                \
                                                            \
    goto STATE_ESCAPE_;                                     \
}

#define one_char_cmd(c)                                     \
{                                                           \
    flush_chars ();                                         \
    debug_one_char(c);                                      \
    exec_cmd (parser, c);                                   \
}

#define check_cancel(c, state)                              \
{                                                           \
    switch (c)                                              \
    {                                                       \
        /* ESC - start new sequence */                      \
        case 0x1B:                                          \
            goto_escape ();                                 \
                                                            \
        /* CAN - cancel sequence in progress */             \
        case 0x18:                                          \
            goto_initial ();                                \
                                                            \
        /* SUB - cancel sequence in progress and            \
                 print error */                             \
        case 0x1A:                                          \
            vt_print_char (ERROR_CHAR);                     \
            goto_initial ();                                \
                                                            \
        case 0x05:                                          \
        case 0x07:                                          \
        case 0x08:                                          \
        case 0x09:                                          \
        case 0x0A:                                          \
        case 0x0B:                                          \
        case 0x0C:                                          \
        case 0x0D:                                          \
        case 0x0E:                                          \
        case 0x0F:                                          \
        case 0x11:                                          \
        case 0x13:                                          \
            one_char_cmd (c);                               \
            goto state;                                     \
    }                                                       \
}


#define dcs_check_cancel(c)                                 \
    switch (c)                                              \
    {                                                       \
        /* ESC - it might be beginning of ST sequence */    \
        case 0x1B:                                          \
            goto STATE_DCS_ESCAPE_;                         \
                                                            \
        /* CAN - cancel sequence in progress */             \
        case 0x18:                                          \
            goto_initial ();                                \
                                                            \
        /* SUB - cancel sequence in progress and            \
            print error */                                  \
        case 0x1A:                                          \
            vt_print_char (ERROR_CHAR);                     \
            goto_initial ();                                \
                                                            \
        case 0x05:                                          \
        case 0x07:                                          \
        case 0x08:                                          \
        case 0x09:                                          \
        case 0x0A:                                          \
        case 0x0B:                                          \
        case 0x0C:                                          \
        case 0x0D:                                          \
        case 0x0E:                                          \
        case 0x0F:                                          \
        case 0x11:                                          \
        case 0x13:                                          \
            one_char_cmd (c);                               \
            goto STATE_DCS_DATA_;                           \
    }


#define append_char(ar, c)                                  \
{                                                           \
    g_string_append_c (parser->ar, c);                      \
}


/* get_char ();
   Does not return on end of input;
   compresses ESC [ to CSI,
              ESC P to DCS,
              ESC ] to OSC,
              ESC ^ to PM,
              ESC \ to ST,
              ESC _ to APC */
#define get_char(c)                                         \
{                                                           \
    c = 0;                                                  \
                                                            \
    while (!iter_eof (parser->current))                     \
    {                                                       \
        iter_get_char (parser->current, c);                 \
                                                            \
        if (c)                                              \
        {                                                   \
            break;                                          \
        }                                                   \
        else                                                \
        {                                                   \
            iter_forward (parser->current);                 \
        }                                                   \
    }                                                       \
                                                            \
    if (!c)                                                 \
    {                                                       \
        if (!iter_eof (parser->cmd_start))                  \
        {                                                   \
            save_cmd ();                                    \
        }                                                   \
        else if (parser->character->len)                    \
        {                                                   \
            save_character ();                              \
        }                                                   \
                                                            \
        parser_finish (parser);                             \
        return;                                             \
    }                                                       \
                                                            \
    iter_forward (parser->current);                         \
}


#define flush_chars()                                       \
{                                                           \
    if (parser->character->len)                             \
    {                                                       \
        g_warning ("%s: invalid UTF8 '%s'", G_STRLOC,       \
                   parser->character->str);                 \
        vt_print_char (ERROR_CHAR);                         \
        g_string_truncate (parser->character, 0);           \
    }                                                       \
}


#define check_character(c)                                  \
{                                                           \
    gunichar uc;                                            \
                                                            \
    g_string_append_c (parser->character, c);               \
                                                            \
    uc = g_utf8_get_char_validated (parser->character->str, \
                                    -1);                    \
                                                            \
    switch (uc)                                             \
    {                                                       \
        case -2:                                            \
            break;                                          \
                                                            \
        case -1:                                            \
            g_warning ("%s: invalid UTF8 '%s'", G_STRLOC,   \
                       parser->character->str);             \
            vt_print_char (ERROR_CHAR);                     \
            g_string_truncate (parser->character, 0);       \
            break;                                          \
                                                            \
        default:                                            \
            vt_print_char (uc);                             \
            g_string_truncate (parser->character, 0);       \
            break;                                          \
    }                                                       \
}


#define process_char(c)                                     \
{                                                           \
    if (parser->character->len)                             \
    {                                                       \
        check_character (c);                                \
    }                                                       \
    else                                                    \
    {                                                       \
        if (c < 128)                                        \
        {                                                   \
            vt_print_char (c);                              \
        }                                                   \
        else                                                \
        {                                                   \
            check_character (c);                            \
        }                                                   \
    }                                                       \
}


static void parser_init (MooTermParser  *parser)
{
    parser->save = FALSE;

    if (parser->input.old_data->len)
    {
        parser->current.old = TRUE;
        parser->current.offset = 0;
    }
    else
    {
        parser->current.old = FALSE;
        parser->current.offset = 0;
    }

    iter_set_eof (parser->cmd_start);

    parser->state = INITIAL_;

    g_string_truncate (parser->character, 0);
    g_string_truncate (parser->intermediate, 0);
    g_string_truncate (parser->parameters, 0);
    g_string_truncate (parser->data, 0);
    parser->final = 0;
}


static void parser_finish (MooTermParser  *parser)
{
    if (!parser->save)
        g_string_truncate (parser->input.old_data, 0);
}


MooTermParser  *moo_term_parser_new     (MooTerm        *term)
{
    MooTermParser *p = g_new0 (MooTermParser, 1);

    p->term = term;

    p->character = g_string_new ("");
    p->intermediate = g_string_new ("");
    p->parameters = g_string_new ("");
    p->data = g_string_new ("");
    p->input.old_data = g_string_new ("");

    p->numbers = g_array_sized_new (FALSE, FALSE,
                                    sizeof(int),
                                    MAX_PARAMS_NUM);

    return p;
}


void            moo_term_parser_reset   (MooTermParser  *parser)
{
    g_string_truncate (parser->character, 0);
    g_string_truncate (parser->intermediate, 0);
    g_string_truncate (parser->parameters, 0);
    g_string_truncate (parser->data, 0);
    g_string_truncate (parser->input.old_data, 0);
}


void            moo_term_parser_free    (MooTermParser  *parser)
{
    if (parser)
    {
        g_string_free (parser->character, TRUE);
        g_string_free (parser->intermediate, TRUE);
        g_string_free (parser->parameters, TRUE);
        g_string_free (parser->data, TRUE);
        g_string_free (parser->input.old_data, TRUE);
        g_array_free (parser->numbers, TRUE);
        g_free (parser);
    }
}


static void exec_escape_sequence   (MooTermParser  *parser);
static void exec_cmd               (MooTermParser  *parser,
                                    guchar          cmd);
static void exec_apc               (MooTermParser  *parser,
                                    guchar          final);
static void exec_pm                (MooTermParser  *parser,
                                    guchar          final);
static void exec_osc               (MooTermParser  *parser,
                                    guchar          final);
static void exec_csi               (MooTermParser  *parser);
static void exec_dcs               (MooTermParser  *parser);


void            moo_term_parser_parse   (MooTermParser  *parser,
                                         const char     *string,
                                         guint           len)
{
    guchar c;

    if (!len)
        return;

    parser->input.data = string;
    parser->input.data_len = len;
    parser_init (parser);

    goto STATE_INITIAL_;

    /* INITIAL - everything starts here. checks input for command sequence start */
STATE_INITIAL_:
{
    get_char (c);
    check_cancel (c, STATE_INITIAL_);

    process_char (c);

    goto STATE_INITIAL_;
}
g_assert_not_reached ();


/* ESCAPE - got escape char */
STATE_ESCAPE_:
{
    get_char (c);
    check_cancel (c, STATE_ESCAPE_);

    switch (c)
    {
        case 0x5B:
            goto STATE_CSI_;
        case 0x5D:
            goto STATE_OSC_;
        case 0x5E:
            goto STATE_PM_;
        case 0x5F:
            goto STATE_APC_;
        case 0x50:
            goto STATE_DCS_;
    }

    /* 0x20-0x2F - intermediate character */
    if (0x20 <= c && c <= 0x2F)
    {
        append_char (intermediate, c);
        goto STATE_ESCAPE_INTERMEDIATE_;
    }
    /* 0x30-0x7E - final character */
    else if (0x30 <= c && c <= 0x7E)
    {
        parser->final = c;
        exec_escape_sequence (parser);
        goto_initial ();
    }
    /* DEL - ignored */
    else if (c == 0x7F)
    {
        goto STATE_ESCAPE_;
    }
    else
    {
        g_warning ("%s: got char '%c' after ESC", G_STRLOC, c);
        goto_initial ();
    }
}
g_assert_not_reached ();


STATE_ESCAPE_INTERMEDIATE_:
{
    get_char (c);
    check_cancel (c, STATE_ESCAPE_INTERMEDIATE_);

    /* 0x20-0x2F - intermediate character */
    if (0x20 <= c && c <= 0x2F)
    {
        append_char (intermediate, c);
        goto STATE_ESCAPE_INTERMEDIATE_;
    }
    /* 0x30-0x7E - final character */
    else if (0x30 <= c && c <= 0x7E)
    {
        parser->final = c;
        exec_escape_sequence (parser);
        goto_initial ();
    }
    /* DEL - ignored */
    else if (c == 0x7F)
    {
        goto STATE_ESCAPE_INTERMEDIATE_;
    }
    else
    {
        g_warning ("%s: got char '%c' after ESC", G_STRLOC, c);
        goto_initial ();
    }
}
g_assert_not_reached ();


/* APC - Application program command - ignore everything until ??? */
STATE_APC_:
{
    get_char (c);

    switch (c)
    {
        /* CAN - cancel sequence in progress */
        case 0x18:
            goto_initial ();

        /* SUB - cancel sequence in progress and
            print error */
        case 0x1A:
            vt_print_char (ERROR_CHAR);
            goto_initial ();

        case 0x1B:
        case 0x05:
        case 0x07:
        case 0x08:
        case 0x09:
        case 0x0A:
        case 0x0B:
        case 0x0C:
        case 0x0D:
        case 0x0E:
        case 0x0F:
        case 0x11:
        case 0x13:
            exec_apc (parser, c);
            goto_initial ();

        default:
            append_char (data, c);
            goto STATE_APC_;
    }
}
g_assert_not_reached ();


/* PM - Privacy message - ignore everything until ??? */
STATE_PM_:
{
    get_char (c);

    switch (c)
    {
        /* CAN - cancel sequence in progress */
        case 0x18:
            goto_initial ();

        /* SUB - cancel sequence in progress and
            print error */
        case 0x1A:
            vt_print_char (ERROR_CHAR);
            goto_initial ();

        case 0x1B:
        case 0x05:
        case 0x07:
        case 0x08:
        case 0x09:
        case 0x0A:
        case 0x0B:
        case 0x0C:
        case 0x0D:
        case 0x0E:
        case 0x0F:
        case 0x11:
        case 0x13:
            exec_pm (parser, c);
            goto_initial ();

        default:
            append_char (data, c);
            goto STATE_PM_;
    }
}
g_assert_not_reached ();


/* OSC - Operating system command - ignore everything until ??? */
STATE_OSC_:
{
    get_char (c);

    switch (c)
    {
        /* CAN - cancel sequence in progress */
        case 0x18:
            goto_initial ();

        /* SUB - cancel sequence in progress and
            print error */
        case 0x1A:
            vt_print_char (ERROR_CHAR);
            goto_initial ();

        case 0x1B:
        case 0x05:
        case 0x07:
        case 0x08:
        case 0x09:
        case 0x0A:
        case 0x0B:
        case 0x0C:
        case 0x0D:
        case 0x0E:
        case 0x0F:
        case 0x11:
        case 0x13:
            exec_osc (parser, c);
            goto_initial ();

        default:
            append_char (data, c);
            goto STATE_OSC_;
    }
}
g_assert_not_reached ();


/* CSI - control sequence introducer */
STATE_CSI_:
{
    get_char (c);

    check_cancel (c, STATE_CSI_);

    if (0x30 <= c && c <= 0x3F)
    {
        append_char (parameters, c);
        goto STATE_CSI_;
    }
    else if (0x20 <= c && c <= 0x2F)
    {
        append_char (intermediate, c);
        goto STATE_CSI_INTERMEDIATE_;
    }
    else if (0x40 <= c && c <= 0x7E)
    {
        parser->final = c;
        exec_csi (parser);
        goto_initial ();
    }
    else
    {
        char *s = _moo_term_nice_char (c);
        g_warning ("%s: got '%s' after CSI", G_STRLOC, s);
        g_free (s);
        goto_initial ();
    }
}
g_assert_not_reached ();


/* STATE_CSI_INTERMEDIATE - CSI, gathering intermediate characters */
STATE_CSI_INTERMEDIATE_:
{
    get_char (c);

    check_cancel (c, STATE_CSI_INTERMEDIATE_);

    if (0x20 <= c && c <= 0x2F)
    {
        append_char (intermediate, c);
        goto STATE_CSI_INTERMEDIATE_;
    }
    else if (0x40 <= c && c <= 0x7E)
    {
        parser->final = c;
        exec_csi (parser);
        goto_initial ();
    }
    else
    {
        char *s = _moo_term_nice_char (c);
        g_warning ("%s: got '%s' after CSI", G_STRLOC, s);
        g_free (s);
        goto_initial ();
    }
}
g_assert_not_reached ();


/* DCS - Device control string */
STATE_DCS_:
{
    get_char (c);
    dcs_check_cancel (c);

    if (0x30 <= c && c <= 0x3F)
    {
        append_char (parameters, c);
        goto STATE_DCS_;
    }
    else if (0x20 <= c && c <= 0x2F)
    {
        append_char (intermediate, c);
        goto STATE_DCS_INTERMEDIATE_;
    }
    else if (0x40 <= c && c <= 0x7E)
    {
        parser->final = c;
        goto STATE_DCS_DATA_;
    }
    else
    {
        char *s = _moo_term_nice_char (c);
        g_warning ("%s: got '%s' after DCS", G_STRLOC, s);
        g_free (s);
        goto_initial ();
    }
}
g_assert_not_reached ();


/* DCS - Device control string */
STATE_DCS_INTERMEDIATE_:
{
    get_char (c);
    dcs_check_cancel (c);

    if (0x20 <= c && c <= 0x2F)
    {
        append_char (intermediate, c);
        goto STATE_DCS_INTERMEDIATE_;
    }
    else if (0x40 <= c && c <= 0x7E)
    {
        parser->final = c;
        goto STATE_DCS_DATA_;
    }
    else
    {
        char *s = _moo_term_nice_char (c);
        g_warning ("%s: got '%s' after DCS", G_STRLOC, s);
        g_free (s);
        goto_initial ();
    }
}
g_assert_not_reached ();


/* DCS - Device control string */
STATE_DCS_DATA_:
{
    get_char (c);
    dcs_check_cancel (c);
    append_char (data, c);
    goto STATE_DCS_DATA_;
}
g_assert_not_reached ();


/* STATE_DCS_ESCAPE_ - got escape char in the DCS sequence */
STATE_DCS_ESCAPE_:
{
    get_char (c);

    if (c != 0x5c)
    {
        iter_backward (parser->current);
        goto_escape ();
    }
    else
    {
        exec_dcs (parser);
        goto_initial ();
    }
}
g_assert_not_reached ();
}


/***************************************************************************/
/* Parsing and executing received command sequence
 */

static void exec_cmd               (MooTermParser  *parser,
                                    guchar          cmd)
{
    switch (cmd)
    {
        case 0x07:
            vt_BEL ();
            break;
        case 0x08:
            vt_BS ();
            break;
        case 0x09:
            vt_TAB ();
            break;
        case 0x0A:
        case 0x0B:
        case 0x0C:
            vt_LF ();
            break;
        case 0x0D:
            vt_CR ();
            break;
        case 0x0E:
            vt_SO ();
            break;
        case 0x0F:
            vt_SI ();
            break;
        case 0x11:
            vt_XON ();
            break;
        case 0x13:
            vt_XOFF ();
            break;

        case 0x84:
            vt_IND ();
            break;
        case 0x85:
            vt_NEL ();
            break;
        case 0x88:
            vt_HTS ();
            break;
        case 0x8D:
            vt_RI ();
            break;
        case 0x8E:
            vt_SS2 ();
            break;
        case 0x8F:
            vt_SS3 ();
            break;
        case 0x9A:
            vt_DECID ();
            break;

        case 0x05:
            break;

        default:
            g_assert_not_reached ();
    }
}


static void init_yyparse    (MooTermParser  *parser,
                             LexType         lex_type)
{
    parser->lex.lex = lex_type;
    parser->lex.part = PART_START;
    parser->lex.offset = 0;

    g_array_set_size (parser->numbers, 0);
}


int     _moo_term_yylex (MooTermParser  *parser)
{
    guint offset = parser->lex.offset;

    switch (parser->lex.lex)
    {
        case LEX_ESCAPE:
            switch (parser->lex.part)
            {
                case PART_START:
                    if (!parser->intermediate->len)
                        parser->lex.part = PART_FINAL;
                    else
                        parser->lex.part = PART_INTERMEDIATE;
                    return 0x1B;

                case PART_INTERMEDIATE:
                    if (offset + 1 == parser->intermediate->len)
                        parser->lex.part = PART_FINAL;
                    else
                        parser->lex.offset++;
                    return parser->intermediate->str[offset];

                case PART_FINAL:
                    parser->lex.part = PART_DONE;
                    return parser->final;

                case PART_DONE:
                    return 0;

                case PART_PARAMETERS:
                case PART_DATA:
                case PART_ST:
                    g_assert_not_reached ();
            }

        case LEX_CONTROL:
            switch (parser->lex.part)
            {
                case PART_START:
                    if (!parser->parameters->len)
                    {
                        if (!parser->intermediate->len)
                            parser->lex.part = PART_FINAL;
                        else
                            parser->lex.part = PART_INTERMEDIATE;
                    }
                    else
                    {
                        parser->lex.part = PART_PARAMETERS;
                    }
                    return 0x9B;

                case PART_PARAMETERS:
                    if (offset + 1 == parser->parameters->len)
                    {
                        parser->lex.offset = 0;

                        if (!parser->intermediate->len)
                            parser->lex.part = PART_FINAL;
                        else
                            parser->lex.part = PART_INTERMEDIATE;
                    }
                    else
                    {
                        parser->lex.offset++;
                    }
                    return parser->parameters->str[offset];

                case PART_INTERMEDIATE:
                    if (offset + 1 == parser->intermediate->len)
                        parser->lex.part = PART_FINAL;
                    else
                        parser->lex.offset++;
                    return parser->intermediate->str[offset];

                case PART_FINAL:
                    parser->lex.part = PART_DONE;
                    return parser->final;

                case PART_DONE:
                    return 0;

                case PART_DATA:
                case PART_ST:
                    g_assert_not_reached ();
            }

        case LEX_DCS:
            switch (parser->lex.part)
            {
                case PART_START:
                    if (!parser->parameters->len)
                    {
                        if (!parser->intermediate->len)
                            parser->lex.part = PART_FINAL;
                        else
                            parser->lex.part = PART_INTERMEDIATE;
                    }
                    else
                    {
                        parser->lex.part = PART_PARAMETERS;
                    }
                    return 0x90;

                case PART_PARAMETERS:
                    if (offset + 1 == parser->parameters->len)
                    {
                        parser->lex.offset = 0;

                        if (!parser->intermediate->len)
                            parser->lex.part = PART_FINAL;
                        else
                            parser->lex.part = PART_INTERMEDIATE;
                    }
                    else
                    {
                        parser->lex.offset++;
                    }
                    return parser->parameters->str[offset];

                case PART_INTERMEDIATE:
                    if (offset + 1 == parser->intermediate->len)
                        parser->lex.part = PART_FINAL;
                    else
                        parser->lex.offset++;
                    return parser->intermediate->str[offset];

                case PART_FINAL:
                    if (!parser->data->len)
                    {
                        parser->lex.part = PART_ST;
                    }
                    else
                    {
                        parser->lex.part = PART_DATA;
                        parser->lex.offset = 0;
                    }
                    return parser->final;

                case PART_DATA:
                    if (offset + 1 == parser->data->len)
                        parser->lex.part = PART_ST;
                    else
                        parser->lex.offset++;
                    return parser->data->str[offset];

                case PART_ST:
                    parser->lex.part = PART_DONE;
                    return 0x9C;

                case PART_DONE:
                    return 0;
            }

        default:
            g_assert_not_reached ();
    }
}


char           *_moo_term_current_ctl   (MooTermParser  *parser)
{
    GString *s;
    char *nice;

    s = g_string_new ("");

    switch (parser->lex.lex)
    {
        case LEX_ESCAPE:
            g_string_append_c (s, '\033');
            g_string_append_len (s, parser->intermediate->str,
                                 parser->intermediate->len);
            g_string_append_c (s, parser->final);
            break;

        case LEX_CONTROL:
            g_string_append_c (s, '\233');
            g_string_append_len (s, parser->parameters->str,
                                 parser->parameters->len);
            g_string_append_len (s, parser->intermediate->str,
                                 parser->intermediate->len);
            g_string_append_c (s, parser->final);
            break;

        case LEX_DCS:
            g_string_append_c (s, '\220');
            g_string_append_len (s, parser->parameters->str,
                                 parser->parameters->len);
            g_string_append_len (s, parser->intermediate->str,
                                 parser->intermediate->len);
            g_string_append_c (s, parser->final);
            g_string_append_len (s, parser->data->str,
                                 parser->data->len);
            g_string_append_c (s, '\234');
            break;

    }

    nice = _moo_term_nice_bytes (s->str, s->len);
    g_string_free (s, TRUE);
    return nice;
}


static void exec_escape_sequence   (MooTermParser  *parser)
{
    init_yyparse (parser, LEX_ESCAPE);
    debug_control ();
    _moo_term_yyparse (parser);
}


static void exec_csi               (MooTermParser  *parser)
{
    init_yyparse (parser, LEX_CONTROL);
    debug_control ();
    _moo_term_yyparse (parser);
}


static void exec_dcs               (MooTermParser  *parser)
{
    init_yyparse (parser, LEX_DCS);
    debug_control ();
    _moo_term_yyparse (parser);
}


void            _moo_term_yyerror       (MooTermParser  *parser,
                                         G_GNUC_UNUSED const char     *string)
{
    char *s = _moo_term_current_ctl (parser);
    g_warning ("parse error: '%s'\n", s);
    g_free (s);
}


static void exec_apc               (MooTermParser  *parser,
                                    guchar          final)
{
    char *s = g_strdup_printf ("\237%s%c", parser->data->str, final);
    char *nice = _moo_term_nice_bytes (s, -1);
    g_print ("%s\n", nice);
    g_free (nice);
    g_free (s);
}


static void exec_pm                (MooTermParser  *parser,
                                    guchar          final)
{
    char *s = g_strdup_printf ("\236%s%c", parser->data->str, final);
    char *nice = _moo_term_nice_bytes (s, -1);
    g_print ("%s\n", nice);
    g_free (nice);
    g_free (s);
}


static void exec_osc               (MooTermParser  *parser,
                                    guchar          final)
{
    if (parser->data->len >= 2 &&
        parser->data->str[0] >= '0' &&
        parser->data->str[0] <= '2' &&
        parser->data->str[1] == ';')
    {
        char *title = parser->data->str + 2;
        char cmd = parser->data->str[0];

        switch (cmd)
        {
            case '0':
                vt_SET_ICON_NAME (title);
                vt_SET_WINDOW_TITLE (title);
                break;
            case '1':
                vt_SET_ICON_NAME (title);
                break;
            case '2':
                vt_SET_WINDOW_TITLE (title);
                break;
        }
    }
    else
    {
        char *s = g_strdup_printf ("\235%s%c", parser->data->str, final);
        char *nice = _moo_term_nice_bytes (s, -1);
        g_print ("%s\n", nice);
        g_free (nice);
        g_free (s);
    }
}


char           *_moo_term_nice_char     (guchar          c)
{
    if (' ' <= c && c <= '~')
    {
        return g_strndup (&c, 1);
    }
    else
    {
        switch (c)
        {
            case 0x1B:
                return g_strdup ("<ESC>");
            case 0x84:
                return g_strdup ("<IND>");
                break;
            case 0x85:
                return g_strdup ("<NEL>");
                break;
            case 0x88:
                return g_strdup ("<HTS>");
                break;
            case 0x8D:
                return g_strdup ("<RI>");
                break;
            case 0x8E:
                return g_strdup ("<SS2>");
                break;
            case 0x8F:
                return g_strdup ("<SS3>");
                break;
            case 0x90:
                return g_strdup ("<DCS>");
                break;
            case 0x98:
                return g_strdup ("<SOS>");
                break;
            case 0x9A:
                return g_strdup ("<DECID>");
                break;
            case 0x9B:
                return g_strdup ("<CSI>");
                break;
            case 0x9C:
                return g_strdup ("<ST>");
                break;
            case 0x9D:
                return g_strdup ("<OSC>");
                break;
            case 0x9E:
                return g_strdup ("<PM>");
                break;
            case 0x9F:
                return g_strdup ("<APC>");
                break;
            default:
                if ('A' - 64 <= c && c <= ']' - 64)
                    return g_strdup_printf ("^%c", c + 64);
                else
                    return g_strdup_printf ("<%d>", c);
        }
    }
}


char           *_moo_term_nice_bytes    (const char     *string,
                                         int             len)
{
    int i;
    GString *str;

    if (len < 0)
        len = strlen (string);

    str = g_string_new ("");

    for (i = 0; i < len; ++i)
    {
        char *s = _moo_term_nice_char (string[i]);
        g_string_append (str, s);
        g_free (s);
    }

    return g_string_free (str, FALSE);
}
