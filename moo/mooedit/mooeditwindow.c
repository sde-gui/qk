/*
 *   mooeditwindow.c
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

#define MOOEDIT_COMPILATION
#include "mooedit/statusbar-glade.h"
#include "mooedit/mooedit-private.h"
#include "mooedit/moolang-private.h"
#include "mooedit/mooeditor.h"
#include "mooedit/mootextbuffer.h"
#include "mooedit/mooeditprefs.h"
#include "mooedit/mooplugin.h"
#include "mooedit/moocmdview.h"
#include "mooutils/moonotebook.h"
#include "mooutils/moostock.h"
#include "mooutils/moomarshals.h"
#include "mooutils/moomenuaction.h"
#include "mooutils/mooutils-misc.h"
#include "mooutils/moocompat.h"
#include "mooutils/mooglade.h"
#include "moofileview/moofilesystem.h"
#if GTK_CHECK_VERSION(2,9,0)
#include "mooedit/mootextprint.h"
#endif
#include <string.h>
#include <gtk/gtk.h>


#define ACTIVE_DOC moo_edit_window_get_active_doc
#define ACTIVE_PAGE(window) (moo_notebook_get_current_page (window->priv->notebook))

#define LANG_ACTION_ID "LanguageMenu"
#define STOP_ACTION_ID "StopJob"

typedef struct {
    char *property;
    MooEditWindowCheckActionFunc func;
    gpointer data;
    GDestroyNotify notify;
} ActionCheck;

static GHashTable *action_checks; /* char* -> GSList* */
static GSList *windows;

typedef struct {
    MooPaneParams *params;
    int position;
} PaneParams;

struct _MooEditWindowPrivate {
    MooEditor *editor;

    GtkStatusbar *statusbar;
    guint last_msg_id;
    GtkLabel *cursor_label;
    GtkLabel *insert_label;
    GtkWidget *info;

    MooNotebook *notebook;
    char *prefix;
    gboolean use_fullname;
    GHashTable *panes;
    GHashTable *panes_to_save; /* char* */
    guint save_params_idle;

    GSList *stop_clients;
    GSList *jobs; /* Job* */
};

enum {
    TARGET_MOO_EDIT_TAB = 1,
    TARGET_URI_LIST = 2
};

static GdkAtom moo_edit_tab_atom;

static GtkTargetEntry dest_targets[] = {
    {(char*) "MOO_EDIT_TAB", GTK_TARGET_SAME_APP, TARGET_MOO_EDIT_TAB},
    {(char*) "text/uri-list", 0, TARGET_URI_LIST}
};


static ActionCheck *action_check_new            (const char         *action_prop,
                                                 MooEditWindowCheckActionFunc func,
                                                 gpointer            data,
                                                 GDestroyNotify      notify);
static void     action_check_free               (ActionCheck        *check);
static void     action_checks_init              (void);
static void     moo_edit_window_check_actions   (MooEditWindow      *window);

GObject        *moo_edit_window_constructor     (GType               type,
                                                 guint               n_props,
                                                 GObjectConstructParam *props);
static void     moo_edit_window_finalize        (GObject            *object);
static void     moo_edit_window_destroy         (GtkObject          *object);

static void     moo_edit_window_set_property    (GObject            *object,
                                                 guint               prop_id,
                                                 const GValue       *value,
                                                 GParamSpec         *pspec);
static void     moo_edit_window_get_property    (GObject            *object,
                                                 guint               prop_id,
                                                 GValue             *value,
                                                 GParamSpec         *pspec);


static gboolean moo_edit_window_close           (MooEditWindow      *window);

static void     setup_notebook                  (MooEditWindow      *window);
static void     update_window_title             (MooEditWindow      *window);

static void     notebook_switch_page            (MooNotebook        *notebook,
                                                 guint               page_num,
                                                 MooEditWindow      *window);
static gboolean notebook_populate_popup         (MooNotebook        *notebook,
                                                 GtkWidget          *child,
                                                 GtkMenu            *menu,
                                                 MooEditWindow      *window);

static void     proxy_boolean_property          (MooEditWindow      *window,
                                                 GParamSpec         *prop,
                                                 MooEdit            *doc);
static void     edit_changed                    (MooEditWindow      *window,
                                                 MooEdit            *doc);
static void     edit_filename_changed           (MooEditWindow      *window,
                                                 const char         *filename,
                                                 MooEdit            *doc);
static void     edit_lang_changed               (MooEditWindow      *window,
                                                 guint               var_id,
                                                 GParamSpec         *pspec,
                                                 MooEdit            *doc);
static void     edit_overwrite_changed          (MooEditWindow      *window,
                                                 GParamSpec         *pspec,
                                                 MooEdit            *doc);
static GtkWidget *create_tab_label              (MooEditWindow      *window,
                                                 MooEdit            *edit);
static void     update_tab_label                (MooEditWindow      *window,
                                                 MooEdit            *doc);
static void     edit_cursor_moved               (MooEditWindow      *window,
                                                 GtkTextIter        *iter,
                                                 MooEdit            *edit);
static void     update_lang_menu                (MooEditWindow      *window);

static void     create_statusbar                (MooEditWindow      *window);
static void     update_statusbar                (MooEditWindow      *window);
static MooEdit *get_nth_tab                     (MooEditWindow      *window,
                                                 guint               n);
static int      get_page_num                    (MooEditWindow      *window,
                                                 MooEdit            *doc);

static MooAction *create_lang_action            (MooEditWindow      *window);

static void     create_paned                    (MooEditWindow      *window);
static PaneParams *load_pane_params             (const char         *pane_id);
static gboolean save_pane_params                (const char         *pane_id,
                                                 PaneParams         *params);
static void     pane_params_changed             (MooEditWindow      *window,
                                                 MooPanePosition     position,
                                                 guint               index);
static void     pane_size_changed               (MooEditWindow      *window,
                                                 MooPanePosition     position);
static PaneParams *pane_params_new              (void);
static void     pane_params_free                (PaneParams         *params);

static void     notebook_drag_data_recv         (GtkWidget          *widget,
                                                 GdkDragContext     *context,
                                                 int                 x,
                                                 int                 y,
                                                 GtkSelectionData   *data,
                                                 guint               info,
                                                 guint               time,
                                                 MooEditWindow      *window);
static gboolean notebook_drag_drop              (GtkWidget          *widget,
                                                 GdkDragContext     *context,
                                                 int                 x,
                                                 int                 y,
                                                 guint               time,
                                                 MooEditWindow      *window);
static gboolean notebook_drag_motion            (GtkWidget          *widget,
                                                 GdkDragContext     *context,
                                                 int                 x,
                                                 int                 y,
                                                 guint               time,
                                                 MooEditWindow      *window);


