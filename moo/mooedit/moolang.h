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

#include <glib-object.h>

G_BEGIN_DECLS


#define MOO_TYPE_LANG              (moo_lang_get_type ())
#define MOO_LANG(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_LANG, MooLang))
#define MOO_IS_LANG(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_LANG))

#define MOO_LANG_NONE       "none"
#define MOO_LANG_NONE_NAME  "None"

typedef struct _MooLang MooLang;

GType        moo_lang_get_type                  (void) G_GNUC_CONST;

/* accepts NULL */
const char  *_moo_lang_id                       (MooLang    *lang);
const char  *_moo_lang_display_name             (MooLang    *lang);

const char  *_moo_lang_get_line_comment         (MooLang    *lang);
const char  *_moo_lang_get_block_comment_start  (MooLang    *lang);
const char  *_moo_lang_get_block_comment_end    (MooLang    *lang);
const char  *_moo_lang_get_brackets             (MooLang    *lang);
const char  *_moo_lang_get_section              (MooLang    *lang);
gboolean     _moo_lang_get_hidden               (MooLang    *lang);

char        *_moo_lang_id_from_name             (const char *whatever);


G_END_DECLS

#endif /* __MOO_LANG_H__ */
