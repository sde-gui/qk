/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   mooeditlang-private.h
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

#ifndef __MOO_EDIT_LANG_PRIVATE_H__
#define __MOO_EDIT_LANG_PRIVATE_H__

#ifndef MOOEDIT_COMPILATION
#error "Do not include this file"
#endif

#include "mooedit/mooeditlang.h"

G_BEGIN_DECLS

/* TODO */
#define MOO_EDIT_LANG_SECTION_OTHERS "Others"

struct _MooEditLangPrivate {
    char        *id;
    char        *name;
    char        *section;

    char        *author;

    char        *filename;
    guint        description_loaded : 1;
    guint        loaded             : 1;

    GSList      *mime_types;        /* char* */
    GSList      *extensions;        /* char* */

    gunichar     escape_char;

    char        *brackets;

    gunichar    *word_chars;
    guint        num_word_chars;

    GSList      *tags;                      /* GtkSourceTag* */
    GHashTable  *tag_id_to_style_id;        /* char* -> char* */
    GHashTable  *style_id_to_tags;          /* char* -> GPtrArray* */
    GHashTable  *style_id_to_style;         /* char* -> GtkSourceTagStyle* */
    GHashTable  *style_id_to_style_name;    /* char* -> char* */
};


gboolean    moo_edit_lang_load_description  (MooEditLang *lang);
gboolean    moo_edit_lang_load_full         (MooEditLang *lang);


G_END_DECLS

#endif /* __MOO_EDIT_LANG_PRIVATE_H__ */
