/*
 *   mooeditfileops.c
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define MOOEDIT_COMPILATION
#include "mooedit/mooedit-private.h"
#include "mooedit/mooeditor-private.h"
#include "mooedit/mooeditfileops.h"
#include "mooedit/mooeditdialogs.h"
#include "mooedit/mootextbuffer.h"
#include "mooedit/mooeditprefs.h"
#include "moofileview/moofile.h"
#include "mooutils/moofilewatch.h"
#include "mooutils/mooencodings.h"
#include "mooutils/mooi18n.h"
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <stdio.h>
#include <errno.h>

#if GLIB_CHECK_VERSION(2,6,0)
# include <glib/gstdio.h>
#endif

#undef LOAD_BINARY
#define ENCODING_LOCALE "LOCALE"

#ifndef O_BINARY
#define O_BINARY 0
#endif

static GSList *UNTITLED = NULL;
static GHashTable *UNTITLED_NO = NULL;

static void     block_buffer_signals        (MooEdit        *edit);
static void     unblock_buffer_signals      (MooEdit        *edit);
static gboolean focus_in_cb                 (MooEdit        *edit);
static void     check_file_status           (MooEdit        *edit,
                                             gboolean        in_focus_only);
static void     file_modified_on_disk       (MooEdit        *edit);
static void     file_deleted                (MooEdit        *edit);
static void     add_status                  (MooEdit        *edit,
                                             MooEditStatus   s);

static gboolean moo_edit_load_local         (MooEdit        *edit,
                                             const char     *file,
                                             const char     *encoding,
                                             GError        **error);
static gboolean moo_edit_reload_local       (MooEdit        *edit,
                                             const char     *encoding,
                                             GError        **error);
static gboolean moo_edit_save_local         (MooEdit        *edit,
                                             const char     *filename,
                                             const char     *encoding,
                                             MooEditSaveFlags flags,
                                             GError        **error);
static gboolean moo_edit_save_copy_local    (MooEdit        *edit,
                                             const char     *filename,
                                             const char     *encoding,
                                             GError        **error);
static char    *_moo_edit_filename_to_utf8  (const char     *filename);
static void     _moo_edit_start_file_watch  (MooEdit        *edit);


GQuark
_moo_edit_file_error_quark (void)
{
    static GQuark q;

    if (!q)
        q = g_quark_from_static_string ("MooEditFileErrorQuark");

    return q;
}


static const char *
normalize_encoding (const char *encoding,
                    gboolean    for_save)
{
    if (!encoding || !encoding[0] || !strcmp (encoding, MOO_ENCODING_AUTO))
        encoding = for_save ? MOO_ENCODING_UTF8 : NULL;
    return encoding;
}


gboolean
_moo_edit_load_file (MooEdit        *edit,
                     const char     *filename,
                     const char     *encoding,
                     GError        **error)
{
    char *filename_copy, *encoding_copy;
    gboolean result;
    GError *error_here = NULL;

    g_return_val_if_fail (MOO_IS_EDIT (edit), FALSE);
    g_return_val_if_fail (filename != NULL, FALSE);

    filename_copy = g_strdup (filename);
    encoding_copy = g_strdup (normalize_encoding (encoding, FALSE));

    result = moo_edit_load_local (edit, filename_copy, encoding_copy, &error_here);

    if (error_here)
        g_propagate_error (error, error_here);

    g_free (filename_copy);
    g_free (encoding_copy);
    return result;
}


gboolean
_moo_edit_reload_file (MooEdit    *edit,
                       const char *encoding,
                       GError    **error)
{
    GError *error_here = NULL;
    gboolean result;

    g_return_val_if_fail (MOO_IS_EDIT (edit), FALSE);

    result = moo_edit_reload_local (edit, encoding, &error_here);

    if (error_here)
        g_propagate_error (error, error_here);

    return result;
}


gboolean
_moo_edit_save_file (MooEdit        *edit,
                     const char     *filename,
                     const char     *encoding,
                     MooEditSaveFlags flags,
                     GError        **error)
{
    GError *error_here = NULL;
    char *filename_copy, *encoding_copy;
    gboolean result;

    g_return_val_if_fail (MOO_IS_EDIT (edit), FALSE);
    g_return_val_if_fail (filename != NULL, FALSE);

    filename_copy = g_strdup (filename);
    encoding_copy = g_strdup (normalize_encoding (encoding, TRUE));

    result = moo_edit_save_local (edit, filename_copy, encoding_copy, flags, &error_here);

    if (error_here)
        g_propagate_error (error, error_here);

    g_free (filename_copy);
    g_free (encoding_copy);
    return result;
}


gboolean
_moo_edit_save_file_copy (MooEdit        *edit,
                          const char     *filename,
                          const char     *encoding,
                          GError        **error)
{
    char *filename_copy, *encoding_copy;
    gboolean result;

    g_return_val_if_fail (MOO_IS_EDIT (edit), FALSE);
    g_return_val_if_fail (filename != NULL, FALSE);

    filename_copy = g_strdup (filename);
    encoding_copy = g_strdup (normalize_encoding (encoding, TRUE));

    result = moo_edit_save_copy_local (edit, filename_copy, encoding_copy, error);

    g_free (filename_copy);
    g_free (encoding_copy);
    return result;
}


static void
set_encoding_error (GError **error)
{
    GError *tmp_error = NULL;
    g_set_error (&tmp_error, MOO_EDIT_FILE_ERROR,
                 MOO_EDIT_FILE_ERROR_ENCODING,
                 "%s", *error ? (*error)->message : "ERROR");
    g_clear_error (error);
    g_propagate_error (error, tmp_error);
}


/***************************************************************************/
/* File loading
 */

