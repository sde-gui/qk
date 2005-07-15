/*
 *   mooterm/mooterm-ctlfuncs.h
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

#ifndef MOOTERM_MOOTERM_CTLFUNCS_H
#define MOOTERM_MOOTERM_CTLFUNCS_H

#include "mooterm/mootermbuffer-private.h"


#if 1
#define vt_not_implemented()                                \
{                                                           \
    char *s = _moo_term_current_ctl (parser);               \
    term_implement_me_warning ("'%s': implement me", s);    \
    g_free (s);                                             \
}
#define vt_ignored()                                        \
{                                                           \
    char *s = _moo_term_current_ctl (parser);               \
    g_warning ("'%s' ignored", s);                          \
    g_free (s);                                             \
}
#else
#define vt_not_implemented()
#define vt_ignored()
#endif


#define vt_print_char(ch) moo_term_buffer_print_unichar (parser->term->priv->buffer, ch)

#define vt_XON()        g_warning ("%s: got XON", G_STRLOC)
#define vt_XOFF()       g_warning ("%s: got XOFF", G_STRLOC)

#define vt_SET_ICON_NAME(s)     moo_term_set_icon_name (parser->term, s)
#define vt_SET_WINDOW_TITLE(s)  moo_term_set_window_title (parser->term, s)

#define vt_BEL()        moo_term_bell (parser->term)
#define vt_BS()         moo_term_buffer_backspace (parser->term->priv->buffer)
#define vt_TAB()        moo_term_buffer_tab (parser->term->priv->buffer, 1)
#define vt_LF()         moo_term_buffer_linefeed (parser->term->priv->buffer)
#define vt_CR()         moo_term_buffer_carriage_return (parser->term->priv->buffer)
#define vt_SO()         moo_term_buffer_shift (parser->term->priv->buffer, 1)
#define vt_SI()         moo_term_buffer_shift (parser->term->priv->buffer, 0)
#define vt_IND()        moo_term_buffer_index (parser->term->priv->buffer)
#define vt_NEL()        moo_term_buffer_new_line (parser->term->priv->buffer)
#define vt_HTS()        moo_term_buffer_set_tab_stop (parser->term->priv->buffer)
#define vt_TBC(w)       moo_term_buffer_clear_tab_stop (parser->term->priv->buffer, w)
#define vt_RI()         moo_term_buffer_reverse_index (parser->term->priv->buffer)
#define vt_SS2()        moo_term_buffer_single_shift (parser->term->priv->buffer, 2)
#define vt_SS3()        moo_term_buffer_single_shift (parser->term->priv->buffer, 3)
#define vt_DECID()      moo_term_decid (parser->term)
#define vt_SGR()        moo_term_buffer_sgr (parser->term->priv->buffer,    \
                                             (int*) parser->numbers->data,  \
                                             parser->numbers->len)
#define vt_CUU(n)       moo_term_buffer_cuu (parser->term->priv->buffer, n)
#define vt_CUD(n)       moo_term_buffer_cud (parser->term->priv->buffer, n)
#define vt_CUF(n)       moo_term_buffer_cursor_move (parser->term->priv->buffer, 0, n)
#define vt_CUB(n)       moo_term_buffer_cursor_move (parser->term->priv->buffer, 0, -n)
#define vt_DCH(n)       moo_term_buffer_delete_char (parser->term->priv->buffer, n)
#define vt_DL(n)        moo_term_buffer_delete_line (parser->term->priv->buffer, n)
#define vt_ECH(n)       moo_term_buffer_erase_char (parser->term->priv->buffer, n)
#define vt_ED(n)        moo_term_buffer_erase_in_display (parser->term->priv->buffer, n)
#define vt_EL(n)        moo_term_buffer_erase_in_line (parser->term->priv->buffer, n)
#define vt_ICH(n)       moo_term_buffer_insert_char (parser->term->priv->buffer, n)
#define vt_IL(n)        moo_term_buffer_insert_line (parser->term->priv->buffer, n)
#define vt_CUP(r,c)     moo_term_buffer_cup (parser->term->priv->buffer, (r)-1, (c)-1)
#define vt_DECSTBM(t,b) moo_term_buffer_set_scrolling_region (parser->term->priv->buffer, (t)-1, (b)-1);
#define vt_DECSC()      moo_term_buffer_decsc (parser->term->priv->buffer)
#define vt_DECRC()      moo_term_buffer_decrc (parser->term->priv->buffer)
#define vt_DECSET()     moo_term_set_dec_modes  (parser->term,                  \
                                                 (int*) parser->numbers->data,  \
                                                 parser->numbers->len,          \
                                                 TRUE)
#define vt_DECRST()     moo_term_set_dec_modes  (parser->term,                  \
                                                 (int*) parser->numbers->data,  \
                                                 parser->numbers->len,          \
                                                 FALSE)
#define vt_SM()         moo_term_set_ansi_modes (parser->term,                  \
                                                 (int*) parser->numbers->data,  \
                                                 parser->numbers->len,          \
                                                 TRUE)
#define vt_RM()         moo_term_set_ansi_modes (parser->term,                  \
                                                 (int*) parser->numbers->data,  \
                                                 parser->numbers->len,          \
                                                 FALSE)
#define vt_SCS(n,s)     moo_term_buffer_select_charset (parser->term->priv->buffer, n, s)
#define vt_DECKPAM()    moo_term_set_mode (parser->term, MODE_DECNKM, TRUE)
#define vt_DECKPNM()    moo_term_set_mode (parser->term, MODE_DECNKM, FALSE)
#define vt_DECSAVE()    moo_term_save_dec_modes     (parser->term,                  \
                                                     (int*) parser->numbers->data,  \
                                                     parser->numbers->len)
#define vt_DECRESTORE() moo_term_restore_dec_modes  (parser->term,                  \
                                                     (int*) parser->numbers->data,  \
                                                     parser->numbers->len)
#define vt_DA1()        moo_term_da1 (parser->term)
#define vt_DA2()        moo_term_da2 (parser->term)
#define vt_DA3()        moo_term_da3 (parser->term)
#define vt_DECRQSS(s)   moo_term_setting_request (parser->term, s)
#define vt_DSR(t,a,e)   moo_term_dsr (parser->term, t, a, e)
#define vt_DECSTR()     moo_term_soft_reset (parser->term)
#define vt_RIS()        moo_term_reset (parser->term)

#define vt_CBT(n)       moo_term_buffer_back_tab (parser->term->priv->buffer, n)
#define vt_CHA(n)       moo_term_buffer_cursor_move_to (parser->term->priv->buffer, -1, n)
#define vt_CHT(n)       moo_term_buffer_tab (parser->term->priv->buffer, n)
#define vt_CNL(n)       moo_term_buffer_cursor_next_line (parser->term->priv->buffer, n)
#define vt_CPL(n)       moo_term_buffer_cursor_prev_line (parser->term->priv->buffer, n)
#define vt_HPA(n)       moo_term_buffer_cursor_move_to (parser->term->priv->buffer, -1, n)
#define vt_HPR(n)       moo_term_buffer_cursor_move (parser->term->priv->buffer, 0, n)
#define vt_VPA(n)       moo_term_buffer_cursor_move_to (parser->term->priv->buffer, n, -1)
#define vt_VPR(n)       moo_term_buffer_cursor_move (parser->term->priv->buffer, n, 0)
#define vt_DECALN()     moo_term_buffer_decaln (parser->term->priv->buffer)


#endif /* MOOTERM_MOOTERM_CTLFUNCS_H */
