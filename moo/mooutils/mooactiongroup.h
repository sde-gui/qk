/*
 *   mooactiongroup.h
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

#ifndef __MOO_ACTION_GROUP_H__
#define __MOO_ACTION_GROUP_H__

#include <mooutils/mooactioncollection.h>

G_BEGIN_DECLS


#define MOO_TYPE_ACTION_GROUP                   (_moo_action_group_get_type ())
#define MOO_ACTION_GROUP(object)                (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_ACTION_GROUP, MooActionGroup))
#define MOO_ACTION_GROUP_CLASS(klass)           (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_ACTION_GROUP, MooActionGroupClass))
#define MOO_IS_ACTION_GROUP(object)             (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_ACTION_GROUP))
#define MOO_IS_ACTION_GROUP_CLASS(klass)        (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_ACTION_GROUP))
#define MOO_ACTION_GROUP_GET_CLASS(obj)         (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_ACTION_GROUP, MooActionGroupClass))

typedef struct _MooActionGroup             MooActionGroup;
typedef struct _MooActionGroupClass        MooActionGroupClass;

struct _MooActionGroup {
    GtkActionGroup base;
    char *display_name;
    MooActionCollection *collection;
};

struct _MooActionGroupClass {
    GtkActionGroupClass base_class;
};


GType                _moo_action_group_get_type         (void) G_GNUC_CONST;

MooActionGroup      *_moo_action_group_new              (MooActionCollection    *collection,
                                                         const char             *name,
                                                         const char             *display_name);

const char          *_moo_action_group_get_display_name (MooActionGroup         *group);
void                 _moo_action_group_set_display_name (MooActionGroup         *group,
                                                         const char             *display_name);

MooActionCollection *_moo_action_group_get_collection   (MooActionGroup         *group);
void                 _moo_action_group_set_collection   (MooActionGroup         *group,
                                                         MooActionCollection    *collection);


G_END_DECLS

#endif /* __MOO_ACTION_GROUP_H__ */
