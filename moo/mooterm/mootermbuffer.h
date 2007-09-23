/*
 *   mootermbuffer.h
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef MOO_TERM_BUFFER_H
#define MOO_TERM_BUFFER_H

#include <glib-object.h>

G_BEGIN_DECLS


#define MOO_TYPE_TERM_BUFFER            (moo_term_buffer_get_type ())
#define MOO_TERM_BUFFER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), MOO_TYPE_TERM_BUFFER, MooTermBuffer))
#define MOO_TERM_BUFFER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  MOO_TYPE_TERM_BUFFER, MooTermBufferClass))
#define MOO_IS_TERM_BUFFER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MOO_TYPE_TERM_BUFFER))
#define MOO_IS_TERM_BUFFER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  MOO_TYPE_TERM_BUFFER))
#define MOO_TERM_BUFFER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  MOO_TYPE_TERM_BUFFER, MooTermBufferClass))

typedef struct _MooTermBuffer           MooTermBuffer;
typedef struct _MooTermBufferPrivate    MooTermBufferPrivate;
typedef struct _MooTermBufferClass      MooTermBufferClass;

struct _MooTermBuffer {
    GObject     parent;

    MooTermBufferPrivate *priv;
};

struct _MooTermBufferClass {
    GObjectClass  parent_class;

    void (*full_reset)          (MooTermBuffer  *buf);
};


GType   moo_term_buffer_get_type            (void) G_GNUC_CONST;

MooTermBuffer  *moo_term_buffer_new         (guint width,
                                             guint height);

/* chars must be valid utf8 */
void    moo_term_buffer_print_chars         (MooTermBuffer  *buf,
                                             const char     *chars,
                                             int             len);
void    moo_term_buffer_print_unichar       (MooTermBuffer  *buf,
                                             gunichar        c);


G_END_DECLS

#endif /* MOO_TERM_BUFFER_H */