/* actions */
static void moo_edit_window_new         (MooEditWindow      *window);
static void moo_edit_window_new_doc     (MooEditWindow      *window);
static void moo_edit_window_open        (MooEditWindow      *window);
static void moo_edit_window_reload      (MooEditWindow      *window);
static void moo_edit_window_save        (MooEditWindow      *window);
static void moo_edit_window_save_as     (MooEditWindow      *window);
static void moo_edit_window_close_tab   (MooEditWindow      *window);
static void moo_edit_window_close_all   (MooEditWindow      *window);
static void moo_edit_window_previous_tab(MooEditWindow      *window);
static void moo_edit_window_next_tab    (MooEditWindow      *window);
static void moo_edit_window_toggle_bookmark (MooEditWindow  *window);
static void moo_edit_window_next_bookmark (MooEditWindow    *window);
static void moo_edit_window_prev_bookmark (MooEditWindow    *window);
static void moo_edit_window_next_ph     (MooEditWindow      *window);
static void moo_edit_window_prev_ph     (MooEditWindow      *window);


#if GTK_CHECK_VERSION(2,9,0)
static void moo_edit_window_page_setup  (MooEditWindow    *window);
static void moo_edit_window_print       (MooEditWindow    *window);
#endif


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
    PROP_HAS_COMMENTS,
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


