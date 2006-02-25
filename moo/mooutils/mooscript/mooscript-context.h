/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   mooscript-context.h
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

#ifndef __MOO_SCRIPT_CONTEXT_H__
#define __MOO_SCRIPT_CONTEXT_H__

#include "mooscript-func.h"

G_BEGIN_DECLS


#define MS_TYPE_CONTEXT                    (ms_context_get_type ())
#define MS_CONTEXT(object)                 (G_TYPE_CHECK_INSTANCE_CAST ((object), MS_TYPE_CONTEXT, MSContext))
#define MS_CONTEXT_CLASS(klass)            (G_TYPE_CHECK_CLASS_CAST ((klass), MS_TYPE_CONTEXT, MSContextClass))
#define MS_IS_CONTEXT(object)              (G_TYPE_CHECK_INSTANCE_TYPE ((object), MS_TYPE_CONTEXT))
#define MS_IS_CONTEXT_CLASS(klass)         (G_TYPE_CHECK_CLASS_TYPE ((klass), MS_TYPE_CONTEXT))
#define MS_CONTEXT_GET_CLASS(obj)          (G_TYPE_INSTANCE_GET_CLASS ((obj), MS_TYPE_CONTEXT, MSContextClass))

typedef struct _MSContextClass MSContextClass;
typedef struct _MSVariable MSVariable;

struct _MSVariable {
    guint ref_count;
    MSValue *value;
    MSFunc *func; /* called with no arguments */
};

typedef enum {
    MS_ERROR_NONE = 0,
    MS_ERROR_TYPE,
    MS_ERROR_VALUE,
    MS_ERROR_NAME,
    MS_ERROR_LMST
} MSError;

typedef void (*MSPrintFunc) (const char *string,
                             MSContext  *ctx);

struct _MSContext {
    GObject object;

    GHashTable *funcs;
    GHashTable *vars;

    MSError error;
    char *error_msg;

    MSPrintFunc print_func;
};

struct _MSContextClass {
    GObjectClass object_class;
};


GType        ms_context_get_type            (void) G_GNUC_CONST;

MSVariable  *ms_variable_new_value          (MSValue    *value);
MSVariable  *ms_variable_new_func           (MSFunc     *func);
MSVariable  *ms_variable_ref                (MSVariable *var);
void         ms_variable_unref              (MSVariable *var);

MSContext   *ms_context_new                 (void);

MSValue     *ms_context_eval_variable       (MSContext  *ctx,
                                             const char *name);
gboolean     ms_context_assign_variable     (MSContext  *ctx,
                                             const char *name,
                                             MSValue    *value);
gboolean     ms_context_assign_positional   (MSContext  *ctx,
                                             guint       n,
                                             MSValue    *value);

MSVariable  *ms_context_lookup_var          (MSContext  *ctx,
                                             const char *name);
gboolean     ms_context_set_var             (MSContext  *ctx,
                                             const char *name,
                                             MSVariable *var);

MSFunc      *ms_context_lookup_func         (MSContext  *ctx,
                                             const char *name);
gboolean     ms_context_set_func            (MSContext  *ctx,
                                             const char *name,
                                             MSFunc     *func);

MSValue     *ms_context_set_error           (MSContext  *ctx,
                                             MSError     error,
                                             const char *message);
MSValue     *ms_context_format_error        (MSContext  *ctx,
                                             MSError     error,
                                             const char *format,
                                             ...);

const char  *ms_context_get_error_msg       (MSContext  *ctx);
void         ms_context_clear_error         (MSContext  *ctx);


G_END_DECLS

#endif /* __MOO_SCRIPT_CONTEXT_H__ */
