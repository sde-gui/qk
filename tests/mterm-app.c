/*
 *   tests/mterm-app.c
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

#include "mterm-ui.h"
#include "mterm-app.h"
#include <mooutils/mooutils-misc.h>


G_DEFINE_TYPE(MooTermApp, moo_term_app, MOO_TYPE_APP)


static gboolean
on_window_close (MooApp *app)
{
    return !moo_app_quit (app);
}


static int
moo_term_app_run (MooApp *app)
{
    MooTermApp *tapp = MOO_TERM_APP (app);

    tapp->window = g_object_new (MOO_TYPE_TERM_WINDOW, NULL);
    tapp->term = tapp->window->terminal;

    moo_window_set_ui_xml (MOO_WINDOW (tapp->window),
                           moo_app_get_ui_xml (app));
    gtk_widget_show (GTK_WIDGET (tapp->window));
    g_signal_connect_swapped (tapp->window, "delete-event",
                              G_CALLBACK (on_window_close),
                              tapp);

    moo_term_start_default_shell (tapp->term, NULL);

    return MOO_APP_CLASS(moo_term_app_parent_class)->run (app);
}


static gboolean
moo_term_app_try_quit (MooApp *app)
{
    return MOO_APP_CLASS(moo_term_app_parent_class)->try_quit (app);
}


static void
moo_term_app_class_init (MooTermAppClass *klass)
{
    MooAppClass *app_class = MOO_APP_CLASS (klass);
    app_class->run = moo_term_app_run;
    app_class->try_quit = moo_term_app_try_quit;
}


static void
moo_term_app_init (MooTermApp *app)
{
}


int main (int argc, char *argv[])
{
    MooApp *app;
    MooUIXML *xml;

    gtk_init (&argc, &argv);
//     gdk_window_set_debug_updates (TRUE);

    moo_set_log_func_window (TRUE);

    app = g_object_new (MOO_TYPE_TERM_APP,
                        "argv", argv,
                        "short-name", "mterm",
                        "full-name", "mterm",
                        "description", "mterm is a terminal emulator app",
                        NULL);

    xml = moo_app_get_ui_xml (app);
    moo_ui_xml_add_ui_from_string (xml, MTERM_UI, -1);

    if (!moo_app_init (app))
        return 0;

    return moo_app_run (app);
}