static void
moo_edit_window_class_init (MooEditWindowClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GtkObjectClass *gtkobject_class = GTK_OBJECT_CLASS (klass);
    MooWindowClass *window_class = MOO_WINDOW_CLASS (klass);

    action_checks_init ();

    gobject_class->constructor = moo_edit_window_constructor;
    gobject_class->finalize = moo_edit_window_finalize;
    gobject_class->set_property = moo_edit_window_set_property;
    gobject_class->get_property = moo_edit_window_get_property;
    gtkobject_class->destroy = moo_edit_window_destroy;
    window_class->close = (gboolean (*) (MooWindow*))moo_edit_window_close;

    moo_edit_tab_atom = gdk_atom_intern ("MOO_EDIT_TAB", FALSE);

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
    INSTALL_PROP (PROP_HAS_COMMENTS, "has-comments");
    INSTALL_PROP (PROP_HAS_TEXT, "has-text");
    INSTALL_PROP (PROP_HAS_JOBS_RUNNING, "has-jobs-running");
    INSTALL_PROP (PROP_HAS_STOP_CLIENTS, "has-stop-clients");

    moo_window_class_set_id (window_class, "Editor", "Editor");

    moo_window_class_new_action (window_class, "NewWindow",
                                 "name", "New Window",
                                 "label", "_New Window",
                                 "tooltip", "Open new editor window",
                                 "icon-stock-id", GTK_STOCK_NEW,
                                 "accel", "<shift><ctrl>N",
                                 "closure-callback", moo_edit_window_new,
                                 NULL);

    moo_window_class_new_action (window_class, "NewDoc",
                                 "name", "New",
                                 "label", "New",
                                 "tooltip", "Create new document",
                                 "icon-stock-id", GTK_STOCK_NEW,
                                 "accel", "<ctrl>N",
                                 "closure-callback", moo_edit_window_new_doc,
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

    moo_window_class_new_action (window_class, "ToggleBookmark",
                                 "name", "Toggle Bookmark",
                                 "label", "Toggle Bookmark",
                                 "tooltip", "Toggle bookmark",
                                 "icon-stock-id", MOO_STOCK_EDIT_BOOKMARK,
                                 "accel", "<ctrl>B",
                                 "closure-callback", moo_edit_window_toggle_bookmark,
                                 "condition::sensitive", "has-open-document",
                                 NULL);

    moo_window_class_new_action (window_class, "NextBookmark",
                                 "name", "Next Bookmark",
                                 "label", "Next Bookmark",
                                 "tooltip", "Next bookmark",
                                 "icon-stock-id", GTK_STOCK_GO_DOWN,
                                 "accel", "<alt>Down",
                                 "closure-callback", moo_edit_window_next_bookmark,
                                 "condition::visible", "has-open-document",
                                 NULL);

    moo_window_class_new_action (window_class, "PreviousBookmark",
                                 "name", "Previous Bookmark",
                                 "label", "Previous Bookmark",
                                 "tooltip", "Previous bookmark",
                                 "icon-stock-id", GTK_STOCK_GO_UP,
                                 "accel", "<alt>Up",
                                 "closure-callback", moo_edit_window_prev_bookmark,
                                 "condition::visible", "has-open-document",
                                 NULL);

    moo_window_class_new_action (window_class, "NextPlaceholder",
                                 "name", "Next Placeholder",
                                 "label", "Next Placeholder",
                                 "tooltip", "Go to next placeholder",
                                 "icon-stock-id", GTK_STOCK_GO_FORWARD,
                                 "closure-callback", moo_edit_window_next_ph,
                                 "condition::visible", "has-open-document",
                                 NULL);

    moo_window_class_new_action (window_class, "PrevPlaceholder",
                                 "name", "Previous Placeholder",
                                 "label", "Previous Placeholder",
                                 "tooltip", "Go to previous placeholder",
                                 "icon-stock-id", GTK_STOCK_GO_BACK,
                                 "closure-callback", moo_edit_window_prev_ph,
                                 "condition::visible", "has-open-document",
                                 NULL);

    moo_window_class_new_action (window_class, "QuickSearch",
                                 "name", "Quick Search",
                                 "label", "Quick Search",
                                 "tooltip", "Quick search",
                                 "icon-stock-id", GTK_STOCK_FIND,
                                 "accel", "<ctrl>slash",
                                 "closure-callback", moo_text_view_start_quick_search,
                                 "closure-proxy-func", moo_edit_window_get_active_doc,
                                 "condition::sensitive", "has-open-document",
                                 NULL);

    moo_window_class_new_action (window_class, "Comment",
                                 "name", "Comment",
                                 "label", "Comment",
                                 "tooltip", "Comment",
                                 "closure-callback", moo_edit_comment,
                                 "closure-proxy-func", moo_edit_window_get_active_doc,
                                 "condition::sensitive", "has-comments",
                                 NULL);

    moo_window_class_new_action (window_class, "Uncomment",
                                 "name", "Uncomment",
                                 "label", "Uncomment",
                                 "tooltip", "Uncomment",
                                 "closure-callback", moo_edit_uncomment,
                                 "closure-proxy-func", moo_edit_window_get_active_doc,
                                 "condition::sensitive", "has-comments",
                                 NULL);

    moo_window_class_new_action (window_class, "Indent",
                                 "name", "Indent",
                                 "label", "Indent",
                                 "tooltip", "Indent",
                                 "accel", "<ctrl>i",
                                 "closure-callback", moo_text_view_indent,
                                 "closure-proxy-func", moo_edit_window_get_active_doc,
                                 "condition::sensitive", "has-open-document",
                                 NULL);

    moo_window_class_new_action (window_class, "Unindent",
                                 "name", "Unindent",
                                 "label", "Unindent",
                                 "tooltip", "Unindent",
                                 "accel", "<shift><ctrl>i",
                                 "closure-callback", moo_text_view_unindent,
                                 "closure-proxy-func", moo_edit_window_get_active_doc,
                                 "condition::sensitive", "has-open-document",
                                 NULL);

#if GTK_CHECK_VERSION(2,9,0)
    moo_window_class_new_action (window_class, "PageSetup",
                                 "name", "Page Setup",
                                 "label", "Page Setup",
                                 "tooltip", "Page Setup",
                                 "accel", "<ctrl><shift>P",
                                 "closure-callback", moo_edit_window_page_setup,
                                 NULL);

    moo_window_class_new_action (window_class, "Print",
                                 "name", "Print",
                                 "label", "Print",
                                 "tooltip", "Print",
                                 "accel", "<ctrl>P",
                                 "icon-stock-id", GTK_STOCK_PRINT,
                                 "closure-callback", moo_edit_window_print,
                                 "condition::sensitive", "has-open-document",
                                 NULL);
#endif

    moo_window_class_new_action_custom (window_class, LANG_ACTION_ID,
                                        (MooWindowActionFunc) create_lang_action,
                                        NULL, NULL);
}


static void
moo_edit_window_init (MooEditWindow *window)
{
    window->priv = g_new0 (MooEditWindowPrivate, 1);
    window->priv->prefix = g_strdup ("medit");
    window->priv->panes = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
    window->priv->panes_to_save = g_hash_table_new_full (g_str_hash, g_str_equal, g_free,
                                                        (GDestroyNotify) pane_params_free);
    g_object_set (G_OBJECT (window),
                  "menubar-ui-name", "Editor/Menubar",
                  "toolbar-ui-name", "Editor/Toolbar",
                  NULL);

    window->priv->use_fullname = TRUE;

    windows = g_slist_prepend (windows, window);
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

    windows = g_slist_remove (windows, window);

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
        case PROP_HAS_COMMENTS:
            doc = ACTIVE_DOC (window);
            g_value_set_boolean (value, doc && _moo_edit_has_comments (doc, NULL, NULL));
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


/****************************************************************************/
/* Constructing
 */

GObject *
moo_edit_window_constructor (GType                  type,
                             guint                  n_props,
                             GObjectConstructParam *props)
{
    GtkWidget *notebook;
    MooEditWindow *window;
    GtkWindowGroup *group;

    GObject *object =
            G_OBJECT_CLASS(moo_edit_window_parent_class)->constructor (type, n_props, props);

    window = MOO_EDIT_WINDOW (object);
    g_return_val_if_fail (window->priv->editor != NULL, object);

    group = gtk_window_group_new ();
    gtk_window_group_add_window (group, GTK_WINDOW (window));
    g_object_unref (group);

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

    create_statusbar (window);

    g_signal_connect (window, "realize", G_CALLBACK (update_window_title), NULL);

    edit_changed (window, NULL);

    return object;
}


/* XXX */
static void
update_window_title (MooEditWindow *window)
{
    MooEdit *edit;
    const char *name;
    MooEditStatus status;
    GString *title;
    gboolean modified;

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

    modified = (status & MOO_EDIT_MODIFIED) && !(status & MOO_EDIT_CLEAN);

    if ((status & MOO_EDIT_NEW) && !modified)
        g_string_append (title, " [new file]");

    if (status & MOO_EDIT_MODIFIED_ON_DISK)
        g_string_append (title, " [modified on disk]");
    else if (status & MOO_EDIT_DELETED)
        g_string_append (title, " [deleted]");

    if (modified)
        g_string_append (title, " [modified]");

    gtk_window_set_title (GTK_WINDOW (window), title->str);
    g_string_free (title, TRUE);
}


MooEditWindow *
moo_edit_get_window (MooEdit *edit)
{
    GtkWidget *toplevel;

    g_return_val_if_fail (MOO_IS_EDIT (edit), NULL);

    toplevel = gtk_widget_get_toplevel (GTK_WIDGET (edit));

    if (MOO_IS_EDIT_WINDOW (toplevel))
        return MOO_EDIT_WINDOW (toplevel);
    else
        return NULL;
}


/****************************************************************************/
/* Actions
 */

static gboolean moo_edit_window_close       (MooEditWindow      *window)
{
    moo_editor_close_window (window->priv->editor, window, TRUE);
    return TRUE;
}


static void moo_edit_window_new             (MooEditWindow   *window)
{
    moo_editor_new_window (window->priv->editor);
}


static void moo_edit_window_new_doc         (MooEditWindow   *window)
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
    _moo_editor_reload (window->priv->editor, edit, NULL);
}


static void moo_edit_window_save            (MooEditWindow   *window)
{
    MooEdit *edit = moo_edit_window_get_active_doc (window);
    g_return_if_fail (edit != NULL);
    _moo_editor_save (window->priv->editor, edit, NULL);
}


static void moo_edit_window_save_as         (MooEditWindow   *window)
{
    MooEdit *edit = moo_edit_window_get_active_doc (window);
    g_return_if_fail (edit != NULL);
    _moo_editor_save_as (window->priv->editor, edit, NULL, NULL, NULL);
}


static void moo_edit_window_close_tab       (MooEditWindow   *window)
{
    MooEdit *edit = moo_edit_window_get_active_doc (window);
    g_return_if_fail (edit != NULL);
    moo_editor_close_doc (window->priv->editor, edit, TRUE);
}


static void moo_edit_window_close_all       (MooEditWindow   *window)
{
    GSList *docs = moo_edit_window_list_docs (window);
    moo_editor_close_docs (window->priv->editor, docs, TRUE);
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


static void
moo_edit_window_toggle_bookmark (MooEditWindow *window)
{
    MooEdit *doc = moo_edit_window_get_active_doc (window);
    g_return_if_fail (doc != NULL);
    moo_edit_toggle_bookmark (doc, moo_text_view_get_cursor_line (MOO_TEXT_VIEW (doc)));
}


static void
moo_edit_window_next_bookmark (MooEditWindow *window)
{
    int cursor;
    GSList *bookmarks;
    MooEdit *doc = moo_edit_window_get_active_doc (window);

    g_return_if_fail (doc != NULL);

    cursor = moo_text_view_get_cursor_line (MOO_TEXT_VIEW (doc));
    bookmarks = moo_edit_get_bookmarks_in_range (doc, cursor + 1, -1);

    if (bookmarks)
    {
        cursor = moo_line_mark_get_line (bookmarks->data);
        moo_text_view_move_cursor (MOO_TEXT_VIEW (doc), cursor, 0, FALSE, FALSE);
        g_slist_free (bookmarks);
    }
}


static void
moo_edit_window_prev_bookmark (MooEditWindow *window)
{
    int cursor;
    GSList *bookmarks = NULL;
    MooEdit *doc = moo_edit_window_get_active_doc (window);

    g_return_if_fail (doc != NULL);

    cursor = moo_text_view_get_cursor_line (MOO_TEXT_VIEW (doc));

    if (cursor > 0)
        bookmarks = moo_edit_get_bookmarks_in_range (doc, 0, cursor - 1);

    if (bookmarks)
    {
        GSList *last = g_slist_last (bookmarks);
        cursor = moo_line_mark_get_line (last->data);
        moo_text_view_move_cursor (MOO_TEXT_VIEW (doc), cursor, 0, FALSE, FALSE);
        g_slist_free (bookmarks);
    }
}


static void
moo_edit_window_next_ph (MooEditWindow *window)
{
    MooEdit *doc = moo_edit_window_get_active_doc (window);
    g_return_if_fail (doc != NULL);
    moo_text_view_next_placeholder (MOO_TEXT_VIEW (doc));
}


static void
moo_edit_window_prev_ph (MooEditWindow *window)
{
    MooEdit *doc = moo_edit_window_get_active_doc (window);
    g_return_if_fail (doc != NULL);
    moo_text_view_prev_placeholder (MOO_TEXT_VIEW (doc));
}


#if GTK_CHECK_VERSION(2,9,0)
static void
moo_edit_window_page_setup (MooEditWindow *window)
{
    gpointer doc = moo_edit_window_get_active_doc (window);
    moo_edit_page_setup (doc, GTK_WIDGET (window));
}


static void
moo_edit_window_print (MooEditWindow *window)
{
    gpointer doc = moo_edit_window_get_active_doc (window);
    g_return_if_fail (doc != NULL);
    moo_edit_print (doc, GTK_WIDGET (window));
}
#endif


/****************************************************************************/
/* Documents
 */

static void
setup_notebook (MooEditWindow *window)
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

    gtk_drag_dest_set (GTK_WIDGET (window->priv->notebook), 0,
                       dest_targets, G_N_ELEMENTS (dest_targets),
                       GDK_ACTION_COPY | GDK_ACTION_MOVE);
    gtk_drag_dest_add_text_targets (GTK_WIDGET (window->priv->notebook));
    g_signal_connect (window->priv->notebook, "drag-motion",
                      G_CALLBACK (notebook_drag_motion), window);
    g_signal_connect (window->priv->notebook, "drag-drop",
                      G_CALLBACK (notebook_drag_drop), window);
    g_signal_connect (window->priv->notebook, "drag-data-received",
                      G_CALLBACK (notebook_drag_data_recv), window);
}


static void
notebook_switch_page (G_GNUC_UNUSED MooNotebook *notebook,
                      guint          page_num,
                      MooEditWindow *window)
{
    edit_changed (window, get_nth_tab (window, page_num));
    moo_edit_window_check_actions (window);
    g_object_notify (G_OBJECT (window), "active-doc");
}


static void
edit_changed (MooEditWindow *window,
              MooEdit       *doc)
{
    if (doc == ACTIVE_DOC (window))
    {
        g_object_freeze_notify (G_OBJECT (window));
        g_object_notify (G_OBJECT (window), "can-reload");
        g_object_notify (G_OBJECT (window), "has-open-document");
        g_object_notify (G_OBJECT (window), "can-undo");
        g_object_notify (G_OBJECT (window), "can-redo");
        g_object_notify (G_OBJECT (window), "has-selection");
        g_object_notify (G_OBJECT (window), "has-comments");
        g_object_notify (G_OBJECT (window), "has-text");
        g_object_thaw_notify (G_OBJECT (window));

        update_window_title (window);
        update_statusbar (window);
        update_lang_menu (window);
    }

    if (doc)
        update_tab_label (window, doc);
}


static void
edit_overwrite_changed (MooEditWindow *window,
                        G_GNUC_UNUSED GParamSpec *pspec,
                        MooEdit       *doc)
{
    if (doc == ACTIVE_DOC (window))
        update_statusbar (window);
}


static void
edit_filename_changed (MooEditWindow      *window,
                       G_GNUC_UNUSED const char *filename,
                       MooEdit            *doc)
{
    edit_changed (window, doc);
}


static void
proxy_boolean_property (MooEditWindow      *window,
                        GParamSpec         *prop,
                        MooEdit            *doc)
{
    if (doc == ACTIVE_DOC (window))
        g_object_notify (G_OBJECT (window), prop->name);
}


MooEdit *
moo_edit_window_get_active_doc (MooEditWindow  *window)
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


void
moo_edit_window_set_active_doc (MooEditWindow *window,
                                MooEdit       *edit)
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


GSList *
moo_edit_window_list_docs (MooEditWindow *window)
{
    GSList *list = NULL;
    int num, i;

    g_return_val_if_fail (MOO_IS_EDIT_WINDOW (window), NULL);

    num = moo_notebook_get_n_pages (window->priv->notebook);

    for (i = 0; i < num; i++)
        list = g_slist_prepend (list, get_nth_tab (window, i));

    return g_slist_reverse (list);
}


guint
moo_edit_window_num_docs (MooEditWindow *window)
{
    g_return_val_if_fail (MOO_IS_EDIT_WINDOW (window), 0);

    if (!window->priv->notebook)
        return 0;
    else
        return moo_notebook_get_n_pages (window->priv->notebook);
}


static MooEdit *
get_nth_tab (MooEditWindow *window,
             guint          n)
{
    GtkWidget *swin;

    swin = moo_notebook_get_nth_page (window->priv->notebook, n);

    if (swin)
        return MOO_EDIT (gtk_bin_get_child (GTK_BIN (swin)));
    else
        return NULL;
}


static int
get_page_num (MooEditWindow *window,
              MooEdit       *doc)
{
    GtkWidget *swin;

    g_return_val_if_fail (MOO_IS_EDIT_WINDOW (window), -1);
    g_return_val_if_fail (MOO_IS_EDIT (doc), -1);

    swin = GTK_WIDGET(doc)->parent;
    return moo_notebook_page_num (window->priv->notebook, swin);
}


void
_moo_edit_window_insert_doc (MooEditWindow  *window,
                             MooEdit        *edit,
                             int             position)
{
    GtkWidget *label;
    GtkWidget *scrolledwindow;

    g_return_if_fail (MOO_IS_EDIT_WINDOW (window));
    g_return_if_fail (MOO_IS_EDIT (edit));

    label = create_tab_label (window, edit);
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
    g_signal_connect_swapped (edit, "notify::overwrite",
                              G_CALLBACK (edit_overwrite_changed), window);
    g_signal_connect_swapped (edit, "filename_changed",
                              G_CALLBACK (edit_filename_changed), window);
    g_signal_connect_swapped (edit, "notify::can-undo",
                              G_CALLBACK (proxy_boolean_property), window);
    g_signal_connect_swapped (edit, "notify::can-redo",
                              G_CALLBACK (proxy_boolean_property), window);
    g_signal_connect_swapped (edit, "notify::has-selection",
                              G_CALLBACK (proxy_boolean_property), window);
    g_signal_connect_swapped (edit, "notify::has-comments",
                              G_CALLBACK (proxy_boolean_property), window);
    g_signal_connect_swapped (edit, "notify::has-text",
                              G_CALLBACK (proxy_boolean_property), window);
    g_signal_connect_swapped (edit, "config-notify::lang",
                              G_CALLBACK (edit_lang_changed), window);
    g_signal_connect_swapped (edit, "cursor-moved",
                              G_CALLBACK (edit_cursor_moved), window);

    g_signal_emit (window, signals[NEW_DOC], 0, edit);

    _moo_doc_attach_plugins (window, edit);

    moo_edit_window_set_active_doc (window, edit);
    edit_changed (window, edit);
    gtk_widget_grab_focus (GTK_WIDGET (edit));
}


void
_moo_edit_window_remove_doc (MooEditWindow  *window,
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


typedef struct {
    int x;
    int y;
} DragInfo;


static gboolean tab_icon_button_press       (GtkWidget      *evbox,
                                             GdkEventButton *event,
                                             MooEditWindow  *window);
static gboolean tab_icon_button_release     (GtkWidget      *evbox,
                                             GdkEventButton *event,
                                             MooEditWindow  *window);
static gboolean tab_icon_motion_notify      (GtkWidget      *evbox,
                                             GdkEventMotion *event,
                                             MooEditWindow  *window);
static void     tab_icon_drag_data_delete   (GtkWidget      *evbox,
                                             GdkDragContext *context,
                                             MooEditWindow  *window);
static void     tab_icon_drag_data_get      (GtkWidget      *evbox,
                                             GdkDragContext *context,
                                             GtkSelectionData *data,
                                             guint           info,
                                             guint           time,
                                             MooEditWindow  *window);
static void     tab_icon_drag_end           (GtkWidget      *evbox,
                                             GdkDragContext *context,
                                             MooEditWindow  *window);

static gboolean
tab_icon_button_release (GtkWidget      *evbox,
                         G_GNUC_UNUSED GdkEventButton *event,
                         MooEditWindow  *window)
{
    g_object_set_data (G_OBJECT (evbox), "moo-drag-info", NULL);
    g_signal_handlers_disconnect_by_func (evbox,
                                          (gpointer) tab_icon_button_release,
                                          window);
    g_signal_handlers_disconnect_by_func (evbox,
                                          (gpointer) tab_icon_motion_notify,
                                          window);
    return FALSE;
}


static gboolean
tab_icon_motion_notify (GtkWidget      *evbox,
                        GdkEventMotion *event,
                        MooEditWindow  *window)
{
    DragInfo *info;

    info = g_object_get_data (G_OBJECT (evbox), "moo-drag-info");
    g_return_val_if_fail (info != NULL, FALSE);

    if (gtk_drag_check_threshold (evbox, info->x, info->y, event->x, event->y))
    {
        GtkTargetList *targets;
        GdkDragContext *context;
        GdkPixbuf *pixbuf;
        GtkImage *icon;
        MooEdit *edit;

        edit = g_object_get_data (G_OBJECT (evbox), "moo-edit");
        g_return_val_if_fail (MOO_IS_EDIT (edit), FALSE);

        targets = gtk_target_list_new (NULL, 0);

        gtk_target_list_add (targets,
                             gdk_atom_intern ("text/uri-list", FALSE),
                             0, TARGET_URI_LIST);
        gtk_target_list_add (targets,
                             gdk_atom_intern ("MOO_EDIT_TAB", FALSE),
                             GTK_TARGET_SAME_APP,
                             TARGET_MOO_EDIT_TAB);

        context = gtk_drag_begin (evbox, targets,
                                  GDK_ACTION_COPY | GDK_ACTION_MOVE | GDK_ACTION_LINK,
                                  1, (GdkEvent*) event);

        icon = g_object_get_data (G_OBJECT (evbox), "moo-edit-icon");
        pixbuf = gtk_image_get_pixbuf (icon);
        gtk_drag_set_icon_pixbuf (context, pixbuf, 0, 0);

        g_signal_connect (evbox, "drag-data-delete",
                          G_CALLBACK (tab_icon_drag_data_delete), window);
        g_signal_connect (evbox, "drag-data-get",
                          G_CALLBACK (tab_icon_drag_data_get), window);
        g_signal_connect (evbox, "drag-end",
                          G_CALLBACK (tab_icon_drag_end), window);

        gtk_target_list_unref (targets);
    }

    return TRUE;
}


static gboolean
tab_icon_button_press (GtkWidget        *evbox,
                       GdkEventButton   *event,
                       MooEditWindow    *window)
{
    DragInfo *info;

    if (event->window != evbox->window || event->button != 1 || event->type != GDK_BUTTON_PRESS)
        return FALSE;

    info = g_new0 (DragInfo, 1);
    info->x = event->x;
    info->y = event->y;

    g_object_set_data_full (G_OBJECT (evbox), "moo-drag-info", info, g_free);
    g_signal_connect (evbox, "motion-notify-event", G_CALLBACK (tab_icon_motion_notify), window);
    g_signal_connect (evbox, "button-release-event", G_CALLBACK (tab_icon_button_release), window);

    return FALSE;
}


static void
tab_icon_drag_data_delete (G_GNUC_UNUSED GtkWidget      *evbox,
                           G_GNUC_UNUSED GdkDragContext *context,
                           G_GNUC_UNUSED MooEditWindow  *window)
{
    g_print ("delete!\n");
}


static void
tab_icon_drag_data_get (GtkWidget      *evbox,
                        G_GNUC_UNUSED GdkDragContext *context,
                        GtkSelectionData *data,
                        guint           info,
                        G_GNUC_UNUSED guint           time,
                        G_GNUC_UNUSED MooEditWindow  *window)
{
    MooEdit *edit = g_object_get_data (G_OBJECT (evbox), "moo-edit");
    g_return_if_fail (MOO_IS_EDIT (edit));

    if (info == TARGET_MOO_EDIT_TAB)
    {
        moo_selection_data_set_pointer (data,
                                        gdk_atom_intern ("MOO_EDIT_TAB", FALSE),
                                        edit);
    }
    else if (info == TARGET_URI_LIST)
    {
        char *uris[] = {NULL, NULL};
        uris[0] = moo_edit_get_uri (edit);
        gtk_selection_data_set_uris (data, uris);
        g_free (uris[0]);
    }
    else
    {
        g_print ("drag-data-get WTF?\n");
        gtk_selection_data_set_text (data, "ERROR", -1);
    }
}


static void
tab_icon_drag_end (GtkWidget      *evbox,
                   G_GNUC_UNUSED GdkDragContext *context,
                   MooEditWindow  *window)
{
    g_object_set_data (G_OBJECT (evbox), "moo-drag-info", NULL);
    g_signal_handlers_disconnect_by_func (evbox,
                                          (gpointer) tab_icon_drag_data_delete,
                                          window);
    g_signal_handlers_disconnect_by_func (evbox,
                                          (gpointer) tab_icon_drag_data_get,
                                          window);
    g_signal_handlers_disconnect_by_func (evbox,
                                          (gpointer) tab_icon_drag_end,
                                          window);
}


static GtkWidget *
create_tab_label (MooEditWindow *window,
                  MooEdit       *edit)
{
    GtkWidget *hbox, *icon, *label, *evbox;
    GtkSizeGroup *group;

    group = gtk_size_group_new (GTK_SIZE_GROUP_VERTICAL);

    hbox = gtk_hbox_new (FALSE, 3);
    gtk_widget_show (hbox);

    evbox = gtk_event_box_new ();
    gtk_box_pack_start (GTK_BOX (hbox), evbox, FALSE, FALSE, 0);
    icon = gtk_image_new ();
    gtk_container_add (GTK_CONTAINER (evbox), icon);
    gtk_widget_show_all (evbox);

    label = gtk_label_new (moo_edit_get_display_basename (edit));
    gtk_widget_show (label);
    gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);

    gtk_size_group_add_widget (group, evbox);
    gtk_size_group_add_widget (group, label);

    g_object_set_data (G_OBJECT (hbox), "moo-edit-icon", icon);
    g_object_set_data (G_OBJECT (hbox), "moo-edit-icon-evbox", evbox);
    g_object_set_data (G_OBJECT (hbox), "moo-edit-label", label);
    g_object_set_data (G_OBJECT (evbox), "moo-edit-icon", icon);
    g_object_set_data (G_OBJECT (evbox), "moo-edit", edit);
    g_object_set_data (G_OBJECT (icon), "moo-edit", edit);

    g_signal_connect (evbox, "button-press-event",
                      G_CALLBACK (tab_icon_button_press),
                      window);

    return hbox;
}


static void
update_tab_label (MooEditWindow *window,
                  MooEdit       *doc)
{
    GtkWidget *hbox, *icon, *label, *evbox;
    MooEditStatus status;
    char *label_text;
    gboolean modified, deleted;
    GdkPixbuf *pixbuf;
    int page;

    page = get_page_num (window, doc);
    g_return_if_fail (page >= 0);

    hbox = moo_notebook_get_tab_label (window->priv->notebook,
                                       GTK_WIDGET(doc)->parent);
    g_return_if_fail (GTK_IS_WIDGET (hbox));

    icon = g_object_get_data (G_OBJECT (hbox), "moo-edit-icon");
    label = g_object_get_data (G_OBJECT (hbox), "moo-edit-label");
    evbox = g_object_get_data (G_OBJECT (hbox), "moo-edit-icon-evbox");
    g_return_if_fail (GTK_IS_WIDGET (icon) && GTK_IS_WIDGET (label));
    g_return_if_fail (GTK_IS_WIDGET (evbox));

    status = moo_edit_get_status (doc);

    deleted = status & (MOO_EDIT_DELETED | MOO_EDIT_MODIFIED_ON_DISK);
    modified = (status & MOO_EDIT_MODIFIED) && !(status & MOO_EDIT_CLEAN);

    label_text = g_strdup_printf ("%s%s%s",
                                  deleted ? "!" : "",
                                  modified ? "*" : "",
                                  moo_edit_get_display_basename (doc));
    gtk_label_set_text (GTK_LABEL (label), label_text);

    /* XXX */
#ifndef __WIN32__
    pixbuf = moo_get_icon_for_path (moo_edit_get_filename (doc),
                                    icon, GTK_ICON_SIZE_MENU);
    gtk_image_set_from_pixbuf (GTK_IMAGE (icon), pixbuf);
#else
    gtk_image_set_from_stock (GTK_IMAGE (icon),
                              GTK_STOCK_FILE,
                              GTK_ICON_SIZE_MENU);
#endif

    g_free (label_text);
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
    moo_editor_close_doc (window->priv->editor, edit, TRUE);
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

    moo_editor_close_docs (window->priv->editor, list, TRUE);

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
    PaneParams *params;

    g_return_val_if_fail (MOO_IS_EDIT_WINDOW (window), FALSE);
    g_return_val_if_fail (user_id != NULL, FALSE);
    g_return_val_if_fail (GTK_IS_WIDGET (widget), FALSE);
    g_return_val_if_fail (label != NULL, FALSE);

    g_return_val_if_fail (moo_edit_window_get_pane (window, user_id) == NULL, FALSE);

    gtk_object_sink (g_object_ref (widget));

    params = load_pane_params (user_id);
    position = (params && params->position >= 0) ?
            (MooPanePosition) params->position : position;

    result = moo_big_paned_insert_pane (window->paned, widget, label,
                                        position, -1);

    if (result >= 0)
    {
        g_hash_table_insert (window->priv->panes,
                             g_strdup (user_id), widget);
        g_object_set_data_full (G_OBJECT (widget), "moo-edit-window-pane-user-id",
                                g_strdup (user_id), g_free);

        if (params)
            moo_big_paned_set_pane_params (window->paned, position, result, params->params);
    }

    pane_params_free (params);
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
#define PROP_PANE_POSITION              "position"
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


static PaneParams *
load_pane_params (const char         *pane_id)
{
    MooMarkupNode *node;
    PaneParams *params;

    node = get_pane_element (pane_id, FALSE);

    if (!node)
        return NULL;

    params = pane_params_new ();
    params->params = moo_pane_params_new ();

    params->params->window_position.x = moo_markup_get_int_prop (node, PROP_PANE_WINDOW_X, 0);
    params->params->window_position.y = moo_markup_get_int_prop (node, PROP_PANE_WINDOW_Y, 0);
    params->params->window_position.width = moo_markup_get_int_prop (node, PROP_PANE_WINDOW_WIDTH, 0);
    params->params->window_position.height = moo_markup_get_int_prop (node, PROP_PANE_WINDOW_HEIGHT, 0);
    params->params->detached = moo_markup_get_bool_prop (node, PROP_PANE_WINDOW_DETACHED, FALSE);
    params->params->maximized = moo_markup_get_bool_prop (node, PROP_PANE_WINDOW_MAXIMIZED, FALSE);
    params->params->keep_on_top = moo_markup_get_bool_prop (node, PROP_PANE_WINDOW_KEEP_ON_TOP, FALSE);

    params->position = moo_markup_get_int_prop (node, PROP_PANE_POSITION, -1);

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
save_pane_params (const char *pane_id,
                  PaneParams *params)
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

    set_if_not_zero (node, PROP_PANE_WINDOW_X, params->params->window_position.x);
    set_if_not_zero (node, PROP_PANE_WINDOW_Y, params->params->window_position.y);
    set_if_not_zero (node, PROP_PANE_WINDOW_WIDTH, params->params->window_position.width);
    set_if_not_zero (node, PROP_PANE_WINDOW_HEIGHT, params->params->window_position.height);
    set_if_not_false (node, PROP_PANE_WINDOW_DETACHED, params->params->detached);
    set_if_not_false (node, PROP_PANE_WINDOW_MAXIMIZED, params->params->maximized);
    set_if_not_false (node, PROP_PANE_WINDOW_KEEP_ON_TOP, params->params->keep_on_top);

    if (params->position >= 0)
        moo_markup_set_prop (node, PROP_PANE_POSITION,
                             moo_convert_int_to_string (params->position));
    else
        moo_markup_set_prop (node, PROP_PANE_POSITION, NULL);

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


static PaneParams *
pane_params_new (void)
{
    PaneParams *params = g_new0 (PaneParams, 1);
    params->position = -1;
    return params;
}


static void
pane_params_free (PaneParams *params)
{
    if (params)
    {
        moo_pane_params_free (params->params);
        g_free (params);
    }
}


static void
pane_params_changed (MooEditWindow      *window,
                     MooPanePosition     position,
                     guint               index)
{
    GtkWidget *widget;
    const char *id;
    PaneParams *params;

    widget = moo_big_paned_get_pane (window->paned, position, index);
    g_return_if_fail (widget != NULL);
    id = g_object_get_data (G_OBJECT (widget), "moo-edit-window-pane-user-id");

    if (id)
    {
        params = pane_params_new ();
        params->params = moo_big_paned_get_pane_params (window->paned, position, index);
        params->position = position;

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
clear_statusbar_message (MooEditWindow *window)
{
    moo_edit_window_pop_message (window, NULL);
}


static void
set_statusbar_numbers (MooEditWindow *window,
                       int            line,
                       int            column)
{
    char *text = g_strdup_printf ("Line: %d Col: %d", line, column);
    gtk_label_set_text (window->priv->cursor_label, text);
    clear_statusbar_message (window);
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
    gboolean ovr;

    edit = ACTIVE_DOC (window);

    if (!edit)
    {
        gtk_widget_set_sensitive (window->priv->info, FALSE);
        clear_statusbar_message (window);
        return;
    }

    gtk_widget_set_sensitive (window->priv->info, TRUE);

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (edit));
    gtk_text_buffer_get_iter_at_mark (buffer, &iter,
                                      gtk_text_buffer_get_insert (buffer));
    line = gtk_text_iter_get_line (&iter) + 1;
    column = moo_text_iter_get_visual_line_offset (&iter, 8) + 1;

    set_statusbar_numbers (window, line, column);

    ovr = gtk_text_view_get_overwrite (GTK_TEXT_VIEW (edit));
    gtk_label_set_text (window->priv->insert_label, ovr ? "OVR" : "INS");
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
        int column = moo_text_iter_get_visual_line_offset (iter, 8) + 1;
        set_statusbar_numbers (window, line, column);
    }
}


static void
create_statusbar (MooEditWindow *window)
{
    MooGladeXML *xml;
    GtkWidget *hbox;

    xml = moo_glade_xml_new_from_buf (STATUSBAR_GLADE_XML, -1, "hbox");
    hbox = moo_glade_xml_get_widget (xml, "hbox");
    g_return_if_fail (hbox != NULL);

    gtk_box_pack_start (GTK_BOX (MOO_WINDOW (window)->vbox),
                        hbox, FALSE, FALSE, 0);

    window->priv->statusbar = moo_glade_xml_get_widget (xml, "statusbar");

    window->priv->cursor_label = moo_glade_xml_get_widget (xml, "cursor");
    window->priv->insert_label = moo_glade_xml_get_widget (xml, "insert");
    window->priv->info = moo_glade_xml_get_widget (xml, "info");
}


guint
moo_edit_window_push_message (MooEditWindow  *window,
                              const char     *message,
                              const char     *id)
{
    guint ctx_id;

    g_return_val_if_fail (MOO_IS_EDIT_WINDOW (window), 0);
    g_return_val_if_fail (message != NULL, 0);

    ctx_id = id ? gtk_statusbar_get_context_id (window->priv->statusbar, id) : 0;
    return gtk_statusbar_push (window->priv->statusbar, ctx_id, message);
}


void
moo_edit_window_pop_message (MooEditWindow  *window,
                             const char     *id)
{
    guint ctx_id;

    g_return_if_fail (MOO_IS_EDIT_WINDOW (window));

    ctx_id = id ? gtk_statusbar_get_context_id (window->priv->statusbar, id) : 0;
    gtk_statusbar_pop (window->priv->statusbar, ctx_id);
}


void
moo_edit_window_message (MooEditWindow  *window,
                         const char     *message)
{
    g_return_if_fail (MOO_IS_EDIT_WINDOW (window));
    g_return_if_fail (message != NULL);
    moo_edit_window_pop_message (window, NULL);
    moo_edit_window_push_message (window, message, NULL);
}


/*****************************************************************************/
/* Language menu
 */

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

    old_val = moo_edit_config_get_string (doc->config, "lang");

    if (old_val)
        do_set = !lang_name || strcmp (old_val, lang_name);
    else
        do_set = !!lang_name;

    if (do_set)
        moo_edit_config_set (doc->config, "lang", 0, lang_name, NULL);
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
                         MOO_LANG_NONE, "None",
                         MOO_MENU_ITEM_RADIO, NULL, NULL);

    for (l = sections; l != NULL; l = l->next)
        moo_menu_mgr_append (menu_mgr, LANG_ACTION_ID,
                             l->data, l->data, 0, NULL, NULL);

    for (l = langs; l != NULL; l = l->next)
    {
        MooLang *lang = l->data;
        if (!lang->hidden)
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
                             moo_lang_id (lang), TRUE);
}


static void
edit_lang_changed (MooEditWindow      *window,
                   G_GNUC_UNUSED guint var_id,
                   G_GNUC_UNUSED GParamSpec *pspec,
                   MooEdit            *doc)
{
    if (doc == ACTIVE_DOC (window))
    {
        update_lang_menu (window);
        moo_edit_window_check_actions (window);
    }
}


/*****************************************************************************/
/* Action properties checks
 */

static void
moo_edit_window_check_action (MooEdit       *doc,
                              MooAction     *action,
                              ActionCheck   *check)
{
    gpointer klass;
    GParamSpec *pspec;
    GValue value;

    klass = G_OBJECT_GET_CLASS (action);
    pspec = g_object_class_find_property (klass, check->property);
    g_return_if_fail (pspec != NULL);

    value.g_type = 0;
    g_value_init (&value, pspec->value_type);

    check->func (action, doc, pspec, &value, check->data);
    g_object_set_property (G_OBJECT (action), check->property, &value);

    g_value_unset (&value);
}


static void
window_check_actions (const char    *action_id,
                      GSList        *checks,
                      MooEditWindow *window)
{
    MooAction *action;
    MooEdit *doc;

    action = moo_window_get_action_by_id (MOO_WINDOW (window), action_id);

    if (!action)
        return;

    doc = ACTIVE_DOC (window);

    while (checks)
    {
        moo_edit_window_check_action (doc, action, checks->data);
        checks = checks->next;
    }
}


static void
moo_edit_window_check_actions (MooEditWindow *window)
{
    g_hash_table_foreach (action_checks,
                          (GHFunc) window_check_actions,
                          window);
}


static void
check_action (const char  *action_id,
              ActionCheck *check)
{
    GSList *l;

    for (l = windows; l != NULL; l = l->next)
    {
        MooEditWindow *window = l->data;
        MooEdit *doc = ACTIVE_DOC (window);
        MooAction *action = moo_window_get_action_by_id (MOO_WINDOW (window), action_id);
        if (action)
            moo_edit_window_check_action (doc, action, check);
    }
}


static int
check_and_prop_cmp (ActionCheck *check,
                    const char  *prop)
{
    return strcmp (check->property, prop);
}

void
moo_edit_window_add_action_check (const char     *action_id,
                                  const char     *action_prop,
                                  MooEditWindowCheckActionFunc func,
                                  gpointer        data,
                                  GDestroyNotify  notify)
{
    ActionCheck *check;
    GSList *list;

    g_return_if_fail (action_id != NULL);
    g_return_if_fail (action_prop != NULL);
    g_return_if_fail (func != NULL);

    action_checks_init ();

    list = g_hash_table_lookup (action_checks, action_id);

    if (list)
    {
        GSList *old = g_slist_find_custom (list, action_prop,
                                           (GCompareFunc) check_and_prop_cmp);

        if (old)
        {
            action_check_free (old->data);
            list = g_slist_delete_link (list, old);
        }
    }

    check = action_check_new (action_prop, func, data, notify);
    list = g_slist_prepend (list, check);
    g_hash_table_insert (action_checks, g_strdup (action_id), list);

    check_action (action_id, check);
}


void
moo_edit_window_remove_action_check (const char *action_id,
                                     const char *action_prop)
{
    GSList *list;

    g_return_if_fail (action_id != NULL);
    g_return_if_fail (action_prop != NULL);

    action_checks_init ();

    list = g_hash_table_lookup (action_checks, action_id);

    if (list)
    {
        GSList *old = g_slist_find_custom (list, action_prop,
                                           (GCompareFunc) check_and_prop_cmp);

        if (old)
        {
            action_check_free (old->data);
            list = g_slist_delete_link (list, old);
        }

        if (!list)
            g_hash_table_remove (action_checks, action_id);
    }
}


static ActionCheck *
action_check_new (const char     *action_prop,
                  MooEditWindowCheckActionFunc func,
                  gpointer        data,
                  GDestroyNotify  notify)
{
    ActionCheck *check = g_new0 (ActionCheck, 1);
    check->property = g_strdup (action_prop);
    check->func = func;
    check->data = data;
    check->notify = notify;
    return check;
}


static void
action_check_free (ActionCheck *check)
{
    if (check)
    {
        if (check->notify)
            check->notify (check->data);
        g_free (check->property);
        g_free (check);
    }
}


static void
action_checks_init (void)
{
    if (!action_checks)
        action_checks =
                g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
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


GtkWidget *
moo_edit_window_get_output (MooEditWindow *window)
{
    MooPaneLabel *label;
    GtkWidget *cmd_view;
    GtkWidget *scrolled_window;

    g_return_val_if_fail (MOO_IS_EDIT_WINDOW (window), NULL);

    scrolled_window = moo_edit_window_get_pane (window, "moo-edit-window-output");

    if (!scrolled_window)
    {
        scrolled_window = gtk_scrolled_window_new (NULL, NULL);
        gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                        GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

        cmd_view = moo_cmd_view_new ();
        gtk_container_add (GTK_CONTAINER (scrolled_window), cmd_view);
        gtk_widget_show_all (scrolled_window);
        g_object_set_data (G_OBJECT (scrolled_window), "moo-output", cmd_view);

        label = moo_pane_label_new (MOO_STOCK_TERMINAL, NULL, NULL, "Output", "Output");

        if (!moo_edit_window_add_pane (window, "moo-edit-window-output",
                                       scrolled_window, label, MOO_PANE_POS_BOTTOM))
        {
            g_critical ("%s: oops", G_STRLOC);
            moo_pane_label_free (label);
            return NULL;
        }

        moo_edit_window_add_stop_client (window, cmd_view);

        moo_pane_label_free (label);
        return cmd_view;
    }

    return g_object_get_data (G_OBJECT (scrolled_window), "moo-output");
}


GtkWidget *
moo_edit_window_get_output_pane (MooEditWindow *window)
{
    return moo_edit_window_get_pane (window, "moo-edit-window-output");
}


/************************************************************************/
/* Drag into the window
 */

static gboolean
notebook_drag_motion (GtkWidget          *widget,
                      GdkDragContext     *context,
                      G_GNUC_UNUSED int   x,
                      G_GNUC_UNUSED int   y,
                      guint               time,
                      G_GNUC_UNUSED MooEditWindow *window)
{
    GdkAtom target;

    target = gtk_drag_dest_find_target (widget, context, NULL);

    if (target == GDK_NONE)
        gdk_drag_status (context, 0, time);
    else if (target == moo_edit_tab_atom)
        gtk_drag_get_data (widget, context, moo_edit_tab_atom, time);
    else
        gdk_drag_status (context, context->suggested_action, time);

    return TRUE;
}


static gboolean
notebook_drag_drop (GtkWidget          *widget,
                    GdkDragContext     *context,
                    G_GNUC_UNUSED int   x,
                    G_GNUC_UNUSED int   y,
                    guint               time,
                    G_GNUC_UNUSED MooEditWindow *window)
{
    GdkAtom target;

    target = gtk_drag_dest_find_target (widget, context, NULL);

    if (target == GDK_NONE)
    {
        gtk_drag_finish (context, FALSE, FALSE, time);
    }
    else
    {
        g_object_set_data (G_OBJECT (widget), "moo-edit-window-drop",
                           GINT_TO_POINTER (TRUE));
        gtk_drag_get_data (widget, context, target, time);
    }

    return TRUE;

}


static void
notebook_drag_data_recv (GtkWidget          *widget,
                         GdkDragContext     *context,
                         G_GNUC_UNUSED int   x,
                         G_GNUC_UNUSED int   y,
                         GtkSelectionData   *data,
                         guint               info,
                         guint               time,
                         MooEditWindow      *window)
{
    if (g_object_get_data (G_OBJECT (widget), "moo-edit-window-drop"))
    {
        char **uris;

        g_object_set_data (G_OBJECT (widget), "moo-edit-window-drop", NULL);

        if (data->target == moo_edit_tab_atom)
        {
            GtkWidget *toplevel;
            MooEdit *doc = moo_selection_data_get_pointer (data, moo_edit_tab_atom);

            if (!doc)
                goto out;

            toplevel = gtk_widget_get_toplevel (GTK_WIDGET (doc));

            if (toplevel == GTK_WIDGET (window))
                goto out;

            g_print ("%s: implement me\n", G_STRLOC);
            goto out;
        }
        else if ((uris = gtk_selection_data_get_uris (data)))
        {
            char **u;

            if (!uris)
                goto out;

            for (u = uris; *u; ++u)
                moo_editor_open_uri (window->priv->editor, window,
                                     NULL, *u, NULL);

            g_strfreev (uris);
            gtk_drag_finish (context, TRUE, FALSE, time);
        }
        else
        {
            MooEdit *doc;
            GtkTextBuffer *buf;
            char *text = (char *) gtk_selection_data_get_text (data);

            if (!text)
                goto out;

            doc = moo_editor_new_doc (window->priv->editor, window);

            if (!doc)
            {
                g_free (text);
                goto out;
            }

            /* XXX */
            buf = gtk_text_view_get_buffer (GTK_TEXT_VIEW (doc));
            gtk_text_buffer_set_text (buf, text, -1);

            g_free (text);
            gtk_drag_finish (context, TRUE,
                             context->suggested_action == GDK_ACTION_MOVE,
                             time);
        }
    }
    else
    {
        if (info == TARGET_MOO_EDIT_TAB)
        {
            GtkWidget *toplevel;
            MooEdit *doc = moo_selection_data_get_pointer (data, moo_edit_tab_atom);

            if (!doc)
                return gdk_drag_status (context, 0, time);

            toplevel = gtk_widget_get_toplevel (GTK_WIDGET (doc));

            if (toplevel == GTK_WIDGET (window))
                return gdk_drag_status (context, 0, time);

            gdk_drag_status (context, GDK_ACTION_MOVE, time);
        }
        else
        {
            gdk_drag_status (context, 0, time);
        }

        return;
    }

out:
    gtk_drag_finish (context, FALSE, FALSE, time);
}
