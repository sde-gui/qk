/*
 *   moofile-private.h
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

#ifndef MOO_FILE_VIEW_COMPILATION
#error "This file may not be included"
#endif

#ifndef __MOO_FILE_PRIVATE_H__
#define __MOO_FILE_PRIVATE_H__

#include "moofileview/moofile.h"
#include <sys/stat.h>
#include <sys/types.h>

G_BEGIN_DECLS


#define MOO_TYPE_FILE               (_moo_file_get_type ())
#define MOO_TYPE_FILE_INFO          (_moo_file_info_get_type ())
#define MOO_TYPE_FILE_FLAGS         (_moo_file_flags_get_type ())
#define MOO_TYPE_FOLDER             (_moo_folder_get_type ())
#define MOO_FOLDER(object)          (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_FOLDER, MooFolder))
#define MOO_FOLDER_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_FOLDER, MooFolderClass))
#define MOO_IS_FOLDER(object)       (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_FOLDER))
#define MOO_IS_FOLDER_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_FOLDER))
#define MOO_FOLDER_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_FOLDER, MooFolderClass))

#ifdef __WIN32__
/* FILETIME */
typedef guint64 MooFileTime;
#else
/* time_t */
/* XXX it's not time_t! */
typedef GTime MooFileTime;
#endif

typedef gint64 MooFileSize;

typedef struct _MooFolderPrivate MooFolderPrivate;
typedef struct _MooFolderClass   MooFolderClass;
typedef struct _MooFileSystem    MooFileSystem;

/* should be ordered TODO why? */
typedef enum {
    MOO_FILE_HAS_STAT       = 1 << 1,
    MOO_FILE_HAS_MIME_TYPE  = 1 << 2,
    MOO_FILE_HAS_ICON       = 1 << 3,
    MOO_FILE_ALL_FLAGS      = (1 << 4) - 1
} MooFileFlags;

struct _MooFolder
{
    GObject         parent;
    MooFolderPrivate *priv;
};

struct _MooFolderClass
{
    GObjectClass    parent_class;

    void     (*deleted)         (MooFolder  *folder);
    void     (*files_added)     (MooFolder  *folder,
                                 GSList     *files);
    void     (*files_changed)   (MooFolder  *folder,
                                 GSList     *files);
    void     (*files_removed)   (MooFolder  *folder,
                                 GSList     *files);
};

struct _MooFile
{
    char           *name;
    char           *link_target;
    char           *display_name; /* normalized */
    char           *case_display_name;
    char           *collation_key;
    MooFileInfo     info;
    MooFileFlags    flags;
    guint8          icon;
    const char     *mime_type;
    int             ref_count;
/* TODO: who needs whole structure? */
    struct stat     statbuf;
};


GType        _moo_file_get_type         (void) G_GNUC_CONST;
GType        _moo_file_flags_get_type   (void) G_GNUC_CONST;
GType        _moo_file_info_get_type    (void) G_GNUC_CONST;
GType        _moo_folder_get_type       (void) G_GNUC_CONST;

MooFile     *_moo_file_ref              (MooFile        *file);
void         _moo_file_unref            (MooFile        *file);

MooFileTime  _moo_file_get_mtime        (const MooFile  *file);
MooFileSize  _moo_file_get_size         (const MooFile  *file);

MooFileInfo  _moo_file_get_info         (const MooFile  *file);

const char  *_moo_file_name             (const MooFile  *file);

/* returned pixbuf is owned by icon cache */
GdkPixbuf   *_moo_file_get_icon         (const MooFile  *file,
                                         GtkWidget      *widget,
                                         GtkIconSize     size);

const char  *_moo_file_collation_key    (const MooFile  *file);
const char  *_moo_file_case_display_name(const MooFile *file);

#ifndef __WIN32__
gconstpointer _moo_file_get_stat        (const MooFile  *file);
const char  *_moo_file_link_get_target  (const MooFile  *file);
#endif

const char  *_moo_folder_get_path       (MooFolder      *folder);
/* list should be freed and elements unref'ed */
GSList      *_moo_folder_list_files     (MooFolder      *folder);
MooFile     *_moo_folder_get_file       (MooFolder      *folder,
                                         const char     *basename);
char        *_moo_folder_get_file_path  (MooFolder      *folder,
                                         MooFile        *file);
char        *_moo_folder_get_file_uri   (MooFolder      *folder,
                                         MooFile        *file);
/* result should be unref'ed */
MooFolder   *_moo_folder_get_parent     (MooFolder      *folder,
                                         MooFileFlags    wanted);
char        *_moo_folder_get_parent_path(MooFolder      *folder);

char       **_moo_folder_get_file_info  (MooFolder      *folder,
                                         MooFile        *file);
void         _moo_folder_reload         (MooFolder      *folder);


MooFolder   *_moo_folder_new            (MooFileSystem  *fs,
                                         const char     *path,
                                         MooFileFlags    wanted,
                                         GError        **error);
void         _moo_folder_set_wanted     (MooFolder      *folder,
                                         MooFileFlags    wanted,
                                         gboolean        bit_now);


G_END_DECLS

#endif /* __MOO_FILE_PRIVATE_H__ */
