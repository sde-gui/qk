/*
 *   mooscript-context-private.h
 *
 *   Copyright (C) 2004-2006 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef MOO_SCRIPT_CONTEXT_PRIVATE_H
#define MOO_SCRIPT_CONTEXT_PRIVATE_H

#include "mooscript-context.h"

G_BEGIN_DECLS


void         _ms_context_add_builtin        (MSContext  *ctx);

void         _ms_context_print              (MSContext  *ctx,
                                             const char *string);

void         _ms_context_set_return         (MSContext  *ctx,
                                             MSValue    *val);
MSValue     *_ms_context_get_return         (MSContext  *ctx);

void         _ms_context_set_break          (MSContext  *ctx);
void         _ms_context_set_continue       (MSContext  *ctx);
void         _ms_context_unset_return       (MSContext  *ctx);
void         _ms_context_unset_break        (MSContext  *ctx);
void         _ms_context_unset_continue     (MSContext  *ctx);

gboolean     _ms_context_return_set         (MSContext  *ctx);
gboolean     _ms_context_break_set          (MSContext  *ctx);
gboolean     _ms_context_continue_set       (MSContext  *ctx);
gboolean     _ms_context_error_set          (MSContext  *ctx);


G_END_DECLS

#endif /* MOO_SCRIPT_CONTEXT_PRIVATE_H */
