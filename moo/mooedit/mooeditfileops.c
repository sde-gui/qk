/*
 *   mooedit/mooeditfileops.c
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
#include "mooedit/mooeditdialogs.h"
#include "mooutils/moofileutils.h"
#include <string.h>


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


gboolean    _moo_edit_open          (MooEdit    *edit,
                                     const char *file,
                                     const char *encoding)
{
    GtkTextIter start, end;
    MooEditFileInfo *info = NULL;
    GError *err = NULL;
    gboolean undo;

    g_return_val_if_fail (MOO_IS_EDIT (edit), FALSE);

    if (!moo_edit_get_clean (edit) && moo_edit_get_modified (edit))
    {
        int resp = moo_edit_save_changes_dialog (edit);

        if (resp == GTK_RESPONSE_YES)
        {
            if (!moo_edit_save (edit))
                return FALSE;
        }
        else if (resp == GTK_RESPONSE_CANCEL)
        {
            return FALSE;
        }
    }

    if (!file || !file[0])
    {
        info = moo_edit_open_dialog (GTK_WIDGET (edit));

        if (!info) return FALSE;

        file = info->filename;
        encoding = info->encoding;
    }

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

    if (!moo_edit_load (edit, file, encoding, &err))
    {
        if (err)
        {
            moo_edit_open_error_dialog (GTK_WIDGET (edit), err->message);
            g_error_free (err);
        }
        else
        {
            moo_edit_open_error_dialog (GTK_WIDGET (edit), NULL);
        }

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


gboolean    _moo_edit_save              (MooEdit    *edit)
{
    GError *err = NULL;

    g_return_val_if_fail (MOO_IS_EDIT (edit), FALSE);

    if (!edit->priv->filename)
        return moo_edit_save_as (edit, NULL, NULL);

    if (edit->priv->status & MOO_EDIT_DOC_MODIFIED_ON_DISK) {
        if (!moo_edit_overwrite_modified_dialog (edit))
            return FALSE;
    }
    else if (edit->priv->status & MOO_EDIT_DOC_DELETED) {
        if (!moo_edit_overwrite_deleted_dialog (edit))
            return FALSE;
    }

    if (!moo_edit_write (edit, edit->priv->filename,
         edit->priv->encoding, &err))
    {
        if (err) {
            moo_edit_save_error_dialog (GTK_WIDGET (edit), err->message);
            g_error_free (err);
        }
        else
            moo_edit_save_error_dialog (GTK_WIDGET (edit), NULL);
        return FALSE;
    }

    edit->priv->status = 0;
    moo_edit_set_modified (edit, FALSE);
    start_file_watch (edit);

    return TRUE;
}


gboolean    _moo_edit_save_as       (MooEdit    *edit,
                                     const char *file,
                                     const char *encoding)
{
    GError *err = NULL;
    MooEditFileInfo *info = NULL;

    g_return_val_if_fail (MOO_IS_EDIT (edit), FALSE);

    if (!file || !file[0])
    {
        if (edit->priv->editor)
        {
            MooEditFileMgr *mgr;
            mgr = moo_editor_get_file_mgr (edit->priv->editor);
            info = moo_edit_file_mgr_save_as_dialog (mgr, edit);
        }
        else
        {
            info = moo_edit_save_as_dialog (edit);
        }

        if (!info)
            return FALSE;

        file = info->filename;
        encoding = info->encoding;
    }

    if (!moo_edit_write (edit, file, encoding, &err))
    {
        if (err) {
            moo_edit_save_error_dialog (GTK_WIDGET (edit), err->message);
            g_error_free (err);
        }
        else
            moo_edit_save_error_dialog (GTK_WIDGET (edit), NULL);
        moo_edit_file_info_free (info);
        return FALSE;
    }

    _moo_edit_set_filename (edit, file, encoding);
    edit->priv->status = 0;
    moo_edit_set_modified (edit, FALSE);
    start_file_watch (edit);

    moo_edit_file_info_free (info);
    return TRUE;
}


gboolean    _moo_edit_close         (MooEdit    *edit)
{
    int response;

    if (moo_edit_get_clean (edit) || !moo_edit_get_modified (edit))
        return FALSE;

    response = moo_edit_save_changes_dialog (edit);
    if (response == GTK_RESPONSE_YES) {
        if (!moo_edit_save (edit))
            return TRUE;
    }
    else if (response == GTK_RESPONSE_CANCEL)
        return TRUE;

    return FALSE;
}


gboolean    _moo_edit_reload        (MooEdit    *edit,
                                     GError    **error)
{
    GtkTextIter start, end;

    g_return_val_if_fail (edit->priv->filename != NULL, FALSE);
//     if (!edit->priv->filename)
//         return FALSE;

    if (!moo_edit_get_clean (edit) && moo_edit_get_modified (edit))
        if (!moo_edit_reload_dialog (edit))
            return FALSE;

    block_buffer_signals (edit);
    gtk_text_buffer_begin_user_action (edit->priv->text_buffer);

    gtk_text_buffer_get_bounds (edit->priv->text_buffer,
                                &start, &end);
    gtk_text_buffer_delete (edit->priv->text_buffer,
                            &start, &end);

    if (!moo_edit_load (edit, edit->priv->filename,
         edit->priv->encoding, error))
    {
        gtk_text_buffer_end_user_action (edit->priv->text_buffer);
        unblock_buffer_signals (edit);
        return FALSE;
    }
    else {
        gtk_text_buffer_end_user_action (edit->priv->text_buffer);
        edit->priv->status = 0;
        unblock_buffer_signals (edit);
        moo_edit_set_modified (edit, FALSE);
        start_file_watch (edit);
        return TRUE;
    }
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
        GError *err = NULL;

        edit->priv->filename = g_strdup (file);
        edit->priv->basename = g_path_get_basename (file);
        edit->priv->display_filename =
                g_filename_to_utf8 (file, -1, NULL, NULL, &err);
        if (!edit->priv->display_filename)
        {
            g_critical ("%s: could not convert filename to utf8", G_STRLOC);
            if (err) {
                g_critical ("%s: %s", G_STRLOC, err->message);
                g_error_free (err);
                err = NULL;
            }
            edit->priv->display_filename = g_strdup ("<Unknown>");
        }
        edit->priv->display_basename =
                g_filename_to_utf8 (edit->priv->basename, -1, NULL, NULL, &err);
        if (!edit->priv->display_basename)
        {
            g_critical ("%s: could not convert filename to utf8", G_STRLOC);
            if (err) {
                g_critical ("%s: %s", G_STRLOC, err->message);
                g_error_free (err);
                err = NULL;
            }
            edit->priv->display_basename = g_strdup ("<Unknown>");
        }
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


static void block_buffer_signals    (MooEdit    *edit)
{
    g_signal_handler_block (edit->priv->source_buffer,
                            edit->priv->can_undo_handler_id);
    g_signal_handler_block (edit->priv->source_buffer,
                            edit->priv->can_redo_handler_id);
    g_signal_handler_block (edit->priv->source_buffer,
                            edit->priv->modified_changed_handler_id);
}


static void unblock_buffer_signals  (MooEdit    *edit)
{
    g_signal_handler_unblock (edit->priv->source_buffer,
                              edit->priv->can_undo_handler_id);
    g_signal_handler_unblock (edit->priv->source_buffer,
                              edit->priv->can_redo_handler_id);
    g_signal_handler_unblock (edit->priv->source_buffer,
                              edit->priv->modified_changed_handler_id);
}


static void start_file_watch        (MooEdit    *edit)
{
    GTime timestamp;

    if (edit->priv->file_watch_id)
        g_source_remove (edit->priv->file_watch_id);
    edit->priv->file_watch_id = 0;

    g_return_if_fail ((edit->priv->status & MOO_EDIT_DOC_CHANGED_ON_DISK) == 0);
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


static void stop_file_watch         (MooEdit    *edit)
{
    if (edit->priv->file_watch_id)
        g_source_remove (edit->priv->file_watch_id);
    edit->priv->file_watch_id = 0;

    if (edit->priv->focus_in_handler_id) {
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

    g_return_val_if_fail ((edit->priv->status & MOO_EDIT_DOC_CHANGED_ON_DISK) == 0, FALSE);
    g_return_val_if_fail (edit->priv->timestamp > 0, FALSE);
    g_return_val_if_fail (edit->priv->filename != NULL, FALSE);

    stamp = moo_get_file_mtime (edit->priv->filename);

    if (stamp < 0) {
        switch (stamp) {
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

    if (stamp < edit->priv->timestamp) {
        g_warning ("%s: mtime of file on disk is less than timestamp", G_STRLOC);
        file_watch_error (edit);
        return FALSE;
    }

    file_modified_on_disk (edit);
    return FALSE;
}


static void add_status (MooEdit             *edit,
                        MooEditDocStatus     s)
{
    edit->priv->status |= s;
    g_signal_emit_by_name (edit, "doc-status-changed", NULL);
}


static void ask_reload_or_not (MooEdit *edit)
{
    switch (moo_edit_file_modified_on_disk_dialog (edit)) {
        case GTK_RESPONSE_CANCEL:
            add_status (edit, MOO_EDIT_DOC_MODIFIED_ON_DISK);
            stop_file_watch (edit);
            return;

        case GTK_RESPONSE_OK:
            moo_edit_reload (edit, NULL);
            return;

        default:
            g_critical ("%s: invalid return value from "
                        "moo_edit_modified_on_disk_dialog()",
                        G_STRLOC);
            return;
    }
    g_assert_not_reached ();
    return;
}


static void     file_modified_on_disk       (MooEdit        *edit)
{
    g_return_if_fail (edit->priv->filename != NULL);
    g_message ("%s: file '%s'", G_STRLOC, edit->priv->filename);

    switch (edit->priv->file_watch_policy) {
        case MOO_EDIT_DONT_WATCH_FILE:
            g_warning ("%s", G_STRLOC);
            stop_file_watch (edit);
            return;

        case MOO_EDIT_ALWAYS_ALERT:
            ask_reload_or_not (edit);
            return;

        case MOO_EDIT_ALWAYS_RELOAD:
            moo_edit_reload (edit, NULL);
            return;

        case MOO_EDIT_RELOAD_IF_SAFE:
            if (edit->priv->status & MOO_EDIT_DOC_MODIFIED)
                ask_reload_or_not (edit);
            else
                moo_edit_reload (edit, NULL);
            return;

        default:
            g_critical ("%s: invalid edit->priv->file_watch_policy "
                        "value", G_STRLOC);
            stop_file_watch (edit);
            return;
    }
}


static void     file_deleted                (MooEdit        *edit)
{
    g_return_if_fail (edit->priv->filename != NULL);
    g_message ("%s: file '%s'", G_STRLOC, edit->priv->filename);

    switch (edit->priv->file_watch_policy) {
        case MOO_EDIT_DONT_WATCH_FILE:
            g_warning ("%s", G_STRLOC);
            stop_file_watch (edit);
            return;

        default:
            moo_edit_file_deleted_dialog (edit);
            stop_file_watch (edit);
            add_status (edit, MOO_EDIT_DOC_DELETED);
            return;
    }
}


static void     file_watch_error            (MooEdit        *edit)
{
    g_critical ("%s", G_STRLOC);
    stop_file_watch (edit);
    add_status (edit, MOO_EDIT_DOC_DELETED);
}


static void     file_watch_access_denied    (MooEdit        *edit)
{
    g_critical ("%s", G_STRLOC);
    stop_file_watch (edit);
    add_status (edit, MOO_EDIT_DOC_DELETED);
}


/***************************************************************************/
/* File loading and saving
 */

