/*
 *   mootermparser.c
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
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
#define DEBUG_ONE_CHAR(c__)                                     \
G_STMT_START {                                                  \
    char *s = _moo_term_nice_char (c__);                        \
    g_message ("got one-char '%s'", s);                         \
    g_free (s);                                                 \
} G_STMT_END
#define DEBUG_CONTROL                                           \
G_STMT_START {                                                  \
    char *s = _moo_term_current_ctl (parser);                   \
    g_message ("got sequence '%s'", s);                         \
    g_free (s);                                                 \
} G_STMT_END
#else
#define DEBUG_ONE_CHAR(c__)
#define DEBUG_CONTROL
#endif


static void  parser_finish          (MooTermParser  *parser);
static void  flush_chars            (MooTermParser  *parser);

static void  exec_escape_sequence   (MooTermParser  *parser);
static void  exec_cmd               (MooTermParser  *parser,
                                     guchar          cmd);
static void  exec_apc               (MooTermParser  *parser,
                                     guchar          final);
static void  exec_pm                (MooTermParser  *parser,
                                     guchar          final);
static void  exec_osc               (MooTermParser  *parser,
                                     guchar          final);
static void  exec_csi               (MooTermParser  *parser);
static void  exec_dcs               (MooTermParser  *parser);

static char *_moo_term_nice_char    (guchar          c);


#define iter_free g_free

static InputIter *
iter_new (MooTermParser *parser)
{
    InputIter *iter = g_new (InputIter, 1);

    iter->parser = parser;
    iter->old = FALSE;
    iter->offset = parser->input.data_len;

    return iter;
}


static gboolean
iter_eof (InputIter *iter)
{
    return !iter->old && iter->offset >=
            iter->parser->input.data_len;
}


static void
iter_forward (InputIter *iter)
{
    g_assert (!iter_eof (iter));

    ++iter->offset;

    if (iter->old)
    {
        if (iter->offset ==
                iter->parser->input.old_data->len)
        {
            iter->offset = 0;
            iter->old = FALSE;
        }
    }
}


static void
iter_backward (InputIter *iter)
{
    g_assert (iter->offset ||
            (!iter->old && iter->parser->input.old_data->len));

    if (iter->offset)
    {
        --iter->offset;
    }
    else
    {
        iter->offset =
                iter->parser->input.old_data->len - 1;
        iter->old = TRUE;
    }
}


static void
iter_set_eof (InputIter *iter)
{
    iter->old = FALSE;
    iter->offset = iter->parser->input.data_len;
}


static guchar
iter_get_char (InputIter *iter)
{
    g_assert (!iter_eof (iter));

    if (iter->old)
        return iter->parser->input.old_data->str[iter->offset];
    else
        return iter->parser->input.data[iter->offset];
}


static void
save_cmd (MooTermParser *parser)
{
    GString *old = parser->input.old_data;
    guint offset = parser->cmd_start->offset;

    g_assert (!iter_eof (parser->cmd_start));

    if (parser->cmd_start->old)
    {
        g_assert (offset < old->len);
        memmove (old->str, old->str + offset,
                 old->len - offset);
        g_string_truncate (old, old->len - offset);

        g_string_append_len (old,
                             (const char *) parser->input.data,
                             parser->input.data_len);
    }
    else
    {
        const char *data = (const char*) parser->input.data;
        guint data_len = parser->input.data_len;

        g_assert (offset < parser->input.data_len);

        g_string_truncate (old, 0);
        g_string_append_len (old,
                             data + offset,
                             data_len - offset);
    }

    parser->save = TRUE;
}


static void
save_character (MooTermParser *parser)
{
    g_string_truncate (parser->input.old_data, 0);
    g_string_append_len (parser->input.old_data,
                         parser->character->str,
                         parser->character->len);
    parser->save = TRUE;
}


static void
one_char_cmd (MooTermParser *parser,
              guchar         c)
{
    flush_chars (parser);
    DEBUG_ONE_CHAR(c);
    exec_cmd (parser, c);
}


#define GOTO_INITIAL                                        \
    iter_set_eof (parser->cmd_start);                       \
    goto STATE_INITIAL_;

#define GOTO_ESCAPE                                         \
    flush_chars (parser);                                   \
                                                            \
    *parser->cmd_start = *parser->current;                  \
    iter_backward (parser->cmd_start);                      \
                                                            \
    g_string_truncate (parser->intermediate, 0);            \
    g_string_truncate (parser->parameters, 0);              \
    g_string_truncate (parser->data, 0);                    \
    parser->final = 0;                                      \
                                                            \
    goto STATE_ESCAPE_;


#define CHECK_CANCEL(c, state)                              \
    switch (c)                                              \
    {                                                       \
        /* ESC - start new sequence */                      \
        case 0x1B:                                          \
            GOTO_ESCAPE                                     \
                                                            \
        /* CAN - cancel sequence in progress */             \
        case 0x18:                                          \
            GOTO_INITIAL                                    \
                                                            \
        /* SUB - cancel sequence in progress and            \
                 print error */                             \
        case 0x1A:                                          \
            VT_PRINT_CHAR (ERROR_CHAR);                     \
            GOTO_INITIAL                                    \
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
            one_char_cmd (parser, c);                       \
            goto state;                                     \
    }


