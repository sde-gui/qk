#include "moo-lua-api-util.h"

#include "moo-lua-api.h"

// methods of MooAction

// methods of MooActionCollection

// methods of MooApp

static int
cfunc_MooApp_get_editor (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooApp *self = (MooApp*) pself;
    gpointer ret = moo_app_get_editor (self);
    return moo_lua_push_instance (L, ret, MOO_TYPE_EDITOR, TRUE);
}

// methods of MooBigPaned

// methods of MooCmdView

// methods of MooCombo

// methods of MooCommand

// methods of MooCommandContext

// methods of MooCommandFactory

// methods of MooDocPlugin

static int
cfunc_MooDocPlugin_get_doc (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooDocPlugin *self = (MooDocPlugin*) pself;
    gpointer ret = moo_doc_plugin_get_doc (self);
    return moo_lua_push_instance (L, ret, MOO_TYPE_EDIT, TRUE);
}

static int
cfunc_MooDocPlugin_get_plugin (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooDocPlugin *self = (MooDocPlugin*) pself;
    gpointer ret = moo_doc_plugin_get_plugin (self);
    return moo_lua_push_instance (L, ret, MOO_TYPE_PLUGIN, TRUE);
}

static int
cfunc_MooDocPlugin_get_window (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooDocPlugin *self = (MooDocPlugin*) pself;
    gpointer ret = moo_doc_plugin_get_window (self);
    return moo_lua_push_instance (L, ret, MOO_TYPE_EDIT_WINDOW, TRUE);
}

// methods of MooEdit

static int
cfunc_MooEdit_close (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooEdit *self = (MooEdit*) pself;
    gboolean arg0 = moo_lua_get_arg_bool_opt (L, first_arg + 0, "ask_confirm", TRUE);
    gboolean ret = moo_edit_close (self, arg0);
    return moo_lua_push_bool (L, ret);
}

static int
cfunc_MooEdit_get_clean (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooEdit *self = (MooEdit*) pself;
    gboolean ret = moo_edit_get_clean (self);
    return moo_lua_push_bool (L, ret);
}

static int
cfunc_MooEdit_get_cursor_pos (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooEdit *self = (MooEdit*) pself;
    int ret = moo_edit_get_cursor_pos (self);
    return moo_lua_push_int (L, ret);
}

static int
cfunc_MooEdit_get_display_basename (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooEdit *self = (MooEdit*) pself;
    const char *ret = moo_edit_get_display_basename (self);
    return moo_lua_push_string_copy (L, ret);
}

static int
cfunc_MooEdit_get_display_name (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooEdit *self = (MooEdit*) pself;
    const char *ret = moo_edit_get_display_name (self);
    return moo_lua_push_string_copy (L, ret);
}

static int
cfunc_MooEdit_get_encoding (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooEdit *self = (MooEdit*) pself;
    const char *ret = moo_edit_get_encoding (self);
    return moo_lua_push_string_copy (L, ret);
}

static int
cfunc_MooEdit_get_file (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooEdit *self = (MooEdit*) pself;
    gpointer ret = moo_edit_get_file (self);
    return moo_lua_push_instance (L, ret, G_TYPE_FILE, TRUE);
}

static int
cfunc_MooEdit_get_filename (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooEdit *self = (MooEdit*) pself;
    char *ret = moo_edit_get_filename (self);
    return moo_lua_push_string (L, ret);
}

static int
cfunc_MooEdit_get_lang_id (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooEdit *self = (MooEdit*) pself;
    const char *ret = moo_edit_get_lang_id (self);
    return moo_lua_push_string_copy (L, ret);
}

static int
cfunc_MooEdit_get_selected_lines (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooEdit *self = (MooEdit*) pself;
    char **ret = moo_edit_get_selected_lines (self);
    return moo_lua_push_strv (L, ret);
}

static int
cfunc_MooEdit_get_selected_text (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooEdit *self = (MooEdit*) pself;
    char *ret = moo_edit_get_selected_text (self);
    return moo_lua_push_string (L, ret);
}

static int
cfunc_MooEdit_get_status (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooEdit *self = (MooEdit*) pself;
    MooEditStatus ret = moo_edit_get_status (self);
    return moo_lua_push_int (L, ret);
}

static int
cfunc_MooEdit_get_text (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooEdit *self = (MooEdit*) pself;
    char *ret = moo_edit_get_text (self);
    return moo_lua_push_string (L, ret);
}

static int
cfunc_MooEdit_get_uri (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooEdit *self = (MooEdit*) pself;
    char *ret = moo_edit_get_uri (self);
    return moo_lua_push_string (L, ret);
}

static int
cfunc_MooEdit_has_selection (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooEdit *self = (MooEdit*) pself;
    gboolean ret = moo_edit_has_selection (self);
    return moo_lua_push_bool (L, ret);
}

static int
cfunc_MooEdit_insert_text (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooEdit *self = (MooEdit*) pself;
    const char* arg0 = moo_lua_get_arg_string (L, first_arg + 0, "text");
    moo_edit_insert_text (self, arg0);
    return 0;
}

static int
cfunc_MooEdit_is_empty (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooEdit *self = (MooEdit*) pself;
    gboolean ret = moo_edit_is_empty (self);
    return moo_lua_push_bool (L, ret);
}

static int
cfunc_MooEdit_is_untitled (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooEdit *self = (MooEdit*) pself;
    gboolean ret = moo_edit_is_untitled (self);
    return moo_lua_push_bool (L, ret);
}

