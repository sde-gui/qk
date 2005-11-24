/*
 *   mooeditwindow.c
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
#include "mooedit-private.h"
#include "mooedit/mooeditor.h"
#include "mooedit/mootextbuffer.h"
#include "mooedit/mooeditprefs.h"
#include "mooedit/mooplugin.h"
#include "mooutils/moonotebook.h"
#include "mooutils/moofileview/moofileview.h"
#include "mooutils/moofileview/moobookmarkmgr.h"
#include "mooutils/moostock.h"
#include "mooutils/moomarshals.h"
#include "mooutils/moomenuaction.h"
#include <string.h>
#include <gtk/gtk.h>


#define ACTIVE_DOC moo_edit_window_get_active_doc
#define ACTIVE_PAGE(window) (moo_notebook_get_current_page (window->priv->notebook))

#define LANG_ACTION_ID "LanguageMenu"
#define STOP_ACTION_ID "StopJob"

enum {
    TARGET_TEXT = 1,
    TARGET_URI = 2
};

struct _MooEditWindowPrivate {
    MooEditor *editor;
    GtkStatusbar *statusbar;
    guint statusbar_context_id;
    MooNotebook *notebook;
    char *prefix;
    gboolean use_fullname;
    GHashTable *panes;
    GHashTable *panes_to_save; /* char* */
    guint save_params_idle;

    GSList *stop_clients;
    GSList *jobs; /* Job* */
};


GObject        *moo_edit_window_constructor (GType                  type,
                                             guint                  n_props,
                                             GObjectConstructParam *props);
static void     moo_edit_window_finalize    (GObject        *object);
static void     moo_edit_window_destroy     (GtkObject      *object);

static void     moo_edit_window_set_property(GObject        *object,
                                             guint           prop_id,
                                             const GValue   *value,
                                             GParamSpec     *pspec);
static void     moo_edit_window_get_property(GObject        *object,
                                             guint           prop_id,
                                             GValue         *value,
                                             GParamSpec     *pspec);


static gboolean moo_edit_window_close       (MooEditWindow  *window);

static void     drag_data_received      (GtkWidget          *widget,
                                         GdkDragContext     *context,
                                         int                 x,
                                         int                 y,
                                         GtkSelectionData   *data,
                                         guint               info,
                                         guint               time);

static void     setup_notebook          (MooEditWindow      *window);
static void     update_window_title     (MooEditWindow      *window);

static void     notebook_switch_page    (MooNotebook        *notebook,
                                         gpointer            whatever,
                                         guint               page_num,
                                         MooEditWindow      *window);
static gboolean notebook_populate_popup (MooNotebook        *notebook,
                                         GtkWidget          *child,
                                         GtkMenu            *menu,
                                         MooEditWindow      *window);

static void     proxy_boolean_property  (MooEditWindow      *window,
                                         GParamSpec         *prop,
                                         MooEdit            *doc);
static void     edit_changed            (MooEditWindow      *window,
                                         MooEdit            *doc);
static void     edit_filename_changed   (MooEditWindow      *window,
                                         const char         *filename,
                                         MooEdit            *doc);
static void     edit_lang_changed       (MooEditWindow      *window,
                                         GParamSpec         *pspec,
                                         MooEdit            *doc);
static GtkWidget *create_tab_label      (MooEdit            *edit);
static void     update_tab_label        (MooEditWindow      *window,
                                         MooEdit            *doc);
static void     edit_cursor_moved       (MooEditWindow      *window,
                                         GtkTextIter        *iter,
                                         MooEdit            *edit);
static void     update_lang_menu        (MooEditWindow      *window);

static void     update_statusbar        (MooEditWindow      *window);
static MooEdit *get_nth_tab             (MooEditWindow      *window,
                                         guint               n);
static int      get_page_num            (MooEditWindow      *window,
                                         MooEdit            *doc);
static void     apply_styles            (MooEditWindow      *window,
                                         MooEdit            *edit);

static MooAction *create_lang_action    (MooEditWindow      *window);

static void     create_paned            (MooEditWindow      *window);
static MooPaneParams *load_pane_params  (const char         *pane_id);
static gboolean save_pane_params        (const char         *pane_id,
                                         MooPaneParams      *params);
static void     pane_params_changed     (MooEditWindow      *window,
                                         MooPanePosition     position,
                                         guint               index);
static void     pane_size_changed       (MooEditWindow      *window,
                                         MooPanePosition     position);


/* actions */
static void moo_edit_window_new         (MooEditWindow      *window);
static void moo_edit_window_new_tab     (MooEditWindow      *window);
static void moo_edit_window_open        (MooEditWindow      *window);
static void moo_edit_window_reload      (MooEditWindow      *window);
static void moo_edit_window_save        (MooEditWindow      *window);
static void moo_edit_window_save_as     (MooEditWindow      *window);
static void moo_edit_window_close_tab   (MooEditWindow      *window);
static void moo_edit_window_close_all   (MooEditWindow      *window);
static void moo_edit_window_previous_tab(MooEditWindow      *window);
static void moo_edit_window_next_tab    (MooEditWindow      *window);


/* MOO_TYPE_EDIT_WINDOW */
G_DEFINE_TYPE (MooEditWindow, moo_edit_window, MOO_TYPE_WINDOW)

enum {
    PROP_0,
    PROP_EDITOR,
    PROP_ACTIVE_DOC,

    /* aux properties */
    PROP_CAN_RELOAD,
    PROP_HAS_OPEN_DOCUMENT,
    PROP_CAN_UNDO,
    PROP_CAN_REDO,
    PROP_HAS_SELECTION,
    PROP_HAS_TEXT,
    PROP_HAS_JOBS_RUNNING,
    PROP_HAS_STOP_CLIENTS
};

enum {
    NEW_DOC,
    CLOSE_DOC,
    CLOSE_DOC_AFTER,
    NUM_SIGNALS
};

static guint signals[NUM_SIGNALS];


#define INSTALL_PROP(prop_id,name)                                          \
    g_object_class_install_property (gobject_class, prop_id,                \
        g_param_spec_boolean (name, name, name, FALSE, G_PARAM_READABLE))


