/*
 *   moohistoryentry.h
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

#ifndef __MOO_HISTORY_ENTRY_H__
#define __MOO_HISTORY_ENTRY_H__

#include <mooutils/moocombo.h>
#include <mooutils/moohistorylist.h>

G_BEGIN_DECLS


#define MOO_TYPE_HISTORY_ENTRY              (moo_history_entry_get_type ())
#define MOO_HISTORY_ENTRY(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_HISTORY_ENTRY, MooHistoryEntry))
#define MOO_HISTORY_ENTRY_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_HISTORY_ENTRY, MooHistoryEntryClass))
#define MOO_IS_HISTORY_ENTRY(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_HISTORY_ENTRY))
#define MOO_IS_HISTORY_ENTRY_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_HISTORY_ENTRY))
#define MOO_HISTORY_ENTRY_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_HISTORY_ENTRY, MooHistoryEntryClass))

typedef struct _MooHistoryEntry         MooHistoryEntry;
typedef struct _MooHistoryEntryPrivate  MooHistoryEntryPrivate;
typedef struct _MooHistoryEntryClass    MooHistoryEntryClass;

struct _MooHistoryEntry
{
    MooCombo parent;
    MooHistoryEntryPrivate *priv;
};

struct _MooHistoryEntryClass
{
    MooComboClass parent_class;
};

typedef gboolean (*MooHistoryEntryFilterFunc)   (const char    *text,
                                                 GtkTreeModel  *model,
                                                 GtkTreeIter   *iter,
                                                 gpointer       data);


GType           moo_history_entry_get_type  (void) G_GNUC_CONST;

GtkWidget      *moo_history_entry_new       (const char         *user_id);

void            moo_history_entry_set_list  (MooHistoryEntry    *entry,
                                             MooHistoryList     *list);
MooHistoryList *moo_history_entry_get_list  (MooHistoryEntry    *entry);

void            moo_history_entry_add_text  (MooHistoryEntry    *entry,
                                             const char         *text);


G_END_DECLS

#endif /* __MOO_HISTORY_ENTRY_H__ */
