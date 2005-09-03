/*
 *   moorecentmgr.h
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

#ifndef __MOO_RECENT_MGR_H__
#define __MOO_RECENT_MGR_H__

#include "mooedit/mooedit.h"

G_BEGIN_DECLS


#define MOO_TYPE_RECENT_MGR              (moo_recent_mgr_get_type ())
#define MOO_RECENT_MGR(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_RECENT_MGR, MooRecentMgr))
#define MOO_RECENT_MGR_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_RECENT_MGR, MooRecentMgrClass))
#define MOO_IS_RECENT_MGR(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_RECENT_MGR))
#define MOO_IS_RECENT_MGR_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_RECENT_MGR))
#define MOO_RECENT_MGR_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_RECENT_MGR, MooRecentMgrClass))


typedef struct _MooRecentMgr        MooRecentMgr;
typedef struct _MooRecentMgrPrivate MooRecentMgrPrivate;
typedef struct _MooRecentMgrClass   MooRecentMgrClass;

struct _MooRecentMgr
{
    GObject      parent;

    MooRecentMgrPrivate *priv;
};

struct _MooRecentMgrClass
{
    GObjectClass parent_class;

    void (*open_recent) (MooRecentMgr       *mgr,
                         MooEditFileInfo    *file,
                         GtkWidget          *menu_item);
    void (*item_added)  (MooRecentMgr       *mgr);
};


GType            moo_recent_mgr_get_type        (void) G_GNUC_CONST;

MooRecentMgr    *moo_recent_mgr_new             (void);

GtkMenuItem     *moo_recent_mgr_create_menu     (MooRecentMgr   *mgr,
                                                 gpointer        data);

void             moo_recent_mgr_add_recent      (MooRecentMgr   *mgr,
                                                 MooEditFileInfo *info);
guint            moo_recent_mgr_get_num_items   (MooRecentMgr   *mgr);


G_END_DECLS

#endif /* __MOO_RECENT_MGR_H__ */