static void moo_edit_window_class_init (MooEditWindowClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GtkObjectClass *gtkobject_class = GTK_OBJECT_CLASS (klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
    MooWindowClass *window_class = MOO_WINDOW_CLASS (klass);

    gobject_class->constructor = moo_edit_window_constructor;
    gobject_class->finalize = moo_edit_window_finalize;
    gobject_class->set_property = moo_edit_window_set_property;
    gobject_class->get_property = moo_edit_window_get_property;
    gtkobject_class->destroy = moo_edit_window_destroy;
    widget_class->drag_data_received = drag_data_received;
    window_class->close = (gboolean (*) (MooWindow*))moo_edit_window_close;

    g_object_class_install_property (gobject_class,
                                     PROP_EDITOR,
                                     g_param_spec_object ("editor",
                                             "editor",
                                             "editor",
                                             MOO_TYPE_EDITOR,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

    g_object_class_install_property (gobject_class,
                                     PROP_ACTIVE_DOC,
                                     g_param_spec_object ("active-doc",
                                             "active-doc",
                                             "active-doc",
                                             MOO_TYPE_EDIT,
                                             G_PARAM_READWRITE));

    signals[NEW_DOC] =
            g_signal_new ("new-doc",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooEditWindowClass, new_doc),
                          NULL, NULL,
                          _moo_marshal_VOID__OBJECT,
                          G_TYPE_NONE, 1,
                          MOO_TYPE_EDIT);

    signals[CLOSE_DOC] =
            g_signal_new ("close-doc",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooEditWindowClass, close_doc),
                          NULL, NULL,
                          _moo_marshal_VOID__OBJECT,
                          G_TYPE_NONE, 1,
                          MOO_TYPE_EDIT);

    signals[CLOSE_DOC_AFTER] =
            g_signal_new ("close-doc-after",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooEditWindowClass, close_doc_after),
                          NULL, NULL,
                          _moo_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);

    INSTALL_PROP (PROP_CAN_RELOAD, "can-reload");
    INSTALL_PROP (PROP_HAS_OPEN_DOCUMENT, "has-open-document");
    INSTALL_PROP (PROP_CAN_UNDO, "can-undo");
    INSTALL_PROP (PROP_CAN_REDO, "can-redo");
    INSTALL_PROP (PROP_HAS_SELECTION, "has-selection");
    INSTALL_PROP (PROP_HAS_TEXT, "has-text");
    INSTALL_PROP (PROP_HAS_JOBS_RUNNING, "has-jobs-running");
    INSTALL_PROP (PROP_HAS_STOP_CLIENTS, "has-stop-clients");

    moo_window_class_set_id (window_class, "Editor", "Editor");

    moo_window_class_new_action (window_class, "NewWindow",
                                 "name", "New Window",
                                 "label", "_New Window",
                                 "tooltip", "Open new editor window",
                                 "icon-stock-id", GTK_STOCK_NEW,
                                 "accel", "<ctrl>N",
                                 "closure-callback", moo_edit_window_new,
                                 NULL);

    moo_window_class_new_action (window_class, "NewTab",
                                 "name", "New Tab",
                                 "label", "New _Tab",
                                 "tooltip", "Create new document tab",
                                 "icon-stock-id", GTK_STOCK_NEW,
                                 "accel", "<ctrl>T",
                                 "closure-callback", moo_edit_window_new_tab,
                                 NULL);

    moo_window_class_new_action (window_class, "Open",
                                 "name", "Open",
                                 "label", "_Open...",
                                 "tooltip", "Open...",
                                 "icon-stock-id", GTK_STOCK_OPEN,
                                 "accel", "<ctrl>O",
                                 "closure-callback", moo_edit_window_open,
                                 NULL);

    moo_window_class_new_action (window_class, "Reload",
                                 "name", "Reload",
                                 "label", "_Reload",
                                 "tooltip", "Reload document",
                                 "icon-stock-id", GTK_STOCK_REFRESH,
                                 "accel", "F5",
                                 "closure-callback", moo_edit_window_reload,
                                 "condition::sensitive", "can-reload",
                                 NULL);

    moo_window_class_new_action (window_class, "Save",
                                 "name", "Save",
                                 "label", "_Save",
                                 "tooltip", "Save document",
                                 "icon-stock-id", GTK_STOCK_SAVE,
                                 "accel", "<ctrl>S",
                                 "closure-callback", moo_edit_window_save,
                                 "condition::sensitive", "has-open-document",
                                 NULL);

    moo_window_class_new_action (window_class, "SaveAs",
                                 "name", "Save As",
                                 "label", "Save _As...",
                                 "tooltip", "Save as...",
                                 "icon-stock-id", GTK_STOCK_SAVE_AS,
                                 "accel", "<ctrl><shift>S",
                                 "closure-callback", moo_edit_window_save_as,
                                 "condition::sensitive", "has-open-document",
                                 NULL);

    moo_window_class_new_action (window_class, "Close",
                                 "name", "Close",
                                 "label", "_Close",
                                 "tooltip", "Close document",
                                 "icon-stock-id", GTK_STOCK_CLOSE,
                                 "accel", "<ctrl>W",
                                 "closure-callback", moo_edit_window_close_tab,
                                 "condition::sensitive", "has-open-document",
                                 NULL);

    moo_window_class_new_action (window_class, "CloseAll",
                                 "name", "Close All",
                                 "label", "Close _All",
                                 "tooltip", "Close all documents",
                                 "accel", "<shift><ctrl>W",
                                 "closure-callback", moo_edit_window_close_all,
                                 "condition::sensitive", "has-open-document",
                                 NULL);

    moo_window_class_new_action (window_class, "Undo",
                                 "name", "Undo",
                                 "label", "_Undo",
                                 "tooltip", "Undo",
                                 "icon-stock-id", GTK_STOCK_UNDO,
                                 "accel", "<ctrl>Z",
                                 "closure-signal", "undo",
                                 "closure-proxy-func", moo_edit_window_get_active_doc,
                                 "condition::sensitive", "can-undo",
                                 NULL);

    moo_window_class_new_action (window_class, "Redo",
                                 "name", "Redo",
                                 "label", "_Redo",
                                 "tooltip", "Redo",
                                 "icon-stock-id", GTK_STOCK_REDO,
                                 "accel", "<shift><ctrl>Z",
                                 "closure-signal", "redo",
                                 "closure-proxy-func", moo_edit_window_get_active_doc,
                                 "condition::sensitive", "can-redo",
                                 NULL);

    moo_window_class_new_action (window_class, "Cut",
                                 "name", "Cut",
                                 "label", "Cu_t",
                                 "tooltip", "Cut",
                                 "icon-stock-id", GTK_STOCK_CUT,
                                 "accel", "<ctrl>X",
                                 "closure-signal", "cut-clipboard",
                                 "closure-proxy-func", moo_edit_window_get_active_doc,
                                 "condition::sensitive", "has-selection",
                                 NULL);

    moo_window_class_new_action (window_class, "Copy",
                                 "name", "Copy",
                                 "label", "_Copy",
                                 "tooltip", "Copy",
                                 "icon-stock-id", GTK_STOCK_COPY,
                                 "accel", "<ctrl>C",
                                 "closure-signal", "copy-clipboard",
                                 "closure-proxy-func", moo_edit_window_get_active_doc,
                                 "condition::sensitive", "has-selection",
                                 NULL);

    moo_window_class_new_action (window_class, "Paste",
                                 "name", "Paste",
                                 "label", "_Paste",
                                 "tooltip", "Paste",
                                 "icon-stock-id", GTK_STOCK_PASTE,
                                 "accel", "<ctrl>V",
                                 "closure-signal", "paste-clipboard",
                                 "closure-proxy-func", moo_edit_window_get_active_doc,
                                 "condition::sensitive", "has-open-document",
                                 NULL);

    moo_window_class_new_action (window_class, "Delete",
                                 "name", "Delete",
                                 "label", "_Delete",
                                 "tooltip", "Delete",
                                 "icon-stock-id", GTK_STOCK_DELETE,
                                 "closure-signal", "delete-selection",
                                 "closure-proxy-func", moo_edit_window_get_active_doc,
                                 "condition::sensitive", "has-selection",
                                 NULL);

    moo_window_class_new_action (window_class, "SelectAll",
                                 "name", "Select All",
                                 "label", "Select _All",
                                 "tooltip", "Select all",
                                 "accel", "<ctrl>A",
                                 "closure-callback", moo_text_view_select_all,
                                 "closure-proxy-func", moo_edit_window_get_active_doc,
                                 "condition::sensitive", "has-text",
                                 NULL);

    moo_window_class_new_action (window_class, "PreviousTab",
                                 "name", "Previous Tab",
                                 "label", "_Previous Tab",
                                 "tooltip", "Previous tab",
                                 "icon-stock-id", GTK_STOCK_GO_BACK,
                                 "accel", "<alt>Left",
                                 "closure-callback", moo_edit_window_previous_tab,
                                 "condition::sensitive", "has-open-document",
                                 NULL);

    moo_window_class_new_action (window_class, "NextTab",
                                 "name", "Next Tab",
                                 "label", "_Next Tab",
                                 "tooltip", "Next tab",
                                 "icon-stock-id", GTK_STOCK_GO_FORWARD,
                                 "accel", "<alt>Right",
                                 "closure-callback", moo_edit_window_next_tab,
                                 "condition::sensitive", "has-open-document",
                                 NULL);

    moo_window_class_new_action (window_class, "Find",
                                 "name", "Find",
                                 "label", "_Find",
                                 "tooltip", "Find",
                                 "icon-stock-id", GTK_STOCK_FIND,
                                 "accel", "<ctrl>F",
                                 "closure-signal", "find-interactive",
                                 "closure-proxy-func", moo_edit_window_get_active_doc,
                                 "condition::sensitive", "has-open-document",
                                 NULL);

    moo_window_class_new_action (window_class, "FindNext",
                                 "name", "Find Next",
                                 "label", "Find _Next",
                                 "tooltip", "Find next",
                                 "icon-stock-id", GTK_STOCK_GO_FORWARD,
                                 "accel", "F3",
                                 "closure-signal", "find-next-interactive",
                                 "closure-proxy-func", moo_edit_window_get_active_doc,
                                 "condition::sensitive", "has-open-document",
                                 NULL);

    moo_window_class_new_action (window_class, "FindPrevious",
                                 "name", "Find Previous",
                                 "label", "Find _Previous",
                                 "tooltip", "Find previous",
                                 "icon-stock-id", GTK_STOCK_GO_BACK,
                                 "accel", "<shift>F3",
                                 "closure-signal", "find-prev-interactive",
                                 "closure-proxy-func", moo_edit_window_get_active_doc,
                                 "condition::sensitive", "has-open-document",
                                 NULL);

    moo_window_class_new_action (window_class, "Replace",
                                 "name", "Replace",
                                 "label", "_Replace",
                                 "tooltip", "Replace",
                                 "icon-stock-id", GTK_STOCK_FIND_AND_REPLACE,
                                 "accel", "<ctrl>R",
                                 "closure-signal", "replace-interactive",
                                 "closure-proxy-func", moo_edit_window_get_active_doc,
                                 "condition::sensitive", "has-open-document",
                                 NULL);

    moo_window_class_new_action (window_class, "GoToLine",
                                 "name", "Go to Line",
                                 "label", "_Go to Line",
                                 "tooltip", "Go to line",
                                 "accel", "<ctrl>G",
                                 "closure-signal", "goto-line-interactive",
                                 "closure-proxy-func", moo_edit_window_get_active_doc,
                                 "condition::sensitive", "has-open-document",
                                 NULL);

    moo_window_class_new_action (window_class, "FocusDoc",
                                 "name", "Focus Doc",
                                 "label", "_Focus Doc",
                                 "tooltip", "Focus Doc",
                                 "accel", "<alt>C",
                                 "closure-callback", gtk_widget_grab_focus,
                                 "closure-proxy-func", moo_edit_window_get_active_doc,
                                 "condition::sensitive", "has-open-document",
                                 NULL);

    moo_window_class_new_action (window_class, STOP_ACTION_ID,
                                 "name", "Stop",
                                 "label", "Stop",
                                 "tooltip", "Stop",
                                 "icon-stock-id", GTK_STOCK_STOP,
                                 "accel", "Escape",
                                 "closure-callback", moo_edit_window_abort_jobs,
                                 "condition::sensitive", "has-jobs-running",
                                 "condition::visible", "has-stop-clients",
                                 NULL);

    moo_window_class_new_action_custom (window_class, LANG_ACTION_ID,
                                        (MooWindowActionFunc) create_lang_action,
                                        NULL, NULL);
}


