/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *   fileselector.c
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
#include "config.h"
#endif

#include <gmodule.h>
#include <gtk/gtkbutton.h>
#include "mooedit/mooplugin-macro.h"
#include "mooutils/moofileview/moofileview.h"
#include "mooutils/moofileview/moobookmarkmgr.h"
#include "mooedit/plugins/mooeditplugins.h"
#include "mooutils/moostock.h"

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


#define MOO_TYPE_FILE_SELECTOR              (moo_file_selector_get_type ())
#define MOO_FILE_SELECTOR(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_FILE_SELECTOR, MooFileSelector))
#define MOO_FILE_SELECTOR_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_FILE_SELECTOR, MooFileSelectorClass))
#define MOO_IS_FILE_SELECTOR(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_FILE_SELECTOR))
#define MOO_IS_FILE_SELECTOR_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_FILE_SELECTOR))
#define MOO_FILE_SELECTOR_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_FILE_SELECTOR, MooFileSelectorClass))

typedef struct {
    MooFileView base;
    MooEditWindow *window;
    GtkWidget *button;
} MooFileSelector;

typedef struct {
    MooFileViewClass base_class;
} MooFileSelectorClass;


GType moo_file_selector_get_type (void) G_GNUC_CONST;
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

static gboolean moo_file_selector_chdir         (MooFileView    *fileview,
                                                 const char     *dir,
                                                 GError        **error);
static void     moo_file_selector_activate      (MooFileView    *fileview,
                                                 const char     *path);


static void
moo_file_selector_class_init (MooFileSelectorClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    MooFileViewClass *fileview_class = MOO_FILE_VIEW_CLASS (klass);

    gobject_class->set_property = moo_file_selector_set_property;
    gobject_class->get_property = moo_file_selector_get_property;
    gobject_class->constructor = moo_file_selector_constructor;

    fileview_class->chdir = moo_file_selector_chdir;
    fileview_class->activate = moo_file_selector_activate;

    g_object_class_install_property (gobject_class,
                                     PROP_WINDOW,
                                     g_param_spec_object ("window",
                                             "window",
                                             "window",
                                             MOO_TYPE_EDIT_WINDOW,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
}


static void
moo_file_selector_init (G_GNUC_UNUSED MooFileSelector *sel)
{
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

    if (!real_dir || !moo_file_view_chdir (fileview, real_dir, NULL))
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
        moo_file_view_chdir (MOO_FILE_VIEW (filesel), dirname, NULL);
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

    moo_action_group_add_action (moo_file_view_get_actions (MOO_FILE_VIEW (fileview)),
                                 "id", "GoToCurrentDocDir",
                                 "icon-stock-id", GTK_STOCK_JUMP_TO,
                                 "tooltip", "Go to current document directory",
                                 "closure-object", filesel,
                                 "closure-callback", goto_current_doc_dir,
                                 NULL);

    xml = moo_file_view_get_ui_xml (MOO_FILE_VIEW (fileview));
    moo_ui_xml_insert_markup (xml, moo_ui_xml_new_merge_id (xml),
                              "MooFileView/Toolbar", -1,
                              "<item action=\"GoToCurrentDocDir\"/>");

    label = moo_pane_label_new (MOO_STOCK_FILE_SELECTOR,
                                NULL, NULL, "File Selector",
                                "File Selector");
    moo_edit_window_add_pane (filesel->window, PLUGIN_ID, GTK_WIDGET (filesel),
                              label, MOO_PANE_POS_RIGHT);
    moo_pane_label_free (label);

    return object;
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


static gboolean
file_selector_plugin_init (Plugin *plugin)
{
    MooWindowClass *klass = g_type_class_ref (MOO_TYPE_EDIT_WINDOW);
    MooEditor *editor = moo_editor_instance ();
    MooUIXML *xml = moo_editor_get_ui_xml (editor);

    g_return_val_if_fail (klass != NULL, FALSE);
    g_return_val_if_fail (editor != NULL, FALSE);

    moo_window_class_new_action (klass, "ShowFileSelector",
                                 "name", "Show File Selector",
                                 "label", "Show File Selector",
                                 "tooltip", "Show file selector",
                                 "icon-stock-id", MOO_STOCK_FILE_SELECTOR,
                                 "closure-callback", show_file_selector,
                                 NULL);

    plugin->ui_merge_id = moo_ui_xml_new_merge_id (xml);

    moo_ui_xml_add_item (xml, plugin->ui_merge_id,
                         "Editor/Menubar/View",
                         "ShowFileSelector",
                         "ShowFileSelector", -1);

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
        plugin->bookmark_mgr = moo_bookmark_mgr_new ();

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
                        MOO_VERSION);
MOO_PLUGIN_DEFINE_FULL (FileSelector, file_selector,
                        file_selector_plugin_init, file_selector_plugin_deinit,
                        file_selector_plugin_attach, file_selector_plugin_detach,
                        NULL, NULL, NULL, 0, 0);


gboolean
moo_file_selector_plugin_init (void)
{
    return moo_plugin_register (file_selector_plugin_get_type ());
}
