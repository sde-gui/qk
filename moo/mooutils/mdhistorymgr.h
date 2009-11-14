/*
 *   mdhistorymgr.h
 *
 *   Copyright (C) 2004-2008 by Yevgen Muntyan <muntyan@tamu.edu>
 *
 *   This file is part of medit.  medit is free software; you can
 *   redistribute it and/or modify it under the terms of the
 *   GNU Lesser General Public License as published by the
 *   Free Software Foundation; either version 2.1 of the License,
 *   or (at your option) any later version.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with medit.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MD_HISTORY_MGR_H
#define MD_HISTORY_MGR_H

#include <gtk/gtk.h>

G_BEGIN_DECLS


#define MD_TYPE_HISTORY_MGR             (md_history_mgr_get_type ())
#define MD_HISTORY_MGR(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), MD_TYPE_HISTORY_MGR, MdHistoryMgr))
#define MD_HISTORY_MGR_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), MD_TYPE_HISTORY_MGR, MdHistoryMgrClass))
#define MD_IS_HISTORY_MGR(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MD_TYPE_HISTORY_MGR))
#define MD_IS_HISTORY_MGR_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), MD_TYPE_HISTORY_MGR))
#define MD_HISTORY_MGR_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), MD_TYPE_HISTORY_MGR, MdHistoryMgrClass))

typedef struct MdHistoryItem MdHistoryItem;
typedef struct MdHistoryMgr MdHistoryMgr;
typedef struct MdHistoryMgrClass MdHistoryMgrClass;
typedef struct MdHistoryMgrPrivate MdHistoryMgrPrivate;

struct MdHistoryMgr {
    GObject base;
    MdHistoryMgrPrivate *priv;
};

struct MdHistoryMgrClass {
    GObjectClass base_class;
};

typedef void (*MdHistoryCallback)               (GSList   *items,
                                                 gpointer  data);

GType       md_history_mgr_get_type             (void) G_GNUC_CONST;

void        md_history_mgr_add_file             (MdHistoryMgr   *mgr,
                                                 MdHistoryItem  *item);
void        md_history_mgr_update_file          (MdHistoryMgr   *mgr,
                                                 MdHistoryItem  *item);
void        md_history_mgr_add_uri              (MdHistoryMgr   *mgr,
                                                 const char     *uri);
void        md_history_mgr_remove_uri           (MdHistoryMgr   *mgr,
                                                 const char     *uri);
MdHistoryItem  *md_history_mgr_find_uri         (MdHistoryMgr   *mgr,
                                                 const char     *uri);

void        md_history_mgr_shutdown             (MdHistoryMgr   *mgr);

guint       md_history_mgr_get_n_items          (MdHistoryMgr   *mgr);

GtkWidget  *md_history_mgr_create_menu          (MdHistoryMgr   *mgr,
                                                 MdHistoryCallback callback,
                                                 gpointer        data,
                                                 GDestroyNotify  notify);
GtkWidget  *md_history_mgr_create_dialog        (MdHistoryMgr   *mgr,
                                                 MdHistoryCallback callback,
                                                 gpointer        data,
                                                 GDestroyNotify  notify);

MdHistoryItem  *md_history_item_new             (const char     *uri,
                                                 const char     *first_key,
                                                 ...);
MdHistoryItem  *md_history_item_copy            (MdHistoryItem  *item);
void            md_history_item_free            (MdHistoryItem  *item);
void            md_history_item_set             (MdHistoryItem  *item,
                                                 const char     *key,
                                                 const char     *value);
const char     *md_history_item_get             (MdHistoryItem  *item,
                                                 const char     *key);
const char     *md_history_item_get_uri         (MdHistoryItem  *item);
void            md_history_item_foreach         (MdHistoryItem  *item,
                                                 GDataForeachFunc func,
                                                 gpointer        user_data);

char          *_md_history_mgr_get_filename     (MdHistoryMgr   *mgr);

G_END_DECLS

#endif /* MD_HISTORY_MGR_H */
