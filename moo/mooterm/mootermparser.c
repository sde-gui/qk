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
#include "mooterm/mooterm-ctlfuncs.h"
#include <string.h>


#define INVALID_CHAR        "?"
#define INVALID_CHAR_LEN    1


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
    parser->state = INITIAL;                                \
    iter_set_eof (parser->cmd_start);                       \
    goto STATE_INITIAL;                                     \
}

#define goto_cmd_state(st)                                  \
{                                                           \
    flush_chars ();                                         \
                                                            \
    parser->cmd_start = parser->current;                    \
    iter_backward (parser->cmd_start);                      \
    if (parser->escape_two_bytes)                           \
    {                                                       \
        iter_backward (parser->cmd_start);                  \
    }                                                       \
                                                            \
    g_string_truncate (parser->intermediate, 0);            \
    g_string_truncate (parser->parameters, 0);              \
    g_string_truncate (parser->data, 0);                    \
    parser->final = 0;                                      \
                                                            \
    parser->state = st;                                     \
                                                            \
    goto STATE_##st;                                        \
}

#define one_char_cmd(c)                                     \
{                                                           \
    flush_chars ();                                         \
                                                            \
    parser->cmd_start = parser->current;                    \
    if (parser->escape_two_bytes)                           \
    {                                                       \
        iter_backward (parser->cmd_start);                  \
    }                                                       \
                                                            \
    exec_cmd (parser, c);                                   \
}

