/*
 *   mooapp/mooapp.h
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

#ifndef MOOAPP_H
#define MOOAPP_H

#include "mooterm/mootermwindow.h"
#include "mooedit/mooeditor.h"

G_BEGIN_DECLS


#define MOO_TYPE_APP                (moo_app_get_type ())
#define MOO_TYPE_APP_INFO           (moo_app_info_get_type ())
#define MOO_TYPE_APP_WINDOW_POLICY  (moo_app_window_policy_get_type ())

#define MOO_APP(object)             (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_APP, MooApp))
#define MOO_APP_CLASS(klass)        (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_APP, MooAppClass))
#define MOO_IS_APP(object)          (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_APP))
#define MOO_IS_APP_CLASS(klass)     (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_APP))
#define MOO_APP_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_APP, MooAppClass))


typedef struct _MooApp              MooApp;
typedef struct _MooAppInfo          MooAppInfo;
typedef struct _MooAppPrivate       MooAppPrivate;
typedef struct _MooAppClass         MooAppClass;

typedef enum _MooAppWindowPolicy
{
    MOO_APP_ONE_EDITOR                  = 1 << 0,
    MOO_APP_MANY_EDITORS                = 1 << 1,
    MOO_APP_USE_EDITOR = MOO_APP_ONE_EDITOR | MOO_APP_MANY_EDITORS,
    MOO_APP_ONE_TERMINAL                = 1 << 2,
    MOO_APP_MANY_TERMINALS              = 1 << 3,
    MOO_APP_USE_TERMINAL = MOO_APP_ONE_TERMINAL | MOO_APP_MANY_TERMINALS,
    MOO_APP_QUIT_ON_CLOSE_ALL_EDITORS   = 1 << 4,
    MOO_APP_QUIT_ON_CLOSE_ALL_TERMINALS = 1 << 5,
    MOO_APP_QUIT_ON_CLOSE_ALL_WINDOWS   = 1 << 6
} MooAppWindowPolicy;

struct _MooAppInfo
{
    char *short_name;
    char *full_name;
    char *description;
    char *version;
    char *website;
    char *website_label;

    char *app_dir;
    char *rc_file;
};

struct _MooApp
{
    GObject          parent;
    MooAppPrivate   *priv;
};

struct _MooAppClass
{
    GObjectClass parent_class;

    gboolean    (*init)         (MooApp *app);
    int         (*run)          (MooApp *app);
    void        (*quit)         (MooApp *app);

    gboolean    (*try_quit)     (MooApp *app);

    GtkWidget*  (*prefs_dialog) (MooApp *app);
};


GType            moo_app_get_type               (void) G_GNUC_CONST;
GType            moo_app_info_get_type          (void) G_GNUC_CONST;
GType            moo_app_window_policy_get_type (void) G_GNUC_CONST;

MooApp          *moo_app_get_instance           (void);

void             moo_app_init                   (MooApp     *app);
int              moo_app_run                    (MooApp     *app);
gboolean         moo_app_quit                   (MooApp     *app);

int              moo_app_get_exit_code          (MooApp     *app);
void             moo_app_set_exit_code          (MooApp     *app,
                                                 int         code);

const MooAppInfo*moo_app_get_info               (MooApp     *app);

const char      *moo_app_get_rc_file_name       (MooApp     *app);
const char      *moo_app_get_input_pipe_name    (MooApp     *app);
const char      *moo_app_get_application_dir    (MooApp     *app);

MooEditor       *moo_app_get_editor             (MooApp     *app);

void             moo_app_prefs_dialog           (GtkWidget  *parent);
void             moo_app_about_dialog           (GtkWidget  *parent);

MooUIXML        *moo_app_get_ui_xml             (MooApp     *app);
void             moo_app_set_ui_xml             (MooApp     *app,
                                                 MooUIXML   *xml);


G_END_DECLS

#endif /* MOOAPP_H */
