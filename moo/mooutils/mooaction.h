/*
 *   mooaction.h
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

#ifndef __MOO_ACTION_H__
#define __MOO_ACTION_H__

#include <mooutils/mooclosure.h>
#include <gtk/gtkwidget.h>

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

    char        *id;
    char        *name;
    char        *group_name;

    MooClosure  *closure;

    char        *default_accel;
    char        *accel_path;

    char        *stock_id;
    char        *icon_stock_id;
    char        *label;
    char        *tooltip;
    GdkPixbuf   *icon;

    guint        dead               : 1;
    guint        visible            : 1;
    guint        sensitive          : 1;
    guint        no_accel           : 1;
    guint        is_important       : 1;
};

struct _MooActionClass
{
    GObjectClass parent_class;

    /* methods */
    GtkWidget   *(*create_menu_item)    (MooAction      *action);
    GtkWidget   *(*create_tool_item)    (MooAction      *action,
                                         GtkWidget      *toolbar,
                                         int             position);

    void         (*add_proxy)           (MooAction      *action,
                                         GtkWidget      *widget);

    /* signals */
    void         (*activate)            (MooAction      *action);
    void         (*set_sensitive)       (MooAction      *action,
                                         gboolean        sensitive);
    void         (*set_visible)         (MooAction      *action,
                                         gboolean        visible);
};


GType        moo_action_get_type            (void) G_GNUC_CONST;

void         moo_action_activate            (MooAction      *action);

const char  *moo_action_get_id              (MooAction      *action);
const char  *moo_action_get_group_name      (MooAction      *action);
const char  *moo_action_get_name            (MooAction      *action);

void         moo_action_set_no_accel        (MooAction      *action,
                                             gboolean        no_accel);
gboolean     moo_action_get_no_accel        (MooAction      *action);

GtkWidget   *moo_action_create_menu_item    (MooAction      *action);
GtkWidget   *moo_action_create_tool_item    (MooAction      *action,
                                             GtkWidget      *toolbar,
                                             int             position);

void         moo_action_set_sensitive       (MooAction      *action,
                                             gboolean        sensitive);
void         moo_action_set_visible         (MooAction      *action,
                                             gboolean        visible);

const char  *moo_action_get_default_accel   (MooAction      *action);
void         _moo_action_set_accel_path     (MooAction      *action,
                                             const char     *accel_path);
const char  *_moo_action_get_accel_path     (MooAction      *action);
const char  *_moo_action_get_accel          (MooAction      *action);

const char  *moo_action_make_accel_path     (const char     *group_id,
                                             const char     *action_id);


G_END_DECLS

#endif /* __MOO_ACTION_H__ */
