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
#include "mooterm/mootermbuffer-private.h"
#include "mooterm/mootermparser-yacc.h"
#include "mooterm/mootermparser.h"
#include <string.h>


static void     exec_command            (MooTermParser  *parser);
static void     parser_init             (MooTermParser  *parser,
                                         const char     *string,
                                         guint           len);
static void     parser_deinit           (MooTermParser  *parser);
static gboolean parser_init_yyparse     (MooTermParser  *parser);


inline static gboolean  iter_equal      (Iter *iter1, Iter *iter2)
{
    return iter1->old == iter2->old && iter1->offset == iter2->offset;
}

inline static gboolean  iter_eof        (Iter *iter)
{
    return !iter->old && iter->offset == iter->parser->data_len;
}

inline static void      iter_set_eof    (Iter *iter)
{
    iter->old = FALSE;
    iter->offset = iter->parser->data_len;
}

inline static char      iter_get_char   (Iter *iter)
{
    g_assert (!iter_eof (iter));

    if (iter->old)
        return iter->parser->old_data[iter->offset];
    else
        return iter->parser->data[iter->offset];
}

inline static void      iter_forward    (Iter *iter)
{
    g_assert (!iter_eof (iter));

    ++iter->offset;

    if (iter->old)
    {
        if (iter->offset == iter->parser->old_data_len)
        {
            iter->offset = 0;
            iter->old = FALSE;
        }
    }
}

inline static char     *iter_get_range  (Iter *start,
                                         Iter *end)
{
    GString *str;

    g_assert (!iter_eof (start));

    if (start->old)
    {
        if (end->old)
        {
            g_assert (start->offset <= end->offset);

            if (start->offset == end->offset)
                return g_strdup ("");
            else
                return g_strndup (start->parser->old_data + start->offset,
                                  end->offset - start->offset);
        }
        else
        {
            str = g_string_new_len (start->parser->old_data,
                                    start->parser->old_data_len - start->offset);
            g_string_append_len (str, end->parser->data, end->offset);
            return g_string_free (str, FALSE);
        }
    }
    else
    {
        g_assert (!end->old);
        g_assert (start->offset <= end->offset);

        if (start->offset == end->offset)
            return g_strdup ("");
        else
            return g_strndup (start->parser->data + start->offset,
                              end->offset - start->offset);
    }
}

inline static char     *block_get_string    (Block *block)
{
    return iter_get_range (&block->start, &block->end);
}

inline static gboolean  block_is_empty      (Block *block)
{
    return iter_eof (&block->start) ||
            iter_equal (&block->start, &block->end);
}

inline static void      block_set_empty     (Block *block)
{
    iter_set_eof (&block->start);
    iter_set_eof (&block->end);
}


inline static void      chars_add_one       (MooTermParser  *parser)
{
    if (block_is_empty (&parser->chars))
        parser->chars.start = parser->chars.end = parser->current;

    iter_forward (&parser->chars.end);
}

inline static void      chars_add_cmd       (MooTermParser  *parser)
{
    if (block_is_empty (&parser->chars))
        parser->chars = parser->cmd_string;
    else
        parser->chars.end = parser->cmd_string.end;
}

inline static void      chars_flush         (MooTermParser  *parser)
{
    if (!block_is_empty (&parser->chars))
    {
        if (parser->chars.start.old)
        {
            if (parser->chars.end.old)
            {
                g_assert (parser->chars.start.offset < parser->chars.end.offset);

                moo_term_buffer_print_chars (parser->term_buffer,
                                             parser->old_data + parser->chars.start.offset,
                                             parser->chars.end.offset - parser->chars.start.offset);
            }
            else
            {
                moo_term_buffer_print_chars (parser->term_buffer,
                                             parser->old_data + parser->chars.start.offset,
                                             parser->old_data_len - parser->chars.start.offset);
                moo_term_buffer_print_chars (parser->term_buffer,
                                             parser->data,
                                             parser->chars.end.offset);
            }
        }
        else
        {
            g_assert (!parser->chars.end.old);
            g_assert (parser->chars.start.offset < parser->chars.end.offset);

            moo_term_buffer_print_chars (parser->term_buffer,
                                         parser->data + parser->chars.start.offset,
                                         parser->chars.end.offset - parser->chars.start.offset);
        }

        block_set_empty (&parser->chars);
    }
}