#define check_cancel(c)                                     \
{                                                           \
    switch (c)                                              \
    {                                                       \
        /* ESC - start new sequence */                      \
        case 0x1B:                                          \
            goto_cmd_state (ESCAPE);                        \
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
        case 0x9B:                                          \
            goto_cmd_state (CSI);                           \
        case 0x9D:                                          \
            goto_cmd_state (OSC);                           \
        case 0x90:                                          \
            goto_cmd_state (DCS);                           \
        case 0x9E:                                          \
            goto_cmd_state (PM);                            \
        case 0x9F:                                          \
            goto_cmd_state (APC);                           \
    }                                                       \
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
                                                            \
    if (c == 0x1B)                                          \
    {                                                       \
        if (iter_eof (parser->current))                     \
        {                                                   \
            flush_chars ();                                 \
                                                            \
            if (!iter_eof (parser->cmd_start))              \
            {                                               \
                save_cmd ();                                \
            }                                               \
            else                                            \
            {                                               \
                save_char (c);                              \
            }                                               \
                                                            \
            parser_finish (parser);                         \
            return;                                         \
        }                                                   \
        else                                                \
        {                                                   \
            guchar c2;                                      \
                                                            \
            iter_get_char (parser->current, c2);            \
                                                            \
            if (c2 == 0x50 ||                               \
                (0x5B <= c2 && c2 <= 0x5F))                 \
            {                                               \
                c = c2 + 0x40;                              \
                iter_forward (parser->current);             \
                parser->escape_two_bytes = TRUE;            \
            }                                               \
            else                                            \
            {                                               \
                parser->escape_two_bytes = FALSE;           \
            }                                               \
        }                                                   \
    }                                                       \
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
    parser->escape_two_bytes = FALSE;

    parser->state = INITIAL;

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
static void exec_apc               (MooTermParser  *parser);
static void exec_pm                (MooTermParser  *parser);
static void exec_osc               (MooTermParser  *parser);
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

    goto STATE_INITIAL;

    /* INITIAL - everything starts here. checks input for command sequence start */
STATE_INITIAL:
{
    get_char (c);

    switch (c)
    {
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
        case 0x84:
        case 0x85:
        case 0x88:
        case 0x8D:
        case 0x8E:
        case 0x8F:
        case 0x9A:
            one_char_cmd (c);
            goto_initial ();

        case 0x18:
        case 0x1A:
        case 0x7F:
        case 0x98:
            flush_chars ();
            break;

        case 0x1B:
            goto_cmd_state (ESCAPE);

        case 0x9C:
            /* ST out of context */
            g_warning ("%s: got ST out of context", G_STRLOC);
            flush_chars ();
            break;

        case 0x90:
            goto_cmd_state (DCS);

        case 0x9B:
            goto_cmd_state (CSI);

        case 0x9D:
            goto_cmd_state (OSC);

        case 0x9E:
            goto_cmd_state (PM);

        case 0x9F:
            goto_cmd_state (APC);

        default:
            process_char (c);
    }

    goto STATE_INITIAL;
}
g_assert_not_reached ();


    /* ESCAPE - got escape char */
STATE_ESCAPE:
{
    get_char (c);

    check_cancel (c);

    /* 0x20-0x2F - intermediate character */
    if (0x20 <= c && c <= 0x2F)
    {
        append_char (intermediate, c);
        goto STATE_ESCAPE_INTERMEDIATE;
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
        goto STATE_ESCAPE;
    }
    else
    {
        g_warning ("%s: got char '%c' after ESC", G_STRLOC, c);
        goto_initial ();
    }
}
g_assert_not_reached ();


STATE_ESCAPE_INTERMEDIATE:
{
    get_char (c);

    check_cancel (c);

    /* 0x20-0x2F - intermediate character */
    if (0x20 <= c && c <= 0x2F)
    {
        append_char (intermediate, c);
        goto STATE_ESCAPE_INTERMEDIATE;
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
        goto STATE_ESCAPE;
    }
    else
    {
        g_warning ("%s: got char '%c' after ESC", G_STRLOC, c);
        goto_initial ();
    }
}
g_assert_not_reached ();


    /* APC - Application program command - ignore everything until ??? */
STATE_APC:
{
    get_char (c);

    switch (c)
    {
        case 0x1A:
        case 0x84:
        case 0x85:
        case 0x88:
        case 0x8D:
        case 0x8E:
        case 0x8F:
        case 0x90:
        case 0x98:
        case 0x9A:
        case 0x9B:
        case 0x9C:
        case 0x9D:
        case 0x9E:
        case 0x9F:
            exec_apc (parser);
            goto_initial ();

        default:
            append_char (data, c);
            goto STATE_APC;
    }
}
g_assert_not_reached ();


    /* PM - Privacy message - ignore everything until ??? */
STATE_PM:
{
    get_char (c);

    switch (c)
    {
        case 0x1A:
        case 0x84:
        case 0x85:
        case 0x88:
        case 0x8D:
        case 0x8E:
        case 0x8F:
        case 0x90:
        case 0x98:
        case 0x9A:
        case 0x9B:
        case 0x9C:
        case 0x9D:
        case 0x9E:
        case 0x9F:
            exec_pm (parser);
            goto_initial ();

        default:
            append_char (data, c);
            goto STATE_PM;
    }
}
g_assert_not_reached ();


    /* OSC - Operating system command - ignore everything until ??? */
STATE_OSC:
{
    get_char (c);

    switch (c)
    {
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
        case 0x18:
        case 0x1A:
        case 0x1B:
        case 0x7F:
        case 0x84:
        case 0x85:
        case 0x88:
        case 0x8D:
        case 0x8E:
        case 0x8F:
        case 0x90:
        case 0x98:
        case 0x9A:
        case 0x9B:
        case 0x9C:
        case 0x9D:
        case 0x9E:
        case 0x9F:
            exec_osc (parser);
            goto_initial ();

        default:
            append_char (data, c);
            goto STATE_OSC;
    }
}
g_assert_not_reached ();


    /* CSI - control sequence introducer */
STATE_CSI:
{
    get_char (c);

    check_cancel (c);

    if (0x30 <= c && c <= 0x3F)
    {
        append_char (parameters, c);
        goto STATE_CSI;
    }
    else if (0x20 <= c && c <= 0x2F)
    {
        append_char (intermediate, c);
        goto STATE_CSI_INTERMEDIATE;
    }
    else if (0x40 <= c && c <= 0x7E)
    {
        parser->final = c;
        exec_csi (parser);
        goto_initial ();
    }
    else
    {
        goto_initial ();
    }
}
g_assert_not_reached ();


    /* STATE_CSI_INTERMEDIATE - CSI, gathering intermediate characters */
STATE_CSI_INTERMEDIATE:
{
    get_char (c);

    check_cancel (c);

    if (0x20 <= c && c <= 0x2F)
    {
        append_char (intermediate, c);
        goto STATE_CSI_INTERMEDIATE;
    }
    else if (0x40 <= c && c <= 0x7E)
    {
        parser->final = c;
        exec_csi (parser);
        goto_initial ();
    }
    else
    {
        goto_initial ();
    }
}
g_assert_not_reached ();


    /* DCS - Device control string */
STATE_DCS:
{
    get_char (c);

    check_cancel (c);

    if (0x30 <= c && c <= 0x3F)
    {
        append_char (parameters, c);
        goto STATE_DCS;
    }
    else if (0x20 <= c && c <= 0x2F)
    {
        append_char (intermediate, c);
        goto STATE_DCS_INTERMEDIATE;
    }
    else if (0x40 <= c && c <= 0x7E)
    {
        parser->final = c;
        goto STATE_DCS_DATA;
    }
    else
    {
        g_warning ("%s: got char '%c' after DCS", G_STRLOC, c);
        goto_initial ();
    }
}
g_assert_not_reached ();


    /* DCS - Device control string */
STATE_DCS_INTERMEDIATE:
{
    get_char (c);

    check_cancel (c);

    if (0x20 <= c && c <= 0x2F)
    {
        append_char (intermediate, c);
        goto STATE_DCS_INTERMEDIATE;
    }
    else if (0x40 <= c && c <= 0x7E)
    {
        parser->final = c;
        goto STATE_DCS_DATA;
    }
    else
    {
        g_warning ("%s: got char '%c' after DCS", G_STRLOC, c);
        goto_initial ();
    }
}
g_assert_not_reached ();


    /* DCS - Device control string */
STATE_DCS_DATA:
{
    get_char (c);

    check_cancel (c);

    if (c != 0x9C)
    {
        append_char (data, c);
        goto STATE_DCS_DATA;
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
//             g_print ("BEL\n");
            break;
        case 0x08:
            vt_BS ();
//             g_print ("BS\n");
            break;
        case 0x09:
            vt_TAB ();
//             g_print ("TAB\n");
            break;
        case 0x0A:
        case 0x0B:
        case 0x0C:
            vt_LF ();
//             g_print ("LF\n");
            break;
        case 0x0D:
            vt_CR ();
//             g_print ("CR\n");
            break;
        case 0x0E:
            vt_SO ();
//             g_print ("SO\n");
            break;
        case 0x0F:
            vt_SI ();
//             g_print ("SI\n");
            break;
        case 0x11:
            vt_XON ();
//             g_print ("XON\n");
            break;
        case 0x13:
            vt_XOFF ();
//             g_print ("XOFF\n");
            break;

        case 0x84:
            vt_IND ();
//             g_print ("IND\n");
            break;
        case 0x85:
            vt_NEL ();
//             g_print ("NEL\n");
            break;
        case 0x88:
            vt_HTS ();
//             g_print ("HTS\n");
            break;
        case 0x8D:
            vt_RI ();
//             g_print ("RI\n");
            break;
        case 0x8E:
            vt_SS2 ();
//             g_print ("SS2\n");
            break;
        case 0x8F:
            vt_SS3 ();
//             g_print ("SS3\n");
            break;
        case 0x9A:
            vt_DECID ();
//             g_print ("DECID\n");
            break;

        case 0x05:
            break;

        default:
            g_assert_not_reached ();
    }
}


static void init_lex    (MooTermParser  *parser,
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
            }

        default:
            g_assert_not_reached ();
    }
}


