/*
 *   mooterm/mootermpt.c
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

#define MOOTERM_COMPILATION
#include "mooterm/mootermpt-private.h"
#include "mooterm/mooterm.h"
#include "mooutils/moomarshals.h"
#include "mooutils/moocompat.h"


static void     moo_term_pt_finalize        (GObject        *object);


/* MOO_TYPE_TERM_PT */
G_DEFINE_TYPE (MooTermPt, moo_term_pt, G_TYPE_OBJECT)

enum {
    CHILD_DIED,
    LAST_SIGNAL
};

enum {
    PROP_0
};

static guint signals[LAST_SIGNAL];


static void moo_term_pt_class_init (MooTermPtClass *klass)
{
    GObjectClass   *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = moo_term_pt_finalize;

    signals[CHILD_DIED] =
            g_signal_new ("child-died",
                          G_OBJECT_CLASS_TYPE (gobject_class),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooTermPtClass, child_died),
                          NULL, NULL,
                          _moo_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);
}


static void     moo_term_pt_init            (MooTermPt      *pt)
{
    pt->priv = g_new0 (MooTermPtPrivate, 1);
    pt->priv->pending_write = g_queue_new ();
}


static void     moo_term_pt_finalize        (GObject            *object)
{
    MooTermPt *pt = MOO_TERM_PT (object);

    pt_discard_pending_write (pt);
    g_queue_free (pt->priv->pending_write);

    g_free (pt->priv);

    G_OBJECT_CLASS (moo_term_pt_parent_class)->finalize (object);
}


MooTermPt*
_moo_term_pt_new (MooTerm    *term)
{
    MooTermPt *pt;
#ifdef __WIN32__
    pt = g_object_new (MOO_TYPE_TERM_PT_CYG, NULL);
#else /* !__WIN32__ */
    pt = g_object_new (MOO_TYPE_TERM_PT_UNIX, NULL);
#endif /* !__WIN32__ */
    pt->priv->term = term;
    return pt;
}


void
_moo_term_pt_set_size (MooTermPt      *pt,
                       guint           width,
                       guint           height)
{
    g_return_if_fail (MOO_IS_TERM_PT (pt));
    MOO_TERM_PT_GET_CLASS(pt)->set_size (pt, width, height);
}


gboolean
_moo_term_pt_fork_command (MooTermPt      *pt,
                           const MooTermCommand *cmd,
                           const char     *working_dir,
                           char          **envp,
                           GError        **error)
{
    g_return_val_if_fail (MOO_IS_TERM_PT (pt), FALSE);
    return MOO_TERM_PT_GET_CLASS(pt)->fork_command (pt, cmd, working_dir, envp, error);
}


char
_moo_term_pt_get_erase_char (MooTermPt *pt)
{
    g_return_val_if_fail (MOO_IS_TERM_PT (pt), 0);
    return MOO_TERM_PT_GET_CLASS(pt)->get_erase_char (pt);
}


void
_moo_term_pt_send_intr (MooTermPt *pt)
{
    g_return_if_fail (MOO_IS_TERM_PT (pt));
    MOO_TERM_PT_GET_CLASS(pt)->send_intr (pt);
}


void
_moo_term_pt_kill_child (MooTermPt      *pt)
{
    g_return_if_fail (MOO_IS_TERM_PT (pt));
    MOO_TERM_PT_GET_CLASS(pt)->kill_child (pt);
}


void
_moo_term_pt_write (MooTermPt      *pt,
                    const char     *data,
                    gssize          len)
{
    g_return_if_fail (MOO_IS_TERM_PT (pt));
    MOO_TERM_PT_GET_CLASS(pt)->write (pt, data, len);
}


gboolean
_moo_term_pt_child_alive (MooTermPt      *pt)
{
    return pt->priv->child_alive;
}
