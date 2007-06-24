/*
 *   moofile.c
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   See COPYING file that comes with this distribution.
 */

/*
 *  TODO!!! fix this mess
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define MOO_FILE_VIEW_COMPILATION
#include "moofileview/moofilesystem.h"
#include "moofileview/moofile-private.h"
#include "moofileview/moofileicons.h"
#include "mooutils/mooutils-fs.h"
#include "mooutils/mooutils-misc.h"
#include "mooutils/moocompat.h"
#include "mooutils/moomarshals.h"
#include "mooutils/xdgmime/xdgmime.h"
#include <glib/gstdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <errno.h>
#include <time.h>
#include <gtk/gtkicontheme.h>
#include <gtk/gtkiconfactory.h>
#include <gtk/gtkstock.h>

#ifdef MOO_US_XDGMIME
#define MIME_TYPE_UNKNOWN xdg_mime_type_unknown
#else
static const char *mime_type_unknown = "application/octet-stream";
#define MIME_TYPE_UNKNOWN mime_type_unknown
#endif

#define NORMAL_PRIORITY         G_PRIORITY_DEFAULT_IDLE
#define NORMAL_TIMEOUT          0.04
#define BACKGROUND_PRIORITY     G_PRIORITY_LOW
#define BACKGROUND_TIMEOUT      0.001

#define TIMER_CLEAR(timer)  \
G_STMT_START {              \
    g_timer_start (timer);  \
    g_timer_stop (timer);   \
} G_STMT_END

static MooIconType   _get_folder_icon           (const char     *path);
static MooIconEmblem _get_icon_flags            (const MooFile  *file);

static GdkPixbuf    *render_icon                (const MooFile  *file,
                                                 GtkWidget      *widget,
                                                 GtkIconSize     size);
static GdkPixbuf    *render_icon_for_path       (const char     *path,
                                                 GtkWidget      *widget,
                                                 GtkIconSize     size);

#define MAKE_PATH(dirname,file) g_build_filename (dirname, file->name, NULL)


void
_moo_file_find_mime_type (MooFile    *file,
                          const char *path)
{
    file->mime_type = xdg_mime_get_mime_type_for_file (path, file->statbuf);

    if (!file->mime_type || !file->mime_type[0])
    {
        /* this should not happen */
        if (0)
            _moo_message ("%s: oops, %s", G_STRLOC, file->display_name);
        file->mime_type = MIME_TYPE_UNKNOWN;
    }

    file->flags |= MOO_FILE_HAS_MIME_TYPE;
}


/********************************************************************/
/* MooFile
 */

MooFile *
_moo_file_new (const char *dirname,
               const char *basename)
{
    MooFile *file = NULL;
    char *path = NULL;
    char *display_name = NULL;

    g_return_val_if_fail (dirname != NULL, NULL);
    g_return_val_if_fail (basename && basename[0], NULL);

    path = g_build_filename (dirname, basename, NULL);
    g_return_val_if_fail (path != NULL, NULL);

    display_name = g_filename_display_basename (path);
    g_assert (g_utf8_validate (display_name, -1, NULL));

    if (!display_name)
    {
        g_free (path);
        g_return_val_if_fail (display_name != NULL, NULL);
    }

    file = _moo_new0 (MooFile);
    file->ref_count = 1;

    file->name = g_strdup (basename);

    file->display_name = g_utf8_normalize (display_name, -1, G_NORMALIZE_ALL);
    file->case_display_name = g_utf8_casefold (file->display_name, -1);
    file->collation_key = g_utf8_collate_key_for_filename (file->display_name, -1);

    g_free (path);
    g_free (display_name);

#ifndef __WIN32__
    if (basename[0] == '.')
        file->info = MOO_FILE_INFO_IS_HIDDEN;
#endif

    return file;
}


MooFile *
_moo_file_ref (MooFile *file)
{
    g_return_val_if_fail (file != NULL, NULL);
    file->ref_count++;
    return file;
}


void
_moo_file_unref (MooFile *file)
{
    if (file && !--file->ref_count)
    {
        g_free (file->name);
        g_free (file->display_name);
        g_free (file->case_display_name);
        g_free (file->collation_key);
        g_free (file->link_target);
        _moo_free (struct stat, file->statbuf);
        _moo_free (MooFile, file);
    }
}