static char *get_ctl_string (MooTermParser  *parser)
{
    GString *s;
    char *nice;

    if (parser->lex.lex == LEX_ESCAPE)
    {
        s = g_string_new ("\033");
        g_string_append_len (s, parser->intermediate->str,
                             parser->intermediate->len);
        g_string_append_c (s, parser->final);
    }
    else
    {
        s = g_string_new ("CSI ");
        g_string_append_len (s, parser->parameters->str,
                             parser->parameters->len);
        g_string_append_len (s, parser->intermediate->str,
                             parser->intermediate->len);
        g_string_append_c (s, parser->final);
    }

    nice = _moo_term_nice_bytes (s->str, s->len);
    g_string_free (s, TRUE);
    return nice;
}


static void exec_escape_sequence   (MooTermParser  *parser)
{
    init_lex (parser, LEX_ESCAPE);

//     {
//         char *s;
//         s = get_ctl_string (parser);
//         g_print ("escape sequence: '%s'\n", s);
//         g_free (s);
//     }

    _moo_term_yyparse (parser);
}


static void exec_csi               (MooTermParser  *parser)
{
    init_lex (parser, LEX_CONTROL);

//     {
//     char *s;
//     s = get_ctl_string (parser);
//     g_print ("csi: '%s'\n", s);
//     g_free (s);
//     }

    _moo_term_yyparse (parser);
}


void            _moo_term_yyerror       (MooTermParser  *parser,
                                         const char     *string)
{
    char *nice = get_ctl_string (parser);
    g_warning ("parse error: '%s'\n", nice);
    g_free (nice);
}


static void exec_dcs               (MooTermParser  *parser)
{
    g_print ("dcs\n");
}


static void exec_apc               (MooTermParser  *parser)
{
    g_print ("apc\n");
}


static void exec_pm                (MooTermParser  *parser)
{
    g_print ("pm\n");
}


static void exec_osc               (MooTermParser  *parser)
{
    g_print ("osc\n");
}


#if 0
/*****************************************************************************/
/* exec_command
 */

#define warn_esc_sequence()                         \
{                                                   \
    char *bytes, *nice_bytes;                       \
    bytes = block_get_string (&parser->cmd_string); \
    nice_bytes = _moo_term_nice_bytes (bytes, -1);  \
    g_warning ("%s: control sequence '%s'",         \
               G_STRLOC, nice_bytes);               \
    g_free (nice_bytes);                            \
    g_free (bytes);                                 \
}

