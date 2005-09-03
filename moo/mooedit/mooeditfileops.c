/*
 *   mooeditfileops.c
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

#define MOOEDIT_COMPILATION
#include "mooedit/mooedit-private.h"
#include "mooedit/mooeditfileops.h"
#include "mooedit/mooeditdialogs.h"
#include "mooutils/moofileutils.h"
#include <string.h>


static MooEditLoader *default_loader = NULL;
static MooEditSaver *default_saver = NULL;


static gboolean moo_edit_load_default   (MooEditLoader  *loader,
                                         MooEdit        *edit,
                                         const char     *file,
                                         const char     *encoding,
                                         GError        **error);
static gboolean moo_edit_reload_default (MooEditLoader  *loader,
                                         MooEdit        *edit,
                                         GError        **error);
static gboolean moo_edit_save_default   (MooEditSaver   *saver,
                                         MooEdit        *edit,
                                         const char     *file,
                                         const char     *encoding,
                                         GError        **error);

static void     block_buffer_signals        (MooEdit        *edit);
static void     unblock_buffer_signals      (MooEdit        *edit);
static void     start_file_watch            (MooEdit        *edit);
static void     stop_file_watch             (MooEdit        *edit);
static gboolean file_watch_timeout_func     (MooEdit        *edit);
static gboolean focus_in_cb                 (MooEdit        *edit);
static gboolean check_file_status           (MooEdit        *edit,
                                             gboolean        in_focus_only);
static void     file_modified_on_disk       (MooEdit        *edit);
static void     file_deleted                (MooEdit        *edit);
static void     file_watch_error            (MooEdit        *edit);
static void     file_watch_access_denied    (MooEdit        *edit);
static void     add_status                  (MooEdit        *edit,
                                             MooEditStatus   s);


MooEditLoader   *moo_edit_loader_get_default    (void)
{
    if (!default_loader)
    {
        default_loader = g_new0 (MooEditLoader, 1);
        default_loader->ref_count = 1;
        default_loader->load = moo_edit_load_default;
        default_loader->reload = moo_edit_reload_default;
    }

    return default_loader;
}


MooEditSaver    *moo_edit_saver_get_default     (void)
{
    if (!default_saver)
    {
        default_saver = g_new0 (MooEditSaver, 1);
        default_saver->ref_count = 1;
        default_saver->save = moo_edit_save_default;
    }

    return default_saver;
}


MooEditLoader   *moo_edit_loader_ref            (MooEditLoader  *loader)
{
    g_return_val_if_fail (loader != NULL, NULL);
    loader->ref_count++;
    return loader;
}


MooEditSaver    *moo_edit_saver_ref             (MooEditSaver   *saver)
{
    g_return_val_if_fail (saver != NULL, NULL);
    saver->ref_count++;
    return saver;
}


void             moo_edit_loader_unref          (MooEditLoader  *loader)
{
    if (!loader || --loader->ref_count)
        return;

    g_free (loader);

    if (loader == default_loader)
        default_loader = NULL;
}


void             moo_edit_saver_unref           (MooEditSaver   *saver)
{
    if (!saver || --saver->ref_count)
        return;

    g_free (saver);

    if (saver == default_saver)
        default_saver = NULL;
}


gboolean         moo_edit_load              (MooEditLoader  *loader,
                                             MooEdit        *edit,
                                             const char     *file,
                                             const char     *encoding,
                                             GError        **error)
{
    g_return_val_if_fail (loader != NULL, FALSE);
    g_return_val_if_fail (MOO_IS_EDIT (edit), FALSE);
    g_return_val_if_fail (file != NULL, FALSE);

    return loader->load (loader, edit, file, encoding, error);
}


gboolean         moo_edit_reload            (MooEditLoader  *loader,
                                             MooEdit        *edit,
                                             GError        **error)
{
    g_return_val_if_fail (loader != NULL, FALSE);
    g_return_val_if_fail (MOO_IS_EDIT (edit), FALSE);

    return loader->reload (loader, edit, error);
}


gboolean         moo_edit_save              (MooEditSaver   *saver,
                                             MooEdit        *edit,
                                             const char     *file,
                                             const char     *encoding,
                                             GError        **error)
{
    g_return_val_if_fail (saver != NULL, FALSE);
    g_return_val_if_fail (MOO_IS_EDIT (edit), FALSE);
    g_return_val_if_fail (file != NULL, FALSE);

    return saver->save (saver, edit, file, encoding, error);
}


/***************************************************************************/
/* File loading
 */