#define DCS_CHECK_CANCEL(c)                                 \
    switch (c)                                              \
    {                                                       \
        /* ESC - it might be beginning of ST sequence */    \
        case 0x1B:                                          \
            goto STATE_DCS_ESCAPE_;                         \
                                                            \
        /* CAN - cancel sequence in progress */             \
        case 0x18:                                          \
            GOTO_INITIAL                                    \
                                                            \
        /* SUB - cancel sequence in progress and            \
            print error */                                  \
        case 0x1A:                                          \
            VT_PRINT_CHAR (ERROR_CHAR);                     \
            GOTO_INITIAL                                    \
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
            one_char_cmd (parser, c);                       \
            goto STATE_DCS_DATA_;                           \
    }


#define APPEND_CHAR(ar, c) g_string_append_c (parser->ar, c)


/* returns 0 on end of input */
static guchar
get_char (MooTermParser *parser)
{
    guchar c = 0;

    while (!c && !iter_eof (parser->current))
    {
        if (!(c = iter_get_char (parser->current)))
            iter_forward (parser->current);
    }

    if (!c)
    {
        if (!iter_eof (parser->cmd_start))
            save_cmd (parser);
        else if (parser->character->len)
            save_character (parser);

        parser_finish (parser);

        return 0;
    }

    iter_forward (parser->current);

    return c;
}


static void
flush_chars (MooTermParser *parser)
{
    if (parser->character->len)
    {
        if (0)
            g_warning ("%s: invalid UTF8 '%s'", G_STRLOC,
                       parser->character->str);
        VT_PRINT_CHAR (ERROR_CHAR);
        g_string_truncate (parser->character, 0);
    }
}


static void
process_char (MooTermParser *parser,
              guchar         c)
{
    if (parser->character->len || c >= 128)
    {
        gunichar uc;

        g_string_append_c (parser->character, c);

        uc = g_utf8_get_char_validated (parser->character->str,
                                        -1);

        switch (uc)
        {
            case -2:
                break;

            case -1:
                if (0)
                    g_warning ("%s: invalid UTF8 '%s'", G_STRLOC,
                               parser->character->str);
                VT_PRINT_CHAR (ERROR_CHAR);
                g_string_truncate (parser->character, 0);
                break;

            default:
                VT_PRINT_CHAR (uc);
                g_string_truncate (parser->character, 0);
                break;
        }
    }
    else
    {
        VT_PRINT_CHAR (c);
    }
}


static void
parser_init (MooTermParser *parser)
{
    parser->save = FALSE;

    if (parser->input.old_data->len)
    {
        parser->current->old = TRUE;
        parser->current->offset = 0;
    }
    else
    {
        parser->current->old = FALSE;
        parser->current->offset = 0;
    }

    iter_set_eof (parser->cmd_start);

    g_string_truncate (parser->character, 0);
    g_string_truncate (parser->intermediate, 0);
    g_string_truncate (parser->parameters, 0);
    g_string_truncate (parser->data, 0);
    parser->final = 0;
}


static void
parser_finish (MooTermParser *parser)
{
    if (!parser->save)
        g_string_truncate (parser->input.old_data, 0);
}


MooTermParser*
_moo_term_parser_new (MooTerm *term)
{
    MooTermParser *p = g_new0 (MooTermParser, 1);

    p->term = term;

    p->character = g_string_new ("");
    p->intermediate = g_string_new ("");
    p->parameters = g_string_new ("");
    p->data = g_string_new ("");
    p->input.old_data = g_string_new ("");

    p->current = iter_new (p);
    p->cmd_start = iter_new (p);

    p->numbers = g_array_sized_new (FALSE, FALSE,
                                    sizeof(int),
                                    MAX_PARAMS_NUM);

    return p;
}