static void
moo_edit_window_init (MooEditWindow *window)
{
    GtkTargetList *targets;

    window->priv = g_new0 (MooEditWindowPrivate, 1);
    window->priv->prefix = g_strdup ("medit");
    window->priv->panes = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
    window->priv->panes_to_save = g_hash_table_new_full (g_str_hash, g_str_equal, g_free,
                                                        (GDestroyNotify) moo_pane_params_free);
    g_object_set (G_OBJECT (window),
                  "menubar-ui-name", "Editor/Menubar",
                  "toolbar-ui-name", "Editor/Toolbar",
                  NULL);

    window->priv->use_fullname = TRUE;

    targets = moo_window_get_target_list (MOO_WINDOW (window));
    gtk_target_list_add_uri_targets (targets, TARGET_URI);
    gtk_target_list_add_text_targets (targets, TARGET_TEXT);
}


MooEditor       *moo_edit_window_get_editor     (MooEditWindow  *window)
{
    g_return_val_if_fail (MOO_IS_EDIT_WINDOW (window), NULL);
    return window->priv->editor;
}


static void
moo_edit_window_destroy (GtkObject *object)
{
    MooEditWindow *window = MOO_EDIT_WINDOW (object);

    if (window->priv->stop_clients || window->priv->jobs)
    {
        GSList *list, *l;

        moo_edit_window_abort_jobs (window);
        g_slist_foreach (window->priv->jobs, (GFunc) g_free, NULL);
        g_slist_free (window->priv->jobs);
        window->priv->jobs = NULL;

        list = g_slist_copy (window->priv->stop_clients);
        for (l = list; l != NULL; l = l->next)
            moo_edit_window_remove_stop_client (window, l->data);
        g_assert (window->priv->stop_clients == NULL);
        g_slist_free (list);
    }

    GTK_OBJECT_CLASS(moo_edit_window_parent_class)->destroy (object);
}


static void moo_edit_window_finalize       (GObject      *object)
{
    MooEditWindow *window = MOO_EDIT_WINDOW (object);
    /* XXX */
    g_hash_table_destroy (window->priv->panes);

    if (window->priv->panes_to_save)
        g_hash_table_destroy (window->priv->panes_to_save);

    if (window->priv->save_params_idle)
    {
        g_warning ("%s: oops", G_STRLOC);
        g_source_remove (window->priv->save_params_idle);
    }

    g_free (window->priv->prefix);
    g_free (window->priv);
    G_OBJECT_CLASS (moo_edit_window_parent_class)->finalize (object);
}


