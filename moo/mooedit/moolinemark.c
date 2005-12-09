/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   moolinemark.c
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

#define MOOEDIT_COMPILATION
#include "mooedit/moolinemark.h"
#include "mooedit/mootextbuffer.h"
#include "mooedit/moolinebuffer.h"
#include "mooutils/moomarshals.h"


struct _MooLineMarkPrivate {
    GdkColor background;
    gboolean background_set;

    char *name;

    MooTextBuffer *buffer;
    Line *line;
    int line_no;
};


static void     moo_line_mark_finalize      (GObject        *object);

static void     moo_line_mark_set_property  (GObject        *object,
                                             guint           prop_id,
                                             const GValue   *value,
                                             GParamSpec     *pspec);
static void     moo_line_mark_get_property  (GObject        *object,
                                             guint           prop_id,
                                             GValue         *value,
                                             GParamSpec     *pspec);

static void     moo_line_mark_moved         (MooLineMark    *mark);
static void     moo_line_mark_changed       (MooLineMark    *mark);


enum {
    CHANGED,
    MOVED,
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];


enum {
    PROP_0,
    PROP_BACKGROUND,
    PROP_BACKGROUND_GDK,
    PROP_BACKGROUND_SET,
    PROP_BUFFER,
    PROP_LINE,
    PROP_NAME
};


/* MOO_TYPE_LINE_MARK */
G_DEFINE_TYPE (MooLineMark, moo_line_mark, G_TYPE_OBJECT)


static void
moo_line_mark_class_init (MooLineMarkClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->set_property = moo_line_mark_set_property;
    gobject_class->get_property = moo_line_mark_get_property;
    gobject_class->finalize = moo_line_mark_finalize;

    g_object_class_install_property (gobject_class,
                                     PROP_BACKGROUND,
                                     g_param_spec_string ("background",
                                             "background",
                                             "background",
                                             NULL,
                                             G_PARAM_WRITABLE));

    g_object_class_install_property (gobject_class,
                                     PROP_BACKGROUND_GDK,
                                     g_param_spec_boxed ("background-gdk",
                                             "background-gdk",
                                             "background-gdk",
                                             GDK_TYPE_COLOR,
                                             G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class,
                                     PROP_BACKGROUND_SET,
                                     g_param_spec_boolean ("background-set",
                                             "background-set",
                                             "background-set",
                                             FALSE,
                                             G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class,
                                     PROP_BUFFER,
                                     g_param_spec_object ("mark",
                                             "mark",
                                             "mark",
                                             MOO_TYPE_TEXT_BUFFER,
                                             G_PARAM_READABLE));

    g_object_class_install_property (gobject_class,
                                     PROP_LINE,
                                     g_param_spec_int ("line",
                                             "line",
                                             "line",
                                             0, 10000000, 0,
                                             G_PARAM_READABLE));

    g_object_class_install_property (gobject_class,
                                     PROP_NAME,
                                     g_param_spec_string ("name",
                                             "name",
                                             "name",
                                             NULL,
                                             G_PARAM_READWRITE));

    signals[CHANGED] =
            g_signal_new ("changed",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooLineMarkClass, changed),
                          NULL, NULL,
                          _moo_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);

    signals[MOVED] =
            g_signal_new ("moved",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooLineMarkClass, moved),
                          NULL, NULL,
                          _moo_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);
}


static void
moo_line_mark_init (MooLineMark *mark)
{
    mark->priv = g_new0 (MooLineMarkPrivate, 1);
    mark->priv->line_no = -1;
    gdk_color_parse ("0xFFF", &mark->priv->background);
}


static void
moo_line_mark_finalize (GObject *object)
{
    MooLineMark *mark = MOO_LINE_MARK (object);

    g_free (mark->priv->name);
    g_free (mark->priv);

    G_OBJECT_CLASS (moo_line_mark_parent_class)->finalize (object);
}


