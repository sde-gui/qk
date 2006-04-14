/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *   moofileentry.c
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

#define MOO_FILE_SYSTEM_COMPILATION
#include "moofileentry.h"
#include "moofilesystem.h"
#include "moofoldermodel.h"
#include MOO_MARSHALS_H
#include "mooutils/mooutils-gobject.h"
#include "moofileview-private.h"
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>


#define COMPLETION_POPUP_LEN 15
#define COLUMN_FILE MOO_FOLDER_MODEL_COLUMN_FILE

#ifndef __WIN32__
#define CASE_SENSITIVE_DEFAULT   TRUE
#else /* __WIN32__ */
#define CASE_SENSITIVE_DEFAULT   FALSE
#endif /* __WIN32__ */


struct _MooFileEntryCompletionPrivate {
    MooFileSystem *fs;
    char *current_dir;
    GtkEntry *entry;

    gboolean managed; /* do not connect to entry signals */

    gboolean enabled;
    gboolean do_complete;

    MooFileVisibleFunc visible_func;
    gpointer visible_func_data;
    gboolean dirs_only;
    gboolean show_hidden;

    GtkWidget *popup;
    GtkTreeView *treeview;
    GtkTreeViewColumn *column;
    GtkTreeModel *model;
    TextFuncs text_funcs;
    gboolean case_sensitive;

    gboolean recheck_prefix;
    char *dirname;
    char *display_dirname;
    char *display_basename;
    guint display_basename_len;
};


static void     moo_file_entry_completion_finalize      (GObject        *object);
static void     moo_file_entry_completion_set_property  (GObject        *object,
                                                         guint           prop_id,
                                                         const GValue   *value,
                                                         GParamSpec     *pspec);
static void     moo_file_entry_completion_get_property  (GObject        *object,
                                                         guint           prop_id,
                                                         GValue         *value,
                                                         GParamSpec     *pspec);

static void     completion_create_popup         (MooFileEntryCompletion *cmpl);
static void     completion_connect_folder       (MooFileEntryCompletion *cmpl,
                                                 MooFolder              *folder);
static void     completion_disconnect_folder    (MooFileEntryCompletion *cmpl);
static void     completion_popup                (MooFileEntryCompletion *cmpl);
static void     completion_popdown              (MooFileEntryCompletion *cmpl);
static gboolean completion_resize_popup         (MooFileEntryCompletion *cmpl);
static void     completion_cell_data_func       (GtkTreeViewColumn      *column,
                                                 GtkCellRenderer        *cell,
                                                 GtkTreeModel           *model,
                                                 GtkTreeIter            *iter,
                                                 MooFileEntryCompletion *cmpl);
static gboolean completion_visible_func         (GtkTreeModel           *model,
                                                 GtkTreeIter            *iter,
                                                 MooFileEntryCompletion *cmpl);
static gboolean completion_entry_focus_out      (GtkEntry               *entry,
                                                 MooFileEntryCompletion *cmpl);
static gboolean completion_popup_button_press   (GtkWidget              *popup_window,
                                                 GdkEventButton         *event,
                                                 MooFileEntryCompletion *cmpl);
static gboolean completion_popup_key_press      (GtkWidget              *popup_window,
                                                 GdkEventKey            *event,
                                                 MooFileEntryCompletion *cmpl);
static gboolean completion_list_button_press    (MooFileEntryCompletion *cmpl,
                                                 GdkEventButton         *event);

static void     completion_entry_changed        (GtkEntry               *entry,
                                                 MooFileEntryCompletion *cmpl);
static gboolean completion_entry_key_press      (GtkEntry               *entry,
                                                 GdkEventKey            *event,
                                                 MooFileEntryCompletion *cmpl);

static gboolean completion_parse_text           (MooFileEntryCompletion *cmpl,
                                                 gboolean                refilter);
static char    *completion_make_text            (MooFileEntryCompletion *cmpl,
                                                 MooFile                *file);
static int      completion_get_selected         (MooFileEntryCompletion *cmpl);
static void     completion_set_case_sensitive   (MooFileEntryCompletion *cmpl,
                                                 gboolean                case_sensitive);
static void     completion_set_file_system      (MooFileEntryCompletion *cmpl,
                                                 MooFileSystem          *file_system);
static void     completion_finished             (MooFileEntryCompletion *cmpl);
static gboolean completion_default_visible_func (MooFile                *file);
static void     completion_entry_destroyed      (MooFileEntryCompletion *cmpl);
static void     completion_finish               (MooFileEntryCompletion *cmpl,
                                                 const char             *text);



/* MOO_TYPE_FILE_ENTRY_COMPLETION */
G_DEFINE_TYPE (MooFileEntryCompletion, moo_file_entry_completion, G_TYPE_OBJECT)

enum {
    PROP_0,
    PROP_ENABLE_COMPLETION,
    PROP_DIRECTORIES_ONLY,
    PROP_SHOW_HIDDEN,
    PROP_FILE_SYSTEM,
    PROP_CURRENT_DIR,
    PROP_CASE_SENSITIVE,
    PROP_ENTRY
};

enum {
    FINISHED,
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];

