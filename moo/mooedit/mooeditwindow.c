/*
 *   mooeditwindow.c
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
#include "mooedit/mooedit-accels.h"
#include "mooedit/mooeditor-private.h"
#include "mooedit/mooeditfiltersettings.h"
#include "mooedit/moolang.h"
#include "mooedit/mootextbuffer.h"
#include "mooedit/mooeditprefs.h"
#include "mooedit/mooplugin.h"
#include "mooedit/moocmdview.h"
#include "mooedit/mooeditaction.h"
#include "mooedit/mooedit-bookmarks.h"
#include "mooedit/moolangmgr.h"
#include "mooutils/moonotebook.h"
#include "mooutils/moostock.h"
#include "marshals.h"
#include "mooutils/moomenuaction.h"
#include "mooutils/mooutils-misc.h"
#include "mooutils/moodialogs.h"
#include "mooutils/moocompat.h"
#include "mooutils/mooi18n.h"
#include "mooutils/mooaction-private.h"
#include "mooutils/moofiledialog.h"
#include "mooutils/mooencodings.h"
#include "mooutils/mooatom.h"
#include "glade/moostatusbar-gxml.h"
#include <string.h>
#include <gtk/gtk.h>
#include <math.h>

#if GTK_CHECK_VERSION(2,10,0)
#define ENABLE_PRINTING
#include "mooedit/mootextprint.h"
#endif

#define ACTIVE_DOC moo_edit_window_get_active_doc

#define LANG_ACTION_ID "LanguageMenu"
#define STOP_ACTION_ID "StopJob"

#define DOCUMENT_ACTION "SwitchToTab"

#define DEFAULT_TITLE_FORMAT        "%a - %f%s"
#define DEFAULT_TITLE_FORMAT_NO_DOC "%a"

typedef struct {
    MooActionCheckFunc func;
    gpointer data;
    GDestroyNotify notify;
} OnePropCheck;

#define N_ACTION_CHECKS 3
typedef struct {
    OnePropCheck checks[N_ACTION_CHECKS];
} ActionCheck;

static GHashTable *action_checks; /* char* -> ActionCheck* */
static GSList *windows;

struct MooEditWindowPrivate {
    MooEditor *editor;

    guint statusbar_idle;
    guint last_msg_id;
    GtkLabel *cursor_label;
    GtkLabel *chars_label;
    GtkLabel *insert_label;
    GtkWidget *info;

    MooNotebook *notebook;
    guint save_params_idle;

    char *title_format;
    char *title_format_no_doc;

    GSList *stop_clients;
    GSList *jobs; /* Job* */

    GList *history;
    guint history_blocked : 1;
};

enum {
    TARGET_MOO_EDIT_TAB = 1,
    TARGET_URI_LIST = 2
};

#define MOO_EDIT_TAB_ATOM (moo_edit_tab_atom ())
MOO_DEFINE_ATOM (MOO_EDIT_TAB, moo_edit_tab)

static GtkTargetEntry dest_targets[] = {
    {(char*) "MOO_EDIT_TAB", GTK_TARGET_SAME_APP, TARGET_MOO_EDIT_TAB},
    {(char*) "text/uri-list", 0, TARGET_URI_LIST}
};


static void     action_checks_init              (void);
static void     moo_edit_window_check_actions   (MooEditWindow      *window);

