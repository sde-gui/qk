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
#include "mooterm/mootermbuffer-vt.h"
#include "mooterm/mootermparser-yacc.h"
#include "mooterm/mootermparser.h"
#include <string.h>


#define INVALID_CHAR        "?"
#define INVALID_CHAR_LEN    1


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


inline static void _flush (MooTermBuffer *buf, const char *string, guint len)
{
    const char *p = string;
    const char *end = string + len;
    const char *s;

    g_assert (len != 0);

    while (p != end)
    {
        for (s = p; s != end && *s && !(*s & 0x80); ++s) ;

        if (s > p)
        {
            moo_term_buffer_print_chars (buf, p, s - p);
            p = s;
        }
        else
        {
            if (*p++ & 0x80)
                moo_term_buffer_print_chars (buf, INVALID_CHAR, INVALID_CHAR_LEN);
        }
    }
}

inline static void      chars_flush         (MooTermParser  *parser)
{
    if (!block_is_empty (&parser->chars))
    {
#if 0
        char *bytes, *nice_bytes;
        bytes = block_get_string (&parser->chars);
        nice_bytes = _moo_term_nice_bytes (bytes, -1);
        g_print ("chars '%s'\n", nice_bytes);
        g_free (nice_bytes);
        g_free (bytes);
#endif

        if (parser->chars.start.old)
        {
            if (parser->chars.end.old)
            {
                g_assert (parser->chars.start.offset < parser->chars.end.offset);

                _flush (parser->term_buffer,
                        parser->old_data + parser->chars.start.offset,
                        parser->chars.end.offset - parser->chars.start.offset);
            }
            else
            {
                _flush (parser->term_buffer,
                        parser->old_data + parser->chars.start.offset,
                        parser->old_data_len - parser->chars.start.offset);
                _flush (parser->term_buffer,
                        parser->data,
                        parser->chars.end.offset);
            }
        }
        else
        {
            g_assert (!parser->chars.end.old);
            g_assert (parser->chars.start.offset < parser->chars.end.offset);

            _flush (parser->term_buffer,
                    parser->data + parser->chars.start.offset,
                    parser->chars.end.offset - parser->chars.start.offset);
        }

        block_set_empty (&parser->chars);
    }
}


#define set_one_char_cmd(num)                   \
{                                               \
    parser->cmd = num;                          \
    parser->cmd_string.start = parser->current; \
    iter_forward (&parser->current);            \
    parser->cmd_string.end = parser->current;   \
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
                        char *nice = _moo_term_nice_bytes (esc_seq, -1);
                        g_warning ("%s: invalid escape sequence '%s'", G_STRLOC, nice);
                        g_free (esc_seq);
                        g_free (nice);
                    }

                    parser->cmd = CMD_ERROR;
                }
                else
                {
                    /* no errors */
                    g_assert (parser->cmd);
                }

                break;

            case '\007':
                set_one_char_cmd (CMD_BELL);
                break;
            case '\010':
                set_one_char_cmd (CMD_BACKSPACE);
                break;
            case '\011':
                set_one_char_cmd (CMD_TAB);
                break;
            case '\012':
                set_one_char_cmd (CMD_LINEFEED);
                break;
            case '\013':
                set_one_char_cmd (CMD_VERT_TAB);
                break;
            case '\014':
                set_one_char_cmd (CMD_FORM_FEED);
                break;
            case '\015':
                set_one_char_cmd (CMD_CARRIAGE_RETURN);
                break;
            case '\016':
                set_one_char_cmd (CMD_ALT_CHARSET);
                break;
            case '\017':
                set_one_char_cmd (CMD_NORM_CHARSET);
                break;

            case 0:
                chars_flush (parser);

            default:
                chars_add_one (parser);
        }

        if (parser->cmd)
        {
            chars_flush (parser);
            if (parser->cmd != CMD_ERROR)
            {
                exec_command (parser);
            }
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
        {
            if ('0' <= c && c <= '9')
            {
                _moo_term_yylval = c - '0';
                return DIGIT;
            }
            else
            {
                _moo_term_yylval = c;
                return c;
            }
        }
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
    parser->string_len = 0;
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


inline static void buf_set_attrs_mask       (MooTermBuffer  *buf,
                                             MooTermTextAttrMask mask)
{
    buf->priv->current_attr.mask = mask;
}

inline static void buf_add_attrs_mask       (MooTermBuffer  *buf,
                                             MooTermTextAttrMask mask)
{
    buf->priv->current_attr.mask |= mask;
}

inline static void buf_remove_attrs_mask    (MooTermBuffer      *buf,
                                             MooTermTextAttrMask mask)
{
    buf->priv->current_attr.mask &= ~mask;
}

inline static void buf_set_ansi_foreground  (MooTermBuffer      *buf,
                                             MooTermBufferColor  color)
{
    if (color < MOO_TERM_COLOR_MAX)
    {
        buf->priv->current_attr.mask |= MOO_TERM_TEXT_FOREGROUND;
        buf->priv->current_attr.foreground = color;
    }
    else
    {
        buf->priv->current_attr.mask &= ~MOO_TERM_TEXT_FOREGROUND;
    }
}

inline static void buf_set_ansi_background  (MooTermBuffer      *buf,
                                             MooTermBufferColor  color)
{
    if (color < MOO_TERM_COLOR_MAX)
    {
        buf->priv->current_attr.mask |= MOO_TERM_TEXT_BACKGROUND;
        buf->priv->current_attr.background = color;
    }
    else
    {
        buf->priv->current_attr.mask &= ~MOO_TERM_TEXT_BACKGROUND;
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
