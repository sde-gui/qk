/*
 *   mooterm/mootermbuffer.h
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

#ifndef MOOTERM_MOOTERMBUFFER_H
#define MOOTERM_MOOTERMBUFFER_H

#include <gdk/gdkcolor.h>

G_BEGIN_DECLS


#define MOO_TYPE_TERM_TEXT_ATTR_MASK    (moo_term_text_attr_mask_get_type ())
#define MOO_TYPE_TERM_TEXT_ATTR         (moo_term_text_attr_get_type ())
#define MOO_TYPE_TERM_BUFFER            (moo_term_buffer_get_type ())

#define MOO_TERM_BUFFER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), MOO_TYPE_TERM_BUFFER, MooTermBuffer))
#define MOO_TERM_BUFFER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  MOO_TYPE_TERM_BUFFER, MooTermBufferClass))
#define MOO_IS_TERM_BUFFER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MOO_TYPE_TERM_BUFFER))
#define MOO_IS_TERM_BUFFER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  MOO_TYPE_TERM_BUFFER))
#define MOO_TERM_BUFFER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  MOO_TYPE_TERM_BUFFER, MooTermBufferClass))

typedef struct _MooTermTextAttr         MooTermTextAttr;
typedef struct _MooTermBuffer           MooTermBuffer;
typedef struct _MooTermBufferPrivate    MooTermBufferPrivate;
typedef struct _MooTermBufferClass      MooTermBufferClass;

typedef enum {
    MOO_TERM_TEXT_REVERSE       = 1 << 0,
    MOO_TERM_TEXT_BLINK         = 1 << 1,
    MOO_TERM_TEXT_FOREGROUND    = 1 << 2,
    MOO_TERM_TEXT_BACKGROUND    = 1 << 3,
    MOO_TERM_TEXT_BOLD          = 1 << 4,
    MOO_TERM_TEXT_UNDERLINE     = 1 << 5
} MooTermTextAttrMask;

typedef enum {
    MOO_TERM_BLACK      = 0,
    MOO_TERM_RED        = 1,
    MOO_TERM_GREEN      = 2,
    MOO_TERM_YELLOW     = 3,
    MOO_TERM_BLUE       = 4,
    MOO_TERM_MAGENTA    = 5,
    MOO_TERM_CYAN       = 6,
    MOO_TERM_WHITE      = 7
} MooTermBufferColor;

#define MOO_TERM_COLOR_MAX 8

struct _MooTermTextAttr {
    MooTermTextAttrMask mask        : 6;
    MooTermBufferColor  foreground  : 3;
    MooTermBufferColor  background  : 3;
};

struct _MooTermBuffer {
    GObject     parent;

    MooTermBufferPrivate *priv;
};

struct _MooTermBufferClass {
    GObjectClass  parent_class;

    void (*full_reset)          (MooTermBuffer  *buf);
};


GType   moo_term_text_attr_mask_get_type    (void) G_GNUC_CONST;
GType   moo_term_text_attr_get_type         (void) G_GNUC_CONST;
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

#endif /* MOOTERM_MOOTERMBUFFER_H */
