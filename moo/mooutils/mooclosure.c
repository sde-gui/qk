/*
 *   mooutils/mooclosure.c
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

#include "mooutils/mooclosure.h"
#include "mooutils/moomarshals.h"
#include "mooutils/moocompat.h"


static void moo_closure_class_init          (MooClosureClass    *klass);

static GObject *moo_closure_constructor     (GType                  type,
                                             guint                  n_construct_properties,
                                             GObjectConstructParam *construct_param);

static void moo_closure_init                (MooClosure         *closure);
static void moo_closure_finalize            (GObject            *object);
static void moo_closure_set_property        (GObject            *object,
                                             guint               prop_id,
                                             const GValue       *value,
                                             GParamSpec         *pspec);

static void moo_closure_invoke_real         (MooClosure         *closure);
static void moo_closure_object_destroyed    (MooClosure        *closure,
                                             gpointer            object);


enum {
    PROP_0,
    PROP_OBJECT,
    PROP_SIGNAL,
    PROP_CALLBACK,
    PROP_PROXY_FUNC,
    PROP_DATA
};

enum {
    INVOKE,
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = {0};


G_DEFINE_TYPE(MooClosure, moo_closure, GTK_TYPE_OBJECT)


static void moo_closure_class_init (MooClosureClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->constructor = moo_closure_constructor;
    gobject_class->finalize = moo_closure_finalize;
    gobject_class->set_property = moo_closure_set_property;

    klass->invoke = moo_closure_invoke_real;

    g_object_class_install_property (gobject_class,
                                     PROP_OBJECT,
                                     g_param_spec_boolean
                                           ("object",
                                            "object",
                                            "object",
                                            FALSE,
                                            G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

    g_object_class_install_property (gobject_class,
                                     PROP_SIGNAL,
                                     g_param_spec_string
                                           ("signal",
                                            "signal",
                                            "signal",
                                            NULL,
                                            G_PARAM_WRITABLE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_CALLBACK,
                                     g_param_spec_pointer
                                           ("callback",
                                            "callback",
                                            "callback",
                                            G_PARAM_WRITABLE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_PROXY_FUNC,
                                     g_param_spec_pointer
                                           ("proxy-func",
                                            "proxy-func",
                                            "proxy-func",
                                            G_PARAM_WRITABLE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_DATA,
                                     g_param_spec_pointer
                                           ("data",
                                            "data",
                                            "data",
                                            G_PARAM_WRITABLE | G_PARAM_CONSTRUCT));

    signals[INVOKE] =
        g_signal_new ("invoke",
                    G_OBJECT_CLASS_TYPE (klass),
                    (GSignalFlags) (G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION),
                    G_STRUCT_OFFSET (MooClosureClass, invoke),
                    NULL, NULL,
                    _moo_marshal_VOID__VOID,
                    G_TYPE_NONE, 0);
}


static void moo_closure_init (MooClosure *closure)
{
    closure->callback = NULL;
    closure->proxy_func = NULL;
    closure->object = FALSE;
    closure->signal = NULL;
    closure->data = NULL;
    closure->valid = TRUE;
    closure->constructed = FALSE;
}


static GObject *moo_closure_constructor    (GType                  type,
                                             guint                  n_construct_properties,
                                             GObjectConstructParam *construct_param)
{
    GObject *object;
    MooClosure *closure;

    object = G_OBJECT_CLASS (moo_closure_parent_class)->constructor (
        type, n_construct_properties, construct_param);
    g_return_val_if_fail (object != NULL, NULL);

    closure = MOO_CLOSURE (object);
    if (closure->object && closure->data)
        g_object_weak_ref (G_OBJECT (closure->data),
                           (GWeakNotify) moo_closure_object_destroyed,
                           closure);
    closure->constructed = TRUE;

    return object;
}


static void moo_closure_finalize       (GObject      *object)
{
    MooClosure *closure = MOO_CLOSURE (object);
    g_return_if_fail (closure != NULL);
    if (closure->object && G_IS_OBJECT (closure->data) && closure->valid)
        g_object_weak_unref (G_OBJECT (closure->data),
                             (GWeakNotify) moo_closure_object_destroyed,
                             closure);
    g_free (closure->signal);
    closure->valid = FALSE;
    G_OBJECT_CLASS (moo_closure_parent_class)->finalize (object);
}


static void moo_closure_invoke_real (MooClosure *closure)
{
    gpointer data = closure->data;

    g_return_if_fail (closure->valid);
    g_return_if_fail (closure->callback || closure->signal);

    if (closure->proxy_func) {
        data = closure->proxy_func (closure->data);
        g_return_if_fail (G_IS_OBJECT (data));
    }

    if (closure->signal)
    {
        gboolean ret;
        g_object_ref (G_OBJECT (data));
        g_signal_emit_by_name (data, closure->signal, &ret);
        g_object_unref (G_OBJECT (data));
    }
    else if (closure->object)
    {
        g_object_ref (G_OBJECT (data));
        closure->callback (data);
        g_object_unref (G_OBJECT (data));
    }
    else
    {
        closure->callback (data);
    }
}


void         moo_closure_invoke     (MooClosure         *closure)
{
    g_return_if_fail (MOO_IS_CLOSURE (closure));
    g_signal_emit (closure, signals[INVOKE], 0);
}


static void moo_closure_set_property       (GObject            *object,
                                             guint               prop_id,
                                             const GValue       *value,
                                             GParamSpec         *pspec)
{
    MooClosure *closure = MOO_CLOSURE (object);

    switch (prop_id)
    {
        case PROP_OBJECT:
            closure->object = g_value_get_boolean (value);
            break;

        case PROP_SIGNAL:
            g_free (closure->signal);
            closure->signal = g_strdup (g_value_get_string (value));
            if (closure->signal) closure->object = TRUE;
            break;

        case PROP_CALLBACK:
            closure->callback = (void(*)(gpointer))g_value_get_pointer (value);
            break;

        case PROP_PROXY_FUNC:
            closure->proxy_func = (gpointer(*)(gpointer))g_value_get_pointer (value);
            break;

        case PROP_DATA:
            if (!closure->constructed) {
                closure->data = g_value_get_pointer (value);
            }
            else {
                if (closure->data && closure->object) {
                    g_object_weak_unref (G_OBJECT (closure->data),
                                         (GWeakNotify) moo_closure_object_destroyed,
                                         closure);
                }
                closure->data = g_value_get_pointer (value);
                if (closure->data && closure->object) {
                    g_object_weak_ref (G_OBJECT (closure->data),
                                       (GWeakNotify) moo_closure_object_destroyed,
                                       closure);
                }
            }
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


void        moo_closure_invalidate (MooClosure     *closure)
{
    g_return_if_fail (MOO_IS_CLOSURE (closure));
    if (closure->object && G_IS_OBJECT (closure->data))
        g_object_weak_unref (G_OBJECT (closure->data),
                             (GWeakNotify) moo_closure_object_destroyed,
                             closure);
    closure->data = NULL;
    closure->object = FALSE;
    closure->valid = FALSE;
}

static void moo_closure_object_destroyed  (MooClosure        *closure,
                                           G_GNUC_UNUSED gpointer           object)
{
    g_assert (closure->data == object);
    closure->object = FALSE;
    moo_closure_invalidate (closure);
}


MooClosure  *moo_closure_new        (GCallback       callback_func,
                                       gpointer        data)
{
    return MOO_CLOSURE (g_object_new (MOO_TYPE_CLOSURE,
                                     "callback", callback_func,
                                     "data", data,
                                     NULL));
}

MooClosure  *moo_closure_new_object (GCallback       callback_func,
                                       gpointer        object)
{
    g_return_val_if_fail (callback_func != NULL, NULL);
    g_return_val_if_fail (G_IS_OBJECT (object), NULL);
    return MOO_CLOSURE (g_object_new (MOO_TYPE_CLOSURE,
                                     "callback", callback_func,
                                     "data", object,
                                     "object", TRUE,
                                     NULL));
}

MooClosure  *moo_closure_new_signal   (const char     *signal,
                                       gpointer        object)
{
    g_return_val_if_fail (signal != NULL, NULL);
    g_return_val_if_fail (G_IS_OBJECT (object), NULL);
    return MOO_CLOSURE (g_object_new (MOO_TYPE_CLOSURE,
                                     "signal", signal,
                                     "data", object,
                                     NULL));
}

MooClosure  *moo_closure_new_proxy  (GCallback       callback_func,
                                     GCallback       proxy_func,
                                     gpointer        object)
{
    g_return_val_if_fail (callback_func != NULL && proxy_func != NULL, NULL);
    g_return_val_if_fail (G_IS_OBJECT (object), NULL);
    return MOO_CLOSURE (g_object_new (MOO_TYPE_CLOSURE,
                                      "callback", callback_func,
                                      "proxy_func", proxy_func,
                                      "data", object,
                                      "object", TRUE,
                                      NULL));
}

MooClosure  *moo_closure_new_proxy_signal  (const char      *signal,
                                            GCallback        proxy_func,
                                            gpointer         object)
{
    g_return_val_if_fail (signal != NULL && proxy_func != NULL, NULL);
    g_return_val_if_fail (G_IS_OBJECT (object), NULL);
    return MOO_CLOSURE (g_object_new (MOO_TYPE_CLOSURE,
                                      "signal", signal,
                                      "proxy_func", proxy_func,
                                      "data", object,
                                      "object", TRUE,
                                      NULL));
}
