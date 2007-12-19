/*
 *   mooeditor.c
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

#define MOOEDIT_COMPILATION
#include "mooedit/mooeditor-private.h"
#include "mooedit/mooeditdialogs.h"
#include "mooedit/mooeditfileops.h"
#include "mooedit/mooplugin.h"
#include "mooedit/mooeditprefs.h"
#include "mooedit/mooedit-private.h"
#include "mooedit/moolangmgr.h"
#include "mooedit/mooeditfiltersettings.h"
#include "mooedit/mooedit-ui.h"
#include "mooedit/medit-ui.h"
#include "mooutils/moomenuaction.h"
#include "mooutils/moomarshals.h"
#include "mooutils/mooutils-misc.h"
#include "mooutils/mooutils-gobject.h"
#include "mooutils/moofilewatch.h"
#include "mooutils/mooutils-fs.h"
#include "mooutils/moostock.h"
#include "mooutils/mooi18n.h"
#include <glib/gbase64.h>
#include <string.h>
#include <stdlib.h>


#define RECENT_ACTION_ID "OpenRecent"

static gpointer editor_instance = NULL;


typedef struct {
    MooEditWindow *window;
    GSList        *docs;
} WindowInfo;

static WindowInfo   *window_info_new        (MooEditWindow  *win);
static void          window_info_free       (WindowInfo     *win);
static void          window_info_add        (WindowInfo     *win,
                                             MooEdit        *doc);
static void          window_info_remove     (WindowInfo     *win,
                                             MooEdit        *doc);
static MooEdit      *window_info_find       (WindowInfo     *win,
                                             const char     *filename);

static void          window_list_free       (MooEditor      *editor);
static void          window_list_delete     (MooEditor      *editor,
                                             WindowInfo     *win);
static WindowInfo   *window_list_add        (MooEditor      *editor,
                                             MooEditWindow  *win);
static WindowInfo   *window_list_find       (MooEditor      *editor,
                                             MooEditWindow  *win);
static WindowInfo   *window_list_find_doc   (MooEditor      *editor,
                                             MooEdit        *edit);
static WindowInfo   *window_list_find_file  (MooEditor      *editor,
                                             const char     *filename,
                                             MooEdit       **edit);

static void          set_single_window      (MooEditor      *editor,
                                             gboolean        single);

#if !GTK_CHECK_VERSION(2,12,0)
static GtkAction    *create_recent_action   (MooEditWindow  *window);
static void          activate_history_item  (MooEditor      *editor,
                                             MooHistoryItem *item,
                                             MooEditWindow  *window);
#endif

static MooEditWindow *create_window         (MooEditor      *editor);
static void          moo_editor_add_doc     (MooEditor      *editor,
                                             MooEditWindow  *window,
                                             MooEdit        *doc);
static gboolean      close_window_handler   (MooEditor      *editor,
                                             MooEditWindow  *window,
                                             gboolean        ask_confirm);
static void          do_close_window        (MooEditor      *editor,
                                             MooEditWindow  *window);
static void          do_close_doc           (MooEditor      *editor,
                                             MooEdit        *doc);
static gboolean      close_docs_real        (MooEditor      *editor,
                                             GSList         *docs,
                                             gboolean        ask_confirm);
static GSList       *find_modified          (GSList         *docs);

static void          add_new_window_action      (void);
static void          remove_new_window_action   (void);


typedef struct {
    GQuark domain;
    char *text;
} Message;

static void          message_free           (Message        *message);


struct _MooEditorPrivate {
    GSList          *messages;
    WindowInfo      *windowless;
    GSList          *windows; /* WindowInfo* */
    char            *app_name;
    MooUIXML        *doc_ui_xml;
    MooUIXML        *ui_xml;
    MooFilterMgr    *filter_mgr;
#if !GTK_CHECK_VERSION(2,12,0)
    MooHistoryList  *history;
#endif
    MooLangMgr      *lang_mgr;
    MooFileWatch    *file_watch;
    gboolean         open_single;
    gboolean         allow_empty_window;
    gboolean         single_window;

    MooEdit         *focused_doc;

    gboolean         save_backups;
    gboolean         strip_whitespace;
    gboolean         silent;

    GType            window_type;
    GType            doc_type;

    char            *default_lang;

    gboolean         autosave;
    guint            autosave_interval;
};


static void     moo_editor_finalize     (GObject        *object);
static void     moo_editor_set_property (GObject        *object,
                                         guint           prop_id,
                                         const GValue   *value,
                                         GParamSpec     *pspec);
static void     moo_editor_get_property (GObject        *object,
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
    PROP_SILENT,
    PROP_AUTOSAVE,
    PROP_AUTOSAVE_INTERVAL,
    PROP_FOCUSED_DOC
};

enum {
    CLOSE_WINDOW,
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];


/* MOO_TYPE_EDITOR */
G_DEFINE_TYPE (MooEditor, moo_editor, G_TYPE_OBJECT)


