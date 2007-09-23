/*
 *   mooactionbase.h
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef MOO_ACTION_BASE_H
#define MOO_ACTION_BASE_H

#include <gtk/gtkwidget.h>

G_BEGIN_DECLS


#define MOO_TYPE_ACTION_BASE                (moo_action_base_get_type ())
#define MOO_ACTION_BASE(obj)                (G_TYPE_CHECK_INSTANCE_CAST ((obj), MOO_TYPE_ACTION_BASE, MooActionBase))
#define MOO_IS_ACTION_BASE(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MOO_TYPE_ACTION_BASE))
#define MOO_ACTION_BASE_GET_CLASS(inst)     (G_TYPE_INSTANCE_GET_INTERFACE ((inst), MOO_TYPE_ACTION_BASE, MooActionBaseClass))

typedef struct _MooActionBase       MooActionBase;
typedef struct _MooActionBaseClass  MooActionBaseClass;

struct _MooActionBaseClass {
    GTypeInterface parent;

    void (*connect_proxy)    (MooActionBase *action,
                              GtkWidget     *proxy);
    void (*disconnect_proxy) (MooActionBase *action,
                              GtkWidget     *proxy);
};


GType   moo_action_base_get_type    (void) G_GNUC_CONST;


G_END_DECLS

#endif /* MOO_ACTION_BASE_H */
