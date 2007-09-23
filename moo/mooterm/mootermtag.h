/*
 *   mootermtag.h
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef MOO_TERM_TAG_H
#define MOO_TERM_TAG_H

#include <glib-object.h>

G_BEGIN_DECLS


#define MOO_TYPE_TERM_TEXT_ATTR         (moo_term_text_attr_get_type ())
#define MOO_TYPE_TERM_TEXT_ATTR_MASK    (moo_term_text_attr_mask_get_type ())
#define MOO_TYPE_TERM_TEXT_COLOR        (moo_term_text_color_get_type ())

#define MOO_TYPE_TERM_TAG               (moo_term_tag_get_type ())
#define MOO_TERM_TAG(obj)               (G_TYPE_CHECK_INSTANCE_CAST ((obj), MOO_TYPE_TERM_TAG, MooTermTag))
#define MOO_TERM_TAG_CLASS(klass)       (G_TYPE_CHECK_CLASS_CAST ((klass),  MOO_TYPE_TERM_TAG, MooTermTagClass))
#define MOO_IS_TERM_TAG(obj)            (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MOO_TYPE_TERM_TAG))
#define MOO_IS_TERM_TAG_CLASS(klass)    (G_TYPE_CHECK_CLASS_TYPE ((klass),  MOO_TYPE_TERM_TAG))
#define MOO_TERM_TAG_GET_CLASS(obj)     (G_TYPE_INSTANCE_GET_CLASS ((obj),  MOO_TYPE_TERM_TAG, MooTermTagClass))

typedef struct _MooTermTextAttr      MooTermTextAttr;
typedef struct _MooTermTag           MooTermTag;
typedef struct _MooTermTagClass      MooTermTagClass;
typedef struct _MooTermTagTable      MooTermTagTable;

typedef enum {
    MOO_TERM_TEXT_REVERSE       = 1 << 0,
    MOO_TERM_TEXT_BLINK         = 1 << 1,
    MOO_TERM_TEXT_BOLD          = 1 << 2,
    MOO_TERM_TEXT_UNDERLINE     = 1 << 3,
    MOO_TERM_TEXT_FOREGROUND    = 1 << 4,
    MOO_TERM_TEXT_BACKGROUND    = 1 << 5
} MooTermTextAttrMask;

typedef enum {
    MOO_TERM_BLACK,
    MOO_TERM_RED,
    MOO_TERM_GREEN,
    MOO_TERM_YELLOW,
    MOO_TERM_BLUE,
    MOO_TERM_MAGENTA,
    MOO_TERM_CYAN,
    MOO_TERM_WHITE
} MooTermTextColor;

#define MOO_TERM_NUM_COLORS 8

struct _MooTermTextAttr {
    guint mask        : 6; /* MooTermTextAttrMask */
    guint foreground  : 3; /* MooTermTextColor */
    guint background  : 3; /* MooTermTextColor */
};


GType   moo_term_text_attr_mask_get_type        (void) G_GNUC_CONST;
GType   moo_term_text_attr_get_type             (void) G_GNUC_CONST;
GType   moo_term_text_color_get_type            (void) G_GNUC_CONST;
GType   moo_term_tag_get_type                   (void) G_GNUC_CONST;

MooTermTextAttr *moo_term_text_attr_new         (MooTermTextAttrMask mask,
                                                 MooTermTextColor    fg,
                                                 MooTermTextColor    bg);

void             moo_term_tag_set_attr          (MooTermTag         *tag,
                                                 MooTermTextAttr    *attr);
void             moo_term_tag_set_attributes    (MooTermTag         *tag,
                                                 MooTermTextAttrMask mask,
                                                 MooTermTextColor    fg,
                                                 MooTermTextColor    bg);

const char      *moo_term_tag_get_name          (MooTermTag         *tag);
const MooTermTextAttr *moo_term_tag_get_attr    (MooTermTag         *tag);


G_END_DECLS

#endif /* MOO_TERM_TAG_H */
