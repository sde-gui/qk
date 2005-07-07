/*
 *   mooterm/mootermpt.c
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

#define MOOTERM_COMPILATION
#include "mooterm/mooterm-private.h"
#include "mooutils/moomarshals.h"
#include "mooutils/moocompat.h"


static void     moo_term_pt_set_property    (GObject        *object,
                                             guint           prop_id,
                                             const GValue   *value,
                                             GParamSpec     *pspec);
static void     moo_term_pt_get_property    (GObject        *object,
                                             guint           prop_id,
                                             GValue         *value,
                                             GParamSpec     *pspec);
static void     moo_term_pt_finalize        (GObject        *object);


/* MOO_TYPE_TERM_PT */
G_DEFINE_TYPE (MooTermPt, moo_term_pt, G_TYPE_OBJECT)

enum {
    CHILD_DIED,
    LAST_SIGNAL
};

enum {
    PROP_0,
    PROP_BUFFER
};

static guint signals[LAST_SIGNAL];


static void moo_term_pt_class_init (MooTermPtClass *klass)
{
    GObjectClass   *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->set_property = moo_term_pt_set_property;
    gobject_class->get_property = moo_term_pt_get_property;
    gobject_class->finalize = moo_term_pt_finalize;

    klass->set_size = NULL;
    klass->fork_command = NULL;
    klass->write = NULL;
    klass->kill_child = NULL;
    klass->child_died = NULL;

    signals[CHILD_DIED] =
            g_signal_new ("child-died",
                          G_OBJECT_CLASS_TYPE (gobject_class),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooTermPtClass, child_died),
                          NULL, NULL,
                          _moo_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);

    g_object_class_install_property (gobject_class,
                                     PROP_BUFFER,
                                     g_param_spec_object ("buffer",
                                             "buffer",
                                             "buffer",
                                             MOO_TYPE_TERM_BUFFER,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
}


static void     moo_term_pt_init            (MooTermPt      *pt)
{
    pt->priv = g_new0 (MooTermPtPrivate, 1);
    pt->priv->pending_write = g_queue_new ();
}


static void     moo_term_pt_finalize        (GObject            *object)
{
    MooTermPt *pt = MOO_TERM_PT (object);

    if (pt->priv->buffer)
        g_object_unref (pt->priv->buffer);

    pt_flush_pending_write (pt);
    g_queue_free (pt->priv->pending_write);

    g_free (pt->priv);

    G_OBJECT_CLASS (moo_term_pt_parent_class)->finalize (object);
}


static void     moo_term_pt_set_property    (GObject        *object,
                                             guint           prop_id,
                                             const GValue   *value,
                                             GParamSpec     *pspec)
{
    MooTermPt *pt = MOO_TERM_PT (object);

    switch (prop_id) {
        case PROP_BUFFER:
            moo_term_pt_set_buffer (pt, g_value_get_object (value));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}


static void     moo_term_pt_get_property    (GObject        *object,
                                             guint           prop_id,
                                             GValue         *value,
                                             GParamSpec     *pspec)
{
    MooTermPt *pt = MOO_TERM_PT (object);

    switch (prop_id) {
        case PROP_BUFFER:
            g_value_set_object (value, pt->priv->buffer);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}


void            moo_term_pt_set_buffer      (MooTermPt      *pt,
                                             MooTermBuffer  *buffer)
{
    if (pt->priv->buffer == buffer)
        return;

    if (pt->priv->buffer)
        g_object_unref (pt->priv->buffer);
    pt->priv->buffer = buffer;
    if (pt->priv->buffer)
        g_object_ref (pt->priv->buffer);

    g_object_notify (G_OBJECT (pt), "buffer");
}


MooTermBuffer  *moo_term_pt_get_buffer      (MooTermPt      *pt)
{
    return pt->priv->buffer;
}


MooTermPt      *moo_term_pt_new         (void)
{
#ifdef __WIN32__
    return g_object_new (MOO_TYPE_TERM_PT_WIN, NULL);
#else /* !__WIN32__ */
    return g_object_new (MOO_TYPE_TERM_PT_UNIX, NULL);
#endif /* !__WIN32__ */
}


void            moo_term_pt_set_size        (MooTermPt      *pt,
                                             guint           width,
                                             guint           height)
{
    g_return_if_fail (MOO_IS_TERM_PT (pt));
    MOO_TERM_PT_GET_CLASS(pt)->set_size (pt, width, height);
}


gboolean        moo_term_pt_fork_command    (MooTermPt      *pt,
                                             const char     *cmd,
                                             const char     *working_dir,
                                             char          **envp)
{
    g_return_val_if_fail (MOO_IS_TERM_PT (pt), FALSE);
    return MOO_TERM_PT_GET_CLASS(pt)->fork_command (pt, cmd, working_dir, envp);
}


void            moo_term_pt_kill_child      (MooTermPt      *pt)
{
    g_return_if_fail (MOO_IS_TERM_PT (pt));
    MOO_TERM_PT_GET_CLASS(pt)->kill_child (pt);
}


void            moo_term_pt_write           (MooTermPt      *pt,
                                             const char     *data,
                                             gssize          len)
{
    g_return_if_fail (MOO_IS_TERM_PT (pt));
    MOO_TERM_PT_GET_CLASS(pt)->write (pt, data, len);
}
