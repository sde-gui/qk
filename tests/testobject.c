/*
 *   testobject.c
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   See COPYING file that comes with this distribution.
 */

#include <gtk/gtk.h>
#include "mooutils/mooclosure.h"


static void test_ptr (void);
static void test_closure (void);


int
main (int argc, char *argv[])
{
    gtk_init (&argc, &argv);

    test_ptr ();
    test_closure ();

    return 0;
}


static void
do_something (gpointer object)
{
    g_assert (G_IS_OBJECT (object));
    g_object_unref (object);
}

static void
do_something2 (gpointer object)
{
    g_object_ref (object);
    g_object_unref (object);
}

static void
test_closure (void)
{
    MooClosure *cl;
    gpointer object;

    object = gtk_text_buffer_new (NULL);
    cl = _moo_closure_new_simple (object, NULL,
                                  G_CALLBACK (do_something),
                                  NULL);
    moo_closure_invoke (cl);
    moo_closure_invoke (cl);
    moo_closure_unref (cl);

    object = gtk_text_buffer_new (NULL);
    cl = _moo_closure_new_simple (object, NULL,
                                  G_CALLBACK (do_something2),
                                  NULL);
    moo_closure_invoke (cl);
    g_object_unref (object);
    moo_closure_invoke (cl);
    moo_closure_unref (cl);
}


static void
object_died (gboolean *died_or_not)
{
    g_assert (!*died_or_not);
    *died_or_not = TRUE;
}

static void
test_ptr (void)
{
    MooObjectPtr *ptr;
    gpointer object;
    static gboolean really_died = FALSE;

    object = gtk_text_buffer_new (NULL);
    ptr = _moo_object_ptr_new (object, (GWeakNotify) object_died, &really_died);
    g_object_unref (object);

    if (!really_died)
        g_error ("oops");

    _moo_object_ptr_free (ptr);
}