static int
cfunc_MooEdit_replace_selected_lines (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooEdit *self = (MooEdit*) pself;
    char **arg0 = moo_lua_get_arg_strv (L, first_arg + 0, "replacement");
    moo_edit_replace_selected_lines (self, arg0);
    g_strfreev (arg0);
    return 0;
}

static int
cfunc_MooEdit_replace_selected_text (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooEdit *self = (MooEdit*) pself;
    const char* arg0 = moo_lua_get_arg_string (L, first_arg + 0, "replacement");
    moo_edit_replace_selected_text (self, arg0);
    return 0;
}

static int
cfunc_MooEdit_set_clean (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooEdit *self = (MooEdit*) pself;
    gboolean arg0 = moo_lua_get_arg_bool (L, first_arg + 0, "clean");
    moo_edit_set_clean (self, arg0);
    return 0;
}

static int
cfunc_MooEdit_set_encoding (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooEdit *self = (MooEdit*) pself;
    const char* arg0 = moo_lua_get_arg_string (L, first_arg + 0, "encoding");
    moo_edit_set_encoding (self, arg0);
    return 0;
}

static int
cfunc_MooEdit_set_selection (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooEdit *self = (MooEdit*) pself;
    int arg0 = moo_lua_get_arg_int (L, first_arg + 0, "pos_start");
    int arg1 = moo_lua_get_arg_int (L, first_arg + 1, "pos_end");
    moo_edit_set_selection (self, arg0, arg1);
    return 0;
}

// methods of MooEditAction

// methods of MooEditBookmark

// methods of MooEditOpenInfo

static int
cfunc_MooEditOpenInfo_dup (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooEditOpenInfo *self = (MooEditOpenInfo*) pself;
    gpointer ret = moo_edit_open_info_dup (self);
    return moo_lua_push_instance (L, ret, MOO_TYPE_EDIT_OPEN_INFO, FALSE);
}

// methods of MooEditReloadInfo

static int
cfunc_MooEditReloadInfo_dup (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooEditReloadInfo *self = (MooEditReloadInfo*) pself;
    gpointer ret = moo_edit_reload_info_dup (self);
    return moo_lua_push_instance (L, ret, MOO_TYPE_EDIT_RELOAD_INFO, FALSE);
}

// methods of MooEditSaveInfo

static int
cfunc_MooEditSaveInfo_dup (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooEditSaveInfo *self = (MooEditSaveInfo*) pself;
    gpointer ret = moo_edit_save_info_dup (self);
    return moo_lua_push_instance (L, ret, MOO_TYPE_EDIT_SAVE_INFO, FALSE);
}

// methods of MooEditWindow

static int
cfunc_MooEditWindow_abort_jobs (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooEditWindow *self = (MooEditWindow*) pself;
    moo_edit_window_abort_jobs (self);
    return 0;
}

static int
cfunc_MooEditWindow_add_pane (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooEditWindow *self = (MooEditWindow*) pself;
    const char* arg0 = moo_lua_get_arg_string (L, first_arg + 0, "user_id");
    GtkWidget *arg1 = (GtkWidget*) moo_lua_get_arg_instance (L, first_arg + 1, "widget", GTK_TYPE_WIDGET);
    MooPaneLabel *arg2 = (MooPaneLabel*) moo_lua_get_arg_instance (L, first_arg + 2, "label", MOO_TYPE_PANE_LABEL);
    MooPanePosition arg3 = (MooPanePosition) moo_lua_get_arg_enum (L, first_arg + 3, "position", MOO_TYPE_PANE_POSITION);
    gpointer ret = moo_edit_window_add_pane (self, arg0, arg1, arg2, arg3);
    return moo_lua_push_instance (L, ret, MOO_TYPE_PANE, TRUE);
}

static int
cfunc_MooEditWindow_add_stop_client (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooEditWindow *self = (MooEditWindow*) pself;
    GObject *arg0 = (GObject*) moo_lua_get_arg_instance (L, first_arg + 0, "client", G_TYPE_OBJECT);
    moo_edit_window_add_stop_client (self, arg0);
    return 0;
}

static int
cfunc_MooEditWindow_close_all (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooEditWindow *self = (MooEditWindow*) pself;
    gboolean ret = moo_edit_window_close_all (self);
    return moo_lua_push_bool (L, ret);
}

static int
cfunc_MooEditWindow_get_active_doc (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooEditWindow *self = (MooEditWindow*) pself;
    gpointer ret = moo_edit_window_get_active_doc (self);
    return moo_lua_push_instance (L, ret, MOO_TYPE_EDIT, TRUE);
}

static int
cfunc_MooEditWindow_get_docs (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooEditWindow *self = (MooEditWindow*) pself;
    MooObjectArray *ret = (MooObjectArray*) moo_edit_window_get_docs (self);
    return moo_lua_push_object_array (L, ret, FALSE);
}

static int
cfunc_MooEditWindow_get_editor (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooEditWindow *self = (MooEditWindow*) pself;
    gpointer ret = moo_edit_window_get_editor (self);
    return moo_lua_push_instance (L, ret, MOO_TYPE_EDITOR, TRUE);
}

static int
cfunc_MooEditWindow_get_nth_doc (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooEditWindow *self = (MooEditWindow*) pself;
    guint arg0 = moo_lua_get_arg_int (L, first_arg + 0, "n");
    gpointer ret = moo_edit_window_get_nth_doc (self, arg0);
    return moo_lua_push_instance (L, ret, MOO_TYPE_EDIT, TRUE);
}

