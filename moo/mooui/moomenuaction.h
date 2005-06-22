/*
 *   mooui/moomenuaction.h
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

#ifndef MOOUI_MOOMENUACTION_H
#define MOOUI_MOOMENUACTION_H

#include "mooui/mooaction.h"
#include <gtk/gtkmenuitem.h>

G_BEGIN_DECLS


#define MOO_TYPE_MENU_ACTION              (moo_menu_action_get_type ())
#define MOO_MENU_ACTION(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_MENU_ACTION, MooMenuAction))
#define MOO_MENU_ACTION_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_MENU_ACTION, MooMenuActionClass))
#define MOO_IS_MENU_ACTION(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_MENU_ACTION))
#define MOO_IS_MENU_ACTION_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_MENU_ACTION))
#define MOO_MENU_ACTION_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_MENU_ACTION, MooMenuActionClass))


typedef struct _MooMenuAction        MooMenuAction;
typedef struct _MooMenuActionClass   MooMenuActionClass;

typedef GtkMenuItem *(*MooMenuCreationFunc) (gpointer    data,
                                             MooAction  *action);

struct _MooMenuAction
{
    MooAction           parent;

    MooMenuCreationFunc create_menu_func;
    gpointer            create_menu_data;
};

struct _MooMenuActionClass
{
    MooActionClass  parent_class;
};


GType            moo_menu_action_get_type     (void) G_GNUC_CONST;

MooAction       *moo_menu_action_new      (const char         *id,
                                           const char         *group_id,
                                           MooMenuCreationFunc create_menu,
                                           gpointer            data);


G_END_DECLS

#endif /* MOOUI_MOOMENUACTION_H */

