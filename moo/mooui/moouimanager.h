/*
 *   mooui/moouimanager.h
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

#ifndef MOOUI_MOOUIMANAGER_H
#define MOOUI_MOOUIMANAGER_H

#include <gtk/gtkwidget.h>
#include "mooui/mooactiongroup.h"

G_BEGIN_DECLS


#define MOO_TYPE_UI_MANAGER              (moo_ui_manager_get_type ())
#define MOO_UI_MANAGER(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_UI_MANAGER, MooUIManager))
#define MOO_UI_MANAGER_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_UI_MANAGER, MooUIManagerClass))
#define MOO_IS_UI_MANAGER(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_UI_MANAGER))
#define MOO_IS_UI_MANAGER_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_UI_MANAGER))
#define MOO_UI_MANAGER_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_UI_MANAGER, MooUIManagerClass))


typedef struct _MooUIManager        MooUIManager;
typedef struct _MooUIManagerPrivate MooUIManagerPrivate;
typedef struct _MooUIManagerClass   MooUIManagerClass;

struct _MooUIManager
{
    GObject              object;
    GtkAccelGroup       *accel_group;
    GtkTooltips         *tooltips;
};

struct _MooUIManagerClass
{
    GObjectClass parent_class;
};


GType           moo_ui_manager_get_type             (void) G_GNUC_CONST;

MooUIManager   *moo_ui_manager_new                  (void);

gboolean        moo_ui_manager_add_ui_from_string   (MooUIManager   *mgr,
                                                     const char     *xml,
                                                     int             len,
                                                     GError        **error);

GtkWidget      *moo_ui_manager_create_widget        (MooUIManager   *mgr,
                                                     const char     *path);

MooActionGroup *moo_ui_manager_get_actions          (MooUIManager   *mgr);

void            moo_ui_manager_set_accel_group      (MooUIManager   *mgr,
                                                     GtkAccelGroup  *accel_group);
GtkAccelGroup  *moo_ui_manager_get_accel_group      (MooUIManager   *mgr);

void            moo_ui_manager_set_tooltips         (MooUIManager   *mgr,
                                                     GtkTooltips    *tooltips);
GtkTooltips    *moo_ui_manager_get_tooltips         (MooUIManager   *mgr);


G_END_DECLS

#endif /* MOOUI_MOOUIMANAGER_H */

