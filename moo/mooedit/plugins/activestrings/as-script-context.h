/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   as-script-context.h
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

#ifndef __AS_SCRIPT_CONTEXT_H__
#define __AS_SCRIPT_CONTEXT_H__

#include "as-script-func.h"

G_BEGIN_DECLS


#define AS_TYPE_CONTEXT                    (as_context_get_type ())
#define AS_CONTEXT(object)                 (G_TYPE_CHECK_INSTANCE_CAST ((object), AS_TYPE_CONTEXT, ASContext))
#define AS_CONTEXT_CLASS(klass)            (G_TYPE_CHECK_CLASS_CAST ((klass), AS_TYPE_CONTEXT, ASContextClass))
#define AS_IS_CONTEXT(object)              (G_TYPE_CHECK_INSTANCE_TYPE ((object), AS_TYPE_CONTEXT))
#define AS_IS_CONTEXT_CLASS(klass)         (G_TYPE_CHECK_CLASS_TYPE ((klass), AS_TYPE_CONTEXT))
#define AS_CONTEXT_GET_CLASS(obj)          (G_TYPE_INSTANCE_GET_CLASS ((obj), AS_TYPE_CONTEXT, ASContextClass))

typedef struct _ASContextClass ASContextClass;
typedef struct _ASVariable ASVariable;

struct _ASVariable {
    guint ref_count;
    ASValue *value;
    ASFunc *func; /* called with no arguments */
};

typedef enum {
    AS_ERROR_NONE = 0,
    AS_ERROR_TYPE,
    AS_ERROR_VALUE,
    AS_ERROR_NAME,
    AS_ERROR_LAST
} ASError;

typedef void (*ASPrintFunc) (const char *string,
                             ASContext  *ctx);

struct _ASContext {
    GObject object;

    GHashTable *funcs;
    GHashTable *named_vars;
    ASValue **positional_vars;

    ASError error;
    char *error_msg;

    ASPrintFunc print_func;
};

struct _ASContextClass {
    GObjectClass object_class;
};


GType        as_context_get_type            (void) G_GNUC_CONST;

ASVariable  *as_variable_new_value          (ASValue    *value);
ASVariable  *as_variable_new_func           (ASFunc     *func);
ASVariable  *as_variable_ref                (ASVariable *var);
void         as_variable_unref              (ASVariable *var);

ASContext   *as_context_new                 (void);

ASValue     *as_context_eval_positional     (ASContext  *ctx,
                                             guint       num);
ASValue     *as_context_eval_named          (ASContext  *ctx,
                                             const char *name);
gboolean     as_context_assign_positional   (ASContext  *ctx,
                                             guint       num,
                                             ASValue    *value);
gboolean     as_context_assign_named        (ASContext  *ctx,
                                             const char *name,
                                             ASValue    *value);

ASVariable  *as_context_lookup_var          (ASContext  *ctx,
                                             const char *name);
gboolean     as_context_set_var             (ASContext  *ctx,
                                             const char *name,
                                             ASVariable *var);

ASFunc      *as_context_lookup_func         (ASContext  *ctx,
                                             const char *name);
gboolean     as_context_set_func            (ASContext  *ctx,
                                             const char *name,
                                             ASFunc     *func);

ASValue     *as_context_set_error           (ASContext  *ctx,
                                             ASError     error,
                                             const char *message);
ASValue     *as_context_format_error        (ASContext  *ctx,
                                             ASError     error,
                                             const char *format,
                                             ...);

const char  *as_context_get_error_msg       (ASContext  *ctx);
void         as_context_clear_error         (ASContext  *ctx);


G_END_DECLS

#endif /* __AS_SCRIPT_CONTEXT_H__ */