static const char *line_end[3] = {"\n", "\r\n", "\r"};
static guint line_end_len[3] = {1, 2, 1};

static gboolean moo_edit_load_enc (MooEdit    *edit,
                                   const char *file,
                                   const char *encoding,
                                   GError    **error);


gboolean    _moo_edit_load              (MooEdit    *edit,
                                         const char *file,
                                         const char *encoding,
                                         GError    **error)
{
    GError *err = NULL;

    g_return_val_if_fail (file != NULL, FALSE);

    if (!encoding) {
        if (moo_edit_load_enc (edit, file, NULL, &err)) {
            if (error) *error = NULL;
            edit->priv->encoding = g_strdup ("UTF8");
            return TRUE;
        }
        else if (err->code == G_CONVERT_ERROR_ILLEGAL_SEQUENCE) {
            const char *enc;
            if (g_get_charset (&enc)) {
                if (error) *error = err;
                return FALSE;
            }
            else {
                if (moo_edit_load_enc (edit, file, enc, &err)) {
                    if (error) *error = NULL;
                    edit->priv->encoding = g_strdup (enc);
                    return TRUE;
                }
                else {
                    if (error) *error = err;
                    else g_error_free (err);
                    return FALSE;
                }
            }
        }
        else {
            if (error) *error = err;
            else g_error_free (err);
            return FALSE;
        }
    }
    else {
        if (!strcmp (encoding, "UTF8")) encoding = NULL;
        if (moo_edit_load_enc (edit, file, encoding, &err)) {
            if (error) *error = NULL;
            edit->priv->encoding = g_strdup (encoding);
            return TRUE;
        }
        else {
            if (error) *error = err;
            else g_error_free (err);
            return FALSE;
        }
    }
}


