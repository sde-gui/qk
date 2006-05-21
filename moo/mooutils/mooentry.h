/*
 *   mooentry.h
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

#ifndef __MOO_ENTRY_H__
#define __MOO_ENTRY_H__

#include <gtk/gtkentry.h>

G_BEGIN_DECLS


#define MOO_TYPE_ENTRY              (moo_entry_get_type ())
#define MOO_ENTRY(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_ENTRY, MooEntry))
#define MOO_ENTRY_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_ENTRY, MooEntryClass))
#define MOO_IS_ENTRY(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_ENTRY))
#define MOO_IS_ENTRY_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_ENTRY))
#define MOO_ENTRY_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_ENTRY, MooEntryClass))

typedef struct _MooEntry         MooEntry;
typedef struct _MooEntryPrivate  MooEntryPrivate;
typedef struct _MooEntryClass    MooEntryClass;

struct _MooEntry
{
    GtkEntry parent;
    MooEntryPrivate *priv;
};

struct _MooEntryClass
{
    GtkEntryClass parent_class;

    void (*undo)              (MooEntry *entry);
    void (*redo)              (MooEntry *entry);
};


GType       moo_entry_get_type                  (void) G_GNUC_CONST;

GtkWidget  *moo_entry_new                       (void);

void        moo_entry_undo                      (MooEntry   *entry);
void        moo_entry_redo                      (MooEntry   *entry);

void        moo_entry_begin_undo_group          (MooEntry   *entry);
void        moo_entry_end_undo_group            (MooEntry   *entry);
void        moo_entry_clear_undo                (MooEntry   *entry);

void        moo_entry_set_use_special_chars_menu(MooEntry   *entry,
                                                 gboolean    use);


G_END_DECLS

#endif /* __MOO_ENTRY_H__ */