void
_moo_file_stat (MooFile    *file,
                const char *dirname)
{
    char *fullname;

    g_return_if_fail (file != NULL);

    fullname = g_build_filename (dirname, file->name, NULL);

    file->info = MOO_FILE_INFO_EXISTS;
    file->flags = MOO_FILE_HAS_STAT;

    g_free (file->link_target);
    file->link_target = NULL;

    errno = 0;

    if (!file->statbuf)
        file->statbuf = _moo_new (struct stat);

    if (g_lstat (fullname, file->statbuf) != 0)
    {
        if (errno == ENOENT)
        {
            gchar *display_name = g_filename_display_name (fullname);
            _moo_message ("%s: file '%s' does not exist",
                          G_STRLOC, display_name);
            g_free (display_name);
            file->info = 0;
        }
        else
        {
            int save_errno = errno;
            gchar *display_name = g_filename_display_name (fullname);
            _moo_message ("%s: error getting information for '%s': %s",
                          G_STRLOC, display_name,
                          g_strerror (save_errno));
            g_free (display_name);
            file->info = MOO_FILE_INFO_IS_LOCKED | MOO_FILE_INFO_EXISTS;
            file->flags = 0;
        }
    }
    else
    {
#ifdef S_ISLNK
        if (S_ISLNK (file->statbuf->st_mode))
        {
            static char buf[1024];
            gssize len;

            file->info |= MOO_FILE_INFO_IS_LINK;
            errno = 0;

            if (g_stat (fullname, file->statbuf) != 0)
            {
                if (errno == ENOENT)
                {
                    gchar *display_name = g_filename_display_name (fullname);
                    _moo_message ("%s: file '%s' is a broken link",
                                  G_STRLOC, display_name);
                    g_free (display_name);
                    file->info = MOO_FILE_INFO_IS_LINK;
                }
                else
                {
                    int save_errno = errno;
                    gchar *display_name = g_filename_display_name (fullname);
                    _moo_message ("%s: error getting information for '%s': %s",
                                  G_STRLOC, display_name,
                                  g_strerror (save_errno));
                    g_free (display_name);
                    file->info = MOO_FILE_INFO_IS_LOCKED | MOO_FILE_INFO_EXISTS;
                    file->flags = 0;
                }
            }

            len = readlink (fullname, buf, 1024);

            if (len == -1)
            {
                int save_errno = errno;
                gchar *display_name = g_filename_display_name (fullname);
                _moo_message ("%s: error getting link target for '%s': %s",
                              G_STRLOC, display_name,
                              g_strerror (save_errno));
                g_free (display_name);
            }
            else
            {
                file->link_target = g_strndup (buf, len);
            }
        }
#endif
    }

    if ((file->info & MOO_FILE_INFO_EXISTS) &&
         !(file->info & MOO_FILE_INFO_IS_LOCKED))
    {
        if (S_ISDIR (file->statbuf->st_mode))
            file->info |= MOO_FILE_INFO_IS_DIR;
#ifdef S_ISBLK
        else if (S_ISBLK (file->statbuf->st_mode))
            file->info |= MOO_FILE_INFO_IS_BLOCK_DEV;
#endif
#ifdef S_ISCHR
        else if (S_ISCHR (file->statbuf->st_mode))
            file->info |= MOO_FILE_INFO_IS_CHAR_DEV;
#endif
#ifdef S_ISFIFO
        else if (S_ISFIFO (file->statbuf->st_mode))
            file->info |= MOO_FILE_INFO_IS_FIFO;
#endif
#ifdef S_ISSOCK
        else if (S_ISSOCK (file->statbuf->st_mode))
            file->info |= MOO_FILE_INFO_IS_SOCKET;
#endif
    }

    if (file->info & MOO_FILE_INFO_IS_DIR)
    {
        file->flags |= MOO_FILE_HAS_MIME_TYPE;
        file->flags |= MOO_FILE_HAS_ICON;
    }

    file->icon = _moo_file_get_icon_type (file, dirname);

    if (file->name[0] == '.')
        file->info |= MOO_FILE_INFO_IS_HIDDEN;

    g_free (fullname);
}