static void     moo_edit_window_set_property(GObject        *object,
                                             guint           prop_id,
                                             const GValue   *value,
                                             GParamSpec     *pspec)
{
    MooEditWindow *window = MOO_EDIT_WINDOW (object);

    switch (prop_id)
    {
        case PROP_EDITOR:
            window->priv->editor = g_value_get_object (value);
            break;

        case PROP_ACTIVE_DOC:
            moo_edit_window_set_active_doc (window, g_value_get_object (value));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void     moo_edit_window_get_property(GObject        *object,
                                             guint           prop_id,
                                             GValue         *value,
                                             GParamSpec     *pspec)
{
    MooEditWindow *window = MOO_EDIT_WINDOW (object);
    MooEdit *doc;

    switch (prop_id)
    {
        case PROP_EDITOR:
            g_value_set_object (value, window->priv->editor);
            break;

        case PROP_ACTIVE_DOC:
            g_value_set_object (value, moo_edit_window_get_active_doc (window));
            break;

        case PROP_CAN_RELOAD:
            doc = ACTIVE_DOC (window);
            g_value_set_boolean (value, doc && moo_edit_get_filename (doc));
            break;
        case PROP_HAS_OPEN_DOCUMENT:
            g_value_set_boolean (value, ACTIVE_DOC (window) != NULL);
            break;
        case PROP_CAN_UNDO:
            doc = ACTIVE_DOC (window);
            g_value_set_boolean (value, doc && moo_text_view_can_undo (MOO_TEXT_VIEW (doc)));
            break;
        case PROP_CAN_REDO:
            doc = ACTIVE_DOC (window);
            g_value_set_boolean (value, doc && moo_text_view_can_redo (MOO_TEXT_VIEW (doc)));
            break;
        case PROP_HAS_SELECTION:
            doc = ACTIVE_DOC (window);
            g_value_set_boolean (value, doc && moo_text_view_has_selection (MOO_TEXT_VIEW (doc)));
            break;
        case PROP_HAS_TEXT:
            doc = ACTIVE_DOC (window);
            g_value_set_boolean (value, doc && moo_text_view_has_text (MOO_TEXT_VIEW (doc)));
            break;
        case PROP_HAS_JOBS_RUNNING:
            g_value_set_boolean (value, window->priv->jobs != NULL);
            break;
        case PROP_HAS_STOP_CLIENTS:
            g_value_set_boolean (value, window->priv->stop_clients != NULL);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}


void         moo_edit_window_set_title_prefix   (MooEditWindow  *window,
                                                 const char     *prefix)
{
    g_return_if_fail (MOO_IS_EDIT_WINDOW (window));

    g_free (window->priv->prefix);
    window->priv->prefix = g_strdup (prefix);

    if (GTK_WIDGET_REALIZED (GTK_WIDGET (window)))
        update_window_title (window);
}


static void
drag_data_received (GtkWidget          *widget,
                    GdkDragContext     *context,
                    int                 x,
                    int                 y,
                    GtkSelectionData   *data,
                    guint               info,
                    guint               time)
{
    char **uris;
    char *text;
    MooEditWindow *window = MOO_EDIT_WINDOW (widget);

    if ((uris = gtk_selection_data_get_uris (data)))
    {
        char **p;

        for (p = uris; *p; ++p)
        {
            char *filename = g_filename_from_uri (*p, NULL, NULL);

            if (filename)
            {
                moo_editor_open_file (window->priv->editor, window,
                                      NULL, filename, NULL);
                g_free (filename);
            }
        }

        g_strfreev (uris);
    }
    else if ((text = gtk_selection_data_get_text (data)))
    {
        MooEdit *doc = moo_editor_new_doc (window->priv->editor, window);

        if (doc)
        {
            /* XXX MooEdit should have appropriate methods */
            GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (doc));
            gtk_text_buffer_insert_at_cursor (buffer, text, -1);
        }

        g_free (text);
    }

    GTK_WIDGET_CLASS(moo_edit_window_parent_class)->drag_data_received (widget, context, x, y,
                                                                        data, info, time);
}


/****************************************************************************/
/* Constructing
 */

GObject        *moo_edit_window_constructor (GType                  type,
                                             guint                  n_props,
                                             GObjectConstructParam *props)
{
    GtkWidget *notebook;
    MooEditWindow *window;

    GObject *object =
            G_OBJECT_CLASS(moo_edit_window_parent_class)->constructor (type, n_props, props);

    window = MOO_EDIT_WINDOW (object);
    g_return_val_if_fail (window->priv->editor != NULL, object);

    window->priv->statusbar = GTK_STATUSBAR (MOO_WINDOW(window)->statusbar);
    gtk_widget_show (MOO_WINDOW(window)->statusbar);
    window->priv->statusbar_context_id =
            gtk_statusbar_get_context_id (window->priv->statusbar,
                                          "MooEditWindow");


    gtk_widget_show (MOO_WINDOW(window)->vbox);

    create_paned (window);

    notebook = g_object_new (MOO_TYPE_NOTEBOOK,
                             "show-tabs", TRUE,
                             "enable-popup", TRUE,
                             "enable-reordering", TRUE,
                             NULL);
    gtk_widget_show (notebook);
    moo_big_paned_add_child (window->paned, notebook);
    window->priv->notebook = MOO_NOTEBOOK (notebook);
    setup_notebook (window);

    g_signal_connect (window, "realize", G_CALLBACK (update_window_title), NULL);

    edit_changed (window, NULL);

    return object;
}


/* XXX */
static void     update_window_title     (MooEditWindow      *window)
{
    MooEdit *edit;
    const char *name;
    MooEditStatus status;
    GString *title;

    edit = ACTIVE_DOC (window);

    if (!edit)
    {
        gtk_window_set_title (GTK_WINDOW (window),
                              window->priv->prefix ? window->priv->prefix : "");
        return;
    }

    if (window->priv->use_fullname)
        name = moo_edit_get_display_filename (edit);
    else
        name = moo_edit_get_display_basename (edit);

    if (!name)
        name = "<\?\?\?\?\?>";

    status = moo_edit_get_status (edit);

    title = g_string_new ("");

    if (window->priv->prefix)
        g_string_append_printf (title, "%s - ", window->priv->prefix);

    g_string_append_printf (title, "%s", name);

    if (status & MOO_EDIT_MODIFIED_ON_DISK)
        g_string_append (title, " [modified on disk]");
    else if (status & MOO_EDIT_DELETED)
        g_string_append (title, " [deleted]");

    if ((status & MOO_EDIT_MODIFIED) && !(status & MOO_EDIT_CLEAN))
        g_string_append (title, " [modified]");

    gtk_window_set_title (GTK_WINDOW (window), title->str);
    g_string_free (title, TRUE);
}


static void
apply_styles (MooEditWindow      *window,
              MooEdit            *edit)
{
    MooLangMgr *mgr;
    MooTextStyleScheme *scheme;

    mgr = moo_editor_get_lang_mgr (window->priv->editor);
    scheme = moo_lang_mgr_get_active_scheme (mgr);
    g_return_if_fail (scheme != NULL);

    moo_text_view_apply_scheme (MOO_TEXT_VIEW (edit), scheme);
}


/****************************************************************************/
/* Actions
 */

static gboolean moo_edit_window_close       (MooEditWindow      *window)
{
    moo_editor_close_window (window->priv->editor, window);
    return TRUE;
}


static void moo_edit_window_new             (MooEditWindow   *window)
{
    moo_editor_new_window (window->priv->editor);
}


static void moo_edit_window_new_tab         (MooEditWindow   *window)
{
    moo_editor_new_doc (window->priv->editor, window);
}


static void moo_edit_window_open            (MooEditWindow   *window)
{
    moo_editor_open (window->priv->editor, window, GTK_WIDGET (window), NULL);
}


static void moo_edit_window_reload          (MooEditWindow   *window)
{
    MooEdit *edit = moo_edit_window_get_active_doc (window);
    g_return_if_fail (edit != NULL);
    _moo_editor_reload (window->priv->editor, edit);
}


static void moo_edit_window_save            (MooEditWindow   *window)
{
    MooEdit *edit = moo_edit_window_get_active_doc (window);
    g_return_if_fail (edit != NULL);
    _moo_editor_save (window->priv->editor, edit);
}


static void moo_edit_window_save_as         (MooEditWindow   *window)
{
    MooEdit *edit = moo_edit_window_get_active_doc (window);
    g_return_if_fail (edit != NULL);
    _moo_editor_save_as (window->priv->editor, edit, NULL, NULL);
}


static void moo_edit_window_close_tab       (MooEditWindow   *window)
{
    MooEdit *edit = moo_edit_window_get_active_doc (window);
    g_return_if_fail (edit != NULL);
    moo_editor_close_doc (window->priv->editor, edit);
}


static void moo_edit_window_close_all       (MooEditWindow   *window)
{
    GSList *docs = moo_edit_window_list_docs (window);
    moo_editor_close_docs (window->priv->editor, docs);
    g_slist_free (docs);
}


static void moo_edit_window_previous_tab    (MooEditWindow   *window)
{
    MooEdit *doc;
    int n = moo_notebook_get_current_page (window->priv->notebook);
    if (n > 0)
        moo_notebook_set_current_page (window->priv->notebook, n - 1);
    else
        moo_notebook_set_current_page (window->priv->notebook, -1);
    doc = moo_edit_window_get_active_doc (window);
    if (doc)
        gtk_widget_grab_focus (GTK_WIDGET (doc));
}


static void moo_edit_window_next_tab        (MooEditWindow   *window)
{
    MooEdit *doc;
    int n = moo_notebook_get_current_page (window->priv->notebook);
    if (n < moo_notebook_get_n_pages (window->priv->notebook) - 1)
        moo_notebook_set_current_page (window->priv->notebook, n + 1);
    else
        moo_notebook_set_current_page (window->priv->notebook, 0);
    doc = moo_edit_window_get_active_doc (window);
    if (doc)
        gtk_widget_grab_focus (GTK_WIDGET (doc));
}


/****************************************************************************/
/* Documents
 */

static void     setup_notebook          (MooEditWindow      *window)
{
    GtkWidget *button, *icon, *frame;

    g_signal_connect_after (window->priv->notebook, "switch-page",
                            G_CALLBACK (notebook_switch_page), window);
    g_signal_connect (window->priv->notebook, "populate-popup",
                      G_CALLBACK (notebook_populate_popup), window);

    frame = gtk_aspect_frame_new (NULL, 0.5, 0.5, 1.0, FALSE);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);

    button = gtk_button_new ();
    gtk_button_set_focus_on_click (GTK_BUTTON (button), FALSE);
    g_signal_connect_swapped (button, "clicked",
                              G_CALLBACK (moo_edit_window_close_tab), window);
    moo_bind_bool_property (button, "sensitive", window, "has-open-document", FALSE);

    icon = gtk_image_new_from_stock (MOO_STOCK_CLOSE, MOO_ICON_SIZE_REAL_SMALL);

    gtk_container_add (GTK_CONTAINER (button), icon);
    gtk_container_add (GTK_CONTAINER (frame), button);
    gtk_widget_show_all (frame);
    moo_notebook_set_action_widget (window->priv->notebook, frame, TRUE);
}


static void     notebook_switch_page    (G_GNUC_UNUSED MooNotebook *notebook,
                                         G_GNUC_UNUSED gpointer whatever,
                                         guint          page_num,
                                         MooEditWindow *window)
{
    edit_changed (window, get_nth_tab (window, page_num));
    g_object_notify (G_OBJECT (window), "active-doc");
}


static void     edit_changed            (MooEditWindow      *window,
                                         MooEdit            *doc)
{
    if (doc == ACTIVE_DOC (window))
    {
        g_object_freeze_notify (G_OBJECT (window));
        g_object_notify (G_OBJECT (window), "can-reload");
        g_object_notify (G_OBJECT (window), "has-open-document");
        g_object_notify (G_OBJECT (window), "can-undo");
        g_object_notify (G_OBJECT (window), "can-redo");
        g_object_notify (G_OBJECT (window), "has-selection");
        g_object_notify (G_OBJECT (window), "has-text");
        g_object_thaw_notify (G_OBJECT (window));

        update_window_title (window);
        update_statusbar (window);
        update_lang_menu (window);
    }

    if (doc)
        update_tab_label (window, doc);
}


static void     edit_filename_changed   (MooEditWindow      *window,
                                         G_GNUC_UNUSED const char *filename,
                                         MooEdit            *doc)
{
    edit_changed (window, doc);
}


static void     proxy_boolean_property  (MooEditWindow      *window,
                                         GParamSpec         *prop,
                                         MooEdit            *doc)
{
    if (doc == ACTIVE_DOC (window))
        g_object_notify (G_OBJECT (window), prop->name);
}


MooEdit *moo_edit_window_get_active_doc (MooEditWindow  *window)
{
    GtkWidget *swin;
    int page;

    g_return_val_if_fail (MOO_IS_EDIT_WINDOW (window), NULL);

    if (!window->priv->notebook)
        return NULL;

    page = moo_notebook_get_current_page (window->priv->notebook);

    if (page < 0)
        return NULL;

    swin = moo_notebook_get_nth_page (window->priv->notebook, page);
    return MOO_EDIT (gtk_bin_get_child (GTK_BIN (swin)));
}


void     moo_edit_window_set_active_doc (MooEditWindow  *window,
                                         MooEdit        *edit)
{
    GtkWidget *swin;
    int page;

    g_return_if_fail (MOO_IS_EDIT_WINDOW (window));
    g_return_if_fail (MOO_IS_EDIT (edit));

    swin = GTK_WIDGET(edit)->parent;
    page = moo_notebook_page_num (window->priv->notebook, swin);
    g_return_if_fail (page >= 0);

    moo_notebook_set_current_page (window->priv->notebook, page);
}


GSList  *moo_edit_window_list_docs      (MooEditWindow  *window)
{
    GSList *list = NULL;
    int num, i;

    g_return_val_if_fail (MOO_IS_EDIT_WINDOW (window), NULL);

    num = moo_notebook_get_n_pages (window->priv->notebook);

    for (i = 0; i < num; i++)
        list = g_slist_prepend (list, get_nth_tab (window, i));

    return g_slist_reverse (list);
}


guint    moo_edit_window_num_docs       (MooEditWindow  *window)
{
    g_return_val_if_fail (MOO_IS_EDIT_WINDOW (window), 0);

    if (!window->priv->notebook)
        return 0;
    else
        return moo_notebook_get_n_pages (window->priv->notebook);
}


static MooEdit *get_nth_tab             (MooEditWindow      *window,
                                         guint               n)
{
    GtkWidget *swin;

    swin = moo_notebook_get_nth_page (window->priv->notebook, n);

    if (swin)
        return MOO_EDIT (gtk_bin_get_child (GTK_BIN (swin)));
    else
        return NULL;
}


static int      get_page_num            (MooEditWindow      *window,
                                         MooEdit            *doc)
{
    GtkWidget *swin;

    g_return_val_if_fail (MOO_IS_EDIT_WINDOW (window), -1);
    g_return_val_if_fail (MOO_IS_EDIT (doc), -1);

    swin = GTK_WIDGET(doc)->parent;
    return moo_notebook_page_num (window->priv->notebook, swin);
}


void             _moo_edit_window_insert_doc    (MooEditWindow  *window,
                                                 MooEdit        *edit,
                                                 int             position)
{
    GtkWidget *label;
    GtkWidget *scrolledwindow;

    g_return_if_fail (MOO_IS_EDIT_WINDOW (window));
    g_return_if_fail (MOO_IS_EDIT (edit));

    label = create_tab_label (edit);
    gtk_widget_show (label);

    scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow),
                                    GTK_POLICY_AUTOMATIC,
                                    GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow),
                                         GTK_SHADOW_ETCHED_IN);
    gtk_container_add (GTK_CONTAINER (scrolledwindow), GTK_WIDGET (edit));
    gtk_widget_show_all (scrolledwindow);

    moo_notebook_insert_page (window->priv->notebook, scrolledwindow, label, position);

    g_signal_connect_swapped (edit, "doc_status_changed",
                              G_CALLBACK (edit_changed), window);
    g_signal_connect_swapped (edit, "filename_changed",
                              G_CALLBACK (edit_filename_changed), window);
    g_signal_connect_swapped (edit, "notify::can-undo",
                              G_CALLBACK (proxy_boolean_property), window);
    g_signal_connect_swapped (edit, "notify::can-redo",
                              G_CALLBACK (proxy_boolean_property), window);
    g_signal_connect_swapped (edit, "notify::has-selection",
                              G_CALLBACK (proxy_boolean_property), window);
    g_signal_connect_swapped (edit, "notify::has-text",
                              G_CALLBACK (proxy_boolean_property), window);
    g_signal_connect_swapped (edit, "notify::lang",
                              G_CALLBACK (edit_lang_changed), window);
    g_signal_connect_swapped (edit, "cursor-moved",
                              G_CALLBACK (edit_cursor_moved), window);

    g_signal_emit (window, signals[NEW_DOC], 0, edit);

    apply_styles (window, edit);

    _moo_doc_attach_plugins (window, edit);

    moo_edit_window_set_active_doc (window, edit);
    edit_changed (window, edit);
    gtk_widget_grab_focus (GTK_WIDGET (edit));
}


