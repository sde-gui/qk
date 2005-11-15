/*
 *   mooundomanager.h
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

#ifndef __MOO_UNDO_MANAGER_H__
#define __MOO_UNDO_MANAGER_H__

#include <glib-object.h>

G_BEGIN_DECLS


#define MOO_TYPE_UNDO_MGR              (moo_undo_mgr_get_type ())
#define MOO_UNDO_MGR(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_UNDO_MGR, MooUndoMgr))
#define MOO_UNDO_MGR_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_UNDO_MGR, MooUndoMgrClass))
#define MOO_IS_UNDO_MGR(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_UNDO_MGR))
#define MOO_IS_UNDO_MGR_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_UNDO_MGR))
#define MOO_UNDO_MGR_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_UNDO_MGR, MooUndoMgrClass))


typedef struct _MooUndoMgr          MooUndoMgr;
typedef struct _MooUndoMgrPrivate   MooUndoMgrPrivate;
typedef struct _MooUndoMgrClass     MooUndoMgrClass;
typedef struct _MooUndoAction       MooUndoAction;
typedef struct _MooUndoActionClass  MooUndoActionClass;

typedef void     (*MooUndoActionUndo)   (MooUndoAction  *action,
                                         gpointer        document);
typedef void     (*MooUndoActionRedo)   (MooUndoAction  *action,
                                         gpointer        document);
typedef gboolean (*MooUndoActionMerge)  (MooUndoAction  *action,
                                         MooUndoAction  *what,
                                         gpointer        document);
typedef void     (*MooUndoActionDestroy)(MooUndoAction  *action,
                                         gpointer        document);

struct _MooUndoActionClass
{
    MooUndoActionUndo undo;
    MooUndoActionRedo redo;
    MooUndoActionMerge merge;
    MooUndoActionDestroy destroy;
};

struct _MooUndoMgr
{
    GObject base;

    gpointer document;
    GSList *undo_stack; /* ActionGroup* */
    GSList *redo_stack; /* ActionGroup* */

    guint frozen;
    guint continue_group;
    gboolean do_continue;
    gboolean new_group;
};

struct _MooUndoMgrClass
{
    GObjectClass base_class;

    void (*undo) (MooUndoMgr    *mgr);
    void (*redo) (MooUndoMgr    *mgr);
};


GType       moo_undo_mgr_get_type       (void) G_GNUC_CONST;

MooUndoMgr *moo_undo_mgr_new            (gpointer        document);

guint       moo_undo_action_register    (MooUndoActionClass *klass);

void        moo_undo_mgr_add_action     (MooUndoMgr     *mgr,
                                         guint           type,
                                         MooUndoAction  *action);

void        moo_undo_mgr_clear          (MooUndoMgr     *mgr);
void        moo_undo_mgr_freeze         (MooUndoMgr     *mgr);
void        moo_undo_mgr_thaw           (MooUndoMgr     *mgr);
gboolean    moo_undo_mgr_frozen         (MooUndoMgr     *mgr);

void        moo_undo_mgr_new_group      (MooUndoMgr     *mgr);
void        moo_undo_mgr_start_group    (MooUndoMgr     *mgr);
void        moo_undo_mgr_end_group      (MooUndoMgr     *mgr);

void        moo_undo_mgr_undo           (MooUndoMgr     *mgr);
void        moo_undo_mgr_redo           (MooUndoMgr     *mgr);
gboolean    moo_undo_mgr_can_undo       (MooUndoMgr     *mgr);
gboolean    moo_undo_mgr_can_redo       (MooUndoMgr     *mgr);


G_END_DECLS

#endif /* __MOO_UNDO_MANAGER_H__ */