static gboolean moo_edit_load_enc (MooEdit    *edit,
                                   const char *filename,
                                   const char *encoding,
                                   GError    **error)
{
    GIOChannel *file = NULL;
    GIOStatus status;

    *error = NULL;
    file = g_io_channel_new_file (filename, "r", error);
    if (!file)
        return FALSE;

    if (encoding && g_io_channel_set_encoding (file, encoding, error) != G_IO_STATUS_NORMAL) {
        g_io_channel_shutdown (file, TRUE, NULL);
        g_io_channel_unref (file);
        return FALSE;
    }

    while (TRUE)
    {
        char *line = NULL;
        gsize len;

        status = g_io_channel_read_line (file, &line, &len, NULL, error);

        if (line) {
            if (!g_utf8_validate (line, len, NULL)) {
                g_io_channel_shutdown (file, TRUE, NULL);
                g_io_channel_unref (file);
                g_free (line);
                *error = g_error_new (G_CONVERT_ERROR,
                                      G_CONVERT_ERROR_ILLEGAL_SEQUENCE,
                                      "Invalid UTF8 data read from file");
                return FALSE;
            }

            gtk_text_buffer_insert_at_cursor (edit->priv->text_buffer,
                                              line, len);
            g_free (line);
        }

        if (status != G_IO_STATUS_NORMAL || *error) {
            if (status != G_IO_STATUS_EOF) {
                g_io_channel_shutdown (file, TRUE, NULL);
                g_io_channel_unref (file);
                return FALSE;
            }
            else {
                g_io_channel_shutdown (file, TRUE, NULL);
                g_io_channel_unref (file);
                *error = NULL;
                return TRUE;
            }
        }
    }
}