static int
cfunc_MooEditWindow_get_pane (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooEditWindow *self = (MooEditWindow*) pself;
    const char* arg0 = moo_lua_get_arg_string (L, first_arg + 0, "user_id");
    gpointer ret = moo_edit_window_get_pane (self, arg0);
    return moo_lua_push_instance (L, ret, GTK_TYPE_WIDGET, TRUE);
}

static int
cfunc_MooEditWindow_num_docs (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooEditWindow *self = (MooEditWindow*) pself;
    int ret = moo_edit_window_num_docs (self);
    return moo_lua_push_int (L, ret);
}

static int
cfunc_MooEditWindow_remove_pane (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooEditWindow *self = (MooEditWindow*) pself;
    const char* arg0 = moo_lua_get_arg_string (L, first_arg + 0, "user_id");
    gboolean ret = moo_edit_window_remove_pane (self, arg0);
    return moo_lua_push_bool (L, ret);
}

static int
cfunc_MooEditWindow_remove_stop_client (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooEditWindow *self = (MooEditWindow*) pself;
    GObject *arg0 = (GObject*) moo_lua_get_arg_instance (L, first_arg + 0, "client", G_TYPE_OBJECT);
    moo_edit_window_remove_stop_client (self, arg0);
    return 0;
}

static int
cfunc_MooEditWindow_set_active_doc (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooEditWindow *self = (MooEditWindow*) pself;
    MooEdit *arg0 = (MooEdit*) moo_lua_get_arg_instance (L, first_arg + 0, "edit", MOO_TYPE_EDIT);
    moo_edit_window_set_active_doc (self, arg0);
    return 0;
}

// methods of MooEditor

static int
cfunc_MooEditor_close_doc (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooEditor *self = (MooEditor*) pself;
    MooEdit *arg0 = (MooEdit*) moo_lua_get_arg_instance (L, first_arg + 0, "doc", MOO_TYPE_EDIT);
    gboolean arg1 = moo_lua_get_arg_bool_opt (L, first_arg + 1, "ask_confirm", TRUE);
    gboolean ret = moo_editor_close_doc (self, arg0, arg1);
    return moo_lua_push_bool (L, ret);
}

static int
cfunc_MooEditor_close_docs (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooEditor *self = (MooEditor*) pself;
    MooEditArray* arg0 = (MooEditArray*) moo_lua_get_arg_object_array (L, first_arg + 0, "docs", MOO_TYPE_EDIT);
    gboolean arg1 = moo_lua_get_arg_bool_opt (L, first_arg + 1, "ask_confirm", TRUE);
    gboolean ret = moo_editor_close_docs (self, arg0, arg1);
    moo_object_array_free ((MooObjectArray*) arg0);
    return moo_lua_push_bool (L, ret);
}

static int
cfunc_MooEditor_close_window (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooEditor *self = (MooEditor*) pself;
    MooEditWindow *arg0 = (MooEditWindow*) moo_lua_get_arg_instance (L, first_arg + 0, "window", MOO_TYPE_EDIT_WINDOW);
    gboolean arg1 = moo_lua_get_arg_bool_opt (L, first_arg + 1, "ask_confirm", TRUE);
    gboolean ret = moo_editor_close_window (self, arg0, arg1);
    return moo_lua_push_bool (L, ret);
}

static int
cfunc_MooEditor_get_active_doc (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooEditor *self = (MooEditor*) pself;
    gpointer ret = moo_editor_get_active_doc (self);
    return moo_lua_push_instance (L, ret, MOO_TYPE_EDIT, TRUE);
}

static int
cfunc_MooEditor_get_active_window (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooEditor *self = (MooEditor*) pself;
    gpointer ret = moo_editor_get_active_window (self);
    return moo_lua_push_instance (L, ret, MOO_TYPE_EDIT_WINDOW, TRUE);
}

static int
cfunc_MooEditor_get_doc (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooEditor *self = (MooEditor*) pself;
    GFile *arg0 = (GFile*) moo_lua_get_arg_instance (L, first_arg + 0, "file", G_TYPE_FILE);
    gpointer ret = moo_editor_get_doc (self, arg0);
    return moo_lua_push_instance (L, ret, MOO_TYPE_EDIT, TRUE);
}

static int
cfunc_MooEditor_get_doc_for_path (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooEditor *self = (MooEditor*) pself;
    const char* arg0 = moo_lua_get_arg_string (L, first_arg + 0, "path");
    gpointer ret = moo_editor_get_doc_for_path (self, arg0);
    return moo_lua_push_instance (L, ret, MOO_TYPE_EDIT, TRUE);
}

static int
cfunc_MooEditor_get_doc_for_uri (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooEditor *self = (MooEditor*) pself;
    const char* arg0 = moo_lua_get_arg_string (L, first_arg + 0, "uri");
    gpointer ret = moo_editor_get_doc_for_uri (self, arg0);
    return moo_lua_push_instance (L, ret, MOO_TYPE_EDIT, TRUE);
}

static int
cfunc_MooEditor_get_doc_ui_xml (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooEditor *self = (MooEditor*) pself;
    gpointer ret = moo_editor_get_doc_ui_xml (self);
    return moo_lua_push_instance (L, ret, MOO_TYPE_UI_XML, TRUE);
}

