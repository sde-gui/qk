/*
 *   moowindow.h
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

#ifndef MOOUI_MOOWINDOW_H
#define MOOUI_MOOWINDOW_H

#include <mooutils/mooutils-gobject.h>
#include <mooutils/moouixml.h>
#include <mooutils/mooactioncollection.h>
#include <gtk/gtkwindow.h>

G_BEGIN_DECLS


#define MOO_TYPE_WINDOW              (moo_window_get_type ())
#define MOO_WINDOW(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_WINDOW, MooWindow))
#define MOO_WINDOW_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_WINDOW, MooWindowClass))
#define MOO_IS_WINDOW(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_WINDOW))
#define MOO_IS_WINDOW_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_WINDOW))
#define MOO_WINDOW_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_WINDOW, MooWindowClass))


typedef struct _MooWindow        MooWindow;
typedef struct _MooWindowPrivate MooWindowPrivate;
typedef struct _MooWindowClass   MooWindowClass;

struct _MooWindow
{
    GtkWindow            gtkwindow;

    GtkAccelGroup       *accel_group;
    MooWindowPrivate    *priv;

    GtkWidget           *menubar;
    GtkWidget           *toolbar;
    GtkWidget           *vbox;
};

struct _MooWindowClass
{
    GtkWindowClass      parent_class;

    /* signals */
    gboolean (*close)   (MooWindow *window);
};

typedef GtkAction *(*MooWindowActionFunc) (MooWindow *window,
                                           gpointer   data);


GType       moo_window_get_type             (void) G_GNUC_CONST;

gboolean    moo_window_close                (MooWindow          *window);


/*****************************************************************************/
/* Actions
 */

void        moo_window_class_set_id         (MooWindowClass     *klass,
                                             const char         *id,
                                             const char         *name);

void        moo_window_class_new_action     (MooWindowClass     *klass,
                                             const char         *id,
                                             const char         *group,
                                             const char         *first_prop_name,
                                             ...) G_GNUC_NULL_TERMINATED;
void        moo_window_class_new_action_custom (MooWindowClass  *klass,
                                             const char         *id,
                                             const char         *group,
                                             MooWindowActionFunc func,
                                             gpointer            data,
                                             GDestroyNotify      notify);
void        _moo_window_class_new_action_callback
                                            (MooWindowClass     *klass,
                                             const char         *id,
                                             const char         *group,
                                             GCallback           callback,
                                             GSignalCMarshaller  marshal,
                                             GType               return_type,
                                             guint               n_args,
                                             ...) G_GNUC_NULL_TERMINATED;

gboolean    moo_window_class_find_action    (MooWindowClass     *klass,
                                             const char         *id);
void        moo_window_class_remove_action  (MooWindowClass     *klass,
                                             const char         *id);

void        moo_window_class_new_group      (MooWindowClass     *klass,
                                             const char         *name,
                                             const char         *display_name);
gboolean    moo_window_class_find_group     (MooWindowClass     *klass,
                                             const char         *name);
void        moo_window_class_remove_group   (MooWindowClass     *klass,
                                             const char         *name);

MooUIXML   *moo_window_get_ui_xml           (MooWindow          *window);
void        moo_window_set_ui_xml           (MooWindow          *window,
                                             MooUIXML           *xml);

MooActionCollection *moo_window_get_actions (MooWindow          *window);
GtkAction  *moo_window_get_action           (MooWindow          *window,
                                             const char         *action);


G_END_DECLS

#endif /* MOOUI_MOOWINDOW_H */