void            moo_term_parser_parse   (MooTermParser  *parser,
                                         const char     *string,
                                         gssize          len)
{
    guint length;

    g_return_if_fail (parser && (string || !len));

    if (!string || !len)
        return;

    length = len > 0 ? (guint)len : strlen (string);

    if (!length)
        return;

    parser_init (parser, string, length);

    while (!iter_eof (&parser->current))
    {
        parser->cmd = 0;

        switch (iter_get_char (&parser->current))
        {
            case '\033':
                if (!parser_init_yyparse (parser) || _moo_term_yyparse (parser))
                {
                    if (parser->eof_error)
                    {
                        parser->tosave = parser->cmd_string.start;
                    }
                    else
                    {
                        char *esc_seq = block_get_string (&parser->cmd_string);
                        g_warning ("invalid escape sequence");
                        _moo_term_print_bytes (esc_seq, -1);
                        g_print ("\n");
                        g_free (esc_seq);
                        chars_add_cmd (parser);
                    }
                }
                else
                {
                    /* no errors */
                    g_assert (parser->cmd);
                }

                break;

            case '\007':
                parser->cmd = CMD_BELL;
                iter_forward (&parser->current);
                break;

            case '\015':
                parser->cmd = CMD_CARRIAGE_RETURN;
                iter_forward (&parser->current);
                break;

            case '\010':
                parser->cmd = CMD_CURSOR_LEFT;
                iter_forward (&parser->current);
                break;
                
            case '\012':
                parser->cmd = CMD_CURSOR_DOWN;
                iter_forward (&parser->current);
                break;
                
            case '\011':
                parser->cmd = CMD_TAB;
                iter_forward (&parser->current);
                break;
                
            case '\017':
                parser->cmd = CMD_EXIT_ALT_CHARSET_MODE;
                iter_forward (&parser->current);
                break;

            case '\016':
                parser->cmd = CMD_ENTER_ALT_CHARSET_MODE;
                iter_forward (&parser->current);
                break;

            case 0:
                chars_flush (parser);

            default:
                chars_add_one (parser);
        }

        if (parser->cmd)
        {
            chars_flush (parser);
            exec_command (parser);
        }
        else
        {
            /* exec_command() or yylex() should call iter_next() */
            iter_forward (&parser->current);
        }
    }

    chars_flush (parser);
    parser_deinit (parser);
}


int             _moo_term_yylex         (MooTermParser  *parser)
{
    while (!iter_eof (&parser->current))
    {
        char c = iter_get_char (&parser->current);

        iter_forward (&parser->current);
        iter_forward (&parser->cmd_string.end);

        /* skip NUL chars*/
        if (c)
            return c;
    }

    parser->eof_error = TRUE;
    return 0;
}