#if 0
void
_moo_term_parser_reset (MooTermParser  *parser)
{
    g_string_truncate (parser->character, 0);
    g_string_truncate (parser->intermediate, 0);
    g_string_truncate (parser->parameters, 0);
    g_string_truncate (parser->data, 0);
    g_string_truncate (parser->input.old_data, 0);
}
#endif


void
_moo_term_parser_free (MooTermParser  *parser)
{
    if (parser)
    {
        g_string_free (parser->character, TRUE);
        g_string_free (parser->intermediate, TRUE);
        g_string_free (parser->parameters, TRUE);
        g_string_free (parser->data, TRUE);
        g_string_free (parser->input.old_data, TRUE);
        g_array_free (parser->numbers, TRUE);
        iter_free (parser->current);
        iter_free (parser->cmd_start);
        g_free (parser);
    }
}


static void
parser_do_parse (MooTermParser  *parser,
                 const char     *string,
                 guint           len)
{
    guchar c;

    if (!len)
        return;

    parser->input.data = (const guchar *) string;
    parser->input.data_len = len;
    parser_init (parser);

    goto STATE_INITIAL_;


/* INITIAL - everything starts here. checks input for command sequence start */
STATE_INITIAL_:
    if (!(c = get_char (parser)))
        return;

    CHECK_CANCEL (c, STATE_INITIAL_);

    process_char (parser, c);

    goto STATE_INITIAL_;


/* ESCAPE - got escape char */
STATE_ESCAPE_:
    if (!(c = get_char (parser)))
        return;

    CHECK_CANCEL (c, STATE_ESCAPE_);

    if (c == 0x5B)
        goto STATE_CSI_;
    else if (c == 0x5D)
        goto STATE_OSC_;
    else if (c == 0x5E)
        goto STATE_PM_;
    else if (c == 0x5F)
        goto STATE_APC_;
    else if (c == 0x50)
        goto STATE_DCS_;

    /* 0x20-0x2F - intermediate character */
    if (0x20 <= c && c <= 0x2F)
    {
        APPEND_CHAR (intermediate, c);
        goto STATE_ESCAPE_INTERMEDIATE_;
    }
    /* 0x30-0x7E - final character */
    else if (0x30 <= c && c <= 0x7E)
    {
        parser->final = c;
        exec_escape_sequence (parser);
        GOTO_INITIAL
    }
    /* DEL - ignored */
    else if (c == 0x7F)
    {
        goto STATE_ESCAPE_;
    }
    else
    {
        g_warning ("%s: got char '%c' after ESC", G_STRLOC, c);
        GOTO_INITIAL
    }


STATE_ESCAPE_INTERMEDIATE_:
    if (!(c = get_char (parser)))
        return;

    CHECK_CANCEL (c, STATE_ESCAPE_INTERMEDIATE_);

    /* 0x20-0x2F - intermediate character */
    if (0x20 <= c && c <= 0x2F)
    {
        APPEND_CHAR (intermediate, c);
        goto STATE_ESCAPE_INTERMEDIATE_;
    }
    /* 0x30-0x7E - final character */
    else if (0x30 <= c && c <= 0x7E)
    {
        parser->final = c;
        exec_escape_sequence (parser);
        GOTO_INITIAL
    }
    /* DEL - ignored */
    else if (c == 0x7F)
    {
        goto STATE_ESCAPE_INTERMEDIATE_;
    }
    else
    {
        g_warning ("%s: got char '%c' after ESC", G_STRLOC, c);
        GOTO_INITIAL
    }


/* APC - Application program command - ignore everything until ??? */
STATE_APC_:
    if (!(c = get_char (parser)))
        return;

    switch (c)
    {
        /* CAN - cancel sequence in progress */
        case 0x18:
            GOTO_INITIAL

        /* SUB - cancel sequence in progress and
            print error */
        case 0x1A:
            VT_PRINT_CHAR (ERROR_CHAR);
            GOTO_INITIAL

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
            GOTO_INITIAL

        default:
            APPEND_CHAR (data, c);
            goto STATE_APC_;
    }


/* PM - Privacy message - ignore everything until ??? */
STATE_PM_:
    if (!(c = get_char (parser)))
        return;

    switch (c)
    {
        /* CAN - cancel sequence in progress */
        case 0x18:
            GOTO_INITIAL

        /* SUB - cancel sequence in progress and
            print error */
        case 0x1A:
            VT_PRINT_CHAR (ERROR_CHAR);
            GOTO_INITIAL

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
            GOTO_INITIAL

        default:
            APPEND_CHAR (data, c);
            goto STATE_PM_;
    }


/* OSC - Operating system command - ignore everything until ??? */
STATE_OSC_:
    if (!(c = get_char (parser)))
        return;

    switch (c)
    {
        /* CAN - cancel sequence in progress */
        case 0x18:
            GOTO_INITIAL

        /* SUB - cancel sequence in progress and
            print error */
        case 0x1A:
            VT_PRINT_CHAR (ERROR_CHAR);
            GOTO_INITIAL

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
            GOTO_INITIAL

        default:
            APPEND_CHAR (data, c);
            goto STATE_OSC_;
    }


/* CSI - control sequence introducer */
STATE_CSI_:
    if (!(c = get_char (parser)))
        return;

    CHECK_CANCEL (c, STATE_CSI_);

    if (0x30 <= c && c <= 0x3F)
    {
        APPEND_CHAR (parameters, c);
        goto STATE_CSI_;
    }
    else if (0x20 <= c && c <= 0x2F)
    {
        APPEND_CHAR (intermediate, c);
        goto STATE_CSI_INTERMEDIATE_;
    }
    else if (0x40 <= c && c <= 0x7E)
    {
        parser->final = c;
        exec_csi (parser);
        GOTO_INITIAL
    }
    else
    {
        char *s = _moo_term_nice_char (c);
        g_warning ("%s: got '%s' after CSI", G_STRLOC, s);
        g_free (s);
        GOTO_INITIAL
    }


/* STATE_CSI_INTERMEDIATE - CSI, gathering intermediate characters */
STATE_CSI_INTERMEDIATE_:
    if (!(c = get_char (parser)))
        return;

    CHECK_CANCEL (c, STATE_CSI_INTERMEDIATE_);

    if (0x20 <= c && c <= 0x2F)
    {
        APPEND_CHAR (intermediate, c);
        goto STATE_CSI_INTERMEDIATE_;
    }
    else if (0x40 <= c && c <= 0x7E)
    {
        parser->final = c;
        exec_csi (parser);
        GOTO_INITIAL
    }
    else
    {
        char *s = _moo_term_nice_char (c);
        g_warning ("%s: got '%s' after CSI", G_STRLOC, s);
        g_free (s);
        GOTO_INITIAL
    }


/* DCS - Device control string */
STATE_DCS_:
    if (!(c = get_char (parser)))
        return;

    DCS_CHECK_CANCEL (c);

    if (0x30 <= c && c <= 0x3F)
    {
        APPEND_CHAR (parameters, c);
        goto STATE_DCS_;
    }
    else if (0x20 <= c && c <= 0x2F)
    {
        APPEND_CHAR (intermediate, c);
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
        GOTO_INITIAL
    }


/* DCS - Device control string */
STATE_DCS_INTERMEDIATE_:
    if (!(c = get_char (parser)))
        return;

    DCS_CHECK_CANCEL (c);

    if (0x20 <= c && c <= 0x2F)
    {
        APPEND_CHAR (intermediate, c);
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
        GOTO_INITIAL
    }


/* DCS - Device control string */
STATE_DCS_DATA_:
    if (!(c = get_char (parser)))
        return;

    DCS_CHECK_CANCEL (c);
    APPEND_CHAR (data, c);
    goto STATE_DCS_DATA_;


/* STATE_DCS_ESCAPE_ - got escape char in the DCS sequence */
STATE_DCS_ESCAPE_:
    if (!(c = get_char (parser)))
        return;

    if (c != 0x5c)
    {
        iter_backward (parser->current);
        GOTO_ESCAPE
    }
    else
    {
        exec_dcs (parser);
        GOTO_INITIAL
    }
}