void             _moo_edit_window_remove_doc    (MooEditWindow  *window,
                                                 MooEdit        *doc)
{
    int page;

    g_return_if_fail (MOO_IS_EDIT_WINDOW (window));
    g_return_if_fail (MOO_IS_EDIT (doc));

    page = get_page_num (window, doc);
    g_return_if_fail (page >= 0);

    g_signal_emit (window, signals[CLOSE_DOC], 0, doc);

    g_signal_handlers_disconnect_by_func (doc,
                                          (gpointer) edit_changed,
                                          window);
    g_signal_handlers_disconnect_by_func (doc,
                                          (gpointer) edit_filename_changed,
                                          window);
    g_signal_handlers_disconnect_by_func (doc,
                                          (gpointer) proxy_boolean_property,
                                          window);
    g_signal_handlers_disconnect_by_func (doc,
                                          (gpointer) edit_cursor_moved,
                                          window);
    g_signal_handlers_disconnect_by_func (doc,
                                          (gpointer) edit_lang_changed,
                                          window);

    _moo_doc_detach_plugins (window, doc);

    moo_notebook_remove_page (window->priv->notebook, page);
    edit_changed (window, NULL);

    g_signal_emit (window, signals[CLOSE_DOC_AFTER], 0);
    g_object_notify (G_OBJECT (window), "active-doc");
}


static GtkWidget *create_tab_label      (MooEdit            *edit)
{
    GtkWidget *hbox, *modified_icon, *modified_on_disk_icon, *label;
    GtkSizeGroup *group;

    group = gtk_size_group_new (GTK_SIZE_GROUP_VERTICAL);

    hbox = gtk_hbox_new (FALSE, 3);
    gtk_widget_show (hbox);

    modified_on_disk_icon = gtk_image_new ();
    gtk_box_pack_start (GTK_BOX (hbox), modified_on_disk_icon, FALSE, FALSE, 0);

    modified_icon = gtk_image_new_from_stock (MOO_STOCK_DOC_MODIFIED,
                                              GTK_ICON_SIZE_MENU);
    gtk_box_pack_start (GTK_BOX (hbox), modified_icon, FALSE, FALSE, 0);

    label = gtk_label_new (moo_edit_get_display_basename (edit));
    gtk_widget_show (label);
    gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);

    gtk_size_group_add_widget (group, modified_on_disk_icon);
    gtk_size_group_add_widget (group, modified_icon);
    gtk_size_group_add_widget (group, label);

    g_object_set_data (G_OBJECT (hbox), "moo-edit-modified-on-disk-icon", modified_on_disk_icon);
    g_object_set_data (G_OBJECT (hbox), "moo-edit-modified-icon", modified_icon);
    g_object_set_data (G_OBJECT (hbox), "moo-edit-label", label);

    return hbox;
}