typedef enum {
    SUCCESS,
    ERROR_FILE,
    ERROR_ENCODING
} LoadResult;

static LoadResult   do_load     (MooEdit            *edit,
                                 const char         *file,
                                 const char         *encoding,
                                 struct stat        *statbuf,
                                 GError            **error);
#ifdef LOAD_BINARY
static LoadResult   load_binary (MooEdit            *edit,
                                 const char         *file,
                                 GError            **error);
#endif

const char *
_moo_get_default_encodings (void)
{
    /* Translators: if translated, it should be a comma-separated list
       of encodings to try when opening files. Encodings names should be
       those understood by iconv, or "LOCALE" which means user's locale
       charset. For instance, the default value is "UTF-8,LOCALE,ISO_8859-15,ISO_8859-1".
       You want to add common preferred non-UTF8 encodings used in your locale.
       Do not remove ISO_8859-15 and ISO_8859-1, instead leave them at the end,
       these are common source files encodings. */
    const char *to_translate = N_("encodings_list");
    const char *encodings;

    encodings = _(to_translate);

    if (!strcmp (encodings, to_translate))
        encodings = "UTF-8," ENCODING_LOCALE ",ISO_8859-1,ISO_8859-15";

    return encodings;
}

static GSList *
get_encodings (void)
{
    const char *encodings;
    char **raw, **p;
    GSList *result;

    encodings = moo_prefs_get_string (moo_edit_setting (MOO_EDIT_PREFS_ENCODINGS));

    if (!encodings || !encodings[0])
        encodings = _moo_get_default_encodings ();

    result = NULL;
    raw = g_strsplit (encodings, ",", 0);

    for (p = raw; p && *p; ++p)
    {
        const char *enc;

        if (!g_ascii_strcasecmp (*p, ENCODING_LOCALE))
        {
            if (g_get_charset (&enc))
                enc = "UTF-8";
        }
        else
        {
            enc = *p;
        }

        if (!g_slist_find_custom (result, enc, (GCompareFunc) g_ascii_strcasecmp))
            result = g_slist_prepend (result, g_strdup (enc));
    }

    if (!result)
    {
        g_critical ("%s: oops", G_STRLOC);
        result = g_slist_prepend (NULL, g_strdup ("UTF-8"));
    }

    g_strfreev (raw);
    return g_slist_reverse (result);
}


static LoadResult
try_load (MooEdit            *edit,
          const char         *file,
          const char         *encoding,
          struct stat        *statbuf,
          GError            **error)
{
    GtkTextBuffer *buffer;
    gboolean enable_highlight;
    LoadResult result;

    g_return_val_if_fail (MOO_IS_EDIT (edit), FALSE);
    g_return_val_if_fail (file && file[0], FALSE);
    g_return_val_if_fail (encoding && encoding[0], FALSE);

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (edit));
    gtk_text_buffer_set_text (buffer, "", 0);

    g_object_get (edit, "enable-highlight", &enable_highlight, NULL);
    g_object_set (edit, "enable-highlight", FALSE, NULL);
    result = do_load (edit, file, encoding, statbuf, error);
    g_object_set (edit, "enable-highlight", enable_highlight, NULL);

    return result;
}