static gboolean parser_init_yyparse     (MooTermParser  *parser)
{
    g_assert (iter_get_char (&parser->current) == '\033');

    parser->cmd_string.start = parser->cmd_string.end = parser->current;
    iter_forward (&parser->cmd_string.end);

    parser->nums_len = 0;
    parser->eof_error = FALSE;

    iter_forward (&parser->current);

    if (iter_eof (&parser->current))
    {
        parser->eof_error = TRUE;
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}


static void     parser_init             (MooTermParser  *parser,
                                         const char     *string,
                                         guint           len)
{
    parser->data = string;
    parser->data_len = len;

    if (parser->old_data_len)
    {
        parser->current.old = TRUE;
        parser->current.offset = 0;
    }
    else
    {
        parser->current.old = FALSE;
        parser->current.offset = 0;
    }

    iter_set_eof (&parser->tosave);
    block_set_empty (&parser->chars);
    block_set_empty (&parser->cmd_string);

    parser->cmd = 0;
    parser->nums_len = 0;
}


static void     parser_deinit           (MooTermParser  *parser)
{
    if (!iter_eof (&parser->tosave))
    {
        guint len;

        if (parser->tosave.old)
            len = parser->data_len + parser->old_data_len - parser->tosave.offset;
        else
            len = parser->data_len - parser->tosave.offset;

        g_assert (len != 0);

        if (len > MAX_ESC_SEQ_LEN)
        {
            g_warning ("%s: too much data, discarding", G_STRLOC);
            parser->old_data_len = 0;
            return;
        }

        if (parser->tosave.old)
        {
            guint l1 = parser->old_data_len - parser->tosave.offset;

            if (len < l1)
                l1 = len;

            memmove (parser->old_data,
                     parser->old_data + parser->tosave.offset, l1);

            if (len > l1)
                memcpy (parser->old_data + l1, parser->data, len - l1);
        }
        else
        {
            memcpy (parser->old_data, parser->data + parser->tosave.offset, len);
        }

        parser->old_data_len = len;
    }
    else
    {
        parser->old_data_len = 0;
    }
}


void            _moo_term_yyerror       (G_GNUC_UNUSED MooTermParser  *parser,
                                         G_GNUC_UNUSED const char     *string)
{
}


MooTermParser  *moo_term_parser_new     (MooTermBuffer  *buf)
{
    MooTermParser *p = g_new0 (MooTermParser, 1);

    p->term_buffer = buf;
    
    p->tosave.parser = p;
    p->current.parser = p;
    p->chars.start.parser = p;
    p->chars.end.parser = p;
    p->cmd_string.start.parser = p;
    p->cmd_string.end.parser = p;

    return p;
}


void            moo_term_parser_free    (MooTermParser  *parser)
{
    if (parser)
    {
        parser->term_buffer = NULL;
        g_free (parser);
    }
}


static void set_attributes      (MooTermBuffer  *buf,
                                 guint          *attrs,
                                 guint           attrs_len)
{
    guint i;

    if (!attrs_len)
    {
        buf_set_attrs_mask (buf, 0);
        buf_set_secure_mode (buf, FALSE);
        return;
    }

    for (i = 0; i < attrs_len; ++i)
    {
        switch (attrs[i])
        {
            case ANSI_ALL_ATTRS_OFF:
                buf_set_attrs_mask (buf, 0);
                buf_set_secure_mode (buf, FALSE);
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
            case ANSI_CONCEALED:
                buf_set_secure_mode (buf, TRUE);
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
            case 20 + ANSI_CONCEALED:
                buf_set_secure_mode (buf, FALSE);
                break;

            default:
                if (30 <= attrs[i] && attrs[i] <= 37)
                    buf_set_ansi_foreground (buf, attrs[i] - 30);
                else if (40 <= attrs[i] && attrs[i] <= 47)
                    buf_set_ansi_background (buf, attrs[i] - 40);
                else if (39 == attrs[i])
                    buf_set_ansi_foreground (buf, 8);
                else if (49 == attrs[i])
                    buf_set_ansi_background (buf, 8);
                else
                    g_warning ("%s: unknown text attribute %d",
                               G_STRLOC, attrs[i]);
        }
    }
}


static void     exec_command            (MooTermParser  *parser)
{
    switch (parser->cmd)
    {
        case CMD_SET_ATTRS:
            if (parser->nums_len)
                set_attributes (parser->term_buffer,
                                parser->nums,
                                parser->nums_len);
            else
                set_attributes (parser->term_buffer,
                                NULL, 0);
            break;

        case CMD_PARM_LEFT_CURSOR:
            buf_cursor_move (parser->term_buffer, 0, -parser->nums[0]);
            break;
        case CMD_PARM_RIGHT_CURSOR:
            buf_cursor_move (parser->term_buffer, 0, parser->nums[0]);
            break;
        case CMD_PARM_DOWN_CURSOR:
            buf_parm_down_cursor (parser->term_buffer, parser->nums[0]);
            break;
        case CMD_PARM_UP_CURSOR:
            buf_cursor_move (parser->term_buffer, -parser->nums[0], 0);
            break;
        case CMD_ROW_ADDRESS:
            buf_cursor_move_to (parser->term_buffer, parser->nums[0] - 1, -1);
            break;
        case CMD_COLUMN_ADDRESS:
            buf_cursor_move_to (parser->term_buffer, -1, parser->nums[0] - 1);
            break;
        case CMD_PARM_DCH:
            buf_parm_dch (parser->term_buffer, parser->nums[0]);
            break;
        case CMD_PARM_DELETE_LINE:
            buf_parm_delete_line (parser->term_buffer, parser->nums[0]);
            break;
        case CMD_ERASE_CHARS:
            buf_erase_chars (parser->term_buffer, parser->nums[0]);
            break;
        case CMD_PARM_ICH:
            buf_parm_ich (parser->term_buffer, parser->nums[0]);
            break;
        case CMD_PARM_INSERT_LINE:
            buf_parm_insert_line (parser->term_buffer, parser->nums[0]);
            break;
        case CMD_SET_BACKGROUND:
            buf_set_background (parser->term_buffer, parser->nums[0]);
            break;
        case CMD_SET_FOREGROUND:
            buf_set_foreground (parser->term_buffer, parser->nums[0]);
            break;

        case CMD_CHANGE_SCROLL_REGION:
            buf_change_scroll_region (parser->term_buffer,
                                      parser->nums[0] - 1,
                                      parser->nums[1] - 1);
            break;
        case CMD_CURSOR_ADDRESS:
            buf_cursor_move_to (parser->term_buffer,
                                parser->nums[0] - 1,
                                parser->nums[1] - 1);
            break;
        case CMD_USER6:
            buf_user6 (parser->term_buffer,
                       parser->nums[0] - 1,
                       parser->nums[1] - 1);
            break;

        case CMD_BACK_TAB:
            buf_back_tab (parser->term_buffer);
            break;
        case CMD_CURSOR_NORMAL:
            buf_cursor_normal (parser->term_buffer);
            break;
        case CMD_CURSOR_INVISIBLE:
            buf_cursor_invisible (parser->term_buffer);
            break;
        case CMD_CLEAR_SCREEN:
            buf_clear_screen (parser->term_buffer);
            break;
        case CMD_CURSOR_RIGHT:
            buf_cursor_move (parser->term_buffer, 0, 1);
            break;
        case CMD_CURSOR_LEFT:
            buf_cursor_move (parser->term_buffer, 0, -1);
            break;
        case CMD_CURSOR_DOWN:
            buf_cursor_down (parser->term_buffer);
            break;
        case CMD_CURSOR_UP:
            buf_cursor_move (parser->term_buffer, -1, 0);
            break;
        case CMD_CURSOR_HOME:
            buf_cursor_home (parser->term_buffer);
            break;
        case CMD_CARRIAGE_RETURN:
            buf_carriage_return (parser->term_buffer);
            break;
        case CMD_DELETE_CHARACTER:
            buf_delete_character (parser->term_buffer);
            break;
        case CMD_DELETE_LINE:
            buf_delete_line (parser->term_buffer);
            break;
        case CMD_CLR_EOS:
            buf_clr_eos (parser->term_buffer);
            break;
        case CMD_CLR_EOL:
            buf_clr_eol (parser->term_buffer);
            break;
        case CMD_CLR_BOL:
            buf_clr_bol (parser->term_buffer);
            break;
        case CMD_ENA_ACS:
            buf_ena_acs (parser->term_buffer);
            break;
        case CMD_FLASH_SCREEN:
            buf_flash_screen (parser->term_buffer);
            break;
        case CMD_INSERT_LINE:
            buf_insert_line (parser->term_buffer);
            break;
        case CMD_ENTER_SECURE_MODE:
            buf_enter_secure_mode (parser->term_buffer);
            break;
        case CMD_INIT_2STRING:
            buf_init_2string (parser->term_buffer);
            break;
        case CMD_RESET_2STRING:
            buf_reset_2string (parser->term_buffer);
            break;
        case CMD_RESET_1STRING:
            buf_reset_1string (parser->term_buffer);
            break;
        case CMD_PRINT_SCREEN:
            buf_print_screen (parser->term_buffer);
            break;
        case CMD_PRTR_OFF:
            buf_prtr_off (parser->term_buffer);
            break;
        case CMD_PRTR_ON:
            buf_prtr_on (parser->term_buffer);
            break;
        case CMD_ORIG_PAIR:
            buf_orig_pair (parser->term_buffer);
            break;
        case CMD_ENTER_REVERSE_MODE:
            buf_enter_reverse_mode (parser->term_buffer);
            break;
        case CMD_ENTER_AM_MODE:
            buf_enter_am_mode (parser->term_buffer);
            break;
        case CMD_EXIT_AM_MODE:
            buf_exit_am_mode (parser->term_buffer);
            break;
        case CMD_ENTER_CA_MODE:
            buf_enter_ca_mode (parser->term_buffer);
            break;
        case CMD_EXIT_CA_MODE:
            buf_exit_ca_mode (parser->term_buffer);
            break;
        case CMD_ENTER_INSERT_MODE:
            buf_enter_insert_mode (parser->term_buffer);
            break;
        case CMD_EXIT_INSERT_MODE:
            buf_exit_insert_mode (parser->term_buffer);
            break;
        case CMD_ENTER_STANDOUT_MODE:
            buf_enter_reverse_mode (parser->term_buffer);
            break;
        case CMD_EXIT_STANDOUT_MODE:
            buf_exit_reverse_mode (parser->term_buffer);
            break;
        case CMD_ENTER_UNDERLINE_MODE:
            buf_enter_underline_mode (parser->term_buffer);
            break;
        case CMD_EXIT_UNDERLINE_MODE:
            buf_exit_underline_mode (parser->term_buffer);
            break;
        case CMD_KEYPAD_LOCAL:
            buf_keypad_local (parser->term_buffer);
            break;
        case CMD_KEYPAD_XMIT:
            buf_keypad_xmit (parser->term_buffer);
            break;
        case CMD_CLEAR_ALL_TABS:
            buf_clear_all_tabs (parser->term_buffer);
            break;
        case CMD_USER7:
            buf_user7 (parser->term_buffer);
            break;
        case CMD_USER8:
            buf_user8 (parser->term_buffer);
            break;
        case CMD_USER9:
            buf_user9 (parser->term_buffer);
            break;
        case CMD_BELL:
            buf_bell (parser->term_buffer);
            break;
        case CMD_ENTER_ALT_CHARSET_MODE:
            buf_enter_alt_charset_mode (parser->term_buffer);
            break;
        case CMD_EXIT_ALT_CHARSET_MODE:
            buf_exit_alt_charset_mode (parser->term_buffer);
            break;
        case CMD_TAB:
            buf_tab (parser->term_buffer);
            break;
        case CMD_SCROLL_FORWARD:
            buf_scroll_forward (parser->term_buffer);
            break;
        case CMD_SCROLL_REVERSE:
            buf_scroll_reverse (parser->term_buffer);
            break;
        case CMD_SET_TAB:
            buf_set_tab (parser->term_buffer);
            break;
        case CMD_ESC_l:
            buf_esc_l (parser->term_buffer);
            break;
        case CMD_ESC_m:
            buf_esc_m (parser->term_buffer);
            break;
        case CMD_RESTORE_CURSOR:
            buf_restore_cursor (parser->term_buffer);
            break;
        case CMD_SAVE_CURSOR:
            buf_save_cursor (parser->term_buffer);
            break;

        case CMD_SET_WINDOW_TITLE:
        case CMD_SET_ICON_NAME:
        case CMD_SET_WINDOW_ICON_NAME:
        {
            gboolean set_title = FALSE, set_icon = FALSE;
            char *title;
            Iter iter;

            /* ^E]0;<string>BEL - 4 chars + string + 1 char */

            for (iter = parser->current;
                 !iter_eof (&iter) && iter_get_char (&iter) != '\007';
                 iter_forward (&iter)) ;

            if (iter_eof (&iter))
            {
                parser->tosave = parser->cmd_string.start;
                parser->current = iter;
                return;
            }
            else
            {
                title = iter_get_range (&parser->current, &iter);
                iter_forward (&iter);
                parser->current = iter;
            }

            if (parser->cmd == CMD_SET_WINDOW_TITLE)
            {
                set_title = TRUE;
            }
            else if (parser->cmd == CMD_SET_ICON_NAME)
            {
                set_icon = TRUE;
            }
            else
            {
                set_title = TRUE;
                set_icon = TRUE;
            }

            if (set_title)
                moo_term_buffer_set_window_title (parser->term_buffer, title);
            if (set_icon)
                moo_term_buffer_set_icon_name (parser->term_buffer, title);

            g_free (title);
        }
        break;

        case CMD_NONE:
            g_assert_not_reached ();
            break;
    }
}


void            _moo_term_print_bytes   (const char     *string,
                                         int             len)
{
    int i;

    if (len < 0)
        len = strlen (string);

    for (i = 0; i < len; ++i)
    {
        if (' ' <= string[i] && string[i] <= '~')
            g_print ("%c", string[i]);
        else if ('A' - 64 <= string[i] && string[i] <= 'Z' - 64)
            g_print ("^%c", string[i] + 64);
        else
            g_print ("<%d>", string[i]);
    }
}
