/*
 *   mooutils/moofileutils.c
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

/* XXX remove all this junk */

#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif
#ifdef HAVE_ERRNO_H
# include <errno.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#include <gdk/gdk.h>
#include <string.h>
#ifdef GDK_WINDOWING_X11
# include <gdk/gdkx.h>
#endif
#include "mooutils/moofileutils.h"
#ifdef __WIN32__
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
# include <shellapi.h>
#endif
#if GLIB_CHECK_VERSION(2,6,0)
# include <glib/gstdio.h>
#endif

/* TODO TODO move this to moocompat.h and copy from glib */
int         moo_unlink              (const char *filename)
{
#if GLIB_CHECK_VERSION(2,6,0)
    return g_unlink (filename);
#elif HAVE_UNLINK
    return unlink (filename);
#else
#error "unlink() is absent"
#endif
}


/* TODO TODO remove this thing */
gboolean moo_save_file_utf8 (const char *name,
                             const char *text,
                             gssize      len,
                             GError    **error)
{
    GError *err = NULL;
    GIOChannel *file;
    gsize bytes_written;
    gsize real_len;

    file = g_io_channel_new_file (name, "w", &err);
    if (!file) {
        g_assert (err != NULL);
        if (error) *error = err;
        else g_error_free (err);
        return FALSE;
    }

    real_len = len < 0 ? strlen (text) : (gsize)len;
    g_io_channel_write_chars (file, text,
                              len, &bytes_written,
                              &err);

    if (bytes_written != real_len || err != NULL) {
        if (err) {
            if (error) *error = err;
            else g_error_free (err);
        }
        g_io_channel_shutdown (file, TRUE, NULL);
        g_io_channel_unref (file);
        return FALSE;
    }

    g_io_channel_shutdown (file, TRUE, NULL);
    g_io_channel_unref (file);
    return TRUE;
}


#ifdef __WIN32__

static gboolean open_uri (const char *uri, gboolean email/* = FALSE */)
{
    g_return_val_if_fail (uri != NULL, FALSE);
    HINSTANCE h = ShellExecute (NULL, "open", uri, NULL, NULL,
                                SW_SHOWNORMAL);
    return (int)h > 32;
}

#else /* ! __WIN32__ */


typedef enum {
    KDE,
    GNOME,
    DEBIAN,
    UNKNOWN
} Desktop;


static gboolean open_uri (const char *uri,
                          gboolean email /* = FALSE */)
{
    gboolean result = FALSE;
    char **argv = NULL;

    Desktop desktop = UNKNOWN;
    char *kfmclient = g_find_program_in_path ("kfmclient");
    char *gnome_open = g_find_program_in_path ("gnome-open");
    char *x_www_browser = g_find_program_in_path ("x-www-browser");

    if (!email &&
        g_file_test ("/etc/debian_version", G_FILE_TEST_EXISTS) &&
        x_www_browser)
            desktop = DEBIAN;

    if (desktop == UNKNOWN) {
        if (kfmclient && g_getenv ("KDE_FULL_SESSION")) desktop = KDE;
        else if (gnome_open && g_getenv ("GNOME_DESKTOP_SESSION_ID")) desktop = GNOME;
    }

#ifdef GDK_WINDOWING_X11
    if (desktop == UNKNOWN) {
        const char *wm =
            gdk_x11_screen_get_window_manager_name (gdk_screen_get_default ());
        if (wm) {
            if (!g_ascii_strcasecmp (wm, "kwin") && kfmclient)
                desktop = KDE;
            else if (!g_ascii_strcasecmp (wm, "metacity") && gnome_open)
                desktop = GNOME;
        }
    }
#endif /* GDK_WINDOWING_X11 */

    if (desktop == UNKNOWN) {
        if (!email && x_www_browser) desktop = DEBIAN;
        else if (kfmclient) desktop = KDE;
        else if (gnome_open) desktop = GNOME;
    }

    switch (desktop) {
        case KDE:
            argv = g_new0 (char*, 4);
            argv[0] = g_strdup (kfmclient);
            argv[1] = g_strdup ("exec");
            argv[2] = g_strdup (uri);
            break;

        case GNOME:
            argv = g_new0 (char*, 3);
            argv[0] = g_strdup (gnome_open);
            argv[1] = g_strdup (uri);
            break;

        case DEBIAN:
            argv = g_new0 (char*, 3);
            argv[0] = g_strdup (x_www_browser);
            argv[1] = g_strdup (uri);
            break;

        case UNKNOWN:
            if (uri)
                g_warning ("could not find a way to open uri '%s'", uri);
            else
                g_warning ("could not find a way to open uri");
            break;

        default:
            g_assert_not_reached ();
    }

    if (argv) {
        GError *err = NULL;
        result = g_spawn_async (NULL, argv, NULL, (GSpawnFlags)0, NULL, NULL,
                                NULL, &err);
        if (err) {
            g_warning ("%s: error in g_spawn_async", G_STRLOC);
            g_warning ("%s: %s", G_STRLOC, err->message);
            g_error_free (err);
        }
    }

    if (argv) g_strfreev (argv);
    g_free (gnome_open);
    g_free (kfmclient);
    g_free (x_www_browser);

    return result;
}

#endif /* ! __WIN32__ */


gboolean moo_open_email (const char *address,
                         const char *subject,
                         const char *body)
{
    GString *uri;
    gboolean res;

    g_return_val_if_fail (address != NULL, FALSE);
    uri = g_string_new ("mailto:");
    g_string_append_printf (uri, "%s%s", address,
                            subject || body ? "?" : "");
    if (subject)
        g_string_append_printf (uri, "subject=%s%s", subject,
                                body ? "&" : "");
    if (body)
        g_string_append_printf (uri, "body=%s", body);

    res = open_uri (uri->str, TRUE);
    g_string_free (uri, TRUE);
    return res;
}


gboolean moo_open_url (const char *url)
{
    return open_uri (url, FALSE);
}


#if 0
int      moo_gtimeval_compare   (GTimeVal   *val1,
                                 GTimeVal   *val2)
{
    g_return_val_if_fail (val1 != NULL && val2 != NULL, 0);

    if (val1->tv_sec > val2->tv_sec)
        return 1;
    else if (val1->tv_sec < val2->tv_sec)
        return -1;
    else if (val1->tv_usec > val2->tv_usec)
        return 1;
    else if (val1->tv_usec < val2->tv_usec)
        return -1;
    else
        return 0;
}


GTime       moo_get_file_mtime      (const char *filename)
{
    struct stat buf;

    g_return_val_if_fail (filename != NULL, MOO_EINVAL);

    if (stat (filename, &buf)) {
        int err = errno;
        g_warning ("%s: error in stat()", G_STRLOC);
        g_warning ("%s: %s", G_STRLOC, g_strerror (err));
        switch (err) {
            case ENOENT:
            case ENOTDIR:
            case ENAMETOOLONG:
#ifdef ELOOP
            case ELOOP:
#endif
                return MOO_ENOENT;

            case EACCES:
                return MOO_EACCESS;

            case EIO:
#ifdef ELOOP
            case EOVERFLOW:
#endif
                return MOO_EIO;
        }
    }

    return buf.st_mtime;
}
#endif