static gboolean
moo_edit_load_local (MooEdit     *edit,
                     const char  *file,
                     const char  *encoding,
                     GError     **error)
{
    GtkTextIter start;
    GtkTextBuffer *buffer;
    MooTextView *view;
    gboolean undo;
    LoadResult result = ERROR_FILE;
    struct stat statbuf;
    char *freeme = NULL;

    g_return_val_if_fail (MOO_IS_EDIT (edit), FALSE);
    g_return_val_if_fail (file && file[0], FALSE);

    if (moo_edit_is_empty (edit))
        undo = FALSE;
    else
        undo = TRUE;

    view = MOO_TEXT_VIEW (edit);
    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (edit));

    block_buffer_signals (edit);

    if (undo)
        gtk_text_buffer_begin_user_action (buffer);
    else
        moo_text_view_begin_not_undoable_action (view);

    moo_text_buffer_begin_non_interactive_action (MOO_TEXT_BUFFER (buffer));

    if (!encoding)
    {
        GSList *encodings;

        encodings = get_encodings ();
        result = ERROR_ENCODING;

        while (encodings)
        {
            char *enc = encodings->data;

            enc = encodings->data;
            encodings = g_slist_delete_link (encodings, encodings);

            g_clear_error (error);
            result = try_load (edit, file, enc, &statbuf, error);

            if (result != ERROR_ENCODING)
            {
                encoding = freeme = enc;
                break;
            }

            g_free (enc);
        }

        g_slist_foreach (encodings, (GFunc) g_free, NULL);
        g_slist_free (encodings);
    }
    else
    {
        result = try_load (edit, file, encoding, &statbuf, error);
    }

#ifdef LOAD_BINARY
    if (result == ERROR_ENCODING)
    {
        g_clear_error (error);
        result = load_binary (edit, file, &statbuf, error);
    }
#endif

    if (result == ERROR_ENCODING)
        set_encoding_error (error);

    if (result != SUCCESS)
        gtk_text_buffer_set_text (buffer, "", 0);

    unblock_buffer_signals (edit);

    if (result == SUCCESS)
    {
        /* XXX */
        gtk_text_buffer_get_start_iter (buffer, &start);
        gtk_text_buffer_place_cursor (buffer, &start);
        edit->priv->status = 0;
        moo_edit_set_modified (edit, FALSE);
        _moo_edit_set_filename (edit, file, encoding);
        edit->priv->mode = statbuf.st_mode;
        edit->priv->mode_set = TRUE;
        _moo_edit_start_file_watch (edit);
    }
    else
    {
        _moo_edit_stop_file_watch (edit);
    }

    if (undo)
        gtk_text_buffer_end_user_action (buffer);
    else
        moo_text_view_end_not_undoable_action (view);

    moo_text_buffer_end_non_interactive_action (MOO_TEXT_BUFFER (buffer));

    g_free (freeme);
    return result == SUCCESS;
}


