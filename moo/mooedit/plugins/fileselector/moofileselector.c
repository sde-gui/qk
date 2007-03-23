/*
 *   fileselector.c
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
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

#include "moofileselector.h"
#include "mooedit/mooplugin-macro.h"
#include "moofileview/moobookmarkmgr.h"
#include "moofileview/moofileview-tools.h"
#include "mooedit/plugins/mooeditplugins.h"
#include "mooedit/plugins/fileselector/moofileselector-glade.h"
#include "mooutils/moostock.h"
#include "mooutils/mooutils-fs.h"
#include "mooutils/mooutils-misc.h"
#include "mooutils/mooglade.h"
#include "mooutils/mooentry.h"
#include "mooutils/moodialogs.h"
#include "mooutils/mooactionfactory.h"
#include "mooutils/moomarshals.h"
#include "mooutils/mooi18n.h"
#include <gmodule.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <glib/gstdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#ifdef MOO_USE_XDGMIME
#include <mooutils/xdgmime/xdgmime.h>
#endif

#ifndef MOO_VERSION
#define MOO_VERSION NULL
#endif

#define DIR_PREFS MOO_PLUGIN_PREFS_ROOT "/" MOO_FILE_SELECTOR_PLUGIN_ID "/last_dir"


typedef struct {
    MooPlugin parent;
    MooBookmarkMgr *bookmark_mgr;
    guint ui_merge_id;
    GSList *instances;
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

G_DEFINE_TYPE (MooFileSelector, _moo_file_selector, MOO_TYPE_FILE_VIEW)


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

static void     moo_file_selector_select_file   (MooFileSelector *filesel,
                                                 const char     *filename,
                                                 const char     *dirname);
static gboolean moo_file_selector_chdir         (MooFileView    *fileview,
                                                 const char     *dir,
                                                 GError        **error);
static void     moo_file_selector_activate      (MooFileView    *fileview,
                                                 const char     *path);
static void     moo_file_selector_populate_popup(MooFileView    *fileview,
                                                 GList          *selected,
                                                 GtkMenu        *menu);

static void     goto_current_doc_dir            (MooFileSelector *filesel);

static gboolean moo_file_selector_drop          (MooFileView    *fileview,
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

static void     moo_file_selector_drop_doc      (MooFileSelector *filesel,
                                                 MooEdit        *doc,
                                                 const char     *destdir,
                                                 GtkWidget      *widget,
                                                 GdkDragContext *context,
                                                 int             x,
                                                 int             y,
                                                 guint           time);

static void     button_drag_leave               (GtkWidget      *button,
                                                 GdkDragContext *context,
                                                 guint           time,
                                                 MooFileSelector *filesel);
static gboolean button_drag_motion              (GtkWidget      *button,
                                                 GdkDragContext *context,
                                                 int             x,
                                                 int             y,
                                                 guint           time,
                                                 MooFileSelector *filesel);


static void
_moo_file_selector_class_init (MooFileSelectorClass *klass)
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
    fileview_class->populate_popup = moo_file_selector_populate_popup;

    g_object_class_install_property (gobject_class,
                                     PROP_WINDOW,
                                     g_param_spec_object ("window",
                                             "window",
                                             "window",
                                             MOO_TYPE_EDIT_WINDOW,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    _moo_signal_new_cb ("goto-current-doc-dir",
                        G_OBJECT_CLASS_TYPE (klass),
                        G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                        G_CALLBACK (goto_current_doc_dir),
                        NULL, NULL,
                        _moo_marshal_VOID__VOID,
                        G_TYPE_NONE, 0);
}


static void
_moo_file_selector_init (MooFileSelector *filesel)
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
    G_OBJECT_CLASS(_moo_file_selector_parent_class)->finalize (object);
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


static void
file_selector_go_home (MooFileView *fileview)
{
    const char *dir;
    char *real_dir = NULL;

    dir = moo_prefs_get_string (DIR_PREFS);

    if (dir)
        real_dir = g_filename_from_utf8 (dir, -1, NULL, NULL, NULL);

    if (!real_dir || !moo_file_view_chdir (fileview, real_dir, NULL))
        g_signal_emit_by_name (fileview, "go-home");

    g_free (real_dir);
}


static gboolean
moo_file_selector_chdir (MooFileView    *fileview,
                         const char     *dir,
                         GError        **error)
{
    gboolean result;

    result = MOO_FILE_VIEW_CLASS(_moo_file_selector_parent_class)->chdir (fileview, dir, error);

    if (result)
    {
        char *new_dir = NULL;
        g_object_get (fileview, "current-directory", &new_dir, NULL);
        moo_prefs_set_filename (DIR_PREFS, new_dir);
        g_free (new_dir);
    }

    return result;
}


static void
moo_file_selector_activate (MooFileView    *fileview,
                            const char     *path)
{
    struct stat statbuf;
    MooFileSelector *filesel = MOO_FILE_SELECTOR (fileview);
    gboolean is_text = TRUE, is_exe = FALSE;

    g_return_if_fail (path != NULL);

    errno = 0;

    if (g_stat (path, &statbuf) != 0)
    {
        int err = errno;
        g_warning ("%s: error in stat(%s): %s", G_STRLOC, path, g_strerror (err));
        return;
    }

#ifdef MOO_USE_XDGMIME
    {
        const char *mime_type = xdg_mime_get_mime_type_for_file (path, &statbuf);
        is_text = !strcmp (mime_type, "application/octet-stream") ||
                   xdg_mime_mime_type_subclass (mime_type, "text/plain");
        is_exe = !strcmp (mime_type, "application/x-executable");
    }
#endif

    if (is_text)
        moo_editor_open_file (moo_edit_window_get_editor (filesel->window),
                              filesel->window, GTK_WIDGET (filesel), path, NULL);
    else if (!is_exe)
        _moo_open_file (path);
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
        moo_file_selector_select_file (filesel, filename, dirname);
        g_free (dirname);
    }
}


/****************************************************************************/
/* NewFile
 */

