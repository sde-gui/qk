/*
 *   mooutils-script.c
 *
 *   Copyright (C) 2004-2010 by Yevgen Muntyan <emuntyan@sourceforge.net>
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

#include "config.h"
#include "mooutils-script.h"
#include "mooutils.h"

static gboolean
quit_main_loop (GMainLoop *main_loop)
{
    g_main_loop_quit (main_loop);
    return FALSE;
}

/**
 * moo_spin_main_loop:
 **/
void
moo_spin_main_loop (double sec)
{
    GMainLoop *main_loop;

    moo_return_if_fail (sec > 0);

    main_loop = g_main_loop_new (NULL, FALSE);
    _moo_timeout_add (sec * 1000, (GSourceFunc) quit_main_loop, main_loop);

    gdk_threads_leave ();
    g_main_loop_run (main_loop);
    gdk_threads_enter ();

    g_main_loop_unref (main_loop);
}