static int
cfunc_MooEditor_get_docs (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooEditor *self = (MooEditor*) pself;
    MooObjectArray *ret = (MooObjectArray*) moo_editor_get_docs (self);
    return moo_lua_push_object_array (L, ret, FALSE);
}

static int
cfunc_MooEditor_get_ui_xml (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooEditor *self = (MooEditor*) pself;
    gpointer ret = moo_editor_get_ui_xml (self);
    return moo_lua_push_instance (L, ret, MOO_TYPE_UI_XML, TRUE);
}

static int
cfunc_MooEditor_get_windows (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooEditor *self = (MooEditor*) pself;
    MooObjectArray *ret = (MooObjectArray*) moo_editor_get_windows (self);
    return moo_lua_push_object_array (L, ret, FALSE);
}

static int
cfunc_MooEditor_new_doc (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooEditor *self = (MooEditor*) pself;
    MooEditWindow *arg0 = (MooEditWindow*) moo_lua_get_arg_instance_opt (L, first_arg + 0, "window", MOO_TYPE_EDIT_WINDOW);
    gpointer ret = moo_editor_new_doc (self, arg0);
    return moo_lua_push_instance (L, ret, MOO_TYPE_EDIT, TRUE);
}

static int
cfunc_MooEditor_new_window (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooEditor *self = (MooEditor*) pself;
    gpointer ret = moo_editor_new_window (self);
    return moo_lua_push_instance (L, ret, MOO_TYPE_EDIT_WINDOW, TRUE);
}

static int
cfunc_MooEditor_open_path (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooEditor *self = (MooEditor*) pself;
    const char* arg0 = moo_lua_get_arg_string (L, first_arg + 0, "path");
    const char* arg1 = moo_lua_get_arg_string_opt (L, first_arg + 1, "encoding", NULL);
    int arg2 = moo_lua_get_arg_int_opt (L, first_arg + 2, "line", -1);
    MooEditWindow *arg3 = (MooEditWindow*) moo_lua_get_arg_instance_opt (L, first_arg + 3, "window", MOO_TYPE_EDIT_WINDOW);
    gpointer ret = moo_editor_open_path (self, arg0, arg1, arg2, arg3);
    return moo_lua_push_instance (L, ret, MOO_TYPE_EDIT, TRUE);
}

static int
cfunc_MooEditor_open_uri (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooEditor *self = (MooEditor*) pself;
    const char* arg0 = moo_lua_get_arg_string (L, first_arg + 0, "uri");
    const char* arg1 = moo_lua_get_arg_string_opt (L, first_arg + 1, "encoding", NULL);
    int arg2 = moo_lua_get_arg_int_opt (L, first_arg + 2, "line", -1);
    MooEditWindow *arg3 = (MooEditWindow*) moo_lua_get_arg_instance_opt (L, first_arg + 3, "window", MOO_TYPE_EDIT_WINDOW);
    gpointer ret = moo_editor_open_uri (self, arg0, arg1, arg2, arg3);
    return moo_lua_push_instance (L, ret, MOO_TYPE_EDIT, TRUE);
}

static int
cfunc_MooEditor_set_active_doc (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooEditor *self = (MooEditor*) pself;
    MooEdit *arg0 = (MooEdit*) moo_lua_get_arg_instance (L, first_arg + 0, "doc", MOO_TYPE_EDIT);
    moo_editor_set_active_doc (self, arg0);
    return 0;
}

static int
cfunc_MooEditor_set_active_window (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooEditor *self = (MooEditor*) pself;
    MooEditWindow *arg0 = (MooEditWindow*) moo_lua_get_arg_instance (L, first_arg + 0, "window", MOO_TYPE_EDIT_WINDOW);
    moo_editor_set_active_window (self, arg0);
    return 0;
}

static int
cfunc_MooEditor_set_ui_xml (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooEditor *self = (MooEditor*) pself;
    MooUiXml *arg0 = (MooUiXml*) moo_lua_get_arg_instance (L, first_arg + 0, "xml", MOO_TYPE_UI_XML);
    moo_editor_set_ui_xml (self, arg0);
    return 0;
}

// methods of MooEntry

// methods of MooFileDialog

// methods of MooGladeXml

// methods of MooHistoryCombo

// methods of MooHistoryList

// methods of MooHistoryMgr

// methods of MooLineMark

// methods of MooLineView

// methods of MooMenuAction

// methods of MooMenuMgr

// methods of MooMenuToolButton

// methods of MooNotebook

// methods of MooOutputFilter

// methods of MooPane

static int
cfunc_MooPane_attach (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooPane *self = (MooPane*) pself;
    moo_pane_attach (self);
    return 0;
}

static int
cfunc_MooPane_detach (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooPane *self = (MooPane*) pself;
    moo_pane_detach (self);
    return 0;
}

static int
cfunc_MooPane_get_child (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooPane *self = (MooPane*) pself;
    gpointer ret = moo_pane_get_child (self);
    return moo_lua_push_instance (L, ret, GTK_TYPE_WIDGET, TRUE);
}

static int
cfunc_MooPane_get_detachable (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooPane *self = (MooPane*) pself;
    gboolean ret = moo_pane_get_detachable (self);
    return moo_lua_push_bool (L, ret);
}

static int
cfunc_MooPane_get_id (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooPane *self = (MooPane*) pself;
    const char *ret = moo_pane_get_id (self);
    return moo_lua_push_string_copy (L, ret);
}

