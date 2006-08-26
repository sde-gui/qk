/*
 *   moolangmgr-private.h
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
#error "This file may not be used"
#endif

#ifndef __MOO_LANG_MGR_PRIVATE_H__
#define __MOO_LANG_MGR_PRIVATE_H__

#include "mooedit/moolangmgr.h"
#include "mooedit/mooeditconfig.h"
#include "gtksourceview/gtksourcelanguagesmanager.h"

G_BEGIN_DECLS


#define MOO_LANG_MGR_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_LANG_MGR, MooLangMgrClass))
#define MOO_IS_LANG_MGR_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_LANG_MGR))
#define MOO_LANG_MGR_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_LANG_MGR, MooLangMgrClass))

typedef struct _MooLangMgrClass MooLangMgrClass;

struct _MooLangMgr {
    GtkSourceLanguagesManager base;

    GHashTable *langs;
    GHashTable *extensions;
    GHashTable *mime_types;
    GHashTable *config;
    GHashTable *schemes;

    MooTextStyleScheme *active_scheme;

    guint got_langs : 1;
    guint got_schemes : 1;
};

struct _MooLangMgrClass
{
    GtkSourceLanguagesManagerClass base_class;
};


// void        _moo_lang_mgr_set_active_scheme     (MooLangMgr         *mgr,
//                                                  const char         *scheme_id);
// void        _moo_lang_mgr_add_lang              (MooLangMgr         *mgr,
//                                                  MooLang            *lang);
// const char *_moo_lang_mgr_get_config            (MooLangMgr         *mgr,
//                                                  const char         *lang_id);
// void        _moo_lang_mgr_set_config            (MooLangMgr         *mgr,
//                                                  const char         *lang_id,
//                                                  const char         *config);
// void        _moo_lang_mgr_update_config         (MooLangMgr         *mgr,
//                                                  MooEditConfig      *config,
//                                                  const char         *lang_id);
// void        _moo_lang_mgr_load_config           (MooLangMgr         *mgr);
// void        _moo_lang_mgr_save_config           (MooLangMgr         *mgr);


G_END_DECLS

#endif /* __MOO_LANG_MGR_PRIVATE_H__ */