gboolean    _moo_edit_write             (MooEdit    *edit,
                                         const char *filename,
                                         const char *encoding,
                                         GError    **error)
{
    GError *err = NULL;
    GIOChannel *file;
    GtkTextIter line_start;
    const char *le = line_end[edit->priv->line_end_type];
    gssize le_len = line_end_len[edit->priv->line_end_type];
    GtkTextBuffer *buf = edit->priv->text_buffer;

    g_return_val_if_fail (filename != NULL, FALSE);

    if (!encoding) {
        if (edit->priv->encoding)
            encoding = edit->priv->encoding;
        else
            encoding = NULL;
    }
    if (encoding && !strcmp (encoding, "UTF8"))
        encoding = NULL;

    file = g_io_channel_new_file (filename, "w", &err);
    if (!file) {
        if (err) {
            if (error) *error = err;
            else g_error_free (err);
        }
        else if (error) *error = NULL;
        return FALSE;
    }

    if (encoding && g_io_channel_set_encoding (file, encoding, &err) != G_IO_STATUS_NORMAL) {
        g_io_channel_shutdown (file, TRUE, NULL);
        g_io_channel_unref (file);
        if (err) {
            if (error) *error = err;
            else g_error_free (err);
        }
        else if (error) *error = NULL;
        return FALSE;
    }

    gtk_text_buffer_get_start_iter (buf, &line_start);
    do {
        gsize written;
        GtkTextIter line_end = line_start;
        gboolean write_line_end = FALSE;

        if (!gtk_text_iter_ends_line (&line_start)) {
            char *line;
            gssize len = -1;

            gtk_text_iter_forward_to_line_end (&line_end);
            line = gtk_text_buffer_get_text (buf, &line_start, &line_end, FALSE);

            g_io_channel_write_chars (file, line, len, &written, &err);
            g_free (line);
            if (err) {
                if (error) *error = err;
                else g_error_free (err);
                g_io_channel_shutdown (file, TRUE, NULL);
                g_io_channel_unref (file);
                return FALSE;
            }
        }

        if (gtk_text_iter_ends_line (&line_start))
            write_line_end = !gtk_text_iter_is_end (&line_start);
        else
            write_line_end = !gtk_text_iter_is_end (&line_end);

        if (write_line_end) {
            g_io_channel_write_chars (file, le, le_len, &written, &err);
            if (written < (gsize)le_len || err) {
                if (err) {
                    if (error) *error = err;
                    else g_error_free (err);
                }
                else if (error) *error = NULL;
                g_io_channel_shutdown (file, TRUE, NULL);
                g_io_channel_unref (file);
                return FALSE;
            }
        }
    }
    while (gtk_text_iter_forward_line (&line_start));

    g_io_channel_shutdown (file, TRUE, &err);
    if (err) {
        if (error) *error = err;
        else g_error_free (err);
        g_io_channel_unref (file);
        return FALSE;
    }
    else if (error) *error = NULL;

    g_io_channel_unref (file);
    return TRUE;
}