gboolean
_moo_term_parser_parse (MooTermParser  *parser,
                        const char     *string,
                        guint           len)
{
    parser_do_parse (parser, string, len);

    if (0 && parser->save)
        g_print ("saved %" G_GSIZE_FORMAT " chars\n", parser->input.old_data->len);

    return !parser->save;
}


/***************************************************************************/
/* Parsing and executing received command sequence
 */

static void
exec_cmd (MooTermParser  *parser,
          guchar          cmd)
{
    switch (cmd)
    {
        case 0x07:
            VT_BEL;
            break;
        case 0x08:
            VT_BS;
            break;
        case 0x09:
            VT_TAB;
            break;
        case 0x0A:
        case 0x0B:
        case 0x0C:
            VT_LF;
            break;
        case 0x0D:
            VT_CR;
            break;
        case 0x0E:
            VT_SO;
            break;
        case 0x0F:
            VT_SI;
            break;
        case 0x11:
            VT_XON;
            break;
        case 0x13:
            VT_XOFF;
            break;

        case 0x84:
            VT_IND;
            break;
        case 0x85:
            VT_NEL;
            break;
        case 0x88:
            VT_HTS;
            break;
        case 0x8D:
            VT_RI;
            break;
        case 0x8E:
            VT_SS2;
            break;
        case 0x8F:
            VT_SS3;
            break;
        case 0x9A:
            VT_DECID;
            break;

        case 0x05:
            break;

        default:
            g_assert_not_reached ();
    }
}