static gboolean do_load                 (MooEdit        *edit,
                                         const char     *file,
                                         const char     *encoding,
                                         GError        **error);


static gboolean moo_edit_load_default   (G_GNUC_UNUSED MooEditLoader *loader,
                                         MooEdit        *edit,
                                         const char     *file,
                                         const char     *encoding,
                                         GError        **error)
{
    GtkTextIter start, end;
    MooEditFileInfo *info = NULL;
    gboolean undo;

    g_return_val_if_fail (MOO_IS_EDIT (edit), FALSE);
    g_return_val_if_fail (file && file[0], FALSE);

    block_buffer_signals (edit);

    if (moo_edit_is_empty (edit))
        undo = FALSE;
    else
        undo = TRUE;

    if (undo)
        gtk_text_buffer_begin_user_action (edit->priv->text_buffer);
    else
        gtk_source_buffer_begin_not_undoable_action (edit->priv->source_buffer);

    gtk_text_buffer_get_bounds (edit->priv->text_buffer, &start, &end);
    gtk_text_buffer_delete (edit->priv->text_buffer, &start, &end);

    if (!do_load (edit, file, encoding, error))
    {
        moo_edit_file_info_free (info);
        edit->priv->status = 0;
        stop_file_watch (edit);
        unblock_buffer_signals (edit);

        if (undo)
            gtk_text_buffer_end_user_action (edit->priv->text_buffer);
        else
            gtk_source_buffer_end_not_undoable_action (edit->priv->source_buffer);

        moo_edit_set_modified (edit, FALSE);
        return FALSE;
    }

    gtk_text_buffer_get_start_iter (edit->priv->text_buffer, &start);
    gtk_text_buffer_place_cursor (edit->priv->text_buffer, &start);

    unblock_buffer_signals (edit);
    edit->priv->status = 0;
    _moo_edit_set_filename (edit, file, encoding);
    moo_edit_set_modified (edit, FALSE);
    start_file_watch (edit);

    moo_edit_file_info_free (info);

    if (undo)
        gtk_text_buffer_end_user_action (edit->priv->text_buffer);
    else
        gtk_source_buffer_end_not_undoable_action (edit->priv->source_buffer);

    return TRUE;
}


static gboolean do_load                 (MooEdit        *edit,
                                         const char     *filename,
                                         const char     *encoding,
                                         GError        **error)
{
    GIOChannel *file = NULL;
    GIOStatus status;

    g_return_val_if_fail (filename != NULL, FALSE);

    *error = NULL;
    file = g_io_channel_new_file (filename, "r", error);

    if (!file)
        return FALSE;

    if (encoding && g_io_channel_set_encoding (file, encoding, error) != G_IO_STATUS_NORMAL)
    {
        g_io_channel_shutdown (file, TRUE, NULL);
        g_io_channel_unref (file);
        return FALSE;
    }

    while (TRUE)
    {
        char *line = NULL;
        gsize len;

        status = g_io_channel_read_line (file, &line, &len, NULL, error);

        if (line)
        {
            if (!g_utf8_validate (line, len, NULL))
            {
                g_io_channel_shutdown (file, TRUE, NULL);
                g_io_channel_unref (file);
                g_set_error (error, G_CONVERT_ERROR,
                             G_CONVERT_ERROR_ILLEGAL_SEQUENCE,
                             "Invalid UTF8 data read from file");
                g_free (line);
                return FALSE;
            }

            gtk_text_buffer_insert_at_cursor (edit->priv->text_buffer,
                                              line, len);
            g_free (line);
        }

        /* XXX */
        if (status != G_IO_STATUS_NORMAL)
        {
            if (status != G_IO_STATUS_EOF)
            {
                g_io_channel_shutdown (file, TRUE, NULL);
                g_io_channel_unref (file);
                return FALSE;
            }
            else
            {
                g_io_channel_shutdown (file, TRUE, NULL);
                g_io_channel_unref (file);
                break;
            }
        }
    }

    g_free (edit->priv->encoding);
    edit->priv->encoding = g_strdup (encoding);
    g_clear_error (error);
    return TRUE;
}