#define check_nums_len(cmd, n)                      \
{                                                   \
    if (parser->nums_len != n)                      \
    {                                               \
        g_warning ("%s: wrong number of arguments"  \
                   "for " #cmd " command",          \
                   G_STRLOC);                       \
        warn_esc_sequence ();                       \
        break;                                      \
    }                                               \
}

#define exec_cmd_1(name)                            \
{                                                   \
    check_nums_len (name, 1);                       \
    buf_vt_##name (parser->term_buffer,             \
                   parser->nums[0]);                \
}

#define exec_cmd_2(name)                            \
{                                                   \
    check_nums_len (name, 2);                       \
    buf_vt_##name (parser->term_buffer,             \
                   parser->nums[0],                 \
                   parser->nums[1]);                \
}

#define exec_cmd_5(name)                            \
{                                                   \
    check_nums_len (name, 5);                       \
    buf_vt_##name (parser->term_buffer,             \
                   parser->nums[0],                 \
                   parser->nums[1],                 \
                   parser->nums[2],                 \
                   parser->nums[3],                 \
                   parser->nums[4]);                \
}


static void     exec_decset             (MooTermBuffer  *buf,
                                         guint          *params,
                                         guint           num_params)
{
    guint i;

    if (!num_params)
    {
        g_warning ("%s: ???", G_STRLOC);
        return;
    }

    for (i = 0; i < num_params; ++i)
    {
        switch (params[i])
        {
            case 1:
                buf->priv->modes |= DECCKM;
                g_object_notify (G_OBJECT (buf), "mode-DECCKM");
                break;

            case 2: /* DECANM */
            case 3: /* DECCOLM */
            case 4: /* DECSCLM */
            case 8: /* DECARM */
            case 18: /* DECPFF */
            case 19: /* DECPEX */
            case 40: /* disallow 80 <-> 132 mode */
            case 45: /* no reverse wraparound mode */
                g_message ("%s: ignoring mode %d", G_STRLOC, params[i]);
                break;

            case 5:
                buf->priv->modes |= DECSCNM;
                g_object_notify (G_OBJECT (buf), "mode-DECSCNM");
                break;

            case 6:
                buf->priv->modes |= DECOM;
                g_object_notify (G_OBJECT (buf), "mode-DECOM");
                break;

            case 7:
                buf->priv->modes |= DECAWM;
                g_object_notify (G_OBJECT (buf), "mode-DECAWM");
                break;

            case 1049: /* turn on ca mode */
                break;

            case 1000:
                buf->priv->modes |= MOUSE_TRACKING;
                g_object_notify (G_OBJECT (buf), "mode-MOUSE-TRACKING");
                break;

            case 1001:
                buf->priv->modes |= HILITE_MOUSE_TRACKING;
                g_object_notify (G_OBJECT (buf), "mode-HILITE-MOUSE-TRACKING");
                break;

            default:
                g_warning ("%s: unknown mode %d", G_STRLOC, params[i]);
        }
    }
}


static void     exec_decrst             (MooTermBuffer  *buf,
                                         guint          *params,
                                         guint           num_params)
{
    guint i;

    if (!num_params)
    {
        g_warning ("%s: ???", G_STRLOC);
        return;
    }

    for (i = 0; i < num_params; ++i)
    {
        switch (params[i])
        {
            case 1:
                buf->priv->modes &= ~DECCKM;
                g_object_notify (G_OBJECT (buf), "mode-DECCKM");
                break;

            case 2: /* DECANM */
            case 3: /* DECCOLM */
            case 4: /* DECSCLM */
            case 8: /* DECARM */
            case 18: /* DECPFF */
            case 19: /* DECPEX */
            case 40: /* disallow 80 <-> 132 mode */
            case 45: /* no reverse wraparound mode */
                g_message ("%s: ignoring mode %d", G_STRLOC, params[i]);
                break;

            case 5:
                buf->priv->modes &= ~DECSCNM;
                g_object_notify (G_OBJECT (buf), "mode-DECSCNM");
                break;

            case 6:
                buf->priv->modes &= ~DECOM;
                g_object_notify (G_OBJECT (buf), "mode-DECOM");
                break;

            case 7:
                buf->priv->modes &= ~DECAWM;
                g_object_notify (G_OBJECT (buf), "mode-DECAWM");
                break;

            case 1049:
                break;

            case 1000:
                buf->priv->modes &= ~MOUSE_TRACKING;
                g_object_notify (G_OBJECT (buf), "mode-MOUSE-TRACKING");
                break;

            case 1001:
                buf->priv->modes &= ~HILITE_MOUSE_TRACKING;
                g_object_notify (G_OBJECT (buf), "mode-HILITE-MOUSE-TRACKING");
                break;

            default:
                g_warning ("%s: uknown mode %d", G_STRLOC, params[i]);
        }
    }
}


