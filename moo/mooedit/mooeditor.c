/*
 *   mooeditor.c
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

#define MOOEDIT_COMPILATION
#include "mooedit/mooeditor-private.h"
#include "mooedit/mooeditdialogs.h"
#include "mooedit/mooedit-fileops.h"
#include "mooedit/mooplugin.h"
#include "mooedit/mooeditprefs.h"
#include "mooedit/mooedit-private.h"
#include "mooedit/mooedit-accels.h"
#include "mooedit/moolangmgr.h"
#include "mooedit/mooeditfiltersettings.h"
#include "mooedit-ui.h"
#include "medit-ui.h"
#include "mooutils/moomenuaction.h"
#include "marshals.h"
#include "mooutils/mooutils-misc.h"
#include "mooutils/mooaction-private.h"
#include "mooutils/mooutils-gobject.h"
#include "mooutils/moofilewatch.h"
#include "mooutils/mooutils-fs.h"
#include "mooutils/moostock.h"
#include "mooutils/mooi18n.h"
#include "mooutils/mooencodings.h"
#include "mooutils/moolist.h"
#include <glib/gbase64.h>
#include <string.h>
#include <stdlib.h>

#define RECENT_ACTION_ID "OpenRecent"
#define RECENT_DIALOG_ACTION_ID "OpenRecentDialog"

#define CURRENT_SESSION_VERSION "2.0"

static gpointer editor_instance = NULL;

MOO_DEFINE_SLIST (DocList, doc_list, MooEdit)
MOO_DEFINE_SLIST (WindowList, window_list, MooEditWindow)

typedef enum {
    OPEN_SINGLE         = 1 << 0,
    ALLOW_EMPTY_WINDOW  = 1 << 1,
    SINGLE_WINDOW       = 1 << 2,
    SAVE_BACKUPS        = 1 << 3,
    STRIP_WHITESPACE    = 1 << 4,
    EMBEDDED            = 1 << 5
} MooEditorOptions;

struct MooEditorPrivate {
    DocList         *windowless;
    WindowList      *windows;
    char            *app_name;
    MooUiXml        *doc_ui_xml;
    MooUiXml        *ui_xml;
    MdHistoryMgr    *history;
    MooLangMgr      *lang_mgr;
    MooFileWatch    *file_watch;
    MooEditorOptions opts;

    MooEdit         *focused_doc;

    GType            window_type;
    GType            doc_type;

    char            *default_lang;

    guint            prefs_idle;
};

static void          set_single_window          (MooEditor      *editor,
                                                 gboolean        single);

static GtkAction    *create_open_recent_action  (MooWindow      *window,
                                                 gpointer        user_data);
static void          action_recent_dialog       (MooEditWindow  *window);

static MooEditWindow *create_window             (MooEditor      *editor);
static void          moo_editor_add_doc         (MooEditor      *editor,
                                                 MooEditWindow  *window,
                                                 MooEdit        *doc);
static gboolean      close_window_handler       (MooEditor      *editor,
                                                 MooEditWindow  *window,
                                                 gboolean        ask_confirm);
static void          do_close_window            (MooEditor      *editor,
                                                 MooEditWindow  *window);
static void          do_close_doc               (MooEditor      *editor,
                                                 MooEdit        *doc);
static gboolean      close_docs_real            (MooEditor      *editor,
                                                 DocList        *docs,
                                                 gboolean        ask_confirm);
static DocList      *find_modified              (DocList        *docs);

static void          add_new_window_action      (void);
static void          remove_new_window_action   (void);

static GObject      *moo_editor_constructor     (GType           type,
                                                 guint           n_props,
                                                 GObjectConstructParam *props);
static void          moo_editor_finalize        (GObject        *object);
static void          moo_editor_set_property    (GObject        *object,
                                                 guint           prop_id,
                                                 const GValue   *value,
                                                 GParamSpec     *pspec);
static void          moo_editor_get_property    (GObject        *object,
                                                 guint           prop_id,
                                                 GValue         *value,
                                                 GParamSpec     *pspec);


enum {
    PROP_0,
    PROP_OPEN_SINGLE_FILE_INSTANCE,
    PROP_ALLOW_EMPTY_WINDOW,
    PROP_SINGLE_WINDOW,
    PROP_SAVE_BACKUPS,
    PROP_STRIP_WHITESPACE,
    PROP_FOCUSED_DOC,
    PROP_EMBEDDED
};

enum {
    CLOSE_WINDOW,
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];


/* MOO_TYPE_EDITOR */
G_DEFINE_TYPE (MooEditor, moo_editor, G_TYPE_OBJECT)


inline static gboolean test_flag(MooEditor *editor, MooEditorOptions flag)
{
    return (editor->priv->opts & flag) != 0;
}

inline static gboolean is_embedded(MooEditor *editor)
{
    return test_flag(editor, EMBEDDED);
}

inline static void set_flag(MooEditor *editor, MooEditorOptions flag, gboolean set_or_not)
{
    if (set_or_not)
        editor->priv->opts |= flag;
    else
        editor->priv->opts &= ~flag;
}

