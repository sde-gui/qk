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
#include "mooutils/mooprefs.h"
#include <string.h>


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
        char *str = ms_value_repr (arg);
        ms_context_format_error (ctx, MS_ERROR_TYPE,
                                 "could not convert %s to int",
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
    MSValue *ret;

    script = ms_value_print (arg);
    ret = ms_context_run_python (ctx, script);

    g_free (script);
    return ret;
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


static MSValue*
prefs_get_func (MSValue   *arg,
                MSContext *ctx)
{
    char *key = NULL;
    const char *val;
    MSValue *ret = NULL;

    key = ms_value_print (arg);

    if (!key || !key[0])
    {
        ms_context_format_error (ctx, MS_ERROR_VALUE,
                                 "empty prefs key");
        goto out;
    }

    if (!moo_prefs_key_registered (key))
        moo_prefs_new_key_string (key, NULL);

    val = moo_prefs_get_string (key);

    if (val)
        ret = ms_value_string (val);
    else
        ret = ms_value_none ();

out:
    g_free (key);
    return ret;
}

static MSValue*
prefs_set_func (MSValue   *arg1,
                MSValue   *arg2,
                MSContext *ctx)
{
    char *key = NULL, *val = NULL;
    MSValue *ret = NULL;

    key = ms_value_print (arg1);

    if (!ms_value_is_none (arg2))
        val = ms_value_print (arg2);

    if (!key || !key[0])
    {
        ms_context_format_error (ctx, MS_ERROR_VALUE,
                                 "empty prefs key");
        goto out;
    }

    if (!moo_prefs_key_registered (key))
        moo_prefs_new_key_string (key, NULL);

    moo_prefs_set_string (key, val);

    ret = ms_value_none ();

out:
    g_free (key);
    g_free (val);
    return ret;
}


static MSValue*
exec_func (MSValue   *arg,
           MSContext *ctx)
{
    char *cmd = NULL, *cmd_out = NULL;
    MSValue *ret = NULL;
    GError *error = NULL;

    cmd = ms_value_print (arg);

    if (!cmd || !cmd[0])
    {
        ms_context_format_error (ctx, MS_ERROR_VALUE,
                                 "empty command");
        goto out;
    }

    if (!g_spawn_command_line_sync (cmd, &cmd_out, NULL, NULL, &error))
    {
        ms_context_format_error (ctx, MS_ERROR_RUNTIME,
                                 "%s", error->message);
        g_error_free (error);
        goto out;
    }

    if (cmd_out)
    {
        guint len = strlen (cmd_out);

        if (len && cmd_out[len-1] == '\n')
        {
            if (len > 1 && cmd_out[len-2] == '\r')
                cmd_out[len-2] = 0;
            else
                cmd_out[len-1] = 0;
        }
    }

    if (cmd_out)
        ret = ms_value_string (cmd_out);
    else
        ret = ms_value_string ("");

out:
    g_free (cmd);
    g_free (cmd_out);
    return ret;
}


static MSValue*
file_exists_func (MSValue   *arg,
                  MSContext *ctx)
{
    char *filename;
    MSValue *ret = NULL;

    filename = ms_value_print (arg);

    if (g_file_test (filename, G_FILE_TEST_EXISTS))
        ret = ms_value_true ();
    else
        ret = ms_value_false ();

    g_free (filename);
    return ret;
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

    ADD_FUNC (ms_cfunc_new_1, str_func, "Str");
    ADD_FUNC (ms_cfunc_new_1, int_func, "Int");
    ADD_FUNC (ms_cfunc_new_1, len_func, "Len");

    ADD_FUNC (ms_cfunc_new_var, print_func, "Print");
    ADD_FUNC (ms_cfunc_new_1, python_func, "Python");
    ADD_FUNC (ms_cfunc_new_1, include_func, "Include");
    ADD_FUNC (ms_cfunc_new_0, abort_func, "Abort");

    ADD_FUNC (ms_cfunc_new_1, exec_func, "Exec");
    ADD_FUNC (ms_cfunc_new_1, file_exists_func, "FileExists");

    ADD_FUNC (ms_cfunc_new_1, prefs_get_func, "PrefsGet");
    ADD_FUNC (ms_cfunc_new_2, prefs_set_func, "PrefsSet");

    ADD_FUNC_OBJ (ms_zenity_text, "Text");
    ADD_FUNC_OBJ (ms_zenity_entry, "Entry");
    ADD_FUNC_OBJ (ms_zenity_history_entry, "HistoryEntry");
    ADD_FUNC_OBJ (ms_zenity_info, "Info");
    ADD_FUNC_OBJ (ms_zenity_error, "Error");
    ADD_FUNC_OBJ (ms_zenity_question, "Question");
    ADD_FUNC_OBJ (ms_zenity_warning, "Warning");
    ADD_FUNC_OBJ (ms_zenity_choose_file, "ChooseFile");
    ADD_FUNC_OBJ (ms_zenity_choose_files, "ChooseFiles");
    ADD_FUNC_OBJ (ms_zenity_choose_dir, "ChooseDir");
    ADD_FUNC_OBJ (ms_zenity_choose_file_save, "ChooseFileSave");
}


/**********************************************************************/
/* Methods
 */

static void
add_meth (MSValueClass *klass,
          const char   *name,
          MSFunc       *func)
{
    ms_value_class_add_method (klass, name, func);
    g_object_unref (func);
}

#define add_meth0(type, name, cfunc)                        \
    add_meth (&types[type], name, ms_cfunc_new_1 (cfunc))
#define add_meth1(type, name, cfunc)                        \
    add_meth (&types[type], name, ms_cfunc_new_2 (cfunc))


static void
dict_add_key (const char *key,
              G_GNUC_UNUSED gpointer val,
              gpointer user_data)
{
    MSValue *vkey;
    struct {
        MSValue *list;
        guint i;
    } *data = user_data;

    vkey = ms_value_string (key);
    ms_value_list_set_elm (data->list, data->i++, vkey);
    ms_value_unref (vkey);
}

static MSValue *
dict_keys_func (MSValue *dict,
                G_GNUC_UNUSED MSContext *ctx)
{
    guint n_keys;
    struct {
        MSValue *list;
        guint i;
    } data;

    n_keys = g_hash_table_size (dict->hash);
    data.list = ms_value_list (n_keys);
    data.i = 0;
    g_hash_table_foreach (dict->hash, (GHFunc) dict_add_key, &data);

    return data.list;
}


static MSValue *
dict_has_key_func (MSValue *dict,
                   MSValue *key,
                   G_GNUC_UNUSED MSContext *ctx)
{
    MSValue *val, *ret;

    if (MS_VALUE_TYPE (key) != MS_VALUE_STRING)
        return ms_value_false ();

    val = ms_value_dict_get_elm (dict, key->str);
    ret = val ? ms_value_true () : ms_value_false ();
    ms_value_unref (val);

    return ret;
}


static MSValue *
str_len_func (MSValue *val,
              G_GNUC_UNUSED MSContext *ctx)
{
    return ms_value_int (g_utf8_strlen (val->str, -1));
}

static MSValue *
dict_len_func (MSValue *val,
               G_GNUC_UNUSED MSContext *ctx)
{
    return ms_value_int (g_hash_table_size (val->hash));
}

static MSValue *
list_len_func (MSValue *val,
               G_GNUC_UNUSED MSContext *ctx)
{
    return ms_value_int (val->list.n_elms);
}

static MSValue *
list_max_func (MSValue   *val,
               MSContext *ctx)
{
    guint i;
    MSValue *max;

    if (!val->list.n_elms)
        return ms_context_format_error (ctx, MS_ERROR_VALUE,
                                        "requested MAX of empty list");

    max = val->list.elms[0];

    for (i = 1; i < val->list.n_elms; ++i)
        if (ms_value_cmp (max, val->list.elms[i]) < 0)
            max = val->list.elms[i];

    return ms_value_ref (max);
}

static MSValue *
list_min_func (MSValue   *val,
               MSContext *ctx)
{
    guint i;
    MSValue *min;

    if (!val->list.n_elms)
        return ms_context_format_error (ctx, MS_ERROR_VALUE,
                                        "requested MIN of empty list");

    min = val->list.elms[0];

    for (i = 1; i < val->list.n_elms; ++i)
        if (ms_value_cmp (min, val->list.elms[i]) > 0)
            min = val->list.elms[i];

    return ms_value_ref (min);
}


static MSValue *
list_copy_func (MSValue   *val,
                G_GNUC_UNUSED MSContext *ctx)
{
    MSValue *copy;
    guint i;

    copy = ms_value_list (val->list.n_elms);

    for (i = 0; i < val->list.n_elms; ++i)
        ms_value_list_set_elm (copy, i, val->list.elms[i]);

    return copy;
}


void
_ms_type_init_builtin (MSValueClass *types)
{
    add_meth0 (MS_VALUE_STRING, "len", str_len_func);

    add_meth0 (MS_VALUE_LIST, "len", list_len_func);
    add_meth0 (MS_VALUE_LIST, "max", list_max_func);
    add_meth0 (MS_VALUE_LIST, "min", list_min_func);
    add_meth0 (MS_VALUE_LIST, "copy", list_copy_func);
//     add_meth0 (MS_VALUE_LIST, "deep_copy", list_deep_copy_func);

    add_meth0 (MS_VALUE_DICT, "len", dict_len_func);
    add_meth0 (MS_VALUE_DICT, "keys", dict_keys_func);
    add_meth1 (MS_VALUE_DICT, "has_key", dict_has_key_func);
//     add_meth0 (MS_VALUE_DICT, "copy", dict_copy_func);
//     add_meth0 (MS_VALUE_DICT, "deep_copy", dict_deep_copy_func);
}
