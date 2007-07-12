/*
 *   mooscript-context.c
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

#include "mooscript-context-private.h"
#include "mooscript-parser.h"
#include "mooscript-func-private.h"
#include "mooutils/moomarshals.h"
#include "mooutils/mooutils-misc.h"
#include <glib/gprintf.h>
#include <gtk/gtkwindow.h>

#define N_POS_VARS 20

typedef void (*MSPrintFunc) (const char *string,
                             MSContext  *ctx);

struct _MSContextPrivate {
    GHashTable *vars;
    MSError error;
    char *error_msg;
    MSPrintFunc print_func;
    gpointer window;
    MSValue *return_val;
    guint break_set : 1;
    guint continue_set : 1;
    guint return_set : 1;
};


@implementation MSContext : MooCObject

static void
default_print_func (const char  *string,
                    G_GNUC_UNUSED MSContext *ctx)
{
    g_print ("%s", string);
}

static void
variable_unref (MSVariable *var)
{
    if (var)
        [var release];
}

- init
{
    [super init];
    priv = _moo_new0 (MSContextPrivate);
    priv->vars = g_hash_table_new_full (g_str_hash, g_str_equal, g_free,
                                        (GDestroyNotify) variable_unref);
    priv->print_func = default_print_func;
    _ms_context_add_builtin (self);
    return self;
}

- (void) dealloc
{
    g_hash_table_destroy (priv->vars);
    g_free (priv->error_msg);
    ms_value_unref (priv->return_val);
    _moo_free (MSContextPrivate, priv);
    [super dealloc];
}

- (gpointer) window
{
    return priv->window;
}

- (void) setWindow:(gpointer) window
{
    priv->window = window;
}

+ (MSContext*) new:(gpointer) window
{
    MSContext *ctx;

    ctx = [[self alloc] init];
    [ctx setWindow:window];

    return ctx;
}

+ (MSContext*) new
{
    return [self new:NULL];
}

+ initialize
{
    ms_type_init ();
    return self;
}


- (MSValue*) evalVariable:(CSTR) name
{
    MSVariable *var;

    g_return_val_if_fail (name != NULL, NULL);

    var = [self lookupVar:name];

    if (!var)
        return [self formatError:MS_ERROR_NAME
                                :"no variable named '%s'",
                                name];

    if ([var value])
        return ms_value_ref ([var value]);

    return [[var func] call:NULL :0 :self];
}

- (BOOL) assignVariable:(CSTR) name
                       :(MSValue*) value
{
    MSVariable *var;

    g_return_val_if_fail (name != NULL, NO);

    var = [self lookupVar:name];

    if (value)
    {
        if (var)
        {
            [var setValue:value];
        }
        else
        {
            var = [MSVariable new:value];
            [self setVar:name :var];
            [var release];
        }
    }
    else if (var)
    {
        [self setVar:name :NULL];
    }

    return YES;
}

- (BOOL) assignPositional:(guint) n
                         :(MSValue*) value
{
    char *name;
    gboolean result;

    name = g_strdup_printf ("_%u", n);
    result = [self assignVariable:name :value];

    g_free (name);
    return result;
}

- (BOOL) assignString:(CSTR) name
                     :(CSTR) str_value
{
    MSValue *value = NULL;
    gboolean retval;

    g_return_val_if_fail (name != NULL, NO);

    if (str_value)
        value = ms_value_string (str_value);

    retval = [self assignVariable:name :value];

    ms_value_unref (value);
    return retval;
}


- (MSValue*) getEnvVariable:(CSTR) name
{
    MOO_UNUSED_VAR (name);
    return NULL;
}


- (BOOL) setVar:(CSTR) name
               :(MSVariable*) var
{
    MSVariable *old;

    g_return_val_if_fail (name != NULL, NO);

    old = g_hash_table_lookup (priv->vars, name);

    if (var != old)
    {
        if (var)
            g_hash_table_insert (priv->vars,
                                 g_strdup (name),
                                 [var retain]);
        else
            g_hash_table_remove (priv->vars, name);
    }

    return YES;
}

- (BOOL) setFunc:(CSTR) name
                :(MSFunc*) func
{
    MSValue *vfunc;
    gboolean ret;

    g_return_val_if_fail (name != NULL, NO);

    vfunc = ms_value_func (func);
    ret = [self assignVariable:name :vfunc];
    ms_value_unref (vfunc);

    return ret;
}


- (MSValue*) setError:(MSError) error
{
    return [self setError:error :NULL];
}

- (MSValue*) setError:(MSError) error
                     :(CSTR) message
{
    const char *errname;

    g_return_val_if_fail (!priv->error && error, NULL);
    g_return_val_if_fail (!priv->error_msg, NULL);

    priv->error = error;
    errname = [self getErrorMsg];

    if (message && *message)
        priv->error_msg = g_strdup_printf ("%s: %s", errname, message);
    else
        priv->error_msg = g_strdup (message);

    return NULL;
}

- (MSValue*) formatError:(MSError) error
                        :(CSTR) format,
                        ...
{
    va_list args;
    char *string;

    g_return_val_if_fail (!priv->error && error, NULL);
    g_return_val_if_fail (!priv->error_msg, NULL);

    if (!format || !format[0])
    {
        [self setError:error :NULL];
        return NULL;
    }

    va_start (args, format);
    string = _ms_vaprintf (format, args);
    va_end (args);

    [self setError:error :string];
    g_free (string);

    return NULL;
}

- (CSTR) getErrorMsg
{
    static const char *msgs[MS_ERROR_LAST] = {
        NULL, "Type error", "Value error", "Name error",
        "Runtime error"
    };

    g_return_val_if_fail (priv->error < MS_ERROR_LAST, NULL);
    g_return_val_if_fail (priv->error != MS_ERROR_NONE, "ERROR");

    if (priv->error_msg)
        return priv->error_msg;
    else
        return msgs[priv->error];
}

- (void) clearError
{
    priv->error = MS_ERROR_NONE;
    g_free (priv->error_msg);
    priv->error_msg = NULL;
}

@end


@implementation MSContext (MSContextPrivate)

- (MSVariable*) lookupVar:(CSTR)name
{
    g_return_val_if_fail (name != NULL, NULL);
    return g_hash_table_lookup (priv->vars, name);
}


- (void) setReturn:(MSValue*) val
{
    g_return_if_fail (!priv->return_set);
    priv->return_set = YES;
    priv->return_val = val ? ms_value_ref (val) : ms_value_none ();
}

- (MSValue*) getReturn
{
    g_return_val_if_fail (priv->return_set, NULL);
    return ms_value_ref (priv->return_val);
}

- (void) unsetReturn
{
    g_return_if_fail (priv->return_set);
    priv->return_set = NO;
    ms_value_unref (priv->return_val);
    priv->return_val = NULL;
}

- (BOOL) returnSet
{
    return priv->return_set;
}


- (void) setBreak
{
    g_return_if_fail (!priv->break_set);
    priv->break_set = YES;
}

- (void) setContinue
{
    g_return_if_fail (!priv->continue_set);
    priv->continue_set = YES;
}

- (void) unsetBreak
{
    g_return_if_fail (priv->break_set);
    priv->break_set = NO;
}

- (void) unsetContinue
{
    g_return_if_fail (priv->continue_set);
    priv->continue_set = NO;
}

- (BOOL) breakSet
{
    return priv->break_set;
}

- (BOOL) continueSet
{
    return priv->continue_set;
}


- (BOOL) errorSet;
{
    return priv->error != 0;
}


- (void) print:(CSTR) string
{
    priv->print_func (string, self);
}

@end


@implementation MSVariable : MooCObject

+ (MSVariable*) new:(MSValue*) value
{
    MSVariable *var;

    g_return_val_if_fail (value != NULL, nil);

    var = [[self alloc] init];
    var->value = ms_value_ref (value);

    return var;
}

- (void) dealloc
{
    if (value)
        ms_value_unref (value);
    if (func)
        [func release];
    [super dealloc];
}

- (void) setValue:(MSValue*) new_value
{
    if (value != new_value)
    {
        if (value)
            ms_value_unref (value);
        if (new_value)
            ms_value_ref (new_value);

        value = new_value;
    }

    if (func)
    {
        [func release];
        func = NULL;
    }
}

- (void) setFunc:(MSFunc*) new_func
{
    if (func != new_func)
    {
        if (func)
            [func release];
        if (new_func)
            [new_func retain];

        func = new_func;
    }

    if (value)
    {
        ms_value_unref (value);
        value = NULL;
    }
}

- (MSValue*) value
{
    return value;
}

- (MSFunc*) func
{
    return func;
}

@end


/* -*- objc -*- */