static GObject *moo_edit_window_constructor     (GType               type,
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


static gboolean moo_edit_window_close           (MooWindow          *window);

static void     setup_notebook                  (MooEditWindow      *window);
static void     update_window_title             (MooEditWindow      *window);
static void     set_title_format_from_prefs     (MooEditWindow      *window);

static void     proxy_boolean_property          (MooEditWindow      *window,
                                                 GParamSpec         *prop,
                                                 MooEdit            *doc);
static void     edit_changed                    (MooEditWindow      *window,
                                                 MooEdit            *doc);
static void     edit_filename_changed           (MooEditWindow      *window,
                                                 const char         *filename,
                                                 MooEdit            *doc);
static void     edit_encoding_changed           (MooEditWindow      *window,
                                                 GParamSpec         *pspec,
                                                 MooEdit            *doc);
static void     edit_line_end_changed           (MooEditWindow      *window,
                                                 GParamSpec         *pspec,
                                                 MooEdit            *doc);
static void     edit_lang_changed               (MooEditWindow      *window,
                                                 guint               var_id,
                                                 GParamSpec         *pspec,
                                                 MooEdit            *doc);
static void     edit_overwrite_changed          (MooEditWindow      *window,
                                                 GParamSpec         *pspec,
                                                 MooEdit            *doc);
static void     edit_wrap_mode_changed          (MooEditWindow      *window,
                                                 GParamSpec         *pspec,
                                                 MooEdit            *doc);
static void     edit_show_line_numbers_changed  (MooEditWindow      *window,
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
static void     update_doc_view_actions         (MooEditWindow      *window);

static void     create_statusbar                (MooEditWindow      *window);
static void     update_statusbar                (MooEditWindow      *window);
static MooEdit *get_nth_tab                     (MooEditWindow      *window,
                                                 guint               n);
static int      get_page_num                    (MooEditWindow      *window,
                                                 MooEdit            *doc);

static GtkAction *create_lang_action            (MooEditWindow      *window);

static void     create_paned                    (MooEditWindow      *window);
static gboolean save_paned_config               (MooEditWindow      *window);

static void     moo_edit_window_connect_menubar (MooWindow          *window);
static void     moo_edit_window_update_doc_list (MooEditWindow      *window);
static void     window_menu_item_selected       (MooWindow          *window,
                                                 GtkMenuItem        *item);

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
static void action_new_doc                      (MooEditWindow      *window);
static void action_open                         (MooEditWindow      *window);
static void action_reload                       (MooEditWindow      *window);
static GtkAction *create_reopen_with_encoding_action (MooEditWindow *window);
static GtkAction *create_doc_encoding_action    (MooEditWindow      *window);
static GtkAction *create_doc_line_end_action    (MooEditWindow      *window);
static void action_save                         (MooEditWindow      *window);
static void action_save_as                      (MooEditWindow      *window);
static void action_close_tab                    (MooEditWindow      *window);
static void action_close_all                    (MooEditWindow      *window);
static void action_previous_tab                 (MooEditWindow      *window);
static void action_next_tab                     (MooEditWindow      *window);
static void action_switch_to_tab                (MooEditWindow      *window,
                                                 guint               n);

static void action_toggle_bookmark              (MooEditWindow      *window);
static void action_next_bookmark                (MooEditWindow      *window);
static void action_prev_bookmark                (MooEditWindow      *window);
static GtkAction *create_goto_bookmark_action   (MooWindow          *window,
                                                 gpointer            data);

static void action_find_now_f                   (MooEditWindow      *window);
static void action_find_now_b                   (MooEditWindow      *window);
static void action_abort_jobs                   (MooEditWindow      *window);

static void wrap_text_toggled                   (MooEditWindow      *window,
                                                 gboolean            active);
static void line_numbers_toggled                (MooEditWindow      *window,
                                                 gboolean            active);


#ifdef ENABLE_PRINTING
static void action_page_setup      (MooEditWindow    *window);
static void action_print           (MooEditWindow    *window);
static void action_print_preview   (MooEditWindow    *window);
static void action_print_pdf       (MooEditWindow    *window);
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
    PROP_HAS_COMMENTS,
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
    guint i;
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GtkObjectClass *gtkobject_class = GTK_OBJECT_CLASS (klass);
    MooWindowClass *window_class = MOO_WINDOW_CLASS (klass);

    action_checks_init ();

    gobject_class->constructor = moo_edit_window_constructor;
    gobject_class->finalize = moo_edit_window_finalize;
    gobject_class->set_property = moo_edit_window_set_property;
    gobject_class->get_property = moo_edit_window_get_property;
    gtkobject_class->destroy = moo_edit_window_destroy;
    window_class->close = moo_edit_window_close;

    g_type_class_add_private (klass, sizeof (MooEditWindowPrivate));

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
    INSTALL_PROP (PROP_HAS_COMMENTS, "has-comments");
    INSTALL_PROP (PROP_HAS_JOBS_RUNNING, "has-jobs-running");
    INSTALL_PROP (PROP_HAS_STOP_CLIENTS, "has-stop-clients");

    moo_window_class_set_id (window_class, "Editor", "Editor");

    moo_window_class_new_action (window_class, "NewDoc", NULL,
                                 "display-name", GTK_STOCK_NEW,
                                 "label", GTK_STOCK_NEW,
                                 "tooltip", _("Create new document"),
                                 "stock-id", GTK_STOCK_NEW,
                                 "default-accel", MOO_EDIT_ACCEL_NEW,
                                 "closure-callback", action_new_doc,
                                 NULL);

    moo_window_class_new_action (window_class, "Open", NULL,
                                 "display-name", GTK_STOCK_OPEN,
                                 "label", _("_Open..."),
                                 "tooltip", _("Open..."),
                                 "stock-id", GTK_STOCK_OPEN,
                                 "default-accel", MOO_EDIT_ACCEL_OPEN,
                                 "closure-callback", action_open,
                                 NULL);

    moo_window_class_new_action (window_class, "Reload", NULL,
                                 "display-name", _("Reload"),
                                 "label", _("_Reload"),
                                 "tooltip", _("Reload document"),
                                 "stock-id", GTK_STOCK_REFRESH,
                                 "default-accel", MOO_EDIT_ACCEL_RELOAD,
                                 "closure-callback", action_reload,
                                 "condition::sensitive", "can-reload",
                                 NULL);

    moo_window_class_new_action_custom (window_class, "ReopenWithEncoding", NULL,
                                        (MooWindowActionFunc) create_reopen_with_encoding_action,
                                        NULL, NULL);

    moo_window_class_new_action_custom (window_class, "EncodingMenu", NULL,
                                        (MooWindowActionFunc) create_doc_encoding_action,
                                        NULL, NULL);

    moo_window_class_new_action_custom (window_class, "LineEndMenu", NULL,
                                        (MooWindowActionFunc) create_doc_line_end_action,
                                        NULL, NULL);

    moo_window_class_new_action (window_class, "Save", NULL,
                                 "display-name", GTK_STOCK_SAVE,
                                 "label", GTK_STOCK_SAVE,
                                 "tooltip", GTK_STOCK_SAVE,
                                 "stock-id", GTK_STOCK_SAVE,
                                 "default-accel", MOO_EDIT_ACCEL_SAVE,
                                 "closure-callback", action_save,
                                 "condition::sensitive", "has-open-document",
                                 NULL);

    moo_window_class_new_action (window_class, "SaveAs", NULL,
                                 "display-name", GTK_STOCK_SAVE_AS,
                                 "label", _("Save _As..."),
                                 "tooltip", _("Save as..."),
                                 "stock-id", GTK_STOCK_SAVE_AS,
                                 "default-accel", MOO_EDIT_ACCEL_SAVE_AS,
                                 "closure-callback", action_save_as,
                                 "condition::sensitive", "has-open-document",
                                 NULL);

    moo_window_class_new_action (window_class, "Close", NULL,
                                 "display-name", GTK_STOCK_CLOSE,
                                 "label", GTK_STOCK_CLOSE,
                                 "tooltip", _("Close document"),
                                 "stock-id", GTK_STOCK_CLOSE,
                                 "default-accel", MOO_EDIT_ACCEL_CLOSE,
                                 "closure-callback", action_close_tab,
                                 "condition::sensitive", "has-open-document",
                                 NULL);

    moo_window_class_new_action (window_class, "CloseAll", NULL,
                                 "display-name", _("Close All"),
                                 "label", _("Close A_ll"),
                                 "tooltip", _("Close all documents"),
                                 "default-accel", MOO_EDIT_ACCEL_CLOSE_ALL,
                                 "closure-callback", action_close_all,
                                 "condition::sensitive", "has-open-document",
                                 NULL);

    moo_window_class_new_action (window_class, "PreviousTab", NULL,
                                 "display-name", _("Previous Tab"),
                                 "label", _("_Previous Tab"),
                                 "tooltip", _("Previous tab"),
                                 "stock-id", GTK_STOCK_GO_BACK,
                                 "default-accel", MOO_EDIT_ACCEL_PREV_TAB,
                                 "closure-callback", action_previous_tab,
                                 "condition::sensitive", "has-open-document",
                                 NULL);

    moo_window_class_new_action (window_class, "NextTab", NULL,
                                 "display-name", _("Next Tab"),
                                 "label", _("_Next Tab"),
                                 "tooltip", _("Next tab"),
                                 "stock-id", GTK_STOCK_GO_FORWARD,
                                 "default-accel", MOO_EDIT_ACCEL_NEXT_TAB,
                                 "closure-callback", action_next_tab,
                                 "condition::sensitive", "has-open-document",
                                 NULL);

    moo_window_class_new_action (window_class, "Find", NULL,
                                 "display-name", GTK_STOCK_FIND,
                                 "label", GTK_STOCK_FIND,
                                 "tooltip", GTK_STOCK_FIND,
                                 "stock-id", GTK_STOCK_FIND,
                                 "default-accel", MOO_EDIT_ACCEL_FIND,
                                 "closure-signal", "find-interactive",
                                 "closure-proxy-func", moo_edit_window_get_active_doc,
                                 "condition::sensitive", "has-open-document",
                                 NULL);

    moo_window_class_new_action (window_class, "FindNext", NULL,
                                 "display-name", _("Find Next"),
                                 "label", _("Find _Next"),
                                 "tooltip", _("Find next"),
                                 "stock-id", GTK_STOCK_GO_FORWARD,
                                 "default-accel", MOO_EDIT_ACCEL_FIND_NEXT,
                                 "closure-signal", "find-next-interactive",
                                 "closure-proxy-func", moo_edit_window_get_active_doc,
                                 "condition::sensitive", "has-open-document",
                                 NULL);

    moo_window_class_new_action (window_class, "FindPrevious", NULL,
                                 "display-name", _("Find Previous"),
                                 "label", _("Find _Previous"),
                                 "tooltip", _("Find previous"),
                                 "stock-id", GTK_STOCK_GO_BACK,
                                 "default-accel", MOO_EDIT_ACCEL_FIND_PREV,
                                 "closure-signal", "find-prev-interactive",
                                 "closure-proxy-func", moo_edit_window_get_active_doc,
                                 "condition::sensitive", "has-open-document",
                                 NULL);

    moo_window_class_new_action (window_class, "Replace", NULL,
                                 "display-name", GTK_STOCK_FIND_AND_REPLACE,
                                 "label", GTK_STOCK_FIND_AND_REPLACE,
                                 "tooltip", GTK_STOCK_FIND_AND_REPLACE,
                                 "stock-id", GTK_STOCK_FIND_AND_REPLACE,
                                 "default-accel", MOO_EDIT_ACCEL_REPLACE,
                                 "closure-signal", "replace-interactive",
                                 "closure-proxy-func", moo_edit_window_get_active_doc,
                                 "condition::sensitive", "has-open-document",
                                 NULL);

    moo_window_class_new_action (window_class, "FindCurrent", NULL,
                                 "display-name", _("Find Current Word"),
                                 "label", _("Find Current _Word"),
                                 "stock-id", GTK_STOCK_FIND,
                                 "default-accel", MOO_EDIT_ACCEL_FIND_CURRENT,
                                 "closure-callback", action_find_now_f,
                                 "condition::sensitive", "has-open-document",
                                 NULL);

    moo_window_class_new_action (window_class, "FindCurrentBack", NULL,
                                 "display-name", _("Find Current Word Backwards"),
                                 "label", _("Find Current Word _Backwards"),
                                 "stock-id", GTK_STOCK_FIND,
                                 "default-accel", MOO_EDIT_ACCEL_FIND_CURRENT_BACK,
                                 "closure-callback", action_find_now_b,
                                 "condition::sensitive", "has-open-document",
                                 NULL);

    moo_window_class_new_action (window_class, "GoToLine", NULL,
                                 "display-name", _("Go to Line"),
                                 "label", _("_Go to Line..."),
                                 "tooltip", _("Go to line..."),
                                 "default-accel", MOO_EDIT_ACCEL_GOTO_LINE,
                                 "closure-signal", "goto-line-interactive",
                                 "closure-proxy-func", moo_edit_window_get_active_doc,
                                 "condition::sensitive", "has-open-document",
                                 NULL);

    moo_window_class_new_action (window_class, "WrapText", NULL,
                                 "action-type::", MOO_TYPE_TOGGLE_ACTION,
                                 "display-name", _("Toggle Text Wrapping"),
                                 "label", _("_Wrap Text"),
                                 "toggled-callback", wrap_text_toggled,
                                 "condition::sensitive", "has-open-document",
                                 NULL);

    moo_window_class_new_action (window_class, "LineNumbers", NULL,
                                 "action-type::", MOO_TYPE_TOGGLE_ACTION,
                                 "display-name", _("Toggle Line Numbers Display"),
                                 "label", _("_Show Line Numbers"),
                                 "toggled-callback", line_numbers_toggled,
                                 "condition::sensitive", "has-open-document",
                                 NULL);

    moo_window_class_new_action (window_class, "FocusDoc", NULL,
                                 "display-name", _("Focus Document"),
                                 "label", _("_Focus Document"),
                                 "default-accel", MOO_EDIT_ACCEL_FOCUS_DOC,
                                 "closure-callback", gtk_widget_grab_focus,
                                 "closure-proxy-func", moo_edit_window_get_active_doc,
                                 "condition::sensitive", "has-open-document",
                                 NULL);

    moo_window_class_new_action (window_class, STOP_ACTION_ID, NULL,
                                 "display-name", GTK_STOCK_STOP,
                                 "label", GTK_STOCK_STOP,
                                 "tooltip", GTK_STOCK_STOP,
                                 "stock-id", GTK_STOCK_STOP,
                                 "default-accel", MOO_EDIT_ACCEL_STOP,
                                 "closure-callback", action_abort_jobs,
                                 "condition::sensitive", "has-jobs-running",
                                 "condition::visible", "has-stop-clients",
                                 NULL);

    for (i = 1; i < 10; ++i)
    {
        char *action_id = g_strdup_printf (DOCUMENT_ACTION "%u", i);
        char *accel = g_strdup_printf (MOO_EDIT_ACCEL_SWITCH_TO_TAB "%u", i);
        _moo_window_class_new_action_callback (window_class, action_id, NULL,
                                               G_CALLBACK (action_switch_to_tab),
                                               _moo_marshal_VOID__UINT,
                                               G_TYPE_NONE, 1,
                                               G_TYPE_UINT, i - 1,
                                               "default-accel", accel,
                                               "connect-accel", TRUE,
                                               "accel-editable", FALSE,
                                               NULL);
        g_free (accel);
        g_free (action_id);
    }

    for (i = 1; i < 10; ++i)
    {
        char *action_id = g_strdup_printf (MOO_EDIT_GOTO_BOOKMARK_ACTION "%u", i);
        moo_window_class_new_action_custom (window_class, action_id, NULL,
                                            create_goto_bookmark_action,
                                            GUINT_TO_POINTER (i),
                                            NULL);
        g_free (action_id);
    }

    moo_window_class_new_action (window_class, "ToggleBookmark", NULL,
                                 "display-name", _("Toggle Bookmark"),
                                 "label", _("Toggle _Bookmark"),
                                 "stock-id", MOO_STOCK_EDIT_BOOKMARK,
                                 "default-accel", MOO_EDIT_ACCEL_BOOKMARK,
                                 "closure-callback", action_toggle_bookmark,
                                 "condition::sensitive", "has-open-document",
                                 NULL);

    moo_window_class_new_action (window_class, "NextBookmark", NULL,
                                 "display-name", _("Next Bookmark"),
                                 "label", _("_Next Bookmark"),
                                 "default-accel", MOO_EDIT_ACCEL_NEXT_BOOKMARK,
                                 "connect-accel", TRUE,
                                 "closure-callback", action_next_bookmark,
                                 NULL);

    moo_window_class_new_action (window_class, "PreviousBookmark", NULL,
                                 "display-name", _("Previous Bookmark"),
                                 "label", _("_Previous Bookmark"),
                                 "default-accel", MOO_EDIT_ACCEL_PREV_BOOKMARK,
                                 "connect-accel", TRUE,
                                 "closure-callback", action_prev_bookmark,
                                 NULL);

    moo_window_class_new_action (window_class, "Comment", NULL,
                                 /* action */
                                 "display-name", _("Comment"),
                                 "label", _("Comment"),
                                 "tooltip", _("Comment"),
                                 "closure-callback", moo_edit_comment,
                                 "closure-proxy-func", moo_edit_window_get_active_doc,
                                 "condition::sensitive", "has-comments",
                                 NULL);

    moo_window_class_new_action (window_class, "Uncomment", NULL,
                                 /* action */
                                 "display-name", _("Uncomment"),
                                 "label", _("Uncomment"),
                                 "tooltip", _("Uncomment"),
                                 "closure-callback", moo_edit_uncomment,
                                 "closure-proxy-func", moo_edit_window_get_active_doc,
                                 "condition::sensitive", "has-comments",
                                 NULL);

    moo_window_class_new_action (window_class, "Indent", NULL,
                                 "display-name", GTK_STOCK_INDENT,
                                 "label", GTK_STOCK_INDENT,
                                 "tooltip", GTK_STOCK_INDENT,
                                 "stock-id", GTK_STOCK_INDENT,
                                 "closure-callback", moo_text_view_indent,
                                 "closure-proxy-func", moo_edit_window_get_active_doc,
                                 "condition::sensitive", "has-open-document",
                                 NULL);

    moo_window_class_new_action (window_class, "Unindent", NULL,
                                 "display-name", GTK_STOCK_UNINDENT,
                                 "label", GTK_STOCK_UNINDENT,
                                 "tooltip", GTK_STOCK_UNINDENT,
                                 "stock-id", GTK_STOCK_UNINDENT,
                                 "closure-callback", moo_text_view_unindent,
                                 "closure-proxy-func", moo_edit_window_get_active_doc,
                                 "condition::sensitive", "has-open-document",
                                 NULL);

    moo_window_class_new_action (window_class, "NoDocuments", NULL,
                                 /* Insensitive menu item which appears in Window menu with no documents open */
                                 "label", _("No Documents"),
                                 "no-accel", TRUE,
                                 "sensitive", FALSE,
                                 "condition::visible", "!has-open-document",
                                 NULL);

#ifdef ENABLE_PRINTING
    moo_window_class_new_action (window_class, "PageSetup", NULL,
                                 "display-name", _("Page Setup"),
                                 "label", _("Page S_etup..."),
                                 "tooltip", _("Page Setup..."),
                                 "default-accel", MOO_EDIT_ACCEL_PAGE_SETUP,
                                 "closure-callback", action_page_setup,
                                 NULL);

    moo_window_class_new_action (window_class, "PrintPreview", NULL,
                                 "display-name", GTK_STOCK_PRINT_PREVIEW,
                                 "label", GTK_STOCK_PRINT_PREVIEW,
                                 "tooltip", GTK_STOCK_PRINT_PREVIEW,
                                 "stock-id", GTK_STOCK_PRINT_PREVIEW,
                                 "closure-callback", action_print_preview,
                                 "condition::sensitive", "has-open-document",
                                 NULL);

    moo_window_class_new_action (window_class, "Print", NULL,
                                 "display-name", GTK_STOCK_PRINT,
                                 "label", _("Print..."),
                                 "tooltip", _("Print..."),
                                 "default-accel", MOO_EDIT_ACCEL_PRINT,
                                 "stock-id", GTK_STOCK_PRINT,
                                 "closure-callback", action_print,
                                 "condition::sensitive", "has-open-document",
                                 NULL);

    moo_window_class_new_action (window_class, "PrintPdf", NULL,
                                 "display-name", _("Export as PDF"),
                                 "label", _("E_xport as PDF..."),
                                 "tooltip", _("Export as PDF..."),
                                 "stock-id", GTK_STOCK_PRINT,
                                 "closure-callback", action_print_pdf,
                                 "condition::sensitive", "has-open-document",
                                 NULL);
#endif

    moo_window_class_new_action_custom (window_class, LANG_ACTION_ID, NULL,
                                        (MooWindowActionFunc) create_lang_action,
                                        NULL, NULL);
}


static void
moo_edit_window_init (MooEditWindow *window)
{
    window->priv = G_TYPE_INSTANCE_GET_PRIVATE (window, MOO_TYPE_EDIT_WINDOW, MooEditWindowPrivate);
    window->priv->history = NULL;
    window->priv->history_blocked = FALSE;

    g_object_set (G_OBJECT (window),
                  "menubar-ui-name", "Editor/Menubar",
                  "toolbar-ui-name", "Editor/Toolbar",
                  NULL);

    set_title_format_from_prefs (window);

    windows = g_slist_prepend (windows, window);
}


MooEditor *
moo_edit_window_get_editor (MooEditWindow *window)
{
    g_return_val_if_fail (MOO_IS_EDIT_WINDOW (window), NULL);
    return window->priv->editor;
}


static void
moo_edit_window_destroy (GtkObject *object)
{
    MooEditWindow *window = MOO_EDIT_WINDOW (object);

    if (window->priv->save_params_idle)
    {
        g_source_remove (window->priv->save_params_idle);
        window->priv->save_params_idle = 0;
    }

    if (window->priv->statusbar_idle)
    {
        g_source_remove (window->priv->statusbar_idle);
        window->priv->statusbar_idle = 0;
    }

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


static void
moo_edit_window_finalize (GObject *object)
{
    MooEditWindow *window = MOO_EDIT_WINDOW (object);

    g_free (window->priv->title_format);
    g_free (window->priv->title_format_no_doc);

    G_OBJECT_CLASS (moo_edit_window_parent_class)->finalize (object);
}


static void
moo_edit_window_set_property (GObject        *object,
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
            g_value_set_object (value, ACTIVE_DOC (window));
            break;

        case PROP_CAN_RELOAD:
            doc = ACTIVE_DOC (window);
            g_value_set_boolean (value, doc && !moo_edit_is_untitled (doc));
            break;
        case PROP_HAS_OPEN_DOCUMENT:
            g_value_set_boolean (value, window->priv->notebook != NULL &&
                                        moo_notebook_get_n_pages (window->priv->notebook) != 0);
            break;
        case PROP_HAS_COMMENTS:
            doc = ACTIVE_DOC (window);
            g_value_set_boolean (value, doc && _moo_edit_has_comments (doc, NULL, NULL));
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


/****************************************************************************/
/* Constructing
 */

static GObject *
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
    g_signal_connect (window, "notify::menubar",
                      G_CALLBACK (moo_edit_window_connect_menubar), NULL);

    edit_changed (window, NULL);
    moo_edit_window_check_actions (window);

    return object;
}


static char *
get_doc_status_string (MooEdit *doc)
{
    MooEditStatus status;
    gboolean modified;

    status = moo_edit_get_status (doc);
    modified = (status & MOO_EDIT_MODIFIED) && !(status & MOO_EDIT_CLEAN);

    if (!modified)
    {
        if (status & MOO_EDIT_NEW)
            /* Translators: this goes into window title, %s stands for filename, e.g. "/foo/bar.txt [new file]" */
            return g_strdup_printf (_("%s [new file]"), "");
        else if (status & MOO_EDIT_MODIFIED_ON_DISK)
            /* Translators: this goes into window title, %s stands for filename, e.g. "/foo/bar.txt [modified on disk]" */
            return g_strdup_printf (_("%s [modified on disk]"), "");
        else if (status & MOO_EDIT_DELETED)
            /* Translators: this goes into window title, %s stands for filename, e.g. "/foo/bar.txt [deleted]" */
            return g_strdup_printf (_("%s [deleted]"), "");
        else
            return g_strdup_printf ("%s", "");
    }
    else
    {
        if (status & MOO_EDIT_NEW)
            /* Translators: this goes into window title, %s stands for filename, e.g. "/foo/bar.txt [new file] [modified]" */
            return g_strdup_printf (_("%s [new file] [modified]"), "");
        else if (status & MOO_EDIT_MODIFIED_ON_DISK)
            /* Translators: this goes into window title, %s stands for filename, e.g. "/foo/bar.txt [modified on disk] [modified]" */
            return g_strdup_printf (_("%s [modified on disk] [modified]"), "");
        else if (status & MOO_EDIT_DELETED)
            /* Translators: this goes into window title, %s stands for filename, e.g. "/foo/bar.txt [deleted] [modified]" */
            return g_strdup_printf (_("%s [deleted] [modified]"), "");
        else
            /* Translators: this goes into window title, %s stands for filename, e.g. "/foo/bar.txt [modified]" */
            return g_strdup_printf (_("%s [modified]"), "");
    }
}

static char *
parse_title_format (const char    *format,
                    MooEditWindow *window,
                    MooEdit       *doc)
{
    GString *str;

    str = g_string_new (NULL);

    while (*format)
    {
        if (*format == '%')
        {
            format++;

            if (!*format)
            {
                g_critical ("%s: trailing percent sign", G_STRFUNC);
                break;
            }

            switch (*format)
            {
                case 'a':
                    g_string_append (str, moo_editor_get_app_name (window->priv->editor));
                    break;
                case 'b':
                    if (!doc)
                        g_critical ("%s: %%b used without document", G_STRFUNC);
                    else
                        g_string_append (str, moo_edit_get_display_basename (doc));
                    break;
                case 'f':
                    if (!doc)
                        g_critical ("%s: %%f used without document", G_STRFUNC);
                    else
                        g_string_append (str, moo_edit_get_display_name (doc));
                    break;
                case 'u':
                    if (!doc)
                        g_critical ("%s: %%u used without document", G_STRFUNC);
                    else
                    {
                        char *tmp = moo_edit_get_uri (doc);
                        if (tmp)
                            g_string_append (str, tmp);
                        else
                            g_string_append (str, moo_edit_get_display_name (doc));
                        g_free (tmp);
                    }
                    break;
                case 's':
                    if (!doc)
                        g_critical ("%s: %%s used without document", G_STRFUNC);
                    else
                    {
                        char *tmp = get_doc_status_string (doc);
                        if (tmp)
                            g_string_append (str, tmp);
                        g_free (tmp);
                    }
                    break;
                case '%':
                    g_string_append_c (str, '%');
                    break;
                default:
                    g_critical ("%s: unknown format '%%%c'", G_STRFUNC, *format);
                    break;
            }
        }
        else
        {
            g_string_append_c (str, *format);
        }

        format++;
    }

    return g_string_free (str, FALSE);
}

static void
update_window_title (MooEditWindow *window)
{
    MooEdit *doc;
    char *title;

    doc = ACTIVE_DOC (window);

    if (doc)
        title = parse_title_format (window->priv->title_format, window, doc);
    else
        title = parse_title_format (window->priv->title_format_no_doc, window, NULL);

    gtk_window_set_title (GTK_WINDOW (window), title);

    g_free (title);
}

static const char *
check_format (const char *format)
{
    if (!format || !format[0])
        return DEFAULT_TITLE_FORMAT;
    if (!g_utf8_validate (format, -1, NULL))
    {
        g_critical ("%s: window title format is not valid UTF8", G_STRLOC);
        return DEFAULT_TITLE_FORMAT;
    }
    return format;
}

static void
set_title_format (MooEditWindow *window,
                  const char    *format,
                  const char    *format_no_doc)
{
    format = check_format (format);
    format_no_doc = check_format (format_no_doc);

    g_free (window->priv->title_format);
    g_free (window->priv->title_format_no_doc);

    window->priv->title_format = g_strdup (format);
    window->priv->title_format_no_doc = g_strdup (format_no_doc);

    if (GTK_WIDGET_REALIZED (window))
        update_window_title (window);
}

static void
set_title_format_from_prefs (MooEditWindow *window)
{
    const char *format, *format_no_doc;
    format = moo_prefs_get_string (moo_edit_setting (MOO_EDIT_PREFS_TITLE_FORMAT));
    format_no_doc = moo_prefs_get_string (moo_edit_setting (MOO_EDIT_PREFS_TITLE_FORMAT_NO_DOC));
    set_title_format (window, format, format_no_doc);
}

void
_moo_edit_window_update_title (void)
{
    GSList *l;
    for (l = windows; l != NULL; l = l->next)
        set_title_format_from_prefs (l->data);
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


gboolean
moo_edit_window_close_all (MooEditWindow *window)
{
    GSList *docs;
    gboolean result;

    g_return_val_if_fail (MOO_IS_EDIT_WINDOW (window), FALSE);

    docs = moo_edit_window_list_docs (window);
    result = moo_editor_close_docs (window->priv->editor, docs, TRUE);

    g_slist_free (docs);
    return result;
}


static gboolean
moo_edit_window_close (MooWindow *window)
{
    MooEditWindow *edit_window = MOO_EDIT_WINDOW (window);
    moo_editor_close_window (edit_window->priv->editor, edit_window, TRUE);
    return TRUE;
}


/****************************************************************************/
/* Actions
 */

static void
action_new_doc (MooEditWindow *window)
{
    moo_editor_new_doc (window->priv->editor, window);
}


static void
action_open (MooEditWindow *window)
{
    moo_editor_open (window->priv->editor, window, GTK_WIDGET (window), NULL);
}


static void
action_reload (MooEditWindow *window)
{
    MooEdit *edit = ACTIVE_DOC (window);
    g_return_if_fail (edit != NULL);
    _moo_editor_reload (window->priv->editor, edit, NULL, NULL);
}


static void
reopen_encoding_item_activated (const char *encoding,
                                gpointer    data)
{
    MooEditWindow *window = data;
    MooEdit *doc;

    doc = ACTIVE_DOC (window);
    g_return_if_fail (doc != NULL);

    _moo_editor_reload (window->priv->editor, doc, encoding, NULL);
}

static GtkAction *
create_reopen_with_encoding_action (MooEditWindow *window)
{
    GtkAction *action;

    action = _moo_encodings_menu_action_new ("ReopenWithEncoding",
                                             _("Reopen Using Encoding"),
                                             reopen_encoding_item_activated,
                                             window);
    moo_bind_bool_property (action, "sensitive",
                            window, "can-reload", FALSE);

    return action;
}


static void
update_doc_encoding_item (MooEditWindow *window)
{
    MooEdit *doc;
    GtkAction *action;
    const char *enc;

    if (!(doc = ACTIVE_DOC (window)))
        return;

    action = moo_window_get_action (MOO_WINDOW (window), "EncodingMenu");
    g_return_if_fail (action != NULL);

    enc = moo_edit_get_encoding (doc);

    if (!enc)
        enc = _moo_edit_get_default_encoding ();

    _moo_encodings_menu_action_set_current (action, enc);
}

static void
doc_encoding_item_activated (const char *encoding,
                             gpointer    data)
{
    MooEditWindow *window = data;
    MooEdit *doc;

    doc = ACTIVE_DOC (window);
    g_return_if_fail (doc != NULL);

    moo_edit_set_encoding (doc, encoding);
}

static GtkAction *
create_doc_encoding_action (MooEditWindow *window)
{
    GtkAction *action;

    action = _moo_encodings_menu_action_new ("EncodingMenu",
                                             _("_Encoding"),
                                             doc_encoding_item_activated,
                                             window);
    moo_bind_bool_property (action, "sensitive",
                            window, "has-open-document", FALSE);

    return action;
}


static const char *line_end_menu_items[] = {
    NULL, /* MOO_LE_NONE */
    "LineEndWin32", /* MOO_LE_UNIX */
    "LineEndUnix", /* MOO_LE_WIN32 */
    "LineEndMac", /* MOO_LE_MAC */
    "LineEndMix" /* MOO_LE_MIX */
};

static void
update_doc_line_end_item (MooEditWindow *window)
{
    MooEdit *doc;
    GtkAction *action;
    MooLineEndType le;

    if (!(doc = ACTIVE_DOC (window)))
        return;

    action = moo_window_get_action (MOO_WINDOW (window), "LineEndMenu");
    g_return_if_fail (action != NULL);

    le = moo_edit_get_line_end_type (doc);
    g_return_if_fail (le > 0 && le < G_N_ELEMENTS(line_end_menu_items));

    moo_menu_mgr_set_active (moo_menu_action_get_mgr (MOO_MENU_ACTION (action)),
                             line_end_menu_items[le], TRUE);
}

static void
doc_line_end_item_set_active (MooEditWindow *window, gpointer data)
{
    MooEdit *doc;

    doc = ACTIVE_DOC (window);
    g_return_if_fail (doc != NULL);

    moo_edit_set_line_end_type (doc, GPOINTER_TO_INT (data));
}

static GtkAction *
create_doc_line_end_action (MooEditWindow *window)
{
    GtkAction *action;
    MooMenuMgr *mgr;

    action = moo_menu_action_new ("LineEndMenu", _("Line En_dings"));
    mgr = moo_menu_action_get_mgr (MOO_MENU_ACTION (action));

    g_signal_connect_swapped (mgr, "radio-set-active", G_CALLBACK (doc_line_end_item_set_active), window);

    moo_menu_mgr_append (mgr, NULL,
                         line_end_menu_items[MOO_LE_WIN32],
                         _("_Windows (CR+LF)"), NULL, MOO_MENU_ITEM_RADIO,
                         GINT_TO_POINTER (MOO_LE_WIN32), NULL);
    moo_menu_mgr_insert (mgr, NULL, MOO_OS_WIN32 ? 1 : 0,
                         line_end_menu_items[MOO_LE_UNIX],
                         _("_Unix (LF)"), NULL, MOO_MENU_ITEM_RADIO,
                         GINT_TO_POINTER (MOO_LE_UNIX), NULL);
    moo_menu_mgr_append (mgr, NULL,
                         line_end_menu_items[MOO_LE_MAC],
                         _("_Mac (CR)"), NULL, MOO_MENU_ITEM_RADIO,
                         GINT_TO_POINTER (MOO_LE_MAC), NULL);
    moo_menu_mgr_append (mgr, NULL,
                         line_end_menu_items[MOO_LE_MIX],
                         _("Mi_xed"), NULL, MOO_MENU_ITEM_RADIO,
                         GINT_TO_POINTER (MOO_LE_MIX), NULL);

    moo_bind_bool_property (action, "sensitive",
                            window, "has-open-document", FALSE);

    return action;
}


static void
action_save (MooEditWindow *window)
{
    MooEdit *edit = ACTIVE_DOC (window);
    g_return_if_fail (edit != NULL);
    _moo_editor_save (window->priv->editor, edit, NULL);
}


static void
action_save_as (MooEditWindow *window)
{
    MooEdit *edit = ACTIVE_DOC (window);
    g_return_if_fail (edit != NULL);
    _moo_editor_save_as (window->priv->editor, edit, NULL, NULL, NULL);
}


static void
action_close_tab (MooEditWindow *window)
{
    MooEdit *edit = ACTIVE_DOC (window);
    g_return_if_fail (edit != NULL);
    moo_editor_close_doc (window->priv->editor, edit, TRUE);
}


static void
action_close_all (MooEditWindow *window)
{
    moo_edit_window_close_all (window);
}


static void
switch_to_tab (MooEditWindow *window,
               int            n)
{
    MooEdit *doc;

    if (n < 0)
        n = moo_edit_window_num_docs (window) - 1;

    if (n < 0 || n >= moo_edit_window_num_docs (window))
        return;

    moo_notebook_set_current_page (window->priv->notebook, n);

    if ((doc = ACTIVE_DOC (window)))
        gtk_widget_grab_focus (GTK_WIDGET (doc));
}

static void
action_previous_tab (MooEditWindow *window)
{
    int n;

    n = moo_notebook_get_current_page (window->priv->notebook);

    if (n > 0)
        switch_to_tab (window, n - 1);
    else
        switch_to_tab (window, -1);
}


static void
action_next_tab (MooEditWindow *window)
{
    int n;

    n = moo_notebook_get_current_page (window->priv->notebook);

    if (n < moo_notebook_get_n_pages (window->priv->notebook) - 1)
        switch_to_tab (window, n + 1);
    else
        switch_to_tab (window, 0);
}


static void
action_switch_to_tab (MooEditWindow *window,
                      guint          n)
{
    switch_to_tab (window, n);
}


static void
moo_edit_window_find_now (MooEditWindow *window,
                          gboolean       forward)
{
    MooEdit *doc;

    doc = ACTIVE_DOC (window);
    g_return_if_fail (doc != NULL);

    g_signal_emit_by_name (doc, "find-word-at-cursor", forward);
}

static void
action_find_now_f (MooEditWindow *window)
{
    moo_edit_window_find_now (window, TRUE);
}


static void
action_find_now_b (MooEditWindow *window)
{
    moo_edit_window_find_now (window, FALSE);
}


static void
action_abort_jobs (MooEditWindow *window)
{
    moo_edit_window_abort_jobs (window);
}


static void
action_toggle_bookmark (MooEditWindow *window)
{
    MooEdit *doc = ACTIVE_DOC (window);
    g_return_if_fail (doc != NULL);
    moo_edit_toggle_bookmark (doc, moo_text_view_get_cursor_line (MOO_TEXT_VIEW (doc)));
}


static void
action_next_bookmark (MooEditWindow *window)
{
    int cursor;
    GSList *bookmarks;
    MooEdit *doc = ACTIVE_DOC (window);

    g_return_if_fail (doc != NULL);

    cursor = moo_text_view_get_cursor_line (MOO_TEXT_VIEW (doc));
    bookmarks = moo_edit_get_bookmarks_in_range (doc, cursor + 1, -1);

    if (bookmarks)
    {
        moo_edit_goto_bookmark (doc, bookmarks->data);
        g_slist_free (bookmarks);
    }
}


static void
action_prev_bookmark (MooEditWindow *window)
{
    int cursor;
    GSList *bookmarks = NULL;
    MooEdit *doc = ACTIVE_DOC (window);

    g_return_if_fail (doc != NULL);

    cursor = moo_text_view_get_cursor_line (MOO_TEXT_VIEW (doc));

    if (cursor > 0)
        bookmarks = moo_edit_get_bookmarks_in_range (doc, 0, cursor - 1);

    if (bookmarks)
    {
        GSList *last = g_slist_last (bookmarks);
        moo_edit_goto_bookmark (doc, last->data);
        g_slist_free (bookmarks);
    }
}


static void
goto_bookmark_activated (GtkAction *action,
                         gpointer   data)
{
    MooEdit *doc;
    MooEditWindow *window;
    MooEditBookmark *bk;
    guint n = GPOINTER_TO_UINT (data);

    window = _moo_action_get_window (action);
    g_return_if_fail (window != NULL);

    doc = ACTIVE_DOC (window);
    g_return_if_fail (doc != NULL);

    if ((bk = moo_edit_get_bookmark (doc, n)))
        moo_edit_goto_bookmark (doc, bk);
}

static GtkAction *
create_goto_bookmark_action (MooWindow *window,
                             gpointer   data)
{
    GtkAction *action;
    guint n = GPOINTER_TO_UINT (data);
    char *accel;
    char *name;

    name = g_strdup_printf (MOO_EDIT_GOTO_BOOKMARK_ACTION "%u", n);
    accel = g_strdup_printf ("<ctrl>%u", n);

    action = g_object_new (MOO_TYPE_ACTION, "name", name, "default-accel", accel,
                           "connect-accel", TRUE, "accel-editable", FALSE, NULL);
    g_signal_connect (action, "activate", G_CALLBACK (goto_bookmark_activated), data);
    moo_bind_bool_property (action, "sensitive", window, "has-open-document", FALSE);

    g_free (name);
    g_free (accel);
    return action;
}


static void
bookmark_item_activated (GtkWidget *item)
{
    moo_edit_goto_bookmark (g_object_get_data (G_OBJECT (item), "moo-edit"),
                            g_object_get_data (G_OBJECT (item), "moo-bookmark"));
}

static GtkWidget *
create_bookmark_item (MooEditWindow   *window,
                      MooEdit         *doc,
                      MooEditBookmark *bk)
{
    char *label, *bk_text;
    GtkWidget *item = NULL;

    bk_text = _moo_edit_bookmark_get_text (bk);
    label = g_strdup_printf ("%d - \"%s\"", 1 + moo_line_mark_get_line (MOO_LINE_MARK (bk)),
                             bk_text ? bk_text : "");
    g_free (bk_text);

    if (bk->no)
    {
        GtkAction *action;
        char *action_name;

        action_name = g_strdup_printf (MOO_EDIT_GOTO_BOOKMARK_ACTION "%u", bk->no);
        action = moo_window_get_action (MOO_WINDOW (window), action_name);

        if (action)
        {
            g_object_set (action, "label", label, "use-underline", FALSE, NULL);
            item = gtk_action_create_menu_item (action);
        }
        else
        {
            g_critical ("%s: oops", G_STRLOC);
        }

        g_free (action_name);
    }

    if (!item)
    {
        item = gtk_menu_item_new_with_label (label);
        g_signal_connect (item, "activate", G_CALLBACK (bookmark_item_activated), NULL);
    }

    g_object_set_data_full (G_OBJECT (item), "moo-bookmark", g_object_ref (bk), g_object_unref);
    g_object_set_data_full (G_OBJECT (item), "moo-edit", g_object_ref (doc), g_object_unref);

    g_free (label);

    return item;
}

static void
populate_bookmark_menu (MooEditWindow *window,
                        GtkWidget     *menu,
                        GtkWidget     *next_bk_item)
{
    GtkAction *pn;
    MooEdit *doc;
    GtkWidget *item;
    const GSList *bookmarks;
    GList *children, *l;
    int pos;

    children = g_list_copy (GTK_MENU_SHELL (menu)->children);
    pos = g_list_index (children, next_bk_item);
    g_return_if_fail (pos >= 0);

    for (l = g_list_nth (children, pos + 1); l != NULL; l = l->next)
    {
        if (g_object_get_data (l->data, "moo-bookmark-menu-item"))
            gtk_container_remove (GTK_CONTAINER (menu), l->data);
        else
            break;
    }

    g_list_free (children);


    doc = ACTIVE_DOC (window);
    bookmarks = doc ? moo_edit_list_bookmarks (doc) : NULL;

    pn = moo_window_get_action (MOO_WINDOW (window), "PreviousBookmark");
    gtk_action_set_sensitive (pn, bookmarks != NULL);
    pn = moo_window_get_action (MOO_WINDOW (window), "NextBookmark");
    gtk_action_set_sensitive (pn, bookmarks != NULL);

    if (bookmarks)
    {
        item = gtk_separator_menu_item_new ();
        gtk_widget_show (item);
        g_object_set_data (G_OBJECT (item), "moo-bookmark-menu-item", GINT_TO_POINTER (TRUE));
        gtk_menu_shell_insert (GTK_MENU_SHELL (menu), item, ++pos);
    }

    while (bookmarks)
    {
        item = create_bookmark_item (window, doc, bookmarks->data);
        gtk_widget_show (item);
        g_object_set_data (G_OBJECT (item), "moo-bookmark-menu-item", GINT_TO_POINTER (TRUE));
        gtk_menu_shell_insert (GTK_MENU_SHELL (menu), item, ++pos);
        bookmarks = bookmarks->next;
    }
}

static void
doc_item_selected (MooWindow   *window,
                   GtkMenuItem *doc_item)
{
    GtkWidget *menu;
    GtkWidget *next_bk_item;

    menu = gtk_menu_item_get_submenu (doc_item);
    next_bk_item = moo_ui_xml_get_widget (moo_window_get_ui_xml (window),
                                          window->menubar,
                                          "Editor/Menubar/Document/NextBookmark");
    g_return_if_fail (menu != NULL && next_bk_item != NULL);

    populate_bookmark_menu (MOO_EDIT_WINDOW (window), menu, next_bk_item);
}


static void
moo_edit_window_connect_menubar (MooWindow *window)
{
    GtkWidget *doc_item;
    GtkWidget *win_item;

    if (!window->menubar)
        return;

    doc_item = moo_ui_xml_get_widget (moo_window_get_ui_xml (window),
                                      window->menubar,
                                      "Editor/Menubar/Document");
    g_return_if_fail (doc_item != NULL);
    g_signal_connect_swapped (doc_item, "select",
                              G_CALLBACK (doc_item_selected),
                              window);

    win_item = moo_ui_xml_get_widget (moo_window_get_ui_xml (window),
                                      window->menubar,
                                      "Editor/Menubar/Window");
    g_return_if_fail (win_item != NULL);
    g_signal_connect_swapped (win_item, "select",
                              G_CALLBACK (window_menu_item_selected),
                              window);
}


#ifdef ENABLE_PRINTING
static void
action_page_setup (MooEditWindow *window)
{
    _moo_edit_page_setup (GTK_WIDGET (window));
}


static void
action_print (MooEditWindow *window)
{
    gpointer doc = ACTIVE_DOC (window);
    g_return_if_fail (doc != NULL);
    _moo_edit_print (doc, GTK_WIDGET (window));
}


static void
action_print_preview (MooEditWindow *window)
{
    gpointer doc = ACTIVE_DOC (window);
    g_return_if_fail (doc != NULL);
    _moo_edit_print_preview (doc, GTK_WIDGET (window));
}


static void
action_print_pdf (MooEditWindow *window)
{
    char *start_name;
    const char *doc_name, *dot;
    const char *filename;
    gpointer doc = ACTIVE_DOC (window);

    doc_name = doc ? moo_edit_get_display_basename (doc) : "output";
    dot = strrchr (doc_name, '.');

    if (dot && dot != doc_name)
    {
        start_name = g_new (char, (dot - doc_name) + 5);
        memcpy (start_name, doc_name, dot - doc_name);
        memcpy (start_name + (dot - doc_name), ".pdf", 5);
    }
    else
    {
        start_name = g_strdup_printf ("%s.pdf", doc_name);
    }

    doc = ACTIVE_DOC (window);
    g_return_if_fail (doc != NULL);

    filename = moo_file_dialogp (GTK_WIDGET (window),
                                 MOO_FILE_DIALOG_SAVE,
                                 start_name,
                                 _("Export as PDF"),
                                 moo_edit_setting (MOO_EDIT_PREFS_PDF_LAST_DIR),
                                 NULL);

    if (filename)
        _moo_edit_export_pdf (doc, filename);

    g_free (start_name);
}
#endif


static void
wrap_text_toggled (MooEditWindow *window,
                   gboolean       active)
{
    MooEdit *doc;
    GtkWrapMode mode;

    doc = ACTIVE_DOC (window);
    g_return_if_fail (doc != NULL);

    g_object_get (doc, "wrap-mode", &mode, NULL);

    if ((active && mode != GTK_WRAP_NONE) || (!active && mode == GTK_WRAP_NONE))
        return;

    if (!active)
    {
        mode = GTK_WRAP_NONE;
    }
    else
    {
        if (moo_prefs_get_bool (moo_edit_setting (MOO_EDIT_PREFS_WRAP_WORDS)))
            mode = GTK_WRAP_WORD;
        else
            mode = GTK_WRAP_CHAR;
    }

    moo_edit_config_set (doc->config, MOO_EDIT_CONFIG_SOURCE_USER, "wrap-mode", mode, NULL);
}


static void
line_numbers_toggled (MooEditWindow *window,
                      gboolean       active)
{
    MooEdit *doc;
    gboolean show;

    doc = ACTIVE_DOC (window);
    g_return_if_fail (doc != NULL);

    g_object_get (doc, "show-line-numbers", &show, NULL);

    if ((active && show) || (!active && !show))
        return;

    moo_edit_config_set (doc->config, MOO_EDIT_CONFIG_SOURCE_USER,
                         "show-line-numbers", active,
                         NULL);
}


/****************************************************************************/
/* Notebook popup menu
 */

static void
close_activated (GtkWidget     *item,
                 MooEditWindow *window)
{
    MooEdit *edit = g_object_get_data (G_OBJECT (item), "moo-edit");
    g_return_if_fail (MOO_IS_EDIT_WINDOW (window));
    g_return_if_fail (MOO_IS_EDIT (edit));
    moo_editor_close_doc (window->priv->editor, edit, TRUE);
}


static void
close_others_activated (GtkWidget     *item,
                        MooEditWindow *window)
{
    GSList *list;
    MooEdit *edit = g_object_get_data (G_OBJECT (item), "moo-edit");

    g_return_if_fail (MOO_IS_EDIT_WINDOW (window));
    g_return_if_fail (MOO_IS_EDIT (edit));

    list = moo_edit_window_list_docs (window);
    list = g_slist_remove (list, edit);

    if (list)
        moo_editor_close_docs (window->priv->editor, list, TRUE);

    g_slist_free (list);
}


static void
detach_activated (GtkWidget     *item,
                  MooEditWindow *window)
{
    MooEdit *doc = g_object_get_data (G_OBJECT (item), "moo-edit");

    g_return_if_fail (MOO_IS_EDIT_WINDOW (window));
    g_return_if_fail (MOO_IS_EDIT (doc));

    _moo_editor_move_doc (window->priv->editor, doc, NULL, TRUE);
}


/****************************************************************************/
/* Documents
 */

static void
notebook_switch_page (G_GNUC_UNUSED MooNotebook *notebook,
                      guint          page_num,
                      MooEditWindow *window)
{
    edit_changed (window, get_nth_tab (window, page_num));
    moo_edit_window_check_actions (window);
    moo_edit_window_update_doc_list (window);
    g_object_notify (G_OBJECT (window), "active-doc");
}


static gboolean
notebook_populate_popup (MooNotebook        *notebook,
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

    if (moo_edit_window_num_docs (window) > 1)
    {
        gtk_menu_shell_append (GTK_MENU_SHELL (menu),
                               g_object_new (GTK_TYPE_SEPARATOR_MENU_ITEM,
                                             "visible", TRUE, NULL));

        item = gtk_menu_item_new_with_label ("Detach");
        gtk_widget_show (item);
        gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
        g_object_set_data (G_OBJECT (item), "moo-edit", edit);
        g_signal_connect (item, "activate",
                          G_CALLBACK (detach_activated),
                          window);
    }

    return FALSE;
}


static gboolean
notebook_button_press (MooNotebook    *notebook,
                       GdkEventButton *event,
                       MooEditWindow  *window)
{
    int n;

    if (event->button != 2 || event->type != GDK_BUTTON_PRESS)
        return FALSE;

    n = moo_notebook_get_event_tab (notebook, (GdkEvent*) event);

    if (n < 0)
        return FALSE;

    moo_editor_close_doc (window->priv->editor,
                          get_nth_tab (window, n),
                          TRUE);

    return TRUE;
}


static void
set_use_tabs (MooEditWindow *window)
{
    g_return_if_fail (MOO_IS_EDIT_WINDOW (window));
    g_object_set (window->priv->notebook, "show-tabs",
                  moo_prefs_get_bool (moo_edit_setting (MOO_EDIT_PREFS_USE_TABS)), NULL);
}

void
_moo_edit_window_set_use_tabs (void)
{
    GSList *l;

    for (l = windows; l != NULL; l = l->next)
        set_use_tabs (l->data);
}


static void
setup_notebook (MooEditWindow *window)
{
    GtkWidget *button, *icon, *frame;

    set_use_tabs (window);

    g_signal_connect_after (window->priv->notebook, "moo-switch-page",
                            G_CALLBACK (notebook_switch_page), window);
    g_signal_connect (window->priv->notebook, "populate-popup",
                      G_CALLBACK (notebook_populate_popup), window);
    g_signal_connect (window->priv->notebook, "button-press-event",
                      G_CALLBACK (notebook_button_press), window);

    frame = gtk_aspect_frame_new (NULL, 0.5, 0.5, 1.0, FALSE);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);

    button = gtk_button_new ();
    gtk_button_set_focus_on_click (GTK_BUTTON (button), FALSE);
    g_signal_connect_swapped (button, "clicked",
                              G_CALLBACK (action_close_tab), window);
    moo_bind_bool_property (button, "sensitive", window, "has-open-document", FALSE);

    icon = _moo_create_small_icon (MOO_SMALL_ICON_CLOSE);

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
edit_changed (MooEditWindow *window,
              MooEdit       *doc)
{
    if (doc == ACTIVE_DOC (window))
    {
        g_object_freeze_notify (G_OBJECT (window));
        g_object_notify (G_OBJECT (window), "can-reload");
        g_object_notify (G_OBJECT (window), "has-open-document");
        g_object_notify (G_OBJECT (window), "has-comments");
        g_object_thaw_notify (G_OBJECT (window));

        update_window_title (window);
        update_statusbar (window);
        update_lang_menu (window);
        update_doc_view_actions (window);
        update_doc_encoding_item (window);
    }

    if (doc)
        update_tab_label (window, doc);
}

static void
edit_encoding_changed (MooEditWindow *window,
                       G_GNUC_UNUSED GParamSpec *pspec,
                       MooEdit       *doc)
{
    if (doc == ACTIVE_DOC (window))
        update_doc_encoding_item (window);
}

static void
edit_line_end_changed (MooEditWindow *window,
                       G_GNUC_UNUSED GParamSpec *pspec,
                       MooEdit       *doc)
{
    if (doc == ACTIVE_DOC (window))
        update_doc_line_end_item (window);
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
edit_cursor_moved (MooEditWindow *window,
                   G_GNUC_UNUSED GtkTextIter *iter,
                   MooEdit *doc)
{
    if (doc == ACTIVE_DOC (window))
        update_statusbar (window);
}


static void
edit_wrap_mode_changed (MooEditWindow *window,
                        G_GNUC_UNUSED GParamSpec *pspec,
                        MooEdit       *doc)
{
    gpointer action;
    GtkWrapMode mode;

    if (doc != ACTIVE_DOC (window))
        return;

    action = moo_window_get_action (MOO_WINDOW (window), "WrapText");
    g_return_if_fail (action != NULL);

    g_object_get (doc, "wrap-mode", &mode, NULL);
    gtk_toggle_action_set_active (action, mode != GTK_WRAP_NONE);
}


static void
edit_show_line_numbers_changed (MooEditWindow *window,
                                G_GNUC_UNUSED GParamSpec *pspec,
                                MooEdit       *doc)
{
    gpointer action;
    gboolean show;

    if (doc != ACTIVE_DOC (window))
        return;

    action = moo_window_get_action (MOO_WINDOW (window), "LineNumbers");
    g_return_if_fail (action != NULL);

    g_object_get (doc, "show-line-numbers", &show, NULL);
    gtk_toggle_action_set_active (action, show);
}


static void
update_doc_view_actions (MooEditWindow *window)
{
    MooEdit *doc;

    doc = ACTIVE_DOC (window);

    if (!doc)
        return;

    edit_wrap_mode_changed (window, NULL, doc);
    edit_show_line_numbers_changed (window, NULL, doc);
}


static void
edit_filename_changed (MooEditWindow      *window,
                       G_GNUC_UNUSED const char *filename,
                       MooEdit            *doc)
{
    edit_changed (window, doc);
    moo_edit_window_update_doc_list (window);
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


int
moo_edit_window_num_docs (MooEditWindow *window)
{
    g_return_val_if_fail (MOO_IS_EDIT_WINDOW (window), 0);

    if (!window->priv->notebook)
        return 0;
    else
        return moo_notebook_get_n_pages (window->priv->notebook);
}


MooEdit *
moo_edit_window_get_nth_doc (MooEditWindow  *window,
                             guint           n)
{
    g_return_val_if_fail (MOO_IS_EDIT_WINDOW (window), NULL);

    if (!window->priv->notebook || n >= (guint) moo_notebook_get_n_pages (window->priv->notebook))
        return NULL;

    return get_nth_tab (window, n);
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


int
_moo_edit_window_get_doc_no (MooEditWindow *window,
                             MooEdit       *doc)
{
    return get_page_num (window, doc);
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

    if (position < 0)
        position = moo_notebook_get_current_page (window->priv->notebook) + 1;
    moo_notebook_insert_page (window->priv->notebook, scrolledwindow, label, position);

    g_signal_connect_swapped (edit, "doc_status_changed",
                              G_CALLBACK (edit_changed), window);
    g_signal_connect_swapped (edit, "notify::encoding",
                              G_CALLBACK (edit_encoding_changed), window);
    g_signal_connect_swapped (edit, "notify::line-end",
                              G_CALLBACK (edit_line_end_changed), window);
    g_signal_connect_swapped (edit, "notify::overwrite",
                              G_CALLBACK (edit_overwrite_changed), window);
    g_signal_connect_swapped (edit, "notify::wrap-mode",
                              G_CALLBACK (edit_wrap_mode_changed), window);
    g_signal_connect_swapped (edit, "notify::show-line-numbers",
                              G_CALLBACK (edit_show_line_numbers_changed), window);
    g_signal_connect_swapped (edit, "filename_changed",
                              G_CALLBACK (edit_filename_changed), window);
    g_signal_connect_swapped (edit, "notify::has-comments",
                              G_CALLBACK (proxy_boolean_property), window);
    g_signal_connect_swapped (edit, "config-notify::lang",
                              G_CALLBACK (edit_lang_changed), window);
    g_signal_connect_swapped (edit, "cursor-moved",
                              G_CALLBACK (edit_cursor_moved), window);

    moo_edit_window_update_doc_list (window);
    g_signal_emit (window, signals[NEW_DOC], 0, edit);

    _moo_doc_attach_plugins (window, edit);

    moo_edit_window_set_active_doc (window, edit);
    edit_changed (window, edit);
    gtk_widget_grab_focus (GTK_WIDGET (edit));
}


void
_moo_edit_window_remove_doc (MooEditWindow  *window,
                             MooEdit        *doc,
                             gboolean        destroy)
{
    int page;
    GtkAction *action;
    MooEdit *new_doc;
    gboolean had_focus;

    g_return_if_fail (MOO_IS_EDIT_WINDOW (window));
    g_return_if_fail (MOO_IS_EDIT (doc));

    page = get_page_num (window, doc);
    g_return_if_fail (page >= 0);

    had_focus = GTK_WIDGET_HAS_FOCUS (doc);

    g_signal_emit (window, signals[CLOSE_DOC], 0, doc);

    g_signal_handlers_disconnect_by_func (doc, (gpointer) edit_changed, window);
    g_signal_handlers_disconnect_by_func (doc, (gpointer) edit_filename_changed, window);
    g_signal_handlers_disconnect_by_func (doc, (gpointer) proxy_boolean_property, window);
    g_signal_handlers_disconnect_by_func (doc, (gpointer) edit_cursor_moved, window);
    g_signal_handlers_disconnect_by_func (doc, (gpointer) edit_lang_changed, window);
    g_signal_handlers_disconnect_by_func (doc, (gpointer) edit_overwrite_changed, window);
    g_signal_handlers_disconnect_by_func (doc, (gpointer) edit_wrap_mode_changed, window);
    g_signal_handlers_disconnect_by_func (doc, (gpointer) edit_show_line_numbers_changed, window);

    _moo_doc_detach_plugins (window, doc);

    action = g_object_get_data (G_OBJECT (doc), "moo-doc-list-action");

    if (action)
    {
        moo_action_collection_remove_action (moo_window_get_actions (MOO_WINDOW (window)), action);
        g_object_set_data (G_OBJECT (doc), "moo-doc-list-action", NULL);
    }

    window->priv->history = g_list_remove (window->priv->history, doc);
    window->priv->history_blocked = TRUE;

    moo_edit_window_update_doc_list (window);

    /* removing scrolled window from the notebook will destroy the scrolled window,
     * and that in turn will destroy the doc if it's not removed before */
    if (!destroy)
        gtk_container_remove (GTK_CONTAINER (GTK_WIDGET(doc)->parent), GTK_WIDGET (doc));
    moo_notebook_remove_page (window->priv->notebook, page);

    window->priv->history_blocked = FALSE;
    if (window->priv->history)
        moo_edit_window_set_active_doc (window, window->priv->history->data);

    edit_changed (window, NULL);

    g_signal_emit (window, signals[CLOSE_DOC_AFTER], 0);
    g_object_notify (G_OBJECT (window), "active-doc");

    new_doc = ACTIVE_DOC (window);

    if (!new_doc)
        moo_edit_window_check_actions (window);
    else if (had_focus)
        gtk_widget_grab_focus (GTK_WIDGET (new_doc));
}


typedef struct {
    int x;
    int y;
    gboolean drag_started;
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

static void     tab_icon_drag_begin         (GtkWidget      *evbox,
                                             GdkDragContext *context,
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
    g_signal_handlers_disconnect_by_func (evbox, (gpointer) tab_icon_button_release, window);
    g_signal_handlers_disconnect_by_func (evbox, (gpointer) tab_icon_motion_notify, window);
    return FALSE;
}


static void
tab_icon_start_drag (GtkWidget      *evbox,
                     GdkEvent       *event,
                     MooEditWindow  *window)
{
    GtkTargetList *targets;
    MooEdit *edit;

    edit = g_object_get_data (G_OBJECT (evbox), "moo-edit");
    g_return_if_fail (MOO_IS_EDIT (edit));

    g_signal_connect (evbox, "drag-begin", G_CALLBACK (tab_icon_drag_begin), window);
    g_signal_connect (evbox, "drag-data-delete", G_CALLBACK (tab_icon_drag_data_delete), window);
    g_signal_connect (evbox, "drag-data-get", G_CALLBACK (tab_icon_drag_data_get), window);
    g_signal_connect (evbox, "drag-end", G_CALLBACK (tab_icon_drag_end), window);

    targets = gtk_target_list_new (NULL, 0);

    gtk_target_list_add (targets,
                         moo_atom_uri_list (),
                         0, TARGET_URI_LIST);
    gtk_target_list_add (targets,
                         MOO_EDIT_TAB_ATOM,
                         GTK_TARGET_SAME_APP,
                         TARGET_MOO_EDIT_TAB);

    gtk_drag_begin (evbox, targets,
                    GDK_ACTION_COPY | GDK_ACTION_MOVE | GDK_ACTION_LINK,
                    1, (GdkEvent*) event);

    gtk_target_list_unref (targets);
}


static gboolean
tab_icon_motion_notify (GtkWidget      *evbox,
                        GdkEventMotion *event,
                        MooEditWindow  *window)
{
    DragInfo *info;

    info = g_object_get_data (G_OBJECT (evbox), "moo-drag-info");
    g_return_val_if_fail (info != NULL, FALSE);

    if (info->drag_started)
        return TRUE;

    if (gtk_drag_check_threshold (evbox, info->x, info->y, event->x, event->y))
    {
        info->drag_started = TRUE;
        tab_icon_start_drag (evbox, (GdkEvent*) event, window);
    }

    return TRUE;
}


static gboolean
tab_icon_button_press (GtkWidget        *evbox,
                       GdkEventButton   *event,
                       MooEditWindow    *window)
{
    DragInfo *info;

    if (event->button != 1 || event->type != GDK_BUTTON_PRESS)
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
tab_icon_drag_begin (GtkWidget      *evbox,
                     GdkDragContext *context,
                     G_GNUC_UNUSED MooEditWindow *window)
{
    GdkPixbuf *pixbuf;
    GtkImage *icon;
    icon = g_object_get_data (G_OBJECT (evbox), "moo-edit-icon");
    pixbuf = gtk_image_get_pixbuf (icon);
    gtk_drag_set_icon_pixbuf (context, pixbuf, 0, 0);
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
        moo_selection_data_set_pointer (data, MOO_EDIT_TAB_ATOM, edit);
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
        gtk_selection_data_set_text (data, "", -1);
    }
}


static void
tab_icon_drag_end (GtkWidget      *evbox,
                   G_GNUC_UNUSED GdkDragContext *context,
                   MooEditWindow  *window)
{
    g_object_set_data (G_OBJECT (evbox), "moo-drag-info", NULL);
    g_signal_handlers_disconnect_by_func (evbox, (gpointer) tab_icon_drag_begin, window);
    g_signal_handlers_disconnect_by_func (evbox, (gpointer) tab_icon_drag_data_delete, window);
    g_signal_handlers_disconnect_by_func (evbox, (gpointer) tab_icon_drag_data_get, window);
    g_signal_handlers_disconnect_by_func (evbox, (gpointer) tab_icon_drag_end, window);
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
    gtk_event_box_set_visible_window (GTK_EVENT_BOX (evbox), FALSE);
    gtk_box_pack_start (GTK_BOX (hbox), evbox, FALSE, FALSE, 0);

    icon = gtk_image_new ();
    gtk_container_add (GTK_CONTAINER (evbox), icon);
    gtk_widget_show_all (evbox);

    label = gtk_label_new (moo_edit_get_display_basename (edit));
    gtk_label_set_single_line_mode (GTK_LABEL (label), TRUE);
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

    g_object_unref (group);

    return hbox;
}


static void
set_tab_icon (GtkWidget *image,
              GdkPixbuf *pixbuf)
{
    GdkPixbuf *old_pixbuf;

    /* file icons are cached, so it's likely the same pixbuf
     * object as before (and it happens every time you switch tabs) */
    old_pixbuf = gtk_image_get_pixbuf (GTK_IMAGE (image));

    if (old_pixbuf != pixbuf)
        gtk_image_set_from_pixbuf (GTK_IMAGE (image), pixbuf);
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

    _moo_widget_set_tooltip (hbox, moo_edit_get_display_name (doc));

    status = moo_edit_get_status (doc);

    deleted = status & (MOO_EDIT_DELETED | MOO_EDIT_MODIFIED_ON_DISK);
    modified = (status & MOO_EDIT_MODIFIED) && !(status & MOO_EDIT_CLEAN);

    label_text = g_strdup_printf ("%s%s%s",
                                  deleted ? "!" : "",
                                  modified ? "*" : "",
                                  moo_edit_get_display_basename (doc));
    gtk_label_set_text (GTK_LABEL (label), label_text);

    {
        int width, max_width, height;
        PangoLayout *M = gtk_widget_create_pango_layout (label, "MMMMMMMMMMMMMMMMMMMM");
        PangoLayout *layout = gtk_widget_create_pango_layout (label, label_text);
        pango_layout_get_pixel_size (layout, &width, &height);
        pango_layout_get_pixel_size (M, &max_width, &height);

        if (width > max_width)
        {
            gtk_widget_set_size_request (label, max_width, -1);
            gtk_label_set_ellipsize (GTK_LABEL (label), PANGO_ELLIPSIZE_END);
        }
        else
        {
            gtk_widget_set_size_request (label, -1, -1);
            gtk_label_set_ellipsize (GTK_LABEL (label), PANGO_ELLIPSIZE_NONE);
        }

        g_object_unref (layout);
    }

    pixbuf = _moo_edit_get_icon (doc, icon, GTK_ICON_SIZE_MENU);
    set_tab_icon (icon, pixbuf);

    g_free (label_text);
}


/****************************************************************************/
/* Panes
 */

static gboolean
save_paned_config (MooEditWindow *window)
{
    char *config;
    const char *old_config;

    window->priv->save_params_idle = 0;

    config = moo_big_paned_get_config (window->paned);
    g_return_val_if_fail (config != NULL, FALSE);

    old_config = moo_prefs_get_string (MOO_EDIT_PREFS_PREFIX "/window/paned");

    if (!old_config || strcmp (old_config, config) != 0)
        moo_prefs_set_string (MOO_EDIT_PREFS_PREFIX "/window/paned", config);

    g_free (config);
    return FALSE;
}

static void
paned_config_changed (MooEditWindow *window)
{
    if (!window->priv->save_params_idle)
        window->priv->save_params_idle =
            moo_idle_add ((GSourceFunc) save_paned_config, window);
}

static void
create_paned (MooEditWindow *window)
{
    MooBigPaned *paned;
    const char *config;

    paned = g_object_new (MOO_TYPE_BIG_PANED,
                          "handle-cursor-type", GDK_FLEUR,
                          "enable-detaching", TRUE,
                          NULL);
    gtk_widget_show (GTK_WIDGET (paned));
    gtk_box_pack_start (GTK_BOX (MOO_WINDOW(window)->vbox),
                        GTK_WIDGET (paned), TRUE, TRUE, 0);

    window->paned = paned;

    moo_prefs_create_key (MOO_EDIT_PREFS_PREFIX "/window/paned",
                          MOO_PREFS_STATE, G_TYPE_STRING, NULL);
    config = moo_prefs_get_string (MOO_EDIT_PREFS_PREFIX "/window/paned");

    if (config)
        moo_big_paned_set_config (paned, config);

    g_signal_connect_swapped (paned, "config-changed",
                              G_CALLBACK (paned_config_changed),
                              window);
}


static char *
make_show_pane_action_id (const char *user_id)
{
    return g_strdup_printf ("MooEditWindow-ShowPane-%s", user_id);
}

static void
show_pane_callback (MooEditWindow *window,
                    const char    *user_id)
{
    GtkWidget *pane;
    pane = moo_edit_window_get_pane (window, user_id);
    moo_big_paned_present_pane (window->paned, pane);
}

static void
add_pane_action (MooEditWindow *window,
                 const char    *user_id,
                 MooPaneLabel  *label)
{
    char *action_id;
    MooWindowClass *klass;
    GtkAction *action;
    MooUiXml *xml;

    action_id = make_show_pane_action_id (user_id);
    klass = g_type_class_peek (MOO_TYPE_EDIT_WINDOW);

    if (!moo_window_class_find_action (klass, action_id))
    {
        guint merge_id;

        _moo_window_class_new_action_callback (klass, action_id, NULL,
                                               G_CALLBACK (show_pane_callback),
                                               _moo_marshal_VOID__STRING,
                                               G_TYPE_NONE, 1,
                                               G_TYPE_STRING, user_id,
                                               "visible", FALSE,
                                               "display-name", label->label,
                                               "label", label->label,
                                               /* XXX IconInfo */
                                               "stock-id", label->icon_stock_id,
                                               NULL);

        xml = moo_editor_get_ui_xml (moo_editor_instance ());
        merge_id = moo_ui_xml_new_merge_id (xml);
        moo_ui_xml_add_item (xml, merge_id,
                             "Editor/Menubar/View/PanesMenu",
                             action_id, action_id, -1);
    }

    action = moo_window_get_action (MOO_WINDOW (window), action_id);
    g_return_if_fail (action != NULL);
    g_object_set (action, "visible", TRUE, NULL);

    g_free (action_id);
}

static void
remove_pane_action (MooEditWindow *window,
                    const char    *user_id)
{
    char *action_id;
    GtkAction *action;

    action_id = make_show_pane_action_id (user_id);
    action = moo_window_get_action (MOO_WINDOW (window), action_id);

    if (action)
        g_object_set (action, "visible", FALSE, NULL);

#if 0
    klass = g_type_class_peek (MOO_TYPE_EDIT_WINDOW);

    if (moo_window_class_find_action (klass, action_id))
        moo_window_class_remove_last_action (klass, action_id);
#endif

    g_free (action_id);
}

static MooPane *
moo_edit_window_add_pane_full (MooEditWindow  *window,
                               const char     *user_id,
                               GtkWidget      *widget,
                               MooPaneLabel   *label,
                               MooPanePosition position,
                               gboolean        add_menu)
{
    MooPane *pane;

    g_return_val_if_fail (MOO_IS_EDIT_WINDOW (window), NULL);
    g_return_val_if_fail (user_id != NULL, NULL);
    g_return_val_if_fail (GTK_IS_WIDGET (widget), NULL);
    g_return_val_if_fail (label != NULL, NULL);

    g_return_val_if_fail (moo_edit_window_get_pane (window, user_id) == NULL, NULL);

    MOO_OBJECT_REF_SINK (widget);

    pane = moo_big_paned_insert_pane (window->paned, widget, user_id, label, position, -1);

    if (pane != NULL)
    {
        moo_pane_set_removable (pane, FALSE);

        if (add_menu)
            add_pane_action (window, user_id, label);
    }

    g_object_unref (widget);
    return pane;
}

MooPane *
moo_edit_window_add_pane (MooEditWindow  *window,
                          const char     *user_id,
                          GtkWidget      *widget,
                          MooPaneLabel   *label,
                          MooPanePosition position)
{
    return moo_edit_window_add_pane_full (window, user_id, widget,
                                          label, position, TRUE);
}


gboolean
moo_edit_window_remove_pane (MooEditWindow *window,
                             const char    *user_id)
{
    MooPane *pane;

    g_return_val_if_fail (MOO_IS_EDIT_WINDOW (window), FALSE);
    g_return_val_if_fail (user_id != NULL, FALSE);

    remove_pane_action (window, user_id);

    if (!(pane = moo_big_paned_lookup_pane (window->paned, user_id)))
        return FALSE;

    moo_big_paned_remove_pane (window->paned, moo_pane_get_child (pane));
    return TRUE;
}


GtkWidget*
moo_edit_window_get_pane (MooEditWindow  *window,
                          const char     *user_id)
{
    MooPane *pane;

    g_return_val_if_fail (MOO_IS_EDIT_WINDOW (window), NULL);
    g_return_val_if_fail (user_id != NULL, NULL);

    pane = moo_big_paned_lookup_pane (window->paned, user_id);
    return pane ? moo_pane_get_child (pane) : NULL;
}


/****************************************************************************/
/* Statusbar
 */

static void
set_statusbar_numbers (MooEditWindow *window,
                       int            line,
                       int            column,
                       int            chars)
{
    char line_buf[10] = {0};
    char column_buf[10] = {0};
    char chars_buf[10] = {0};
    char *text, *text2;

    if (line > 0 && column > 0)
    {
        g_snprintf (line_buf, sizeof line_buf, "%d", line);
        g_snprintf (column_buf, sizeof column_buf, "%d", column);
    }

    if (chars >= 0)
        g_snprintf (chars_buf, sizeof chars_buf, "%d", chars);

    text = g_strdup_printf (_("Line: %s Col: %s"), line_buf, column_buf);
    text2 = g_strdup_printf (_("Chars: %s"), chars_buf);

    gtk_label_set_text (window->priv->cursor_label, text);
    gtk_label_set_text (window->priv->chars_label, text2);

    g_free (text2);
    g_free (text);
}

static void
do_update_statusbar (MooEditWindow *window)
{
    MooEdit *doc;
    int line, column, chars;
    GtkTextIter iter;
    GtkTextBuffer *buffer;
    gboolean ovr;

    doc = ACTIVE_DOC (window);

    if (!doc)
    {
        gtk_widget_set_sensitive (window->priv->info, FALSE);
        set_statusbar_numbers (window, 0, 0, -1);
        return;
    }

    gtk_widget_set_sensitive (window->priv->info, TRUE);

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (doc));

    gtk_text_buffer_get_iter_at_mark (buffer, &iter,
                                      gtk_text_buffer_get_insert (buffer));
    line = gtk_text_iter_get_line (&iter) + 1;
    column = moo_text_iter_get_visual_line_offset (&iter, 8) + 1;
    chars = gtk_text_buffer_get_char_count (buffer);

    set_statusbar_numbers (window, line, column, chars);

    ovr = gtk_text_view_get_overwrite (GTK_TEXT_VIEW (doc));
    /* Label in the editor window statusbar - Overwrite or Insert mode */
    gtk_label_set_text (window->priv->insert_label, ovr ? _("OVR") : _("INS"));
}

static gboolean
update_statusbar_idle (MooEditWindow *window)
{
    window->priv->statusbar_idle = 0;
    do_update_statusbar (window);
    return FALSE;
}

static void
update_statusbar (MooEditWindow *window)
{
    if (!window->priv->statusbar_idle)
        window->priv->statusbar_idle =
            g_idle_add_full (G_PRIORITY_HIGH,
                             (GSourceFunc) update_statusbar_idle,
                             window, NULL);
    moo_window_message (MOO_WINDOW (window), NULL);
}

static void
create_statusbar (MooEditWindow *window)
{
    EditorStatusbarXml *xml;

    xml = editor_statusbar_xml_new ();

    gtk_container_add_with_properties (GTK_CONTAINER (MOO_WINDOW (window)->status_area),
                                       GTK_WIDGET (xml->EditorStatusbar),
                                       "pack-type", GTK_PACK_END,
                                       "expand", FALSE,
                                       "fill", FALSE,
                                       NULL);

    window->priv->cursor_label = xml->cursor;
    window->priv->chars_label = xml->chars;
    window->priv->insert_label = xml->insert;
    window->priv->info = GTK_WIDGET (xml->info);
}


/*****************************************************************************/
/* Language menu
 */

static int
cmp_langs (MooLang *lang1,
           MooLang *lang2)
{
    return g_utf8_collate (_moo_lang_display_name (lang1),
                           _moo_lang_display_name (lang2));
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
        moo_edit_config_set (doc->config, MOO_EDIT_CONFIG_SOURCE_USER,
                             "lang", lang_name, NULL);
}


static GtkAction*
create_lang_action (MooEditWindow *window)
{
    GtkAction *action;
    MooMenuMgr *menu_mgr;
    MooLangMgr *lang_mgr;
    GSList *langs, *sections, *l;

    lang_mgr = moo_lang_mgr_default ();

    /* TODO display names, etc. */
    sections = moo_lang_mgr_get_sections (lang_mgr);
    sections = g_slist_sort (sections, (GCompareFunc) strcmp);

    langs = moo_lang_mgr_get_available_langs (lang_mgr);
    langs = g_slist_sort (langs, (GCompareFunc) cmp_langs);

    action = moo_menu_action_new (LANG_ACTION_ID, _("_Language"));
    menu_mgr = moo_menu_action_get_mgr (MOO_MENU_ACTION (action));

    moo_menu_mgr_append (menu_mgr, NULL,
                         /* Menu item in the Language menu */
                         MOO_LANG_NONE, Q_("Language|None"), NULL,
                         MOO_MENU_ITEM_RADIO, NULL, NULL);

    for (l = sections; l != NULL; l = l->next)
        moo_menu_mgr_append (menu_mgr, NULL,
                             l->data, l->data, NULL,
                             0, NULL, NULL);

    for (l = langs; l != NULL; l = l->next)
    {
        MooLang *lang = l->data;
        if (!_moo_lang_get_hidden (lang))
            moo_menu_mgr_append (menu_mgr, _moo_lang_get_section (lang),
                                 _moo_lang_id (lang),
                                 _moo_lang_display_name (lang),
                                 NULL, MOO_MENU_ITEM_RADIO,
                                 g_strdup (_moo_lang_id (lang)),
                                 g_free);
    }

    g_signal_connect_swapped (menu_mgr, "radio-set-active",
                              G_CALLBACK (lang_item_activated), window);

    g_slist_foreach (langs, (GFunc) g_object_unref, NULL);
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
    GtkAction *action;
    MooLang *lang;

    doc = ACTIVE_DOC (window);

    if (!doc)
        return;

    lang = moo_text_view_get_lang (MOO_TEXT_VIEW (doc));
    action = moo_window_get_action (MOO_WINDOW (window), LANG_ACTION_ID);
    g_return_if_fail (action != NULL);

    moo_menu_mgr_set_active (moo_menu_action_get_mgr (MOO_MENU_ACTION (action)),
                             _moo_lang_id (lang), TRUE);
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
window_check_one_action (const char    *action_id,
                         ActionCheck   *set,
                         MooEditWindow *window,
                         MooEdit       *doc)
{
    MooActionCheckFunc func;
    GtkAction *action;
    gboolean visible = TRUE, sensitive = TRUE;

    action = moo_window_get_action (MOO_WINDOW (window), action_id);

    if (!action)
        return;

    if ((func = set->checks[MOO_ACTION_CHECK_ACTIVE].func))
    {
        if (!func (action, window, doc,
                   set->checks[MOO_ACTION_CHECK_ACTIVE].data))
        {
            visible = FALSE;
            sensitive = FALSE;
        }
    }

    if (visible && (func = set->checks[MOO_ACTION_CHECK_VISIBLE].func))
    {
        gpointer data = set->checks[MOO_ACTION_CHECK_VISIBLE].data;
        visible = func (action, window, doc, data);
    }

    if (sensitive && (func = set->checks[MOO_ACTION_CHECK_SENSITIVE].func))
    {
        gpointer data = set->checks[MOO_ACTION_CHECK_SENSITIVE].data;
        sensitive = func (action, window, doc, data);
    }

    g_object_set (action, "visible", visible, "sensitive", sensitive, NULL);
}


static void
check_action_hash_cb (const char    *action_id,
                      ActionCheck   *check,
                      gpointer       user_data)
{
    struct {
        MooEdit *doc;
        MooEditWindow *window;
    } *data = user_data;

    window_check_one_action (action_id, check, data->window, data->doc);
}


static void
moo_edit_window_check_actions (MooEditWindow *window)
{
    struct {
        MooEdit *doc;
        MooEditWindow *window;
    } data;

    data.window = window;
    data.doc = ACTIVE_DOC (window);

    g_hash_table_foreach (action_checks,
                          (GHFunc) check_action_hash_cb,
                          &data);
}


void
moo_edit_window_set_action_check (const char     *action_id,
                                  MooActionCheckType type,
                                  MooActionCheckFunc func,
                                  gpointer        data,
                                  GDestroyNotify  notify)
{
    ActionCheck *check;
    GSList *l;

    g_return_if_fail (action_id != NULL);
    g_return_if_fail (type < N_ACTION_CHECKS);
    g_return_if_fail (func != NULL);

    action_checks_init ();

    check = g_hash_table_lookup (action_checks, action_id);

    if (!check)
    {
        check = g_new0 (ActionCheck, 1);
        g_hash_table_insert (action_checks, g_strdup (action_id), check);
    }

    if (check->checks[type].func)
    {
        check->checks[type].func = NULL;

        if (check->checks[type].notify)
            check->checks[type].notify (check->checks[type].data);
    }

    check->checks[type].func = func;
    check->checks[type].data = data;
    check->checks[type].notify = notify;

    for (l = windows; l != NULL; l = l->next)
    {
        MooEditWindow *window = l->data;
        MooEdit *doc = ACTIVE_DOC (window);
        window_check_one_action (action_id, check, window, doc);
    }
}


static void
moo_edit_window_remove_action_check (const char        *action_id,
                                     MooActionCheckType type)
{
    ActionCheck *check;
    gboolean remove = TRUE;
    guint i;

    g_return_if_fail (action_id != NULL);
    g_return_if_fail (type <= N_ACTION_CHECKS);

    if (!action_checks)
        return;

    check = g_hash_table_lookup (action_checks, action_id);

    if (!check)
        return;

    if (type < N_ACTION_CHECKS)
    {
        for (i = 0; i < N_ACTION_CHECKS; ++i)
        {
            if (check->checks[i].func && i != type)
            {
                remove = FALSE;
                break;
            }
        }

        if (!remove && check->checks[type].func && check->checks[type].notify)
        {
            check->checks[type].func = NULL;
            check->checks[type].notify (check->checks[type].data);
        }
    }

    if (remove)
    {
        g_hash_table_remove (action_checks, action_id);

        for (i = 0; i < N_ACTION_CHECKS; ++i)
            if (check->checks[i].func && check->checks[i].notify)
                check->checks[i].notify (check->checks[i].data);

        g_free (check);
    }
}


GSList *
_moo_edit_parse_langs (const char *string)
{
    char **pieces, **p;
    GSList *list = NULL;

    if (!string)
        return NULL;

    pieces = g_strsplit_set (string, " \t\r\n;,", 0);

    if (!pieces)
        return NULL;

    for (p = pieces; *p != NULL; ++p)
    {
        g_strstrip (*p);

        if (**p)
            list = g_slist_prepend (list, _moo_lang_id_from_name (*p));
    }

    g_strfreev (pieces);
    return g_slist_reverse (list);
}


static gboolean
check_action_filter (G_GNUC_UNUSED GtkAction *action,
                     G_GNUC_UNUSED MooEditWindow *window,
                     MooEdit *doc,
                     gpointer filter)
{
    gboolean value = FALSE;

    if (doc)
        value = _moo_edit_filter_match (filter, doc);

    return value;
}

void
moo_edit_window_set_action_filter (const char        *action_id,
                                   MooActionCheckType type,
                                   const char        *filter_string)
{
    MooEditFilter *filter = NULL;

    g_return_if_fail (action_id != NULL);
    g_return_if_fail (type < N_ACTION_CHECKS);

    if (filter_string && filter_string[0])
        filter = _moo_edit_filter_new (filter_string);

    if (filter)
        moo_edit_window_set_action_check (action_id, type,
                                          check_action_filter,
                                          filter,
                                          (GDestroyNotify) _moo_edit_filter_free);
    else
        moo_edit_window_remove_action_check (action_id, type);
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

static void moo_edit_window_job_finished (MooEditWindow  *window,
                                          gpointer        job);
static void moo_edit_window_job_started  (MooEditWindow  *window,
                                          const char     *name,
                                          MooAbortJobFunc func,
                                          gpointer        job);


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


static void
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


static void
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
                                        GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
        gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_window),
                                             GTK_SHADOW_ETCHED_IN);

        cmd_view = moo_cmd_view_new ();
        moo_text_view_set_font_from_string (MOO_TEXT_VIEW (cmd_view), "Monospace");
        gtk_container_add (GTK_CONTAINER (scrolled_window), cmd_view);
        gtk_widget_show_all (scrolled_window);
        g_object_set_data (G_OBJECT (scrolled_window), "moo-output", cmd_view);

        label = moo_pane_label_new (MOO_STOCK_TERMINAL, NULL, "Output", "Output");

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
    g_return_val_if_fail (MOO_IS_EDIT_WINDOW (window), NULL);
    return moo_edit_window_get_pane (window, "moo-edit-window-output");
}


void
moo_edit_window_present_output (MooEditWindow *window)
{
    g_return_if_fail (MOO_IS_EDIT_WINDOW (window));
    moo_edit_window_get_output (window);
    moo_big_paned_present_pane (window->paned,
                                moo_edit_window_get_output_pane (window));
}


/************************************************************************/
/* Doc list
 */

static int
compare_docs_for_menu (MooEdit *doc1,
                       MooEdit *doc2)
{
    char *k1, *k2;
    int result;

    k1 = g_utf8_collate_key_for_filename (moo_edit_get_display_basename (doc1), -1);
    k2 = g_utf8_collate_key_for_filename (moo_edit_get_display_basename (doc2), -1);

    result = strcmp (k1, k2);

    if (!result)
    {
        MooEditWindow *window = moo_edit_get_window (doc1);
        result = get_page_num (window, doc1) - get_page_num (window, doc2);
    }

    g_free (k2);
    g_free (k1);
    return result;
}

static void
doc_menu_item_activated (MooEdit *doc)
{
    MooEditWindow *window;

    window = moo_edit_get_window (doc);
    g_return_if_fail (MOO_IS_EDIT_WINDOW (window));

    moo_edit_window_set_active_doc (window, doc);
}

static void
populate_window_menu (MooEditWindow *window,
                      GtkWidget     *menu,
                      GtkWidget     *no_docs_item)
{
    MooEdit *active_doc;
    GSList *docs;
    GList *children, *l;
    int pos;
    GtkWidget *item;

    children = g_list_copy (GTK_MENU_SHELL (menu)->children);
    pos = g_list_index (children, no_docs_item);
    g_return_if_fail (pos >= 0);

    for (l = g_list_nth (children, pos + 1); l != NULL; l = l->next)
    {
        if (g_object_get_data (l->data, "moo-document-menu-item"))
            gtk_container_remove (GTK_CONTAINER (menu), l->data);
        else
            break;
    }

    g_list_free (children);


    docs = moo_edit_window_list_docs (window);

    if (!docs)
        return;

    item = gtk_separator_menu_item_new ();
    g_object_set_data (G_OBJECT (item), "moo-document-menu-item", GINT_TO_POINTER (TRUE));
    gtk_widget_show (item);
    gtk_menu_shell_insert (GTK_MENU_SHELL (menu), item, ++pos);

    docs = g_slist_sort (docs, (GCompareFunc) compare_docs_for_menu);
    active_doc = ACTIVE_DOC (window);

    while (docs)
    {
        int idx;
        MooEdit *doc = docs->data;

        idx = get_page_num (window, doc);

        item = gtk_check_menu_item_new_with_label (moo_edit_get_display_basename (doc));
        _moo_widget_set_tooltip (item, moo_edit_get_display_name (doc));
        gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (item), doc == active_doc);
        gtk_check_menu_item_set_draw_as_radio (GTK_CHECK_MENU_ITEM (item), TRUE);
        g_object_set_data (G_OBJECT (item), "moo-document-menu-item", GINT_TO_POINTER (TRUE));
        gtk_widget_show (item);
        gtk_menu_shell_insert (GTK_MENU_SHELL (menu), item, ++pos);
        g_signal_connect_swapped (item, "activate", G_CALLBACK (doc_menu_item_activated), doc);

        if (idx < 9)
        {
            char *action_name = g_strdup_printf (DOCUMENT_ACTION "%u", idx + 1);
            GtkAction *action = moo_window_get_action (MOO_WINDOW (window), action_name);
            gtk_menu_item_set_accel_path (GTK_MENU_ITEM (item),
                                          gtk_action_get_accel_path (action));
            g_free (action_name);
        }

        docs = g_slist_delete_link (docs, docs);
    }
}

static void
window_menu_item_selected (MooWindow   *window,
                           GtkMenuItem *win_item)
{
    GtkWidget *menu;
    GtkWidget *no_docs_item;

    menu = gtk_menu_item_get_submenu (win_item);
    no_docs_item = moo_ui_xml_get_widget (moo_window_get_ui_xml (window),
                                          window->menubar,
                                          "Editor/Menubar/Window/NoDocuments");
    g_return_if_fail (menu != NULL && no_docs_item != NULL);

    populate_window_menu (MOO_EDIT_WINDOW (window), menu, no_docs_item);
}


static void
moo_edit_window_update_doc_list (MooEditWindow *window)
{
    MooEdit *doc;

    if (!window->priv->history_blocked &&
        (doc = ACTIVE_DOC (window)))
    {
        GList *link = g_list_find (window->priv->history, doc);

        if (link && link != window->priv->history)
        {
            window->priv->history = g_list_delete_link (window->priv->history, link);
            window->priv->history = g_list_prepend (window->priv->history, doc);
        }
        else if (!link)
        {
            window->priv->history = g_list_prepend (window->priv->history, doc);
            if (g_list_length (window->priv->history) > 2)
                window->priv->history = g_list_delete_link (window->priv->history,
                                                            g_list_last (window->priv->history));
        }
    }
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
        return FALSE;

    if (target == MOO_EDIT_TAB_ATOM)
        gtk_drag_get_data (widget, context, MOO_EDIT_TAB_ATOM, time);
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
    gboolean finished = FALSE;

    if (g_object_get_data (G_OBJECT (widget), "moo-edit-window-drop"))
    {
        g_object_set_data (G_OBJECT (widget), "moo-edit-window-drop", NULL);

        if (data->target == MOO_EDIT_TAB_ATOM)
        {
            GtkWidget *toplevel;
            MooEdit *doc = moo_selection_data_get_pointer (data, MOO_EDIT_TAB_ATOM);

            if (!doc)
                goto out;

            toplevel = gtk_widget_get_toplevel (GTK_WIDGET (doc));

            if (toplevel != GTK_WIDGET (window))
                _moo_editor_move_doc (window->priv->editor, doc, window, TRUE);

            goto out;
        }
        else if (data->target == moo_atom_uri_list ())
        {
            char **uris;
            char **u;

            /* XXX this is wrong but works. gtk_selection_data_get_uris()
             * does not work on windows */
            uris = g_uri_list_extract_uris ((char*) data->data);

            if (!uris)
                goto out;

            for (u = uris; *u; ++u)
            {
                char *filename = g_filename_from_uri (*u, NULL, NULL);
                if (!filename || !g_file_test (filename, G_FILE_TEST_IS_DIR))
                    moo_editor_open_uri (window->priv->editor, window,
                                         NULL, *u, NULL);
                g_free (filename);
            }

            g_strfreev (uris);
            gtk_drag_finish (context, TRUE, FALSE, time);
            finished = TRUE;
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
            finished = TRUE;
        }
    }
    else
    {
        if (info == TARGET_MOO_EDIT_TAB)
        {
            GtkWidget *toplevel;
            MooEdit *doc = moo_selection_data_get_pointer (data, MOO_EDIT_TAB_ATOM);

            if (!doc)
            {
                g_critical ("%s: oops", G_STRLOC);
                gdk_drag_status (context, 0, time);
                return;
            }

            toplevel = gtk_widget_get_toplevel (GTK_WIDGET (doc));

            if (toplevel == GTK_WIDGET (window))
            {
                gdk_drag_status (context, 0, time);
                return;
            }

            gdk_drag_status (context, GDK_ACTION_MOVE, time);
        }
        else
        {
            gdk_drag_status (context, 0, time);
        }

        return;
    }

out:
    if (!finished)
        gtk_drag_finish (context, FALSE, FALSE, time);
}