static void     update_tab_label        (MooEditWindow      *window,
                                         MooEdit            *doc)
{
    GtkWidget *hbox, *modified_icon, *modified_on_disk_icon, *label;
    MooEditStatus status;
    int page = get_page_num (window, doc);

    g_return_if_fail (page >= 0);

    hbox = moo_notebook_get_tab_label (window->priv->notebook,
                                       GTK_WIDGET(doc)->parent);
    modified_icon = g_object_get_data (G_OBJECT (hbox), "moo-edit-modified-icon");
    modified_on_disk_icon = g_object_get_data (G_OBJECT (hbox), "moo-edit-modified-on-disk-icon");
    label = g_object_get_data (G_OBJECT (hbox), "moo-edit-label");

    g_return_if_fail (GTK_IS_WIDGET (hbox) && GTK_IS_WIDGET (modified_icon) &&
            GTK_IS_WIDGET (modified_on_disk_icon));

    status = moo_edit_get_status (doc);

    if (status & MOO_EDIT_MODIFIED_ON_DISK)
    {
        gtk_image_set_from_stock (GTK_IMAGE (modified_on_disk_icon),
                                  MOO_STOCK_DOC_MODIFIED_ON_DISK,
                                  GTK_ICON_SIZE_MENU);
        gtk_widget_show (modified_on_disk_icon);
    }
    else if (status & MOO_EDIT_DELETED)
    {
        gtk_image_set_from_stock (GTK_IMAGE (modified_on_disk_icon),
                                  MOO_STOCK_DOC_DELETED,
                                  GTK_ICON_SIZE_MENU);
        gtk_widget_show (modified_on_disk_icon);
    }
    else
    {
        gtk_widget_hide (modified_on_disk_icon);
    }

    if ((status & MOO_EDIT_MODIFIED) && !(status & MOO_EDIT_CLEAN))
        gtk_widget_show (modified_icon);
    else
        gtk_widget_hide (modified_icon);

    gtk_label_set_text (GTK_LABEL (label), moo_edit_get_display_basename (doc));
}


/****************************************************************************/
/* Notebook popup menu
 */

static void     close_activated         (GtkWidget          *item,
                                         MooEditWindow      *window)
{
    MooEdit *edit = g_object_get_data (G_OBJECT (item), "moo-edit");
    g_return_if_fail (MOO_IS_EDIT_WINDOW (window));
    g_return_if_fail (MOO_IS_EDIT (edit));
    moo_editor_close_doc (window->priv->editor, edit);
}


static void     close_others_activated  (GtkWidget          *item,
                                         MooEditWindow      *window)
{
    GSList *list;
    MooEdit *edit = g_object_get_data (G_OBJECT (item), "moo-edit");

    g_return_if_fail (MOO_IS_EDIT_WINDOW (window));
    g_return_if_fail (MOO_IS_EDIT (edit));

    list = moo_edit_window_list_docs (window);
    list = g_slist_remove (list, edit);

    moo_editor_close_docs (window->priv->editor, list);

    g_slist_free (list);
}


static gboolean notebook_populate_popup (MooNotebook        *notebook,
                                         GtkWidget          *child,
                                         GtkMenu            *menu,
                                         MooEditWindow      *window)
{
    MooEdit *edit;
    GtkWidget *item;

    g_return_val_if_fail (MOO_IS_EDIT_WINDOW (window), TRUE);
    g_return_val_if_fail (window->priv->notebook == notebook, TRUE);
    g_return_val_if_fail (GTK_IS_SCROLLED_WINDOW (child), TRUE);

    edit = MOO_EDIT (gtk_bin_get_child (GTK_BIN (child)));
    g_return_val_if_fail (MOO_IS_EDIT (edit), TRUE);

    item = gtk_menu_item_new_with_label ("Close");
    gtk_widget_show (item);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
    g_object_set_data (G_OBJECT (item), "moo-edit", edit);
    g_signal_connect (item, "activate",
                      G_CALLBACK (close_activated),
                      window);

    if (moo_edit_window_num_docs (window) > 1)
    {
        item = gtk_menu_item_new_with_label ("Close All Others");
        gtk_widget_show (item);
        gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
        g_object_set_data (G_OBJECT (item), "moo-edit", edit);
        g_signal_connect (item, "activate",
                          G_CALLBACK (close_others_activated),
                          window);
    }

    return FALSE;
}


/****************************************************************************/
/* Panes
 */

static const char *PANE_POSITIONS[4] = {
    "left",
    "right",
    "top",
    "bottom"
};


static void
create_paned (MooEditWindow *window)
{
    MooBigPaned *paned;
    guint i;

    paned = g_object_new (MOO_TYPE_BIG_PANED,
                          "handle-cursor-type", GDK_FLEUR,
                          "enable-detaching", TRUE,
                          NULL);
    gtk_widget_show (GTK_WIDGET (paned));
    gtk_box_pack_start (GTK_BOX (MOO_WINDOW(window)->vbox),
                        GTK_WIDGET (paned), TRUE, TRUE, 0);

    window->paned = paned;

    for (i = 0; i < 4; ++i)
    {
        int size;
        char *key = g_strdup_printf (MOO_EDIT_PREFS_PREFIX "/window/panes/%s",
                                     PANE_POSITIONS[i]);

        moo_prefs_new_key_int (key, -1);
        size = moo_prefs_get_int (key);

        if (size > 0)
            moo_paned_set_pane_size (MOO_PANED (paned->paned[i]), size);

        g_free (key);
    }

    g_signal_connect_swapped (paned, "pane-params-changed",
                              G_CALLBACK (pane_params_changed),
                              window);
    g_signal_connect_swapped (paned, "set-pane-size",
                              G_CALLBACK (pane_size_changed),
                              window);
}


gboolean
moo_edit_window_add_pane (MooEditWindow  *window,
                          const char     *user_id,
                          GtkWidget      *widget,
                          MooPaneLabel   *label,
                          MooPanePosition position)
{
    int result;

    g_return_val_if_fail (MOO_IS_EDIT_WINDOW (window), FALSE);
    g_return_val_if_fail (user_id != NULL, FALSE);
    g_return_val_if_fail (GTK_IS_WIDGET (widget), FALSE);
    g_return_val_if_fail (label != NULL, FALSE);

    g_return_val_if_fail (moo_edit_window_get_pane (window, user_id) == NULL, FALSE);

    gtk_object_sink (g_object_ref (widget));

    result = moo_big_paned_insert_pane (window->paned, widget, label,
                                        position, -1);

    if (result >= 0)
    {
        MooPaneParams *params;

        g_hash_table_insert (window->priv->panes,
                             g_strdup (user_id), widget);
        g_object_set_data_full (G_OBJECT (widget), "moo-edit-window-pane-user-id",
                                g_strdup (user_id), g_free);

        params = load_pane_params (user_id);

        if (params)
        {
            moo_big_paned_set_pane_params (window->paned, position, result, params);
            moo_pane_params_free (params);
        }
    }

    g_object_unref (widget);

    return result >= 0;
}


gboolean
moo_edit_window_remove_pane (MooEditWindow  *window,
                             const char     *user_id)
{
    GtkWidget *widget;
    gboolean result;

    g_return_val_if_fail (MOO_IS_EDIT_WINDOW (window), FALSE);
    g_return_val_if_fail (user_id != NULL, FALSE);

    widget = g_hash_table_lookup (window->priv->panes, user_id);

    if (!widget)
        return FALSE;

    g_hash_table_remove (window->priv->panes, user_id);

    result = moo_big_paned_remove_pane (window->paned, widget);
    g_return_val_if_fail (result, FALSE);
    return TRUE;
}


GtkWidget*
moo_edit_window_get_pane (MooEditWindow  *window,
                          const char     *user_id)
{
    g_return_val_if_fail (MOO_IS_EDIT_WINDOW (window), NULL);
    g_return_val_if_fail (user_id != NULL, NULL);
    return g_hash_table_lookup (window->priv->panes, user_id);
}


#define PANE_PREFS_ROOT                 MOO_EDIT_PREFS_PREFIX "/panes"
#define ELEMENT_PANE                    "pane"
#define PROP_PANE_ID                    "id"
#define PROP_PANE_WINDOW_X              "x"
#define PROP_PANE_WINDOW_Y              "y"
#define PROP_PANE_WINDOW_WIDTH          "width"
#define PROP_PANE_WINDOW_HEIGHT         "height"
#define PROP_PANE_WINDOW_DETACHED       "detached"
#define PROP_PANE_WINDOW_MAXIMIZED      "maximized"
#define PROP_PANE_WINDOW_KEEP_ON_TOP    "keep-on-top"