static gboolean moo_edit_reload_default (MooEditLoader  *loader,
                                         MooEdit        *edit,
                                         GError        **error)
{
    GtkTextIter start, end;

    g_return_val_if_fail (edit->priv->filename != NULL, FALSE);

    block_buffer_signals (edit);
    gtk_text_buffer_begin_user_action (edit->priv->text_buffer);

    gtk_text_buffer_get_bounds (edit->priv->text_buffer,
                                &start, &end);
    gtk_text_buffer_delete (edit->priv->text_buffer,
                            &start, &end);

    if (!moo_edit_load_default (loader, edit, edit->priv->filename,
                                edit->priv->encoding, error))
    {
        gtk_text_buffer_end_user_action (edit->priv->text_buffer);
        unblock_buffer_signals (edit);
        return FALSE;
    }
    else
    {
        gtk_text_buffer_end_user_action (edit->priv->text_buffer);
        edit->priv->status = 0;
        unblock_buffer_signals (edit);
        moo_edit_set_modified (edit, FALSE);
        start_file_watch (edit);
        g_clear_error (error);
        return TRUE;
    }
}


/***************************************************************************/
/* File saving
 */

/* XXX */
static const char *line_end[3] = {"\n", "\r\n", "\r"};
static guint line_end_len[3] = {1, 2, 1};

static gboolean do_write                (MooEdit        *edit,
                                         const char     *file,
                                         const char     *encoding,
                                         GError        **error);


static gboolean moo_edit_save_default   (G_GNUC_UNUSED MooEditSaver *saver,
                                         MooEdit        *edit,
                                         const char     *filename,
                                         const char     *encoding,
                                         GError        **error)
{
    g_return_val_if_fail (MOO_IS_EDIT (edit), FALSE);
    g_return_val_if_fail (filename && filename[0], FALSE);

    if (!do_write (edit, filename, encoding, error))
        return FALSE;

    edit->priv->status = 0;
    _moo_edit_set_filename (edit, filename, encoding);
    moo_edit_set_modified (edit, FALSE);
    start_file_watch (edit);

    return TRUE;
}


