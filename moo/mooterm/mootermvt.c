/*
 *   mooterm/mootermvt.c
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

#include "mooterm/mootermvt.h"


struct _MooTermVtPrivate {
    MooTermBuffer   *buffer;
};


static void     moo_term_vt_set_property    (GObject        *object,
                                             guint           prop_id,
                                             const GValue   *value,
                                             GParamSpec     *pspec);
static void     moo_term_vt_get_property    (GObject        *object,
                                             guint           prop_id,
                                             GValue         *value,
                                             GParamSpec     *pspec);
static void     moo_term_vt_finalize        (GObject        *object);


/* MOO_TYPE_TERM_VT */
G_DEFINE_TYPE (MooTermVt, moo_term_vt, G_TYPE_OBJECT)

enum {
    LAST_SIGNAL
};

enum {
    PROP_0,
    PROP_BUFFER
};

// static guint signals[LAST_SIGNAL];


static void moo_term_vt_class_init (MooTermVtClass *klass)
{
    GObjectClass   *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->set_property = moo_term_vt_set_property;
    gobject_class->get_property = moo_term_vt_get_property;
    gobject_class->finalize = moo_term_vt_finalize;

    g_object_class_install_property (gobject_class,
                                     PROP_BUFFER,
                                     g_param_spec_object ("buffer",
                                             "buffer",
                                             "buffer",
                                             MOO_TYPE_TERM_BUFFER,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
}


static void     moo_term_vt_init            (MooTermVt      *vt)
{
    vt->priv = g_new0 (MooTermVtPrivate, 1);
}


static void     moo_term_vt_finalize        (GObject            *object)
{
    MooTermVt *vt = MOO_TERM_VT (object);

    if (vt->priv->buffer)
        g_object_unref (vt->priv->buffer);
    g_free (vt->priv);

    G_OBJECT_CLASS (moo_term_vt_parent_class)->finalize (object);
}


static void     moo_term_vt_set_property    (GObject        *object,
                                             guint           prop_id,
                                             const GValue   *value,
                                             GParamSpec     *pspec)
{
    MooTermVt *vt = MOO_TERM_VT (object);

    switch (prop_id) {
        case PROP_BUFFER:
            moo_term_vt_set_buffer (vt, g_value_get_object (value));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}


static void     moo_term_vt_get_property    (GObject        *object,
                                             guint           prop_id,
                                             GValue         *value,
                                             GParamSpec     *pspec)
{
    MooTermVt *vt = MOO_TERM_VT (object);

    switch (prop_id) {
        case PROP_BUFFER:
            g_value_set_object (value, vt->priv->buffer);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}


void            moo_term_vt_set_buffer      (MooTermVt      *vt,
                                             MooTermBuffer  *buffer)
{
    if (vt->priv->buffer == buffer)
        return;

    if (vt->priv->buffer)
        g_object_unref (vt->priv->buffer);
    vt->priv->buffer = buffer;
    if (vt->priv->buffer)
        g_object_ref (vt->priv->buffer);

    g_object_notify (G_OBJECT (vt), "buffer");
}


MooTermBuffer  *moo_term_vt_get_buffer      (MooTermVt      *vt)
{
    return vt->priv->buffer;
}


MooTermVt      *moo_term_vt_new         (void)
{
    return MOO_TERM_VT (g_object_new (MOO_TYPE_TERM_VT, NULL));
}


void            moo_term_vt_set_size    (G_GNUC_UNUSED MooTermVt  *vt,
                                         G_GNUC_UNUSED gulong      width,
                                         G_GNUC_UNUSED gulong      height)
{
//     g_message ("%s: implement me", G_STRLOC);
}
