/*
 *   mooui/mooaction.h
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

#ifndef MOOUI_MOOACTION_H
#define MOOUI_MOOACTION_H

#include "mooutils/mooclosure.h"
#include <gtk/gtkwidget.h>
#include <gtk/gtktoolbar.h>
#include <gtk/gtkmenushell.h>

G_BEGIN_DECLS


#define MOO_TYPE_ACTION              (moo_action_get_type ())
#define MOO_ACTION(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_ACTION, MooAction))
#define MOO_ACTION_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_ACTION, MooActionClass))
#define MOO_IS_ACTION(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_ACTION))
#define MOO_IS_ACTION_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_ACTION))
#define MOO_ACTION_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_ACTION, MooActionClass))


typedef struct _MooAction        MooAction;
typedef struct _MooActionPrivate MooActionPrivate;
typedef struct _MooActionClass   MooActionClass;

struct _MooAction
{
    GObject      object;

    guint        dead               : 1;
    guint        visible            : 1;
    guint        sensitive          : 1;

    guint        constructed        : 1;

    gpointer     group;

    char        *id;
    char        *group_id;
    char        *name;

    MooClosure  *closure;

    guint        no_accel           : 1;
    char        *accel;
    char        *default_accel;

    char        *stock_id;
    char        *icon_stock_id;
    char        *label;
    char        *tooltip;
    GdkPixbuf   *icon;
};

struct _MooActionClass
{
    GObjectClass parent_class;

    /* methods */
    GtkWidget   *(*create_menu_item)    (MooAction      *action);
    GtkWidget   *(*create_tool_item)    (MooAction      *action,
                                         GtkToolbar     *toolbar,
                                         int             position);

    void         (*add_proxy)           (MooAction      *action,
                                         GtkWidget      *widget);

    /* signals */
    void         (*activate)            (MooAction      *action);
    void         (*set_sensitive)       (MooAction      *action,
                                         gboolean        sensitive);
    void         (*set_visible)         (MooAction      *action,
                                         gboolean        sensitive);
};


GType        moo_action_get_type            (void) G_GNUC_CONST;

MooAction   *moo_action_new                 (const char     *id,
                                             const char     *label,
                                             const char     *tooltip,
                                             const char     *accel,
                                             GCallback       func,
                                             gpointer        data);
MooAction   *moo_action_new_stock           (const char     *id,
                                             const char     *stock_id,
                                             GCallback       func,
                                             gpointer        data);

void         moo_action_activate            (MooAction      *action);

const char  *moo_action_get_id              (MooAction      *action);
const char  *moo_action_get_group_id        (MooAction      *action);
const char  *moo_action_get_name            (MooAction      *action);

const char  *moo_action_get_accel_path      (MooAction      *action);

void         moo_action_set_no_accel        (MooAction      *action,
                                             gboolean        no_accel);
gboolean     moo_action_get_no_accel        (MooAction      *action);

void         moo_action_set_accel           (MooAction      *action,
                                             const char     *accel);
void         moo_action_set_default_accel   (MooAction      *action,
                                             const char     *accel);
const char  *moo_action_get_accel           (MooAction      *action);
const char  *moo_action_get_default_accel   (MooAction      *action);
char        *moo_action_get_accel_label     (MooAction      *action);
char        *moo_action_get_default_accel_label (MooAction      *action);

GtkWidget   *moo_action_create_menu_item    (MooAction      *action);
GtkWidget   *moo_action_create_tool_item    (MooAction      *action,
                                             GtkToolbar     *toolbar,
                                             int             position);

void         moo_action_set_sensitive       (MooAction      *action,
                                             gboolean        sensitive);
void         moo_action_set_visible         (MooAction      *action,
                                             gboolean        visible);

const char  *moo_action_make_accel_path     (const char     *group_id,
                                             const char     *action_id);


G_END_DECLS

#endif /* MOOUI_MOOACTION_H */