static gboolean do_write                (MooEdit        *edit,
                                         const char     *filename,
                                         const char     *encoding,
                                         GError        **error)
{
    GIOChannel *file;
    GIOStatus status;
    GtkTextIter line_start;
    const char *le = line_end[edit->priv->line_end_type];
    gssize le_len = line_end_len[edit->priv->line_end_type];
    GtkTextBuffer *buf = edit->priv->text_buffer;

    g_return_val_if_fail (filename != NULL, FALSE);

    if (encoding && !strcmp (encoding, "UTF8"))
        encoding = NULL;

    file = g_io_channel_new_file (filename, "w", error);

    if (!file)
        return FALSE;

    if (encoding && g_io_channel_set_encoding (file, encoding, error) != G_IO_STATUS_NORMAL)
    {
        g_io_channel_shutdown (file, TRUE, NULL);
        g_io_channel_unref (file);
        return FALSE;
    }

    gtk_text_buffer_get_start_iter (buf, &line_start);

    do
    {
        gsize written;
        GtkTextIter line_end = line_start;
        gboolean write_line_end = FALSE;

        if (!gtk_text_iter_ends_line (&line_start))
        {
            char *line;
            gssize len = -1;

            gtk_text_iter_forward_to_line_end (&line_end);
            line = gtk_text_buffer_get_text (buf, &line_start, &line_end, FALSE);

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

        if (gtk_text_iter_ends_line (&line_start))
            write_line_end = !gtk_text_iter_is_end (&line_start);
        else
            write_line_end = !gtk_text_iter_is_end (&line_end);

        if (write_line_end)
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

static void     block_buffer_signals        (MooEdit        *edit)
{
    g_signal_handler_block (edit->priv->source_buffer,
                            edit->priv->can_undo_handler_id);
    g_signal_handler_block (edit->priv->source_buffer,
                            edit->priv->can_redo_handler_id);
    g_signal_handler_block (edit->priv->source_buffer,
                            edit->priv->modified_changed_handler_id);
}


static void     unblock_buffer_signals      (MooEdit        *edit)
{
    g_signal_handler_unblock (edit->priv->source_buffer,
                              edit->priv->can_undo_handler_id);
    g_signal_handler_unblock (edit->priv->source_buffer,
                              edit->priv->can_redo_handler_id);
    g_signal_handler_unblock (edit->priv->source_buffer,
                              edit->priv->modified_changed_handler_id);
}


/* XXX use MooFileWatch */
static void     start_file_watch            (MooEdit        *edit)
{
    GTime timestamp;

    if (edit->priv->file_watch_id)
        g_source_remove (edit->priv->file_watch_id);
    edit->priv->file_watch_id = 0;

    g_return_if_fail ((edit->priv->status & MOO_EDIT_CHANGED_ON_DISK) == 0);
    g_return_if_fail (edit->priv->filename != NULL);

    timestamp = moo_get_file_mtime (edit->priv->filename);
    g_return_if_fail (timestamp > 0); /* TODO TODO */
    edit->priv->timestamp = timestamp;

    if (edit->priv->file_watch_timeout)
        edit->priv->file_watch_id =
                g_timeout_add (edit->priv->file_watch_timeout,
                               (GSourceFunc) file_watch_timeout_func,
                               edit);
    if (!edit->priv->focus_in_handler_id)
        edit->priv->focus_in_handler_id =
                g_signal_connect (edit, "focus-in-event",
                                  G_CALLBACK (focus_in_cb),
                                  NULL);
}


static void     stop_file_watch             (MooEdit        *edit)
{
    if (edit->priv->file_watch_id)
        g_source_remove (edit->priv->file_watch_id);

    edit->priv->file_watch_id = 0;

    if (edit->priv->focus_in_handler_id)
    {
        g_signal_handler_disconnect (edit, edit->priv->focus_in_handler_id);
        edit->priv->focus_in_handler_id = 0;
    }
}


static gboolean file_watch_timeout_func     (MooEdit        *edit)
{
    return check_file_status (edit, TRUE);
}


static gboolean focus_in_cb                 (MooEdit        *edit)
{
    check_file_status (edit, TRUE);
    return FALSE;
}


static gboolean check_file_status           (MooEdit        *edit,
                                             gboolean        in_focus_only)
{
    GTime stamp;

    if (in_focus_only && !GTK_WIDGET_HAS_FOCUS (edit))
        return TRUE;

    g_return_val_if_fail (edit->priv->filename != NULL, FALSE);

    g_return_val_if_fail (!(edit->priv->status & MOO_EDIT_CHANGED_ON_DISK), FALSE);
    g_return_val_if_fail (edit->priv->timestamp > 0, FALSE);
    g_return_val_if_fail (edit->priv->filename != NULL, FALSE);

    stamp = moo_get_file_mtime (edit->priv->filename);

    if (stamp < 0)
    {
        switch (stamp)
        {
            case MOO_EACCESS:
                file_watch_access_denied (edit);
                return FALSE;

            case MOO_ENOENT:
                file_deleted (edit);
                return FALSE;

            case MOO_EIO:
            default:
                g_critical ("%s: error in moo_get_file_mtime", G_STRLOC);
                file_watch_error (edit);
                return FALSE;
        }
    }

    if (stamp == edit->priv->timestamp)
        return TRUE;

    if (stamp < edit->priv->timestamp)
    {
        g_warning ("%s: mtime of file on disk is less than timestamp", G_STRLOC);
        file_watch_error (edit);
        return FALSE;
    }

    file_modified_on_disk (edit);
    return FALSE;
}


static void     file_modified_on_disk       (MooEdit        *edit)
{
    g_return_if_fail (edit->priv->filename != NULL);
    g_message ("%s: file '%s'", G_STRLOC, edit->priv->filename);
    stop_file_watch (edit);
    add_status (edit, MOO_EDIT_MODIFIED_ON_DISK);
}


static void     file_deleted                (MooEdit        *edit)
{
    g_return_if_fail (edit->priv->filename != NULL);
    g_message ("%s: file '%s'", G_STRLOC, edit->priv->filename);
    stop_file_watch (edit);
    add_status (edit, MOO_EDIT_DELETED);
}


static void     file_watch_error            (MooEdit        *edit)
{
    g_critical ("%s", G_STRLOC);
    stop_file_watch (edit);
    add_status (edit, MOO_EDIT_DELETED);
}


static void     file_watch_access_denied    (MooEdit        *edit)
{
    g_critical ("%s", G_STRLOC);
    stop_file_watch (edit);
    /* XXX */
    add_status (edit, MOO_EDIT_DELETED);
}


static void     add_status                  (MooEdit        *edit,
                                             MooEditStatus   s)
{
    edit->priv->status |= s;
    g_signal_emit_by_name (edit, "doc-status-changed", NULL);
}


void        _moo_edit_set_filename      (MooEdit    *edit,
                                         const char *file,
                                         const char *encoding)
{
    MooEditLang *lang;

    g_free (edit->priv->filename);
    g_free (edit->priv->basename);
    g_free (edit->priv->display_filename);
    g_free (edit->priv->display_basename);

    if (!file)
    {
        edit->priv->filename = NULL;
        edit->priv->basename = NULL;
        /* TODO TODO */
        edit->priv->display_filename = g_strdup ("Untitled");
        edit->priv->display_basename = g_strdup ("Untitled");
    }
    else
    {
        edit->priv->filename = g_strdup (file);
        edit->priv->basename = g_path_get_basename (file);
        edit->priv->display_filename =
                _moo_edit_filename_to_utf8 (file);
        edit->priv->display_basename =
                _moo_edit_filename_to_utf8 (edit->priv->basename);
    }

    if (!edit->priv->lang_custom)
    {
        if (file)
            lang = moo_edit_lang_mgr_get_language_for_file (_moo_edit_get_lang_mgr (edit), file);
        else
            lang = moo_edit_lang_mgr_get_default_language (_moo_edit_get_lang_mgr (edit));
        moo_edit_set_lang (edit, lang);
    }

    g_free (edit->priv->encoding);
    edit->priv->encoding = g_strdup (encoding);

    g_signal_emit_by_name (edit, "filename-changed", edit->priv->filename, NULL);
    g_signal_emit_by_name (edit, "doc-status-changed", NULL);
}


char       *_moo_edit_filename_to_utf8      (const char         *filename)
{
    GError *err = NULL;
    char *utf_filename;

    g_return_val_if_fail (filename != NULL, NULL);

    utf_filename = g_filename_to_utf8 (filename, -1, NULL, NULL, &err);

    if (!utf_filename)
    {
        g_critical ("%s: could not convert filename to utf8", G_STRLOC);

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