static void moo_file_entry_completion_class_init (MooFileEntryCompletionClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = moo_file_entry_completion_finalize;
    gobject_class->set_property = moo_file_entry_completion_set_property;
    gobject_class->get_property = moo_file_entry_completion_get_property;

    g_object_class_install_property (gobject_class,
                                     PROP_ENABLE_COMPLETION,
                                     g_param_spec_boolean ("enable-completion",
                                             "enable-completion",
                                             "enable-completion",
                                             TRUE,
                                             G_PARAM_CONSTRUCT | G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class,
                                     PROP_DIRECTORIES_ONLY,
                                     g_param_spec_boolean ("directories-only",
                                             "directories-only",
                                             "directories-only",
                                             FALSE,
                                             G_PARAM_CONSTRUCT | G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class,
                                     PROP_SHOW_HIDDEN,
                                     g_param_spec_boolean ("show-hidden",
                                             "show-hidden",
                                             "show-hidden",
                                             TRUE,
                                             G_PARAM_CONSTRUCT | G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class,
                                     PROP_ENTRY,
                                     g_param_spec_object ("entry",
                                             "entry",
                                             "entry",
                                             GTK_TYPE_ENTRY,
                                             G_PARAM_CONSTRUCT | G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class,
                                     PROP_CASE_SENSITIVE,
                                     g_param_spec_boolean ("case-sensitive",
                                             "case-sensitive",
                                             "case-sensitive",
                                             CASE_SENSITIVE_DEFAULT,
                                             G_PARAM_CONSTRUCT | G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class,
                                     PROP_CURRENT_DIR,
                                     g_param_spec_string ("current-dir",
                                             "current-dir",
                                             "current-dir",
                                             NULL,
                                             G_PARAM_CONSTRUCT | G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class,
                                     PROP_FILE_SYSTEM,
                                     g_param_spec_object ("file-system",
                                             "file-system",
                                             "file-system",
                                             MOO_TYPE_FILE_SYSTEM,
                                             G_PARAM_CONSTRUCT | G_PARAM_READWRITE));

    signals[FINISHED] =
            moo_signal_new_cb ("finished",
                               G_OBJECT_CLASS_TYPE (klass),
                               G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                               G_CALLBACK (completion_finished),
                               NULL, NULL,
                               _moo_marshal_VOID__VOID,
                               G_TYPE_NONE, 0);
}


static void moo_file_entry_completion_init      (MooFileEntryCompletion *cmpl)
{
    GtkTreeModel *model;

    cmpl->priv = g_new0 (MooFileEntryCompletionPrivate, 1);

    /* XXX */
    cmpl->priv->managed = FALSE;

    cmpl->priv->enabled = TRUE;
    cmpl->priv->do_complete = FALSE;

    cmpl->priv->case_sensitive = CASE_SENSITIVE_DEFAULT;
    cmpl->priv->visible_func = (MooFileVisibleFunc) completion_default_visible_func;

    if (cmpl->priv->case_sensitive)
    {
        cmpl->priv->text_funcs.strcmp_func = strcmp_func;
        cmpl->priv->text_funcs.strncmp_func = strncmp_func;
        cmpl->priv->text_funcs.normalize_func = normalize_func;
    }
    else
    {
        cmpl->priv->text_funcs.strcmp_func = case_strcmp_func;
        cmpl->priv->text_funcs.strncmp_func = case_strncmp_func;
        cmpl->priv->text_funcs.normalize_func = case_normalize_func;
    }

    model = moo_folder_model_new (NULL);
    cmpl->priv->model = moo_folder_filter_new (MOO_FOLDER_MODEL (model));
    gtk_tree_model_filter_set_visible_func (GTK_TREE_MODEL_FILTER (cmpl->priv->model),
                                            (GtkTreeModelFilterVisibleFunc) completion_visible_func,
                                            cmpl, NULL);
    completion_create_popup (cmpl);
    g_object_unref (model);
}


static void
moo_file_entry_completion_finalize (GObject *object)
{
    MooFileEntryCompletion *cmpl = MOO_FILE_ENTRY_COMPLETION (object);

    if (cmpl->priv->entry)
    {
        if (!cmpl->priv->managed)
        {
            g_signal_handlers_disconnect_by_func (cmpl->priv->entry,
                    (gpointer) completion_entry_changed,
                    cmpl);
            g_signal_handlers_disconnect_by_func (cmpl->priv->entry,
                    (gpointer) completion_entry_key_press,
                    cmpl);
        }

        g_object_weak_unref (G_OBJECT (cmpl->priv->entry),
                             (GWeakNotify) completion_entry_destroyed,
                             cmpl);
    }

    if (cmpl->priv->fs)
        g_object_unref (cmpl->priv->fs);
    g_free (cmpl->priv->current_dir);

    completion_disconnect_folder (cmpl);
    g_free (cmpl->priv->dirname);

    gtk_widget_destroy (cmpl->priv->popup);
    g_object_unref (cmpl->priv->model);

    g_free (cmpl->priv);
    cmpl->priv = NULL;

    G_OBJECT_CLASS (moo_file_entry_completion_parent_class)->finalize (object);
}


static void
moo_file_entry_completion_set_property (GObject        *object,
                                        guint           prop_id,
                                        const GValue   *value,
                                        GParamSpec     *pspec)
{
    MooFileEntryCompletion *cmpl = MOO_FILE_ENTRY_COMPLETION (object);

    switch (prop_id)
    {
        case PROP_ENTRY:
            moo_file_entry_completion_set_entry (cmpl, g_value_get_object (value));
            break;

        case PROP_ENABLE_COMPLETION:
            cmpl->priv->enabled = g_value_get_boolean (value);
            g_object_notify (object, "enable-completion");
            break;

        case PROP_DIRECTORIES_ONLY:
            cmpl->priv->dirs_only = g_value_get_boolean (value);
            g_object_notify (object, "directories-only");
            break;

        case PROP_SHOW_HIDDEN:
            cmpl->priv->show_hidden = g_value_get_boolean (value);
            g_object_notify (object, "show-hidden");
            break;

        case PROP_CURRENT_DIR:
            g_free (cmpl->priv->current_dir);
            cmpl->priv->current_dir = g_strdup (g_value_get_string (value));
            g_object_notify (object, "current-dir");
            break;

        case PROP_FILE_SYSTEM:
            completion_set_file_system (cmpl, g_value_get_object (value));
            break;

        case PROP_CASE_SENSITIVE:
            completion_set_case_sensitive (cmpl, g_value_get_boolean (value));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}


static void
moo_file_entry_completion_get_property (GObject        *object,
                                        guint           prop_id,
                                        GValue         *value,
                                        GParamSpec     *pspec)
{
    MooFileEntryCompletion *cmpl = MOO_FILE_ENTRY_COMPLETION (object);

    switch (prop_id)
    {
        case PROP_ENTRY:
            g_value_set_object (value, cmpl->priv->entry);
            break;

        case PROP_ENABLE_COMPLETION:
            g_value_set_boolean (value, cmpl->priv->enabled);
            break;

        case PROP_DIRECTORIES_ONLY:
            g_value_set_boolean (value, cmpl->priv->dirs_only);
            break;

        case PROP_SHOW_HIDDEN:
            g_value_set_boolean (value, cmpl->priv->show_hidden);
            break;

        case PROP_CURRENT_DIR:
            g_value_set_string (value, cmpl->priv->current_dir);
            break;

        case PROP_FILE_SYSTEM:
            g_value_set_object (value, cmpl->priv->fs);
            break;

        case PROP_CASE_SENSITIVE:
            g_value_set_boolean (value, cmpl->priv->case_sensitive);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}


#define TAKE_STRING(p,s)    \
G_STMT_START {              \
    g_free (p);             \
    p = s;                  \
    s = NULL;               \
} G_STMT_END

#define DELETE_MEM(p)       \
G_STMT_START {              \
    g_free (p);             \
    p = NULL;               \
} G_STMT_END


static void
completion_finished (MooFileEntryCompletion *cmpl)
{
    cmpl->priv->do_complete = FALSE;
}


static void
completion_finish (MooFileEntryCompletion *cmpl,
                   const char             *text)
{
    completion_popdown (cmpl);

    if (text)
    {
        gtk_entry_set_text (GTK_ENTRY (cmpl->priv->entry), text);
        gtk_editable_set_position (GTK_EDITABLE (cmpl->priv->entry), -1);
    }

    g_signal_emit (cmpl, signals[FINISHED], 0);
}


static void
completion_entry_changed (G_GNUC_UNUSED GtkEntry *entry,
                          MooFileEntryCompletion *cmpl)
{
    int n_items, selected;
    GtkTreePath *path;

    if (!cmpl->priv->enabled)
        return;

    if (!cmpl->priv->do_complete)
        return;

    if (!completion_parse_text (cmpl, FALSE))
        goto finish;

    gtk_tree_model_filter_refilter (GTK_TREE_MODEL_FILTER (cmpl->priv->model));

    n_items = gtk_tree_model_iter_n_children (cmpl->priv->model, NULL);

    if (!n_items)
        goto finish;

    completion_resize_popup (cmpl);
    selected = completion_get_selected (cmpl);

    if (selected < 0)
        selected = 0;

    path = gtk_tree_path_new_from_indices (selected, -1);
    gtk_tree_view_scroll_to_cell (cmpl->priv->treeview, path, NULL, FALSE, 0, 0);
    gtk_tree_path_free (path);

    cmpl->priv->recheck_prefix = TRUE;

    return;

finish:
    DELETE_MEM (cmpl->priv->display_dirname);
    DELETE_MEM (cmpl->priv->display_basename);
    DELETE_MEM (cmpl->priv->dirname);
    completion_finish (cmpl, NULL);
}


static char*
completion_make_text (MooFileEntryCompletion *cmpl,
                      MooFile                *file)
{
    g_return_val_if_fail (file != NULL, NULL);
    g_return_val_if_fail (cmpl->priv->display_dirname != NULL, NULL);

    if (MOO_FILE_IS_DIR (file))
        return g_strdup_printf ("%s%s%c", cmpl->priv->display_dirname,
                                moo_file_display_name (file),
                                G_DIR_SEPARATOR);
    else
        return g_strdup_printf ("%s%s", cmpl->priv->display_dirname,
                                moo_file_display_name (file));
}


static gboolean
completion_parse_text (MooFileEntryCompletion *cmpl,
                       gboolean                refilter)
{
    const char *text;
    GError *error = NULL;
    MooFolder *folder;
    gboolean result = FALSE;
    guint text_len;
    char *path = NULL, *dirname = NULL;
    char *display_dirname = NULL, *display_basename = NULL;

    text = gtk_entry_get_text (GTK_ENTRY (cmpl->priv->entry));

    if (!text || !text[0])
        return FALSE;

    path = moo_file_system_get_absolute_path (cmpl->priv->fs, text,
                                              cmpl->priv->current_dir);

    if (!path)
        return FALSE;

    if (!moo_file_system_parse_path (cmpl->priv->fs,
                                     path, &dirname, &display_dirname,
                                     &display_basename, &error))
    {
        g_message ("%s: could not parse path '%s'", G_STRLOC, path);
        g_message ("%s: %s", G_STRLOC, error->message);
        goto out;
    }

    TAKE_STRING (cmpl->priv->display_basename, display_basename);
    cmpl->priv->display_basename_len = strlen (cmpl->priv->display_basename);

    text_len = strlen (text);

#if 0
    /* this was needed in the widget */
    if (text_len <= cmpl->priv->display_basename_len)
    {
        g_warning ("%s: something wrong", G_STRLOC);
        goto out;
    }
#endif

    if (!cmpl->priv->dirname || strcmp (cmpl->priv->dirname, dirname))
    {
        completion_disconnect_folder (cmpl);
        DELETE_MEM (cmpl->priv->dirname);

        folder = moo_file_system_get_folder (cmpl->priv->fs,
                                             dirname, MOO_FILE_HAS_STAT,
                                             &error);
        if (!folder)
        {
            g_message ("%s: could not get folder '%s'", G_STRLOC, dirname);
            g_message ("%s: %s", G_STRLOC, error->message);
            g_error_free (error);
            goto out;
        }

        g_free (cmpl->priv->display_dirname);
        cmpl->priv->display_dirname = g_strndup (text, text_len - cmpl->priv->display_basename_len);
        TAKE_STRING (cmpl->priv->dirname, dirname);

        completion_connect_folder (cmpl, folder);
    }
    else if (refilter)
    {
        gtk_tree_model_filter_refilter (GTK_TREE_MODEL_FILTER (cmpl->priv->model));
    }

    result = TRUE;

out:
    g_free (path);
    g_free (dirname);
    g_free (display_dirname);
    g_free (display_basename);
    return result;
}


static void
completion_tab_key (MooFileEntryCompletion *cmpl)
{
    int n_items;
    GtkTreePath *path;
    GtkTreeIter iter;
    MooFile *file = NULL;
    char *text;
    GString *prefix;
    gboolean unique;

    if (!completion_parse_text (cmpl, TRUE))
        return;

    n_items = gtk_tree_model_iter_n_children (cmpl->priv->model, NULL);

    if (!n_items)
        return;

    cmpl->priv->do_complete = TRUE;

    if (n_items == 1)
    {
        path = gtk_tree_path_new_from_indices (0, -1);
        gtk_tree_model_get_iter (cmpl->priv->model, &iter, path);
        gtk_tree_path_free (path);

        gtk_tree_model_get (cmpl->priv->model, &iter, COLUMN_FILE, &file, -1);
        g_return_if_fail (file != NULL);

        text = completion_make_text (cmpl, file);
        completion_finish (cmpl, text);

        g_free (text);
        return;
    }

    if (cmpl->priv->display_basename && cmpl->priv->display_basename[0])
    {
        prefix = model_find_max_prefix (cmpl->priv->model,
                                        cmpl->priv->display_basename,
                                        &cmpl->priv->text_funcs, &unique, NULL);

        if (!prefix)
        {
            g_critical ("%s: oops", G_STRLOC);
        }
        else if (prefix->len > cmpl->priv->display_basename_len)
        {
            text = g_strdup_printf ("%s%s", cmpl->priv->display_dirname,
                                    prefix->str);
            gtk_entry_set_text (GTK_ENTRY (cmpl->priv->entry), text);
            gtk_editable_set_position (GTK_EDITABLE (cmpl->priv->entry), -1);
            g_free (text);
        }

        g_string_free (prefix, TRUE);
    }

    /* XXX scroll to selected */
    completion_popup (cmpl);
}


static gboolean
completion_popup_tab_key_press (MooFileEntryCompletion *cmpl)
{
    int n_items;
    GtkTreePath *path;
    GtkTreeIter iter;
    MooFile *file = NULL;
    char *text;
    GString *prefix;
    gboolean unique;

    cmpl->priv->recheck_prefix = FALSE;

    n_items = gtk_tree_model_iter_n_children (cmpl->priv->model, NULL);

    if (!n_items)
    {
        completion_popdown (cmpl);
        g_return_val_if_reached (TRUE);
    }

    if (n_items == 1)
    {
        path = gtk_tree_path_new_from_indices (0, -1);
        gtk_tree_model_get_iter (cmpl->priv->model, &iter, path);
        gtk_tree_path_free (path);

        gtk_tree_model_get (cmpl->priv->model, &iter, COLUMN_FILE, &file, -1);
        g_return_val_if_fail (file != NULL, FALSE);

        text = completion_make_text (cmpl, file);
        completion_finish (cmpl, text);

        moo_file_unref (file);
        g_free (text);
        return TRUE;
    }

    if (cmpl->priv->display_basename && cmpl->priv->display_basename[0])
    {
        prefix = model_find_max_prefix (cmpl->priv->model, cmpl->priv->display_basename,
                                        &cmpl->priv->text_funcs, &unique, NULL);

        if (!prefix)
        {
            g_critical ("%s: oops", G_STRLOC);
            return FALSE;
        }

        if (prefix->len > cmpl->priv->display_basename_len)
        {
            text = g_strdup_printf ("%s%s", cmpl->priv->display_dirname,
                                    prefix->str);
            completion_finish (cmpl, text);
            g_free (text);
            g_string_free (prefix, TRUE);
            return TRUE;
        }

        g_string_free (prefix, TRUE);
    }

    return FALSE;
}


static void
completion_set_case_sensitive (MooFileEntryCompletion *cmpl,
                               gboolean      case_sensitive)
{
    case_sensitive = case_sensitive ? TRUE : FALSE;

    if (case_sensitive != cmpl->priv->case_sensitive)
    {
        cmpl->priv->case_sensitive = case_sensitive;

        if (case_sensitive)
        {
            cmpl->priv->text_funcs.strcmp_func = strcmp_func;
            cmpl->priv->text_funcs.strncmp_func = strncmp_func;
            cmpl->priv->text_funcs.normalize_func = normalize_func;
        }
        else
        {
            cmpl->priv->text_funcs.strcmp_func = case_strcmp_func;
            cmpl->priv->text_funcs.strncmp_func = case_strncmp_func;
            cmpl->priv->text_funcs.normalize_func = case_normalize_func;
        }

        g_object_notify (G_OBJECT (cmpl), "case-sensitive");
    }
}


static void
send_focus_change (GtkWidget *widget, gboolean in)
{
    GdkEvent *fevent = gdk_event_new (GDK_FOCUS_CHANGE);

    g_object_ref (widget);

    if (in)
        GTK_WIDGET_SET_FLAGS (widget, GTK_HAS_FOCUS);
    else
        GTK_WIDGET_UNSET_FLAGS (widget, GTK_HAS_FOCUS);

    fevent->focus_change.type = GDK_FOCUS_CHANGE;
    fevent->focus_change.window = g_object_ref (widget->window);
    fevent->focus_change.in = in;

    gtk_widget_event (widget, fevent);

    g_object_notify (G_OBJECT (widget), "has-focus");

    g_object_unref (widget);
    gdk_event_free (fevent);
}


static void
completion_popup (MooFileEntryCompletion *cmpl)
{
    GtkWidget *window;

    if (GTK_WIDGET_MAPPED (cmpl->priv->popup))
        return;

    window = gtk_widget_get_toplevel (GTK_WIDGET (cmpl->priv->entry));
    g_return_if_fail (GTK_IS_WINDOW (window));

    gtk_widget_realize (cmpl->priv->popup);

    GTK_WIDGET_SET_FLAGS (GTK_WIDGET (cmpl->priv->treeview), GTK_CAN_FOCUS);

    if (GTK_WINDOW (window)->group)
        gtk_window_group_add_window (GTK_WINDOW (window)->group,
                                     GTK_WINDOW (cmpl->priv->popup));
    gtk_window_set_modal (GTK_WINDOW (cmpl->priv->popup), TRUE);

    completion_resize_popup (cmpl);

    gtk_widget_show (cmpl->priv->popup);
    send_focus_change (GTK_WIDGET (cmpl->priv->popup), TRUE);

    /* XXX deselect and scroll */

    gtk_grab_add (cmpl->priv->popup);
    gdk_pointer_grab (cmpl->priv->popup->window, TRUE,
                      GDK_BUTTON_PRESS_MASK |
                              GDK_BUTTON_RELEASE_MASK |
                              GDK_POINTER_MOTION_MASK,
                      NULL, NULL, GDK_CURRENT_TIME);

    g_signal_connect (cmpl->priv->entry, "focus-out-event",
                      G_CALLBACK (completion_entry_focus_out), cmpl);

    g_signal_connect (cmpl->priv->popup, "button-press-event",
                      G_CALLBACK (completion_popup_button_press), cmpl);
    g_signal_connect (cmpl->priv->popup, "key-press-event",
                      G_CALLBACK (completion_popup_key_press), cmpl);

    g_signal_connect_swapped (cmpl->priv->treeview, "button-press-event",
                              G_CALLBACK (completion_list_button_press),
                              cmpl);
}


static gboolean
completion_entry_focus_out (G_GNUC_UNUSED GtkEntry *entry,
                            MooFileEntryCompletion *cmpl)
{
    completion_popdown (cmpl);
    return FALSE;
}


static void
completion_popdown (MooFileEntryCompletion *cmpl)
{
    if (!GTK_WIDGET_MAPPED (cmpl->priv->popup))
        return;

    g_signal_handlers_disconnect_by_func (cmpl->priv->entry,
                                          (gpointer) completion_entry_focus_out,
                                          cmpl);

    g_signal_handlers_disconnect_by_func (cmpl->priv->popup,
                                          (gpointer) completion_popup_button_press,
                                          cmpl);
    g_signal_handlers_disconnect_by_func (cmpl->priv->popup,
                                          (gpointer) completion_popup_key_press,
                                          cmpl);

    g_signal_handlers_disconnect_by_func (cmpl->priv->treeview,
                                          (gpointer) completion_list_button_press,
                                          cmpl);

    gdk_pointer_ungrab (GDK_CURRENT_TIME);
    gtk_grab_remove (cmpl->priv->popup);
    gtk_widget_hide (cmpl->priv->popup);
    gtk_widget_unrealize (cmpl->priv->popup);
}


static gboolean
completion_popup_button_press (G_GNUC_UNUSED GtkWidget *popup_window,
                               GdkEventButton *event,
                               MooFileEntryCompletion   *cmpl)
{
    if (event->window == cmpl->priv->popup->window)
    {
        gint width, height;
        gdk_drawable_get_size (GDK_DRAWABLE (event->window),
                               &width, &height);
        if (event->x < 0 || event->x >= width ||
            event->y < 0 || event->y >= height)
        {
            completion_popdown (cmpl);
        }
    }
    else
    {
        g_print ("click\n");
        completion_popdown (cmpl);
        return FALSE;
    }

    return TRUE;
}


static void
completion_select_iter (MooFileEntryCompletion *cmpl,
                        GtkTreeIter  *iter)
{
    MooFile *file = NULL;
    char *text;

    gtk_tree_model_get (cmpl->priv->model, iter, COLUMN_FILE, &file, -1);
    g_return_if_fail (file != NULL);

    text = completion_make_text (cmpl, file);
    completion_finish (cmpl, text);

    g_free (text);
    moo_file_unref (file);
}


static gboolean
completion_list_button_press (MooFileEntryCompletion *cmpl,
                              GdkEventButton         *event)
{
    GtkTreePath *path;
    GtkTreeIter iter;

    if (gtk_tree_view_get_path_at_pos (cmpl->priv->treeview,
                                       event->x, event->y,
                                       &path, NULL, NULL, NULL))
    {
        gtk_tree_model_get_iter (cmpl->priv->model, &iter, path);
        completion_select_iter (cmpl, &iter);
        gtk_tree_path_free (path);
        return TRUE;
    }

    g_print ("click click\n");
    return FALSE;
}


static gboolean
completion_return_key (MooFileEntryCompletion *cmpl)
{
    GtkTreeIter iter;
    GtkTreeView *treeview = cmpl->priv->treeview;
    GtkTreeSelection *selection = gtk_tree_view_get_selection (treeview);

    if (gtk_tree_selection_get_selected (selection, NULL, &iter))
        completion_select_iter (cmpl, &iter);
    else
        completion_popdown (cmpl);

    return TRUE;
}


static int
completion_get_selected (MooFileEntryCompletion *cmpl)
{
    int ind;
    GtkTreeIter iter;
    GtkTreePath *path;
    GtkTreeView *treeview = cmpl->priv->treeview;
    GtkTreeSelection *selection = gtk_tree_view_get_selection (treeview);

    if (gtk_tree_selection_get_selected (selection, NULL, &iter))
    {
        path = gtk_tree_model_get_path (cmpl->priv->model, &iter);
        g_return_val_if_fail (path != NULL, -1);
        ind = gtk_tree_path_get_indices (path)[0];
        gtk_tree_path_free (path);
        return ind;
    }
    else
    {
        return -1;
    }
}


static void
completion_move_selection (MooFileEntryCompletion *cmpl,
                           GdkEventKey            *event)
{
    int n_items, current_item, new_item = -1;
    GtkTreePath *path;
    GtkTreeSelection *selection;

    n_items = gtk_tree_model_iter_n_children (cmpl->priv->model, NULL);
    g_return_if_fail (n_items != 0);

    current_item = completion_get_selected (cmpl);

    switch (event->keyval)
    {
        case GDK_Down:
        case GDK_KP_Down:
            if (current_item < n_items - 1)
                new_item = current_item + 1;
            else
                new_item = -1;
            break;

        case GDK_Up:
        case GDK_KP_Up:
            if (current_item < 0)
                new_item = n_items - 1;
            else
                new_item = current_item - 1;
            break;

        case GDK_Page_Down:
        case GDK_KP_Page_Down:
            new_item = current_item + COMPLETION_POPUP_LEN - 1;
            if (new_item >= n_items)
                new_item = n_items - 1;
            break;

        case GDK_Page_Up:
        case GDK_KP_Page_Up:
            new_item = current_item - COMPLETION_POPUP_LEN + 1;
            if (new_item < 0)
                new_item = 0;
            break;

        case GDK_Tab:
        case GDK_KP_Tab:
            if (current_item < n_items - 1)
                new_item = current_item + 1;
            else
                new_item = 0;
            break;

        case GDK_ISO_Left_Tab:
            if (current_item <= 0)
                new_item = n_items - 1;
            else
                new_item = current_item - 1;
            break;

        default:
            g_return_if_reached ();
    }

    selection = gtk_tree_view_get_selection (cmpl->priv->treeview);
    gtk_tree_selection_unselect_all (selection);

    if (new_item >= 0)
    {
        path = gtk_tree_path_new_from_indices (new_item, -1);
        gtk_tree_selection_select_path (selection, path);
        gtk_tree_view_scroll_to_cell (cmpl->priv->treeview, path, NULL, FALSE, 0, 0);
        gtk_tree_path_free (path);
    }
}


static gboolean
completion_popup_key_press (G_GNUC_UNUSED GtkWidget *popup,
                            GdkEventKey            *event,
                            MooFileEntryCompletion *cmpl)
{
    switch (event->keyval)
    {
        case GDK_Down:
        case GDK_Up:
        case GDK_KP_Down:
        case GDK_KP_Up:
        case GDK_Page_Down:
        case GDK_Page_Up:
            completion_move_selection (cmpl, event);
            return TRUE;

        case GDK_Tab:
        case GDK_KP_Tab:
            if (cmpl->priv->recheck_prefix && completion_popup_tab_key_press (cmpl))
                return TRUE;
            /* fallthrough */
        case GDK_ISO_Left_Tab:
            completion_move_selection (cmpl, event);
            return TRUE;

        case GDK_Escape:
            completion_popdown (cmpl);
            return TRUE;

        case GDK_Return:
        case GDK_ISO_Enter:
        case GDK_KP_Enter:
            return completion_return_key (cmpl);

        default:
            if (gtk_widget_event (GTK_WIDGET (cmpl->priv->entry), (GdkEvent*) event))
            {
                completion_entry_changed (cmpl->priv->entry, cmpl);
                return TRUE;
            }
            else
            {
                return FALSE;
            }
    }
}


static void
completion_connect_folder (MooFileEntryCompletion *cmpl,
                           MooFolder              *folder)
{
    moo_folder_filter_set_folder (MOO_FOLDER_FILTER (cmpl->priv->model), folder);
}


static void
completion_disconnect_folder (MooFileEntryCompletion *cmpl)
{
    moo_folder_filter_set_folder (MOO_FOLDER_FILTER (cmpl->priv->model), NULL);
}


static void
completion_create_popup (MooFileEntryCompletion *cmpl)
{
    GtkCellRenderer *cell;
    GtkWidget *scrolled_window, *frame;
    GtkTreeSelection *selection;

    cmpl->priv->popup = gtk_window_new (GTK_WINDOW_POPUP);
    gtk_widget_set_size_request (cmpl->priv->popup, -1, -1);
    gtk_window_set_default_size (GTK_WINDOW (cmpl->priv->popup), 1, 1);
    gtk_window_set_resizable (GTK_WINDOW (cmpl->priv->popup), FALSE);
    gtk_widget_add_events (cmpl->priv->popup, GDK_KEY_PRESS_MASK | GDK_BUTTON_PRESS_MASK);

    cell = gtk_cell_renderer_text_new ();
    cmpl->priv->column = gtk_tree_view_column_new ();
    gtk_tree_view_column_pack_start (cmpl->priv->column, cell, TRUE);
    gtk_tree_view_column_set_cell_data_func (cmpl->priv->column, cell,
                                             (GtkTreeCellDataFunc) completion_cell_data_func,
                                             cmpl, NULL);

    cmpl->priv->treeview = GTK_TREE_VIEW (gtk_tree_view_new_with_model (cmpl->priv->model));
    gtk_widget_set_size_request (GTK_WIDGET (cmpl->priv->treeview), -1, -1);
    gtk_tree_view_append_column (cmpl->priv->treeview, cmpl->priv->column);
    gtk_tree_view_set_headers_visible (cmpl->priv->treeview, FALSE);
    gtk_tree_view_set_hover_selection (cmpl->priv->treeview, TRUE);

    selection = gtk_tree_view_get_selection (cmpl->priv->treeview);
    gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);

    scrolled_window = gtk_scrolled_window_new (NULL, NULL);
    gtk_widget_set_size_request (scrolled_window, -1, -1);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                    GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
//     gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_window),
//                                          GTK_SHADOW_ETCHED_IN);
    gtk_container_add (GTK_CONTAINER (scrolled_window), GTK_WIDGET (cmpl->priv->treeview));

    frame = gtk_frame_new (NULL);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_IN);
    gtk_container_add (GTK_CONTAINER (frame), scrolled_window);

    gtk_widget_show_all (frame);
    gtk_container_add (GTK_CONTAINER (cmpl->priv->popup), frame);
}


static void
completion_cell_data_func (G_GNUC_UNUSED GtkTreeViewColumn *column,
                           GtkCellRenderer        *cell,
                           GtkTreeModel           *model,
                           GtkTreeIter            *iter,
                           MooFileEntryCompletion *cmpl)
{
    MooFile *file = NULL;
    char *text;

    if (!cmpl->priv->display_dirname)
        return;

    gtk_tree_model_get (model, iter, COLUMN_FILE, &file, -1);
    g_return_if_fail (file != NULL);

    text = completion_make_text (cmpl, file);
    g_object_set (cell, "text", text, NULL);

    g_free (text);
    moo_file_unref (file);
}


static gboolean
completion_visible_func (GtkTreeModel           *model,
                         GtkTreeIter            *iter,
                         MooFileEntryCompletion *cmpl)
{
    MooFile *file = NULL;
    gboolean visible = FALSE;

    if (!cmpl->priv->display_basename)
        return FALSE;

    g_return_val_if_fail (cmpl->priv->visible_func != NULL, FALSE);

    gtk_tree_model_get (model, iter, COLUMN_FILE, &file, -1);
    g_return_val_if_fail (file != NULL, FALSE);

    if (cmpl->priv->dirs_only && !MOO_FILE_IS_DIR (file))
    {
        visible = FALSE;
    }
    else if (!cmpl->priv->show_hidden &&
              MOO_FILE_IS_HIDDEN (file) &&
              !cmpl->priv->display_basename[0])
    {
        visible = FALSE;
    }
    else if (cmpl->priv->visible_func (file, cmpl->priv->visible_func_data))
    {
        visible = !cmpl->priv->text_funcs.strncmp_func (cmpl->priv->display_basename, file,
                                                        cmpl->priv->display_basename_len);
    }

    moo_file_unref (file);
    return visible ? TRUE : FALSE; /* #314335 */
}


/* XXX _gtk_entry_get_borders from gtkentry.c */
static void
entry_get_borders (GtkEntry *entry,
                   gint     *xborder,
                   gint     *yborder)
{
    GtkWidget *widget = GTK_WIDGET (entry);
    gint focus_width;
    gboolean interior_focus;

    gtk_widget_style_get (widget,
                          "interior-focus", &interior_focus,
                          "focus-line-width", &focus_width,
                          NULL);

    if (entry->has_frame)
    {
        *xborder = widget->style->xthickness;
        *yborder = widget->style->ythickness;
    }
    else
    {
        *xborder = 0;
        *yborder = 0;
    }

    if (!interior_focus)
    {
        *xborder += focus_width;
        *yborder += focus_width;
    }
}


/* XXX gtk_entry_resize_popup from gtkentrycompletion.c */
static gboolean
completion_resize_popup (MooFileEntryCompletion *cmpl)
{
    GtkWidget *widget = GTK_WIDGET (cmpl->priv->entry);
    gint x, y;
    gint matches, items, height, x_border, y_border;
    GdkScreen *screen;
    gint monitor_num;
    GdkRectangle monitor;
    GtkRequisition popup_req;
    GtkRequisition entry_req;
    gboolean above;
    gint width, vert_separator = 0;

    g_return_val_if_fail (GTK_WIDGET_REALIZED (cmpl->priv->entry), FALSE);

    gdk_window_get_origin (widget->window, &x, &y);
    /* XXX */
    entry_get_borders (GTK_ENTRY (cmpl->priv->entry), &x_border, &y_border);

    matches = gtk_tree_model_iter_n_children (GTK_TREE_MODEL (cmpl->priv->model), NULL);

    items = MIN (matches, COMPLETION_POPUP_LEN);

    gtk_tree_view_column_cell_get_size (cmpl->priv->column, NULL,
                                        NULL, NULL, NULL, &height);

    screen = gtk_widget_get_screen (widget);
    monitor_num = gdk_screen_get_monitor_at_window (screen, widget->window);
    gdk_screen_get_monitor_geometry (screen, monitor_num, &monitor);

    width = MIN (widget->allocation.width, monitor.width) - 2 * x_border;
    gtk_widget_style_get (GTK_WIDGET (cmpl->priv->treeview), "vertical-separator",
                          &vert_separator, NULL);
    gtk_widget_set_size_request (GTK_WIDGET (cmpl->priv->treeview), width,
                                 items * (height + vert_separator));

    gtk_widget_set_size_request (cmpl->priv->popup, -1, -1);
    gtk_widget_size_request (cmpl->priv->popup, &popup_req);
    gtk_widget_size_request (widget, &entry_req);

    if (x < monitor.x)
        x = monitor.x;
    else if (x + popup_req.width > monitor.x + monitor.width)
        x = monitor.x + monitor.width - popup_req.width;

    if (y + entry_req.height + popup_req.height <= monitor.y + monitor.height)
    {
        y += entry_req.height;
        above = FALSE;
    }
    else
    {
        y -= popup_req.height;
        above = TRUE;
    }

    gtk_window_move (GTK_WINDOW (cmpl->priv->popup), x, y);

    return above;
}


static void
completion_set_file_system (MooFileEntryCompletion *cmpl,
                            MooFileSystem          *fs)
{
    g_return_if_fail (!fs || MOO_IS_FILE_SYSTEM (fs));

    if (fs && fs == cmpl->priv->fs)
        return;

    if (cmpl->priv->fs)
        g_object_unref (cmpl->priv->fs);

    if (fs)
        cmpl->priv->fs = g_object_ref (fs);
    else
        cmpl->priv->fs = moo_file_system_create ();

    g_object_notify (G_OBJECT (cmpl), "file-system");
}


static void
completion_entry_destroyed (MooFileEntryCompletion *cmpl)
{
    cmpl->priv->entry = NULL;
    g_object_notify (G_OBJECT (cmpl), "entry");
}


void
moo_file_entry_completion_set_entry (MooFileEntryCompletion *cmpl,
                                     GtkEntry               *entry)
{
    g_return_if_fail (MOO_IS_FILE_ENTRY_COMPLETION (cmpl));
    g_return_if_fail (!entry || GTK_IS_ENTRY (entry));

    if (entry == cmpl->priv->entry)
        return;

    if (cmpl->priv->entry)
    {
        if (!cmpl->priv->managed)
        {
            g_signal_handlers_disconnect_by_func (cmpl->priv->entry,
                    (gpointer) completion_entry_changed,
                    cmpl);
            g_signal_handlers_disconnect_by_func (cmpl->priv->entry,
                    (gpointer) completion_entry_key_press,
                    cmpl);
        }

        g_object_weak_unref (G_OBJECT (cmpl->priv->entry),
                             (GWeakNotify) completion_entry_destroyed,
                             cmpl);
    }

    cmpl->priv->entry = entry;

    if (cmpl->priv->entry)
    {
        g_object_weak_ref (G_OBJECT (cmpl->priv->entry),
                           (GWeakNotify) completion_entry_destroyed,
                           cmpl);
        if (!cmpl->priv->managed)
        {
            g_signal_connect (cmpl->priv->entry, "changed",
                              G_CALLBACK (completion_entry_changed), cmpl);
            g_signal_connect (cmpl->priv->entry, "key-press-event",
                              G_CALLBACK (completion_entry_key_press), cmpl);
        }
    }

    g_object_notify (G_OBJECT (cmpl), "entry");
}


static gboolean
completion_entry_key_press (GtkEntry               *entry,
                            GdkEventKey            *event,
                            MooFileEntryCompletion *cmpl)
{
    g_return_val_if_fail (entry == cmpl->priv->entry, FALSE);

    if (cmpl->priv->enabled &&
        !(event->state & (GDK_CONTROL_MASK | GDK_SHIFT_MASK | GDK_MOD1_MASK)) &&
        event->keyval == GDK_Tab)
    {
        completion_tab_key (cmpl);
        return TRUE;
    }

    return FALSE;
}


static gboolean
completion_default_visible_func (MooFile *file)
{
    return file && strcmp (moo_file_name (file), "..");
}


void
moo_file_entry_completion_set_visible_func (MooFileEntryCompletion *cmpl,
                                            MooFileVisibleFunc      func,
                                            gpointer                data)
{
    g_return_if_fail (MOO_IS_FILE_ENTRY_COMPLETION (cmpl));

    if (!func)
        func = (MooFileVisibleFunc) completion_default_visible_func;

    cmpl->priv->visible_func = func;
    cmpl->priv->visible_func_data = data;
}


void
moo_file_entry_completion_set_path (MooFileEntryCompletion *cmpl,
                                    const char             *path)
{
    char *path_utf8;

    g_return_if_fail (MOO_IS_FILE_ENTRY_COMPLETION (cmpl));
    g_return_if_fail (path != NULL);
    g_return_if_fail (cmpl->priv->entry != NULL);

    path_utf8 = g_filename_display_name (path);
    g_return_if_fail (path_utf8 != NULL);

    gtk_entry_set_text (GTK_ENTRY (cmpl->priv->entry), path_utf8);
    g_free (path_utf8);
}


/*****************************************************************************/
/* MooFileEntry
 */

struct _MooFileEntryPrivate {
    MooFileEntryCompletion *completion;
};


static void     moo_file_entry_set_property (GObject        *object,
                                             guint           prop_id,
                                             const GValue   *value,
                                             GParamSpec     *pspec);
static void     moo_file_entry_get_property (GObject        *object,
                                             guint           prop_id,
                                             GValue         *value,
                                             GParamSpec     *pspec);
static void     moo_file_entry_destroy      (GtkObject      *object);
static gboolean moo_file_entry_key_press    (GtkWidget      *widget,
                                             GdkEventKey    *event);


/* MOO_TYPE_FILE_ENTRY */
G_DEFINE_TYPE (MooFileEntry, moo_file_entry, MOO_TYPE_ENTRY)

enum {
    ENTRY_PROP_0,
    ENTRY_PROP_COMPLETION
};

static void
moo_file_entry_class_init (MooFileEntryClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GtkObjectClass *gtkobject_class = GTK_OBJECT_CLASS (klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

    gobject_class->set_property = moo_file_entry_set_property;
    gobject_class->get_property = moo_file_entry_get_property;
    gtkobject_class->destroy = moo_file_entry_destroy;
    widget_class->key_press_event = moo_file_entry_key_press;

    g_object_class_install_property (gobject_class,
                                     ENTRY_PROP_COMPLETION,
                                     g_param_spec_object ("completion",
                                             "completion",
                                             "completion",
                                             MOO_TYPE_FILE_ENTRY_COMPLETION,
                                             G_PARAM_READABLE));
}


static void
moo_file_entry_init (G_GNUC_UNUSED MooFileEntry *entry)
{
    entry->completion = g_object_new (MOO_TYPE_FILE_ENTRY_COMPLETION, NULL);
    entry->completion->priv->managed = TRUE;
    moo_file_entry_completion_set_entry (entry->completion, GTK_ENTRY (entry));
}


static void
moo_file_entry_destroy (GtkObject *object)
{
    MooFileEntry *entry = MOO_FILE_ENTRY (object);

    if (entry->completion)
    {
        moo_file_entry_completion_set_entry (entry->completion, NULL);
        g_object_unref (entry->completion);
        entry->completion = NULL;
    }

    GTK_OBJECT_CLASS(moo_file_entry_parent_class)->destroy (object);
}


static void
moo_file_entry_set_property (GObject        *object,
                             guint           prop_id,
                             G_GNUC_UNUSED const GValue *value,
                             GParamSpec     *pspec)
{
    switch (prop_id)
    {
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}


static void
moo_file_entry_get_property (GObject        *object,
                             guint           prop_id,
                             GValue         *value,
                             GParamSpec     *pspec)
{
    MooFileEntry *entry = MOO_FILE_ENTRY (object);

    switch (prop_id)
    {
        case ENTRY_PROP_COMPLETION:
            g_value_set_object (value, entry->completion);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}


GtkWidget*
moo_file_entry_new (void)
{
    return g_object_new (MOO_TYPE_FILE_ENTRY, NULL);
}


static gboolean
moo_file_entry_key_press (GtkWidget      *widget,
                          GdkEventKey    *event)
{
    return completion_entry_key_press (GTK_ENTRY (widget), event,
                                       MOO_FILE_ENTRY(widget)->completion) ||
            GTK_WIDGET_CLASS(moo_file_entry_parent_class)->key_press_event (widget, event);
}