static void
init_yyparse (MooTermParser *parser,
              LexType        lex_type)
{
    parser->lex.lex = lex_type;
    parser->lex.part = PART_START;
    parser->lex.offset = 0;

    g_array_set_size (parser->numbers, 0);
}


int
_moo_term_yylex (MooTermParser *parser)
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
    }

    g_return_val_if_reached (0);
}


char *
_moo_term_current_ctl (MooTermParser *parser)
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


static void
exec_escape_sequence (MooTermParser *parser)
{
    init_yyparse (parser, LEX_ESCAPE);
    DEBUG_CONTROL;
    _moo_term_yyparse (parser);
}


static void
exec_csi (MooTermParser *parser)
{
    init_yyparse (parser, LEX_CONTROL);
    DEBUG_CONTROL;
    _moo_term_yyparse (parser);
}


static void
exec_dcs (MooTermParser *parser)
{
    init_yyparse (parser, LEX_DCS);
    DEBUG_CONTROL;
    _moo_term_yyparse (parser);
}


void
_moo_term_yyerror (MooTermParser  *parser,
                   G_GNUC_UNUSED const char *string)
{
    char *s = _moo_term_current_ctl (parser);
    g_warning ("parse error: '%s'\n", s);
    g_free (s);
}


static void
exec_apc (MooTermParser *parser,
          guchar         final)
{
    char *s = g_strdup_printf ("\237%s%c", parser->data->str, final);
    char *nice = _moo_term_nice_bytes (s, -1);
    g_print ("%s\n", nice);
    g_free (nice);
    g_free (s);
}


static void
exec_pm (MooTermParser *parser,
         guchar         final)
{
    char *s = g_strdup_printf ("\236%s%c", parser->data->str, final);
    char *nice = _moo_term_nice_bytes (s, -1);
    g_print ("%s\n", nice);
    g_free (nice);
    g_free (s);
}


static void
exec_osc (MooTermParser *parser,
          guchar         final)
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


static char *
_moo_term_nice_char (guchar c)
{
    if (' ' <= c && c <= '~')
        return g_strndup ((char*)&c, 1);

    switch (c)
    {
        case 0x1B:
            return g_strdup ("<ESC>");
        case 0x84:
            return g_strdup ("<IND>");
        case 0x85:
            return g_strdup ("<NEL>");
        case 0x88:
            return g_strdup ("<HTS>");
        case 0x8D:
            return g_strdup ("<RI>");
        case 0x8E:
            return g_strdup ("<SS2>");
        case 0x8F:
            return g_strdup ("<SS3>");
        case 0x90:
            return g_strdup ("<DCS>");
        case 0x98:
            return g_strdup ("<SOS>");
        case 0x9A:
            return g_strdup ("<DECID>");
        case 0x9B:
            return g_strdup ("<CSI>");
        case 0x9C:
            return g_strdup ("<ST>");
        case 0x9D:
            return g_strdup ("<OSC>");
        case 0x9E:
            return g_strdup ("<PM>");
        case 0x9F:
            return g_strdup ("<APC>");
    }

    if ('A' - 64 <= c && c <= ']' - 64)
        return g_strdup_printf ("^%c", c + 64);
    else
        return g_strdup_printf ("<%d>", c);
}


char *
_moo_term_nice_bytes (const char *string,
                      int         len)
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
