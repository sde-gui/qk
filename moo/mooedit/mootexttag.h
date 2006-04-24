/*
 *   mootexttag.h
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

#ifndef MOOEDIT_COMPILATION
#error "This file may not be included"
#endif

#ifndef __MOO_TEXT_TAG_H__
#define __MOO_TEXT_TAG_H__

#include <gtk/gtktextview.h>

G_BEGIN_DECLS


#define MOO_TYPE_TEXT_TAG_TYPE         (_moo_text_tag_type_get_type ())
#define MOO_TYPE_TEXT_TAG              (_moo_text_tag_get_type ())
#define MOO_TEXT_TAG(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_TEXT_TAG, MooTextTag))
#define MOO_TEXT_TAG_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_TEXT_TAG, MooTextTagClass))
#define MOO_IS_TEXT_TAG(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_TEXT_TAG))
#define MOO_IS_TEXT_TAG_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_TEXT_TAG))
#define MOO_TEXT_TAG_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_TEXT_TAG, MooTextTagClass))

#define MOO_TEXT_UNKNOWN_CHAR   0xFFFC
#define MOO_TEXT_UNKNOWN_CHAR_S "\xEF\xBF\xBC"

#define MOO_PLACEHOLDER_START   "moo-placeholder-start"
#define MOO_PLACEHOLDER_END     "moo-placeholder-end"

typedef struct _MooTextTag          MooTextTag;
typedef struct _MooTextTagPrivate   MooTextTagPrivate;
typedef struct _MooTextTagClass     MooTextTagClass;

typedef enum {
    MOO_TEXT_TAG_PLACEHOLDER_START,
    MOO_TEXT_TAG_PLACEHOLDER_END
} MooTextTagType;

struct _MooTextTag
{
    GtkTextTag parent;
    MooTextTagType type;
};

struct _MooTextTagClass
{
    GtkTextTagClass parent_class;
};


GType       _moo_text_tag_get_type              (void) G_GNUC_CONST;
GType       _moo_text_tag_type_get_type         (void) G_GNUC_CONST;

GtkTextTag *_moo_text_tag_new                   (MooTextTagType      type,
                                                 const char         *name);

MooTextTag *_moo_text_tag_at_iter               (const GtkTextIter  *iter);
gboolean    _moo_text_iter_is_placeholder_start (const GtkTextIter  *iter);
gboolean    _moo_text_iter_is_placeholder_end   (const GtkTextIter  *iter);


G_END_DECLS

#endif /* __MOO_TEXT_TAG_H__ */
