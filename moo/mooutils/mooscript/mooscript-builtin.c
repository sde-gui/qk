/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   mooscript-builtin.c
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

#include "mooscript-context.h"
#include "mooscript-parser.h"
#include "mooscript-zenity.h"
#include "mooutils/moopython.h"


static MSValue*
print_func (MSValue   **args,
            guint       n_args,
            MSContext  *ctx)
{
    guint i;

    for (i = 0; i < n_args; ++i)
    {
        char *s = ms_value_print (args[i]);
        ctx->print_func (s, ctx);
        g_free (s);
    }

    ctx->print_func ("\n", ctx);
    return ms_value_none ();
}


static MSValue *
len_func (MSValue    *val,
          MSContext  *ctx)
{
    switch (val->klass->type)
    {
        case MS_VALUE_STRING:
            return ms_value_int (g_utf8_strlen (val->str, -1));
        case MS_VALUE_LIST:
            return ms_value_int (val->list.n_elms);
        case MS_VALUE_DICT:
            return ms_value_int (g_hash_table_size (val->hash));
        default:
            return ms_context_set_error (ctx, MS_ERROR_TYPE, NULL);
    }
}


static MSValue*
abort_func (MSContext *ctx)
{
    return ms_context_format_error (ctx, MS_ERROR_RUNTIME, "Aborted");
}


static MSValue*
str_func (MSValue    *arg,
          G_GNUC_UNUSED MSContext *ctx)
{
    char *str;
    MSValue *ret;

    str = ms_value_print (arg);
    ret = ms_value_take_string (str);

    return ret;
}


static MSValue*
int_func (MSValue    *arg,
          MSContext  *ctx)
{
    int ival;

    if (!ms_value_get_int (arg, &ival))
    {
        char *str = ms_value_print (arg);
        ms_context_format_error (ctx, MS_ERROR_TYPE,
                                 "could not convert '%s' to int",
                                 str);
        g_free (str);
        return NULL;
    }

    return ms_value_int (ival);
}


static MSValue*
python_func (MSValue    *arg,
             MSContext  *ctx)
{
    char *script;
    MooPyObject *ret;

    if (!moo_python_running())
        return ms_context_format_error (ctx, MS_ERROR_RUNTIME,
                                        "Python support not available");

    script = ms_value_print (arg);
    ret = moo_python_run_string (script);
    g_free (script);

    if (ret)
    {
        moo_Py_DECREF (ret);
        return ms_value_none ();
    }
    else
    {
        moo_PyErr_Print ();
        return ms_context_format_error (ctx, MS_ERROR_RUNTIME,
                                        "python script raised exception");
    }
}


static MSValue*
include_func (MSValue    *arg,
              MSContext  *ctx)
{
    char *file = NULL, *script = NULL;
    GError *error = NULL;
    MSNode *node = NULL;
    MSValue *ret;

    file = ms_value_print (arg);

    if (!g_file_get_contents (file, &script, NULL, &error))
    {
        ms_context_format_error (ctx, MS_ERROR_RUNTIME,
                                 "%s", error->message);
        goto error;
    }

    node = ms_script_parse (script);

    if (!node)
    {
        ms_context_format_error (ctx, MS_ERROR_RUNTIME,
                                 "%s", error->message);
        goto error;
    }

    ret = ms_top_node_eval (node, ctx);

    g_free (file);
    g_free (script);
    ms_node_unref (node);

    return ret;

error:
    g_error_free (error);
    g_free (file);
    g_free (script);
    if (node)
        g_object_unref (node);
    return NULL;
}


#define ADD_FUNC(type_,func_,name_)             \
G_STMT_START {                                  \
    MSFunc *msfunc__;                           \
    msfunc__ = type_ (func_);                   \
    ms_context_set_func (ctx, name_, msfunc__); \
    g_object_unref (msfunc__);                  \
} G_STMT_END

#define ADD_FUNC_OBJ(factory_,name_)            \
G_STMT_START {                                  \
    MSFunc *msfunc__;                           \
    msfunc__ = factory_ ();                     \
    ms_context_set_func (ctx, name_, msfunc__); \
    g_object_unref (msfunc__);                  \
} G_STMT_END

#define ADD_CONSTANT(func_,name_)               \
G_STMT_START {                                  \
    MSVariable *var_;                           \
    MSValue *val_;                              \
    val_ = func_ ();                            \
    var_ = ms_variable_new_value (val_);        \
    ms_context_set_var (ctx, name_, var_);      \
    ms_variable_unref (var_);                   \
    ms_value_unref (val_);                      \
} G_STMT_END;

void
_ms_context_add_builtin (MSContext *ctx)
{
    guint i;

    ADD_CONSTANT (ms_value_none, "none");
    ADD_CONSTANT (ms_value_true, "true");
    ADD_CONSTANT (ms_value_false, "false");

    for (i = 0; i < MS_BINARY_OP_LAST; ++i)
        ADD_FUNC (ms_cfunc_new_2,
                  ms_binary_op_cfunc (i),
                  ms_binary_op_name (i));

    for (i = 0; i < MS_UNARY_OP_LAST; ++i)
        ADD_FUNC (ms_cfunc_new_1,
                  ms_unary_op_cfunc (i),
                  ms_unary_op_name (i));

    ADD_FUNC (ms_cfunc_new_var, print_func, "Print");
    ADD_FUNC (ms_cfunc_new_1, python_func, "Python");
    ADD_FUNC (ms_cfunc_new_1, include_func, "Include");
    ADD_FUNC (ms_cfunc_new_0, abort_func, "Abort");
    ADD_FUNC (ms_cfunc_new_1, str_func, "Str");
    ADD_FUNC (ms_cfunc_new_1, int_func, "Int");
    ADD_FUNC (ms_cfunc_new_1, len_func, "Len");

    ADD_FUNC_OBJ (ms_zenity_text, "Text");
    ADD_FUNC_OBJ (ms_zenity_entry, "Entry");
    ADD_FUNC_OBJ (ms_zenity_info, "Info");
    ADD_FUNC_OBJ (ms_zenity_error, "Error");
    ADD_FUNC_OBJ (ms_zenity_question, "Question");
    ADD_FUNC_OBJ (ms_zenity_warning, "Warning");
    ADD_FUNC_OBJ (ms_zenity_choose_file, "ChooseFile");
    ADD_FUNC_OBJ (ms_zenity_choose_files, "ChooseFiles");
    ADD_FUNC_OBJ (ms_zenity_choose_dir, "ChooseDir");
    ADD_FUNC_OBJ (ms_zenity_choose_file_save, "ChooseFileSave");
}
