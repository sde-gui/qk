/*
 *   mooapp/mooapp.h
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef MOO_APP_H
#define MOO_APP_H

#include <mooedit/mooeditor.h>

G_BEGIN_DECLS


#define MOO_TYPE_APP                (moo_app_get_type ())
#define MOO_APP(object)             (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_APP, MooApp))
#define MOO_APP_CLASS(klass)        (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_APP, MooAppClass))
#define MOO_IS_APP(object)          (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_APP))
#define MOO_IS_APP_CLASS(klass)     (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_APP))
#define MOO_APP_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_APP, MooAppClass))


typedef struct _MooApp              MooApp;
typedef struct _MooAppPrivate       MooAppPrivate;
typedef struct _MooAppClass         MooAppClass;

struct _MooApp
{
    GObject          parent;
    MooAppPrivate   *priv;
};

struct _MooAppClass
{
    GObjectClass parent_class;

    gboolean    (*init)         (MooApp         *app);
    int         (*run)          (MooApp         *app);
    void        (*quit)         (MooApp         *app);

    gboolean    (*try_quit)     (MooApp         *app);

    GtkWidget*  (*prefs_dialog) (MooApp         *app);

    void        (*exec_cmd)     (MooApp         *app,
                                 char            cmd,
                                 const char     *data,
                                 guint           len);

    void        (*load_session) (MooApp         *app,
                                 MooMarkupNode  *xml);
    void        (*save_session) (MooApp         *app,
                                 MooMarkupNode  *xml);

    void        (*init_plugins) (MooApp         *app);
};


GType            moo_app_get_type               (void) G_GNUC_CONST;

MooApp          *moo_app_get_instance           (void);

gboolean         moo_app_init                   (MooApp     *app);
int              moo_app_run                    (MooApp     *app);
gboolean         moo_app_quit                   (MooApp     *app);

void             moo_app_load_session           (MooApp     *app);

MooEditor       *moo_app_get_editor             (MooApp     *app);

void             moo_app_prefs_dialog           (GtkWidget  *parent);
void             moo_app_about_dialog           (GtkWidget  *parent);

void             moo_app_system_info_dialog     (GtkWidget  *parent);
char            *moo_app_get_system_info        (MooApp     *app);

MooUIXML        *moo_app_get_ui_xml             (MooApp     *app);
void             moo_app_set_ui_xml             (MooApp     *app,
                                                 MooUIXML   *xml);

gboolean         moo_app_send_msg               (const char *pid,
                                                 const char *data,
                                                 int         len);
gboolean         moo_app_send_files             (char      **files,
                                                 guint32     line,
                                                 guint32     stamp,
                                                 const char *pid,
                                                 guint       options);
void             moo_app_open_files             (MooApp     *app,
                                                 char      **files,
                                                 guint32     line,
                                                 guint32     stamp,
                                                 guint       options);

void             moo_app_reload_python_plugins  (void);

const char      *moo_app_get_input_pipe_name    (void);


G_END_DECLS

#endif /* MOO_APP_H */
