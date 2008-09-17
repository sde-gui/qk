/*
 *   mooapp/mooapp.h
 *
 *   Copyright (C) 2004-2008 by Yevgen Muntyan <muntyan@tamu.edu>
 *
 *   This file is part of medit.  medit is free software; you can
 *   redistribute it and/or modify it under the terms of the
 *   GNU Lesser General Public License as published by the
 *   Free Software Foundation; either version 2.1 of the License,
 *   or (at your option) any later version.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with medit.  If not, see <http://www.gnu.org/licenses/>.
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


typedef struct MooApp        MooApp;
typedef struct MooAppPrivate MooAppPrivate;
typedef struct MooAppClass   MooAppClass;

struct MooApp
{
    GObject          parent;
    MooAppPrivate   *priv;
};

struct MooAppClass
{
    GObjectClass parent_class;

    gboolean    (*init)         (MooApp         *app);
    int         (*run)          (MooApp         *app);
    void        (*quit)         (MooApp         *app);

    gboolean    (*try_quit)     (MooApp         *app);

    GtkWidget*  (*prefs_dialog) (MooApp         *app);

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

char            *moo_app_get_system_info        (MooApp     *app);

MooUiXml        *moo_app_get_ui_xml             (MooApp     *app);
void             moo_app_set_ui_xml             (MooApp     *app,
                                                 MooUiXml   *xml);

gboolean         moo_app_send_msg               (const char *pid,
                                                 const char *data,
                                                 int         len);

typedef struct {
    char *uri;
    char *encoding;
    guint line : 24; /* 0 means unset */
    guint options : 7;
} MooAppFileInfo;

gboolean         moo_app_send_files             (MooAppFileInfo *files,
                                                 int             n_files,
                                                 guint32         stamp,
                                                 const char     *pid);
void             moo_app_open_files             (MooApp         *app,
                                                 MooAppFileInfo *files,
                                                 int             n_files,
                                                 guint32         stamp);


G_END_DECLS

#endif /* MOO_APP_H */