static MooMarkupNode*
get_pane_element (const char *pane_id,
                  gboolean    create)
{
    MooMarkupDoc *xml;
    MooMarkupNode *root, *node;

    xml = moo_prefs_get_markup ();

    root = moo_markup_get_element (MOO_MARKUP_NODE (xml), PANE_PREFS_ROOT);

    if (!root)
    {
        if (create)
        {
            node = moo_markup_create_element (MOO_MARKUP_NODE (xml),
                                              PANE_PREFS_ROOT "/" ELEMENT_PANE);
            moo_markup_set_prop (node, PROP_PANE_ID, pane_id);
            return node;
        }
        else
        {
            return NULL;
        }
    }

    for (node = root->children; node != NULL; node = node->next)
    {
        const char *id;

        if (!MOO_MARKUP_IS_ELEMENT (node))
            continue;

        if (strcmp (node->name, ELEMENT_PANE))
        {
            g_warning ("%s: invalid element '%s'", G_STRLOC, node->name);
            continue;
        }

        id = moo_markup_get_prop (node, PROP_PANE_ID);

        if (!id || !id[0])
        {
            g_warning ("%s: id missing", G_STRLOC);
            continue;
        }

        if (strcmp (id, pane_id))
            continue;

        return node;
    }

    if (create)
    {
        node = moo_markup_create_element (MOO_MARKUP_NODE (xml),
                                          PANE_PREFS_ROOT "/" ELEMENT_PANE);
        moo_markup_set_prop (node, PROP_PANE_ID, pane_id);
        return node;
    }

    return NULL;
}


static MooPaneParams*
load_pane_params (const char         *pane_id)
{
    MooMarkupNode *node;
    MooPaneParams *params;

    node = get_pane_element (pane_id, FALSE);

    if (!node)
        return NULL;

    params = moo_pane_params_new ();

    params->window_position.x = moo_markup_get_int_prop (node, PROP_PANE_WINDOW_X, 0);
    params->window_position.y = moo_markup_get_int_prop (node, PROP_PANE_WINDOW_Y, 0);
    params->window_position.width = moo_markup_get_int_prop (node, PROP_PANE_WINDOW_WIDTH, 0);
    params->window_position.height = moo_markup_get_int_prop (node, PROP_PANE_WINDOW_HEIGHT, 0);
    params->detached = moo_markup_get_bool_prop (node, PROP_PANE_WINDOW_DETACHED, FALSE);
    params->maximized = moo_markup_get_bool_prop (node, PROP_PANE_WINDOW_MAXIMIZED, FALSE);
    params->keep_on_top = moo_markup_get_bool_prop (node, PROP_PANE_WINDOW_KEEP_ON_TOP, FALSE);

    return params;
}


static void
set_if_not_zero (MooMarkupNode *node,
                 const char    *attr,
                 int            val)
{
    if (val)
        moo_markup_set_int_prop (node, attr, val);
    else
        moo_markup_set_prop (node, attr, NULL);
}


static void
set_if_not_false (MooMarkupNode *node,
                  const char    *attr,
                  gboolean       val)
{
    if (val)
        moo_markup_set_bool_prop (node, attr, val);
    else
        moo_markup_set_prop (node, attr, NULL);
}


static gboolean
save_pane_params (const char         *pane_id,
                  MooPaneParams      *params)
{
    MooMarkupNode *node;

    if (!params)
    {
        node = get_pane_element (pane_id, FALSE);
        if (node)
            moo_markup_delete_node (node);
        return TRUE;
    }

    node = get_pane_element (pane_id, TRUE);
    g_return_val_if_fail (node != NULL, TRUE);

    set_if_not_zero (node, PROP_PANE_WINDOW_X, params->window_position.x);
    set_if_not_zero (node, PROP_PANE_WINDOW_Y, params->window_position.y);
    set_if_not_zero (node, PROP_PANE_WINDOW_WIDTH, params->window_position.width);
    set_if_not_zero (node, PROP_PANE_WINDOW_HEIGHT, params->window_position.height);
    set_if_not_false (node, PROP_PANE_WINDOW_DETACHED, params->detached);
    set_if_not_false (node, PROP_PANE_WINDOW_MAXIMIZED, params->maximized);
    set_if_not_false (node, PROP_PANE_WINDOW_KEEP_ON_TOP, params->keep_on_top);

    return TRUE;
}


static gboolean
do_save_panes (MooEditWindow *window)
{
    g_hash_table_foreach_remove (window->priv->panes_to_save,
                                 (GHRFunc) save_pane_params, NULL);
    window->priv->save_params_idle = 0;
    return FALSE;
}


static void
pane_params_changed (MooEditWindow      *window,
                     MooPanePosition     position,
                     guint               index)
{
    GtkWidget *widget;
    const char *id;
    MooPaneParams *params;

    widget = moo_big_paned_get_pane (window->paned, position, index);
    g_return_if_fail (widget != NULL);
    id = g_object_get_data (G_OBJECT (widget), "moo-edit-window-pane-user-id");

    if (id)
    {
        params = moo_big_paned_get_pane_params (window->paned, position, index);
        g_hash_table_insert (window->priv->panes_to_save,
                             g_strdup (id), params);
        if (!window->priv->save_params_idle)
            window->priv->save_params_idle =
                    g_idle_add ((GSourceFunc) do_save_panes, window);
    }
}


static void
pane_size_changed (MooEditWindow      *window,
                   MooPanePosition     pos)
{
    GtkWidget *paned = window->paned->paned[pos];
    char *key;

    g_return_if_fail (MOO_IS_PANED (paned));
    g_return_if_fail (pos < 4);

    key = g_strdup_printf (MOO_EDIT_PREFS_PREFIX "/window/panes/%s",
                           PANE_POSITIONS[pos]);
    moo_prefs_set_int (key, moo_paned_get_pane_size (MOO_PANED (paned)));
    g_free (key);
}


/****************************************************************************/
/* Statusbar
 */

static void
set_statusbar_numbers (MooEditWindow *window,
                       int            line,
                       int            column)
{
    char *text = g_strdup_printf ("Line: %d Col: %d", line, column);
    gtk_statusbar_pop (window->priv->statusbar,
                       window->priv->statusbar_context_id);
    gtk_statusbar_push (window->priv->statusbar,
                        window->priv->statusbar_context_id,
                        text);
    g_free (text);
}


/* XXX */
static void
update_statusbar (MooEditWindow *window)
{
    MooEdit *edit;
    int line, column;
    GtkTextIter iter;
    GtkTextBuffer *buffer;

    edit = ACTIVE_DOC (window);

    if (!edit)
    {
        gtk_statusbar_pop (window->priv->statusbar,
                           window->priv->statusbar_context_id);
        return;
    }

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (edit));
    gtk_text_buffer_get_iter_at_mark (buffer, &iter,
                                      gtk_text_buffer_get_insert (buffer));
    line = gtk_text_iter_get_line (&iter) + 1;
    column = gtk_text_iter_get_line_offset (&iter) + 1;

    set_statusbar_numbers (window, line, column);
}


/* XXX */
static void
edit_cursor_moved (MooEditWindow      *window,
                   GtkTextIter        *iter,
                   MooEdit            *edit)
{
    if (edit == ACTIVE_DOC (window))
    {
        int line = gtk_text_iter_get_line (iter) + 1;
        int column = gtk_text_iter_get_line_offset (iter) + 1;
        set_statusbar_numbers (window, line, column);
    }
}


/*****************************************************************************/
/* Language menu
 */

#define NONE_LANGUAGE_ID "MOO_LANG_NONE"

static int
cmp_langs (MooLang *lang1,
           MooLang *lang2)
{
    int result;

    result = strcmp (lang1->section, lang2->section);

    if (result)
        return result;
    else
        return strcmp (lang1->display_name, lang2->display_name);
}


static void
lang_item_activated (MooEditWindow *window,
                     const char    *lang_name)
{
    MooEdit *doc = ACTIVE_DOC (window);
    const char *old_val;
    gboolean do_set = FALSE;

    g_return_if_fail (doc != NULL);
    g_return_if_fail (MOO_IS_EDIT_WINDOW (window));

    old_val = moo_edit_get_var (doc, MOO_EDIT_VAR_LANG);

    if (old_val)
        do_set = !lang_name || strcmp (old_val, lang_name);
    else
        do_set = !!lang_name;

    if (do_set)
        moo_edit_set_var_full (doc, MOO_EDIT_VAR_LANG, lang_name,
                               MOO_EDIT_VAR_DEP_NONE);
}