static void     exec_set_mode           (MooTermBuffer  *buf,
                                         guint          *params,
                                         guint           num_params)
{
    guint i;

    if (!num_params)
    {
        g_warning ("%s: ???", G_STRLOC);
        return;
    }

    for (i = 0; i < num_params; ++i)
    {
        switch (params[i])
        {
            case 4:
                buf->priv->modes |= IRM;
                g_object_notify (G_OBJECT (buf), "mode-IRM");
                break;

            case 20:
                buf->priv->modes |= LNM;
                g_object_notify (G_OBJECT (buf), "mode-LNM");
                break;

            case 2: /* KAM */
                g_warning ("%s: ignoring mode %d", G_STRLOC, params[i]);
                break;

            default:
                g_warning ("%s: uknown mode %d", G_STRLOC, params[i]);
        }
    }
}


static void     exec_reset_mode         (MooTermBuffer  *buf,
                                         guint          *params,
                                         guint           num_params)
{
    guint i;

    if (!num_params)
    {
        g_warning ("%s: ???", G_STRLOC);
        return;
    }

    for (i = 0; i < num_params; ++i)
    {
        switch (params[i])
        {
            case 4:
                buf->priv->modes &= ~IRM;
                g_object_notify (G_OBJECT (buf), "mode-IRM");
                break;

            case 20:
                buf->priv->modes &= ~LNM;
                g_object_notify (G_OBJECT (buf), "mode-LNM");
                break;

            case 2: /* KAM */
                g_warning ("%s: ignoring mode %d", G_STRLOC, params[i]);
                break;

            default:
                g_warning ("%s: uknown mode %d", G_STRLOC, params[i]);
        }
    }
}


static void     exec_sgr                (MooTermBuffer  *buf,
                                         guint          *params,
                                         guint           num_params)
{
    guint i;

    if (!num_params)
    {
        buf_set_attrs_mask (buf, 0);
        return;
    }

    for (i = 0; i < num_params; ++i)
    {
        switch (params[i])
        {
            case ANSI_ALL_ATTRS_OFF:
                buf_set_attrs_mask (buf, 0);
                break;
            case ANSI_BOLD:
                buf_add_attrs_mask (buf, MOO_TERM_TEXT_BOLD);
                break;
            case ANSI_UNDERSCORE:
                buf_add_attrs_mask (buf, MOO_TERM_TEXT_UNDERLINE);
                break;
            case ANSI_BLINK:
                g_warning ("%s: ignoring blink", G_STRLOC);
                break;
            case ANSI_REVERSE:
                buf_add_attrs_mask (buf, MOO_TERM_TEXT_REVERSE);
                break;
            case 20 + ANSI_BOLD:
                buf_remove_attrs_mask (buf, MOO_TERM_TEXT_BOLD);
                break;
            case 20 + ANSI_UNDERSCORE:
                buf_remove_attrs_mask (buf, MOO_TERM_TEXT_UNDERLINE);
                break;
            case 20 + ANSI_BLINK:
                g_warning ("%s: ignoring blink", G_STRLOC);
                break;
            case 20 + ANSI_REVERSE:
                buf_remove_attrs_mask (buf, MOO_TERM_TEXT_REVERSE);
                break;

            default:
                if (30 <= params[i] && params[i] <= 37)
                    buf_set_ansi_foreground (buf, params[i] - 30);
                else if (40 <= params[i] && params[i] <= 47)
                    buf_set_ansi_background (buf, params[i] - 40);
                else if (39 == params[i])
                    buf_set_ansi_foreground (buf, 8);
                else if (49 == params[i])
                    buf_set_ansi_background (buf, 8);
                else
                    g_warning ("%s: unknown text attribute %d",
                               G_STRLOC, params[i]);
        }
    }
}


static void     exec_dsr                (MooTermBuffer  *buf,
                                         guint          *params,
                                         guint           num_params)
{
    guint i;

    if (!num_params)
    {
        buf_vt_report_status (buf);
        return;
    }

    for (i = 0; i < num_params; ++i)
    {
        switch (params[i])
        {
            case 5:
                buf_vt_report_status (buf);
                break;

            case 6:
                buf_vt_report_active_position (buf);
                break;

            default:
                g_warning ("%s: invalid request %d",
                           G_STRLOC, params[i]);
        }
    }
}


