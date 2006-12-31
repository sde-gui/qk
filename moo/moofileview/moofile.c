/*
 *   moofile.c
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
 *  Icon handling code was copied (and modified) from gtk/gtkfilesystemunix.c,
 *  Copyright (C) 2003, Red Hat, Inc.
 *
 *  g_utf8_collate_key_for_filename is taken from glib/gunicollate.c
 *  Copyright 2001,2005 Red Hat, Inc.
 *
 *  TODO!!! fix this mess
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define MOO_FILE_VIEW_COMPILATION
#include "moofileview/moofilesystem.h"
#include "moofileview/moofile-private.h"
#include "moofileview/symlink.h"
#include "mooutils/mooutils-fs.h"
#include "mooutils/mooutils-misc.h"
#include "mooutils/moomarshals.h"
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

#ifndef __WIN32__
#include "mooutils/xdgmime/xdgmime.h"
#endif

#ifdef MOO_US_XDGMIME
#define MIME_TYPE_UNKNOWN xdg_mime_type_unknown
#else
static const char *mime_type_unknown = "application/octet-stream";
#define MIME_TYPE_UNKNOWN mime_type_unknown
#endif

#if !GLIB_CHECK_VERSION(2,8,0)
static gchar *g_utf8_collate_key_for_filename   (const gchar *str,
                                                 gssize       len);
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

static GdkPixbuf *render_icon               (const MooFile  *file,
                                             GtkWidget      *widget,
                                             GtkIconSize     size);
static GdkPixbuf *render_icon_for_path      (const char     *path,
                                             GtkWidget      *widget,
                                             GtkIconSize     size);

#define MAKE_PATH(dirname,file) g_build_filename (dirname, file->name, NULL)


#ifndef __WIN32__
void
_moo_file_find_mime_type (MooFile    *file,
                          const char *path)
{
    if (file->flags & MOO_FILE_HAS_STAT)
        file->mime_type = xdg_mime_get_mime_type_for_file (path, &file->statbuf);
    else
        file->mime_type = xdg_mime_get_mime_type_for_file (path, NULL);

    if (!file->mime_type || !file->mime_type[0])
    {
        /* this should not happen */
        if (0)
            _moo_message ("%s: oops, %s", G_STRLOC, file->display_name);
        file->mime_type = MIME_TYPE_UNKNOWN;
    }

    file->flags |= MOO_FILE_HAS_MIME_TYPE;
}
#endif /* !__WIN32__ */


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

    file = g_new0 (MooFile, 1);
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
        g_free (file);
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

    if (g_lstat (fullname, &file->statbuf) != 0)
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
        if (S_ISLNK (file->statbuf.st_mode))
        {
            static char buf[1024];
            gssize len;

            file->info |= MOO_FILE_INFO_IS_LINK;
            errno = 0;

            if (g_stat (fullname, &file->statbuf) != 0)
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
        if (S_ISDIR (file->statbuf.st_mode))
            file->info |= MOO_FILE_INFO_IS_DIR;
#ifdef S_ISBLK
        else if (S_ISBLK (file->statbuf.st_mode))
            file->info |= MOO_FILE_INFO_IS_BLOCK_DEV;
#endif
#ifdef S_ISCHR
        else if (S_ISCHR (file->statbuf.st_mode))
            file->info |= MOO_FILE_INFO_IS_CHAR_DEV;
#endif
#ifdef S_ISFIFO
        else if (S_ISFIFO (file->statbuf.st_mode))
            file->info |= MOO_FILE_INFO_IS_FIFO;
#endif
#ifdef S_ISSOCK
        else if (S_ISSOCK (file->statbuf.st_mode))
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


#if !GLIB_CHECK_VERSION(2,8,0)
/* This is a collation key that is very very likely to sort before any
   collation key that libc strxfrm generates. We use this before any
   special case (dot or number) to make sure that its sorted before
   anything else.
 */
#define COLLATION_SENTINEL "\1\1\1"

/**
 * g_utf8_collate_key_for_filename:
 * @str: a UTF-8 encoded string.
 * @len: length of @str, in bytes, or -1 if @str is nul-terminated.
 *
 * Converts a string into a collation key that can be compared
 * with other collation keys produced by the same function using strcmp().
 *
 * In order to sort filenames correctly, this function treats the dot '.'
 * as a special case. Most dictionary orderings seem to consider it
 * insignificant, thus producing the ordering "event.c" "eventgenerator.c"
 * "event.h" instead of "event.c" "event.h" "eventgenerator.c". Also, we
 * would like to treat numbers intelligently so that "file1" "file10" "file5"
 * is sorted as "file1" "file5" "file10".
 *
 * Return value: a newly allocated string. This string should
 *   be freed with g_free() when you are done with it.
 *
 * Since: 2.8
 */
static gchar *g_utf8_collate_key_for_filename   (const gchar *str,
                                                 gssize       len)
{
    GString *result;
    GString *append;
    const gchar *p;
    const gchar *prev;
    gchar *collate_key;
    gint digits;
    gint leading_zeros;

  /*
   * How it works:
    *
    * Split the filename into collatable substrings which do
    * not contain [.0-9] and special-cased substrings. The collatable
    * substrings are run through the normal g_utf8_collate_key() and the
    * resulting keys are concatenated with keys generated from the
    * special-cased substrings.
    *
    * Special cases: Dots are handled by replacing them with '\1' which
    * implies that short dot-delimited substrings are before long ones,
    * e.g.
    *
    *   a\1a   (a.a)
    *   a-\1a  (a-.a)
    *   aa\1a  (aa.a)
    *
    * Numbers are handled by prepending to each number d-1 superdigits
    * where d = number of digits in the number and SUPERDIGIT is a
    * character with an integer value higher than any digit (for instance
    * ':'). This ensures that single-digit numbers are sorted before
    * double-digit numbers which in turn are sorted separately from
    * triple-digit numbers, etc. To avoid strange side-effects when
    * sorting strings that already contain SUPERDIGITs, a '\2'
    * is also prepended, like this
    *
    *   file\21      (file1)
    *   file\25      (file5)
    *   file\2:10    (file10)
    *   file\2:26    (file26)
    *   file\2::100  (file100)
    *   file:foo     (file:foo)
    *
    * This has the side-effect of sorting numbers before everything else (except
    * dots), but this is probably OK.
    *
    * Leading digits are ignored when doing the above. To discriminate
    * numbers which differ only in the number of leading digits, we append
    * the number of leading digits as a byte at the very end of the collation
    * key.
    *
    * To try avoid conflict with any collation key sequence generated by libc we
    * start each switch to a special cased part with a sentinel that hopefully
    * will sort before anything libc will generate.
  */

    if (len < 0)
        len = strlen (str);

    result = g_string_sized_new (len * 2);
    append = g_string_sized_new (0);

    /* No need to use utf8 functions, since we're only looking for ascii chars */
    for (prev = p = str; *p != '\0'; p++)
    {
        switch (*p)
        {
            case '.':
                if (prev != p)
                {
                    collate_key = g_utf8_collate_key (prev, p - prev);
                    g_string_append (result, collate_key);
                    g_free (collate_key);
                }

                g_string_append (result, COLLATION_SENTINEL "\1");

                /* skip the dot */
                prev = p + 1;
                break;

            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                if (prev != p)
                {
                    collate_key = g_utf8_collate_key (prev, p - prev);
                    g_string_append (result, collate_key);
                    g_free (collate_key);
                }

                g_string_append (result, COLLATION_SENTINEL "\2");

                prev = p;

                /* write d-1 colons */
                if (*p == '0')
                {
                    leading_zeros = 1;
                    digits = 0;
                }
                else
                {
                    leading_zeros = 0;
                    digits = 1;
                }

                do
                {
                    p++;

                    if (*p == '0' && !digits)
                        ++leading_zeros;
                    else if (g_ascii_isdigit(*p))
                        ++digits;
                    else
                        break;
                }
                while (*p != '\0');

                while (digits > 1)
                {
                    g_string_append_c (result, ':');
                    --digits;
                }

                if (leading_zeros > 0)
                {
                    g_string_append_c (append, (char)leading_zeros);
                    prev += leading_zeros;
                }

                /* write the number itself */
                g_string_append_len (result, prev, p - prev);

                prev = p;
                --p;	  /* go one step back to avoid disturbing outer loop */
                break;

            default:
                /* other characters just accumulate */
                break;
        }
    }

    if (prev != p)
    {
        collate_key = g_utf8_collate_key (prev, p - prev);
        g_string_append (result, collate_key);
        g_free (collate_key);
    }

    g_string_append (result, append->str);
    g_string_free (append, TRUE);

    return g_string_free (result, FALSE);
}
#endif /* !GLIB_CHECK_VERSION(2,8,0) */


/***************************************************************************/

/* XXX check out IconTheme changes */

typedef enum {
    MOO_ICON_LINK       = 1 << 0,
    MOO_ICON_LOCK       = 1 << 1,
    MOO_ICON_FLAGS_LEN  = 1 << 2
} MooIconFlags;

typedef enum {
    MOO_ICON_MIME = 0,
    MOO_ICON_HOME,
    MOO_ICON_DESKTOP,
    MOO_ICON_TRASH,
    MOO_ICON_DIRECTORY,
    MOO_ICON_BROKEN_LINK,
    MOO_ICON_NONEXISTENT,
    MOO_ICON_BLOCK_DEVICE,
    MOO_ICON_CHARACTER_DEVICE,
    MOO_ICON_FIFO,
    MOO_ICON_SOCKET,
    MOO_ICON_FILE,
    MOO_ICON_BLANK,
    MOO_ICON_MAX
} MooIconType;

typedef struct {
    GdkPixbuf *data[MOO_ICON_FLAGS_LEN];
} MooIconVars;

typedef struct {
    MooIconVars *special_icons[MOO_ICON_MAX];
    GHashTable  *mime_icons;                    /* char* -> MooIconVars* */
} MooIconCache;

static MooIconCache *moo_icon_cache_new         (void);
static void          moo_icon_cache_free        (MooIconCache   *cache);
static MooIconVars  *moo_icon_cache_lookup      (MooIconCache   *cache,
                                                 MooIconType     icon,
                                                 const char     *mime_type);
static void          moo_icon_cache_insert      (MooIconCache   *cache,
                                                 MooIconType     icon,
                                                 const char     *mime_type,
                                                 MooIconVars    *pixbufs);
static MooIconVars  *moo_icon_vars_new          (void);

static GdkPixbuf    *_create_icon_simple        (GtkIconTheme   *icon_theme,
                                                 MooIconType     icon,
                                                 const char     *mime_type,
                                                 GtkWidget      *widget,
                                                 GtkIconSize     size);
static GdkPixbuf    *_create_icon_with_flags    (GdkPixbuf      *original,
                                                 MooIconFlags    flags,
                                                 GtkIconTheme   *icon_theme,
                                                 GtkWidget      *widget,
                                                 GtkIconSize     size);
static GdkPixbuf    *_create_icon_for_mime_type (GtkIconTheme   *icon_theme,
                                                 const char     *mime_type,
                                                 GtkWidget      *widget,
                                                 GtkIconSize     size,
                                                 int             pixel_size);
static GdkPixbuf    *_create_named_icon         (GtkIconTheme   *icon_theme,
                                                 const char     *name,
                                                 const char     *fallback_name,
                                                 const char     *fallback_stock,
                                                 GtkWidget      *widget,
                                                 GtkIconSize     size,
                                                 int             pixel_size);
static GdkPixbuf    *_create_broken_link_icon   (GtkIconTheme   *icon_theme,
                                                 GtkWidget      *widget,
                                                 GtkIconSize     size);
static GdkPixbuf    *_create_broken_icon        (GtkIconTheme   *icon_theme,
                                                 GtkWidget      *widget,
                                                 GtkIconSize     size);

static MooIconType   _get_folder_icon           (const char     *path);
static MooIconFlags  _get_icon_flags            (const MooFile  *file);


static GdkPixbuf *
_render_icon (MooIconType     icon,
              const char     *mime_type,
              MooIconFlags    flags,
              GtkWidget      *widget,
              GtkIconSize     size)
{
    GtkIconTheme *icon_theme;
    GHashTable *all_sizes_cache;
    MooIconCache *cache;
    MooIconVars *pixbufs;

    g_return_val_if_fail (flags < MOO_ICON_FLAGS_LEN,
                          _render_icon (icon, mime_type, 0, widget, size));

    icon_theme = gtk_icon_theme_get_for_screen (gtk_widget_get_screen (widget));
    all_sizes_cache = g_object_get_data (G_OBJECT (icon_theme), "moo-file-icon-cache");

    if (!all_sizes_cache)
    {
        all_sizes_cache =
                g_hash_table_new_full (g_direct_hash, g_direct_equal,
                                       NULL,
                                       (GDestroyNotify) moo_icon_cache_free);
        g_object_set_data_full (G_OBJECT (icon_theme),
                                "moo-file-icon-cache",
                                all_sizes_cache,
                                (GDestroyNotify) g_hash_table_destroy);
    }

    cache = g_hash_table_lookup (all_sizes_cache, GINT_TO_POINTER (size));

    if (!cache)
    {
        cache = moo_icon_cache_new ();
        g_hash_table_insert (all_sizes_cache, GINT_TO_POINTER (size), cache);
    }

    pixbufs = moo_icon_cache_lookup (cache, icon, mime_type);

    if (!pixbufs)
    {
        pixbufs = moo_icon_vars_new ();
        pixbufs->data[0] = _create_icon_simple (icon_theme, icon, mime_type, widget, size);
        g_assert (pixbufs->data[0] != NULL);
        moo_icon_cache_insert (cache, icon, mime_type, pixbufs);
    }

    if (!pixbufs->data[flags])
    {
        g_assert (flags != 0);
        g_assert (pixbufs->data[0] != NULL);
        pixbufs->data[flags] =
                _create_icon_with_flags (pixbufs->data[0], flags,
                                         icon_theme, widget, size);
        g_assert (pixbufs->data[flags] != NULL);
    }

    return pixbufs->data[flags];
}


static GdkPixbuf *
render_icon (const MooFile  *file,
             GtkWidget      *widget,
             GtkIconSize     size)
{
    GdkPixbuf *pixbuf = _render_icon (file->icon, file->mime_type,
                                      _get_icon_flags (file), widget, size);
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

    return _render_icon (MOO_ICON_MIME, mime_type, 0, widget, size);
}


static GdkPixbuf *
_create_icon_simple (GtkIconTheme   *icon_theme,
                     MooIconType     icon,
                     const char     *mime_type,
                     GtkWidget      *widget,
                     GtkIconSize     size)
{
    int width, height;
    GdkPixbuf *pixbuf;

    g_return_val_if_fail (icon < MOO_ICON_MAX, NULL);
    g_return_val_if_fail (icon != MOO_ICON_MIME || mime_type != NULL, NULL);

    if (!gtk_icon_size_lookup (size, &width, &height))
        if (!gtk_icon_size_lookup (GTK_ICON_SIZE_MENU, &width, &height))
            width = height = 16;

    switch (icon)
    {
        case MOO_ICON_MIME:
            pixbuf = _create_icon_for_mime_type (icon_theme, mime_type,
                                                 widget, size, width);

            if (pixbuf)
                return pixbuf;
            else
                return _create_icon_simple (icon_theme, MOO_ICON_FILE,
                                            NULL, widget, size);

        case MOO_ICON_HOME:
            return _create_named_icon (icon_theme,
                                       "gnome-fs-home",
                                       NULL,
                                       GTK_STOCK_HOME,
                                       widget, size, width);
        case MOO_ICON_DESKTOP:
            return _create_named_icon (icon_theme,
                                       "gnome-fs-desktop",
                                       "gnome-fs-directory",
                                       GTK_STOCK_DIRECTORY,
                                       widget, size, width);
        case MOO_ICON_TRASH:
            return _create_named_icon (icon_theme,
                                       "gnome-fs-trash-full",
                                       "gnome-fs-directory",
                                       GTK_STOCK_DIRECTORY,
                                       widget, size, width);
        case MOO_ICON_DIRECTORY:
            return _create_named_icon (icon_theme,
                                       "gnome-fs-directory",
                                       NULL,
                                       GTK_STOCK_DIRECTORY,
                                       widget, size, width);
        case MOO_ICON_BROKEN_LINK:
            return _create_broken_link_icon (icon_theme, widget, size);
        case MOO_ICON_NONEXISTENT:
            return _create_broken_icon (icon_theme, widget, size);
        case MOO_ICON_BLOCK_DEVICE:
            return _create_named_icon (icon_theme,
                                       "gnome-fs-blockdev",
                                       NULL,
                                       GTK_STOCK_HARDDISK,
                                       widget, size, width);
        case MOO_ICON_CHARACTER_DEVICE:
            return _create_named_icon (icon_theme,
                                       "gnome-fs-chardev",
                                       "gnome-fs-regular",
                                       GTK_STOCK_FILE,
                                       widget, size, width);
        case MOO_ICON_FIFO:
            return _create_named_icon (icon_theme,
                                       "gnome-fs-fifo",
                                       "gnome-fs-regular",
                                       GTK_STOCK_FILE,
                                       widget, size, width);
        case MOO_ICON_SOCKET:
            return _create_named_icon (icon_theme,
                                       "gnome-fs-socket",
                                       "gnome-fs-regular",
                                       GTK_STOCK_FILE,
                                       widget, size, width);
        case MOO_ICON_FILE:
            return _create_named_icon (icon_theme,
                                       "gnome-fs-regular",
                                       NULL,
                                       GTK_STOCK_FILE,
                                       widget, size, width);
        case MOO_ICON_BLANK:
            return _create_named_icon (icon_theme,
                                       "gnome-fs-regular",
                                       NULL,
                                       GTK_STOCK_FILE,
                                       widget, size, width);

        case MOO_ICON_MAX:
            g_return_val_if_reached (NULL);
    }

    g_return_val_if_reached (NULL);
}


static GdkPixbuf *
_create_named_icon (GtkIconTheme   *icon_theme,
                    const char     *name,
                    const char     *fallback_name,
                    const char     *fallback_stock,
                    GtkWidget      *widget,
                    GtkIconSize     size,
                    int             pixel_size)
{
    GdkPixbuf *pixbuf;

    g_return_val_if_fail (name != NULL, NULL);

    pixbuf = gtk_icon_theme_load_icon (icon_theme, name, pixel_size, 0, NULL);

    if (!pixbuf && 0)
        g_warning ("could not load '%s' icon", name);

    if (!pixbuf && fallback_name)
    {
        pixbuf = gtk_icon_theme_load_icon (icon_theme, fallback_name, pixel_size, 0, NULL);
        if (!pixbuf && 0)
            g_warning ("could not load '%s' icon", fallback_name);
    }

    if (!pixbuf && fallback_stock)
    {
        pixbuf = gtk_widget_render_icon (widget, fallback_stock, size, NULL);
        if (!pixbuf && 0)
            g_warning ("could not load stock '%s' icon", fallback_stock);
    }

    if (!pixbuf)
    {
        pixbuf = gtk_widget_render_icon (widget, GTK_STOCK_FILE, size, NULL);
        if (!pixbuf && 0)
            g_warning ("could not load stock '%s' icon", GTK_STOCK_FILE);
    }

    return pixbuf;
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


static GdkPixbuf *
_create_icon_for_mime_type (GtkIconTheme   *icon_theme,
                            const char     *mime_type,
                            GtkWidget      *widget,
                            GtkIconSize     size,
                            int             pixel_size)
{
    const char *separator;
    GString *icon_name;
    GdkPixbuf *pixbuf;
    char **parent_types;

    if (!strcmp (mime_type, MIME_TYPE_UNKNOWN))
        return _create_icon_simple (icon_theme, MOO_ICON_FILE, NULL,
                                    widget, size);

    separator = strchr (mime_type, '/');
    if (!separator)
    {
        g_warning ("%s: mime type '%s' is invalid",
                   G_STRLOC, mime_type);
        return _create_icon_simple (icon_theme, MOO_ICON_FILE, NULL,
                                    widget, size);
    }

    icon_name = g_string_new ("gnome-mime-");
    g_string_append_len (icon_name, mime_type, separator - mime_type);
    g_string_append_c (icon_name, '-');
    g_string_append (icon_name, separator + 1);
    pixbuf = gtk_icon_theme_load_icon (icon_theme, icon_name->str,
                                       pixel_size, 0, NULL);
    g_string_free (icon_name, TRUE);

    if (pixbuf)
        return pixbuf;

    icon_name = g_string_new ("gnome-mime-");
    g_string_append_len (icon_name, mime_type, separator - mime_type);
    pixbuf = gtk_icon_theme_load_icon (icon_theme, icon_name->str,
                                       pixel_size, 0, NULL);
    g_string_free (icon_name, TRUE);

    if (pixbuf)
        return pixbuf;

    icon_name = g_string_new_len (mime_type, separator - mime_type);
    g_string_append_c (icon_name, '-');
    g_string_append (icon_name, separator + 1);
    pixbuf = gtk_icon_theme_load_icon (icon_theme, icon_name->str,
                                       pixel_size, 0, NULL);
    g_string_free (icon_name, TRUE);

    if (pixbuf)
        return pixbuf;

    icon_name = g_string_new_len (mime_type, separator - mime_type);
    pixbuf = gtk_icon_theme_load_icon (icon_theme, icon_name->str,
                                       pixel_size, 0, NULL);
    g_string_free (icon_name, TRUE);

    if (pixbuf)
        return pixbuf;

#ifndef __WIN32__
    parent_types = xdg_mime_list_mime_parents (mime_type);

    if (parent_types && parent_types[0] && strcmp (parent_types[0], MIME_TYPE_UNKNOWN) != 0)
        pixbuf = _create_icon_for_mime_type (icon_theme, parent_types[0],
                                             widget, size, pixel_size);

    if (parent_types)
        free (parent_types);

    if (pixbuf)
        return pixbuf;
#endif

    _moo_message ("%s: could not find icon for mime type '%s'",
                  G_STRLOC, mime_type);
    return _create_icon_simple (icon_theme, MOO_ICON_FILE, NULL,
                                widget, size);
}


static GdkPixbuf *
_create_broken_link_icon (G_GNUC_UNUSED GtkIconTheme *icon_theme,
                          GtkWidget      *widget,
                          GtkIconSize     size)
{
    /* XXX */
    return gtk_widget_render_icon (widget, GTK_STOCK_MISSING_IMAGE, size, NULL);
}


static GdkPixbuf *
_create_broken_icon (G_GNUC_UNUSED GtkIconTheme *icon_theme,
                     GtkWidget      *widget,
                     GtkIconSize     size)
{
    /* XXX */
    return gtk_widget_render_icon (widget, GTK_STOCK_MISSING_IMAGE, size, NULL);
}


static GdkPixbuf *
_create_icon_with_flags (GdkPixbuf      *original,
                         MooIconFlags    flags,
                         G_GNUC_UNUSED GtkIconTheme *icon_theme,
                         G_GNUC_UNUSED GtkWidget *widget,
                         GtkIconSize     size)
{
    static GdkPixbuf *arrow = NULL, *small_arrow = NULL;
    int width, height;
    GdkPixbuf *pixbuf;

    /* XXX */
    g_assert (flags != 0);

    if (!gtk_icon_size_lookup (size, &width, &height))
    {
        width = gdk_pixbuf_get_width (original);
        height = gdk_pixbuf_get_height (original);
    }

    if (flags & MOO_ICON_LINK)
    {
        GdkPixbuf *emblem;

        if (!arrow)
        {
            arrow = gdk_pixbuf_new_from_inline (-1, SYMLINK_ARROW, TRUE, NULL);
            g_return_val_if_fail (arrow != NULL, g_object_ref (original));
        }

        if (!small_arrow)
        {
            small_arrow = gdk_pixbuf_new_from_inline (-1, SYMLINK_ARROW_SMALL, TRUE, NULL);
            g_return_val_if_fail (arrow != NULL, g_object_ref (original));
        }

        if (size == GTK_ICON_SIZE_MENU)
            emblem = small_arrow;
        else
            emblem = arrow;

        pixbuf = gdk_pixbuf_copy (original);
        g_return_val_if_fail (pixbuf != NULL, g_object_ref (original));

        gdk_pixbuf_composite (emblem, pixbuf,
                              0,
                              gdk_pixbuf_get_height (pixbuf) - gdk_pixbuf_get_height (emblem),
                              gdk_pixbuf_get_width (emblem),
                              gdk_pixbuf_get_height (emblem),
                              0,
                              gdk_pixbuf_get_height (pixbuf) - gdk_pixbuf_get_height (emblem),
                              1, 1,
                              GDK_INTERP_BILINEAR,
                              255);
    }
    else
    {
        pixbuf = g_object_ref (original);
    }

    return pixbuf;
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


static MooIconFlags
_get_icon_flags (const MooFile *file)
{
    return (MOO_FILE_IS_LOCKED (file) ? MOO_ICON_LOCK : 0) |
            (MOO_FILE_IS_LINK (file) ? MOO_ICON_LINK : 0);
}


static MooIconVars *
moo_icon_vars_new (void)
{
    return g_new0 (MooIconVars, 1);
}


static void
moo_icon_vars_free (MooIconVars *pixbufs)
{
    if (pixbufs)
    {
        guint i;

        for (i = 0; i < MOO_ICON_FLAGS_LEN; ++i)
        {
            if (pixbufs->data[i])
                g_object_unref (pixbufs->data[i]);
            pixbufs->data[i] = NULL;
        }

        g_free (pixbufs);
    }
}


static MooIconCache *
moo_icon_cache_new (void)
{
    MooIconCache *cache = g_new0 (MooIconCache, 1);
    cache->mime_icons = g_hash_table_new_full (g_str_hash, g_str_equal,
                                               g_free,
                                               (GDestroyNotify) moo_icon_vars_free);
    return cache;
}


static void
moo_icon_cache_free (MooIconCache *cache)
{
    if (cache)
    {
        guint i;

        for (i = 0; i < MOO_ICON_MAX; ++i)
        {
            moo_icon_vars_free (cache->special_icons[i]);
            cache->special_icons[i] = NULL;
        }

        g_hash_table_destroy (cache->mime_icons);
        g_free (cache);
    }
}


static MooIconVars *
moo_icon_cache_lookup (MooIconCache   *cache,
                       MooIconType     icon,
                       const char     *mime_type)
{
    if (icon != MOO_ICON_MIME)
    {
        g_assert (icon < MOO_ICON_MAX);
        return cache->special_icons[icon];
    }
    else
    {
        g_assert (mime_type != NULL);
        return g_hash_table_lookup (cache->mime_icons, mime_type);
    }
}


static void
moo_icon_cache_insert (MooIconCache   *cache,
                       MooIconType     icon,
                       const char     *mime_type,
                       MooIconVars    *pixbufs)
{
    if (icon != MOO_ICON_MIME)
    {
        g_assert (icon < MOO_ICON_MAX);
        g_assert (pixbufs != NULL);
        cache->special_icons[icon] = pixbufs;
    }
    else
    {
        g_assert (mime_type != NULL);
        g_assert (pixbufs != NULL);
        g_hash_table_insert (cache->mime_icons, g_strdup (mime_type), pixbufs);
    }
}