static LoadResult
do_load (MooEdit            *edit,
         const char         *filename,
         const char         *encoding,
         struct stat        *statbuf,
         GError            **error)
{
    GIOChannel *file = NULL;
    GIOStatus status;
    GtkTextBuffer *buffer;
    MooEditLineEndType le = MOO_EDIT_LINE_END_NONE;
    GString *text = NULL;
    char *line = NULL;
    LoadResult result = ERROR_FILE;

    g_return_val_if_fail (filename != NULL, ERROR_FILE);
    g_return_val_if_fail (encoding != NULL, ERROR_FILE);

    if (g_stat (filename, statbuf) == 0)
    {
        if (
#ifdef S_ISBLK
            S_ISBLK (statbuf->st_mode) ||
#endif
#ifdef S_ISCHR
            S_ISCHR (statbuf->st_mode) ||
#endif
#ifdef S_ISFIFO
            S_ISFIFO (statbuf->st_mode) ||
#endif
#ifdef S_ISSOCK
            S_ISSOCK (statbuf->st_mode) ||
#endif
            0)
        {
            g_set_error (error, G_FILE_ERROR,
                         G_FILE_ERROR_FAILED,
                         "Not a regular file.");
            return ERROR_FILE;
        }
    }

    file = g_io_channel_new_file (filename, "r", error);

    if (!file)
        return ERROR_FILE;

    if (g_io_channel_set_encoding (file, encoding, error) != G_IO_STATUS_NORMAL)
    {
        result = ERROR_ENCODING;
        goto error_out;
    }

    text = g_string_new (NULL);
    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (edit));

    while (TRUE)
    {
        gboolean insert_line_term = FALSE;
        gsize len, line_term_pos;
        MooEditLineEndType le_here = 0;

        status = g_io_channel_read_line (file, &line, &len, &line_term_pos, error);

        if (status != G_IO_STATUS_NORMAL && status != G_IO_STATUS_EOF)
        {
            if ((*error)->domain == G_CONVERT_ERROR)
                result = ERROR_ENCODING;
            else
                result = ERROR_FILE;
            goto error_out;
        }

        if (line)
        {
            if (!g_utf8_validate (line, len, NULL))
            {
                result = ERROR_ENCODING;
                g_set_error (error, G_CONVERT_ERROR,
                             G_CONVERT_ERROR_ILLEGAL_SEQUENCE,
                             "Invalid UTF8 data read from file");
                goto error_out;
            }

            if (line_term_pos != len)
            {
                char *line_term = line + line_term_pos;

                insert_line_term = TRUE;
                len = line_term_pos;

                if (!strcmp ("\xe2\x80\xa9", line_term))
                {
                    insert_line_term = FALSE;
                    len = len + 3;
                }
                else if (!strcmp (line_term, "\r"))
                {
                    le_here = MOO_EDIT_LINE_END_MAC;
                }
                else if (!strcmp (line_term, "\n"))
                {
                    le_here = MOO_EDIT_LINE_END_UNIX;
                }
                else if (!strcmp (line_term, "\r\n"))
                {
                    le_here = MOO_EDIT_LINE_END_WIN32;
                }

                if (le_here)
                {
                    if (le && le != le_here)
                        le = MOO_EDIT_LINE_END_MIX;
                    else
                        le = le_here;
                }
            }

            g_string_append_len (text, line, len);

            if (insert_line_term)
                g_string_append_c (text, '\n');

#define MAX_TEXT_BUF_LEN 1000000
            if (text->len > MAX_TEXT_BUF_LEN)
            {
                gtk_text_buffer_insert_at_cursor (buffer, text->str, text->len);
                g_string_truncate (text, 0);
            }
#undef MAX_TEXT_BUF_LEN

            g_free (line);
        }

        if (status == G_IO_STATUS_EOF)
        {
            g_io_channel_shutdown (file, TRUE, NULL);
            g_io_channel_unref (file);
            break;
        }
    }

    if (text->len)
        gtk_text_buffer_insert_at_cursor (buffer, text->str, text->len);

    edit->priv->line_end_type = le;

    g_string_free (text, TRUE);
    g_clear_error (error);
    return SUCCESS;

error_out:
    if (text)
        g_string_free (text, TRUE);

    if (file)
    {
        g_io_channel_shutdown (file, TRUE, NULL);
        g_io_channel_unref (file);
    }

    g_free (line);
    return result;
}


#ifdef LOAD_BINARY
#define TEXT_BUF_LEN (1 << 16)
#define REPLACEMENT_CHAR '\1'
#define LINE_LEN 80
static LoadResult
load_binary (MooEdit     *edit,
             const char  *filename,
             GError     **error)
{
    GIOChannel *file = NULL;
    GIOStatus status;
    GtkTextBuffer *buffer;

    g_return_val_if_fail (filename != NULL, FALSE);

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (edit));

    file = g_io_channel_new_file (filename, "r", error);

    if (!file)
        return ERROR_FILE;

    g_io_channel_set_encoding (file, NULL, NULL);

    while (TRUE)
    {
        gsize len, line_len;
        char buf[TEXT_BUF_LEN];

        status = g_io_channel_read_chars (file, buf, TEXT_BUF_LEN, &len, error);

        if (status != G_IO_STATUS_NORMAL && status != G_IO_STATUS_EOF)
            goto error_out;

        if (len)
        {
            guint i;

            for (i = 0, line_len = 0; i < len; ++i, ++line_len)
            {
                if (buf[i] && !(buf[i] & 0x80))
                    continue;

                if (line_len > LINE_LEN)
                {
                    buf[i] = '\n';
                    line_len = 0;
                }
                else
                {
                    buf[i] = REPLACEMENT_CHAR;
                }
            }

            gtk_text_buffer_insert_at_cursor (buffer, buf, len);
        }

        if (status == G_IO_STATUS_EOF)
        {
            g_io_channel_shutdown (file, TRUE, NULL);
            g_io_channel_unref (file);
            break;
        }
    }

    g_clear_error (error);
    return SUCCESS;

