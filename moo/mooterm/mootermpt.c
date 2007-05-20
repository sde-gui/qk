/*
 *   mooterm/mootermpt.c
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
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
#include "mooterm/mooterm-private.h"
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


static void
moo_term_pt_class_init (MooTermPtClass *klass)
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


static void
moo_term_pt_init (MooTermPt *pt)
{
    pt->pending_write = g_queue_new ();
    pt->child_alive = FALSE;
    pt->alive = FALSE;
    pt->width = 80;
    pt->height = 24;
    pt->echo = TRUE;
    pt->priority = PT_READER_PRIORITY;
}


static void
moo_term_pt_finalize (GObject *object)
{
    MooTermPt *pt = MOO_TERM_PT (object);

    pt_discard_pending_write (pt);
    g_queue_free (pt->pending_write);

    G_OBJECT_CLASS (moo_term_pt_parent_class)->finalize (object);
}


MooTermPt*
moo_term_pt_new (MooTermIOFunc     func,
                 gpointer          data)
{
    MooTermPt *pt;

    g_return_val_if_fail (func != NULL, NULL);

#ifdef __WIN32__
    pt = g_object_new (MOO_TYPE_TERM_PT_CYG, NULL);
#else /* !__WIN32__ */
    pt = g_object_new (MOO_TYPE_TERM_PT_UNIX, NULL);
#endif /* !__WIN32__ */

    pt->io_func = func;
    pt->io_func_data = data;

    return pt;
}

void
moo_term_pt_set_priority (MooTermPt *pt,
                          int        priority)
{
    g_return_if_fail (MOO_IS_TERM_PT (pt));
    pt->priority = priority;
}

void
_moo_term_pt_set_io_size_func (MooTermPt      *pt,
                               MooTermIOSizeFunc size_func,
                               gpointer        data)
{
    g_return_if_fail (MOO_IS_TERM_PT (pt));
    pt->size_func = size_func;
    pt->size_func_data = data;
}


gsize
_moo_term_pt_get_input_chunk_len (MooTermPt *pt,
                                  gsize      max_len)
{
    gsize len;

    if (pt->size_func)
        len = pt->size_func (pt->size_func_data);
    else
        len = 1024;

    return MIN (len, max_len);
}


void
_moo_term_pt_process_data (MooTermPt  *pt,
                           const char *data,
                           int         len)
{
    g_object_ref (pt);
    if (len < 0)
        len = strlen (data);
    pt->io_func (data, len, pt->io_func_data);
    g_object_unref (pt);
}


void
_moo_term_pt_set_size (MooTermPt      *pt,
                       guint           width,
                       guint           height)
{
    g_return_if_fail (MOO_IS_TERM_PT (pt));
    MOO_TERM_PT_GET_CLASS(pt)->set_size (pt, width, height);
}

void
moo_term_pt_set_echo_input (MooTermPt *pt,
                            gboolean   echo)
{
    g_return_if_fail (MOO_IS_TERM_PT (pt));
    g_return_if_fail (MOO_TERM_PT_GET_CLASS(pt)->set_echo_input != NULL);
    MOO_TERM_PT_GET_CLASS(pt)->set_echo_input (pt, echo);
}


gboolean
moo_term_pt_fork_command (MooTermPt      *pt,
                          const MooTermCommand *cmd,
                          GError        **error)
{
    MooTermCommand *copy;
    gboolean result;

    g_return_val_if_fail (MOO_IS_TERM_PT (pt), FALSE);
    g_return_val_if_fail (cmd != NULL, FALSE);

    copy = moo_term_command_copy (cmd);
    result = _moo_term_check_cmd (copy, error);

    if (!result)
    {
        moo_term_command_free (copy);
        return FALSE;
    }

    result = MOO_TERM_PT_GET_CLASS(pt)->fork_command (pt, copy, error);

    moo_term_command_free (copy);
    return result;
}


char
_moo_term_pt_get_erase_char (MooTermPt *pt)
{
    g_return_val_if_fail (MOO_IS_TERM_PT (pt), 0);
    return MOO_TERM_PT_GET_CLASS(pt)->get_erase_char (pt);
}


void
moo_term_pt_send_intr (MooTermPt *pt)
{
    g_return_if_fail (MOO_IS_TERM_PT (pt));
    MOO_TERM_PT_GET_CLASS(pt)->send_intr (pt);
}


void
moo_term_pt_kill_child (MooTermPt *pt)
{
    g_return_if_fail (MOO_IS_TERM_PT (pt));
    MOO_TERM_PT_GET_CLASS(pt)->kill_child (pt);
}


void
moo_term_pt_write (MooTermPt      *pt,
                   const char     *data,
                   gssize          len)
{
    g_return_if_fail (MOO_IS_TERM_PT (pt));
    g_return_if_fail (data != NULL);
    MOO_TERM_PT_GET_CLASS(pt)->write (pt, data, len);
}


gboolean
moo_term_pt_child_alive (MooTermPt *pt)
{
    return pt->child_alive || pt->alive;
}

gboolean
_moo_term_pt_alive (MooTermPt *pt)
{
    return pt->alive;
}


gboolean
_moo_term_pt_set_fd (MooTermPt *pt,
                     int        master)
{
    g_return_val_if_fail (MOO_IS_TERM_PT (pt), FALSE);
    g_return_val_if_fail (!pt->child_alive, FALSE);
    g_return_val_if_fail (MOO_TERM_PT_GET_CLASS(pt)->set_fd != NULL, FALSE);
    return MOO_TERM_PT_GET_CLASS(pt)->set_fd (pt, master);
}
