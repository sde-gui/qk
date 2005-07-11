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

#include "mooterm/mooterm-private.h"
#include "mooterm/mootermbuffer-private.h"

G_BEGIN_DECLS


#define vt_print_char(ch) moo_term_buffer_print_unichar (parser->term->priv->buffer, ch)


#define vt_BEL()    moo_term_bell (parser->term)
#define vt_BS()     moo_term_buffer_backspace (parser->term->priv->buffer)
#define vt_TAB()    moo_term_buffer_tab (parser->term->priv->buffer)
#define vt_LF()     moo_term_buffer_linefeed (parser->term->priv->buffer)
#define vt_CR()     moo_term_buffer_carriage_return (parser->term->priv->buffer)
#define vt_SO()     moo_term_buffer_shift (parser->term->priv->buffer, 0)
#define vt_SI()     moo_term_buffer_shift (parser->term->priv->buffer, 1)
#define vt_XON()    g_warning ("%s: got XON", G_STRLOC)
#define vt_XOFF()   g_warning ("%s: got XOFF", G_STRLOC)
#define vt_IND()    moo_term_buffer_index (parser->term->priv->buffer)
#define vt_NEL()    moo_term_buffer_new_line (parser->term->priv->buffer)
#define vt_HTS()    moo_term_buffer_set_tab_stop (parser->term->priv->buffer)
#define vt_RI()     moo_term_buffer_reverse_index (parser->term->priv->buffer)
#define vt_SS2()    moo_term_buffer_single_shift (parser->term->priv->buffer, 2)
#define vt_SS3()    moo_term_buffer_single_shift (parser->term->priv->buffer, 3)
#define vt_DECID()  moo_term_decid (parser->term)
#define vt_SGR()    moo_term_buffer_sgr (parser->term->priv->buffer,            \
                                         (int*) parser->numbers->data,          \
                                         parser->numbers->len)
#define vt_CUU(n)   moo_term_buffer_cursor_move (parser->term->priv->buffer, -n, 0)
#define vt_CUD(n)   moo_term_buffer_cursor_move (parser->term->priv->buffer, n, 0)
#define vt_CUF(n)   moo_term_buffer_cursor_move (parser->term->priv->buffer, 0, n)
#define vt_CUB(n)   moo_term_buffer_cursor_move (parser->term->priv->buffer, 0, -n)
#define vt_DCH(n)   moo_term_buffer_delete_char (parser->term->priv->buffer, n)
#define vt_DL(n)    /* moo_term_buffer_delete_line (parser->term->priv->buffer, n) */
#define vt_ECH(n)   moo_term_buffer_erase_char (parser->term->priv->buffer, n)
#define vt_ED(n)    moo_term_buffer_erase_in_display (parser->term->priv->buffer, n)
#define vt_EL(n)    moo_term_buffer_erase_in_line (parser->term->priv->buffer, n)
#define vt_ICH(n)   moo_term_buffer_insert_char (parser->term->priv->buffer, n)
#define vt_IL(n)    /* moo_term_buffer_insert_line (parser->term->priv->buffer, n) */


G_END_DECLS

#endif /* MOOTERM_MOOTERM_CTLFUNCS_H */