error_out:
    if (file)
    {
        g_io_channel_shutdown (file, TRUE, NULL);
        g_io_channel_unref (file);
    }

    return ERROR_FILE;
}
#undef TEXT_BUF_LEN
#undef REPLACEMENT_CHAR
#undef LINE_LEN
#endif


/* XXX */
static gboolean
moo_edit_reload_local (MooEdit    *edit,
                       const char *encoding,
                       GError    **error)
{
    GtkTextIter start, end;
    GtkTextBuffer *buffer;
    gboolean result, enable_highlight;

    g_return_val_if_fail (edit->priv->filename != NULL, FALSE);

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (edit));

    block_buffer_signals (edit);
    gtk_text_buffer_begin_user_action (buffer);

    gtk_text_buffer_get_bounds (buffer, &start, &end);
    gtk_text_buffer_delete (buffer, &start, &end);
    g_object_get (edit, "enable-highlight", &enable_highlight, NULL);
    g_object_set (edit, "enable-highlight", FALSE, NULL);

    result = _moo_edit_load_file (edit, edit->priv->filename,
                                  encoding ? encoding : edit->priv->encoding,
                                  error);

    g_object_set (edit, "enable-highlight", enable_highlight, NULL);
    gtk_text_buffer_end_user_action (buffer);
    unblock_buffer_signals (edit);

    if (result)
    {
        edit->priv->status = 0;
        moo_edit_set_modified (edit, FALSE);
        _moo_edit_start_file_watch (edit);
        g_clear_error (error);
    }

    return result;
}


/***************************************************************************/
/* File saving
 */

#ifdef __WIN32__
#define BAK_SUFFIX ".bak"
#define DEFAULT_LE_TYPE MOO_EDIT_LINE_END_WIN32
#else
#define BAK_SUFFIX "~"
#define DEFAULT_LE_TYPE MOO_EDIT_LINE_END_UNIX
#endif

static gboolean do_write                (MooEdit        *edit,
                                         const char     *file,
                                         const char     *encoding,
                                         GError        **error);


static gboolean
do_save_local (MooEdit        *edit,
               const char     *filename,
               const char     *encoding,
               GError        **error,
               gboolean       *retval)
{
    *retval = TRUE;

    if (!do_write (edit, filename, encoding, error))
    {
        if ((*error)->domain != G_CONVERT_ERROR ||
            _moo_encodings_equal (encoding, MOO_ENCODING_UTF8))
        {
            g_clear_error (error);

            if (!do_write (edit, filename, MOO_ENCODING_UTF8, error))
            {
                *retval = FALSE;
                return FALSE;
            }
        }

        *retval = FALSE;
        set_encoding_error (error);
    }

    return TRUE;
}


static gboolean
moo_edit_save_local (MooEdit        *edit,
                     const char     *filename,
                     const char     *encoding,
                     MooEditSaveFlags flags,
                     GError        **error)
{
    gboolean result;

    g_return_val_if_fail (MOO_IS_EDIT (edit), FALSE);
    g_return_val_if_fail (filename && filename[0], FALSE);

    if (do_save_local (edit, filename, encoding, error, &result))
    {
        edit->priv->status = 0;
        _moo_edit_set_filename (edit, filename,
                                result ? encoding : MOO_ENCODING_UTF8);
        moo_edit_set_modified (edit, FALSE);
        _moo_edit_start_file_watch (edit);
    }

    return result;
}


static gboolean
moo_edit_save_copy_local (MooEdit        *edit,
                          const char     *filename,
                          const char     *encoding,
                          GError        **error)
{
    gboolean result;
    g_return_val_if_fail (MOO_IS_EDIT (edit), FALSE);
    g_return_val_if_fail (filename && filename[0], FALSE);
    do_save_local (edit, filename, encoding, error, &result);
    return result;
}


