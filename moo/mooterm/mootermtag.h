/*
 *   mootermtag.h
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

#ifndef __MOO_TERM_TAG_H__
#define __MOO_TERM_TAG_H__

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
    MOO_TERM_BLACK      = 0,
    MOO_TERM_RED        = 1,
    MOO_TERM_GREEN      = 2,
    MOO_TERM_YELLOW     = 3,
    MOO_TERM_BLUE       = 4,
    MOO_TERM_MAGENTA    = 5,
    MOO_TERM_CYAN       = 6,
    MOO_TERM_WHITE      = 7
} MooTermTextColor;

#define MOO_TERM_COLOR_MAX 8

struct _MooTermTextAttr {
    MooTermTextAttrMask mask        : 6;
    MooTermTextColor    foreground  : 3;
    MooTermTextColor    background  : 3;
};

struct _MooTermTag {
    GObject parent;
    MooTermTagTable *table;
    char *name;
    MooTermTextAttr attr;
    GSList *lines;
};

struct _MooTermTagClass {
    GObjectClass  parent_class;

    void (*changed) (MooTermTag *tag);
};


GType   moo_term_text_attr_mask_get_type    (void) G_GNUC_CONST;
GType   moo_term_text_attr_get_type         (void) G_GNUC_CONST;
GType   moo_term_text_color_get_type        (void) G_GNUC_CONST;
GType   moo_term_tag_get_type               (void) G_GNUC_CONST;

MooTermTextAttr *moo_term_text_attr_new     (MooTermTextAttrMask mask,
                                             MooTermTextColor    fg,
                                             MooTermTextColor    bg);

void    moo_term_tag_set_attr               (MooTermTag         *tag,
                                             MooTermTextAttr    *attr);
void    moo_term_tag_set_attributes         (MooTermTag         *tag,
                                             MooTermTextAttrMask mask,
                                             MooTermTextColor    fg,
                                             MooTermTextColor    bg);

void    _moo_term_tag_add_line              (MooTermTag         *tag,
                                             gpointer            line);
void    _moo_term_tag_remove_line           (MooTermTag         *tag,
                                             gpointer            line);


G_END_DECLS

#endif /* __MOO_TERM_TAG_H__ */
