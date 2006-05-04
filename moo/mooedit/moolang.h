/*
 *   moolang.h
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

#ifndef __MOO_LANG_H__
#define __MOO_LANG_H__

#include <gtk/gtktexttag.h>
#include <mooedit/mootextstylescheme.h>

G_BEGIN_DECLS


#define MOO_LANG_DIR_BASENAME   "syntax"
#define MOO_STYLES_PREFS_PREFIX MOO_EDIT_PREFS_PREFIX "/styles"

#define MOO_LANG_NONE "none"

#define MOO_TYPE_LANG       (moo_lang_get_type ())


typedef struct _MooLang MooLang;


GType       moo_lang_get_type                   (void) G_GNUC_CONST;

MooLang    *moo_lang_ref                        (MooLang            *lang);
void        moo_lang_unref                      (MooLang            *lang);

char       *moo_lang_id_from_name               (const char         *name);
/* MOO_LANG_NONE if lang == NULL */
const char *moo_lang_id                         (MooLang            *lang);

/* result of these two must not be modified */
GSList     *moo_lang_get_extensions             (MooLang            *lang);
GSList     *moo_lang_get_mime_types             (MooLang            *lang);


G_END_DECLS

#endif /* __MOO_LANG_H__ */
