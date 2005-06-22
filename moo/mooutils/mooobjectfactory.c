/*
 *   mooutils/mooobjectfactory.c
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

#include "mooutils/mooobjectfactory.h"
#include "mooutils/mootypes.h"
#include "mooutils/moocompat.h"
#include "mooutils/mooparam.h"
#include <string.h>
#include <gobject/gvaluecollector.h>


static void moo_object_factory_class_init      (MooObjectFactoryClass *klass);
static void moo_object_factory_init            (MooObjectFactory      *factory);
static void moo_object_factory_finalize        (GObject               *object);

static void moo_object_factory_set_property    (GObject            *object,
                                                guint               prop_id,
                                                const GValue       *value,
                                                GParamSpec         *pspec);
static void moo_object_factory_get_property    (GObject            *object,
                                                guint               prop_id,
                                                GValue             *value,
                                                GParamSpec         *pspec);


enum {
    PROP_0,
    PROP_OBJECT_TYPE,
    PROP_FACTORY_FUNC,
    PROP_FACTORY_FUNC_DATA
};



/* MOO_TYPE_OBJECT_FACTORY */
G_DEFINE_TYPE (MooObjectFactory, moo_object_factory, G_TYPE_OBJECT)


static void moo_object_factory_class_init (MooObjectFactoryClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->set_property = moo_object_factory_set_property;
    gobject_class->get_property = moo_object_factory_get_property;
    gobject_class->finalize = moo_object_factory_finalize;

    g_object_class_install_property (gobject_class,
                                     PROP_OBJECT_TYPE,
                                     g_param_spec_boxed ("object_type",
                                                         "object_type",
                                                         "object_type",
                                                         MOO_TYPE_GTYPE,
                                                         G_PARAM_CONSTRUCT | G_PARAM_READWRITE));
    g_object_class_install_property (gobject_class,
                                     PROP_FACTORY_FUNC,
                                     g_param_spec_pointer ("factory_func",
                                                           "factory_func",
                                                           "factory_func",
                                                           G_PARAM_CONSTRUCT | G_PARAM_READWRITE));
    g_object_class_install_property (gobject_class,
                                     PROP_FACTORY_FUNC_DATA,
                                     g_param_spec_pointer ("factory_func_data",
                                                           "factory_func_data",
                                                           "factory_func_data",
                                                           G_PARAM_CONSTRUCT | G_PARAM_READWRITE));
}


static void moo_object_factory_init (MooObjectFactory *factory)
{
    factory->factory_func = NULL;
    factory->factory_func_data = NULL;
    factory->object_type = 0;
    factory->n_params = 0;
    factory->params = NULL;
}


static void moo_object_factory_get_property    (GObject        *object,
                                             guint           prop_id,
                                             GValue         *value,
                                             GParamSpec     *pspec)
{
    MooObjectFactory *factory = MOO_OBJECT_FACTORY (object);

    switch (prop_id)
    {
        case PROP_FACTORY_FUNC:
            g_value_set_pointer (value, (gpointer)factory->factory_func);
            break;

        case PROP_FACTORY_FUNC_DATA:
            g_value_set_pointer (value, factory->factory_func_data);
            break;

        case PROP_OBJECT_TYPE:
            moo_value_set_gtype (value, factory->object_type);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void moo_object_factory_set_property  (GObject        *object,
                                              guint           prop_id,
                                              const GValue   *value,
                                              GParamSpec     *pspec)
{
    MooObjectFactory *factory = MOO_OBJECT_FACTORY (object);

    switch (prop_id)
    {
        case PROP_FACTORY_FUNC:
            factory->factory_func = (MooObjectFactoryFunc)g_value_get_pointer (value);
            break;

        case PROP_FACTORY_FUNC_DATA:
            factory->factory_func_data = g_value_get_pointer (value);
            break;

        case PROP_OBJECT_TYPE:
            factory->object_type = moo_value_get_gtype (value);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void moo_object_factory_finalize        (GObject               *object)
{
    MooObjectFactory *factory = MOO_OBJECT_FACTORY (object);
    moo_param_array_free (factory->params, factory->n_params);

    G_OBJECT_CLASS (moo_object_factory_parent_class)->finalize (object);
}


GObject     *moo_object_factory_create_object   (MooObjectFactory   *factory,
                                                 gpointer            data,
                                                 const char         *prop_name,
                                                 ...)
{
    g_return_val_if_fail (MOO_IS_OBJECT_FACTORY (factory), NULL);

    if (factory->factory_func) {
        if (factory->factory_func_data)
            data = factory->factory_func_data;
        return factory->factory_func (data, factory);
    }

    if (!prop_name)
        return g_object_newv (factory->object_type,
                              factory->n_params,
                              factory->params);
    else {
        GObject *object;
        GParameter *params;
        guint n_params;
        va_list var_args;

        va_start (var_args, prop_name);
        params = moo_param_array_add_valist (factory->object_type,
                                             factory->params,
                                             factory->n_params,
                                             &n_params,
                                             prop_name,
                                             var_args);
        va_end (var_args);

        g_return_val_if_fail (params != NULL, NULL);
        object = g_object_newv (factory->object_type,
                                n_params,
                                params);
        moo_param_array_free (params, n_params);
        return object;
    }
}


MooObjectFactory *moo_object_factory_new_func   (MooObjectFactoryFunc factory_func,
                                                 gpointer            data)
{
    return MOO_OBJECT_FACTORY (g_object_new (MOO_TYPE_OBJECT_FACTORY,
                                             "factory_func", factory_func,
                                             "factory_func_data", data,
                                             NULL));
}


MooObjectFactory *moo_object_factory_new        (GType               object_type,
                                                 const char         *first_prop_name,
                                                 ...)
{
    MooObjectFactory *factory;
    va_list var_args;

    g_return_val_if_fail (G_TYPE_IS_OBJECT (object_type), NULL);

    va_start (var_args, first_prop_name);
    factory = moo_object_factory_new_valist (object_type, first_prop_name, var_args);
    va_end (var_args);

    return factory;
}


MooObjectFactory *moo_object_factory_new_valist (GType               object_type,
                                                 const char         *first_prop_name,
                                                 va_list             var_args)
{
    GParameter *params = NULL;
    guint n_params = 0;
    MooObjectFactory *factory;

    g_return_val_if_fail (G_TYPE_IS_OBJECT (object_type), NULL);

    if (!first_prop_name)
        return g_object_new (MOO_TYPE_OBJECT_FACTORY,
                             "object_type", object_type,
                             NULL);

    params = moo_param_array_collect_valist (object_type,
                                             &n_params,
                                             first_prop_name,
                                             var_args);
    g_return_val_if_fail (params != NULL, NULL);

    factory = g_object_new (MOO_TYPE_OBJECT_FACTORY,
                            "object_type", object_type,
                            NULL);
    factory->n_params = n_params;
    factory->params = params;

    return factory;
}


MooObjectFactory *moo_object_factory_new_a      (GType               object_type,
                                                 GParameter         *params,
                                                 guint               n_params)
{
    MooObjectFactory *factory;

    g_return_val_if_fail (G_TYPE_IS_OBJECT (object_type), NULL);
    g_return_val_if_fail (params != NULL, NULL);

    factory = g_object_new (MOO_TYPE_OBJECT_FACTORY,
                            "object_type", object_type,
                            NULL);

    factory->params = params;
    factory->n_params = n_params;

    return factory;
}