static gboolean
do_write (MooEdit        *edit,
          const char     *filename,
          const char     *encoding,
          GError        **error)
{
    int fd = -1;
    GIOChannel *file;
    GIOStatus status;
    GtkTextIter line_start;
    GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (edit));
    MooEditLineEndType le_type = DEFAULT_LE_TYPE;
    const char *le = "\n";
    gssize le_len = 1;
    mode_t mode;

    g_return_val_if_fail (filename != NULL, FALSE);

    if (encoding && (!g_ascii_strcasecmp (encoding, "UTF-8") || !g_ascii_strcasecmp (encoding, "UTF8")))
        encoding = NULL;

#ifdef __WIN32__
    mode = S_IRUSR | S_IWUSR;
#else
    if (edit->priv->mode_set)
        mode = (edit->priv->mode & (S_IRWXU | S_IRWXG | S_IRWXO)) | S_IRUSR | S_IWUSR;
    else
        mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
#endif

    errno = 0;
    fd = g_open (filename, O_BINARY | O_WRONLY | O_CREAT | O_TRUNC, mode);

    if (fd == -1)
    {
        int err = errno;
        g_set_error (error, G_FILE_ERROR,
                     g_file_error_from_errno (err),
                     "%s", g_strerror (err));
        return FALSE;
    }

#ifndef __WIN32__
    file = g_io_channel_unix_new (fd);
#else
    file = g_io_channel_win32_new_fd (fd);
#endif

    g_io_channel_set_close_on_unref (file, TRUE);

    if (encoding && g_io_channel_set_encoding (file, encoding, error) != G_IO_STATUS_NORMAL)
    {
        g_io_channel_shutdown (file, TRUE, NULL);
        g_io_channel_unref (file);
        return FALSE;
    }

    switch (edit->priv->line_end_type)
    {
        case MOO_EDIT_LINE_END_NONE:
            le_type = DEFAULT_LE_TYPE;
            break;

        case MOO_EDIT_LINE_END_UNIX:
        case MOO_EDIT_LINE_END_WIN32:
        case MOO_EDIT_LINE_END_MAC:
            le_type = edit->priv->line_end_type;
            break;

        case MOO_EDIT_LINE_END_MIX:
            edit->priv->line_end_type = le_type = DEFAULT_LE_TYPE;
            break;
    }

    switch (le_type)
    {
        case MOO_EDIT_LINE_END_UNIX:
            le = "\n";
            le_len = 1;
            break;
        case MOO_EDIT_LINE_END_WIN32:
            le = "\r\n";
            le_len = 2;
            break;
        case MOO_EDIT_LINE_END_MAC:
            le = "\r";
            le_len = 1;
            break;
        case MOO_EDIT_LINE_END_MIX:
        case MOO_EDIT_LINE_END_NONE:
            g_assert_not_reached ();
    }

    gtk_text_buffer_get_start_iter (buffer, &line_start);

    do
    {
        gsize written;
        GtkTextIter line_end = line_start;

        if (!gtk_text_iter_ends_line (&line_start))
        {
            char *line;
            gssize len = -1;

            gtk_text_iter_forward_to_line_end (&line_end);
            line = gtk_text_buffer_get_slice (buffer, &line_start, &line_end, TRUE);

            status = g_io_channel_write_chars (file, line, len, &written, error);
            g_free (line);

            /* XXX */
            if (status != G_IO_STATUS_NORMAL)
            {
                g_io_channel_shutdown (file, TRUE, NULL);
                g_io_channel_unref (file);
                return FALSE;
            }
        }

        if (!gtk_text_iter_is_end (&line_end))
        {
            status = g_io_channel_write_chars (file, le, le_len, &written, error);

            if (written < (gsize)le_len || status != G_IO_STATUS_NORMAL)
            {
                g_io_channel_shutdown (file, TRUE, NULL);
                g_io_channel_unref (file);
                return FALSE;
            }
        }
    }
    while (gtk_text_iter_forward_line (&line_start));

    /* glib #320668 */
    status = g_io_channel_flush (file, error);

    if (status != G_IO_STATUS_NORMAL)
    {
        g_io_channel_shutdown (file, FALSE, NULL);
        g_io_channel_unref (file);
        return FALSE;
    }

    status = g_io_channel_shutdown (file, TRUE, error);

    if (status != G_IO_STATUS_NORMAL)
    {
        g_io_channel_unref (file);
        return FALSE;
    }

    g_io_channel_unref (file);
    g_clear_error (error);
    return TRUE;
}


/***************************************************************************/
/* Aux stuff
 */

