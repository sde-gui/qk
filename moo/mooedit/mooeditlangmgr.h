/*
 *   mooedit/mooeditlangmgr.h
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

#ifndef MOOEDIT_MOOEDITLANGMGR_H
#define MOOEDIT_MOOEDITLANGMGR_H

#include "mooedit/mooeditlang.h"

G_BEGIN_DECLS


#define MOO_TYPE_EDIT_LANG_MGR              (moo_edit_lang_mgr_get_type ())
#define MOO_EDIT_LANG_MGR(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_EDIT_LANG_MGR, MooEditLangMgr))
#define MOO_EDIT_LANG_MGR_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_EDIT_LANG_MGR, MooEditLangMgrClass))
#define MOO_IS_EDIT_LANG_MGR(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_EDIT_LANG_MGR))
#define MOO_IS_EDIT_LANG_MGR_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_EDIT_LANG_MGR))
#define MOO_EDIT_LANG_MGR_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_EDIT_LANG_MGR, MooEditLangMgrClass))


typedef struct _MooEditLangMgr        MooEditLangMgr;
typedef struct _MooEditLangMgrPrivate MooEditLangMgrPrivate;
typedef struct _MooEditLangMgrClass   MooEditLangMgrClass;

struct _MooEditLangMgr
{
    GObject      parent;

    MooEditLangMgrPrivate *priv;
};

struct _MooEditLangMgrClass
{
    GObjectClass parent_class;
};


GType            moo_edit_lang_mgr_get_type                     (void) G_GNUC_CONST;

MooEditLangMgr  *moo_edit_lang_mgr_new                          (void);

/* list should be freed together with its contents */
GSList          *moo_edit_lang_mgr_get_sections                 (MooEditLangMgr *mgr);
const GSList    *moo_edit_lang_mgr_get_available_languages      (MooEditLangMgr *mgr);
void             moo_edit_lang_mgr_add_language                 (MooEditLangMgr *mgr,
                                                                 MooEditLang    *lang);

MooEditLang     *moo_edit_lang_mgr_get_default_language         (MooEditLangMgr *mgr);
void             moo_edit_lang_mgr_set_default_language         (MooEditLangMgr *mgr,
                                                                 MooEditLang    *lang);

MooEditLang     *moo_edit_lang_mgr_get_language_by_id           (MooEditLangMgr *mgr,
                                                                 const char     *lang_id);

MooEditLang     *moo_edit_lang_mgr_get_language_for_file        (MooEditLangMgr *mgr,
                                                                 const char     *filename);
MooEditLang     *moo_edit_lang_mgr_get_language_for_mime_type   (MooEditLangMgr *mgr,
                                                                 const char     *mime_type);
MooEditLang     *moo_edit_lang_mgr_get_language_for_filename    (MooEditLangMgr *mgr,
                                                                 const char     *filename);


const GSList    *moo_edit_lang_mgr_get_lang_files_dirs          (MooEditLangMgr *mgr);
void             moo_edit_lang_mgr_add_lang_files_dir           (MooEditLangMgr *mgr,
                                                                 const char     *dir);


G_END_DECLS

#endif /* MOOEDIT_MOOEDITLANGMGR_H */
