/*
 *   mooaction.h
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

#ifndef __MOO_ACTION_H__
#define __MOO_ACTION_H__

#include <gtk/gtkactiongroup.h>
#include <mooutils/mooclosure.h>

G_BEGIN_DECLS


const char  *moo_action_get_display_name    (GtkAction      *action);
void         moo_action_set_display_name    (GtkAction      *action,
                                             const char     *name);

const char  *moo_action_get_default_accel   (GtkAction      *action);
void         moo_action_set_default_accel   (GtkAction      *action,
                                             const char     *accel);

void         moo_action_set_no_accel        (GtkAction      *action,
                                             gboolean        no_accel);
gboolean     moo_action_get_no_accel        (GtkAction      *action);

void         moo_sync_toggle_action         (GtkAction      *action,
                                             gpointer        master,
                                             const char     *prop,
                                             gboolean        invert);

void         _moo_action_set_force_accel_label (GtkAction   *action,
                                             gboolean        force);
gboolean     _moo_action_get_force_accel_label (GtkAction   *action);

void         _moo_action_set_accel_path     (GtkAction      *action,
                                             const char     *accel_path);
const char  *_moo_action_get_accel_path     (GtkAction      *action);
const char  *_moo_action_get_accel          (GtkAction      *action);

const char  *_moo_action_make_accel_path    (const char     *group_id,
                                             const char     *action_id);

gboolean     _moo_action_get_dead           (GtkAction      *action);
void         _moo_action_set_dead           (GtkAction      *action,
                                             gboolean        dead);

gboolean     _moo_action_get_has_submenu    (GtkAction      *action);
void         _moo_action_set_has_submenu    (GtkAction      *action,
                                             gboolean        dead);

void         _moo_action_set_closure        (GtkAction      *action,
                                             MooClosure     *closure);
MooClosure  *_moo_action_get_closure        (GtkAction      *action);


G_END_DECLS

#endif /* __MOO_ACTION_H__ */