static void
block_buffer_signals (MooEdit *edit)
{
    GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (edit));
    g_signal_handler_block (buffer, edit->priv->modified_changed_handler_id);
}


static void
unblock_buffer_signals (MooEdit *edit)
{
    GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (edit));
    g_signal_handler_unblock (buffer, edit->priv->modified_changed_handler_id);
}


static void
file_watch_callback (G_GNUC_UNUSED MooFileWatch *watch,
                     MooFileEvent  *event,
                     gpointer       data)
{
    MooEdit *edit = data;

    g_return_if_fail (MOO_IS_EDIT (data));
    g_return_if_fail (event->monitor_id == edit->priv->file_monitor_id);
    g_return_if_fail (edit->priv->filename != NULL);
    g_return_if_fail (!(edit->priv->status & MOO_EDIT_CHANGED_ON_DISK));

    switch (event->code)
    {
        case MOO_FILE_EVENT_CHANGED:
            edit->priv->modified_on_disk = TRUE;
            break;

        case MOO_FILE_EVENT_DELETED:
            edit->priv->deleted_from_disk = TRUE;
            edit->priv->file_monitor_id = 0;
            break;

        case MOO_FILE_EVENT_ERROR:
            /* XXX and what to do now? */
            break;

        case MOO_FILE_EVENT_CREATED:
            g_critical ("%s: oops", G_STRLOC);
            break;
    }

    check_file_status (edit, FALSE);
}


static void
_moo_edit_start_file_watch (MooEdit *edit)
{
    MooFileWatch *watch;
    GError *error = NULL;

    watch = _moo_editor_get_file_watch (edit->priv->editor);
    g_return_if_fail (watch != NULL);

    if (edit->priv->file_monitor_id)
        moo_file_watch_cancel_monitor (watch, edit->priv->file_monitor_id);
    edit->priv->file_monitor_id = 0;

    g_return_if_fail ((edit->priv->status & MOO_EDIT_CHANGED_ON_DISK) == 0);
    g_return_if_fail (edit->priv->filename != NULL);

    edit->priv->file_monitor_id =
        moo_file_watch_create_monitor (watch, edit->priv->filename,
                                       file_watch_callback,
                                       edit, NULL, &error);

    if (!edit->priv->file_monitor_id)
    {
        g_warning ("%s: could not start watch for '%s'",
                   G_STRLOC, edit->priv->filename);

        if (error)
        {
            g_warning ("%s: %s", G_STRLOC, error->message);
            g_error_free (error);
        }

        return;
    }

    if (!edit->priv->focus_in_handler_id)
        edit->priv->focus_in_handler_id =
                g_signal_connect (edit, "focus-in-event",
                                  G_CALLBACK (focus_in_cb),
                                  NULL);
}


void
_moo_edit_stop_file_watch (MooEdit *edit)
{
    MooFileWatch *watch;

    watch = _moo_editor_get_file_watch (edit->priv->editor);
    g_return_if_fail (watch != NULL);

    if (edit->priv->file_monitor_id)
        moo_file_watch_cancel_monitor (watch, edit->priv->file_monitor_id);
    edit->priv->file_monitor_id = 0;

    if (edit->priv->focus_in_handler_id)
    {
        g_signal_handler_disconnect (edit, edit->priv->focus_in_handler_id);
        edit->priv->focus_in_handler_id = 0;
    }
}


static gboolean
focus_in_cb (MooEdit *edit)
{
    check_file_status (edit, TRUE);
    return FALSE;
}


static void
check_file_status (MooEdit        *edit,
                   gboolean        in_focus_only)
{
    if (in_focus_only && !GTK_WIDGET_HAS_FOCUS (edit))
        return;

    g_return_if_fail (edit->priv->filename != NULL);
    g_return_if_fail (!(edit->priv->status & MOO_EDIT_CHANGED_ON_DISK));

    if (edit->priv->deleted_from_disk)
        file_deleted (edit);
    else if (edit->priv->modified_on_disk)
        file_modified_on_disk (edit);
}


static void
file_modified_on_disk (MooEdit *edit)
{
    g_return_if_fail (edit->priv->filename != NULL);
    edit->priv->modified_on_disk = FALSE;
    edit->priv->deleted_from_disk = FALSE;
    _moo_edit_stop_file_watch (edit);
    add_status (edit, MOO_EDIT_MODIFIED_ON_DISK);
}