static void
moo_file_selector_populate_popup (MooFileView *fileview,
                                  GList       *selected,
                                  G_GNUC_UNUSED GtkMenu *menu)
{
    GtkAction *new_file, *open;

    new_file = moo_action_collection_get_action (moo_file_view_get_actions (fileview), "NewFile");
    open = moo_action_collection_get_action (moo_file_view_get_actions (fileview), "Open");

    if (new_file)
        gtk_action_set_sensitive (new_file, !selected || !selected->next);

    if (open)
        gtk_action_set_visible (open, selected && selected->next);
}


static GtkWidget *
create_new_file_dialog (GtkWidget    *parent,
                        const char   *dirname,
                        const char   *start_text,
                        MooGladeXML **xml)
{
    GtkWidget *dialog, *button;
    GtkEntry *entry;
    GtkLabel *label;
    char *display_dirname, *label_text;

    display_dirname = g_filename_display_basename (dirname);
    g_return_val_if_fail (display_dirname != NULL, NULL);

    *xml = moo_glade_xml_new_empty (GETTEXT_PACKAGE);
    moo_glade_xml_map_class (*xml, "GtkEntry", MOO_TYPE_ENTRY);
    moo_glade_xml_parse_memory (*xml, MOO_FILE_SELECTOR_GLADE_XML, -1,
                                "new_file_dialog", NULL);

    dialog = moo_glade_xml_get_widget (*xml, "new_file_dialog");
    g_return_val_if_fail (dialog != NULL, NULL);

    moo_window_set_parent (dialog, parent);

    entry = moo_glade_xml_get_widget (*xml, "entry");
    gtk_entry_set_text (entry, start_text);
    moo_entry_clear_undo (MOO_ENTRY (entry));

    label = moo_glade_xml_get_widget (*xml, "label");
    label_text = g_strdup_printf (_("Create file in folder '%s':"), display_dirname);
    gtk_label_set_text (label, label_text);

    gtk_widget_show_all (dialog);
    gtk_widget_grab_focus (GTK_WIDGET (entry));

    button = moo_glade_xml_get_widget (*xml, "ok_button");
    moo_bind_bool_property (button, "sensitive", entry, "empty", TRUE);

    g_free (label_text);
    g_free (display_dirname);
    return dialog;
}

