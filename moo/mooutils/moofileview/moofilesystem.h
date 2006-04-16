/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *   mooedit/moofilesystem.h
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

/*
 *  MooFile, MooFolder, and MooFileSystem are utility classes for loading/caching
 *  folders and files, ala GtkFile* stuff (I guess, at least this header was copied
 *  from gtk/gtkfilesystem.h; I also borrowed icon code from there.).
 */

#ifndef __MOO_FILE_SYSTEM_H__
#define __MOO_FILE_SYSTEM_H__

#include <moofileview/moofile.h>
#include <gtk/gtkwidget.h>
#ifdef MOO_FILE_SYSTEM_COMPILATION
#include "mooutils/moofilewatch.h"
#endif


G_BEGIN_DECLS


#define MOO_TYPE_FILE_SYSTEM              (moo_file_system_get_type ())
#define MOO_FILE_SYSTEM(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_FILE_SYSTEM, MooFileSystem))
#define MOO_FILE_SYSTEM_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_FILE_SYSTEM, MooFileSystemClass))
#define MOO_IS_FILE_SYSTEM(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_FILE_SYSTEM))
#define MOO_IS_FILE_SYSTEM_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_FILE_SYSTEM))
#define MOO_FILE_SYSTEM_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_FILE_SYSTEM, MooFileSystemClass))

typedef struct _MooFileSystemPrivate  MooFileSystemPrivate;
typedef struct _MooFileSystemClass    MooFileSystemClass;

struct _MooFileSystem
{
    GObject         parent;
    MooFileSystemPrivate *priv;
};

struct _MooFileSystemClass
{
    GObjectClass    parent_class;

    MooFolder*  (*get_root_folder)  (MooFileSystem  *fs,
                                     MooFileFlags    flags);
    MooFolder*  (*get_folder)       (MooFileSystem  *fs,
                                     const char     *path,
                                     MooFileFlags    flags,
                                     GError        **error);
    MooFolder*  (*get_parent_folder)(MooFileSystem  *fs,
                                     MooFolder      *folder,
                                     MooFileFlags    flags);

    char*       (*normalize_path)   (MooFileSystem  *fs,
                                     const char     *path,
                                     gboolean        is_folder,
                                     GError        **error);
    char*       (*make_path)        (MooFileSystem  *fs,
                                     const char     *base_path,
                                     const char     *display_name,
                                     GError        **error);
    gboolean    (*create_folder)    (MooFileSystem  *fs,
                                     const char     *path,
                                     GError        **error);
    gboolean    (*delete_file)      (MooFileSystem  *fs,
                                     const char     *path,
                                     gboolean        recursive,
                                     GError        **error);
    gboolean    (*move_file)        (MooFileSystem  *fs,
                                     const char     *old_path,
                                     const char     *new_path,
                                     GError        **error);
    gboolean    (*parse_path)       (MooFileSystem  *fs,
                                     const char     *path_utf8,
                                     char          **dirname,
                                     char          **display_dirname,
                                     char          **display_basename,
                                     GError        **error);
    char*       (*get_absolute_path)(MooFileSystem  *fs,
                                     const char     *display_name,
                                     const char     *current_dir);
};


GType        moo_file_system_get_type           (void) G_GNUC_CONST;

MooFileSystem *moo_file_system_create           (void);

MooFolder   *moo_file_system_get_root_folder    (MooFileSystem  *fs,
                                                 MooFileFlags    wanted);
MooFolder   *moo_file_system_get_folder         (MooFileSystem  *fs,
                                                 const char     *path,
                                                 MooFileFlags    wanted,
                                                 GError        **error);
MooFolder   *moo_file_system_get_parent_folder  (MooFileSystem  *fs,
                                                 MooFolder      *folder,
                                                 MooFileFlags    wanted);

gboolean     moo_file_system_create_folder      (MooFileSystem  *fs,
                                                 const char     *path,
                                                 GError        **error);
gboolean     moo_file_system_delete_file        (MooFileSystem  *fs,
                                                 const char     *path,
                                                 gboolean        recursive,
                                                 GError        **error);
gboolean     moo_file_system_move_file          (MooFileSystem  *fs,
                                                 const char     *old_path,
                                                 const char     *new_path,
                                                 GError        **error);

char        *moo_file_system_make_path          (MooFileSystem  *fs,
                                                 const char     *base_path,
                                                 const char     *display_name,
                                                 GError        **error);
char        *moo_file_system_normalize_path     (MooFileSystem  *fs,
                                                 const char     *path,
                                                 gboolean        is_folder,
                                                 GError        **error);
gboolean     moo_file_system_parse_path         (MooFileSystem  *fs,
                                                 const char     *path_utf8,
                                                 char          **dirname,
                                                 char          **display_dirname,
                                                 char          **display_basename,
                                                 GError        **error);
char        *moo_file_system_get_absolute_path  (MooFileSystem  *fs,
                                                 const char     *display_name,
                                                 const char     *current_dir);


MooFileSystem *moo_folder_get_file_system       (MooFolder      *folder);


#ifdef MOO_FILE_SYSTEM_COMPILATION
MooFileWatch *moo_file_system_get_file_watch    (MooFileSystem  *fs);
#endif


G_END_DECLS

#endif /* __MOO_FILE_SYSTEM_H__ */