static int
cfunc_MooPane_get_index (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooPane *self = (MooPane*) pself;
    int ret = moo_pane_get_index (self);
    return moo_lua_push_int (L, ret);
}

static int
cfunc_MooPane_get_label (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooPane *self = (MooPane*) pself;
    gpointer ret = moo_pane_get_label (self);
    return moo_lua_push_instance (L, ret, MOO_TYPE_PANE_LABEL, TRUE);
}

static int
cfunc_MooPane_get_params (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooPane *self = (MooPane*) pself;
    gpointer ret = moo_pane_get_params (self);
    return moo_lua_push_instance (L, ret, MOO_TYPE_PANE_PARAMS, TRUE);
}

static int
cfunc_MooPane_get_removable (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooPane *self = (MooPane*) pself;
    gboolean ret = moo_pane_get_removable (self);
    return moo_lua_push_bool (L, ret);
}

static int
cfunc_MooPane_open (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooPane *self = (MooPane*) pself;
    moo_pane_open (self);
    return 0;
}

static int
cfunc_MooPane_present (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooPane *self = (MooPane*) pself;
    moo_pane_present (self);
    return 0;
}

static int
cfunc_MooPane_set_detachable (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooPane *self = (MooPane*) pself;
    gboolean arg0 = moo_lua_get_arg_bool (L, first_arg + 0, "detachable");
    moo_pane_set_detachable (self, arg0);
    return 0;
}

static int
cfunc_MooPane_set_drag_dest (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooPane *self = (MooPane*) pself;
    moo_pane_set_drag_dest (self);
    return 0;
}

static int
cfunc_MooPane_set_frame_markup (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooPane *self = (MooPane*) pself;
    const char* arg0 = moo_lua_get_arg_string (L, first_arg + 0, "markup");
    moo_pane_set_frame_markup (self, arg0);
    return 0;
}

static int
cfunc_MooPane_set_frame_text (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooPane *self = (MooPane*) pself;
    const char* arg0 = moo_lua_get_arg_string (L, first_arg + 0, "text");
    moo_pane_set_frame_text (self, arg0);
    return 0;
}

static int
cfunc_MooPane_set_label (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooPane *self = (MooPane*) pself;
    MooPaneLabel *arg0 = (MooPaneLabel*) moo_lua_get_arg_instance (L, first_arg + 0, "label", MOO_TYPE_PANE_LABEL);
    moo_pane_set_label (self, arg0);
    return 0;
}

static int
cfunc_MooPane_set_params (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooPane *self = (MooPane*) pself;
    MooPaneParams *arg0 = (MooPaneParams*) moo_lua_get_arg_instance (L, first_arg + 0, "params", MOO_TYPE_PANE_PARAMS);
    moo_pane_set_params (self, arg0);
    return 0;
}

static int
cfunc_MooPane_set_removable (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooPane *self = (MooPane*) pself;
    gboolean arg0 = moo_lua_get_arg_bool (L, first_arg + 0, "removable");
    moo_pane_set_removable (self, arg0);
    return 0;
}

static int
cfunc_MooPane_unset_drag_dest (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooPane *self = (MooPane*) pself;
    moo_pane_unset_drag_dest (self);
    return 0;
}

// methods of MooPaned

// methods of MooPlugin

static int
cfunc_MooPlugin_set_info (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooPlugin *self = (MooPlugin*) pself;
    MooPluginInfo *arg0 = (MooPluginInfo*) moo_lua_get_arg_instance (L, first_arg + 0, "info", MOO_TYPE_PLUGIN_INFO);
    moo_plugin_set_info (self, arg0);
    return 0;
}

// methods of MooPrefsDialog

// methods of MooPrefsPage

// methods of MooTextBuffer

// methods of MooTextView

static int
cfunc_MooTextView_set_lang_by_id (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooTextView *self = (MooTextView*) pself;
    const char* arg0 = moo_lua_get_arg_string (L, first_arg + 0, "id");
    moo_text_view_set_lang_by_id (self, arg0);
    return 0;
}

// methods of MooUiXml

static int
cfunc_MooUiXml_add_item (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooUiXml *self = (MooUiXml*) pself;
    guint arg0 = moo_lua_get_arg_int (L, first_arg + 0, "merge_id");
    const char* arg1 = moo_lua_get_arg_string (L, first_arg + 1, "parent_path");
    const char* arg2 = moo_lua_get_arg_string (L, first_arg + 2, "name");
    const char* arg3 = moo_lua_get_arg_string (L, first_arg + 3, "action");
    int arg4 = moo_lua_get_arg_int (L, first_arg + 4, "position");
    gpointer ret = moo_ui_xml_add_item (self, arg0, arg1, arg2, arg3, arg4);
    return moo_lua_push_instance (L, ret, MOO_TYPE_UI_NODE, TRUE);
}

static int
cfunc_MooUiXml_create_widget (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooUiXml *self = (MooUiXml*) pself;
    MooUiWidgetType arg0 = (MooUiWidgetType) moo_lua_get_arg_enum (L, first_arg + 0, "type", MOO_TYPE_UI_WIDGET_TYPE);
    const char* arg1 = moo_lua_get_arg_string (L, first_arg + 1, "path");
    MooActionCollection *arg2 = (MooActionCollection*) moo_lua_get_arg_instance (L, first_arg + 2, "actions", MOO_TYPE_ACTION_COLLECTION);
    GtkAccelGroup *arg3 = (GtkAccelGroup*) moo_lua_get_arg_instance (L, first_arg + 3, "accel_group", GTK_TYPE_ACCEL_GROUP);
    gpointer ret = moo_ui_xml_create_widget (self, arg0, arg1, arg2, arg3);
    return moo_lua_push_instance (L, ret, GTK_TYPE_WIDGET, TRUE);
}

