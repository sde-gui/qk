/*
 *   moolangmgr.h
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

#ifndef __MOO_LANG_MGR_H__
#define __MOO_LANG_MGR_H__

#include <mooedit/moolang.h>

G_BEGIN_DECLS


#define MOO_LANG_DIR_BASENAME   "syntax"
#define MOO_STYLES_PREFS_PREFIX MOO_EDIT_PREFS_PREFIX "/styles"

#define MOO_TYPE_LANG_MGR              (moo_lang_mgr_get_type ())
#define MOO_LANG_MGR(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_LANG_MGR, MooLangMgr))
#define MOO_LANG_MGR_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_LANG_MGR, MooLangMgrClass))
#define MOO_IS_LANG_MGR(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_LANG_MGR))
#define MOO_IS_LANG_MGR_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_LANG_MGR))
#define MOO_LANG_MGR_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_LANG_MGR, MooLangMgrClass))


typedef struct _MooLangMgr MooLangMgr;

GType           moo_lang_mgr_get_type               (void) G_GNUC_CONST;

MooLangMgr     *moo_lang_mgr_new                    (void);

/* list must be freed */
GSList         *moo_lang_mgr_get_available_langs    (MooLangMgr         *mgr);
/* list must be freed together with content */
GSList         *moo_lang_mgr_get_sections           (MooLangMgr         *mgr);

MooLang        *moo_lang_mgr_get_lang_for_file      (MooLangMgr         *mgr,
                                                     const char         *filename);
MooLang        *moo_lang_mgr_get_lang_for_filename  (MooLangMgr         *mgr,
                                                     const char         *filename);
MooLang        *moo_lang_mgr_get_lang_for_mime_type (MooLangMgr         *mgr,
                                                     const char         *mime_type);
MooLang        *moo_lang_mgr_get_lang               (MooLangMgr         *mgr,
                                                     const char         *name);

GSList         *moo_lang_mgr_list_schemes           (MooLangMgr         *mgr);

void            moo_lang_mgr_read_dirs              (MooLangMgr         *mgr);

MooTextStyleScheme *moo_lang_mgr_get_active_scheme  (MooLangMgr         *mgr);


G_END_DECLS

#endif /* __MOO_LANG_MGR_H__ */
