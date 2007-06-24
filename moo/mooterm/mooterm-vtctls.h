/*
 *   mooterm/mooterm-ctlfuncs.h
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

#ifndef MOOTERM_MOOTERM_CTLFUNCS_H
#define MOOTERM_MOOTERM_CTLFUNCS_H

#include "mooterm/mootermbuffer-private.h"


#if 1
#define VT_NOT_IMPLEMENTED                                  \
G_STMT_START {                                              \
    char *s = _moo_term_current_ctl (parser);               \
    TERM_IMPLEMENT_ME_WARNING ("'%s': implement me", s);    \
    g_free (s);                                             \
} G_STMT_END
#define VT_IGNORED                                          \
G_STMT_START {                                              \
    char *s = _moo_term_current_ctl (parser);               \
    g_warning ("'%s' ignored", s);                          \
    g_free (s);                                             \
} G_STMT_END
#else
#define VT_NOT_IMPLEMENTED  G_STMT_START {} G_STMT_END
#define VT_IGNORED          G_STMT_START {} G_STMT_END
#endif


#define VT_PRINT_CHAR(ch) moo_term_buffer_print_unichar (parser->term->priv->buffer, ch)

#define VT_XON          g_warning ("%s: got XON", G_STRLOC)
#define VT_XOFF         g_warning ("%s: got XOFF", G_STRLOC)

#define vt_SET_ICON_NAME(s)     _moo_term_set_icon_name (parser->term, s)
#define vt_SET_WINDOW_TITLE(s)  _moo_term_set_window_title (parser->term, s)

#define VT_BEL          _moo_term_bell (parser->term)
#define VT_BS           _moo_term_buffer_backspace (parser->term->priv->buffer)
#define VT_TAB          _moo_term_buffer_tab (parser->term->priv->buffer, 1)
#define VT_LF           _moo_term_buffer_linefeed (parser->term->priv->buffer)
#define VT_CR           _moo_term_buffer_carriage_return (parser->term->priv->buffer)
#define VT_SO           _moo_term_buffer_shift (parser->term->priv->buffer, 1)
#define VT_SI           _moo_term_buffer_shift (parser->term->priv->buffer, 0)
#define VT_IND          _moo_term_buffer_index (parser->term->priv->buffer)
#define VT_NEL          _moo_term_buffer_new_line (parser->term->priv->buffer)
#define VT_HTS          _moo_term_buffer_set_tab_stop (parser->term->priv->buffer)
#define VT_TBC(w)       _moo_term_buffer_clear_tab_stop (parser->term->priv->buffer, w)
#define VT_RI           _moo_term_buffer_reverse_index (parser->term->priv->buffer)
#define VT_SS2          _moo_term_buffer_single_shift (parser->term->priv->buffer, 2)
#define VT_SS3          _moo_term_buffer_single_shift (parser->term->priv->buffer, 3)
#define VT_DECID        _moo_term_decid (parser->term)
#define VT_SGR          _moo_term_buffer_sgr(parser->term->priv->buffer,    \
                                             (int*) parser->numbers->data,  \
                                             parser->numbers->len)
#define VT_CUU(n)       _moo_term_buffer_cuu (parser->term->priv->buffer, n)
#define VT_CUD(n)       _moo_term_buffer_cud (parser->term->priv->buffer, n)
#define VT_CUF(n)       _moo_term_buffer_cursor_move (parser->term->priv->buffer, 0, n)
#define VT_CUB(n)       _moo_term_buffer_cursor_move (parser->term->priv->buffer, 0, -n)
#define VT_DCH(n)       _moo_term_buffer_delete_char (parser->term->priv->buffer, n)
#define VT_DL(n)        _moo_term_buffer_delete_line (parser->term->priv->buffer, n)
#define VT_ECH(n)       _moo_term_buffer_erase_char (parser->term->priv->buffer, n)
#define VT_ED(n)        _moo_term_buffer_erase_in_display (parser->term->priv->buffer, n)
#define VT_EL(n)        _moo_term_buffer_erase_in_line (parser->term->priv->buffer, n)
#define VT_ICH(n)       _moo_term_buffer_insert_char (parser->term->priv->buffer, n)
#define VT_IL(n)        _moo_term_buffer_insert_line (parser->term->priv->buffer, n)
#define VT_CUP(r,c)     _moo_term_buffer_cup (parser->term->priv->buffer, (r)-1, (c)-1)
#define VT_DECSTBM(t,b) _moo_term_buffer_set_scrolling_region (parser->term->priv->buffer, (t)-1, (b)-1);
#define VT_DECSC        _moo_term_decsc (parser->term)
#define VT_DECRC        _moo_term_decrc (parser->term)
#define VT_DECSET       _moo_term_set_dec_modes (parser->term,                  \
                                                 (int*) parser->numbers->data,  \
                                                 parser->numbers->len,          \
                                                 TRUE)
#define VT_DECRST       _moo_term_set_dec_modes (parser->term,                  \
                                                 (int*) parser->numbers->data,  \
                                                 parser->numbers->len,          \
                                                 FALSE)
#define VT_SM           _moo_term_set_ansi_modes(parser->term,                  \
                                                 (int*) parser->numbers->data,  \
                                                 parser->numbers->len,          \
                                                 TRUE)
#define VT_RM           _moo_term_set_ansi_modes(parser->term,                  \
                                                 (int*) parser->numbers->data,  \
                                                 parser->numbers->len,          \
                                                 FALSE)
#define VT_SCS(n,s)     _moo_term_buffer_select_charset (parser->term->priv->buffer, n, s)
#define VT_DECKPAM      _moo_term_set_mode (parser->term, MODE_DECNKM, TRUE)
#define VT_DECKPNM      _moo_term_set_mode (parser->term, MODE_DECNKM, FALSE)
#define VT_DECSAVE      _moo_term_save_dec_modes    (parser->term,                  \
                                                     (int*) parser->numbers->data,  \
                                                     parser->numbers->len)
#define VT_DECRESTORE   _moo_term_restore_dec_modes (parser->term,                  \
                                                     (int*) parser->numbers->data,  \
                                                     parser->numbers->len)
#define VT_DA1          _moo_term_da1 (parser->term)
#define VT_DA2          _moo_term_da2 (parser->term)
#define VT_DA3          _moo_term_da3 (parser->term)
#define VT_DECRQSS(s)   _moo_term_setting_request (parser->term, s)
#define VT_DSR(t,a,e)   _moo_term_dsr (parser->term, t, a, e)
#define VT_DECSTR       moo_term_soft_reset (parser->term)
#define VT_RIS          moo_term_reset (parser->term)

#define VT_CBT(n)       _moo_term_buffer_back_tab (parser->term->priv->buffer, n)
#define VT_CHA(n)       _moo_term_buffer_cursor_move_to (parser->term->priv->buffer, -1, (n)-1) /* XXX is it right? */
#define VT_CHT(n)       _moo_term_buffer_tab (parser->term->priv->buffer, n)
#define VT_CNL(n)       _moo_term_buffer_cursor_next_line (parser->term->priv->buffer, n)
#define VT_CPL(n)       _moo_term_buffer_cursor_prev_line (parser->term->priv->buffer, n)
#define VT_HPA(n)       _moo_term_buffer_cursor_move_to (parser->term->priv->buffer, -1, (n)-1) /* XXX is it right? */
#define VT_HPR(n)       _moo_term_buffer_cursor_move (parser->term->priv->buffer, 0, n)
#define VT_VPA(n)       _moo_term_buffer_cursor_move_to (parser->term->priv->buffer, (n)-1, -1) /* XXX is it right? */
#define VT_VPR(n)       _moo_term_buffer_cursor_move (parser->term->priv->buffer, n, 0)
#define VT_DECALN       _moo_term_buffer_decaln (parser->term->priv->buffer)


#endif /* MOOTERM_MOOTERM_CTLFUNCS_H */
