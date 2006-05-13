/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *   fileselector.c
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mooedit/plugins/moofileselector.h"
#include "mooedit/mooplugin-macro.h"
#include "moofileview/moobookmarkmgr.h"
#include "mooedit/plugins/mooeditplugins.h"
#include "mooedit/plugins/moofileselector-glade.h"
#include "mooedit/plugins/moofilechooser.h"
#include "mooutils/moostock.h"
#include "mooutils/mooutils-fs.h"
#include "mooutils/mooutils-misc.h"
#include "mooutils/mooglade.h"
#include "mooutils/mooentry.h"
#include "mooutils/moodialogs.h"
#include "mooutils/mooactionfactory.h"
#include <string.h>
#include <gmodule.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#ifndef MOO_VERSION
#define MOO_VERSION NULL
#endif

#define PLUGIN_ID "FileSelector"
#define DIR_PREFS MOO_PLUGIN_PREFS_ROOT "/" PLUGIN_ID "/last_dir"


typedef struct {
    MooPlugin parent;
    MooBookmarkMgr *bookmark_mgr;
    guint ui_merge_id;
} FileSelectorPlugin;

#define Plugin FileSelectorPlugin


enum {
    TARGET_MOO_EDIT_TAB = 1,
    TARGET_URI_LIST = 2
};

static GdkAtom moo_edit_tab_atom;

static GtkTargetEntry targets[] = {
    { (char*) "MOO_EDIT_TAB", GTK_TARGET_SAME_APP, TARGET_MOO_EDIT_TAB },
    { (char*) "text/uri-list", 0, TARGET_URI_LIST }
};

#define OPEN_PANE_TIMEOUT 300

G_DEFINE_TYPE (MooFileSelector, moo_file_selector, MOO_TYPE_FILE_VIEW)


enum {
    PROP_0,
    PROP_WINDOW
};


static void     moo_file_selector_set_property  (GObject        *object,
                                                 guint           prop_id,
                                                 const GValue   *value,
                                                 GParamSpec     *pspec);
static void     moo_file_selector_get_property  (GObject        *object,
                                                 guint           prop_id,
                                                 GValue         *value,
                                                 GParamSpec     *pspec);
static GObject *moo_file_selector_constructor   (GType           type,
                                                 guint           n_props,
                                                 GObjectConstructParam *props);
static void     moo_file_selector_finalize      (GObject        *object);

static gboolean moo_file_selector_chdir     (MooFileView    *fileview,
                                             const char     *dir,
                                             GError        **error);
static void     moo_file_selector_activate  (MooFileView    *fileview,
                                             const char     *path);
static gboolean moo_file_selector_drop      (MooFileView    *fileview,
                                             const char     *path,
                                             GtkWidget      *widget,
                                             GdkDragContext *context,
                                             int             x,
                                             int             y,
                                             guint           time);
static gboolean moo_file_selector_drop_data_received (MooFileView *fileview,
                                             const char     *path,
                                             GtkWidget      *widget,
                                             GdkDragContext *context,
                                             int             x,
                                             int             y,
                                             GtkSelectionData *data,
                                             guint           info,
                                             guint           time);

static void     moo_file_selector_drop_doc  (MooFileSelector *filesel,
                                             MooEdit        *doc,
                                             const char     *destdir,
                                             GtkWidget      *widget,
                                             GdkDragContext *context,
                                             int             x,
                                             int             y,
                                             guint           time);

static void     button_drag_leave           (GtkWidget      *button,
                                             GdkDragContext *context,
                                             guint           time,
                                             MooFileSelector *filesel);
static gboolean button_drag_motion          (GtkWidget      *button,
                                             GdkDragContext *context,
                                             int             x,
                                             int             y,
                                             guint           time,
                                             MooFileSelector *filesel);


