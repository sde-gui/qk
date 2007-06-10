/*
 *   mooscript-func-private.h
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

#ifndef MOO_SCRIPT_FUNC_PRIVATE_H
#define MOO_SCRIPT_FUNC_PRIVATE_H

#include "mooscript-func.h"
#include "mooscript-value-private.h"

G_BEGIN_DECLS


MSValue        *_ms_func_call       (MSFunc     *func,
                                     MSValue   **args,
                                     guint       n_args,
                                     MSContext  *ctx);

const char     *_ms_binary_op_name  (MSBinaryOp      op);
MSCFunc_2       _ms_binary_op_cfunc (MSBinaryOp      op);
const char     *_ms_unary_op_name   (MSUnaryOp       op);
MSCFunc_1       _ms_unary_op_cfunc  (MSUnaryOp       op);


G_END_DECLS

#endif /* MOO_SCRIPT_FUNC_PRIVATE_H */