static void
moo_line_mark_set_property (GObject        *object,
                            guint           prop_id,
                            const GValue   *value,
                            GParamSpec     *pspec)
{
    MooLineMark *mark = MOO_LINE_MARK (object);

    switch (prop_id)
    {
        case PROP_BACKGROUND_GDK:
            moo_line_mark_set_background_gdk (mark, g_value_get_boxed (value));
            break;

        case PROP_BACKGROUND:
            moo_line_mark_set_background (mark, g_value_get_string (value));
            break;

        case PROP_BACKGROUND_SET:
            mark->priv->background_set = g_value_get_boolean (value) != 0;
            g_object_notify (object, "background-set");
            moo_line_mark_changed (mark);
            break;

        case PROP_NAME:
            moo_line_mark_set_name (mark, g_value_get_string (value));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}


static void
moo_line_mark_get_property (GObject        *object,
                            guint           prop_id,
                            GValue         *value,
                            GParamSpec     *pspec)
{
    MooLineMark *mark = MOO_LINE_MARK (object);

    switch (prop_id)
    {
        case PROP_BACKGROUND_GDK:
            g_value_set_boxed (value, &mark->priv->background);
            break;

        case PROP_BACKGROUND_SET:
            g_value_set_boolean (value, mark->priv->background_set);
            break;

        case PROP_BUFFER:
            g_value_set_object (value, mark->priv->buffer);
            break;

        case PROP_LINE:
            g_value_set_int (value, mark->priv->line_no);
            break;

        case PROP_NAME:
            g_value_set_string (value, mark->priv->name);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}


void
moo_line_mark_set_background_gdk (MooLineMark    *mark,
                                  const GdkColor *color)
{
    gboolean changed = FALSE, notify_set = FALSE, notify_bg = FALSE;

    g_return_if_fail (MOO_IS_LINE_MARK (mark));

    if (color)
    {
        if (!mark->priv->background_set)
        {
            mark->priv->background_set = TRUE;
            notify_set = TRUE;
        }

        changed = TRUE;
        notify_bg = TRUE;

        mark->priv->background = *color;
    }
    else
    {
        if (mark->priv->background_set)
        {
            mark->priv->background_set = FALSE;
            notify_set = TRUE;
            notify_bg = TRUE;
            changed = TRUE;
        }
    }

    if (notify_set || notify_bg)
    {
        g_object_freeze_notify (G_OBJECT (mark));

        if (notify_set)
            g_object_notify (G_OBJECT (mark), "background-set");

        if (notify_bg)
        {
            g_object_notify (G_OBJECT (mark), "background");
            g_object_notify (G_OBJECT (mark), "background-gdk");
        }

        g_object_thaw_notify (G_OBJECT (mark));
    }

    if (changed)
        moo_line_mark_changed (mark);
}


void
moo_line_mark_set_background (MooLineMark    *mark,
                              const char     *color)
{
    GdkColor gdk_color;

    g_return_if_fail (MOO_IS_LINE_MARK (mark));

    if (color)
    {
        if (gdk_color_parse (color, &gdk_color))
            moo_line_mark_set_background_gdk (mark, &gdk_color);
        else
            g_warning ("%s: could not parse color '%s'",
                       G_STRLOC, color);
    }
    else
    {
        moo_line_mark_set_background_gdk (mark, NULL);
    }
}


static void
moo_line_mark_moved (MooLineMark *mark)
{
    g_object_notify (G_OBJECT (mark), "line");
    g_signal_emit (mark, signals[MOVED], 0);
}


static void
moo_line_mark_changed (MooLineMark *mark)
{
    g_signal_emit (mark, signals[CHANGED], 0);
}


void
moo_line_mark_set_name (MooLineMark    *mark,
                        const char     *name)
{
    g_return_if_fail (MOO_IS_LINE_MARK (mark));

    if (mark->priv->name != name)
    {
        g_free (mark->priv->name);
        mark->priv->name = g_strdup (name);
        g_object_notify (G_OBJECT (mark), "name");
    }
}


const char *
moo_line_mark_get_name (MooLineMark *mark)
{
    g_return_val_if_fail (MOO_IS_LINE_MARK (mark), NULL);
    return mark->priv->name;
}


int
moo_line_mark_get_line (MooLineMark *mark)
{
    g_return_val_if_fail (MOO_IS_LINE_MARK (mark), -1);
    return mark->priv->line_no;
}


void
_moo_line_mark_set_line (MooLineMark    *mark,
                         gpointer        line,
                         int             line_no)
{
    gboolean moved = FALSE;

    g_assert (MOO_IS_LINE_MARK (mark));
    g_assert (mark->priv->buffer != NULL);

    if (mark->priv->line_no != line_no)
        moved = TRUE;

    mark->priv->line = line;
    mark->priv->line_no = line_no;

    if (moved)
        moo_line_mark_moved (mark);
}
