/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
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
typedef struct _MooLangMgrClass MooLangMgrClass;

struct _MooLangMgr {
    GObject base;

    GSList *lang_dirs;
    GSList *langs;
    GHashTable *lang_names;
    GHashTable *schemes;
    MooTextStyleScheme *active_scheme;
    guint dirs_read : 1;
};

struct _MooLangMgrClass
{
    GObjectClass base_class;

    void (*lang_added)   (MooLangMgr *mgr,
                          const char   *lang_name);
    void (*lang_removed) (MooLangMgr *mgr,
                          const char   *lang_name);
};


GType           moo_lang_mgr_get_type               (void) G_GNUC_CONST;

MooLangMgr     *moo_lang_mgr_new                    (void);

GSList         *moo_lang_mgr_get_available_langs    (MooLangMgr         *mgr);
GSList         *moo_lang_mgr_get_sections           (MooLangMgr         *mgr);

MooLang        *moo_lang_mgr_get_lang_for_file      (MooLangMgr         *mgr,
                                                     const char         *filename);
MooLang        *moo_lang_mgr_get_lang_for_filename  (MooLangMgr         *mgr,
                                                     const char         *filename);
MooLang        *moo_lang_mgr_get_lang_for_mime_type (MooLangMgr         *mgr,
                                                     const char         *mime_type);
MooLang        *moo_lang_mgr_get_lang               (MooLangMgr         *mgr,
                                                     const char         *name);
MooContext     *moo_lang_mgr_get_context            (MooLangMgr         *mgr,
                                                     const char         *lang_name,
                                                     const char         *ctx_name);

GSList         *moo_lang_mgr_list_schemes           (MooLangMgr         *mgr);

void            moo_lang_mgr_add_dir                (MooLangMgr         *mgr,
                                                     const char         *dir);
void            moo_lang_mgr_read_dirs              (MooLangMgr         *mgr);

MooTextStyle   *moo_lang_mgr_get_style              (MooLangMgr         *mgr,
                                                     const char         *lang_name, /* default style if NULL */
                                                     const char         *style_name,
                                                     MooTextStyleScheme *scheme);
MooTextStyleScheme *moo_lang_mgr_get_active_scheme  (MooLangMgr         *mgr);
void            moo_lang_mgr_set_active_scheme      (MooLangMgr         *mgr,
                                                     const char         *scheme_name);

void            _moo_lang_mgr_add_lang              (MooLangMgr         *mgr,
                                                     MooLang            *lang);


G_END_DECLS

#endif /* __MOO_LANG_MGR_H__ */
