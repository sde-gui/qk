/*
 *   mooedit-fileops.c
 *
 *   Copyright (C) 2004-2008 by Yevgen Muntyan <muntyan@tamu.edu>
 *
 *   This file is part of medit.  medit is free software; you can
 *   redistribute it and/or modify it under the terms of the
 *   GNU Lesser General Public License as published by the
 *   Free Software Foundation; either version 2.1 of the License,
 *   or (at your option) any later version.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with medit.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define MOOEDIT_COMPILATION
#include "mooedit/mooedit-private.h"
#include "mooedit/mooeditor-private.h"
#include "mooedit/mooedit-fileops.h"
#include "mooedit/mooeditdialogs.h"
#include "mooedit/mootextbuffer.h"
#include "mooedit/mooeditprefs.h"
#include "mooutils/moofileicon.h"
#include "mooutils/moofilewatch.h"
#include "mooutils/mooencodings.h"
#include "mooutils/mooi18n.h"
#include "mooutils/mootype-macros.h"
#include "mooutils/mooutils-messages.h"
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

MOO_DEFINE_QUARK (MooEditFileErrorQuark, _moo_edit_file_error_quark)

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
                                             GFile          *file,
                                             const char     *encoding,
                                             GError        **error);
static gboolean moo_edit_reload_local       (MooEdit        *edit,
                                             const char     *encoding,
                                             GError        **error);
static gboolean moo_edit_save_local         (MooEdit        *edit,
                                             GFile          *file,
                                             const char     *encoding,
                                             MooEditSaveFlags flags,
                                             GError        **error);
static gboolean moo_edit_save_copy_local    (MooEdit        *edit,
                                             GFile          *file,
                                             const char     *encoding,
                                             MooEditSaveFlags flags,
                                             GError        **error);
static void     _moo_edit_start_file_watch  (MooEdit        *edit);


static const char *
normalize_encoding (const char *encoding,
                    gboolean    for_save)
{
    if (!encoding || !encoding[0] || !strcmp (encoding, MOO_ENCODING_AUTO))
        encoding = for_save ? MOO_ENCODING_UTF8 : NULL;
    return encoding;
}


gboolean
_moo_edit_load_file (MooEdit      *edit,
                     GFile        *file,
                     const char   *encoding,
                     GError      **error)
{
    char *encoding_copy;
    gboolean result;
    GError *error_here = NULL;

    g_return_val_if_fail (MOO_IS_EDIT (edit), FALSE);
    g_return_val_if_fail (G_IS_FILE (file), FALSE);

    encoding_copy = g_strdup (normalize_encoding (encoding, FALSE));

    result = moo_edit_load_local (edit, file, encoding_copy, &error_here);

    if (error_here)
        g_propagate_error (error, error_here);

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
                     GFile          *file,
                     const char     *encoding,
                     MooEditSaveFlags flags,
                     GError        **error)
{
    GError *error_here = NULL;
    char *encoding_copy;
    gboolean result;

    g_return_val_if_fail (MOO_IS_EDIT (edit), FALSE);
    g_return_val_if_fail (G_IS_FILE (file), FALSE);

    encoding_copy = g_strdup (normalize_encoding (encoding, TRUE));

    result = moo_edit_save_local (edit, file, encoding_copy, flags, &error_here);

    if (error_here)
        g_propagate_error (error, error_here);

    g_free (encoding_copy);
    return result;
}


gboolean
_moo_edit_save_file_copy (MooEdit        *edit,
                          GFile          *file,
                          const char     *encoding,
                          MooEditSaveFlags flags,
                          GError        **error)
{
    char *encoding_copy;
    gboolean result;

    g_return_val_if_fail (MOO_IS_EDIT (edit), FALSE);
    g_return_val_if_fail (G_IS_FILE (file), FALSE);

    encoding_copy = g_strdup (normalize_encoding (encoding, TRUE));

    result = moo_edit_save_copy_local (edit, file, encoding_copy, flags, error);

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


MooLineEndType
_moo_edit_get_line_end_type (MooEdit *edit)
{
    g_return_val_if_fail (MOO_IS_EDIT (edit), MOO_LE_DEFAULT);
    return edit->priv->line_end_type;
}

static void
moo_edit_set_line_end_type_full (MooEdit        *edit,
                                 MooLineEndType  le,
                                 gboolean        quiet)
{
    moo_return_if_fail (MOO_IS_EDIT (edit));
    moo_return_if_fail (le > 0);

    if (edit->priv->line_end_type != le)
    {
        edit->priv->line_end_type = le;
        if (!quiet)
            g_object_notify (G_OBJECT (edit), "line-end-type");
    }
}

void
_moo_edit_set_line_end_type (MooEdit        *edit,
                             MooLineEndType  le)
{
    moo_edit_set_line_end_type_full (edit, le, FALSE);
}


/***************************************************************************/
/* File loading
 */

