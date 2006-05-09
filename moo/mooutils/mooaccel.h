/*
 *   mooaccel.h
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

#ifndef __MOOACCEL_H__
#define __MOOACCEL_H__

#include <gtk/gtkactiongroup.h>

G_BEGIN_DECLS


void         _moo_prefs_set_accel           (const char     *accel_path,
                                             const char     *accel);
const char  *_moo_prefs_get_accel           (const char     *accel_path);

void         _moo_set_accel                 (const char     *accel_path,
                                             const char     *accel);
const char  *_moo_get_accel                 (const char     *accel_path);
void         _moo_set_default_accel         (const char     *accel_path,
                                             const char     *accel);
const char  *_moo_get_default_accel         (const char     *accel_path);

char        *_moo_get_accel_label           (const char     *accel);
char        *_moo_get_accel_label_by_path   (const char     *accel_path);

gboolean     _moo_accel_parse               (const char     *accel,
                                             guint          *key,
                                             GdkModifierType *mods);
char        *_moo_accel_normalize           (const char     *accel);

void         _moo_accel_label_set_action    (GtkWidget      *label,
                                             GtkAction      *action);


GtkWidget   *_moo_accel_prefs_page_new      (GtkActionGroup *group);
GtkWidget   *_moo_accel_prefs_dialog_new    (GtkActionGroup *group);
void         _moo_accel_prefs_dialog_run    (GtkActionGroup *group,
                                             GtkWidget      *parent);


G_END_DECLS

#endif /* __MOOACCEL_H__ */
