/*
 *   mooedit/mooeditfilemgr.h
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

#ifndef MOOEDIT_MOOEDITFILEMGR_H
#define MOOEDIT_MOOEDITFILEMGR_H

#include "mooedit/mooedit.h"

G_BEGIN_DECLS


#define MOO_TYPE_EDIT_FILE_MGR              (moo_edit_file_mgr_get_type ())
#define MOO_EDIT_FILE_MGR(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_EDIT_FILE_MGR, MooEditFileMgr))
#define MOO_EDIT_FILE_MGR_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_EDIT_FILE_MGR, MooEditFileMgrClass))
#define MOO_IS_EDIT_FILE_MGR(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_EDIT_FILE_MGR))
#define MOO_IS_EDIT_FILE_MGR_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_EDIT_FILE_MGR))
#define MOO_EDIT_FILE_MGR_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_EDIT_FILE_MGR, MooEditFileMgrClass))


typedef struct _MooEditFileMgr        MooEditFileMgr;
typedef struct _MooEditFileMgrPrivate MooEditFileMgrPrivate;
typedef struct _MooEditFileMgrClass   MooEditFileMgrClass;

struct _MooEditFileMgr
{
    GObject      parent;

    MooEditFileMgrPrivate *priv;
};

struct _MooEditFileMgrClass
{
    GObjectClass parent_class;

    void (*open_recent) (MooEditFileMgr     *mgr,
                         MooEditFileInfo    *file,
                         GtkWidget          *menu_item);
};


GType            moo_edit_file_mgr_get_type         (void) G_GNUC_CONST;

MooEditFileMgr  *moo_edit_file_mgr_new              (void);

GtkMenuItem     *moo_edit_file_mgr_create_recent_files_menu
                                                    (MooEditFileMgr *mgr,
                                                     gpointer        data);

void             moo_edit_file_mgr_add_recent       (MooEditFileMgr *mgr,
                                                     MooEditFileInfo *info);

MooEditFileInfo *moo_edit_file_mgr_save_as_dialog   (MooEditFileMgr *mgr,
                                                     MooEdit        *edit);
GSList          *moo_edit_file_mgr_open_dialog      (MooEditFileMgr *mgr,
                                                     GtkWidget      *parent);


G_END_DECLS

#endif /* MOOEDIT_MOOEDITFILEMGR_H */