typedef enum {
    SUCCESS,
    ERROR_FILE,
    ERROR_ENCODING
} LoadResult;

static LoadResult   do_load     (MooEdit      *edit,
                                 GFile        *file,
                                 const char   *encoding,
                                 GError      **error);
#ifdef LOAD_BINARY
static LoadResult   load_binary (MooEdit      *edit,
                                 const char   *file,
                                 GError      **error);
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
try_load (MooEdit      *edit,
          GFile        *file,
          const char   *encoding,
          GError      **error)
{
    GtkTextBuffer *buffer;
    gboolean enable_highlight;
    LoadResult result;

    g_return_val_if_fail (MOO_IS_EDIT (edit), FALSE);
    g_return_val_if_fail (G_IS_FILE (file), FALSE);
    g_return_val_if_fail (encoding && encoding[0], FALSE);

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (edit));
    gtk_text_buffer_set_text (buffer, "", 0);

    g_object_get (edit, "enable-highlight", &enable_highlight, NULL);
    g_object_set (edit, "enable-highlight", FALSE, NULL);
    result = do_load (edit, file, encoding, error);
    g_object_set (edit, "enable-highlight", enable_highlight, NULL);

    return result;
}


static gboolean
check_regular (GFile   *file,
               GError **error)
{
    GFileInfo *info;
    GFileType type;
    gboolean retval = TRUE;

    if (!g_file_is_native (file))
        return TRUE;

    if (!(info = g_file_query_info (file, G_FILE_ATTRIBUTE_STANDARD_TYPE, 0, NULL, NULL)))
        return TRUE;

    type = g_file_info_get_file_type (info);
    if (type != G_FILE_TYPE_REGULAR && type != G_FILE_TYPE_UNKNOWN)
    {
        g_set_error (error, MOO_EDIT_FILE_ERROR,
                     MOO_EDIT_FILE_ERROR_FAILED,
                     "%s", D_("Not a regular file", "glib20"));
        retval = FALSE;
    }

    g_object_unref (info);
    return retval;
}