static void
moo_editor_class_init (MooEditorClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    G_GNUC_UNUSED MooWindowClass *edit_window_class;

    gobject_class->finalize = moo_editor_finalize;
    gobject_class->set_property = moo_editor_set_property;
    gobject_class->get_property = moo_editor_get_property;

    klass->close_window = close_window_handler;

    _moo_edit_init_config ();
    g_type_class_unref (g_type_class_ref (MOO_TYPE_EDIT));
    g_type_class_unref (g_type_class_ref (MOO_TYPE_EDIT_WINDOW));

    g_type_class_add_private (klass, sizeof (MooEditorPrivate));

    g_object_class_install_property (gobject_class,
                                     PROP_OPEN_SINGLE_FILE_INSTANCE,
                                     g_param_spec_boolean ("open-single-file-instance",
                                             "open-single-file-instance",
                                             "open-single-file-instance",
                                             TRUE,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_ALLOW_EMPTY_WINDOW,
                                     g_param_spec_boolean ("allow-empty-window",
                                             "allow-empty-window",
                                             "allow-empty-window",
                                             FALSE,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_SINGLE_WINDOW,
                                     g_param_spec_boolean ("single-window",
                                             "single-window",
                                             "single-window",
                                             FALSE,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_SAVE_BACKUPS,
                                     g_param_spec_boolean ("save-backups",
                                             "save-backups",
                                             "save-backups",
                                             FALSE,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_STRIP_WHITESPACE,
                                     g_param_spec_boolean ("strip-whitespace",
                                             "strip-whitespace",
                                             "strip-whitespace",
                                             FALSE,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_SILENT,
                                     g_param_spec_boolean ("silent",
                                             "silent",
                                             "silent",
                                             FALSE,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_AUTOSAVE,
                                     g_param_spec_boolean ("autosave",
                                             "autosave",
                                             "autosave",
                                             FALSE,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_AUTOSAVE_INTERVAL,
                                     g_param_spec_uint ("autosave-interval",
                                             "autosave-interval",
                                             "autosave-interval",
                                             1, 1000, 5,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_FOCUSED_DOC,
                                     g_param_spec_object ("focused-doc",
                                             "focused-doc",
                                             "focused-doc",
                                             MOO_TYPE_EDIT,
                                             G_PARAM_READABLE));

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

#if !GTK_CHECK_VERSION(2,12,0)
    edit_window_class = g_type_class_ref (MOO_TYPE_EDIT_WINDOW);
    moo_window_class_new_action_custom (edit_window_class, RECENT_ACTION_ID, NULL,
                                        (MooWindowActionFunc) create_recent_action,
                                        NULL, NULL);
    g_type_class_unref (edit_window_class);
#endif

    add_new_window_action ();
}


static void
moo_editor_init (MooEditor *editor)
{
    editor->priv = G_TYPE_INSTANCE_GET_PRIVATE (editor, MOO_TYPE_EDITOR, MooEditorPrivate);

    _moo_stock_init ();

    editor->priv->doc_ui_xml = moo_ui_xml_new ();
    moo_ui_xml_add_ui_from_string (editor->priv->doc_ui_xml,
                                   MOO_EDIT_UI_XML, -1);

    editor->priv->lang_mgr = moo_lang_mgr_new ();
    g_signal_connect_swapped (editor->priv->lang_mgr, "loaded",
                              G_CALLBACK (moo_editor_apply_prefs),
                              editor);

    editor->priv->filter_mgr = moo_filter_mgr_new ();

#if !GTK_CHECK_VERSION(2,12,0)
    editor->priv->history = moo_history_list_get ("Editor");
    g_object_ref (editor->priv->history);
    moo_history_list_set_display_func (editor->priv->history,
                                       moo_history_list_display_basename,
                                       NULL);
    moo_history_list_set_tip_func (editor->priv->history,
                                   moo_history_list_display_filename,
                                   NULL);
    g_signal_connect_swapped (editor->priv->history, "activate-item",
                              G_CALLBACK (activate_history_item), editor);
#endif

    editor->priv->windows = NULL;
    editor->priv->windowless = window_info_new (NULL);

    moo_prefs_new_key_string (moo_edit_setting (MOO_EDIT_PREFS_DEFAULT_LANG),
                              MOO_LANG_NONE);

    _moo_edit_filter_settings_load ();
    moo_editor_apply_prefs (editor);
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
            editor->priv->open_single = g_value_get_boolean (value);
            g_object_notify (object, "open-single-file-instance");
            break;

        case PROP_ALLOW_EMPTY_WINDOW:
            editor->priv->allow_empty_window = g_value_get_boolean (value);
            g_object_notify (object, "allow-empty-window");
            break;

        case PROP_SINGLE_WINDOW:
            set_single_window (editor, g_value_get_boolean (value));
            break;

        case PROP_SAVE_BACKUPS:
            editor->priv->save_backups = g_value_get_boolean (value);
            g_object_notify (object, "save-backups");
            break;

        case PROP_STRIP_WHITESPACE:
            editor->priv->strip_whitespace = g_value_get_boolean (value);
            g_object_notify (object, "strip-whitespace");
            break;

        case PROP_SILENT:
            editor->priv->silent = g_value_get_boolean (value);
            g_object_notify (object, "silent");
            break;

        case PROP_AUTOSAVE:
            editor->priv->autosave = g_value_get_boolean (value);
            g_object_notify (object, "autosave");
            do {
                static int c = 1;
                if (!c++)
                    g_message ("implement Editor::autosave");
            } while (0);
            break;

        case PROP_AUTOSAVE_INTERVAL:
            editor->priv->autosave_interval = g_value_get_uint (value);
            g_object_notify (object, "autosave-interval");
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
            g_value_set_boolean (value, editor->priv->open_single);
            break;

        case PROP_ALLOW_EMPTY_WINDOW:
            g_value_set_boolean (value, editor->priv->allow_empty_window);
            break;

        case PROP_SINGLE_WINDOW:
            g_value_set_boolean (value, editor->priv->single_window);
            break;

        case PROP_SAVE_BACKUPS:
            g_value_set_boolean (value, editor->priv->save_backups);
            break;

        case PROP_STRIP_WHITESPACE:
            g_value_set_boolean (value, editor->priv->strip_whitespace);
            break;

        case PROP_SILENT:
            g_value_set_boolean (value, editor->priv->silent);
            break;

        case PROP_AUTOSAVE:
            g_value_set_boolean (value, editor->priv->autosave);
            break;

        case PROP_AUTOSAVE_INTERVAL:
            g_value_set_uint (value, editor->priv->autosave_interval);
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
    if (editor->priv->filter_mgr)
        g_object_unref (editor->priv->filter_mgr);
#if !GTK_CHECK_VERSION(2,12,0)
    if (editor->priv->history)
        g_object_unref (editor->priv->history);
#endif
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
    {
        g_critical ("finalizing editor while some windows are open");
        window_list_free (editor);
    }

    window_info_free (editor->priv->windowless);

    g_free (editor->priv->default_lang);

    g_slist_foreach (editor->priv->messages, (GFunc) message_free, NULL);
    g_slist_free (editor->priv->messages);

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


#if 0
static Message *
message_new (GQuark      domain,
             const char *text)
{
    Message *msg = g_new0 (Message, 1);
    msg->domain = domain;
    msg->text = g_strdup (text);
    return msg;
}
#endif


static void
message_free (Message *msg)
{
    if (msg)
    {
        g_free (msg->text);
        g_free (msg);
    }
}


#if 0
void
_moo_editor_post_message (MooEditor      *editor,
                          GQuark          domain,
                          const char     *text)
{
    g_return_if_fail (MOO_IS_EDITOR (editor));
    editor->priv->messages = g_slist_prepend (editor->priv->messages,
                                              message_new (domain, text));
}
#endif


gpointer
_moo_editor_get_file_watch (MooEditor *editor)
{
    g_return_val_if_fail (MOO_IS_EDITOR (editor), NULL);

    if (!editor->priv->file_watch)
        editor->priv->file_watch = moo_file_watch_new (NULL);

    return editor->priv->file_watch;
}


MooEditor*
moo_editor_create_instance (void)
{
    if (!editor_instance)
    {
        editor_instance = g_object_new (MOO_TYPE_EDITOR, NULL);
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
    editor->priv->single_window = single;

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
                                     "accel", "<shift><ctrl>N",
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


static MooEditWindow*
get_top_window (MooEditor *editor)
{
    GSList *list = NULL, *l;
    GtkWindow *window;

    if (!editor->priv->windows)
        return NULL;

    for (l = editor->priv->windows; l != NULL; l = l->next)
    {
        WindowInfo *info = l->data;
        list = g_slist_prepend (list, info->window);
    }

    list = g_slist_reverse (list);
    window = _moo_get_top_window (list);

    if (!window)
    {
        WindowInfo *info = editor->priv->windows->data;
        window = GTK_WINDOW (info->window);
    }

    g_slist_free (list);
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


MooFilterMgr *
moo_editor_get_filter_mgr (MooEditor *editor)
{
    g_return_val_if_fail (MOO_IS_EDITOR (editor), NULL);
    return editor->priv->filter_mgr;
}


MooUIXML *
moo_editor_get_ui_xml (MooEditor *editor)
{
    g_return_val_if_fail (MOO_IS_EDITOR (editor), NULL);

    if (!editor->priv->ui_xml)
    {
        editor->priv->ui_xml = moo_ui_xml_new ();
        moo_ui_xml_add_ui_from_string (editor->priv->ui_xml, MEDIT_UI_XML, -1);
    }

    return editor->priv->ui_xml;
}


MooUIXML *
moo_editor_get_doc_ui_xml (MooEditor *editor)
{
    g_return_val_if_fail (MOO_IS_EDITOR (editor), NULL);
    return editor->priv->doc_ui_xml;
}


void
moo_editor_set_ui_xml (MooEditor      *editor,
                       MooUIXML       *xml)
{
    GSList *l;

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
        moo_window_set_ui_xml (l->data, editor->priv->ui_xml);
}


static WindowInfo*
window_info_new (MooEditWindow  *win)
{
    WindowInfo *w = g_new0 (WindowInfo, 1);
    w->window = win;
    return w;
}

static void
window_info_free (WindowInfo     *win)
{
    if (win)
    {
        g_slist_free (win->docs);
        g_free (win);
    }
}

static void
window_info_add (WindowInfo     *win,
                 MooEdit        *edit)
{
    g_return_if_fail (!g_slist_find (win->docs, edit));
    win->docs = g_slist_append (win->docs, edit);
}

static void
window_info_remove (WindowInfo     *win,
                    MooEdit        *edit)
{
    g_return_if_fail (g_slist_find (win->docs, edit));
    win->docs = g_slist_remove (win->docs, edit);
}


static int
edit_and_file_cmp (MooEdit *edit, const char *filename)
{
    char *edit_filename;

    g_return_val_if_fail (MOO_IS_EDIT (edit) && filename != NULL, TRUE);

    edit_filename = moo_edit_get_filename (edit);

    if (edit_filename)
    {
        int result = strcmp (edit_filename, filename);
        g_free (edit_filename);
        return result;
    }
    else
        return TRUE;
}

static MooEdit*
window_info_find (WindowInfo     *win,
                  const char     *filename)
{
    GSList *l;
    g_return_val_if_fail (win != NULL && filename != NULL, NULL);
    l = g_slist_find_custom (win->docs, filename,
                             (GCompareFunc) edit_and_file_cmp);
    return l ? l->data : NULL;
}


static void
window_list_free (MooEditor      *editor)
{
    g_slist_foreach (editor->priv->windows,
                     (GFunc) window_info_free, NULL);
    g_slist_free (editor->priv->windows);
    editor->priv->windows = NULL;
}

static void
window_list_delete (MooEditor      *editor,
                    WindowInfo     *win)
{
    g_return_if_fail (g_slist_find (editor->priv->windows, win));
    window_info_free (win);
    editor->priv->windows =
            g_slist_remove (editor->priv->windows, win);
}

static WindowInfo*
window_list_add (MooEditor      *editor,
                 MooEditWindow  *win)
{
    WindowInfo *w = window_info_new (win);
    editor->priv->windows =
            g_slist_prepend (editor->priv->windows, w);
    return w;
}

static int
window_cmp (WindowInfo *w, MooEditWindow *e)
{
    g_return_val_if_fail (w != NULL, TRUE);
    return !(w->window == e);
}

static WindowInfo*
window_list_find (MooEditor      *editor,
                  MooEditWindow  *win)
{
    GSList *l = g_slist_find_custom (editor->priv->windows, win,
                                     (GCompareFunc) window_cmp);
    if (l)
        return l->data;
    else
        return NULL;
}

static int
doc_cmp (WindowInfo *w, MooEdit *e)
{
    g_return_val_if_fail (w != NULL, 1);
    return !g_slist_find (w->docs, e);
}

static WindowInfo*
window_list_find_doc (MooEditor      *editor,
                      MooEdit        *edit)
{
    GSList *l = g_slist_find_custom (editor->priv->windows, edit,
                                     (GCompareFunc) doc_cmp);

    if (l)
        return l->data;

    if (g_slist_find (editor->priv->windowless->docs, edit))
        return editor->priv->windowless;

    return NULL;
}


static int
filename_and_doc_cmp (WindowInfo *w, gpointer user_data)
{
    struct {
        char    *filename;
        MooEdit *edit;
    } *data = user_data;

    g_return_val_if_fail (w != NULL, TRUE);

    data->edit = window_info_find (w, data->filename);
    return data->edit == NULL;
}

static WindowInfo*
window_list_find_file (MooEditor  *editor,
                       const char *filename,
                       MooEdit   **edit)
{
    GSList *link;
    struct {
        char    *filename;
        MooEdit *edit;
    } data;

    data.filename = _moo_normalize_file_path (filename);
    data.edit = NULL;
    link = g_slist_find_custom (editor->priv->windows, &data,
                                (GCompareFunc) filename_and_doc_cmp);
    g_free (data.filename);

    if (link)
    {
        g_assert (data.edit != NULL);

        if (edit)
            *edit = data.edit;

        return link->data;
    }

    if (window_info_find (editor->priv->windowless, filename))
        return editor->priv->windowless;

    return NULL;
}


#if GTK_CHECK_VERSION(2,12,0)
static void
add_recent_file (G_GNUC_UNUSED MooEditor *editor,
                 const char *filename)
{
    char *uri;

    uri = g_filename_to_uri (filename, NULL, NULL);
    g_return_if_fail (uri != NULL);

    gtk_recent_manager_add_item (gtk_recent_manager_get_default (), uri);

    g_free (uri);
}
#else
static void
add_recent_file (MooEditor  *editor,
                 const char *filename)
{
    moo_history_list_add_filename (editor->priv->history, filename);
}
#endif


#if !GTK_CHECK_VERSION(2,12,0)

static GtkAction*
create_recent_action (MooEditWindow  *window)
{
    MooEditor *editor = moo_edit_window_get_editor (window);
    GtkAction *action;
    G_GNUC_UNUSED MooMenuMgr *mgr;

    g_return_val_if_fail (editor != NULL, NULL);

    action = moo_menu_action_new (RECENT_ACTION_ID, _("Open Recent"));

    mgr = moo_history_list_get_menu_mgr (editor->priv->history);
    moo_menu_mgr_set_show_tooltips (mgr, TRUE);
    moo_menu_action_set_mgr (MOO_MENU_ACTION (action), mgr);
    moo_menu_action_set_menu_data (MOO_MENU_ACTION (action), window, TRUE);

    moo_bind_bool_property (action, "sensitive",
                            editor->priv->history, "empty", TRUE);

    return action;
}

static void
activate_history_item (MooEditor      *editor,
                       MooHistoryItem *item,
                       MooEditWindow  *window)
{
    WindowInfo *win_info;

    g_return_if_fail (MOO_IS_EDITOR (editor));
    g_return_if_fail (MOO_IS_EDIT_WINDOW (window));
    g_return_if_fail (item != NULL && item->data != NULL);

    win_info = window_list_find (editor, window);
    g_return_if_fail (win_info != NULL);

    if (!moo_editor_open_file (editor, window, GTK_WIDGET (window), item->data, NULL))
        moo_history_list_remove (editor->priv->history, item->data);
}

#endif /* !GTK_CHECK_VERSION(2,12,0) */


/*****************************************************************************/

static MooEditWindow*
create_window (MooEditor *editor)
{
    MooEditWindow *window = g_object_new (get_window_type (editor),
                                          "editor", editor,
                                          "ui-xml",
                                          moo_editor_get_ui_xml (editor),
                                          NULL);
    window_list_add (editor, window);
    _moo_window_attach_plugins (window);
    gtk_widget_show (GTK_WIDGET (window));
    return window;
}


static void
moo_editor_add_doc (MooEditor      *editor,
                    MooEditWindow  *window,
                    MooEdit        *doc)
{
    WindowInfo *info;

    if (window)
    {
        info = window_list_find (editor, window);
        g_return_if_fail (info != NULL);
    }
    else
    {
        info = editor->priv->windowless;
    }

    g_return_if_fail (g_slist_find (info->docs, doc) == NULL);

    window_info_add (info, doc);

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

    if (!editor->priv->allow_empty_window)
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

    g_return_val_if_fail (MOO_IS_EDITOR (editor), NULL);

    doc = g_object_new (get_doc_type (editor), "editor", editor, NULL);

    if (filename && !_moo_edit_load_file (doc, filename, encoding, error))
    {
        MOO_OBJECT_REF_SINK (doc);
        g_object_unref (doc);
        return NULL;
    }

    moo_editor_add_doc (editor, NULL, doc);
    _moo_doc_attach_plugins (NULL, doc);

    return doc;
}


MooEdit *
moo_editor_new_doc (MooEditor      *editor,
                    MooEditWindow  *window)
{
    MooEdit *doc;

    g_return_val_if_fail (MOO_IS_EDITOR (editor), NULL);
    g_return_val_if_fail (!window || MOO_IS_EDIT_WINDOW (window), NULL);
    g_return_val_if_fail (!window || window_list_find (editor, window) != NULL, NULL);

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
    WindowInfo *old, *new;
    MooEdit *old_doc;
    int new_pos = -1;

    g_return_if_fail (MOO_IS_EDITOR (editor));
    g_return_if_fail (MOO_IS_EDIT (doc) && doc->priv->editor == editor);
    g_return_if_fail (!dest || (MOO_IS_EDIT_WINDOW (dest) && moo_edit_window_get_editor (dest) == editor));

    old = window_list_find_doc (editor, doc);

    if (!dest)
        dest = moo_editor_new_window (editor);

    new = window_list_find (editor, dest);
    g_return_if_fail (old != NULL && new != NULL);

    g_object_ref (doc);

    window_info_remove (old, doc);

    if (old->window)
    {
        _moo_edit_window_remove_doc (old->window, doc, FALSE);

        if (!moo_edit_window_get_active_doc (old->window))
            moo_editor_close_window (editor, old->window, FALSE);
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
                      MooEdit        **docp)
{
    GError *error = NULL;
    gboolean new_doc = FALSE;
    MooEdit *doc = NULL;
    char *filename;
    gboolean result = TRUE;

    *docp = NULL;
    filename = _moo_normalize_file_path (info->filename);

    if (window_list_find_file (editor, filename, docp))
    {
        if (add_history)
            add_recent_file (editor, filename);
        g_free (filename);
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
    if (!_moo_edit_load_file (doc, filename, info->encoding, &error))
    {
        if (!silent)
        {
            if (!parent && !window)
                window = moo_editor_get_active_window (editor);
            if (!parent && window)
                parent = GTK_WIDGET (window);
            _moo_edit_open_error_dialog (parent, filename, info->encoding, error);
        }
        g_error_free (error);
        result = FALSE;
    }
    else
    {
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
            add_recent_file (editor, filename);
    }

    if (result)
        *docp = doc;

    g_free (filename);
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
    g_return_val_if_fail (!window || window_list_find (editor, window) != NULL, FALSE);

    if (window && !parent)
        parent = GTK_WIDGET (window);

    if (!files)
    {
        MooEdit *active = window ? moo_edit_window_get_active_doc (window) : NULL;

        files = _moo_edit_open_dialog (parent, editor->priv->filter_mgr, active);

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

        if (moo_editor_load_file (editor, window, parent, info, editor->priv->silent, TRUE, &doc))
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
moo_editor_set_active_window (MooEditor    *editor,
                              MooEditWindow  *window)
{
    WindowInfo *info;

    g_return_if_fail (MOO_IS_EDITOR (editor));
    g_return_if_fail (MOO_IS_EDIT_WINDOW (window));

    info = window_list_find (editor, window);
    g_return_if_fail (info != NULL);

    gtk_window_present (GTK_WINDOW (info->window));
}


void
moo_editor_set_active_doc (MooEditor      *editor,
                           MooEdit        *doc)
{
    WindowInfo *info;

    g_return_if_fail (MOO_IS_EDITOR (editor));
    g_return_if_fail (MOO_IS_EDIT (doc));

    info = window_list_find_doc (editor, doc);
    g_return_if_fail (info != NULL);
    g_return_if_fail (info->window != NULL);

    moo_window_present (GTK_WINDOW (info->window), 0);
    moo_edit_window_set_active_doc (info->window, doc);
}


static MooEdit *
find_busy (GSList *docs)
{
    while (docs)
    {
        if (MOO_EDIT_IS_BUSY (docs->data))
            return docs->data;
        docs = docs->next;
    }

    return NULL;
}


static gboolean
close_window_handler (MooEditor     *editor,
                      MooEditWindow *window,
                      gboolean       ask_confirm)
{
    WindowInfo *info;
    MooSaveChangesDialogResponse response;
    GSList *modified;
    gboolean do_close = FALSE;
    MooEdit *busy = NULL;

    g_return_val_if_fail (MOO_IS_EDITOR (editor), FALSE);
    g_return_val_if_fail (MOO_IS_EDIT_WINDOW (window), FALSE);

    info = window_list_find (editor, window);
    g_return_val_if_fail (info != NULL, FALSE);

    busy = find_busy (info->docs);

    if (busy)
    {
        moo_editor_set_active_doc (editor, busy);
        return TRUE;
    }

    modified = find_modified (info->docs);

    if (!modified || !ask_confirm)
    {
        do_close = TRUE;
    }
    else if (!modified->next)
    {
        if (info->window)
            moo_edit_window_set_active_doc (info->window, modified->data);

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

        response = _moo_edit_save_multiple_changes_dialog (modified, &to_save);

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

    g_slist_free (modified);
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

    if (!window_list_find (editor, window))
        return TRUE;

    g_object_ref (window);

    g_signal_emit (editor, signals[CLOSE_WINDOW], 0, window, ask_confirm, &stopped);

    if (!stopped && window_list_find (editor, window))
        do_close_window (editor, window);

    g_object_unref (window);

    return !stopped;
}


static void
do_close_window (MooEditor      *editor,
                 MooEditWindow  *window)
{
    WindowInfo *info;
    GSList *l, *list;

    info = window_list_find (editor, window);
    g_return_if_fail (info != NULL);

    list = g_slist_copy (info->docs);

    for (l = list; l != NULL; l = l->next)
        do_close_doc (editor, l->data);

    window_list_delete (editor, info);

    _moo_window_detach_plugins (window);
    gtk_widget_destroy (GTK_WIDGET (window));

    g_slist_free (list);
}


static void
do_close_doc (MooEditor      *editor,
              MooEdit        *doc)
{
    WindowInfo *info = window_list_find_doc (editor, doc);
    g_return_if_fail (info != NULL);
    window_info_remove (info, doc);

    if (info->window)
        _moo_edit_window_remove_doc (info->window, doc, TRUE);
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
moo_editor_close_docs (MooEditor      *editor,
                       GSList         *list,
                       gboolean        ask_confirm)
{
    WindowInfo *info;
    GSList *l;

    g_return_val_if_fail (MOO_IS_EDITOR (editor), FALSE);

    if (!list)
        return TRUE;

    for (l = list; l != NULL; l = l->next)
    {
        MooEdit *doc = l->data;
        g_return_val_if_fail (MOO_IS_EDIT (doc), FALSE);

        info = window_list_find_doc (editor, doc);
        g_return_val_if_fail (info != NULL, FALSE);

        if (MOO_EDIT_IS_BUSY (doc))
        {
            moo_editor_set_active_doc (editor, doc);
            return FALSE;
        }
    }

    /* do i care? */
    info = window_list_find_doc (editor, list->data);
    g_return_val_if_fail (info != NULL, FALSE);

    if (close_docs_real (editor, list, ask_confirm))
    {
        if (info->window &&
            !moo_edit_window_num_docs (info->window) &&
             !editor->priv->allow_empty_window)
        {
            MooEdit *doc = g_object_new (get_doc_type (editor),
                                         "editor", editor, NULL);
            _moo_edit_window_insert_doc (info->window, doc, -1);
            moo_editor_add_doc (editor, info->window, doc);
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
                 GSList         *docs,
                 gboolean        ask_confirm)
{
    MooSaveChangesDialogResponse response;
    GSList *modified, *l;
    gboolean do_close = FALSE;

    modified = find_modified (docs);

    if (!modified || !ask_confirm)
    {
        do_close = TRUE;
    }
    else if (!modified->next)
    {
        WindowInfo *info = window_list_find_doc (editor, modified->data);
        g_return_val_if_fail (info != NULL, FALSE);

        if (info->window)
            moo_edit_window_set_active_doc (info->window, modified->data);

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
        GSList *to_save = NULL;
        gboolean saved = TRUE;

        response = _moo_edit_save_multiple_changes_dialog (modified, &to_save);

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
        for (l = docs; l != NULL; l = l->next)
            do_close_doc (editor, l->data);

    g_slist_free (modified);
    return do_close;
}


static GSList*
find_modified (GSList *docs)
{
    GSList *modified = NULL, *l;
    for (l = docs; l != NULL; l = l->next)
        if (MOO_EDIT_IS_MODIFIED (l->data) && !MOO_EDIT_IS_CLEAN (l->data))
            modified = g_slist_prepend (modified, l->data);
    return g_slist_reverse (modified);
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

static char *
filename_to_utf8 (const char *filename)
{
    char *encoded, *ret;

    if (g_utf8_validate (filename, -1, NULL))
        return g_strdup (filename);

    encoded = g_base64_encode ((const guchar *) filename, strlen (filename) + 1);
    ret = g_strdup_printf ("base64%s", encoded);
    g_free (encoded);
    return ret;
}

static MooEdit *
load_doc_session (MooEditor     *editor,
                  MooEditWindow *window,
                  MooMarkupNode *elm)
{
    char *filename;
    const char *utf8_filename;
    const char *encoding;
    MooEdit *doc = NULL;
    MooEditFileInfo *info;

    utf8_filename = moo_markup_get_content (elm);

    if (!utf8_filename || !utf8_filename[0])
        return moo_editor_new_doc (editor, window);

    filename = filename_from_utf8 (utf8_filename);
    g_return_val_if_fail (filename != NULL, NULL);

    encoding = moo_markup_get_prop (elm, "encoding");
    info = moo_edit_file_info_new (filename, encoding);

    moo_editor_load_file (editor, window, GTK_WIDGET (window), info, TRUE, FALSE, &doc);

    if (doc)
    {
        int line = moo_markup_get_int_prop (elm, "line", 0);
        moo_text_view_move_cursor (MOO_TEXT_VIEW (doc), line, 0, FALSE, TRUE);
    }

    moo_edit_file_info_free (info);
    g_free (filename);

    return doc;
}

static MooMarkupNode *
save_doc_session (MooEdit       *doc,
                  MooMarkupNode *elm)
{
    char *filename;
    const char *encoding;
    MooMarkupNode *node;

    filename = moo_edit_get_filename (doc);
    encoding = moo_edit_get_encoding (doc);

    if (filename)
    {
        char *utf8_filename;

        utf8_filename = filename_to_utf8 (filename);
        g_return_val_if_fail (utf8_filename != NULL, NULL);

        node = moo_markup_create_text_element (elm, "document", utf8_filename);
        moo_markup_set_int_prop (node, "line", moo_text_view_get_cursor_line (MOO_TEXT_VIEW (doc)));

        if (encoding && encoding[0])
            moo_markup_set_prop (node, "encoding", encoding);

        g_free (utf8_filename);
    }
    else
    {
        node = moo_markup_create_element (elm, "document");
    }

    g_free (filename);
    return node;
}

static MooEditWindow *
load_window_session (MooEditor     *editor,
                     MooMarkupNode *elm)
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

            doc = load_doc_session (editor, window, node);

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
    GSList *docs;

    active_doc = moo_edit_window_get_active_doc (window);
    docs = moo_edit_window_list_docs (window);

    node = moo_markup_create_element (elm, "window");

    while (docs)
    {
        MooMarkupNode *doc_node;
        MooEdit *doc = docs->data;

        doc_node = save_doc_session (doc, node);

        if (doc_node && doc == active_doc)
            moo_markup_set_bool_prop (doc_node, "active", TRUE);

        docs = g_slist_delete_link (docs, docs);
    }

    return node;
}

void
_moo_editor_load_session (MooEditor     *editor,
                          MooMarkupNode *xml)
{
    MooMarkupNode *editor_node;

    g_return_if_fail (MOO_IS_EDITOR (editor));
    g_return_if_fail (MOO_MARKUP_IS_ELEMENT (xml));

    editor_node = moo_markup_get_element (xml, "editor");

    if (editor_node)
    {
        MooEditWindow *active_window = NULL;
        MooMarkupNode *node;

        for (node = editor_node->children; node != NULL; node = node->next)
        {
            MooEditWindow *window;

            if (!MOO_MARKUP_IS_ELEMENT (node))
                continue;

            window = load_window_session (editor, node);

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


GSList*
moo_editor_list_windows (MooEditor *editor)
{
    GSList *windows = NULL, *l;

    g_return_val_if_fail (MOO_IS_EDITOR (editor), NULL);

    for (l = editor->priv->windows; l != NULL; l = l->next)
    {
        WindowInfo *info = l->data;
        windows = g_slist_prepend (windows, info->window);
    }

    return g_slist_reverse (windows);
}


GSList*
moo_editor_list_docs (MooEditor *editor)
{
    GSList *docs = NULL, *l, *list;

    g_return_val_if_fail (MOO_IS_EDITOR (editor), NULL);

    for (l = editor->priv->windows; l != NULL; l = l->next)
    {
        WindowInfo *info = l->data;
        list = g_slist_copy (info->docs);
        list = g_slist_reverse (list);
        docs = g_slist_concat (list, docs);
    }

    list = g_slist_copy (editor->priv->windowless->docs);
    list = g_slist_reverse (list);
    docs = g_slist_concat (list, docs);

    return g_slist_reverse (docs);
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

        info = moo_edit_file_info_new (filename, encoding);
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

    doc = moo_editor_open_file (editor, window, NULL, filename, NULL);
    g_return_val_if_fail (doc != NULL, NULL);

    /* XXX */
    moo_editor_set_active_doc (editor, doc);
    if (line >= 0)
        moo_text_view_move_cursor (MOO_TEXT_VIEW (doc), line, 0, FALSE, TRUE);
    gtk_widget_grab_focus (GTK_WIDGET (doc));

out:
    g_free (freeme);
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
    _moo_edit_set_filename (doc, filename, encoding);
    moo_editor_set_active_doc (editor, doc);
    gtk_widget_grab_focus (GTK_WIDGET (doc));

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
_moo_editor_reload (MooEditor      *editor,
                    MooEdit        *doc,
                    const char     *encoding,
                    GError        **error)
{
    WindowInfo *info;
    GError *error_here = NULL;
    int cursor_line, cursor_offset;
    GtkTextIter iter;

    g_return_if_fail (MOO_IS_EDITOR (editor));

    if (MOO_EDIT_IS_BUSY (doc))
        return;

    info = window_list_find_doc (editor, doc);
    g_return_if_fail (info != NULL);

    /* XXX */
    g_return_if_fail (!moo_edit_is_untitled (doc));

    if (!editor->priv->silent &&
         !MOO_EDIT_IS_CLEAN (doc) &&
         MOO_EDIT_IS_MODIFIED (doc) &&
         !_moo_edit_reload_modified_dialog (doc))
                return;

    moo_text_view_get_cursor (MOO_TEXT_VIEW (doc), &iter);
    cursor_line = gtk_text_iter_get_line (&iter);
    cursor_offset = moo_text_iter_get_visual_line_offset (&iter, 8);

    if (!_moo_edit_reload_file (doc, encoding, &error_here))
    {
        if (!editor->priv->silent)
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

    if (editor->priv->save_backups)
        flags |= MOO_EDIT_SAVE_BACKUP;

    return flags;
}


static gboolean
do_save (MooEditor    *editor,
         MooEdit      *doc,
         const char   *filename,
         const char   *encoding,
         GError      **error)
{
    gboolean result;
    gboolean strip;

    strip = moo_edit_config_get_bool (doc->config, "strip");

    if (strip)
        moo_text_view_strip_whitespace (MOO_TEXT_VIEW (doc));

    g_signal_emit_by_name (doc, "save-before");
    result = _moo_edit_save_file (doc, filename, encoding,
                                  moo_editor_get_save_flags (editor),
                                  error);
    g_signal_emit_by_name (doc, "save-after");

    return result;
}


gboolean
_moo_editor_save (MooEditor      *editor,
                  MooEdit        *doc,
                  GError        **error)
{
    WindowInfo *info;
    GError *error_here = NULL;
    char *filename;
    char *encoding;
    gboolean result = FALSE;

    g_return_val_if_fail (MOO_IS_EDITOR (editor), FALSE);

    if (MOO_EDIT_IS_BUSY (doc))
        return FALSE;

    info = window_list_find_doc (editor, doc);
    g_return_val_if_fail (info != NULL, FALSE);

    if (moo_edit_is_untitled (doc))
        return _moo_editor_save_as (editor, doc, NULL, NULL, error);

    filename = moo_edit_get_filename (doc);
    encoding = g_strdup (moo_edit_get_encoding (doc));

    if (!editor->priv->silent &&
         (moo_edit_get_status (doc) & MOO_EDIT_MODIFIED_ON_DISK) &&
         !_moo_edit_overwrite_modified_dialog (doc))
            goto out;

    if (!do_save (editor, doc, filename, encoding, &error_here))
    {
        if (!editor->priv->silent)
        {
            gboolean saved_utf8 = error_here->domain == MOO_EDIT_FILE_ERROR &&
                                  error_here->code == MOO_EDIT_FILE_ERROR_ENCODING;
            if (saved_utf8)
                _moo_edit_save_error_enc_dialog (GTK_WIDGET (doc), filename, encoding);
            else
                _moo_edit_save_error_dialog (GTK_WIDGET (doc), filename, error_here);
            g_error_free (error_here);
        }
        else
        {
            /* XXX */
            g_propagate_error (error, error_here);
        }

        goto out;
    }

    add_recent_file (editor, filename);
    result = TRUE;

    /* fall through */
out:
    g_free (filename);
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
    WindowInfo *info;
    GError *error_here = NULL;
    MooEditFileInfo *file_info = NULL;
    gboolean result = FALSE;

    g_return_val_if_fail (MOO_IS_EDITOR (editor), FALSE);

    if (MOO_EDIT_IS_BUSY (doc))
        return FALSE;

    info = window_list_find_doc (editor, doc);
    g_return_val_if_fail (info != NULL, FALSE);

    if (!filename)
    {
        file_info = _moo_edit_save_as_dialog (doc, editor->priv->filter_mgr,
                                              moo_edit_get_display_basename (doc));

        if (!file_info)
            goto out;
    }
    else
    {
        file_info = moo_edit_file_info_new (filename, encoding);
    }

    if (!do_save (editor, doc, file_info->filename, file_info->encoding, &error_here))
    {
        if (!editor->priv->silent)
        {
            gboolean saved_utf8 = error_here->domain == MOO_EDIT_FILE_ERROR &&
                                  error_here->code == MOO_EDIT_FILE_ERROR_ENCODING;
            if (saved_utf8)
                _moo_edit_save_error_enc_dialog (GTK_WIDGET (doc),
                                                 file_info->filename,
                                                 file_info->encoding);
            else
                _moo_edit_save_error_dialog (GTK_WIDGET (doc),
                                             file_info->filename,
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

    add_recent_file (editor, file_info->filename);
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
    WindowInfo *info;

    g_return_val_if_fail (MOO_IS_EDITOR (editor), FALSE);
    g_return_val_if_fail (filename != NULL, FALSE);

    info = window_list_find_doc (editor, doc);
    g_return_val_if_fail (info != NULL, FALSE);

    return _moo_edit_save_file_copy (doc, filename, encoding, error);
}


MooEdit*
moo_editor_get_doc (MooEditor      *editor,
                    const char     *filename)
{
    MooEdit *doc = NULL;

    g_return_val_if_fail (MOO_IS_EDITOR (editor), NULL);
    g_return_val_if_fail (filename != NULL, NULL);

    window_list_find_file (editor, filename, &doc);

    return doc;
}


MooLangMgr*
moo_editor_get_lang_mgr (MooEditor *editor)
{
    g_return_val_if_fail (MOO_IS_EDITOR (editor), NULL);
    return editor->priv->lang_mgr;
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


void
moo_editor_apply_prefs (MooEditor *editor)
{
    GSList *docs;
    gboolean autosave, backups;
    int autosave_interval;
    const char *color_scheme, *default_lang;

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

    autosave = moo_prefs_get_bool (moo_edit_setting (MOO_EDIT_PREFS_AUTO_SAVE));
    autosave_interval = moo_prefs_get_int (moo_edit_setting (MOO_EDIT_PREFS_AUTO_SAVE_INTERVAL));
    backups = moo_prefs_get_bool (moo_edit_setting (MOO_EDIT_PREFS_MAKE_BACKUPS));

    g_object_set (editor,
                  "autosave", autosave,
                  "autosave-interval", autosave_interval,
                  "save-backups", backups,
                  NULL);
}


char *
_moo_edit_filename_to_uri (const char *filename,
                           guint       line,
                           guint       options)
{
    GString *string;
    char *uri;
    GError *error = NULL;

    g_return_val_if_fail (filename != NULL, NULL);

    if (!(uri = g_filename_to_uri (filename, NULL, &error)))
    {
        g_warning ("%s: could not convert filename to URI: %s",
                   G_STRLOC, error->message);
        g_error_free (error);
        return NULL;
    }

    if (!line && !options)
        return uri;

    string = g_string_new (uri);
    g_string_append (string, "?");

    if (line > 0)
        g_string_append_printf (string, "line=%u;", line);

    if (options)
    {
        g_string_append (string, "options=");
        if (options & MOO_EDIT_OPEN_NEW_WINDOW)
            g_string_append (string, "new-window");
        if (options & MOO_EDIT_OPEN_NEW_TAB)
            g_string_append (string, "new-tab");
        g_string_append (string, ";");
    }

    g_free (uri);
    return g_string_free (string, FALSE);
}

static void
parse_options (const char *optstring,
               guint      *line,
               guint      *options)
{
    char **p, **comps;

    comps = g_strsplit (optstring, ";", 0);

    for (p = comps; p && *p; ++p)
    {
        if (!strncmp (*p, "line=", strlen ("line=")))
        {
            /* doesn't matter if there is an error */
            *line = strtoul (*p + strlen ("line="), NULL, 10);
        }
        else if (!strncmp (*p, "options=", strlen ("options=")))
        {
            char **opts, **op;
            opts = g_strsplit (*p + strlen ("options="), ",", 0);
            for (op = opts; op && *op; ++op)
            {
                if (!strcmp (*op, "new-window"))
                    *options |= MOO_EDIT_OPEN_NEW_WINDOW;
                else if (!strcmp (*op, "new-tab"))
                    *options |= MOO_EDIT_OPEN_NEW_TAB;
            }
            g_strfreev (opts);
        }
    }

    g_strfreev (comps);
}

char *
_moo_edit_uri_to_filename (const char *uri,
                           guint      *line,
                           guint      *options)
{
    const char *question_mark;
    const char *optstring = NULL;
    char *freeme = NULL;
    char *filename;
    GError *error = NULL;

    g_return_val_if_fail (uri != NULL, NULL);
    g_return_val_if_fail (line != NULL, NULL);
    g_return_val_if_fail (options != NULL, NULL);

    *line = 0;
    *options = 0;

    question_mark = strchr (uri, '?');

    if (question_mark && question_mark > uri)
    {
        freeme = g_strndup (uri, question_mark - uri);
        optstring = question_mark + 1;
        uri = freeme;
    }

    if (!(filename = g_filename_from_uri (uri, NULL, &error)))
    {
        g_warning ("%s: could not convert URI to filename: %s",
                   G_STRLOC, error->message);
        g_error_free (error);
        g_free (freeme);
        return NULL;
    }

    if (optstring)
        parse_options (optstring, line, options);

    g_free (freeme);
    return filename;
}

void
_moo_editor_open_file (MooEditor  *editor,
                       const char *filename,
                       guint       line,
                       guint       options)
{
    MooEdit *doc;
    MooEditWindow *window;

    g_return_if_fail (MOO_IS_EDITOR (editor));
    g_return_if_fail (filename != NULL);

    doc = moo_editor_get_doc (editor, filename);

    if (doc)
    {
        if (line > 0)
            moo_text_view_move_cursor (MOO_TEXT_VIEW (doc), line - 1, 0, FALSE, FALSE);
        moo_editor_set_active_doc (editor, doc);
        gtk_widget_grab_focus (GTK_WIDGET (doc));
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

    doc = moo_editor_new_file (editor, window, NULL, filename, NULL);
    g_return_if_fail (doc != NULL);

    moo_editor_set_active_doc (editor, doc);
    if (line > 0)
        moo_text_view_move_cursor (MOO_TEXT_VIEW (doc), line - 1, 0, FALSE, TRUE);
    gtk_widget_grab_focus (GTK_WIDGET (doc));
}