static void
moo_editor_class_init (MooEditorClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    G_GNUC_UNUSED MooWindowClass *edit_window_class;

    gobject_class->constructor = moo_editor_constructor;
    gobject_class->finalize = moo_editor_finalize;
    gobject_class->set_property = moo_editor_set_property;
    gobject_class->get_property = moo_editor_get_property;

    klass->close_window = close_window_handler;

    _moo_edit_init_config ();
    g_type_class_unref (g_type_class_ref (MOO_TYPE_EDIT));
    g_type_class_unref (g_type_class_ref (MOO_TYPE_EDIT_WINDOW));

    g_type_class_add_private (klass, sizeof (MooEditorPrivate));

    g_object_class_install_property (gobject_class, PROP_OPEN_SINGLE_FILE_INSTANCE,
        g_param_spec_boolean ("open-single-file-instance", "open-single-file-instance", "open-single-file-instance",
                              TRUE, G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class, PROP_ALLOW_EMPTY_WINDOW,
        g_param_spec_boolean ("allow-empty-window", "allow-empty-window", "allow-empty-window",
                              FALSE, G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class, PROP_SINGLE_WINDOW,
        g_param_spec_boolean ("single-window", "single-window", "single-window",
                              FALSE, G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class, PROP_SAVE_BACKUPS,
        g_param_spec_boolean ("save-backups", "save-backups", "save-backups",
                              FALSE, G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class, PROP_STRIP_WHITESPACE,
        g_param_spec_boolean ("strip-whitespace", "strip-whitespace", "strip-whitespace",
                              FALSE, G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class, PROP_FOCUSED_DOC,
        g_param_spec_object ("focused-doc", "focused-doc", "focused-doc",
                             MOO_TYPE_EDIT, G_PARAM_READABLE));

    g_object_class_install_property (gobject_class, PROP_EMBEDDED,
        g_param_spec_boolean ("embedded", "embedded", "embedded",
                              FALSE, G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    signals[CLOSE_WINDOW] =
            g_signal_new ("close-window",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooEditorClass, close_window),
                          g_signal_accumulator_true_handled, NULL,
                          _moo_marshal_BOOLEAN__OBJECT_BOOLEAN,
                          G_TYPE_BOOLEAN, 2,
                          MOO_TYPE_EDIT_WINDOW,
                          G_TYPE_BOOLEAN);

    edit_window_class = g_type_class_ref (MOO_TYPE_EDIT_WINDOW);
    moo_window_class_new_action_custom (edit_window_class, RECENT_ACTION_ID, NULL,
                                        create_open_recent_action,
                                        NULL, NULL);
    moo_window_class_new_action (edit_window_class, RECENT_DIALOG_ACTION_ID, NULL,
                                 "display-name", _("Open Recent Files Dialog"),
                                 "label", Q_("Open Recent|_More..."),
                                 "closure-callback", action_recent_dialog,
                                 NULL);
    g_type_class_unref (edit_window_class);

    add_new_window_action ();
}


static void
moo_editor_init (MooEditor *editor)
{
    editor->priv = G_TYPE_INSTANCE_GET_PRIVATE (editor, MOO_TYPE_EDITOR, MooEditorPrivate);
}

static GObject *
moo_editor_constructor (GType                  type,
                        guint                  n_props,
                        GObjectConstructParam *props)
{
    MooEditor *editor;
    GObject *object;

    object = G_OBJECT_CLASS (moo_editor_parent_class)->constructor (type, n_props, props);
    editor = MOO_EDITOR (object);

    _moo_stock_init ();

    editor->priv->doc_ui_xml = moo_ui_xml_new ();
    moo_ui_xml_add_ui_from_string (editor->priv->doc_ui_xml,
                                   mooedit_ui_xml, -1);

    editor->priv->lang_mgr = g_object_ref (moo_lang_mgr_default ());
    g_signal_connect_swapped (editor->priv->lang_mgr, "loaded",
                              G_CALLBACK (moo_editor_apply_prefs),
                              editor);

    editor->priv->history = NULL;
    if (!is_embedded (editor))
        editor->priv->history = g_object_new (MD_TYPE_HISTORY_MGR,
                                              "name", "Editor",
                                              NULL);

    editor->priv->windows = NULL;
    editor->priv->windowless = NULL;

    moo_prefs_new_key_string (moo_edit_setting (MOO_EDIT_PREFS_DEFAULT_LANG),
                              MOO_LANG_NONE);

    _moo_edit_filter_settings_load ();
    moo_editor_apply_prefs (editor);

    return object;
}


static void
moo_editor_set_property (GObject        *object,
                         guint           prop_id,
                         const GValue   *value,
                         GParamSpec     *pspec)
{
    MooEditor *editor = MOO_EDITOR (object);

    switch (prop_id) {
        case PROP_OPEN_SINGLE_FILE_INSTANCE:
            set_flag (editor, OPEN_SINGLE, g_value_get_boolean (value));
            break;

        case PROP_ALLOW_EMPTY_WINDOW:
            set_flag (editor, ALLOW_EMPTY_WINDOW, g_value_get_boolean (value));
            break;

        case PROP_SINGLE_WINDOW:
            set_single_window (editor, g_value_get_boolean (value));
            break;

        case PROP_SAVE_BACKUPS:
            set_flag (editor, SAVE_BACKUPS, g_value_get_boolean (value));
            break;

        case PROP_STRIP_WHITESPACE:
            set_flag (editor, STRIP_WHITESPACE, g_value_get_boolean (value));
            break;

        case PROP_EMBEDDED:
            set_flag (editor, EMBEDDED, g_value_get_boolean (value));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}


static void
moo_editor_get_property (GObject        *object,
                         guint           prop_id,
                         GValue         *value,
                         GParamSpec     *pspec)
{
    MooEditor *editor = MOO_EDITOR (object);

    switch (prop_id) {
        case PROP_OPEN_SINGLE_FILE_INSTANCE:
            g_value_set_boolean (value, test_flag (editor, OPEN_SINGLE));
            break;

        case PROP_ALLOW_EMPTY_WINDOW:
            g_value_set_boolean (value, test_flag (editor, ALLOW_EMPTY_WINDOW));
            break;

        case PROP_SINGLE_WINDOW:
            g_value_set_boolean (value, test_flag (editor, SINGLE_WINDOW));
            break;

        case PROP_SAVE_BACKUPS:
            g_value_set_boolean (value, test_flag (editor, SAVE_BACKUPS));
            break;

        case PROP_STRIP_WHITESPACE:
            g_value_set_boolean (value, test_flag (editor, STRIP_WHITESPACE));
            break;

        case PROP_EMBEDDED:
            g_value_set_boolean (value, test_flag (editor, EMBEDDED));
            break;

        case PROP_FOCUSED_DOC:
            g_value_set_object (value, editor->priv->focused_doc);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}


static void
moo_editor_finalize (GObject *object)
{
    MooEditor *editor = MOO_EDITOR (object);

    g_free (editor->priv->app_name);

    if (editor->priv->ui_xml)
        g_object_unref (editor->priv->ui_xml);
    if (editor->priv->history)
        g_object_unref (editor->priv->history);
    g_object_unref (editor->priv->lang_mgr);
    g_object_unref (editor->priv->doc_ui_xml);

    if (editor->priv->file_watch)
    {
        GError *error = NULL;

        if (!moo_file_watch_close (editor->priv->file_watch, &error))
        {
            g_warning ("%s: error in moo_file_watch_close", G_STRLOC);
            if (error)
            {
                g_warning ("%s: %s", G_STRLOC, error->message);
                g_error_free (error);
            }
        }

        moo_file_watch_unref (editor->priv->file_watch);
    }

    if (editor->priv->windows)
        g_critical ("finalizing editor while some windows are open");
    if (editor->priv->windowless)
        g_critical ("finalizing editor while some documents are open");

    g_free (editor->priv->default_lang);

    if (editor->priv->prefs_idle)
        g_source_remove (editor->priv->prefs_idle);

    G_OBJECT_CLASS (moo_editor_parent_class)->finalize (object);
}


void
_moo_editor_set_focused_doc (MooEditor *editor,
                             MooEdit   *doc)
{
    g_return_if_fail (MOO_IS_EDITOR (editor));
    g_return_if_fail (MOO_IS_EDIT (doc));
    g_return_if_fail (doc->priv->editor == editor);

    if (editor->priv->focused_doc != doc)
    {
        editor->priv->focused_doc = doc;
        g_object_notify (G_OBJECT (editor), "focused-doc");
    }
}

void
_moo_editor_unset_focused_doc (MooEditor *editor,
                               MooEdit   *doc)
{
    g_return_if_fail (MOO_IS_EDITOR (editor));
    g_return_if_fail (MOO_IS_EDIT (doc));
    g_return_if_fail (doc->priv->editor == editor);

    if (editor->priv->focused_doc == doc)
    {
        editor->priv->focused_doc = NULL;
        g_object_notify (G_OBJECT (editor), "focused-doc");
    }
}


gpointer
_moo_editor_get_file_watch (MooEditor *editor)
{
    g_return_val_if_fail (MOO_IS_EDITOR (editor), NULL);

    if (!editor->priv->file_watch)
        editor->priv->file_watch = moo_file_watch_new (NULL);

    return editor->priv->file_watch;
}


MooEditor *
moo_editor_create_instance (gboolean embedded)
{
    if (!editor_instance)
    {
        editor_instance = g_object_new (MOO_TYPE_EDITOR,
                                        "embedded", embedded,
                                        NULL);
        g_object_add_weak_pointer (editor_instance, &editor_instance);
    }
    else
    {
        g_object_ref (editor_instance);
    }

    return editor_instance;
}


MooEditor*
moo_editor_instance (void)
{
    return editor_instance;
}


static GType
get_window_type (MooEditor *editor)
{
    return editor->priv->window_type ?
            editor->priv->window_type : MOO_TYPE_EDIT_WINDOW;
}


static GType
get_doc_type (MooEditor *editor)
{
    return editor->priv->doc_type ?
            editor->priv->doc_type : MOO_TYPE_EDIT;
}


static void
set_single_window (MooEditor      *editor,
                   gboolean        single)
{
    /* XXX move documents to some window if several windows open? */
    set_flag (editor, SINGLE_WINDOW, single);

    if (single)
        remove_new_window_action ();
    else
        add_new_window_action ();

    g_object_notify (G_OBJECT (editor), "single-window");
}


static void
add_new_window_action (void)
{
    MooWindowClass *klass;

    klass = g_type_class_peek (MOO_TYPE_EDIT_WINDOW);

    if (!moo_window_class_find_action (klass, "NewWindow"))
        moo_window_class_new_action (klass, "NewWindow", NULL,
                                     "display-name", MOO_STOCK_NEW_WINDOW,
                                     "label", MOO_STOCK_NEW_WINDOW,
                                     "tooltip", _("Open new editor window"),
                                     "stock-id", MOO_STOCK_NEW_WINDOW,
                                     "default-accel", MOO_EDIT_ACCEL_NEW_WINDOW,
                                     "closure-callback", moo_editor_new_window,
                                     "closure-proxy-func", moo_edit_window_get_editor,
                                     NULL);
}


static void
remove_new_window_action (void)
{
    MooWindowClass *klass;
    klass = g_type_class_peek (MOO_TYPE_EDIT_WINDOW);
    moo_window_class_remove_action (klass, "NewWindow");
}


static MooEditWindow *
get_top_window (MooEditor *editor)
{
    GtkWindow *window;

    if (!editor->priv->windows)
        return NULL;

    window = _moo_get_top_window (window_list_to_gslist (editor->priv->windows));

    if (!window)
        window = GTK_WINDOW (editor->priv->windows->data);

    return MOO_EDIT_WINDOW (window);
}


static void
file_info_list_free (GSList *list)
{
    g_slist_foreach (list, (GFunc) moo_edit_file_info_free, NULL);
    g_slist_free (list);
}


void
moo_editor_set_app_name (MooEditor  *editor,
                         const char *name)
{
    g_return_if_fail (MOO_IS_EDITOR (editor));
    MOO_ASSIGN_STRING (editor->priv->app_name, name);
    _moo_edit_window_update_title ();
}

const char *
moo_editor_get_app_name (MooEditor *editor)
{
    g_return_val_if_fail (MOO_IS_EDITOR (editor), NULL);

    if (!editor->priv->app_name)
        return g_get_prgname ();
    else
        return editor->priv->app_name;
}


MooUiXml *
moo_editor_get_ui_xml (MooEditor *editor)
{
    g_return_val_if_fail (MOO_IS_EDITOR (editor), NULL);

    if (!editor->priv->ui_xml)
    {
        editor->priv->ui_xml = moo_ui_xml_new ();
        moo_ui_xml_add_ui_from_string (editor->priv->ui_xml, medit_ui_xml, -1);
    }

    return editor->priv->ui_xml;
}


MooUiXml *
moo_editor_get_doc_ui_xml (MooEditor *editor)
{
    g_return_val_if_fail (MOO_IS_EDITOR (editor), NULL);
    return editor->priv->doc_ui_xml;
}


void
moo_editor_set_ui_xml (MooEditor      *editor,
                       MooUiXml       *xml)
{
    WindowList *l;

    g_return_if_fail (MOO_IS_EDITOR (editor));
    g_return_if_fail (MOO_IS_UI_XML (xml));

    if (editor->priv->ui_xml == xml)
        return;

    if (editor->priv->ui_xml)
        g_object_unref (editor->priv->ui_xml);

    editor->priv->ui_xml = xml;

    if (editor->priv->ui_xml)
        g_object_ref (editor->priv->ui_xml);

    for (l = editor->priv->windows; l != NULL; l = l->next)
        moo_window_set_ui_xml (MOO_WINDOW (l->data), editor->priv->ui_xml);
}


MdHistoryMgr *
_moo_editor_get_history_mgr (MooEditor *editor)
{
    g_return_val_if_fail (MOO_IS_EDITOR (editor), NULL);
    return editor->priv->history;
}

static void
add_recent_uri (MooEditor  *editor,
                const char *uri)
{
    if (!is_embedded (editor))
        md_history_mgr_add_uri (editor->priv->history, uri);
}

static void
add_recent_file (MooEditor *editor,
                 GFile     *file)
{
    if (!is_embedded (editor))
    {
        char *uri = g_file_get_uri (file);
        if (uri)
            md_history_mgr_add_uri (editor->priv->history, uri);
        g_free (uri);
    }
}

static void
recent_item_activated (GSList   *items,
                       gpointer  data)
{
    MooEditWindow *window = data;
    MooEditor *editor = moo_editor_instance ();

    g_return_if_fail (MOO_IS_EDIT_WINDOW (window));
    g_return_if_fail (MOO_IS_EDITOR (editor));

    while (items)
    {
        const char *encoding;
        const char *uri;
        char *filename;
        MdHistoryItem *item = items->data;

        uri = md_history_item_get_uri (item);
        filename = g_filename_from_uri (uri, NULL, NULL);
        g_return_if_fail (filename != NULL);

        encoding = _moo_edit_history_item_get_encoding (item);
        if (!moo_editor_open_uri (editor, window, GTK_WIDGET (window), uri, encoding))
            md_history_mgr_remove_uri (editor->priv->history, uri);

        g_free (filename);

        items = items->next;
    }
}

static GtkWidget *
create_recent_menu (GtkAction *action)
{
    GtkWidget *menu, *item;
    GtkAction *action_more;
    MooWindow *window;
    MooEditor *editor;

    window = _moo_action_get_window (action);
    g_return_val_if_fail (MOO_IS_EDIT_WINDOW (window), NULL);

    editor = moo_editor_instance ();
    menu = md_history_mgr_create_menu (editor->priv->history,
                                       recent_item_activated,
                                       window, NULL);
    moo_bind_bool_property (action,
                            "sensitive", editor->priv->history,
                            "empty", TRUE);

    item = gtk_separator_menu_item_new ();
    gtk_widget_show (item);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

    action_more = moo_window_get_action (window, RECENT_DIALOG_ACTION_ID);
    item = gtk_action_create_menu_item (action_more);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

    item = gtk_menu_item_new ();
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (item), menu);

    return item;
}

static GtkAction *
create_open_recent_action (G_GNUC_UNUSED MooWindow *window,
                           G_GNUC_UNUSED gpointer   user_data)
{
    GtkAction *action;

    action = moo_menu_action_new ("OpenRecent", _("Open Recent"));
    moo_menu_action_set_func (MOO_MENU_ACTION (action), create_recent_menu);

    return action;
}

static void
action_recent_dialog (MooEditWindow *window)
{
    GtkWidget *dialog;
    MooEditor *editor;

    editor = moo_editor_instance ();
    g_return_if_fail (MOO_IS_EDITOR (editor));

    dialog = md_history_mgr_create_dialog (editor->priv->history,
                                           recent_item_activated,
                                           window, NULL);
    gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (window));
    _moo_window_set_remember_size (GTK_WINDOW (dialog),
                                   moo_edit_setting (MOO_EDIT_PREFS_DIALOGS "/recent-files"),
                                   FALSE);

    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);
}


/*****************************************************************************/

static MooEditWindow *
create_window (MooEditor *editor)
{
    MooEditWindow *window = g_object_new (get_window_type (editor),
                                          "editor", editor,
                                          "ui-xml",
                                          moo_editor_get_ui_xml (editor),
                                          NULL);
    editor->priv->windows = window_list_append (editor->priv->windows, window);
    _moo_window_attach_plugins (window);
    gtk_widget_show (GTK_WIDGET (window));
    return window;
}


static void
moo_editor_add_doc (MooEditor      *editor,
                    MooEditWindow  *window,
                    MooEdit        *doc)
{
    if (!window)
        editor->priv->windowless = doc_list_prepend (editor->priv->windowless, doc);

    if (moo_edit_is_untitled (doc) &&
        !moo_edit_config_get_string (doc->config, "lang") &&
        editor->priv->default_lang)
    {
        moo_edit_config_set (doc->config, MOO_EDIT_CONFIG_SOURCE_FILENAME,
                             "lang", editor->priv->default_lang, NULL);
    }

    _moo_edit_apply_prefs (doc);
}


MooEditWindow *
moo_editor_new_window (MooEditor *editor)
{
    MooEditWindow *window;
    MooEdit *doc;

    g_return_val_if_fail (MOO_IS_EDITOR (editor), NULL);

    window = create_window (editor);

    if (!test_flag (editor, ALLOW_EMPTY_WINDOW))
    {
        doc = g_object_new (get_doc_type (editor), "editor", editor, NULL);
        _moo_edit_window_insert_doc (window, doc, -1);
        moo_editor_add_doc (editor, window, doc);
    }

    return window;
}


/* this creates MooEdit instance which can not be put into a window */
MooEdit *
moo_editor_create_doc (MooEditor      *editor,
                       const char     *filename,
                       const char     *encoding,
                       GError        **error)
{
    MooEdit *doc;
    GFile *file = NULL;

    g_return_val_if_fail (MOO_IS_EDITOR (editor), NULL);

    doc = g_object_new (get_doc_type (editor), "editor", editor, NULL);

    if (filename)
        file = g_file_new_for_path (filename);

    if (file && !_moo_edit_load_file (doc, file, encoding, error))
    {
        MOO_OBJECT_REF_SINK (doc);
        g_object_unref (file);
        g_object_unref (doc);
        return NULL;
    }

    moo_editor_add_doc (editor, NULL, doc);
    _moo_doc_attach_plugins (NULL, doc);

    if (file)
        g_object_unref (file);

    return doc;
}


MooEdit *
moo_editor_new_doc (MooEditor      *editor,
                    MooEditWindow  *window)
{
    MooEdit *doc;

    g_return_val_if_fail (MOO_IS_EDITOR (editor), NULL);
    g_return_val_if_fail (!window || MOO_IS_EDIT_WINDOW (window), NULL);

    if (!window)
        window = get_top_window (editor);

    if (!window)
        window = moo_editor_new_window (editor);

    g_return_val_if_fail (window != NULL, NULL);

    doc = g_object_new (get_doc_type (editor), "editor", editor, NULL);
    _moo_edit_window_insert_doc (window, doc, -1);
    moo_editor_add_doc (editor, window, doc);

    return doc;
}


void
_moo_editor_move_doc (MooEditor     *editor,
                      MooEdit       *doc,
                      MooEditWindow *dest,
                      gboolean       focus)
{
    MooEditWindow *old_window;
    MooEdit *old_doc;
    int new_pos = -1;

    g_return_if_fail (MOO_IS_EDITOR (editor));
    g_return_if_fail (MOO_IS_EDIT (doc) && doc->priv->editor == editor);
    g_return_if_fail (!dest || (MOO_IS_EDIT_WINDOW (dest) && moo_edit_window_get_editor (dest) == editor));

    if (!dest)
        dest = moo_editor_new_window (editor);

    g_object_ref (doc);

    if ((old_window = moo_edit_get_window (doc)))
    {
        _moo_edit_window_remove_doc (old_window, doc, FALSE);

        if (!moo_edit_window_get_active_doc (old_window))
            moo_editor_close_window (editor, old_window, FALSE);
    }

    old_doc = moo_edit_window_get_active_doc (dest);

    if (old_doc && moo_edit_is_empty (old_doc))
        new_pos = _moo_edit_window_get_doc_no (dest, old_doc);
    else
        old_doc = NULL;

    _moo_edit_window_insert_doc (dest, doc, new_pos);
    moo_editor_add_doc (editor, dest, doc);

    if (old_doc)
        moo_editor_close_doc (editor, old_doc, FALSE);

    if (focus)
        moo_editor_set_active_doc (editor, doc);

    g_object_unref (doc);
}


static gboolean
moo_editor_load_file (MooEditor       *editor,
                      MooEditWindow   *window,
                      GtkWidget       *parent,
                      MooEditFileInfo *info,
                      gboolean         silent,
                      gboolean         add_history,
                      int              line,
                      MooEdit        **docp)
{
    GError *error = NULL;
    gboolean new_doc = FALSE;
    MooEdit *doc = NULL;
    char *uri;
    gboolean result = TRUE;

    *docp = NULL;
    uri = g_file_get_uri (info->file);
    g_return_val_if_fail (uri != NULL, FALSE);

    if ((doc = moo_editor_get_doc_for_uri (editor, uri)))
    {
        *docp = doc;
        if (add_history)
            add_recent_uri (editor, uri);
        g_free (uri);
        return FALSE;
    }

    if (window)
    {
        doc = moo_edit_window_get_active_doc (window);

        if (doc && moo_edit_is_empty (doc))
            g_object_ref (doc);
        else
            doc = NULL;
    }

    if (!doc)
    {
        doc = g_object_new (get_doc_type (editor), "editor", editor, NULL);
        MOO_OBJECT_REF_SINK (doc);
        new_doc = TRUE;
    }

    /* XXX open_single */
    if (!_moo_edit_load_file (doc, info->file, info->encoding, &error))
    {
        if (!silent)
        {
            if (!parent && !window)
                window = moo_editor_get_active_window (editor);
            if (!parent && window)
                parent = GTK_WIDGET (window);
            _moo_edit_open_error_dialog (parent, info->file, info->encoding, error);
        }
        g_error_free (error);
        result = FALSE;
    }
    else
    {
        MdHistoryItem *hist_item;

        if (line < 0)
        {
            hist_item = md_history_mgr_find_uri (editor->priv->history, uri);
            if (hist_item)
                line = _moo_edit_history_item_get_line (hist_item);
        }

        if (line >= 0)
            moo_text_view_move_cursor (MOO_TEXT_VIEW (doc), line, 0, FALSE, TRUE);

        if (!window)
            window = moo_editor_get_active_window (editor);
        if (!window)
            window = create_window (editor);

        if (new_doc)
        {
            _moo_edit_window_insert_doc (window, doc, -1);
            moo_editor_add_doc (editor, window, doc);
        }

        if (add_history)
            add_recent_uri (editor, uri);
    }

    if (result)
        *docp = doc;

    g_free (uri);
    g_object_unref (doc);
    return result;
}

gboolean
moo_editor_open (MooEditor      *editor,
                 MooEditWindow  *window,
                 GtkWidget      *parent,
                 GSList         *files)
{
    GSList *l;
    MooEdit *bring_to_front = NULL;
    gboolean result = TRUE;

    g_return_val_if_fail (MOO_IS_EDITOR (editor), FALSE);
    g_return_val_if_fail (!window || MOO_IS_EDIT_WINDOW (window), FALSE);
    g_return_val_if_fail (!parent || GTK_IS_WIDGET (parent), FALSE);

    if (window && !parent)
        parent = GTK_WIDGET (window);

    if (!files)
    {
        MooEdit *active = window ? moo_edit_window_get_active_doc (window) : NULL;

        files = _moo_edit_open_dialog (parent, active);

        if (!files)
            return FALSE;

        result = moo_editor_open (editor, window, parent, files);
        file_info_list_free (files);

        return result;
    }

    bring_to_front = NULL;

    for (l = files; l != NULL; l = l->next)
    {
        MooEditFileInfo *info = l->data;
        MooEdit *doc = NULL;

        if (!window)
            window = moo_editor_get_active_window (editor);

        if (moo_editor_load_file (editor, window, parent, info,
                                  is_embedded (editor), TRUE, -1, &doc))
            parent = GTK_WIDGET (doc);

        if (doc)
            bring_to_front = doc;
    }

    if (bring_to_front)
    {
        moo_editor_set_active_doc (editor, bring_to_front);
        gtk_widget_grab_focus (GTK_WIDGET (bring_to_front));
    }

    return result;
}


MooEdit*
moo_editor_get_active_doc (MooEditor *editor)
{
    MooEditWindow *window = moo_editor_get_active_window (editor);
    return window ? moo_edit_window_get_active_doc (window) : NULL;
}


MooEditWindow*
moo_editor_get_active_window (MooEditor *editor)
{
    g_return_val_if_fail (MOO_IS_EDITOR (editor), NULL);
    return get_top_window (editor);
}


void
moo_editor_present (MooEditor *editor,
                    guint32    stamp)
{
    MooEditWindow *window;

    g_return_if_fail (MOO_IS_EDITOR (editor));

    window = moo_editor_get_active_window (editor);

    if (!window)
        window = moo_editor_new_window (editor);

    g_return_if_fail (window != NULL);
    moo_window_present (GTK_WINDOW (window), stamp);
}


void
moo_editor_set_active_window (MooEditor      *editor,
                              MooEditWindow  *window)
{
    g_return_if_fail (MOO_IS_EDITOR (editor));
    g_return_if_fail (MOO_IS_EDIT_WINDOW (window));
    gtk_window_present (GTK_WINDOW (window));
}


void
moo_editor_set_active_doc (MooEditor      *editor,
                           MooEdit        *doc)
{
    MooEditWindow *window;

    g_return_if_fail (MOO_IS_EDITOR (editor));
    g_return_if_fail (MOO_IS_EDIT (doc));

    window = moo_edit_get_window (doc);
    g_return_if_fail (MOO_IS_EDIT_WINDOW (window));

    moo_window_present (GTK_WINDOW (window), 0);
    moo_edit_window_set_active_doc (window, doc);
}


static MooEdit *
find_busy (DocList *docs)
{
    while (docs)
    {
        if (MOO_EDIT_IS_BUSY (docs->data))
            return docs->data;
        docs = docs->next;
    }

    return NULL;
}


inline static DocList *
window_list_docs (MooEditWindow *window)
{
    return doc_list_from_gslist (moo_edit_window_list_docs (window));
}


static gboolean
close_window_handler (MooEditor     *editor,
                      MooEditWindow *window,
                      gboolean       ask_confirm)
{
    MooSaveChangesDialogResponse response;
    DocList *modified;
    DocList *docs;
    gboolean do_close = FALSE;
    MooEdit *busy = NULL;

    g_return_val_if_fail (MOO_IS_EDITOR (editor), FALSE);
    g_return_val_if_fail (MOO_IS_EDIT_WINDOW (window), FALSE);

    docs = window_list_docs (window);
    busy = find_busy (docs);

    if (busy)
    {
        moo_editor_set_active_doc (editor, busy);
        doc_list_free (docs);
        return TRUE;
    }

    modified = find_modified (docs);

    if (!modified || !ask_confirm)
    {
        do_close = TRUE;
    }
    else if (!modified->next)
    {
        if (window)
            moo_edit_window_set_active_doc (window, modified->data);

        response = _moo_edit_save_changes_dialog (modified->data);

        switch (response)
        {
            case MOO_SAVE_CHANGES_RESPONSE_SAVE:
                if (_moo_editor_save (editor, modified->data, NULL))
                    do_close = TRUE;
                break;

            case MOO_SAVE_CHANGES_RESPONSE_CANCEL:
                break;

            default:
                do_close = TRUE;
                break;
        }
    }
    else
    {
        GSList *to_save = NULL, *l;
        gboolean saved = TRUE;

        response = _moo_edit_save_multiple_changes_dialog (doc_list_to_gslist (modified), &to_save);

        switch (response)
        {
            case MOO_SAVE_CHANGES_RESPONSE_SAVE:
                for (l = to_save; l != NULL; l = l->next)
                    if (!_moo_editor_save (editor, l->data, NULL))
                    {
                        saved = FALSE;
                        break;
                    }

                if (saved)
                    do_close = TRUE;

                g_slist_free (to_save);
                break;

            case MOO_SAVE_CHANGES_RESPONSE_CANCEL:
                break;

            default:
                do_close = TRUE;
                break;
        }
    }

    doc_list_free (modified);
    doc_list_free (docs);
    return !do_close;
}


gboolean
moo_editor_close_window (MooEditor      *editor,
                         MooEditWindow  *window,
                         gboolean        ask_confirm)
{
    gboolean stopped = FALSE;

    g_return_val_if_fail (MOO_IS_EDITOR (editor), FALSE);
    g_return_val_if_fail (MOO_IS_EDIT_WINDOW (window), FALSE);

    if (!window_list_find (editor->priv->windows, window))
        return TRUE;

    g_object_ref (window);

    g_signal_emit (editor, signals[CLOSE_WINDOW], 0, window, ask_confirm, &stopped);

    if (!stopped && window_list_find (editor->priv->windows, window))
        do_close_window (editor, window);

    g_object_unref (window);

    return !stopped;
}


static void
do_close_window (MooEditor      *editor,
                 MooEditWindow  *window)
{
    DocList *l, *list;

    list = window_list_docs (window);

    for (l = list; l != NULL; l = l->next)
        do_close_doc (editor, l->data);

    editor->priv->windows =
        window_list_remove (editor->priv->windows, window);

    _moo_window_detach_plugins (window);
    gtk_widget_destroy (GTK_WIDGET (window));

    doc_list_free (list);
}


static void
do_close_doc (MooEditor *editor,
              MooEdit   *doc)
{
    char *uri;
    MooEditWindow *window;

    window = moo_edit_get_window (doc);

    if (!window)
    {
        g_assert (doc_list_find (editor->priv->windowless, doc));
        editor->priv->windowless = doc_list_remove (editor->priv->windowless, doc);
    }

    if (!is_embedded (editor) && (uri = moo_edit_get_uri (doc)))
    {
        MdHistoryItem *item;
        int line;
        const char *enc;

        item = md_history_item_new (uri, NULL);

        line = moo_text_view_get_cursor_line (MOO_TEXT_VIEW (doc));
        if (line != 0)
            _moo_edit_history_item_set_line (item, line);

        enc = moo_edit_get_encoding (doc);
        if (enc && !_moo_encodings_equal (enc, MOO_ENCODING_UTF8))
            _moo_edit_history_item_set_encoding (item, enc);

        md_history_mgr_update_file (editor->priv->history, item);
        md_history_item_free (item);
        g_free (uri);
    }

    if (window)
        _moo_edit_window_remove_doc (window, doc, TRUE);
    else
        _moo_doc_detach_plugins (NULL, doc);
}


gboolean
moo_editor_close_doc (MooEditor      *editor,
                      MooEdit        *doc,
                      gboolean        ask_confirm)
{
    gboolean result;
    GSList *list;

    list = g_slist_prepend (NULL, doc);
    result = moo_editor_close_docs (editor, list, ask_confirm);

    g_slist_free (list);
    return result;
}


gboolean
moo_editor_close_docs (MooEditor *editor,
                       GSList    *list,
                       gboolean   ask_confirm)
{
    GSList *l;
    MooEditWindow *window;

    g_return_val_if_fail (MOO_IS_EDITOR (editor), FALSE);

    if (!list)
        return TRUE;

    for (l = list; l != NULL; l = l->next)
    {
        MooEdit *doc = l->data;
        g_return_val_if_fail (MOO_IS_EDIT (doc), FALSE);

        if (MOO_EDIT_IS_BUSY (doc))
        {
            moo_editor_set_active_doc (editor, doc);
            return FALSE;
        }
    }

    window = moo_edit_get_window (list->data);

    if (close_docs_real (editor, doc_list_from_gslist (list), ask_confirm))
    {
        if (window &&
            !moo_edit_window_num_docs (window) &&
            !test_flag (editor, ALLOW_EMPTY_WINDOW))
        {
            MooEdit *doc = g_object_new (get_doc_type (editor),
                                         "editor", editor, NULL);
            _moo_edit_window_insert_doc (window, doc, -1);
            moo_editor_add_doc (editor, window, doc);
        }

        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


static gboolean
close_docs_real (MooEditor      *editor,
                 DocList        *docs,
                 gboolean        ask_confirm)
{
    MooSaveChangesDialogResponse response;
    DocList *modified;
    gboolean do_close = FALSE;

    modified = find_modified (docs);

    if (!modified || !ask_confirm)
    {
        do_close = TRUE;
    }
    else if (!modified->next)
    {
        MooEditWindow *window = moo_edit_get_window (modified->data);

        if (window)
            moo_edit_window_set_active_doc (window, modified->data);

        response = _moo_edit_save_changes_dialog (modified->data);

        switch (response)
        {
            case MOO_SAVE_CHANGES_RESPONSE_SAVE:
                if (_moo_editor_save (editor, modified->data, NULL))
                    do_close = TRUE;
                break;

            case MOO_SAVE_CHANGES_RESPONSE_CANCEL:
                break;

            default:
                do_close = TRUE;
                break;
        }
    }
    else
    {
        GSList *l;
        GSList *to_save = NULL;
        gboolean saved = TRUE;

        response = _moo_edit_save_multiple_changes_dialog (doc_list_to_gslist (modified), &to_save);

        switch (response)
        {
            case MOO_SAVE_CHANGES_RESPONSE_SAVE:
                for (l = to_save; l != NULL; l = l->next)
                    if (!_moo_editor_save (editor, l->data, NULL))
                {
                    saved = FALSE;
                    break;
                }

                if (saved)
                    do_close = TRUE;

                g_slist_free (to_save);
                break;

            case MOO_SAVE_CHANGES_RESPONSE_CANCEL:
                break;

            default:
                do_close = TRUE;
                break;
        }
    }

    if (do_close)
    {
        DocList *l;
        for (l = docs; l != NULL; l = l->next)
            do_close_doc (editor, l->data);
    }

    doc_list_free (modified);
    return do_close;
}


static DocList*
find_modified (DocList *docs)
{
    DocList *modified = NULL, *l;
    for (l = docs; l != NULL; l = l->next)
        if (MOO_EDIT_IS_MODIFIED (l->data) && !MOO_EDIT_IS_CLEAN (l->data))
            modified = doc_list_prepend (modified, l->data);
    return doc_list_reverse (modified);
}


/* XXX ask in each window, then close */
gboolean
moo_editor_close_all (MooEditor *editor,
                      gboolean   ask_confirm,
                      gboolean   leave_one)
{
    GSList *windows, *l;

    g_return_val_if_fail (MOO_IS_EDITOR (editor), FALSE);

    windows = moo_editor_list_windows (editor);

    for (l = windows; l != NULL; l = l->next)
    {
        gboolean closed = FALSE;

        if (l->next || !leave_one || !ask_confirm)
        {
            closed = moo_editor_close_window (editor, l->data, ask_confirm);
        }
        else
        {
            GSList *docs = moo_edit_window_list_docs (l->data);
            closed = moo_editor_close_docs (editor, docs, ask_confirm);
            g_slist_free (docs);
        }

        if (!closed)
        {
            g_slist_free (windows);
            return FALSE;
        }
    }

    g_slist_free (windows);
    return TRUE;
}


/* Remove after March 2009 */
static char *
filename_from_utf8 (const char *encoded)
{
    if (g_str_has_prefix (encoded, "base64"))
    {
        guchar *filename;
        gsize len;

        filename = g_base64_decode (encoded + strlen ("base64"), &len);

        if (!filename || !len || filename[len-1] != 0)
        {
            g_critical ("%s: oops", G_STRLOC);
            return NULL;
        }

        return (char*) filename;
    }
    else
    {
        return g_strdup (encoded);
    }
}

static MooEdit *
load_doc_session (MooEditor     *editor,
                  MooEditWindow *window,
                  MooMarkupNode *elm,
                  gboolean       file_is_uri)
{
    const char *uri = NULL;
    const char *encoding;
    char *freeme = NULL;
    MooEdit *doc = NULL;
    MooEditFileInfo *info;

    if (file_is_uri)
    {
        uri = moo_markup_get_content (elm);
    }
    else
    {
        const char *filename_utf8 = moo_markup_get_content (elm);
        char *filename = filename_from_utf8 (filename_utf8);
        if (filename)
        {
            freeme = g_filename_to_uri (filename, NULL, NULL);
            uri = freeme;
        }
        g_free (filename);
    }

    if (!uri || !uri[0])
    {
        g_free (freeme);
        return moo_editor_new_doc (editor, window);
    }

    encoding = moo_markup_get_prop (elm, "encoding");
    info = moo_edit_file_info_new_uri (uri, encoding);

    moo_editor_load_file (editor, window, GTK_WIDGET (window), info, TRUE, FALSE, -1, &doc);

    moo_edit_file_info_free (info);
    g_free (freeme);
    return doc;
}

static MooMarkupNode *
save_doc_session (MooEdit       *doc,
                  MooMarkupNode *elm)
{
    char *uri;
    const char *encoding;
    MooMarkupNode *node;

    uri = moo_edit_get_uri (doc);
    encoding = moo_edit_get_encoding (doc);

    if (uri)
    {
        node = moo_markup_create_text_element (elm, "document", uri);

        if (encoding && encoding[0])
            moo_markup_set_prop (node, "encoding", encoding);
    }
    else
    {
        node = moo_markup_create_element (elm, "document");
    }

    g_free (uri);
    return node;
}

static MooEditWindow *
load_window_session (MooEditor     *editor,
                     MooMarkupNode *elm,
                     gboolean       file_is_uri)
{
    MooEditWindow *window;
    MooEdit *active_doc = NULL;
    MooMarkupNode *node;

    window = create_window (editor);

    for (node = elm->children; node != NULL; node = node->next)
    {
        if (MOO_MARKUP_IS_ELEMENT (node))
        {
            MooEdit *doc;

            doc = load_doc_session (editor, window, node, file_is_uri);

            if (doc && moo_markup_get_bool_prop (node, "active", FALSE))
                active_doc = doc;
        }
    }

    if (active_doc)
        moo_edit_window_set_active_doc (window, active_doc);

    return window;
}

static MooMarkupNode *
save_window_session (MooEditWindow *window,
                     MooMarkupNode *elm)
{
    MooMarkupNode *node;
    MooEdit *active_doc;
    DocList *docs;

    active_doc = moo_edit_window_get_active_doc (window);
    docs = window_list_docs (window);

    node = moo_markup_create_element (elm, "window");

    while (docs)
    {
        MooMarkupNode *doc_node;
        MooEdit *doc = docs->data;

        doc_node = save_doc_session (doc, node);

        if (doc_node && doc == active_doc)
            moo_markup_set_bool_prop (doc_node, "active", TRUE);

        docs = doc_list_delete_link (docs, docs);
    }

    return node;
}

void
_moo_editor_load_session (MooEditor     *editor,
                          MooMarkupNode *xml)
{
    MooMarkupNode *editor_node;
    gboolean old_format = FALSE;

    g_return_if_fail (MOO_IS_EDITOR (editor));
    g_return_if_fail (MOO_MARKUP_IS_ELEMENT (xml));

    editor_node = moo_markup_get_element (xml, "editor");

    if (editor_node)
    {
        const char *version = moo_markup_get_prop (editor_node, "version");
        if (!version)
            old_format = TRUE;
        else if (strcmp (version, CURRENT_SESSION_VERSION) != 0)
            editor_node = NULL;
    }

    if (editor_node)
    {
        MooEditWindow *active_window = NULL;
        MooMarkupNode *node;

        for (node = editor_node->children; node != NULL; node = node->next)
        {
            MooEditWindow *window;

            if (!MOO_MARKUP_IS_ELEMENT (node))
                continue;

            window = load_window_session (editor, node, !old_format);

            if (window && moo_markup_get_bool_prop (node, "active", FALSE))
                active_window = window;
        }

        if (active_window)
            moo_editor_set_active_window (editor, active_window);
    }
}

void
_moo_editor_save_session (MooEditor     *editor,
                          MooMarkupNode *xml)
{
    MooMarkupNode *node;
    MooEditWindow *active_window;
    GSList *windows;

    g_return_if_fail (MOO_IS_EDITOR (editor));
    g_return_if_fail (MOO_MARKUP_IS_ELEMENT (xml));

    active_window = moo_editor_get_active_window (editor);
    windows = moo_editor_list_windows (editor);

    node = moo_markup_create_element (xml, "editor");
    moo_markup_set_prop (node, "version", CURRENT_SESSION_VERSION);

    while (windows)
    {
        MooEditWindow *window = windows->data;
        MooMarkupNode *window_node;

        window_node = save_window_session (window, node);

        if (window_node && window == active_window)
            moo_markup_set_bool_prop (window_node, "active", TRUE);

        windows = g_slist_delete_link (windows, windows);
    }
}


GSList *
moo_editor_list_windows (MooEditor *editor)
{
    g_return_val_if_fail (MOO_IS_EDITOR (editor), NULL);
    return window_list_to_gslist (window_list_copy (editor->priv->windows));
}


GSList *
moo_editor_list_docs (MooEditor *editor)
{
    DocList *docs = NULL, *list;
    WindowList *l;

    g_return_val_if_fail (MOO_IS_EDITOR (editor), NULL);

    for (l = editor->priv->windows; l != NULL; l = l->next)
    {
        list = window_list_docs (l->data);
        list = doc_list_reverse (list);
        docs = doc_list_concat (list, docs);
    }

    list = doc_list_copy (editor->priv->windowless);
    list = doc_list_reverse (list);
    docs = doc_list_concat (list, docs);

    return g_slist_reverse (doc_list_to_gslist (docs));
}


MooEdit *
moo_editor_open_file (MooEditor      *editor,
                      MooEditWindow  *window,
                      GtkWidget      *parent,
                      const char     *filename,
                      const char     *encoding)
{
    gboolean result;

    g_return_val_if_fail (MOO_IS_EDITOR (editor), NULL);
    g_return_val_if_fail (!window || MOO_IS_EDIT_WINDOW (window), NULL);
    g_return_val_if_fail (!parent || GTK_IS_WIDGET (parent), NULL);

    if (!filename)
    {
        result = moo_editor_open (editor, window, parent, NULL);
    }
    else
    {
        MooEditFileInfo *info;
        GSList *list;

        info = moo_edit_file_info_new_path (filename, encoding);
        list = g_slist_prepend (NULL, info);

        result = moo_editor_open (editor, window, parent, list);

        moo_edit_file_info_free (info);
        g_slist_free (list);
    }

    if (!result)
        return NULL;

    return moo_editor_get_doc (editor, filename);
}


MooEdit *
moo_editor_open_file_line (MooEditor      *editor,
                           const char     *filename,
                           int             line,
                           MooEditWindow  *window)
{
    MooEdit *doc = NULL;
    char *freeme = NULL;
    MooEditFileInfo *info = NULL;

    g_return_val_if_fail (MOO_IS_EDITOR (editor), NULL);
    g_return_val_if_fail (filename != NULL, NULL);

    doc = moo_editor_get_doc (editor, filename);

    if (doc)
    {
        if (line >= 0)
            moo_text_view_move_cursor (MOO_TEXT_VIEW (doc), line, 0, FALSE, FALSE);
        moo_editor_set_active_doc (editor, doc);
        gtk_widget_grab_focus (GTK_WIDGET (doc));
        return doc;
    }

    freeme = _moo_normalize_file_path (filename);
    filename = freeme;

    if (!g_file_test (filename, G_FILE_TEST_EXISTS))
        goto out;

    info = moo_edit_file_info_new_path (filename, NULL);
    moo_editor_load_file (editor, window, NULL, info,
                          is_embedded (editor),
                          TRUE, line, &doc);

    /* XXX */
    moo_editor_set_active_doc (editor, doc);
    if (line >= 0)
        moo_text_view_move_cursor (MOO_TEXT_VIEW (doc), line, 0, FALSE, TRUE);
    gtk_widget_grab_focus (GTK_WIDGET (doc));

out:
    moo_edit_file_info_free (info);
    g_free (freeme);
    return doc;
}


static MooEdit *
moo_editor_new_uri (MooEditor     *editor,
                    MooEditWindow *window,
                    GtkWidget     *parent,
                    const char    *uri,
                    const char    *encoding)
{
    MooEdit *doc;
    char *path;
    GFile *file;

    g_return_val_if_fail (MOO_IS_EDITOR (editor), NULL);
    g_return_val_if_fail (!window || MOO_IS_EDIT_WINDOW (window), NULL);
    g_return_val_if_fail (!parent || GTK_IS_WIDGET (parent), NULL);

    if (!uri)
        return moo_editor_open_uri (editor, window, parent, NULL, NULL);

    file = g_file_new_for_uri (uri);
    path = g_file_is_native (file) ? g_file_get_path (file) : NULL;

    if (path && g_file_test (path, G_FILE_TEST_EXISTS))
    {
        g_free (path);
        g_object_unref (file);
        return moo_editor_open_uri (editor, window, parent,
                                    uri, encoding);
    }

    if (!window)
        window = moo_editor_get_active_window (editor);

    if (window)
    {
        doc = moo_edit_window_get_active_doc (window);

        if (!doc || !moo_edit_is_empty (doc))
            doc = NULL;
    }

    if (!doc)
        doc = moo_editor_new_doc (editor, window);

    doc->priv->status = MOO_EDIT_NEW;
    _moo_edit_set_file (doc, file, encoding);
    moo_editor_set_active_doc (editor, doc);
    gtk_widget_grab_focus (GTK_WIDGET (doc));

    g_free (path);
    g_object_unref (file);
    return doc;
}

MooEdit *
moo_editor_new_file (MooEditor      *editor,
                     MooEditWindow  *window,
                     GtkWidget      *parent,
                     const char     *filename,
                     const char     *encoding)
{
    MooEdit *doc = NULL;
    char *freeme = NULL;
    GFile *file;

    g_return_val_if_fail (MOO_IS_EDITOR (editor), NULL);
    g_return_val_if_fail (!window || MOO_IS_EDIT_WINDOW (window), NULL);
    g_return_val_if_fail (!parent || GTK_IS_WIDGET (parent), NULL);

    if (!filename)
        return moo_editor_open_file (editor, window, parent, NULL, NULL);

    if (g_file_test (filename, G_FILE_TEST_EXISTS))
        return moo_editor_open_file (editor, window, parent,
                                     filename, encoding);

    freeme = _moo_normalize_file_path (filename);
    filename = freeme;

    if (!window)
        window = moo_editor_get_active_window (editor);

    if (window)
    {
        doc = moo_edit_window_get_active_doc (window);

        if (!doc || !moo_edit_is_empty (doc))
            doc = NULL;
    }

    if (!doc)
        doc = moo_editor_new_doc (editor, window);

    doc->priv->status = MOO_EDIT_NEW;
    file = g_file_new_for_path (filename);
    _moo_edit_set_file (doc, file, encoding);
    moo_editor_set_active_doc (editor, doc);
    gtk_widget_grab_focus (GTK_WIDGET (doc));

    g_object_unref (file);
    g_free (freeme);
    return doc;
}


MooEdit *
moo_editor_open_uri (MooEditor      *editor,
                     MooEditWindow  *window,
                     GtkWidget      *parent,
                     const char     *uri,
                     const char     *encoding)
{
    char *filename;
    MooEdit *doc;

    g_return_val_if_fail (MOO_IS_EDITOR (editor), NULL);
    g_return_val_if_fail (!window || MOO_IS_EDIT_WINDOW (window), NULL);
    g_return_val_if_fail (!parent || GTK_IS_WIDGET (parent), NULL);
    g_return_val_if_fail (uri != NULL, NULL);

    filename = g_filename_from_uri (uri, NULL, NULL);
    g_return_val_if_fail (filename != NULL, NULL);

    doc = moo_editor_open_file (editor, window, parent, filename, encoding);

    g_free (filename);
    return doc;
}


void
_moo_editor_reload (MooEditor   *editor,
                    MooEdit     *doc,
                    const char  *encoding,
                    GError     **error)
{
    GError *error_here = NULL;
    int cursor_line, cursor_offset;
    GtkTextIter iter;

    g_return_if_fail (MOO_IS_EDITOR (editor));

    if (MOO_EDIT_IS_BUSY (doc))
        return;

    /* XXX */
    g_return_if_fail (!moo_edit_is_untitled (doc));

    if (!is_embedded (editor) &&
        !MOO_EDIT_IS_CLEAN (doc) &&
        MOO_EDIT_IS_MODIFIED (doc) &&
        !_moo_edit_reload_modified_dialog (doc))
            return;

    moo_text_view_get_cursor (MOO_TEXT_VIEW (doc), &iter);
    cursor_line = gtk_text_iter_get_line (&iter);
    cursor_offset = moo_text_iter_get_visual_line_offset (&iter, 8);

    if (!_moo_edit_reload_file (doc, encoding, &error_here))
    {
        if (!is_embedded (editor))
        {
            _moo_edit_reload_error_dialog (doc, error_here);
            g_error_free (error_here);
        }
        else
        {
            g_propagate_error (error, error_here);
        }

        moo_text_view_undo (MOO_TEXT_VIEW (doc));
        g_object_set_data (G_OBJECT (doc), "moo-scroll-to", NULL);
        return;
    }

    moo_text_view_move_cursor (MOO_TEXT_VIEW (doc), cursor_line,
                               cursor_offset, TRUE, FALSE);
}


static MooEditSaveFlags
moo_editor_get_save_flags (MooEditor *editor)
{
    MooEditSaveFlags flags = 0;

    if (test_flag (editor, SAVE_BACKUPS))
        flags |= MOO_EDIT_SAVE_BACKUP;

    return flags;
}


static gboolean
do_save (MooEditor    *editor,
         MooEdit      *doc,
         GFile        *file,
         const char   *encoding,
         GError      **error)
{
    gboolean result;
    gboolean strip;
    gboolean add_newline;

    strip = moo_edit_config_get_bool (doc->config, "strip");
    add_newline = moo_edit_config_get_bool (doc->config, "add-newline");

    if (strip)
        moo_text_view_strip_whitespace (MOO_TEXT_VIEW (doc));
    if (add_newline)
        _moo_edit_ensure_newline (doc);

    g_signal_emit_by_name (doc, "save-before");
    result = _moo_edit_save_file (doc, file, encoding,
                                  moo_editor_get_save_flags (editor),
                                  error);
    g_signal_emit_by_name (doc, "save-after");

    return result;
}


gboolean
_moo_editor_save (MooEditor  *editor,
                  MooEdit    *doc,
                  GError    **error)
{
    GError *error_here = NULL;
    GFile *file;
    char *encoding;
    gboolean result = FALSE;

    g_return_val_if_fail (MOO_IS_EDITOR (editor), FALSE);

    if (MOO_EDIT_IS_BUSY (doc))
        return FALSE;

    if (moo_edit_is_untitled (doc))
        return _moo_editor_save_as (editor, doc, NULL, NULL, error);

    file = _moo_edit_get_file (doc);
    encoding = g_strdup (moo_edit_get_encoding (doc));

    if (!is_embedded (editor) &&
        (moo_edit_get_status (doc) & MOO_EDIT_MODIFIED_ON_DISK) &&
        !_moo_edit_overwrite_modified_dialog (doc))
            goto out;

    if (!do_save (editor, doc, file, encoding, &error_here))
    {
        if (!is_embedded (editor))
        {
            gboolean saved_utf8 = error_here->domain == MOO_EDIT_FILE_ERROR &&
                                  error_here->code == MOO_EDIT_FILE_ERROR_ENCODING;
            if (saved_utf8)
                _moo_edit_save_error_enc_dialog (GTK_WIDGET (doc), file, encoding);
            else
                _moo_edit_save_error_dialog (GTK_WIDGET (doc), file, error_here);
            g_error_free (error_here);
        }
        else
        {
            /* XXX */
            g_propagate_error (error, error_here);
        }

        goto out;
    }

    add_recent_file (editor, file);
    result = TRUE;

    /* fall through */
out:
    if (file)
        g_object_unref (file);
    g_free (encoding);
    return result;
}


gboolean
_moo_editor_save_as (MooEditor      *editor,
                     MooEdit        *doc,
                     const char     *filename,
                     const char     *encoding,
                     GError        **error)
{
    GError *error_here = NULL;
    MooEditFileInfo *file_info = NULL;
    gboolean result = FALSE;

    g_return_val_if_fail (MOO_IS_EDITOR (editor), FALSE);

    if (MOO_EDIT_IS_BUSY (doc))
        return FALSE;

    if (!filename)
    {
        file_info = _moo_edit_save_as_dialog (doc, moo_edit_get_display_basename (doc));

        if (!file_info)
            goto out;
    }
    else
    {
        if (!encoding)
            encoding = doc->priv->encoding;
        file_info = moo_edit_file_info_new_path (filename, encoding);
    }

    if (!do_save (editor, doc, file_info->file, file_info->encoding, &error_here))
    {
        if (!is_embedded (editor))
        {
            gboolean saved_utf8 = error_here->domain == MOO_EDIT_FILE_ERROR &&
                                  error_here->code == MOO_EDIT_FILE_ERROR_ENCODING;
            if (saved_utf8)
                _moo_edit_save_error_enc_dialog (GTK_WIDGET (doc),
                                                 file_info->file,
                                                 file_info->encoding);
            else
                _moo_edit_save_error_dialog (GTK_WIDGET (doc),
                                             file_info->file,
                                             error_here);
            g_error_free (error_here);
        }
        else
        {
            /* XXX */
            g_propagate_error (error, error_here);
        }

        goto out;
    }

    add_recent_file (editor, file_info->file);
    result = TRUE;

    /* fall through */
out:
    moo_edit_file_info_free (file_info);
    return result;
}


gboolean
moo_editor_save_copy (MooEditor      *editor,
                      MooEdit        *doc,
                      const char     *filename,
                      const char     *encoding,
                      GError        **error)
{
    GFile *file;
    gboolean retval;

    g_return_val_if_fail (MOO_IS_EDITOR (editor), FALSE);
    g_return_val_if_fail (filename != NULL, FALSE);

    file = g_file_new_for_path (filename);
    retval = _moo_edit_save_file_copy (doc, file, encoding,
                                       moo_editor_get_save_flags (editor),
                                       error);

    g_object_unref (file);
    return retval;
}


static MooEdit *
doc_list_find_filename (DocList    *list,
                        const char *filename)
{
    while (list)
    {
        char *tmp = moo_edit_get_filename (list->data);
        /* XXX */
        if (tmp && strcmp (tmp, filename) == 0)
        {
            g_free (tmp);
            return list->data;
        }
        g_free (tmp);
        list = list->next;
    }
    return NULL;
}

MooEdit *
moo_editor_get_doc (MooEditor  *editor,
                    const char *filename)
{
    char *freeme = NULL;
    MooEdit *doc = NULL;
    WindowList *l;

    g_return_val_if_fail (MOO_IS_EDITOR (editor), NULL);
    g_return_val_if_fail (filename != NULL, NULL);

    freeme = _moo_normalize_file_path (filename);
    filename = freeme;

    if ((doc = doc_list_find_filename (editor->priv->windowless, filename)))
        goto out;

    for (l = editor->priv->windows; l != NULL; l = l->next)
    {
        DocList *docs = window_list_docs (l->data);
        doc = doc_list_find_filename (docs, filename);
        doc_list_free (docs);
        if (doc)
            goto out;
    }

out:
    g_free (freeme);
    return doc;
}

static MooEdit *
doc_list_find_uri (DocList    *list,
                   const char *uri)
{
    while (list)
    {
        char *tmp = moo_edit_get_uri (list->data);
        /* XXX */
        if (tmp && strcmp (tmp, uri) == 0)
        {
            g_free (tmp);
            return list->data;
        }
        g_free (tmp);
        list = list->next;
    }
    return NULL;
}

MooEdit *
moo_editor_get_doc_for_uri (MooEditor  *editor,
                            const char *uri)
{
    MooEdit *doc;
    WindowList *l;

    g_return_val_if_fail (MOO_IS_EDITOR (editor), NULL);
    g_return_val_if_fail (uri != NULL, NULL);

    if ((doc = doc_list_find_uri (editor->priv->windowless, uri)))
        return doc;

    for (l = editor->priv->windows; l != NULL; l = l->next)
    {
        DocList *docs = window_list_docs (l->data);
        doc = doc_list_find_uri (docs, uri);
        doc_list_free (docs);
        if (doc)
            return doc;
    }

    return NULL;
}


void
moo_editor_set_window_type (MooEditor      *editor,
                            GType           type)
{
    g_return_if_fail (MOO_IS_EDITOR (editor));
    g_return_if_fail (g_type_is_a (type, MOO_TYPE_EDIT_WINDOW));
    editor->priv->window_type = type;
}


void
moo_editor_set_edit_type (MooEditor      *editor,
                          GType           type)
{
    g_return_if_fail (MOO_IS_EDITOR (editor));
    g_return_if_fail (g_type_is_a (type, MOO_TYPE_EDIT));
    editor->priv->doc_type = type;
}


static void
set_default_lang (MooEditor  *editor,
                  const char *name)
{
    g_return_if_fail (MOO_IS_EDITOR (editor));
    g_free (editor->priv->default_lang);
    editor->priv->default_lang = g_strdup (name);
}


static gboolean
moo_editor_apply_prefs_in_idle (MooEditor *editor)
{
    editor->priv->prefs_idle = 0;
    moo_editor_apply_prefs (editor);
    return FALSE;
}

void
moo_editor_queue_apply_prefs (MooEditor *editor)
{
    g_return_if_fail (MOO_IS_EDITOR (editor));
    if (!editor->priv->prefs_idle)
        editor->priv->prefs_idle =
            moo_idle_add_full (G_PRIORITY_HIGH,
                               (GSourceFunc) moo_editor_apply_prefs_in_idle,
                               editor, NULL);
}

void
moo_editor_apply_prefs (MooEditor *editor)
{
    GSList *docs;
    gboolean backups;
    const char *color_scheme, *default_lang;

    if (editor->priv->prefs_idle)
    {
        g_source_remove (editor->priv->prefs_idle);
        editor->priv->prefs_idle = 0;
    }

    _moo_edit_window_update_title ();
    _moo_edit_window_set_use_tabs ();

    default_lang = moo_prefs_get_string (moo_edit_setting (MOO_EDIT_PREFS_DEFAULT_LANG));

    if (default_lang && !strcmp (default_lang, MOO_LANG_NONE))
        default_lang = NULL;

    set_default_lang (editor, default_lang);

    _moo_edit_update_global_config ();

    color_scheme = moo_prefs_get_string (moo_edit_setting (MOO_EDIT_PREFS_COLOR_SCHEME));

    if (color_scheme)
        _moo_lang_mgr_set_active_scheme (editor->priv->lang_mgr, color_scheme);

    docs = moo_editor_list_docs (editor);
    g_slist_foreach (docs, (GFunc) _moo_edit_apply_prefs, NULL);
    g_slist_free (docs);

    backups = moo_prefs_get_bool (moo_edit_setting (MOO_EDIT_PREFS_MAKE_BACKUPS));

    g_object_set (editor,
                  "save-backups", backups,
                  NULL);
}


void
_moo_editor_open_uri (MooEditor  *editor,
                      const char *uri,
                      const char *encoding,
                      guint       line,
                      guint       options)
{
    MooEdit *doc;
    MooEditWindow *window;

    g_return_if_fail (MOO_IS_EDITOR (editor));
    g_return_if_fail (uri != NULL);

    doc = moo_editor_get_doc_for_uri (editor, uri);

    if (doc)
    {
        if (line > 0)
            moo_text_view_move_cursor (MOO_TEXT_VIEW (doc), line - 1, 0, FALSE, FALSE);
        moo_editor_set_active_doc (editor, doc);
        gtk_widget_grab_focus (GTK_WIDGET (doc));

        if (options & MOO_EDIT_OPEN_RELOAD)
            _moo_editor_reload (editor, doc, NULL, NULL);

        return;
    }

    window = moo_editor_get_active_window (editor);
    doc = window ? moo_edit_window_get_active_doc (window) : NULL;

    if (!doc || !moo_edit_is_empty (doc))
    {
        gboolean new_window = moo_prefs_get_bool (moo_edit_setting (MOO_EDIT_PREFS_OPEN_NEW_WINDOW));

        if (options & MOO_EDIT_OPEN_NEW_TAB)
            new_window = FALSE;
        else if (options & MOO_EDIT_OPEN_NEW_WINDOW)
            new_window = TRUE;

        if (new_window)
            window = moo_editor_new_window (editor);
    }

    doc = moo_editor_new_uri (editor, window, NULL, uri, encoding);
    g_return_if_fail (doc != NULL);

    moo_editor_set_active_doc (editor, doc);
    if (line > 0)
        moo_text_view_move_cursor (MOO_TEXT_VIEW (doc), line - 1, 0, FALSE, TRUE);
    gtk_widget_grab_focus (GTK_WIDGET (doc));

    if (options & MOO_EDIT_OPEN_RELOAD)
        _moo_editor_reload (editor, doc, NULL, NULL);
}