static gboolean
moo_edit_load_local (MooEdit     *edit,
                     GFile       *file,
                     const char  *encoding,
                     GError     **error)
{
    GtkTextIter start;
    GtkTextBuffer *buffer;
    MooTextView *view;
    gboolean undo;
    LoadResult result = ERROR_FILE;
    char *freeme = NULL;
    MooLineEndType saved_le;

    g_return_val_if_fail (MOO_IS_EDIT (edit), FALSE);
    g_return_val_if_fail (file != NULL, FALSE);

    if (!check_regular (file, error))
        return FALSE;

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

    saved_le = edit->priv->line_end_type;

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
            result = try_load (edit, file, enc, error);

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
        result = try_load (edit, file, encoding, error);
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
        _moo_edit_set_file (edit, file, encoding);
        if (edit->priv->line_end_type != saved_le)
            g_object_notify (G_OBJECT (edit), "line-end-type");
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
do_load (MooEdit      *edit,
         GFile        *gfile,
         const char   *encoding,
         GError      **error)
{
    GIOChannel *file = NULL;
    GIOStatus status;
    GtkTextBuffer *buffer;
    MooLineEndType le = edit->priv->line_end_type;
    GString *text = NULL;
    char *line = NULL;
    LoadResult result = ERROR_FILE;
    char *path;

    g_return_val_if_fail (G_IS_FILE (gfile), ERROR_FILE);
    g_return_val_if_fail (encoding != NULL, ERROR_FILE);

    if (!(path = g_file_get_path (gfile)))
    {
        g_set_error (error, MOO_EDIT_FILE_ERROR,
                     MOO_EDIT_FILE_ERROR_NOT_IMPLEMENTED,
                     "Loading remote files is not implemented");
        return ERROR_FILE;
    }

    file = g_io_channel_new_file (path, "r", error);
    g_free (path);

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
        MooLineEndType le_here = 0;

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
                    le_here = MOO_LE_MAC;
                }
                else if (!strcmp (line_term, "\n"))
                {
                    le_here = MOO_LE_UNIX;
                }
                else if (!strcmp (line_term, "\r\n"))
                {
                    le_here = MOO_LE_WIN32;
                }

                if (le_here)
                {
                    if (le && le != le_here)
                        le = MOO_LE_MIX;
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

    switch (edit->priv->line_end_type)
    {
        case MOO_LE_MIX:
            break;
        default:
            moo_edit_set_line_end_type_full (edit, le, TRUE);
            break;
    }

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
    GFile *file;

    file = _moo_edit_get_file (edit);
    g_return_val_if_fail (file != NULL, FALSE);

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (edit));

    block_buffer_signals (edit);
    gtk_text_buffer_begin_user_action (buffer);

    gtk_text_buffer_get_bounds (buffer, &start, &end);
    gtk_text_buffer_delete (buffer, &start, &end);
    g_object_get (edit, "enable-highlight", &enable_highlight, NULL);
    g_object_set (edit, "enable-highlight", FALSE, NULL);

    result = _moo_edit_load_file (edit, file,
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

    g_object_unref (file);
    return result;
}


/***************************************************************************/
/* File saving
 */

static char *
get_contents_with_fixed_line_end (GtkTextBuffer *buffer, const char *le, guint le_len)
{
    GtkTextIter line_start;
    GString *contents;

    contents = g_string_new (NULL);
    gtk_text_buffer_get_start_iter (buffer, &line_start);

    do
    {
        GtkTextIter line_end = line_start;

        if (!gtk_text_iter_ends_line (&line_start))
        {
            char *line;
            gsize len;

            gtk_text_iter_forward_to_line_end (&line_end);
            line = gtk_text_buffer_get_text (buffer, &line_start, &line_end, TRUE);
            len = strlen (line);

            g_string_append_len (contents, line, len);

            g_free (line);
        }

        if (!gtk_text_iter_is_end (&line_end))
            g_string_append_len (contents, le, le_len);
    }
    while (gtk_text_iter_forward_line (&line_start));

    return g_string_free (contents, FALSE);
}

static char *
get_contents (MooEdit *edit)
{
    gboolean normalize_le = FALSE;
    const char *le;
    gsize le_len;
    GtkTextBuffer *buffer;
    char *contents;

    switch (edit->priv->line_end_type)
    {
        case MOO_LE_UNIX:
            normalize_le = TRUE;
            le = "\n";
            le_len = 1;
            break;
        case MOO_LE_WIN32:
            normalize_le = TRUE;
            le = "\r\n";
            le_len = 2;
            break;
        case MOO_LE_MAC:
            normalize_le = TRUE;
            le = "\r";
            le_len = 1;
            break;

        case MOO_LE_MIX:
            break;

        default:
            moo_assert_not_reached ();
    }

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (edit));

    if (normalize_le)
    {
        contents = get_contents_with_fixed_line_end (buffer, le, le_len);
    }
    else
    {
        GtkTextIter start, end;
        gtk_text_buffer_get_bounds (buffer, &start, &end);
        contents = gtk_text_buffer_get_text (buffer, &start, &end, TRUE);
    }

    return contents;
}

static gboolean
do_write (GFile             *file,
          const char        *data,
          gsize              len,
          MooEditSaveFlags   flags,
          GError           **error)
{
    MooFileWriter *writer;
    MooFileWriterFlags writer_flags;
    gboolean success = FALSE;

    g_return_val_if_fail (G_IS_FILE (file), FALSE);

    writer_flags = (flags & MOO_EDIT_SAVE_BACKUP) ? MOO_FILE_WRITER_SAVE_BACKUP : 0;

    if ((writer = moo_file_writer_new_for_file (file, writer_flags, error)))
    {
        moo_file_writer_write (writer, data, len);
        success = moo_file_writer_close (writer, error);
    }

    return success;
}

static gboolean
do_save_local (MooEdit        *edit,
               GFile          *file,
               const char     *encoding,
               MooEditSaveFlags flags,
               GError        **error,
               gboolean       *retval)
{
    char *utf8_contents;
    const char *to_save;
    gsize to_save_size;
    GError *encoding_error = NULL;
    char *freeme = NULL;
    gboolean success;

    success = TRUE;
    *retval = TRUE;
    utf8_contents = get_contents (edit);

    moo_release_assert (utf8_contents != NULL);

    if (encoding && (!g_ascii_strcasecmp (encoding, "UTF-8") || !g_ascii_strcasecmp (encoding, "UTF8")))
        encoding = NULL;

    if (encoding)
    {
        gsize bytes_read;
        gsize bytes_written;
        char *encoded = g_convert (utf8_contents, -1,
                                   encoding, "UTF-8",
                                   &bytes_read, &bytes_written,
                                   &encoding_error);

        if (encoded)
        {
            to_save = encoded;
            to_save_size = bytes_written;
        }
        else
        {
            *retval = FALSE;
            to_save = utf8_contents;
            to_save_size = strlen (utf8_contents);
        }
    }
    else
    {
        to_save = utf8_contents;
        to_save_size = strlen (utf8_contents);
    }

    if (!do_write (file, to_save, to_save_size, flags, error))
    {
        *retval = FALSE;
        success = FALSE;
    }
    else if (encoding_error)
    {
        g_propagate_error (error, encoding_error);
        set_encoding_error (error);
    }

    g_free (freeme);
    g_free (utf8_contents);
    return success;
}


