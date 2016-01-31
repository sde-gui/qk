/*
 *   mooapp/mooapp.h
 *
 *   Copyright (C) 2004-2016 by Yevgen Muntyan <emuntyan@users.sourceforge.net>
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

#pragma once

#include <mooedit/mooeditor.h>

G_BEGIN_DECLS


#define MOO_TYPE_APP                (moo_app_get_type ())
#define MOO_APP(object)             (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_APP, MooApp))
#define MOO_APP_CLASS(klass)        (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_APP, MooAppClass))
#define MOO_IS_APP(object)          (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_APP))
#define MOO_IS_APP_CLASS(klass)     (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_APP))
#define MOO_APP_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_APP, MooAppClass))


typedef struct MooApp       MooApp;
typedef struct MooAppClass  MooAppClass;

struct MooApp
{
    GObject     object;
};

struct MooAppClass
{
    GObjectClass parent_class;

    /**signal:MooApp**/
    void        (*started)      (MooApp         *app);
    /**signal:MooApp**/
    void        (*quit)         (MooApp         *app);

    /**signal:MooApp**/
    void        (*load_session) (MooApp         *app);
    /**signal:MooApp**/
    void        (*save_session) (MooApp         *app);

    void        (*init_plugins) (MooApp         *app);
};


GType            moo_app_get_type               (void) G_GNUC_CONST;

MooApp          *moo_app_instance               (void);
gboolean         moo_app_quit                   (MooApp                 *app);
MooEditor       *moo_app_get_editor             (MooApp                 *app);


G_END_DECLS

#ifdef __cplusplus

#include <moocpp/moocpp.h>

MOO_DEFINE_GOBJ_TYPE(MooApp, GObject, MOO_TYPE_APP)

struct App : public moo::gobj_wrapper<MooApp, App>
{
    using Super = moo::gobj_wrapper<MooApp, App>;

public:
    struct StartupOptions
    {
        bool run_input = false;
        int use_session = -1;
        moo::gstr instance_name;
    };

    App(gobj_wrapper_data& d, const StartupOptions& opts);
    ~App();

    static App& instance();

    bool init();
    int run();

    bool quit();
    void set_exit_status(int value);

    void load_session();

    MooEditor* get_editor();

    static moo::gstr get_system_info();
    static void about_dialog(GtkWidget* parent);

    static bool send_msg(const char* pid, const char* data, gssize len);
    static bool send_files(MooOpenInfoArray* files, guint32 stamp, const char* pid);

    void open_files(MooOpenInfoArray* files, guint32 stamp);
    void run_script(const char* script);

protected:
    virtual void init_plugins() {}

private:
    struct Private;
    Private* p;
};

#endif __cplusplus