static MooAction*
create_lang_action (MooEditWindow      *window)
{
    MooAction *action;
    MooMenuMgr *menu_mgr;
    MooLangMgr *lang_mgr;
    GSList *langs, *sections, *l;

    lang_mgr = moo_editor_get_lang_mgr (window->priv->editor);

    /* TODO display names, etc. */
    sections = moo_lang_mgr_get_sections (lang_mgr);
    sections = g_slist_sort (sections, (GCompareFunc) strcmp);

    langs = moo_lang_mgr_get_available_langs (lang_mgr);
    langs = g_slist_sort (langs, (GCompareFunc) cmp_langs);

    action = moo_menu_action_new (LANG_ACTION_ID);
    g_object_set (action, "no-accel", TRUE, NULL);
    menu_mgr = moo_menu_action_get_mgr (MOO_MENU_ACTION (action));

    moo_menu_mgr_append (menu_mgr, NULL, LANG_ACTION_ID,
                         "Language", 0, NULL, NULL);

    moo_menu_mgr_append (menu_mgr, LANG_ACTION_ID,
                         NONE_LANGUAGE_ID, "None",
                         MOO_MENU_ITEM_RADIO, NULL, NULL);

    for (l = sections; l != NULL; l = l->next)
        moo_menu_mgr_append (menu_mgr, LANG_ACTION_ID,
                             l->data, l->data, 0, NULL, NULL);

    for (l = langs; l != NULL; l = l->next)
    {
        MooLang *lang = l->data;
        moo_menu_mgr_append (menu_mgr, lang->section,
                             lang->id, lang->display_name, MOO_MENU_ITEM_RADIO,
                             g_strdup (lang->id), g_free);
    }

    g_signal_connect_swapped (menu_mgr, "radio-set-active",
                              G_CALLBACK (lang_item_activated), window);

    g_slist_free (langs);
    g_slist_foreach (sections, (GFunc) g_free, NULL);
    g_slist_free (sections);

    moo_bind_bool_property (action, "sensitive", window, "has-open-document", FALSE);
    return action;
}


static void
update_lang_menu (MooEditWindow      *window)
{
    MooEdit *doc;
    MooAction *action;
    MooLang *lang;

    doc = ACTIVE_DOC (window);

    if (!doc)
        return;

    lang = moo_text_view_get_lang (MOO_TEXT_VIEW (doc));
    action = moo_window_get_action_by_id (MOO_WINDOW (window), LANG_ACTION_ID);
    g_return_if_fail (action != NULL);

    moo_menu_mgr_set_active (moo_menu_action_get_mgr (MOO_MENU_ACTION (action)),
                             lang ? lang->id : NONE_LANGUAGE_ID, TRUE);
}


static void
edit_lang_changed (MooEditWindow      *window,
                   G_GNUC_UNUSED GParamSpec *pspec,
                   MooEdit            *doc)
{
    if (doc == ACTIVE_DOC (window))
        update_lang_menu (window);
}


/*****************************************************************************/
/* Stop button
 */

typedef struct {
    gpointer job;
    MooAbortJobFunc abort;
} Job;


static void
client_died (MooEditWindow  *window,
             gpointer        client)
{
    window->priv->stop_clients = g_slist_remove (window->priv->stop_clients, client);
    moo_edit_window_job_finished (window, client);
}


static void
abort_client_job (gpointer client)
{
    gboolean ret;
    g_signal_emit_by_name (client, "abort", &ret);
}


static void
client_job_started (gpointer        client,
                    const char     *job_name,
                    MooEditWindow  *window)
{
    moo_edit_window_job_started (window, job_name, abort_client_job, client);
}


static void
client_job_finished (gpointer        client,
                     MooEditWindow  *window)
{
    moo_edit_window_job_finished (window, client);
}


void
moo_edit_window_add_stop_client (MooEditWindow  *window,
                                 gpointer        client)
{
    GType type, return_type;
    guint signal_abort, signal_started, signal_finished;
    GSignalQuery query;
    gboolean had_clients;

    g_return_if_fail (MOO_IS_EDIT_WINDOW (window));
    g_return_if_fail (G_IS_OBJECT (client));

    g_return_if_fail (!g_slist_find (window->priv->stop_clients, client));

    type = G_OBJECT_TYPE (client);
    signal_abort = g_signal_lookup ("abort", type);
    signal_started = g_signal_lookup ("job-started", type);
    signal_finished = g_signal_lookup ("job-finished", type);
    g_return_if_fail (signal_abort && signal_started && signal_finished);

#define REAL_TYPE(t__) ((t__) & ~(G_SIGNAL_TYPE_STATIC_SCOPE))
    g_signal_query (signal_abort, &query);
    return_type = REAL_TYPE (query.return_type);
    g_return_if_fail (return_type == G_TYPE_NONE || return_type == G_TYPE_BOOLEAN);
    g_return_if_fail (query.n_params == 0);

    g_signal_query (signal_started, &query);
    g_return_if_fail (REAL_TYPE (query.return_type) == G_TYPE_NONE);
    g_return_if_fail (query.n_params == 1);
    g_return_if_fail (REAL_TYPE (query.param_types[0]) == G_TYPE_STRING);

    g_signal_query (signal_finished, &query);
    g_return_if_fail (REAL_TYPE (query.return_type) == G_TYPE_NONE);
    g_return_if_fail (query.n_params == 0);
#undef REAL_TYPE

    had_clients = window->priv->stop_clients != NULL;
    window->priv->stop_clients = g_slist_prepend (window->priv->stop_clients, client);
    g_object_weak_ref (client, (GWeakNotify) client_died, window);
    g_signal_connect (client, "job-started", G_CALLBACK (client_job_started), window);
    g_signal_connect (client, "job-finished", G_CALLBACK (client_job_finished), window);

    if (!had_clients)
        g_object_notify (G_OBJECT (window), "has-stop-clients");
}


void
moo_edit_window_remove_stop_client (MooEditWindow  *window,
                                    gpointer        client)
{
    g_return_if_fail (MOO_IS_EDIT_WINDOW (window));
    g_return_if_fail (g_slist_find (window->priv->stop_clients, client));

    window->priv->stop_clients = g_slist_remove (window->priv->stop_clients, client);

    if (G_IS_OBJECT (client))
    {
        g_object_weak_unref (client, (GWeakNotify) client_died, window);
        g_signal_handlers_disconnect_by_func (client, (gpointer)client_job_started, window);
        g_signal_handlers_disconnect_by_func (client, (gpointer)client_job_finished, window);
    }

    if (!window->priv->stop_clients)
        g_object_notify (G_OBJECT (window), "has-stop-clients");
}


void
moo_edit_window_abort_jobs (MooEditWindow *window)
{
    GSList *l, *jobs;

    g_return_if_fail (MOO_IS_EDIT_WINDOW (window));

    jobs = g_slist_copy (window->priv->jobs);

    for (l = jobs; l != NULL; l = l->next)
    {
        Job *j = l->data;
        j->abort (j->job);
    }

    g_slist_free (jobs);
}


void
moo_edit_window_job_started (MooEditWindow  *window,
                             G_GNUC_UNUSED const char *name,
                             MooAbortJobFunc func,
                             gpointer        job)
{
    Job *j;
    gboolean had_jobs;

    g_return_if_fail (MOO_IS_EDIT_WINDOW (window));
    g_return_if_fail (func != NULL);
    g_return_if_fail (job != NULL);

    j = g_new0 (Job, 1);
    j->abort = func;
    j->job = job;

    had_jobs = window->priv->jobs != NULL;
    window->priv->jobs = g_slist_prepend (window->priv->jobs, j);

    if (!had_jobs)
        g_object_notify (G_OBJECT (window), "has-jobs-running");
}


void
moo_edit_window_job_finished (MooEditWindow  *window,
                              gpointer        job)
{
    GSList *l;
    Job *j = NULL;

    g_return_if_fail (MOO_IS_EDIT_WINDOW (window));
    g_return_if_fail (job != NULL);

    for (l = window->priv->jobs; l != NULL; l = l->next)
    {
        j = l->data;

        if (j->job == job)
            break;
        else
            j = NULL;
    }

    if (j)
    {
        window->priv->jobs = g_slist_remove (window->priv->jobs, j);

        if (!window->priv->jobs)
            g_object_notify (G_OBJECT (window), "has-jobs-running");

        g_free (j);
    }
}


/* kate: space-indent: on; indent-width: 4; strip on; */