static gboolean
moo_edit_save_local (MooEdit        *edit,
                     GFile          *file,
                     const char     *encoding,
                     MooEditSaveFlags flags,
                     GError        **error)
{
    gboolean result;

    g_return_val_if_fail (MOO_IS_EDIT (edit), FALSE);
    g_return_val_if_fail (G_IS_FILE (file), FALSE);

    if (do_save_local (edit, file, encoding, flags, error, &result))
    {
        edit->priv->status = 0;
        _moo_edit_set_file (edit, file,
                           result ? encoding : MOO_ENCODING_UTF8);
        moo_edit_set_modified (edit, FALSE);
        _moo_edit_start_file_watch (edit);
    }

    return result;
}


static gboolean
moo_edit_save_copy_local (MooEdit        *edit,
                          GFile          *file,
                          const char     *encoding,
                          MooEditSaveFlags flags,
                          GError        **error)
{
    gboolean result;
    g_return_val_if_fail (MOO_IS_EDIT (edit), FALSE);
    g_return_val_if_fail (G_IS_FILE (file), FALSE);
    do_save_local (edit, file, encoding, flags, error, &result);
    return result;
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
check_file_status (MooEdit  *edit,
                   gboolean  in_focus_only)
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


const char *
_moo_edit_get_default_encoding (void)
{
    return moo_prefs_get_string (moo_edit_setting (MOO_EDIT_PREFS_ENCODING_SAVE));
}


char *
_moo_file_get_display_name (GFile *file)
{
    g_return_val_if_fail (G_IS_FILE (file), NULL);
    return g_file_get_parse_name (file);
}

char *
_moo_file_get_display_basename (GFile *file)
{
    char *name;
    const char *slash;

    g_return_val_if_fail (G_IS_FILE (file), NULL);

    name = _moo_file_get_display_name (file);
    g_return_val_if_fail (name != NULL, NULL);

    slash = strrchr (name, '/');

#ifdef G_OS_WIN32
    {
        const char *backslash = strrchr (name, '\\');
        if (backslash && (!slash || backslash > slash))
            slash = backslash;
    }
#endif

    if (slash)
        memmove (name, slash + 1, strlen (slash + 1) + 1);

    return name;
}


void
_moo_edit_set_file (MooEdit    *edit,
                    GFile      *file,
                    const char *encoding)
{
    GFile *tmp;
    char *tmp2, *tmp3, *tmp4;

    tmp = edit->priv->file;
    tmp2 = edit->priv->filename;
    tmp3 = edit->priv->display_filename;
    tmp4 = edit->priv->display_basename;

    if (!UNTITLED_NO)
        UNTITLED_NO = g_hash_table_new (g_direct_hash, g_direct_equal);

    if (!file)
    {
        int n = add_untitled (edit);

        edit->priv->file = NULL;
        edit->priv->filename = NULL;

        if (n == 1)
            edit->priv->display_filename = g_strdup (_("Untitled"));
        else
            edit->priv->display_filename = g_strdup_printf (_("Untitled %d"), n);

        edit->priv->display_basename = g_strdup (edit->priv->display_filename);
    }
    else
    {
        remove_untitled (NULL, edit);
        edit->priv->file = g_file_dup (file);
        edit->priv->filename = g_file_get_path (file);
        edit->priv->display_filename = _moo_file_get_display_name (file);
        edit->priv->display_basename = _moo_file_get_display_basename (file);
    }

    if (!encoding)
        _moo_edit_set_encoding (edit, _moo_edit_get_default_encoding ());
    else
        _moo_edit_set_encoding (edit, encoding);

    g_signal_emit_by_name (edit, "filename-changed", edit->priv->filename, NULL);
    moo_edit_status_changed (edit);

    if (tmp)
        g_object_unref (tmp);
    g_free (tmp2);
    g_free (tmp3);
    g_free (tmp4);
}


GdkPixbuf *
_moo_edit_get_icon (MooEdit     *doc,
                    GtkWidget   *widget,
                    GtkIconSize  size)
{
    if (doc->priv->filename)
        return moo_get_icon_for_file (doc->priv->filename, widget, size);
    else if (doc->priv->file)
        return moo_get_icon_for_file (doc->priv->display_basename, widget, size);
    else
        return moo_get_icon_for_file (NULL, widget, size);
}
