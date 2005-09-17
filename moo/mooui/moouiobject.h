/*
 *   mooui/moouiobject.h
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

#ifndef MOOUI_MOOUIOBJECT_H
#define MOOUI_MOOUIOBJECT_H

#include "mooui/mooactiongroup.h"
#include "mooui/mooaction.h"
#include "mooui/moouixml.h"
#include "mooutils/mooutils-gobject.h"

G_BEGIN_DECLS


#define MOO_TYPE_UI_OBJECT             (moo_ui_object_get_type ())
#define MOO_UI_OBJECT(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), MOO_TYPE_UI_OBJECT, MooUIObject))
#define MOO_IS_UI_OBJECT(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MOO_TYPE_UI_OBJECT))
#define MOO_UI_OBJECT_GET_IFACE(inst)  (G_TYPE_INSTANCE_GET_INTERFACE ((inst), MOO_TYPE_UI_OBJECT, MooUIObjectIface))


typedef struct _MooUIObject         MooUIObject;
typedef struct _MooUIObjectIface    MooUIObjectIface;

struct _MooUIObjectIface {
    GTypeInterface       parent;
};


GType            moo_ui_object_get_type         (void);

const char      *moo_ui_object_class_get_id     (GObjectClass       *klass);
const char      *moo_ui_object_class_get_name   (GObjectClass       *klass);

void             moo_ui_object_class_install_action
                                                (GObjectClass       *klass,
                                                 const char         *id,
                                                 MooObjectFactory   *action,
                                                 MooObjectFactory   *closure,
                                                 char              **conditions);
void             moo_ui_object_class_new_action (GObjectClass       *klass,
                                                 const char         *id,
                                                 const char         *first_prop_name,
                                                 ...);
void             moo_ui_object_class_new_actionv(GObjectClass       *klass,
                                                 const char         *id,
                                                 const char         *first_prop_name,
                                                 va_list             props);

void             moo_ui_object_class_remove_action (GObjectClass    *klass,
                                                 const char         *id);

MooUIXML        *moo_ui_object_get_ui_xml       (MooUIObject        *object);
void             moo_ui_object_set_ui_xml       (MooUIObject        *object,
                                                 MooUIXML           *xml);

MooActionGroup  *moo_ui_object_get_actions      (MooUIObject        *object);

char            *moo_ui_object_get_name         (MooUIObject        *object);
char            *moo_ui_object_get_id           (MooUIObject        *object);
void             moo_ui_object_set_name         (MooUIObject        *object,
                                                 const char         *name);


G_END_DECLS

#endif /* MOOUI_MOOUIOBJECT_H */