static void     exec_restore_decset     (MooTermBuffer  *buf,
                                         guint          *params,
                                         guint           num_params)
{
    guint i;

    if (!num_params)
    {
        g_warning ("%s: ???", G_STRLOC);
        return;
    }

    for (i = 0; i < num_params; ++i)
    {
        switch (params[i])
        {
            case 1:
                if (buf->priv->saved_modes & DECCKM)
                    buf->priv->modes |= DECCKM;
                else
                    buf->priv->modes &= ~DECCKM;
                g_object_notify (G_OBJECT (buf), "mode-DECCKM");
                break;

            case 2: /* DECANM */
            case 3: /* DECCOLM */
            case 4: /* DECSCLM */
            case 8: /* DECARM */
            case 18: /* DECPFF */
            case 19: /* DECPEX */
            case 40: /* disallow 80 <-> 132 mode */
            case 45: /* no reverse wraparound mode */
                g_warning ("%s: ignoring mode %d", G_STRLOC, params[i]);
                break;

            case 5:
                if (buf->priv->saved_modes & DECSCNM)
                    buf->priv->modes |= DECSCNM;
                else
                    buf->priv->modes &= ~DECSCNM;
                g_object_notify (G_OBJECT (buf), "mode-DECSCNM");
                break;

            case 6:
                if (buf->priv->saved_modes & DECOM)
                    buf->priv->modes |= DECOM;
                else
                    buf->priv->modes &= ~DECOM;
                g_object_notify (G_OBJECT (buf), "mode-DECOM");
                break;

            case 7:
                if (buf->priv->saved_modes & DECAWM)
                    buf->priv->modes |= DECAWM;
                else
                    buf->priv->modes &= ~DECAWM;
                g_object_notify (G_OBJECT (buf), "mode-DECAWM");
                break;

            case 1000:
                if (buf->priv->saved_modes & MOUSE_TRACKING)
                    buf->priv->modes |= MOUSE_TRACKING;
                else
                    buf->priv->modes &= ~MOUSE_TRACKING;
                g_object_notify (G_OBJECT (buf), "mode-MOUSE_TRACKING");
                break;

            case 1001:
                if (buf->priv->saved_modes & HILITE_MOUSE_TRACKING)
                    buf->priv->modes |= HILITE_MOUSE_TRACKING;
                else
                    buf->priv->modes &= ~HILITE_MOUSE_TRACKING;
                g_object_notify (G_OBJECT (buf), "mode-HILITE_MOUSE_TRACKING");
                break;

            default:
                g_warning ("%s: unknown mode %d", G_STRLOC, params[i]);
        }
    }
}

static void     exec_save_decset        (MooTermBuffer  *buf,
                                         guint          *params,
                                         guint           num_params)
{
    guint i;

    if (!num_params)
    {
        g_warning ("%s: ???", G_STRLOC);
        buf->priv->saved_modes = buf->priv->modes;
        return;
    }

    for (i = 0; i < num_params; ++i)
    {
        switch (params[i])
        {
            case 1:
                if (buf->priv->modes & DECCKM)
                    buf->priv->saved_modes |= DECCKM;
                else
                    buf->priv->saved_modes &= ~DECCKM;
                break;

            case 2: /* DECANM */
            case 3: /* DECCOLM */
            case 4: /* DECSCLM */
            case 8: /* DECARM */
            case 18: /* DECPFF */
            case 19: /* DECPEX */
            case 40: /* disallow 80 <-> 132 mode */
            case 45: /* no reverse wraparound mode */
                g_warning ("%s: ignoring mode %d", G_STRLOC, params[i]);
                break;

            case 5:
                if (buf->priv->modes & DECSCNM)
                    buf->priv->saved_modes |= DECSCNM;
                else
                    buf->priv->saved_modes &= ~DECSCNM;
                break;

            case 6:
                if (buf->priv->modes & DECOM)
                    buf->priv->saved_modes |= DECOM;
                else
                    buf->priv->saved_modes &= ~DECOM;
                break;

            case 7:
                if (buf->priv->modes & DECAWM)
                    buf->priv->saved_modes |= DECAWM;
                else
                    buf->priv->saved_modes &= ~DECAWM;
                break;

            case 1000:
                if (buf->priv->modes & MOUSE_TRACKING)
                    buf->priv->saved_modes |= MOUSE_TRACKING;
                else
                    buf->priv->saved_modes &= ~MOUSE_TRACKING;
                break;

            case 1001:
                if (buf->priv->modes & HILITE_MOUSE_TRACKING)
                    buf->priv->saved_modes |= HILITE_MOUSE_TRACKING;
                else
                    buf->priv->saved_modes &= ~HILITE_MOUSE_TRACKING;
                break;

            default:
                g_warning ("%s: uknown mode %d", G_STRLOC, params[i]);
        }
    }
}


