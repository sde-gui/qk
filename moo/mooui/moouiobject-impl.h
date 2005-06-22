/*
 *   mooui/moouiobject-impl.h
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

#ifndef MOOUI_MOOUIOBJECT_IMPL_H
#define MOOUI_MOOUIOBJECT_IMPL_H

#include "mooui/moouiobject.h"

G_BEGIN_DECLS


void        moo_ui_object_class_init        (GObjectClass   *klass,
                                             const char     *id,
                                             const char     *name);

void        moo_ui_object_init              (MooUIObject    *object);

void        moo_ui_object_add_action        (MooUIObject    *object,
                                             MooAction      *action);

// void             _moo_ui_object_base_init           (gpointer iface);
// 
// void             _moo_ui_object_class_init          (GObjectClass   *klass,
//                                                      const char     *class_id);
// 
// void             _moo_ui_object_add_class_actions   (MooUIObject    *object);
// 
// void             _moo_ui_object_set_id              (MooUIObject    *object,
//                                                      const char     *id);


G_END_DECLS

#endif /* MOOUI_MOOUIOBJECT_IMPL_H */