void
_moo_file_free_statbuf (MooFile *file)
{
    g_return_if_fail (file != NULL);
    _moo_free (struct stat, file->statbuf);
    file->statbuf = NULL;
    file->flags &= ~MOO_FILE_HAS_STAT;
}


gboolean
_moo_file_test (const MooFile  *file,
                MooFileInfo     test)
{
    g_return_val_if_fail (file != NULL, FALSE);
    return file->info & test;
}


const char *
_moo_file_display_name (const MooFile *file)
{
    g_return_val_if_fail (file != NULL, NULL);
    return file->display_name;
}

const char *
_moo_file_collation_key (const MooFile *file)
{
    g_return_val_if_fail (file != NULL, NULL);
    return file->collation_key;
}

const char *
_moo_file_case_display_name (const MooFile *file)
{
    g_return_val_if_fail (file != NULL, NULL);
    return file->case_display_name;
}

const char *
_moo_file_name (const MooFile *file)
{
    g_return_val_if_fail (file != NULL, NULL);
    return file->name;
}


const char *
_moo_file_get_mime_type (const MooFile *file)
{
    g_return_val_if_fail (file != NULL, NULL);
    return file->mime_type;
}


GdkPixbuf *
_moo_file_get_icon (const MooFile  *file,
                    GtkWidget      *widget,
                    GtkIconSize     size)
{
    g_return_val_if_fail (file != NULL, NULL);
    g_return_val_if_fail (GTK_IS_WIDGET (widget), NULL);
    return render_icon (file, widget, size);
}


GdkPixbuf *
_moo_get_icon_for_path (const char     *path,
                        GtkWidget      *widget,
                        GtkIconSize     size)
{
    g_return_val_if_fail (GTK_IS_WIDGET (widget), NULL);
    return render_icon_for_path (path, widget, size);
}


#ifndef __WIN32__
const char *
_moo_file_link_get_target (const MooFile *file)
{
    return file->link_target;
}
#endif


#if 0
MooFileTime
_moo_file_get_mtime (const MooFile *file)
{
    g_return_val_if_fail (file != NULL, 0);
    return file->statbuf.st_mtime;
}

MooFileSize
_moo_file_get_size (const MooFile *file)
{
    g_return_val_if_fail (file != NULL, 0);
    return file->statbuf.st_size;
}

#ifndef __WIN32__
const struct stat *
_moo_file_get_stat (const MooFile *file)
{
    g_return_val_if_fail (file != NULL, NULL);
    if (file->flags & MOO_FILE_HAS_STAT && file->info & MOO_FILE_INFO_EXISTS)
        return &file->statbuf;
    else
        return NULL;
}
#endif
#endif


GType
_moo_file_get_type (void)
{
    static GType type = 0;

    if (G_UNLIKELY (!type))
        type = g_boxed_type_register_static ("MooFile",
                                             (GBoxedCopyFunc) _moo_file_ref,
                                             (GBoxedFreeFunc) _moo_file_unref);

    return type;
}


GType
_moo_file_flags_get_type (void)
{
    static GType type = 0;

    static const GFlagsValue values[] = {
        { MOO_FILE_HAS_MIME_TYPE, (char*)"MOO_FILE_HAS_MIME_TYPE", (char*)"has-mime-type" },
        { MOO_FILE_HAS_ICON, (char*)"MOO_FILE_HAS_ICON", (char*)"has-icon" },
        { MOO_FILE_HAS_STAT, (char*)"MOO_FILE_HAS_STAT", (char*)"has-stat" },
        { MOO_FILE_ALL_FLAGS, (char*)"MOO_FILE_ALL_FLAGS", (char*)"all-flags" },
        { 0, NULL, NULL }
    };

    if (G_UNLIKELY (!type))
        type = g_flags_register_static ("MooFileFlags", values);

    return type;
}