static void     exec_command            (MooTermParser  *parser)
{
#if 0
    if (parser->cmd != CMD_IGNORE)
    {
        char *bytes, *nice_bytes;
        bytes = block_get_string (&parser->cmd_string);
        nice_bytes = _moo_term_nice_bytes (bytes, -1);
        g_print ("command '%s'\n", nice_bytes);
        g_free (nice_bytes);
        g_free (bytes);
    }
#endif

    switch (parser->cmd)
    {
        case CMD_IGNORE:
        {
            char *bytes, *nice_bytes;
            bytes = block_get_string (&parser->cmd_string);
            nice_bytes = _moo_term_nice_bytes (bytes, -1);
            g_message ("%s: ignoring control sequence '%s'",
                       G_STRLOC, nice_bytes);
            g_free (nice_bytes);
            g_free (bytes);
        }
        break;

        case CMD_DECALN:
            buf_vt_decaln (parser->term_buffer);
            break;
        case CMD_DECSET:     /* see skip */
            exec_decset (parser->term_buffer, parser->nums, parser->nums_len);
            break;

        case CMD_DECRST:     /* see skip */
            exec_decrst (parser->term_buffer, parser->nums, parser->nums_len);
            break;

        case CMD_G0_CHARSET:
            buf_vt_select_charset (parser->term_buffer, 0, parser->nums[0]);
            break;
        case CMD_G1_CHARSET:
            buf_vt_select_charset (parser->term_buffer, 1, parser->nums[0]);
            break;
        case CMD_G2_CHARSET:
            buf_vt_select_charset (parser->term_buffer, 2, parser->nums[0]);
            break;
        case CMD_G3_CHARSET:
            buf_vt_select_charset (parser->term_buffer, 3, parser->nums[0]);
            break;

        case CMD_DECSC:
            buf_vt_save_cursor (parser->term_buffer);
            break;
        case CMD_DECRC:
            buf_vt_restore_cursor (parser->term_buffer);
            break;
        case CMD_IND:
            buf_vt_index (parser->term_buffer);
            break;
        case CMD_NEL:
            buf_vt_next_line (parser->term_buffer);
            break;
        case CMD_HTS:
            buf_vt_set_tab_stop (parser->term_buffer);
            break;
        case CMD_RI:
            buf_vt_reverse_index (parser->term_buffer);
            break;
        case CMD_SS2:
            buf_vt_single_shift (parser->term_buffer, 2);
            break;
        case CMD_SS3:
            buf_vt_single_shift (parser->term_buffer, 3);
            break;
        case CMD_DA:
            buf_vt_da (parser->term_buffer);
            break;
        case CMD_ICH:
            exec_cmd_1 (ich);
            break;
        case CMD_DCH:
            exec_cmd_1 (dch);
            break;
        case CMD_CUU:
            exec_cmd_1 (cuu);
            break;
        case CMD_CUD:
            exec_cmd_1 (cud);
            break;
        case CMD_CUF:
            exec_cmd_1 (cuf);
            break;
        case CMD_CUB:
            exec_cmd_1 (cub);
            break;
        case CMD_CUP:
            exec_cmd_2 (cup);
            break;
        case CMD_ED:
            check_nums_len (ED, 1);
            switch (parser->nums[0])
            {
                case 0:
                    buf_vt_erase_from_cursor (parser->term_buffer);
                    break;
                case 1:
                    buf_vt_erase_to_cursor (parser->term_buffer);
                    break;
                case 2:
                    buf_vt_erase_display (parser->term_buffer);
                    break;
                default:
                    g_warning ("%s: wrong argument %d for ED command",
                               G_STRLOC, parser->nums[0]);
                    warn_esc_sequence ();
                    break;
            }

        case CMD_EL:
            check_nums_len (EL, 1);
            switch (parser->nums[0])
            {
                case 0:
                    buf_vt_erase_line_from_cursor (parser->term_buffer);
                    break;
                case 1:
                    buf_vt_erase_line_to_cursor (parser->term_buffer);
                    break;
                case 2:
                    buf_vt_erase_line (parser->term_buffer);
                    break;
                default:
                    g_warning ("%s: wrong argument %d for EL command",
                               G_STRLOC, parser->nums[0]);
                    warn_esc_sequence ();
                    break;
            }

        case CMD_IL:
            exec_cmd_1 (il);
            break;
        case CMD_DL:
            exec_cmd_1 (dl);
            break;
        case CMD_INIT_HILITE_MOUSE_TRACKING:
            exec_cmd_5 (init_hilite_mouse_tracking);
            break;
        case CMD_TBC:
            check_nums_len (TBC, 1);
            switch (parser->nums[0])
            {
                case 0:
                    buf_vt_clear_tab_stop (parser->term_buffer);
                    break;
                case 3:
                    buf_vt_clear_all_tab_stops (parser->term_buffer);
                    break;
                default:
                    g_warning ("%s: wrong argument %d for TBC command",
                               G_STRLOC, parser->nums[0]);
                    warn_esc_sequence ();
                    break;
            }
        case CMD_SM:
            exec_set_mode (parser->term_buffer, parser->nums, parser->nums_len);
            break;
        case CMD_RM:
            exec_reset_mode (parser->term_buffer, parser->nums, parser->nums_len);
            break;
        case CMD_SGR:
            exec_sgr (parser->term_buffer, parser->nums, parser->nums_len);
            break;
        case CMD_DSR:
            exec_dsr (parser->term_buffer, parser->nums, parser->nums_len);
            break;
        case CMD_DECSTBM:
            exec_cmd_2 (set_scrolling_region);
            break;
        case CMD_DECREQTPARM:
            exec_cmd_1 (request_terminal_parameters);
            break;
        case CMD_RESTORE_DECSET:
            exec_restore_decset (parser->term_buffer, parser->nums, parser->nums_len);
            break;
        case CMD_SAVE_DECSET:
            exec_save_decset (parser->term_buffer, parser->nums, parser->nums_len);
            break;
        case CMD_SET_TEXT:
            check_nums_len (SET_TEXT_PARAMETERS, 1);

            {
                char *s = g_strndup (parser->string,
                                     parser->string_len);

                switch (parser->nums[0])
                {
                    case 0:
                        buf_vt_set_window_title (parser->term_buffer, s);
                        buf_vt_set_icon_name (parser->term_buffer, s);
                        g_free (s);
                        break;
                    case 1:
                        buf_vt_set_icon_name (parser->term_buffer, s);
                        g_free (s);
                        break;
                    case 2:
                        buf_vt_set_window_title (parser->term_buffer, s);
                        g_free (s);
                        break;
                    case 50:
                        g_warning ("%s: ignoring SET_FONT command",
                                   G_STRLOC);
                        warn_esc_sequence ();
                        g_free (s);
                        break;
                    case 46:
                        g_warning ("%s: ignoring SET_LOG_FILE command",
                                   G_STRLOC);
                        warn_esc_sequence ();
                        g_free (s);
                        break;
                    default:
                        g_warning ("%s: wrong argument %d for SET_TEXT_PARAMETERS command",
                                   G_STRLOC, parser->nums[0]);
                        warn_esc_sequence ();
                        g_free (s);
                        break;
                }
            }
            break;

        case CMD_RIS:
            buf_vt_full_reset (parser->term_buffer);
            break;
        case CMD_LS2:
            buf_vt_invoke_charset (parser->term_buffer, 2);
            break;
        case CMD_LS3:
            buf_vt_invoke_charset (parser->term_buffer, 3);
            break;
        case CMD_BELL:
            buf_vt_bell (parser->term_buffer);
            break;
        case CMD_BACKSPACE:
            buf_vt_backspace (parser->term_buffer);
            break;
        case CMD_TAB:
            buf_vt_tab (parser->term_buffer);
            break;
        case CMD_LINEFEED:
            buf_vt_linefeed (parser->term_buffer);
            break;
        case CMD_VERT_TAB:
            buf_vt_vert_tab (parser->term_buffer);
            break;
        case CMD_FORM_FEED:
            buf_vt_form_feed (parser->term_buffer);
            break;
        case CMD_CARRIAGE_RETURN:
            buf_vt_carriage_return (parser->term_buffer);
            break;
        case CMD_ALT_CHARSET:
            buf_vt_invoke_charset (parser->term_buffer, 1);
            break;
        case CMD_NORM_CHARSET:
            buf_vt_invoke_charset (parser->term_buffer, 0);
            break;
        case CMD_COLUMN_ADDRESS:
            exec_cmd_1 (column_address);
            break;
        case CMD_ROW_ADDRESS:
            exec_cmd_1 (row_address);
            break;
        case CMD_BACK_TAB:
            buf_vt_tab (parser->term_buffer);
            break;
        case CMD_RESET_2STRING:
            buf_vt_reset_2 (parser->term_buffer);
            break;

        case CMD_DECKPAM:
            moo_term_buffer_set_keypad_numeric (parser->term_buffer, FALSE);
            break;
        case CMD_DECKPNM:
            moo_term_buffer_set_keypad_numeric (parser->term_buffer, TRUE);
            break;

        case CMD_NONE:
        case CMD_ERROR:
            g_assert_not_reached ();
    }
}
#endif


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
        if (' ' <= string[i] && string[i] <= '~')
            g_string_append_printf (str, "%c", string[i]);
        else if ('A' - 64 <= string[i] && string[i] <= 'Z' - 64)
            g_string_append_printf (str, "^%c", string[i] + 64);
        else
            g_string_append_printf (str, "<%d>", string[i]);
    }

    return g_string_free (str, FALSE);
}