static void
moo_file_selector_class_init (MooFileSelectorClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    MooFileViewClass *fileview_class = MOO_FILE_VIEW_CLASS (klass);

    moo_edit_tab_atom = gdk_atom_intern ("MOO_EDIT_TAB", FALSE);

    gobject_class->set_property = moo_file_selector_set_property;
    gobject_class->get_property = moo_file_selector_get_property;
    gobject_class->constructor = moo_file_selector_constructor;
    gobject_class->finalize = moo_file_selector_finalize;

    fileview_class->chdir = moo_file_selector_chdir;
    fileview_class->activate = moo_file_selector_activate;
    fileview_class->drop = moo_file_selector_drop;
    fileview_class->drop_data_received = moo_file_selector_drop_data_received;

    g_object_class_install_property (gobject_class,
                                     PROP_WINDOW,
                                     g_param_spec_object ("window",
                                             "window",
                                             "window",
                                             MOO_TYPE_EDIT_WINDOW,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
}


static void
moo_file_selector_init (MooFileSelector *filesel)
{
    filesel->targets = gtk_target_list_new (targets, G_N_ELEMENTS (targets));
    _moo_file_view_add_target (MOO_FILE_VIEW (filesel), moo_edit_tab_atom,
                               GTK_TARGET_SAME_APP, TARGET_MOO_EDIT_TAB);
}


static void
moo_file_selector_finalize (GObject *object)
{
    MooFileSelector *filesel = MOO_FILE_SELECTOR (object);
    gtk_target_list_unref (filesel->targets);
    G_OBJECT_CLASS(moo_file_selector_parent_class)->finalize (object);
}


static void
moo_file_selector_set_property (GObject        *object,
                                guint           prop_id,
                                const GValue   *value,
                                GParamSpec     *pspec)
{
    MooFileSelector *sel = MOO_FILE_SELECTOR (object);

    switch (prop_id)
    {
        case PROP_WINDOW:
            sel->window = g_value_get_object (value);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void
moo_file_selector_get_property (GObject        *object,
                                guint           prop_id,
                                GValue         *value,
                                GParamSpec     *pspec)
{
    MooFileSelector *sel = MOO_FILE_SELECTOR (object);

    switch (prop_id)
    {
        case PROP_WINDOW:
            g_value_set_object (value, sel->window);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static gboolean
file_selector_go_home (MooFileView *fileview)
{
    const char *dir;
    char *real_dir = NULL;

    dir = moo_prefs_get_string (DIR_PREFS);

    if (dir)
        real_dir = g_filename_from_utf8 (dir, -1, NULL, NULL, NULL);

    if (!real_dir || !_moo_file_view_chdir (fileview, real_dir, NULL))
        g_signal_emit_by_name (fileview, "go-home");

    /* it's refed in g_idle_add() */
    g_object_unref (fileview);
    g_free (real_dir);
    return FALSE;
}


static gboolean
moo_file_selector_chdir (MooFileView    *fileview,
                         const char     *dir,
                         GError        **error)
{
    gboolean result;

    result = MOO_FILE_VIEW_CLASS(moo_file_selector_parent_class)->chdir (fileview, dir, error);

    if (result)
    {
        char *dir = NULL;
        g_object_get (fileview, "current-directory", &dir, NULL);
        moo_prefs_set_filename (DIR_PREFS, dir);
        g_free (dir);
    }

    return result;
}


static void
moo_file_selector_activate (MooFileView    *fileview,
                            const char     *path)
{
    MooFileSelector *filesel = MOO_FILE_SELECTOR (fileview);
    moo_editor_open_file (moo_edit_window_get_editor (filesel->window),
                          filesel->window, GTK_WIDGET (filesel), path, NULL);
}


static void
goto_current_doc_dir (MooFileSelector *filesel)
{
    MooEdit *doc;
    const char *filename;

    doc = moo_edit_window_get_active_doc (filesel->window);
    filename = doc ? moo_edit_get_filename (doc) : NULL;

    if (filename)
    {
        char *dirname = g_path_get_dirname (filename);
        _moo_file_view_chdir (MOO_FILE_VIEW (filesel), dirname, NULL);
        g_free (dirname);
    }
}


static GObject *
moo_file_selector_constructor (GType           type,
                               guint           n_props,
                               GObjectConstructParam *props)
{
    MooEditor *editor;
    MooPaneLabel *label;
    MooUIXML *xml;
    MooFileSelector *filesel;
    MooFileView *fileview;
    GObject *object;

    object = G_OBJECT_CLASS(moo_file_selector_parent_class)->constructor (type, n_props, props);
    filesel = MOO_FILE_SELECTOR (object);
    fileview = MOO_FILE_VIEW (object);

    g_return_val_if_fail (filesel->window != NULL, object);

    editor = moo_edit_window_get_editor (filesel->window);

    g_idle_add ((GSourceFunc) file_selector_go_home, g_object_ref (filesel));

    moo_action_group_add_action (_moo_file_view_get_actions (MOO_FILE_VIEW (fileview)),
                                 "GoToCurrentDocDir",
                                 "stock-id", GTK_STOCK_JUMP_TO,
                                 "tooltip", "Go to current document directory",
                                 "closure-object", filesel,
                                 "closure-callback", goto_current_doc_dir,
                                 NULL);

    xml = _moo_file_view_get_ui_xml (MOO_FILE_VIEW (fileview));
    moo_ui_xml_insert_markup (xml, moo_ui_xml_new_merge_id (xml),
                              "MooFileView/Toolbar", -1,
                              "<item action=\"GoToCurrentDocDir\"/>");

    label = moo_pane_label_new (MOO_STOCK_FILE_SELECTOR,
                                NULL, NULL/*button*/, "File Selector",
                                "File Selector");
    moo_edit_window_add_pane (filesel->window, PLUGIN_ID, GTK_WIDGET (filesel),
                              label, MOO_PANE_POS_RIGHT);
    moo_pane_label_free (label);

    filesel->button = moo_big_paned_get_button (filesel->window->paned,
                                       GTK_WIDGET (filesel));
    g_return_val_if_fail (filesel->button != NULL, object);

    gtk_drag_dest_set (filesel->button, 0,
                       targets, G_N_ELEMENTS (targets),
                       GDK_ACTION_COPY | GDK_ACTION_MOVE | GDK_ACTION_LINK);
    g_signal_connect (filesel->button, "drag-motion",
                      G_CALLBACK (button_drag_motion), filesel);
    g_signal_connect (filesel->button, "drag-leave",
                      G_CALLBACK (button_drag_leave), filesel);

    return object;
}


static void
button_drag_leave (GtkWidget      *button,
                   G_GNUC_UNUSED GdkDragContext *context,
                   G_GNUC_UNUSED guint           time,
                   MooFileSelector *filesel)
{
    if (filesel->open_pane_timeout)
        g_source_remove (filesel->open_pane_timeout);
    filesel->open_pane_timeout = 0;
    if (filesel->button_highlight)
        gtk_drag_unhighlight (button);
    filesel->button_highlight = FALSE;
}


static gboolean
drag_open_pane (MooFileSelector *filesel)
{
    MooPaned *paned;
    int pane_index;

    if (!moo_big_paned_find_pane (filesel->window->paned,
                                  GTK_WIDGET (filesel),
                                  &paned, &pane_index))
    {
        g_critical ("%s: oops", G_STRLOC);
        goto out;
    }

    moo_paned_open_pane (paned, pane_index);

    if (filesel->button_highlight)
        gtk_drag_unhighlight (filesel->button);
    filesel->button_highlight = FALSE;

out:
    filesel->open_pane_timeout = 0;
    return FALSE;
}


static gboolean
button_drag_motion (GtkWidget      *button,
                    G_GNUC_UNUSED GdkDragContext *context,
                    G_GNUC_UNUSED int x,
                    G_GNUC_UNUSED int y,
                    G_GNUC_UNUSED guint time,
                    MooFileSelector *filesel)
{
    MooPaneParams *params = NULL;
    MooPaned *paned;
    int pane_index;

    if (!moo_big_paned_find_pane (filesel->window->paned,
                                  GTK_WIDGET (filesel),
                                  &paned, &pane_index))
    {
        g_critical ("%s: oops", G_STRLOC);
        goto out;
    }

    if (moo_paned_is_open (paned) &&
        moo_paned_get_open_pane (paned) == pane_index)
    {
        goto out;
    }

    params = moo_paned_get_pane_params (paned, pane_index);

    if (params->detached)
        goto out;

    if (!filesel->button_highlight)
    {
        gtk_drag_highlight (button);
        filesel->button_highlight = TRUE;
    }

    if (!filesel->open_pane_timeout)
        filesel->open_pane_timeout = g_timeout_add (OPEN_PANE_TIMEOUT,
                                                    (GSourceFunc) drag_open_pane,
                                                    filesel);

    gdk_drag_status (context, 0, time);

    moo_pane_params_free (params);
    return TRUE;

out:
    if (filesel->button_highlight)
        gtk_drag_unhighlight (button);
    filesel->button_highlight = FALSE;
    if (params)
        moo_pane_params_free (params);
    if (filesel->open_pane_timeout)
        g_source_remove (filesel->open_pane_timeout);
    filesel->open_pane_timeout = 0;

    gdk_drag_status (context, 0, time);

    return TRUE;
}


static gboolean
moo_file_selector_drop (MooFileView    *fileview,
                        const char     *path,
                        GtkWidget      *widget,
                        GdkDragContext *context,
                        int             x,
                        int             y,
                        guint           time)
{
    MooFileSelector *filesel = MOO_FILE_SELECTOR (fileview);
    GdkAtom target;

    filesel->waiting_for_tab = FALSE;
    target = gtk_drag_dest_find_target (widget, context, filesel->targets);

    if (target != moo_edit_tab_atom)
        return MOO_FILE_VIEW_CLASS(moo_file_selector_parent_class)->
                drop (fileview, path, widget, context, x, y, time);

    filesel->waiting_for_tab = TRUE;

    gtk_drag_get_data (widget, context, moo_edit_tab_atom, time);

    return TRUE;
}


static gboolean
moo_file_selector_drop_data_received (MooFileView *fileview,
                                      const char     *path,
                                      GtkWidget      *widget,
                                      GdkDragContext *context,
                                      int             x,
                                      int             y,
                                      GtkSelectionData *data,
                                      guint           info,
                                      guint           time)
{
    MooEdit *doc;
    MooFileSelector *filesel = MOO_FILE_SELECTOR (fileview);

    if (!filesel->waiting_for_tab)
        goto parent;

    if (info != TARGET_MOO_EDIT_TAB)
    {
        g_critical ("%s: oops", G_STRLOC);
        goto parent;
    }
    else if (data->length < 0)
    {
        g_warning ("%s: could not get MOO_EDIT_TAB data", G_STRLOC);
        goto error;
    }

    doc = moo_selection_data_get_pointer (data, moo_edit_tab_atom);

    if (!MOO_IS_EDIT (doc))
    {
        g_critical ("%s: oops", G_STRLOC);
        goto error;
    }

    moo_file_selector_drop_doc (MOO_FILE_SELECTOR (fileview),
                                doc, path, widget, context,
                                x, y, time);
    return TRUE;

error:
    _moo_file_view_drag_finish (fileview, context, FALSE, FALSE, time);
    return FALSE;

parent:
    return MOO_FILE_VIEW_CLASS(moo_file_selector_parent_class)->
                drop_data_received (fileview, path, widget, context,
                                    x, y, data, info, time);
}


static GtkWidget *
create_save_as_dialog (GtkWidget   *parent,
                       const char  *start_text,
                       const char  *title,
                       MooGladeXML **xml)
{
    GtkWidget *dialog, *button;
    GtkEntry *entry;

    *xml = moo_glade_xml_new_empty ();
    moo_glade_xml_map_class (*xml, "GtkEntry", MOO_TYPE_ENTRY);
    moo_glade_xml_parse_memory (*xml, MOO_FILE_SELECTOR_GLADE_XML, -1, NULL);

    dialog = moo_glade_xml_get_widget (*xml, "save_untitled_dialog");
    g_return_val_if_fail (dialog != NULL, NULL);

    gtk_window_set_title (GTK_WINDOW (dialog), title);

    moo_position_window (dialog, parent, FALSE, FALSE, 0, 0);

    entry = moo_glade_xml_get_widget (*xml, "entry");

    gtk_entry_set_text (entry, start_text);
    moo_entry_clear_undo (MOO_ENTRY (entry));

    gtk_widget_show_all (dialog);
    gtk_widget_grab_focus (GTK_WIDGET (entry));

    button = moo_glade_xml_get_widget (*xml, "ok_button");
    moo_bind_bool_property (button, "sensitive", entry, "empty", TRUE);

    return dialog;
}

static char *
save_as_dialog (GtkWidget   *parent,
                const char  *dirname,
                const char  *start_name,
                gboolean     ask_name,
                const char  *title)
{
    MooGladeXML *xml = NULL;
    GtkWidget *dialog = NULL;
    GtkEntry *entry = NULL;
    char *fullname = NULL;
    gboolean first_time = TRUE;

    g_return_val_if_fail (dirname != NULL, NULL);

    while (TRUE)
    {
        const char *text;
        char *name, *display_dirname;

        if (!ask_name)
        {
            ask_name = TRUE;
            text = start_name;
        }
        else
        {
            if (!dialog)
            {
                dialog = create_save_as_dialog (parent, start_name, title, &xml);
                entry = moo_glade_xml_get_widget (xml, "entry");
            }

            if (gtk_dialog_run (GTK_DIALOG (dialog)) != GTK_RESPONSE_OK)
                goto out;

            text = gtk_entry_get_text (entry);
        }

        first_time = FALSE;

        if (!text[0])
        {
            g_critical ("%s: ooops", G_STRLOC);
            goto out;
        }

        /* XXX error checking, you know */
        name = g_filename_from_utf8 (text, -1, NULL, NULL, NULL);

        if (!name)
        {
            char *err_text, *sec_text;
            err_text = g_strdup_printf ("Could not save file as '%s'", text);
            sec_text = g_strdup_printf ("Could not convert '%s' to filename encoding.\n"
                                        "Please consider simpler name, such as foo.blah "
                                        "or blah.foo", text);
            moo_error_dialog (dialog ? dialog : parent, err_text, sec_text);
            g_free (err_text);
            g_free (sec_text);
            continue;
        }

        fullname = g_build_filename (dirname, name, NULL);
        g_free (name);

        if (!g_file_test (fullname, G_FILE_TEST_EXISTS))
            goto out;

        display_dirname = g_filename_display_name (dirname);

        if (moo_overwrite_file_dialog (dialog ? dialog : parent, text, display_dirname))
        {
            g_free (display_dirname);
            goto out;
        }

        g_free (display_dirname);
        g_free (fullname);
        fullname = NULL;
    }

out:
    if (xml)
        g_object_unref (xml);
    if (dialog)
        gtk_widget_destroy (dialog);
    return fullname;
}


static void
select_file (MooFileSelector *filesel,
             const char      *filename,
             const char      *dirname)
{
    char *curdir = NULL;

    g_object_get (filesel, "current-directory", &curdir, NULL);

    if (curdir && !strcmp (curdir, dirname))
    {
        char *basename = g_path_get_basename (filename);
        _moo_file_view_select_name (MOO_FILE_VIEW (filesel), basename);
        g_free (basename);
    }

    g_free (curdir);
}


static gboolean
drop_untitled (MooFileSelector *filesel,
               MooEdit        *doc,
               const char     *destdir,
               GtkWidget      *widget,
               G_GNUC_UNUSED GdkDragContext *context,
               G_GNUC_UNUSED int             x,
               G_GNUC_UNUSED int             y,
               G_GNUC_UNUSED guint time)
{
    char *name;
    gboolean result;

    name = save_as_dialog (widget, destdir,
                           moo_edit_get_display_basename (doc),
                           TRUE, "Save As");

    if (!name)
        return FALSE;

    result = moo_edit_save_as (doc, name, NULL, NULL);

    if (result)
        select_file (filesel, name, destdir);

    g_free (name);
    return result;
}


static void
doc_save_as (MooFileSelector *filesel,
             MooEdit         *doc,
             gboolean         ask_name,
             const char      *destdir)
{
    char *filename;

    filename = save_as_dialog (GTK_WIDGET (filesel), destdir,
                               moo_edit_get_display_basename (doc),
                               ask_name, "Save As");

    if (filename)
    {
        if (moo_edit_save_as (doc, filename, moo_edit_get_encoding (doc), NULL))
            select_file (filesel, filename, destdir);
        g_free (filename);
    }
}

static void
doc_save_copy (MooFileSelector *filesel,
               MooEdit         *doc,
               gboolean         ask_name,
               const char      *destdir)
{
    char *filename;

    filename = save_as_dialog (GTK_WIDGET (filesel), destdir,
                               moo_edit_get_display_basename (doc),
                               ask_name, "Save Copy As");

    if (filename)
    {
        if (moo_edit_save_copy (doc, filename, moo_edit_get_encoding (doc), NULL))
            select_file (filesel, filename, destdir);
        g_free (filename);
    }
}

static void
doc_move (MooFileSelector *filesel,
          MooEdit         *doc,
          gboolean         ask_name,
          const char      *destdir)
{
    char *filename, *old_filename;

    old_filename = g_strdup (moo_edit_get_filename (doc));

    filename = save_as_dialog (GTK_WIDGET (filesel), destdir,
                               moo_edit_get_display_basename (doc),
                               ask_name, "Rename To");

    if (filename)
    {
        if (moo_edit_save_as (doc, filename, moo_edit_get_encoding (doc), NULL))
        {
            m_unlink (old_filename);
            select_file (filesel, filename, destdir);
        }

        g_free (filename);
    }

    g_free (old_filename);
}


static gboolean
really_destroy_menu (GtkWidget *menu)
{
    g_object_unref (menu);
    return FALSE;
}

static void
destroy_menu (GtkWidget *menu)
{
    g_idle_add ((GSourceFunc) really_destroy_menu, menu);
}


typedef enum {
    DROP_DOC_ASK = 1,
    DROP_DOC_SAVE_AS,
    DROP_DOC_SAVE_COPY,
    DROP_DOC_MOVE
} DropDocAction;


static void
drop_item_activated (GObject         *item,
                     MooFileSelector *filesel)
{
    DropDocAction action;
    gpointer data;
    MooEdit *doc;
    char *destdir;
    gboolean alternate;

    data = g_object_get_data (item, "moo-file-selector-drop-action");
    doc = g_object_get_data (item, "moo-file-selector-drop-doc");
    destdir = g_object_get_data (item, "moo-file-selector-drop-destdir");
    alternate = GPOINTER_TO_INT (g_object_get_data (item, "moo-menu-item-alternate"));
    g_return_if_fail (doc != NULL && destdir != NULL);

    action = GPOINTER_TO_INT (data);

    switch (action)
    {
        case DROP_DOC_SAVE_AS:
            doc_save_as (filesel, doc, alternate, destdir);
            break;
        case DROP_DOC_SAVE_COPY:
            doc_save_copy (filesel, doc, alternate, destdir);
            break;
        case DROP_DOC_MOVE:
            doc_move (filesel, doc, alternate, destdir);
            break;
        default:
            g_return_if_reached ();
    }
}


static GtkWidget *
create_menu_item (MooFileSelector *filesel,
                  MooEdit         *doc,
                  const char      *destdir,
                  const char      *stock_icon,
                  const char      *label,
                  const char      *alternate_label,
                  const char      *accel_label,
                  DropDocAction    action)
{
    GtkWidget *item;

    item = gtk_image_menu_item_new_with_label (label);

    if (stock_icon)
    {
        GtkWidget *icon = gtk_image_new_from_stock (stock_icon, GTK_ICON_SIZE_MENU);
        gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (item), icon);
    }

    g_object_set_data_full (G_OBJECT (item), "moo-menu-item-label",
                            g_strdup (label), g_free);
    g_object_set_data_full (G_OBJECT (item), "moo-menu-item-alternate-label",
                            g_strdup (alternate_label), g_free);

    g_object_set_data_full (G_OBJECT (item), "moo-file-selector-drop-doc",
                            g_object_ref (doc), g_object_unref);
    g_object_set_data_full (G_OBJECT (item), "moo-file-selector-drop-destdir",
                            g_strdup (destdir), g_free);
    g_object_set_data (G_OBJECT (item), "moo-file-selector-drop-action",
                       GINT_TO_POINTER (action));

    g_signal_connect (item, "activate", G_CALLBACK (drop_item_activated), filesel);
    gtk_widget_show (item);
    moo_menu_item_set_accel_label (item, accel_label);

    return item;
}

static gboolean
menu_key_event (GtkWidget   *menu,
                GdkEventKey *event)
{
    GdkModifierType mask;
    gboolean alternate;
    GSList *items, *l;

    switch (event->keyval)
    {
        case GDK_Shift_L:
        case GDK_Shift_R:
        case GDK_Control_L:
        case GDK_Control_R:
            break;

        default:
            return FALSE;
    }

    mask = moo_get_modifiers (menu);
    alternate = (mask & (GDK_SHIFT_MASK | GDK_CONTROL_MASK)) != 0;

    items = g_object_get_data (G_OBJECT (menu), "moo-menu-items");

    for (l = items; l != NULL; l = l->next)
    {
        GtkWidget *item = l->data;
        const char *label;

        if (alternate)
            label = g_object_get_data (G_OBJECT (item), "moo-menu-item-alternate-label");
        else
            label = g_object_get_data (G_OBJECT (item), "moo-menu-item-label");

        g_object_set_data (G_OBJECT (item), "moo-menu-item-alternate",
                           GINT_TO_POINTER (alternate));
        moo_menu_item_set_label (item, label, FALSE);
    }

    return FALSE;
}

static GtkWidget *
create_drop_doc_menu (MooFileSelector *filesel,
                      MooEdit         *doc,
                      const char      *destdir)
{
    GtkWidget *menu, *item;
    GSList *items = NULL;

    menu = gtk_menu_new ();
    gtk_object_sink (g_object_ref (menu));
    g_signal_connect (menu, "deactivate", G_CALLBACK (destroy_menu), NULL);
    g_signal_connect (menu, "key-press-event", G_CALLBACK (menu_key_event), NULL);
    g_signal_connect (menu, "key-release-event", G_CALLBACK (menu_key_event), NULL);

    item = create_menu_item (filesel, doc, destdir,
                             MOO_STOCK_FILE_MOVE,
                             "Move Here",
                             "Move/Rename...",
                             "Shift",
                             DROP_DOC_MOVE);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
    items = g_slist_prepend (items, item);

    item = create_menu_item (filesel, doc, destdir,
                             MOO_STOCK_FILE_SAVE_AS,
                             "Save Here",
                             "Save As...",
                             "Control",
                             DROP_DOC_SAVE_AS);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
    items = g_slist_prepend (items, item);

    item = create_menu_item (filesel, doc, destdir,
                             MOO_STOCK_FILE_SAVE_COPY,
                             "Save Copy",
                             "Save Copy As...",
                             "  Control+Shift",
                             DROP_DOC_SAVE_COPY);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
    items = g_slist_prepend (items, item);

    g_object_set_data_full (G_OBJECT (menu), "moo-menu-items",
                            items, (GDestroyNotify) g_slist_free);

    item = gtk_separator_menu_item_new ();
    gtk_widget_show (item);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

    item = gtk_image_menu_item_new_from_stock (GTK_STOCK_CANCEL, NULL);
    gtk_widget_show (item);
    moo_menu_item_set_accel_label (item, "Escape");
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

    return menu;
}


static void
moo_file_selector_drop_doc (MooFileSelector *filesel,
                            MooEdit        *doc,
                            const char     *destdir,
                            GtkWidget      *widget,
                            GdkDragContext *context,
                            int             x,
                            int             y,
                            guint           time)
{
    const char *filename;
    GdkModifierType mods;
    DropDocAction action;

    g_return_if_fail (MOO_IS_EDIT (doc));
    g_return_if_fail (destdir != NULL);
    g_return_if_fail (GTK_IS_WIDGET (widget));

    filename = moo_edit_get_filename (doc);

    if (!filename)
    {
        gboolean result = drop_untitled (filesel, doc, destdir,
                                         widget, context, x, y, time);
        _moo_file_view_drag_finish (MOO_FILE_VIEW (filesel), context, result, FALSE, time);
        return;
    }

    mods = moo_get_modifiers (widget) & (GDK_CONTROL_MASK | GDK_SHIFT_MASK | GDK_MOD1_MASK);

    if (!mods || mods & GDK_MOD1_MASK)
        action = DROP_DOC_ASK;
    else if (mods == (GDK_SHIFT_MASK | GDK_CONTROL_MASK))
        action = DROP_DOC_SAVE_COPY;
    else if (mods & GDK_SHIFT_MASK)
        action = DROP_DOC_MOVE;
    else
        action = DROP_DOC_SAVE_AS;

    if (action == DROP_DOC_ASK)
    {
        GtkWidget *menu = create_drop_doc_menu (filesel, doc, destdir);
        _moo_file_view_drag_finish (MOO_FILE_VIEW (filesel), context, TRUE, FALSE, time);
        gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL, 0, 0);
        return;
    }

    _moo_file_view_drag_finish (MOO_FILE_VIEW (filesel), context, TRUE, FALSE, time);

    switch (action)
    {
        case DROP_DOC_SAVE_AS:
            doc_save_as (filesel, doc, FALSE, destdir);
            break;
        case DROP_DOC_SAVE_COPY:
            doc_save_copy (filesel, doc, FALSE, destdir);
            break;
        case DROP_DOC_MOVE:
            doc_move (filesel, doc, FALSE, destdir);
            break;
        default:
            g_return_if_reached ();
    }
}


/****************************************************************************/
/* Plugin
 */

static void
show_file_selector (MooEditWindow *window)
{
    GtkWidget *pane;
    pane = moo_edit_window_get_pane (window, PLUGIN_ID);
    moo_big_paned_present_pane (window->paned, pane);
}


#if 0
static void
show_file_chooser (MooEditWindow *window)
{
    GtkWidget *dialog;
    int response;

    dialog = g_object_get_data (G_OBJECT (window), "moo-file-chooser");

    if (!dialog)
    {
        dialog = moo_file_chooser_new ();
        gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (window));
        g_object_set_data_full (G_OBJECT (window), "moo-file-chooser",
                                dialog, (GDestroyNotify) gtk_widget_destroy);
    }

    response = gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_hide (dialog);

    if (response == GTK_RESPONSE_OK)
    {
        GSList *files = NULL;
        GList *filenames;

        filenames = _moo_file_view_get_filenames (MOO_FILE_CHOOSER (dialog)->fileview);

        while (filenames)
        {
            MooEditFileInfo *info = moo_edit_file_info_new (filenames->data, NULL);
            files = g_slist_prepend (files, info);
            g_free (filenames->data);
            filenames = g_list_delete_link (filenames, filenames);
        }

        moo_editor_open (moo_edit_window_get_editor (window), window, NULL, files);

        g_slist_foreach (files, (GFunc) moo_edit_file_info_free, NULL);
        g_slist_free (files);
    }
}
#endif


static gboolean
file_selector_plugin_init (Plugin *plugin)
{
    MooWindowClass *klass = g_type_class_ref (MOO_TYPE_EDIT_WINDOW);
    MooEditor *editor = moo_editor_instance ();
    MooUIXML *xml = moo_editor_get_ui_xml (editor);

    g_return_val_if_fail (klass != NULL, FALSE);
    g_return_val_if_fail (editor != NULL, FALSE);

    moo_window_class_new_action (klass, "ShowFileSelector",
                                 "display-name", "Show File Selector",
                                 "label", "Show File Selector",
                                 "tooltip", "Show file selector",
                                 "stock-id", MOO_STOCK_FILE_SELECTOR,
                                 "closure-callback", show_file_selector,
                                 NULL);

    if (xml)
    {
        plugin->ui_merge_id = moo_ui_xml_new_merge_id (xml);
        moo_ui_xml_add_item (xml, plugin->ui_merge_id,
                             "Editor/Menubar/View",
                             "ShowFileSelector",
                             "ShowFileSelector", -1);
    }

    moo_prefs_new_key_string (DIR_PREFS, NULL);

    g_type_class_unref (klass);
    return TRUE;
}


static void
file_selector_plugin_deinit (Plugin *plugin)
{
    MooWindowClass *klass;
    MooEditor *editor = moo_editor_instance ();
    MooUIXML *xml = moo_editor_get_ui_xml (editor);

    if (plugin->bookmark_mgr)
        g_object_unref (plugin->bookmark_mgr);
    plugin->bookmark_mgr = NULL;

    klass = g_type_class_ref (MOO_TYPE_EDIT_WINDOW);
    moo_window_class_remove_action (klass, "ShowFileSelector");

    if (plugin->ui_merge_id)
        moo_ui_xml_remove_ui (xml, plugin->ui_merge_id);
    plugin->ui_merge_id = 0;

    g_type_class_unref (klass);
}


static void
file_selector_plugin_attach (Plugin        *plugin,
                             MooEditWindow *window)
{
    MooEditor *editor;

    editor = moo_edit_window_get_editor (window);

    if (!plugin->bookmark_mgr)
        plugin->bookmark_mgr = _moo_bookmark_mgr_new ();

    /* it attaches itself to window */
    g_object_new (MOO_TYPE_FILE_SELECTOR,
                  "filter-mgr", moo_editor_get_filter_mgr (editor),
                  "bookmark-mgr", plugin->bookmark_mgr,
                  "window", window,
                  NULL);
}


static void
file_selector_plugin_detach (G_GNUC_UNUSED Plugin *plugin,
                             MooEditWindow *window)
{
    moo_edit_window_remove_pane (window, PLUGIN_ID);
}


MOO_PLUGIN_DEFINE_INFO (file_selector, PLUGIN_ID,
                        "File Selector", "Selects files",
                        "Yevgen Muntyan <muntyan@tamu.edu>",
                        MOO_VERSION, NULL);
MOO_PLUGIN_DEFINE_FULL (FileSelector, file_selector,
                        file_selector_plugin_init, file_selector_plugin_deinit,
                        file_selector_plugin_attach, file_selector_plugin_detach,
                        NULL, NULL, NULL, 0, 0);


gboolean
_moo_file_selector_plugin_init (void)
{
    return moo_plugin_register (file_selector_plugin_get_type ());
}