static void
file_deleted (MooEdit *edit)
{
    g_return_if_fail (edit->priv->filename != NULL);
    edit->priv->modified_on_disk = FALSE;
    edit->priv->deleted_from_disk = FALSE;
    _moo_edit_stop_file_watch (edit);
    add_status (edit, MOO_EDIT_DELETED);
}


static void
add_status (MooEdit        *edit,
            MooEditStatus   s)
{
    edit->priv->status |= s;
    g_signal_emit_by_name (edit, "doc-status-changed", NULL);
}


static void
remove_untitled (gpointer    destroyed,
                 MooEdit    *edit)
{
    gpointer n = g_hash_table_lookup (UNTITLED_NO, edit);

    if (n)
    {
        UNTITLED = g_slist_remove (UNTITLED, n);
        g_hash_table_remove (UNTITLED_NO, edit);
        if (!destroyed)
            g_object_weak_unref (G_OBJECT (edit),
                                 (GWeakNotify) remove_untitled,
                                 GINT_TO_POINTER (TRUE));
    }
}


static int
add_untitled (MooEdit *edit)
{
    int n;

    if (!(n = GPOINTER_TO_INT (g_hash_table_lookup (UNTITLED_NO, edit))))
    {
        for (n = 1; ; ++n)
        {
            if (!g_slist_find (UNTITLED, GINT_TO_POINTER (n)))
            {
                UNTITLED = g_slist_prepend (UNTITLED, GINT_TO_POINTER (n));
                break;
            }
        }

        g_hash_table_insert (UNTITLED_NO, edit, GINT_TO_POINTER (n));
        g_object_weak_ref (G_OBJECT (edit),
                           (GWeakNotify) remove_untitled,
                           GINT_TO_POINTER (TRUE));
    }

    return n;
}


char *
_moo_edit_get_default_encoding (void)
{
    return g_strdup (moo_prefs_get_string (moo_edit_setting (MOO_EDIT_PREFS_ENCODING_SAVE)));
}


void
_moo_edit_set_filename (MooEdit    *edit,
                        const char *file,
                        const char *encoding)
{
    char *tmp1, *tmp3, *tmp4, *tmp5;

    tmp1 = edit->priv->filename;
    tmp3 = edit->priv->display_filename;
    tmp4 = edit->priv->display_basename;
    tmp5 = edit->priv->encoding;

    if (!UNTITLED_NO)
        UNTITLED_NO = g_hash_table_new (g_direct_hash, g_direct_equal);

    if (!file)
    {
        int n = add_untitled (edit);

        edit->priv->filename = NULL;

        if (n == 1)
            edit->priv->display_filename = g_strdup (_("Untitled"));
        else
            edit->priv->display_filename = g_strdup_printf (_("Untitled %d"), n);

        edit->priv->display_basename = g_strdup (edit->priv->display_filename);
    }
    else
    {
        char *basename;

        remove_untitled (NULL, edit);

        edit->priv->filename = g_strdup (file);
        edit->priv->display_filename = _moo_edit_filename_to_utf8 (file);

        basename = g_path_get_basename (file);
        edit->priv->display_basename = _moo_edit_filename_to_utf8 (basename);

        g_free (basename);
    }

    if (!encoding)
        edit->priv->encoding = _moo_edit_get_default_encoding ();
    else
        edit->priv->encoding = g_strdup (encoding);

    g_signal_emit_by_name (edit, "filename-changed", edit->priv->filename, NULL);
    moo_edit_status_changed (edit);

    g_free (tmp1);
    g_free (tmp3);
    g_free (tmp4);
    g_free (tmp5);
}


static char *
_moo_edit_filename_to_utf8 (const char *filename)
{
    GError *err = NULL;
    char *utf_filename;

    g_return_val_if_fail (filename != NULL, NULL);

    utf_filename = g_filename_to_utf8 (filename, -1, NULL, NULL, &err);

    if (!utf_filename)
    {
        g_critical ("%s: could not convert filename to UTF8", G_STRLOC);

        if (err)
        {
            g_critical ("%s: %s", G_STRLOC, err->message);
            g_error_free (err);
            err = NULL;
        }

        utf_filename = g_strdup ("<Unknown>");
    }

    return utf_filename;
}


GdkPixbuf *
_moo_edit_get_icon (MooEdit     *doc,
                    GtkWidget   *widget,
                    GtkIconSize  size)
{
    return _moo_get_icon_for_path (doc->priv->filename, widget, size);
}