static int
cfunc_MooUiXml_find_placeholder (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooUiXml *self = (MooUiXml*) pself;
    const char* arg0 = moo_lua_get_arg_string (L, first_arg + 0, "name");
    gpointer ret = moo_ui_xml_find_placeholder (self, arg0);
    return moo_lua_push_instance (L, ret, MOO_TYPE_UI_NODE, TRUE);
}

static int
cfunc_MooUiXml_get_node (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooUiXml *self = (MooUiXml*) pself;
    const char* arg0 = moo_lua_get_arg_string (L, first_arg + 0, "path");
    gpointer ret = moo_ui_xml_get_node (self, arg0);
    return moo_lua_push_instance (L, ret, MOO_TYPE_UI_NODE, TRUE);
}

static int
cfunc_MooUiXml_get_widget (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooUiXml *self = (MooUiXml*) pself;
    GtkWidget *arg0 = (GtkWidget*) moo_lua_get_arg_instance (L, first_arg + 0, "toplevel", GTK_TYPE_WIDGET);
    const char* arg1 = moo_lua_get_arg_string (L, first_arg + 1, "path");
    gpointer ret = moo_ui_xml_get_widget (self, arg0, arg1);
    return moo_lua_push_instance (L, ret, GTK_TYPE_WIDGET, TRUE);
}

static int
cfunc_MooUiXml_insert (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooUiXml *self = (MooUiXml*) pself;
    guint arg0 = moo_lua_get_arg_int (L, first_arg + 0, "merge_id");
    MooUiNode *arg1 = (MooUiNode*) moo_lua_get_arg_instance (L, first_arg + 1, "parent", MOO_TYPE_UI_NODE);
    int arg2 = moo_lua_get_arg_int (L, first_arg + 2, "position");
    const char* arg3 = moo_lua_get_arg_string (L, first_arg + 3, "markup");
    moo_ui_xml_insert (self, arg0, arg1, arg2, arg3);
    return 0;
}

static int
cfunc_MooUiXml_insert_after (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooUiXml *self = (MooUiXml*) pself;
    guint arg0 = moo_lua_get_arg_int (L, first_arg + 0, "merge_id");
    MooUiNode *arg1 = (MooUiNode*) moo_lua_get_arg_instance (L, first_arg + 1, "parent", MOO_TYPE_UI_NODE);
    MooUiNode *arg2 = (MooUiNode*) moo_lua_get_arg_instance (L, first_arg + 2, "after", MOO_TYPE_UI_NODE);
    const char* arg3 = moo_lua_get_arg_string (L, first_arg + 3, "markup");
    moo_ui_xml_insert_after (self, arg0, arg1, arg2, arg3);
    return 0;
}

static int
cfunc_MooUiXml_insert_before (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooUiXml *self = (MooUiXml*) pself;
    guint arg0 = moo_lua_get_arg_int (L, first_arg + 0, "merge_id");
    MooUiNode *arg1 = (MooUiNode*) moo_lua_get_arg_instance (L, first_arg + 1, "parent", MOO_TYPE_UI_NODE);
    MooUiNode *arg2 = (MooUiNode*) moo_lua_get_arg_instance (L, first_arg + 2, "before", MOO_TYPE_UI_NODE);
    const char* arg3 = moo_lua_get_arg_string (L, first_arg + 3, "markup");
    moo_ui_xml_insert_before (self, arg0, arg1, arg2, arg3);
    return 0;
}

static int
cfunc_MooUiXml_insert_markup (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooUiXml *self = (MooUiXml*) pself;
    guint arg0 = moo_lua_get_arg_int (L, first_arg + 0, "merge_id");
    const char* arg1 = moo_lua_get_arg_string (L, first_arg + 1, "parent_path");
    int arg2 = moo_lua_get_arg_int (L, first_arg + 2, "position");
    const char* arg3 = moo_lua_get_arg_string (L, first_arg + 3, "markup");
    moo_ui_xml_insert_markup (self, arg0, arg1, arg2, arg3);
    return 0;
}

static int
cfunc_MooUiXml_insert_markup_after (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooUiXml *self = (MooUiXml*) pself;
    guint arg0 = moo_lua_get_arg_int (L, first_arg + 0, "merge_id");
    const char* arg1 = moo_lua_get_arg_string (L, first_arg + 1, "parent_path");
    const char* arg2 = moo_lua_get_arg_string (L, first_arg + 2, "after");
    const char* arg3 = moo_lua_get_arg_string (L, first_arg + 3, "markup");
    moo_ui_xml_insert_markup_after (self, arg0, arg1, arg2, arg3);
    return 0;
}

static int
cfunc_MooUiXml_insert_markup_before (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooUiXml *self = (MooUiXml*) pself;
    guint arg0 = moo_lua_get_arg_int (L, first_arg + 0, "merge_id");
    const char* arg1 = moo_lua_get_arg_string (L, first_arg + 1, "parent_path");
    const char* arg2 = moo_lua_get_arg_string (L, first_arg + 2, "before");
    const char* arg3 = moo_lua_get_arg_string (L, first_arg + 3, "markup");
    moo_ui_xml_insert_markup_before (self, arg0, arg1, arg2, arg3);
    return 0;
}

