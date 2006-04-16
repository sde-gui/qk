/*
 *   moofileentry.h
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

#ifndef __MOO_FILE_ENTRY_H__
#define __MOO_FILE_ENTRY_H__

#include <mooutils/mooentry.h>
#include <moofileview/moofile.h>

G_BEGIN_DECLS


#define MOO_TYPE_FILE_ENTRY_COMPLETION              (moo_file_entry_completion_get_type ())
#define MOO_FILE_ENTRY_COMPLETION(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_FILE_ENTRY_COMPLETION, MooFileEntryCompletion))
#define MOO_FILE_ENTRY_COMPLETION_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_FILE_ENTRY_COMPLETION, MooFileEntryCompletionClass))
#define MOO_IS_FILE_ENTRY_COMPLETION(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_FILE_ENTRY_COMPLETION))
#define MOO_IS_FILE_ENTRY_COMPLETION_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_FILE_ENTRY_COMPLETION))
#define MOO_FILE_ENTRY_COMPLETION_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_FILE_ENTRY_COMPLETION, MooFileEntryCompletionClass))

#define MOO_TYPE_FILE_ENTRY              (moo_file_entry_get_type ())
#define MOO_FILE_ENTRY(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_FILE_ENTRY, MooFileEntry))
#define MOO_FILE_ENTRY_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_FILE_ENTRY, MooFileEntryClass))
#define MOO_IS_FILE_ENTRY(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_FILE_ENTRY))
#define MOO_IS_FILE_ENTRY_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_FILE_ENTRY))
#define MOO_FILE_ENTRY_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_FILE_ENTRY, MooFileEntryClass))

typedef struct _MooFileEntry         MooFileEntry;
typedef struct _MooFileEntryClass    MooFileEntryClass;

typedef struct _MooFileEntryCompletion         MooFileEntryCompletion;
typedef struct _MooFileEntryCompletionPrivate  MooFileEntryCompletionPrivate;
typedef struct _MooFileEntryCompletionClass    MooFileEntryCompletionClass;

struct _MooFileEntry
{
    MooEntry parent;

    MooFileEntryCompletion *completion;
};

struct _MooFileEntryClass
{
    MooEntryClass parent_class;
};

struct _MooFileEntryCompletion
{
    GObject parent;
    MooFileEntryCompletionPrivate *priv;
};

struct _MooFileEntryCompletionClass
{
    GObjectClass parent_class;
};

typedef gboolean (*MooFileVisibleFunc) (MooFile   *file,
                                        gpointer   data);


GType       moo_file_entry_get_type                     (void) G_GNUC_CONST;
GType       moo_file_entry_completion_get_type          (void) G_GNUC_CONST;

GtkWidget  *moo_file_entry_new                          (void);

void        moo_file_entry_completion_set_visible_func  (MooFileEntryCompletion *completion,
                                                         MooFileVisibleFunc      func,
                                                         gpointer                data);
void        moo_file_entry_completion_set_entry         (MooFileEntryCompletion *completion,
                                                         GtkEntry               *entry);

/* converts path to utf8 and sets entry content */
void        moo_file_entry_completion_set_path          (MooFileEntryCompletion *completion,
                                                         const char             *path);


G_END_DECLS

#endif /* __MOO_FILE_ENTRY_H__ */
