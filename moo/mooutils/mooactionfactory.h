/*
 *   mooactionfactory.h
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

#ifndef __MOO_ACTION_FACTORY_H__
#define __MOO_ACTION_FACTORY_H__

#include <mooutils/mooaction.h>

#ifndef G_GNUC_NULL_TERMINATED
#if __GNUC__ >= 4
#define G_GNUC_NULL_TERMINATED __attribute__((__sentinel__))
#else
#define G_GNUC_NULL_TERMINATED
#endif
#endif

G_BEGIN_DECLS


#define MOO_TYPE_ACTION_FACTORY              (moo_action_factory_get_type ())
#define MOO_ACTION_FACTORY(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_ACTION_FACTORY, MooActionFactory))
#define MOO_ACTION_FACTORY_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_ACTION_FACTORY, MooActionFactoryClass))
#define MOO_IS_ACTION_FACTORY(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_ACTION_FACTORY))
#define MOO_IS_ACTION_FACTORY_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_ACTION_FACTORY))
#define MOO_ACTION_FACTORY_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_ACTION_FACTORY, MooActionFactoryClass))


typedef struct _MooActionFactory        MooActionFactory;
typedef struct _MooActionFactoryClass   MooActionFactoryClass;

typedef GtkAction* (*MooActionFactoryFunc) (gpointer          data,
                                            MooActionFactory *factory);

struct _MooActionFactory
{
    GObject      object;

    MooActionFactoryFunc factory_func;
    gpointer             factory_func_data;

    GType       action_type;
    guint       n_real_props;
    GParameter *real_props;
    guint       n_fake_props;
    GParameter *fake_props;
};

struct _MooActionFactoryClass
{
    GObjectClass parent_class;
};


GType               moo_action_factory_get_type     (void) G_GNUC_CONST;

MooActionFactory   *moo_action_factory_new_func     (MooActionFactoryFunc factory_func,
                                                     gpointer            data);
MooActionFactory   *moo_action_factory_new          (GType               action_type,
                                                     const char         *first_prop_name,
                                                     ...) G_GNUC_NULL_TERMINATED;
MooActionFactory   *moo_action_factory_new_valist   (GType               object_type,
                                                     const char         *first_prop_name,
                                                     va_list             var_args);
MooActionFactory   *moo_action_factory_new_a        (GType               object_type,
                                                     GParameter         *params,
                                                     guint               n_params);

gpointer            moo_action_factory_create_action(MooActionFactory   *factory,
                                                     gpointer            data,
                                                     const char         *additional_prop_name,
                                                     ...);


GtkAction          *moo_action_group_add_action     (GtkActionGroup     *group,
                                                     const char         *action_id,
                                                     const char         *first_prop_name,
                                                     ...) G_GNUC_NULL_TERMINATED;
GParamSpec         *_moo_action_find_property       (GObjectClass       *klass,
                                                     const char         *name);
GParamSpec         *_moo_action_find_fake_property  (GObjectClass       *klass,
                                                     const char         *name);
GtkAction          *moo_action_new                  (GType               action_type,
                                                     const char         *first_prop_name,
                                                     ...);
GtkAction          *moo_action_newv                 (GType               action_type,
                                                     guint               n_parameters,
                                                     GParameter         *parameters);
GtkAction          *moo_action_new_valist           (GType               action_type,
                                                     const char         *name,
                                                     const char         *first_property_name,
                                                     va_list             var_args);


G_END_DECLS

#endif /* __MOO_ACTION_FACTORY_H__ */