static int
cfunc_MooUiXml_new_merge_id (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooUiXml *self = (MooUiXml*) pself;
    guint ret = moo_ui_xml_new_merge_id (self);
    return moo_lua_push_int (L, ret);
}

static int
cfunc_MooUiXml_remove_node (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooUiXml *self = (MooUiXml*) pself;
    MooUiNode *arg0 = (MooUiNode*) moo_lua_get_arg_instance (L, first_arg + 0, "node", MOO_TYPE_UI_NODE);
    moo_ui_xml_remove_node (self, arg0);
    return 0;
}

static int
cfunc_MooUiXml_remove_ui (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooUiXml *self = (MooUiXml*) pself;
    guint arg0 = moo_lua_get_arg_int (L, first_arg + 0, "merge_id");
    moo_ui_xml_remove_ui (self, arg0);
    return 0;
}

// methods of MooWinPlugin

static int
cfunc_MooWinPlugin_get_plugin (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooWinPlugin *self = (MooWinPlugin*) pself;
    gpointer ret = moo_win_plugin_get_plugin (self);
    return moo_lua_push_instance (L, ret, MOO_TYPE_PLUGIN, TRUE);
}

static int
cfunc_MooWinPlugin_get_window (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooWinPlugin *self = (MooWinPlugin*) pself;
    gpointer ret = moo_win_plugin_get_window (self);
    return moo_lua_push_instance (L, ret, MOO_TYPE_EDIT_WINDOW, TRUE);
}

// methods of MooWindow