static char *
new_file_dialog (GtkWidget   *parent,
                 const char  *dirname,
                 const char  *start_name)
{
    MooGladeXML *xml = NULL;
    GtkWidget *dialog = NULL;
    GtkEntry *entry = NULL;
    char *fullname = NULL;

    g_return_val_if_fail (dirname != NULL, NULL);

    while (TRUE)
    {
        const char *text;
        char *name;
        char *err_text;

        if (!dialog)
        {
            dialog = create_new_file_dialog (parent, dirname, start_name, &xml);
            g_return_val_if_fail (dialog != NULL, NULL);
            entry = moo_glade_xml_get_widget (xml, "entry");
        }

        if (gtk_dialog_run (GTK_DIALOG (dialog)) != GTK_RESPONSE_OK)
            goto out;

        text = gtk_entry_get_text (entry);

        if (!text[0])
        {
            g_critical ("%s: ooops", G_STRLOC);
            goto out;
        }

        /* XXX error checking, you know */
        name = g_filename_from_utf8 (text, -1, NULL, NULL, NULL);

        if (!name)
        {
            char *sec_text;
            err_text = g_strdup_printf (_("Could not create file '%s'"), text);
            sec_text = g_strdup_printf (_("Could not convert '%s' to filename encoding. "
                                          "Please consider simpler name, such as foo.blah "
                                          "or blah.foo"), text);
            moo_error_dialog (dialog ? dialog : parent, err_text, sec_text);
            g_free (err_text);
            g_free (sec_text);
            continue;
        }

        fullname = g_build_filename (dirname, name, NULL);
        g_free (name);

        if (!g_file_test (fullname, G_FILE_TEST_EXISTS))
            goto out;

        err_text = g_strdup_printf (_("File '%s' already exists"), text);
        moo_error_dialog (dialog, err_text, NULL);
        g_free (err_text);

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
file_selector_create_file (MooFileSelector *filesel)
{
    char *path = NULL, *dir = NULL;
    MooEdit *doc;
    GList *selected = NULL;

    selected = _moo_file_view_get_filenames (MOO_FILE_VIEW (filesel));

    if (selected)
    {
        if (selected->next)
        {
            g_critical ("%s: oops", G_STRLOC);
            goto out;
        }

        dir = selected->data;
        g_list_free (selected);
        selected = NULL;

        if (!g_file_test (dir, G_FILE_TEST_IS_DIR))
        {
            g_free (dir);
            dir = NULL;
        }
    }

    if (!dir)
    {
        g_object_get (filesel, "current-directory", &dir, NULL);

        if (!dir)
            goto out;
    }

    path = new_file_dialog (GTK_WIDGET (filesel), dir, _("Untitled"));

    if (!path)
        goto out;

    doc = moo_editor_new_file (moo_edit_window_get_editor (filesel->window),
                               filesel->window, GTK_WIDGET (filesel), path, NULL);

    if (doc)
        moo_edit_save (doc, NULL);

out:
    g_free (path);
    g_free (dir);
    g_list_foreach (selected, (GFunc) g_free, NULL);
    g_list_free (selected);
}


static void
file_selector_open_files (MooFileSelector *filesel)
{
    GList *selected = NULL;
    GList *files = NULL;

    selected = _moo_file_view_get_filenames (MOO_FILE_VIEW (filesel));

    if (!selected)
    {
        g_critical ("%s: oops", G_STRLOC);
        return;
    }

    while (selected)
    {
        char *filename = selected->data;

        if (g_file_test (filename, G_FILE_TEST_IS_REGULAR))
            files = g_list_prepend (files, filename);
        else
            g_free (filename);

        selected = g_list_delete_link (selected, selected);
    }

    files = g_list_reverse (files);

    while (files)
    {
        moo_editor_open_file (moo_edit_window_get_editor (filesel->window),
                              filesel->window, GTK_WIDGET (filesel),
                              files->data, NULL);
        g_free (files->data);
        files = g_list_delete_link (files, files);
    }
}


/****************************************************************************/
/* Constructor
 */

static GObject *
moo_file_selector_constructor (GType           type,
                               guint           n_props,
                               GObjectConstructParam *props)
{
    MooPaneLabel *label;
    MooUIXML *xml;
    MooFileSelector *filesel;
    MooFileView *fileview;
    GObject *object;
    guint merge_id;
    GtkActionGroup *group;

    object = G_OBJECT_CLASS(_moo_file_selector_parent_class)->constructor (type, n_props, props);
    filesel = MOO_FILE_SELECTOR (object);
    fileview = MOO_FILE_VIEW (object);

    g_return_val_if_fail (filesel->window != NULL, object);

    file_selector_go_home (MOO_FILE_VIEW (fileview));

    group = moo_action_collection_get_group (moo_file_view_get_actions (MOO_FILE_VIEW (fileview)), NULL);
    xml = moo_file_view_get_ui_xml (MOO_FILE_VIEW (fileview));
    merge_id = moo_ui_xml_new_merge_id (xml);

    moo_action_group_add_action (group, "GoToCurrentDocDir",
                                 "stock-id", GTK_STOCK_JUMP_TO,
                                 "tooltip", _("Go to current document directory"),
                                 "closure-object", fileview,
                                 "closure-signal", "goto-current-doc-dir",
                                 NULL);
    moo_ui_xml_insert_markup (xml, merge_id,
                              "MooFileView/Toolbar", -1,
                              "<item action=\"GoToCurrentDocDir\"/>");
    _moo_file_view_setup_button_drag_dest (MOO_FILE_VIEW (filesel),
                                           "MooFileView/Toolbar/GoToCurrentDocDir",
                                           "goto-current-doc-dir");

    moo_action_group_add_action (group, "NewFile",
                                 "label", _("New File..."),
                                 "tooltip", _("New File..."),
                                 "stock-id", GTK_STOCK_NEW,
                                 "closure-object", filesel,
                                 "closure-callback", file_selector_create_file,
                                 NULL);
    moo_action_group_add_action (group, "Open",
                                 "label", GTK_STOCK_OPEN,
                                 "tooltip", GTK_STOCK_OPEN,
                                 "stock-id", GTK_STOCK_OPEN,
                                 "closure-object", filesel,
                                 "closure-callback", file_selector_open_files,
                                 NULL);
    moo_ui_xml_insert_markup_before (xml, merge_id,
                                     "MooFileView/Menu",
                                     "NewFolder",
                                     "<item action=\"NewFile\"/>"
                                     "<separator/>");
    moo_ui_xml_insert_markup (xml, merge_id,
                              "MooFileView/Menu", 0,
                              "<item action=\"Open\"/>");

    label = moo_pane_label_new (MOO_STOCK_FILE_SELECTOR,
                                NULL, NULL/*button*/, _("File Selector"),
                                _("File Selector"));
    moo_edit_window_add_pane (filesel->window, MOO_FILE_SELECTOR_PLUGIN_ID,
                              GTK_WIDGET (filesel), label, MOO_PANE_POS_RIGHT);
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
        filesel->open_pane_timeout = _moo_timeout_add (OPEN_PANE_TIMEOUT,
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
        return MOO_FILE_VIEW_CLASS(_moo_file_selector_parent_class)->
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

    doc = _moo_selection_data_get_pointer (data, moo_edit_tab_atom);

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
    return MOO_FILE_VIEW_CLASS(_moo_file_selector_parent_class)->
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

    *xml = moo_glade_xml_new_empty (GETTEXT_PACKAGE);
    moo_glade_xml_map_class (*xml, "GtkEntry", MOO_TYPE_ENTRY);
    moo_glade_xml_parse_memory (*xml, MOO_FILE_SELECTOR_GLADE_XML, -1,
                                "save_untitled_dialog", NULL);

    dialog = moo_glade_xml_get_widget (*xml, "save_untitled_dialog");
    g_return_val_if_fail (dialog != NULL, NULL);

    gtk_window_set_title (GTK_WINDOW (dialog), title);

    moo_window_set_parent (dialog, parent);

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
            err_text = g_strdup_printf (_("Could not save file as '%s'"), text);
            sec_text = g_strdup_printf (_("Could not convert '%s' to filename encoding. "
                                          "Please consider simpler name, such as foo.blah "
                                          "or blah.foo"), text);
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
moo_file_selector_select_file (MooFileSelector *filesel,
                               const char      *filename,
                               const char      *dirname)
{
    char *freeme = NULL;
    char *curdir = NULL;

    g_return_if_fail (filename != NULL);
    g_return_if_fail (dirname != NULL);

    g_object_get (filesel, "current-directory", &curdir, NULL);

    /* XXX */
    if (dirname[0] && dirname[strlen(dirname) - 1] != '/')
    {
        freeme = g_strdup_printf ("%s/", dirname);
        dirname = freeme;
    }

    if (curdir && !strcmp (curdir, dirname))
    {
        char *basename = g_path_get_basename (filename);
        _moo_file_view_select_name (MOO_FILE_VIEW (filesel), basename);
        g_free (basename);
    }

    g_free (curdir);
    g_free (freeme);
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
                           TRUE, _("Save As"));

    if (!name)
        return FALSE;

    result = moo_edit_save_as (doc, name, NULL, NULL);

    if (result)
        moo_file_selector_select_file (filesel, name, destdir);

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
                               ask_name, _("Save As"));

    if (filename)
    {
        if (moo_edit_save_as (doc, filename, moo_edit_get_encoding (doc), NULL))
            moo_file_selector_select_file (filesel, filename, destdir);
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
                               ask_name, _("Save Copy As"));

    if (filename)
    {
        if (moo_edit_save_copy (doc, filename, moo_edit_get_encoding (doc), NULL))
            moo_file_selector_select_file (filesel, filename, destdir);
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
                               ask_name, _("Rename To"));

    if (filename)
    {
        if (moo_edit_save_as (doc, filename, moo_edit_get_encoding (doc), NULL))
        {
            _moo_unlink (old_filename);
            moo_file_selector_select_file (filesel, filename, destdir);
        }

        g_free (filename);
    }

    g_free (old_filename);
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
    _moo_menu_item_set_accel_label (item, accel_label);

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

    mask = _moo_get_modifiers (menu);
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
        _moo_menu_item_set_label (item, label, FALSE);
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
    gtk_menu_attach_to_widget (GTK_MENU (menu), GTK_WIDGET (filesel), NULL);
    g_signal_connect (menu, "key-press-event", G_CALLBACK (menu_key_event), NULL);
    g_signal_connect (menu, "key-release-event", G_CALLBACK (menu_key_event), NULL);

    item = create_menu_item (filesel, doc, destdir,
                             MOO_STOCK_FILE_MOVE,
                             _("Move Here"),
                             _("Move/Rename..."),
                             "Shift",
                             DROP_DOC_MOVE);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
    items = g_slist_prepend (items, item);

    item = create_menu_item (filesel, doc, destdir,
                             MOO_STOCK_FILE_SAVE_AS,
                             _("Save Here"),
                             _("Save As..."),
                             "Control",
                             DROP_DOC_SAVE_AS);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
    items = g_slist_prepend (items, item);

    item = create_menu_item (filesel, doc, destdir,
                             MOO_STOCK_FILE_SAVE_COPY,
                             _("Save Copy"),
                             _("Save Copy As..."),
                             "Control+Shift",
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
    _moo_menu_item_set_accel_label (item, "Escape");
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

    mods = _moo_get_modifiers (widget) & (GDK_CONTROL_MASK | GDK_SHIFT_MASK | GDK_MOD1_MASK);

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
    pane = moo_edit_window_get_pane (window, MOO_FILE_SELECTOR_PLUGIN_ID);
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

    moo_window_class_new_action (klass, "ShowFileSelector", NULL,
                                 "display-name", _("Show File Selector"),
                                 "label", _("Show File Selector"),
                                 "tooltip", _("Show file selector"),
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

    moo_prefs_create_key (DIR_PREFS, MOO_PREFS_STATE, G_TYPE_STRING, NULL);

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
    GtkWidget *filesel;

    editor = moo_edit_window_get_editor (window);

    if (!plugin->bookmark_mgr)
        plugin->bookmark_mgr = _moo_bookmark_mgr_new ();

    /* it attaches itself to window */
    filesel = g_object_new (MOO_TYPE_FILE_SELECTOR,
                            "filter-mgr", moo_editor_get_filter_mgr (editor),
                            "bookmark-mgr", plugin->bookmark_mgr,
                            "window", window,
                            NULL);

    plugin->instances = g_slist_prepend (plugin->instances, filesel);
}


static void
file_selector_plugin_detach (Plugin        *plugin,
                             MooEditWindow *window)
{
    GtkWidget *filesel = moo_edit_window_get_pane (window, MOO_FILE_SELECTOR_PLUGIN_ID);
    g_return_if_fail (filesel != NULL);
    plugin->instances = g_slist_remove (plugin->instances, filesel);
    moo_edit_window_remove_pane (window, MOO_FILE_SELECTOR_PLUGIN_ID);
}


void
_moo_file_selector_update_tools (MooPlugin *plugin)
{
    GSList *l;
    for (l = ((Plugin*)plugin)->instances; l != NULL; l = l->next)
        _moo_file_view_tools_load (l->data);
}


MOO_PLUGIN_DEFINE_INFO (file_selector,
                        N_("File Selector"),
                        N_("File selector pane for editor window"),
                        "Yevgen Muntyan <muntyan@tamu.edu>",
                        MOO_VERSION, NULL)
MOO_PLUGIN_DEFINE_FULL (FileSelector, file_selector,
                        file_selector_plugin_init, file_selector_plugin_deinit,
                        file_selector_plugin_attach, file_selector_plugin_detach,
                        NULL, NULL,
                        _moo_file_selector_prefs_page,
                        0, 0)


static gpointer
get_widget_meth (G_GNUC_UNUSED gpointer plugin,
                 MooEditWindow *window)
{
    gpointer widget = moo_edit_window_get_pane (window, MOO_FILE_SELECTOR_PLUGIN_ID);
    return widget ? g_object_ref (widget) : NULL;
}


gboolean
_moo_file_selector_plugin_init (void)
{
    GType ptype = file_selector_plugin_get_type ();

    if (!moo_plugin_register (MOO_FILE_SELECTOR_PLUGIN_ID,
                              ptype,
                              &file_selector_plugin_info,
                              NULL))
        return FALSE;

    moo_plugin_method_new ("get-widget", ptype,
                           G_CALLBACK (get_widget_meth),
                           _moo_marshal_OBJECT__OBJECT,
                           _moo_file_selector_get_type (),
                           1, MOO_TYPE_EDIT_WINDOW);

    return TRUE;
}
