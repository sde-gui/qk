/*
 *   mooutils/mooobjectfactory.h
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

#ifndef MOOUTILS_MOOOBJECTFACTORY_H
#define MOOUTILS_MOOOBJECTFACTORY_H

#include <glib-object.h>

G_BEGIN_DECLS


#define MOO_TYPE_OBJECT_FACTORY              (moo_object_factory_get_type ())
#define MOO_OBJECT_FACTORY(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_OBJECT_FACTORY, MooObjectFactory))
#define MOO_OBJECT_FACTORY_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_OBJECT_FACTORY, MooObjectFactoryClass))
#define MOO_IS_OBJECT_FACTORY(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_OBJECT_FACTORY))
#define MOO_IS_OBJECT_FACTORY_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_OBJECT_FACTORY))
#define MOO_OBJECT_FACTORY_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_OBJECT_FACTORY, MooObjectFactoryClass))


typedef struct _MooObjectFactory        MooObjectFactory;
typedef struct _MooObjectFactoryClass   MooObjectFactoryClass;

typedef GObject *(*MooObjectFactoryFunc) (gpointer             data,
                                          MooObjectFactory    *factory);

struct _MooObjectFactory
{
    GObject      object;

    MooObjectFactoryFunc factory_func;
    gpointer             factory_func_data;

    GType       object_type;
    guint       n_params;
    GParameter *params;
};

struct _MooObjectFactoryClass
{
    GObjectClass parent_class;
};


GType        moo_object_factory_get_type        (void) G_GNUC_CONST;

MooObjectFactory *moo_object_factory_new_func   (MooObjectFactoryFunc factory_func,
                                                 gpointer            data);

MooObjectFactory *moo_object_factory_new        (GType               object_type,
                                                 const char         *first_prop_name,
                                                 ...);
MooObjectFactory *moo_object_factory_new_valist (GType               object_type,
                                                 const char         *first_prop_name,
                                                 va_list             var_args);
MooObjectFactory *moo_object_factory_new_a      (GType               object_type,
                                                 GParameter         *params,
                                                 guint               n_params);

GObject     *moo_object_factory_create_object   (MooObjectFactory   *factory,
                                                 gpointer            data,
                                                 const char         *additional_prop_name,
                                                 ...);


G_END_DECLS

#endif /* MOOUTILS_MOOOBJECTFACTORY_H */