void
moo_lua_api_register (void)
{
    static gboolean been_here = FALSE;

    if (been_here)
        return;

    been_here = TRUE;

    MooLuaMethodEntry methods_MooApp[] = {
        { "get_editor", cfunc_MooApp_get_editor },
        { NULL, NULL }
    };
    moo_lua_register_methods (MOO_TYPE_APP, methods_MooApp);

    MooLuaMethodEntry methods_MooDocPlugin[] = {
        { "get_doc", cfunc_MooDocPlugin_get_doc },
        { "get_plugin", cfunc_MooDocPlugin_get_plugin },
        { "get_window", cfunc_MooDocPlugin_get_window },
        { NULL, NULL }
    };
    moo_lua_register_methods (MOO_TYPE_DOC_PLUGIN, methods_MooDocPlugin);

    MooLuaMethodEntry methods_MooEdit[] = {
        { "close", cfunc_MooEdit_close },
        { "get_clean", cfunc_MooEdit_get_clean },
        { "get_cursor_pos", cfunc_MooEdit_get_cursor_pos },
        { "get_display_basename", cfunc_MooEdit_get_display_basename },
        { "get_display_name", cfunc_MooEdit_get_display_name },
        { "get_encoding", cfunc_MooEdit_get_encoding },
        { "get_file", cfunc_MooEdit_get_file },
        { "get_filename", cfunc_MooEdit_get_filename },
        { "get_lang_id", cfunc_MooEdit_get_lang_id },
        { "get_selected_lines", cfunc_MooEdit_get_selected_lines },
        { "get_selected_text", cfunc_MooEdit_get_selected_text },
        { "get_status", cfunc_MooEdit_get_status },
        { "get_text", cfunc_MooEdit_get_text },
        { "get_uri", cfunc_MooEdit_get_uri },
        { "has_selection", cfunc_MooEdit_has_selection },
        { "insert_text", cfunc_MooEdit_insert_text },
        { "is_empty", cfunc_MooEdit_is_empty },
        { "is_untitled", cfunc_MooEdit_is_untitled },
        { "replace_selected_lines", cfunc_MooEdit_replace_selected_lines },
        { "replace_selected_text", cfunc_MooEdit_replace_selected_text },
        { "set_clean", cfunc_MooEdit_set_clean },
        { "set_encoding", cfunc_MooEdit_set_encoding },
        { "set_selection", cfunc_MooEdit_set_selection },
        { NULL, NULL }
    };
    moo_lua_register_methods (MOO_TYPE_EDIT, methods_MooEdit);

    MooLuaMethodEntry methods_MooEditOpenInfo[] = {
        { "dup", cfunc_MooEditOpenInfo_dup },
        { NULL, NULL }
    };
    moo_lua_register_methods (MOO_TYPE_EDIT_OPEN_INFO, methods_MooEditOpenInfo);

    MooLuaMethodEntry methods_MooEditReloadInfo[] = {
        { "dup", cfunc_MooEditReloadInfo_dup },
        { NULL, NULL }
    };
    moo_lua_register_methods (MOO_TYPE_EDIT_RELOAD_INFO, methods_MooEditReloadInfo);

    MooLuaMethodEntry methods_MooEditSaveInfo[] = {
        { "dup", cfunc_MooEditSaveInfo_dup },
        { NULL, NULL }
    };
    moo_lua_register_methods (MOO_TYPE_EDIT_SAVE_INFO, methods_MooEditSaveInfo);

    MooLuaMethodEntry methods_MooEditWindow[] = {
        { "abort_jobs", cfunc_MooEditWindow_abort_jobs },
        { "add_pane", cfunc_MooEditWindow_add_pane },
        { "add_stop_client", cfunc_MooEditWindow_add_stop_client },
        { "close_all", cfunc_MooEditWindow_close_all },
        { "get_active_doc", cfunc_MooEditWindow_get_active_doc },
        { "get_docs", cfunc_MooEditWindow_get_docs },
        { "get_editor", cfunc_MooEditWindow_get_editor },
        { "get_nth_doc", cfunc_MooEditWindow_get_nth_doc },
        { "get_pane", cfunc_MooEditWindow_get_pane },
        { "num_docs", cfunc_MooEditWindow_num_docs },
        { "remove_pane", cfunc_MooEditWindow_remove_pane },
        { "remove_stop_client", cfunc_MooEditWindow_remove_stop_client },
        { "set_active_doc", cfunc_MooEditWindow_set_active_doc },
        { NULL, NULL }
    };
    moo_lua_register_methods (MOO_TYPE_EDIT_WINDOW, methods_MooEditWindow);

    MooLuaMethodEntry methods_MooEditor[] = {
        { "close_doc", cfunc_MooEditor_close_doc },
        { "close_docs", cfunc_MooEditor_close_docs },
        { "close_window", cfunc_MooEditor_close_window },
        { "get_active_doc", cfunc_MooEditor_get_active_doc },
        { "get_active_window", cfunc_MooEditor_get_active_window },
        { "get_doc", cfunc_MooEditor_get_doc },
        { "get_doc_for_path", cfunc_MooEditor_get_doc_for_path },
        { "get_doc_for_uri", cfunc_MooEditor_get_doc_for_uri },
        { "get_doc_ui_xml", cfunc_MooEditor_get_doc_ui_xml },
        { "get_docs", cfunc_MooEditor_get_docs },
        { "get_ui_xml", cfunc_MooEditor_get_ui_xml },
        { "get_windows", cfunc_MooEditor_get_windows },
        { "new_doc", cfunc_MooEditor_new_doc },
        { "new_window", cfunc_MooEditor_new_window },
        { "open_path", cfunc_MooEditor_open_path },
        { "open_uri", cfunc_MooEditor_open_uri },
        { "set_active_doc", cfunc_MooEditor_set_active_doc },
        { "set_active_window", cfunc_MooEditor_set_active_window },
        { "set_ui_xml", cfunc_MooEditor_set_ui_xml },
        { NULL, NULL }
    };
    moo_lua_register_methods (MOO_TYPE_EDITOR, methods_MooEditor);

    MooLuaMethodEntry methods_MooPane[] = {
        { "attach", cfunc_MooPane_attach },
        { "detach", cfunc_MooPane_detach },
        { "get_child", cfunc_MooPane_get_child },
        { "get_detachable", cfunc_MooPane_get_detachable },
        { "get_id", cfunc_MooPane_get_id },
        { "get_index", cfunc_MooPane_get_index },
        { "get_label", cfunc_MooPane_get_label },
        { "get_params", cfunc_MooPane_get_params },
        { "get_removable", cfunc_MooPane_get_removable },
        { "open", cfunc_MooPane_open },
        { "present", cfunc_MooPane_present },
        { "set_detachable", cfunc_MooPane_set_detachable },
        { "set_drag_dest", cfunc_MooPane_set_drag_dest },
        { "set_frame_markup", cfunc_MooPane_set_frame_markup },
        { "set_frame_text", cfunc_MooPane_set_frame_text },
        { "set_label", cfunc_MooPane_set_label },
        { "set_params", cfunc_MooPane_set_params },
        { "set_removable", cfunc_MooPane_set_removable },
        { "unset_drag_dest", cfunc_MooPane_unset_drag_dest },
        { NULL, NULL }
    };
    moo_lua_register_methods (MOO_TYPE_PANE, methods_MooPane);

    MooLuaMethodEntry methods_MooPlugin[] = {
        { "set_info", cfunc_MooPlugin_set_info },
        { NULL, NULL }
    };
    moo_lua_register_methods (MOO_TYPE_PLUGIN, methods_MooPlugin);

    MooLuaMethodEntry methods_MooTextView[] = {
        { "set_lang_by_id", cfunc_MooTextView_set_lang_by_id },
        { NULL, NULL }
    };
    moo_lua_register_methods (MOO_TYPE_TEXT_VIEW, methods_MooTextView);

    MooLuaMethodEntry methods_MooUiXml[] = {
        { "add_item", cfunc_MooUiXml_add_item },
        { "create_widget", cfunc_MooUiXml_create_widget },
        { "find_placeholder", cfunc_MooUiXml_find_placeholder },
        { "get_node", cfunc_MooUiXml_get_node },
        { "get_widget", cfunc_MooUiXml_get_widget },
        { "insert", cfunc_MooUiXml_insert },
        { "insert_after", cfunc_MooUiXml_insert_after },
        { "insert_before", cfunc_MooUiXml_insert_before },
        { "insert_markup", cfunc_MooUiXml_insert_markup },
        { "insert_markup_after", cfunc_MooUiXml_insert_markup_after },
        { "insert_markup_before", cfunc_MooUiXml_insert_markup_before },
        { "new_merge_id", cfunc_MooUiXml_new_merge_id },
        { "remove_node", cfunc_MooUiXml_remove_node },
        { "remove_ui", cfunc_MooUiXml_remove_ui },
        { NULL, NULL }
    };
    moo_lua_register_methods (MOO_TYPE_UI_XML, methods_MooUiXml);

    MooLuaMethodEntry methods_MooWinPlugin[] = {
        { "get_plugin", cfunc_MooWinPlugin_get_plugin },
        { "get_window", cfunc_MooWinPlugin_get_window },
        { NULL, NULL }
    };
    moo_lua_register_methods (MOO_TYPE_WIN_PLUGIN, methods_MooWinPlugin);

}
