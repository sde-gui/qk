/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *   mooeditlang.h
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

#ifndef __MOO_EDIT_LANG_H__
#define __MOO_EDIT_LANG_H__

#include <gtksourceview/gtksourcetag.h>

G_BEGIN_DECLS


#define MOO_TYPE_EDIT_LANG              (moo_edit_lang_get_type ())
#define MOO_EDIT_LANG(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_EDIT_LANG, MooEditLang))
#define MOO_EDIT_LANG_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_EDIT_LANG, MooEditLangClass))
#define MOO_IS_EDIT_LANG(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_EDIT_LANG))
#define MOO_IS_EDIT_LANG_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_EDIT_LANG))
#define MOO_EDIT_LANG_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_EDIT_LANG, MooEditLangClass))


typedef struct _MooEditLang        MooEditLang;
typedef struct _MooEditLangPrivate MooEditLangPrivate;
typedef struct _MooEditLangClass   MooEditLangClass;

struct _MooEditLang
{
    GObject      parent;

    MooEditLangPrivate *priv;
};

struct _MooEditLangClass
{
    GObjectClass parent_class;
};


GType                moo_edit_lang_get_type         (void) G_GNUC_CONST;

MooEditLang         *moo_edit_lang_new              (const char     *id);
MooEditLang         *moo_edit_lang_new_from_file    (const char     *filename);

const char          *moo_edit_lang_get_id           (MooEditLang    *lang);
const char          *moo_edit_lang_get_name         (MooEditLang    *lang);
const char          *moo_edit_lang_get_section      (MooEditLang    *lang);

/* The list must be freed and the tags unref'ed */
GSList              *moo_edit_lang_get_tags         (MooEditLang    *lang);
/* Does not increment reference count of the tag */
GtkSourceTag        *moo_edit_lang_get_tag          (MooEditLang    *lang,
                                                     const char     *tag_id);

gunichar             moo_edit_lang_get_escape_char  (MooEditLang    *lang);
void                 moo_edit_lang_set_escape_char  (MooEditLang    *lang,
                                                     gunichar        escape_char);
const char          *moo_edit_lang_get_brackets     (MooEditLang    *lang);

/* Should free the list (and free each string in it also). */
GSList              *moo_edit_lang_get_mime_types   (MooEditLang    *lang);
void                 moo_edit_lang_set_mime_types   (MooEditLang    *lang,
                                                     const GSList   *mime_types);
GSList              *moo_edit_lang_get_extensions   (MooEditLang    *lang);
void                 moo_edit_lang_set_extensions   (MooEditLang    *lang,
                                                     const GSList   *extensions);

/* Allocates new null-terminated array; even indexes are style ids, odd are style names */
GPtrArray           *moo_edit_lang_get_style_names  (MooEditLang    *lang);

const GtkSourceTagStyle *moo_edit_lang_get_style    (MooEditLang    *lang,
                                                     const char     *style_id);
void                 moo_edit_lang_set_style        (MooEditLang    *lang,
                                                     const char     *style_id,
                                                     const GtkSourceTagStyle *style);

/* Allocates new null-terminated array; even indexes are style ids, odd are style names */
GPtrArray           *moo_edit_lang_get_default_style_names (void);
GtkSourceTagStyle   *moo_edit_lang_get_default_style (const char *style_id);


G_END_DECLS

#endif /* __MOO_EDIT_LANG_H__ */