GType
_moo_file_info_get_type (void)
{
    static GType type = 0;

    static const GFlagsValue values[] = {
        { MOO_FILE_INFO_EXISTS, (char*)"MOO_FILE_INFO_EXISTS", (char*)"exists" },
        { MOO_FILE_INFO_IS_DIR, (char*)"MOO_FILE_INFO_IS_DIR", (char*)"is-folder" },
        { MOO_FILE_INFO_IS_HIDDEN, (char*)"MOO_FILE_INFO_IS_HIDDEN", (char*)"is-hidden" },
        { MOO_FILE_INFO_IS_LINK, (char*)"MOO_FILE_INFO_IS_LINK", (char*)"is-link" },
        { 0, NULL, NULL }
    };

    if (G_UNLIKELY (!type))
        type = g_flags_register_static ("MooFileInfo", values);

    return type;
}


static GdkPixbuf *
render_icon (const MooFile  *file,
             GtkWidget      *widget,
             GtkIconSize     size)
{
    GdkPixbuf *pixbuf = _moo_get_icon (widget, file->icon, file->mime_type,
                                       _get_icon_flags (file), size);
    g_assert (pixbuf != NULL);
    return pixbuf;
}


static GdkPixbuf *
render_icon_for_path (const char     *path,
                      GtkWidget      *widget,
                      GtkIconSize     size)
{
    const char *mime_type = MIME_TYPE_UNKNOWN;

#ifndef __WIN32__
    if (path)
    {
        mime_type = xdg_mime_get_mime_type_for_file (path, NULL);

        if (!mime_type || !mime_type[0])
            mime_type = MIME_TYPE_UNKNOWN;
    }
#else
#ifdef __GNUC__
#warning "Implement render_icon_for_path()"
#endif
#endif

    return _moo_get_icon (widget, MOO_ICON_MIME, mime_type, 0, size);
}


guint8
_moo_file_get_icon_type (MooFile    *file,
                         const char *dirname)
{
    if (MOO_FILE_IS_BROKEN_LINK (file))
        return MOO_ICON_BROKEN_LINK;

    if (!MOO_FILE_EXISTS (file))
        return MOO_ICON_NONEXISTENT;

    if (MOO_FILE_IS_DIR (file))
    {
        char *path = MAKE_PATH (dirname, file);
        MooIconType icon = _get_folder_icon (path);
        g_free (path);
        return icon;
    }

    if (MOO_FILE_IS_SPECIAL (file))
    {
        if (_moo_file_test (file, MOO_FILE_INFO_IS_BLOCK_DEV))
            return MOO_ICON_BLOCK_DEVICE;
        else if (_moo_file_test (file, MOO_FILE_INFO_IS_CHAR_DEV))
            return MOO_ICON_CHARACTER_DEVICE;
        else if (_moo_file_test (file, MOO_FILE_INFO_IS_FIFO))
            return MOO_ICON_FIFO;
        else if (_moo_file_test (file, MOO_FILE_INFO_IS_SOCKET))
            return MOO_ICON_SOCKET;
    }

    if (file->flags & MOO_FILE_HAS_MIME_TYPE && file->mime_type)
        return MOO_ICON_MIME;

    return MOO_ICON_FILE;
}


guint8
_moo_file_icon_blank (void)
{
    return MOO_ICON_BLANK;
}


static MooIconType
_get_folder_icon (const char *path)
{
    static const char *home_path = NULL;
    static char *desktop_path = NULL;
    static char *trash_path = NULL;

    if (!path)
        return MOO_ICON_DIRECTORY;

    if (!home_path)
        home_path = g_get_home_dir ();

    if (!home_path)
        return MOO_ICON_DIRECTORY;

    if (!desktop_path)
        desktop_path = g_build_filename (home_path, "Desktop", NULL);

    if (!trash_path)
        trash_path = g_build_filename (desktop_path, "Trash", NULL);

        /* keep this in sync with create_fallback_icon() */
    if (strcmp (home_path, path) == 0)
        return MOO_ICON_HOME;
    else if (strcmp (desktop_path, path) == 0)
        return MOO_ICON_DESKTOP;
    else if (strcmp (trash_path, path) == 0)
        return MOO_ICON_TRASH;
    else
        return MOO_ICON_DIRECTORY;
}


static MooIconEmblem
_get_icon_flags (const MooFile *file)
{
    return
#if 0
        (MOO_FILE_IS_LOCKED (file) ? MOO_ICON_EMBLEM_LOCK : 0) |
#endif
        (MOO_FILE_IS_LINK (file) ? MOO_ICON_EMBLEM_LINK : 0);
}
