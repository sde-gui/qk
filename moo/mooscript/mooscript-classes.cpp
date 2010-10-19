#include "mooscript-classes.h"
#include "mooscript-classes-util.h"

namespace mom {

MOM_GOBJECT_DEFN(DocumentView, MooEdit)
MOM_SINGLETON_DEFN(Application)
MOM_GOBJECT_DEFN(Document, MooEdit)
MOM_SINGLETON_DEFN(Editor)
MOM_GOBJECT_DEFN(DocumentWindow, MooEditWindow)

void DocumentView::InitMetaObject(MetaObject &meta)
{
    meta.add_method("set_show_line_numbers", &DocumentView::_set_show_line_numbers);
    meta.add_method("set_line_wrap_mode", &DocumentView::_set_line_wrap_mode);
    meta.add_method("overwrite_mode", &DocumentView::_overwrite_mode);
    meta.add_method("set_overwrite_mode", &DocumentView::_set_overwrite_mode);
    meta.add_method("window", &DocumentView::_window);
    meta.add_method("line_wrap_mode", &DocumentView::_line_wrap_mode);
    meta.add_method("show_line_numbers", &DocumentView::_show_line_numbers);
    meta.add_method("document", &DocumentView::_document);
}

void Application::InitMetaObject(MetaObject &meta)
{
    meta.add_method("active_document", &Application::_active_document);
    meta.add_method("quit", &Application::_quit);
    meta.add_method("active_window", &Application::_active_window);
    meta.add_method("set_active_window", &Application::_set_active_window);
    meta.add_method("windows", &Application::_windows);
    meta.add_method("editor", &Application::_editor);
    meta.add_method("active_view", &Application::_active_view);
}

void Document::InitMetaObject(MetaObject &meta)
{
    meta.add_method("line_endings", &Document::_line_endings);
    meta.add_method("set_encoding", &Document::_set_encoding);
    meta.add_method("encoding", &Document::_encoding);
    meta.add_method("set_line_endings", &Document::_set_line_endings);
    meta.add_method("text", &Document::_text);
    meta.add_method("basename", &Document::_basename);
    meta.add_method("end_not_undoable_action", &Document::_end_not_undoable_action);
    meta.add_method("set_selection", &Document::_set_selection);
    meta.add_method("selection", &Document::_selection);
    meta.add_method("replace_selected_lines", &Document::_replace_selected_lines);
    meta.add_method("redo", &Document::_redo);
    meta.add_method("save_as", &Document::_save_as);
    meta.add_method("cut", &Document::_cut);
    meta.add_method("select_lines_at_pos", &Document::_select_lines_at_pos);
    meta.add_method("select_all", &Document::_select_all);
    meta.add_method("select_text", &Document::_select_text);
    meta.add_method("line_count", &Document::_line_count);
    meta.add_method("can_redo", &Document::_can_redo);
    meta.add_method("line_at_pos", &Document::_line_at_pos);
    meta.add_method("filename", &Document::_filename);
    meta.add_method("append_text", &Document::_append_text);
    meta.add_method("replace_selected_text", &Document::_replace_selected_text);
    meta.add_method("start_pos", &Document::_start_pos);
    meta.add_method("end_pos", &Document::_end_pos);
    meta.add_method("selection_bound", &Document::_selection_bound);
    meta.add_method("pos_at_line_end", &Document::_pos_at_line_end);
    meta.add_method("paste", &Document::_paste);
    meta.add_method("replace_text", &Document::_replace_text);
    meta.add_method("views", &Document::_views);
    meta.add_method("has_selection", &Document::_has_selection);
    meta.add_method("selected_lines", &Document::_selected_lines);
    meta.add_method("undo", &Document::_undo);
    meta.add_method("select_lines", &Document::_select_lines);
    meta.add_method("delete_selected_lines", &Document::_delete_selected_lines);
    meta.add_method("pos_at_line", &Document::_pos_at_line);
    meta.add_method("active_view", &Document::_active_view);
    meta.add_method("cursor_pos", &Document::_cursor_pos);
    meta.add_method("copy", &Document::_copy);
    meta.add_method("char_at_pos", &Document::_char_at_pos);
    meta.add_method("selected_text", &Document::_selected_text);
    meta.add_method("begin_not_undoable_action", &Document::_begin_not_undoable_action);
    meta.add_method("save", &Document::_save);
    meta.add_method("insert_text", &Document::_insert_text);
    meta.add_method("char_count", &Document::_char_count);
    meta.add_method("uri", &Document::_uri);
    meta.add_method("reload", &Document::_reload);
    meta.add_method("delete_text", &Document::_delete_text);
    meta.add_method("can_undo", &Document::_can_undo);
    meta.add_method("set_cursor_pos", &Document::_set_cursor_pos);
    meta.add_method("delete_selected_text", &Document::_delete_selected_text);
    meta.add_method("clear", &Document::_clear);
}

void Editor::InitMetaObject(MetaObject &meta)
{
    meta.add_method("get_document_by_uri", &Editor::_get_document_by_uri);
    meta.add_method("active_document", &Editor::_active_document);
    meta.add_method("get_document_by_path", &Editor::_get_document_by_path);
    meta.add_method("documents", &Editor::_documents);
    meta.add_method("windows", &Editor::_windows);
    meta.add_method("views", &Editor::_views);
    meta.add_method("set_active_document", &Editor::_set_active_document);
    meta.add_method("new_file", &Editor::_new_file);
    meta.add_method("set_active_view", &Editor::_set_active_view);
    meta.add_method("set_active_window", &Editor::_set_active_window);
    meta.add_method("active_window", &Editor::_active_window);
    meta.add_method("active_view", &Editor::_active_view);
    meta.add_method("reload", &Editor::_reload);
    meta.add_method("open_uris", &Editor::_open_uris);
    meta.add_method("open_file", &Editor::_open_file);
    meta.add_method("close", &Editor::_close);
    meta.add_method("open_files", &Editor::_open_files);
    meta.add_method("save", &Editor::_save);
    meta.add_method("open_uri", &Editor::_open_uri);
    meta.add_method("save_as", &Editor::_save_as);
    meta.add_signal("document-save-before");
    meta.add_signal("document-save-after");
}

void DocumentWindow::InitMetaObject(MetaObject &meta)
{
    meta.add_method("active_document", &DocumentWindow::_active_document);
    meta.add_method("documents", &DocumentWindow::_documents);
    meta.add_method("views", &DocumentWindow::_views);
    meta.add_method("set_active_document", &DocumentWindow::_set_active_document);
    meta.add_method("is_active", &DocumentWindow::_is_active);
    meta.add_method("set_active", &DocumentWindow::_set_active);
    meta.add_method("set_active_view", &DocumentWindow::_set_active_view);
    meta.add_method("editor", &DocumentWindow::_editor);
    meta.add_method("active_view", &DocumentWindow::_active_view);
}

Variant DocumentView::_set_show_line_numbers(const ArgArray &args)
{
    if (args.size() <= 0)
        Error::raise("argument 'show' missing");
    bool arg0 = get_arg_bool(args[0], "show");
    set_show_line_numbers(arg0);
    return Variant();
}

Variant DocumentView::_set_line_wrap_mode(const ArgArray &args)
{
    if (args.size() <= 0)
        Error::raise("argument 'enabled' missing");
    bool arg0 = get_arg_bool(args[0], "enabled");
    set_line_wrap_mode(arg0);
    return Variant();
}

Variant DocumentView::_overwrite_mode(const ArgArray &args)
{
    if (args.size() != 0)
        Error::raise("no arguments expected");
    return wrap_bool(overwrite_mode());
}

Variant DocumentView::_set_overwrite_mode(const ArgArray &args)
{
    if (args.size() <= 0)
        Error::raise("argument 'enabled' missing");
    bool arg0 = get_arg_bool(args[0], "enabled");
    set_overwrite_mode(arg0);
    return Variant();
}

Variant DocumentView::_window(const ArgArray &args)
{
    if (args.size() != 0)
        Error::raise("no arguments expected");
    return wrap_object(window());
}

Variant DocumentView::_line_wrap_mode(const ArgArray &args)
{
    if (args.size() != 0)
        Error::raise("no arguments expected");
    return wrap_bool(line_wrap_mode());
}

Variant DocumentView::_show_line_numbers(const ArgArray &args)
{
    if (args.size() != 0)
        Error::raise("no arguments expected");
    return wrap_bool(show_line_numbers());
}

Variant DocumentView::_document(const ArgArray &args)
{
    if (args.size() != 0)
        Error::raise("no arguments expected");
    return wrap_object(document());
}

Variant Application::_active_document(const ArgArray &args)
{
    if (args.size() != 0)
        Error::raise("no arguments expected");
    return wrap_object(active_document());
}

Variant Application::_quit(const ArgArray &args)
{
    if (args.size() != 0)
        Error::raise("no arguments expected");
    quit();
    return Variant();
}

Variant Application::_active_window(const ArgArray &args)
{
    if (args.size() != 0)
        Error::raise("no arguments expected");
    return wrap_object(active_window());
}

Variant Application::_set_active_window(const ArgArray &args)
{
    if (args.size() <= 0)
        Error::raise("argument 'window' missing");
    DocumentWindow &arg0 = get_object_arg<DocumentWindow>(args[0], "window");
    set_active_window(arg0);
    return Variant();
}

Variant Application::_windows(const ArgArray &args)
{
    if (args.size() != 0)
        Error::raise("no arguments expected");
    return wrap_array(windows());
}

Variant Application::_editor(const ArgArray &args)
{
    if (args.size() != 0)
        Error::raise("no arguments expected");
    return wrap_object(editor());
}

Variant Application::_active_view(const ArgArray &args)
{
    if (args.size() != 0)
        Error::raise("no arguments expected");
    return wrap_object(active_view());
}

Variant Document::_line_endings(const ArgArray &args)
{
    if (args.size() != 0)
        Error::raise("no arguments expected");
    return wrap_string(line_endings());
}

Variant Document::_set_encoding(const ArgArray &args)
{
    if (args.size() <= 0)
        Error::raise("argument 'encoding' missing");
    String arg0 = get_arg_string(args[0], "encoding");
    set_encoding(arg0);
    return Variant();
}

Variant Document::_encoding(const ArgArray &args)
{
    if (args.size() != 0)
        Error::raise("no arguments expected");
    return wrap_string(encoding());
}

Variant Document::_set_line_endings(const ArgArray &args)
{
    if (args.size() <= 0)
        Error::raise("argument 'value' missing");
    String arg0 = get_arg_string(args[0], "value");
    set_line_endings(arg0);
    return Variant();
}

Variant Document::_text(const ArgArray &args)
{
    return wrap_string(text(args));
}

Variant Document::_basename(const ArgArray &args)
{
    if (args.size() != 0)
        Error::raise("no arguments expected");
    return wrap_string(basename());
}

Variant Document::_end_not_undoable_action(const ArgArray &args)
{
    if (args.size() != 0)
        Error::raise("no arguments expected");
    end_not_undoable_action();
    return Variant();
}

Variant Document::_set_selection(const ArgArray &args)
{
    set_selection(args);
    return Variant();
}

Variant Document::_selection(const ArgArray &args)
{
    if (args.size() != 0)
        Error::raise("no arguments expected");
    return wrap_array(selection());
}

Variant Document::_replace_selected_lines(const ArgArray &args)
{
    if (args.size() <= 0)
        Error::raise("argument 'repl' missing");
    Variant arg0 = get_arg_variant(args[0], "repl");
    replace_selected_lines(arg0);
    return Variant();
}

Variant Document::_redo(const ArgArray &args)
{
    if (args.size() != 0)
        Error::raise("no arguments expected");
    redo();
    return Variant();
}

Variant Document::_save_as(const ArgArray &args)
{
    if (args.size() <= 0)
        Error::raise("argument 'filename' missing");
    String arg0 = get_arg_string(args[0], "filename");
    return wrap_bool(save_as(arg0));
}

Variant Document::_cut(const ArgArray &args)
{
    if (args.size() != 0)
        Error::raise("no arguments expected");
    cut();
    return Variant();
}

Variant Document::_select_lines_at_pos(const ArgArray &args)
{
    select_lines_at_pos(args);
    return Variant();
}

Variant Document::_select_all(const ArgArray &args)
{
    if (args.size() != 0)
        Error::raise("no arguments expected");
    select_all();
    return Variant();
}

Variant Document::_select_text(const ArgArray &args)
{
    select_text(args);
    return Variant();
}

Variant Document::_line_count(const ArgArray &args)
{
    if (args.size() != 0)
        Error::raise("no arguments expected");
    return wrap_int(line_count());
}

Variant Document::_can_redo(const ArgArray &args)
{
    if (args.size() != 0)
        Error::raise("no arguments expected");
    return wrap_bool(can_redo());
}

Variant Document::_line_at_pos(const ArgArray &args)
{
    if (args.size() <= 0)
        Error::raise("argument 'pos' missing");
    gint64 arg0 = get_arg_index(args[0], "pos");
    return wrap_index(line_at_pos(arg0));
}

Variant Document::_filename(const ArgArray &args)
{
    if (args.size() != 0)
        Error::raise("no arguments expected");
    return wrap_string(filename());
}

Variant Document::_append_text(const ArgArray &args)
{
    if (args.size() <= 0)
        Error::raise("argument 'text' missing");
    String arg0 = get_arg_string(args[0], "text");
    append_text(arg0);
    return Variant();
}

Variant Document::_replace_selected_text(const ArgArray &args)
{
    if (args.size() <= 0)
        Error::raise("argument 'text' missing");
    String arg0 = get_arg_string(args[0], "text");
    replace_selected_text(arg0);
    return Variant();
}

Variant Document::_start_pos(const ArgArray &args)
{
    if (args.size() != 0)
        Error::raise("no arguments expected");
    return wrap_index(start_pos());
}

Variant Document::_end_pos(const ArgArray &args)
{
    if (args.size() != 0)
        Error::raise("no arguments expected");
    return wrap_index(end_pos());
}

Variant Document::_selection_bound(const ArgArray &args)
{
    if (args.size() != 0)
        Error::raise("no arguments expected");
    return wrap_index(selection_bound());
}

Variant Document::_pos_at_line_end(const ArgArray &args)
{
    if (args.size() <= 0)
        Error::raise("argument 'line' missing");
    gint64 arg0 = get_arg_index(args[0], "line");
    return wrap_index(pos_at_line_end(arg0));
}

Variant Document::_paste(const ArgArray &args)
{
    if (args.size() != 0)
        Error::raise("no arguments expected");
    paste();
    return Variant();
}

Variant Document::_replace_text(const ArgArray &args)
{
    if (args.size() <= 0)
        Error::raise("argument 'start' missing");
    gint64 arg0 = get_arg_index(args[0], "start");
    if (args.size() <= 1)
        Error::raise("argument 'end' missing");
    gint64 arg1 = get_arg_index(args[1], "end");
    if (args.size() <= 2)
        Error::raise("argument 'text' missing");
    String arg2 = get_arg_string(args[2], "text");
    replace_text(arg0, arg1, arg2);
    return Variant();
}

Variant Document::_views(const ArgArray &args)
{
    if (args.size() != 0)
        Error::raise("no arguments expected");
    return wrap_array(views());
}

Variant Document::_has_selection(const ArgArray &args)
{
    if (args.size() != 0)
        Error::raise("no arguments expected");
    return wrap_bool(has_selection());
}

Variant Document::_selected_lines(const ArgArray &args)
{
    if (args.size() != 0)
        Error::raise("no arguments expected");
    return wrap_array(selected_lines());
}

Variant Document::_undo(const ArgArray &args)
{
    if (args.size() != 0)
        Error::raise("no arguments expected");
    undo();
    return Variant();
}

Variant Document::_select_lines(const ArgArray &args)
{
    select_lines(args);
    return Variant();
}

Variant Document::_delete_selected_lines(const ArgArray &args)
{
    if (args.size() != 0)
        Error::raise("no arguments expected");
    delete_selected_lines();
    return Variant();
}

Variant Document::_pos_at_line(const ArgArray &args)
{
    if (args.size() <= 0)
        Error::raise("argument 'line' missing");
    gint64 arg0 = get_arg_index(args[0], "line");
    return wrap_index(pos_at_line(arg0));
}

Variant Document::_active_view(const ArgArray &args)
{
    if (args.size() != 0)
        Error::raise("no arguments expected");
    return wrap_object(active_view());
}

Variant Document::_cursor_pos(const ArgArray &args)
{
    if (args.size() != 0)
        Error::raise("no arguments expected");
    return wrap_index(cursor_pos());
}

Variant Document::_copy(const ArgArray &args)
{
    if (args.size() != 0)
        Error::raise("no arguments expected");
    copy();
    return Variant();
}

Variant Document::_char_at_pos(const ArgArray &args)
{
    if (args.size() <= 0)
        Error::raise("argument 'pos' missing");
    gint64 arg0 = get_arg_index(args[0], "pos");
    return wrap_string(char_at_pos(arg0));
}

Variant Document::_selected_text(const ArgArray &args)
{
    if (args.size() != 0)
        Error::raise("no arguments expected");
    return wrap_string(selected_text());
}

Variant Document::_begin_not_undoable_action(const ArgArray &args)
{
    if (args.size() != 0)
        Error::raise("no arguments expected");
    begin_not_undoable_action();
    return Variant();
}

Variant Document::_save(const ArgArray &args)
{
    if (args.size() != 0)
        Error::raise("no arguments expected");
    return wrap_bool(save());
}

Variant Document::_insert_text(const ArgArray &args)
{
    insert_text(args);
    return Variant();
}

Variant Document::_char_count(const ArgArray &args)
{
    if (args.size() != 0)
        Error::raise("no arguments expected");
    return wrap_int(char_count());
}

Variant Document::_uri(const ArgArray &args)
{
    if (args.size() != 0)
        Error::raise("no arguments expected");
    return wrap_string(uri());
}

Variant Document::_reload(const ArgArray &args)
{
    if (args.size() != 0)
        Error::raise("no arguments expected");
    reload();
    return Variant();
}

Variant Document::_delete_text(const ArgArray &args)
{
    if (args.size() <= 0)
        Error::raise("argument 'start' missing");
    gint64 arg0 = get_arg_index(args[0], "start");
    if (args.size() <= 1)
        Error::raise("argument 'end' missing");
    gint64 arg1 = get_arg_index(args[1], "end");
    delete_text(arg0, arg1);
    return Variant();
}

Variant Document::_can_undo(const ArgArray &args)
{
    if (args.size() != 0)
        Error::raise("no arguments expected");
    return wrap_bool(can_undo());
}

Variant Document::_set_cursor_pos(const ArgArray &args)
{
    if (args.size() <= 0)
        Error::raise("argument 'pos' missing");
    gint64 arg0 = get_arg_index(args[0], "pos");
    set_cursor_pos(arg0);
    return Variant();
}

Variant Document::_delete_selected_text(const ArgArray &args)
{
    if (args.size() != 0)
        Error::raise("no arguments expected");
    delete_selected_text();
    return Variant();
}

Variant Document::_clear(const ArgArray &args)
{
    if (args.size() != 0)
        Error::raise("no arguments expected");
    clear();
    return Variant();
}

Variant Editor::_get_document_by_uri(const ArgArray &args)
{
    if (args.size() <= 0)
        Error::raise("argument 'uri' missing");
    String arg0 = get_arg_string(args[0], "uri");
    return wrap_object(get_document_by_uri(arg0));
}

Variant Editor::_active_document(const ArgArray &args)
{
    if (args.size() != 0)
        Error::raise("no arguments expected");
    return wrap_object(active_document());
}

Variant Editor::_get_document_by_path(const ArgArray &args)
{
    if (args.size() <= 0)
        Error::raise("argument 'path' missing");
    String arg0 = get_arg_string(args[0], "path");
    return wrap_object(get_document_by_path(arg0));
}

Variant Editor::_documents(const ArgArray &args)
{
    if (args.size() != 0)
        Error::raise("no arguments expected");
    return wrap_array(documents());
}

Variant Editor::_windows(const ArgArray &args)
{
    if (args.size() != 0)
        Error::raise("no arguments expected");
    return wrap_array(windows());
}

Variant Editor::_views(const ArgArray &args)
{
    if (args.size() != 0)
        Error::raise("no arguments expected");
    return wrap_array(views());
}

Variant Editor::_set_active_document(const ArgArray &args)
{
    if (args.size() <= 0)
        Error::raise("argument 'doc' missing");
    Document &arg0 = get_object_arg<Document>(args[0], "doc");
    set_active_document(arg0);
    return Variant();
}

Variant Editor::_new_file(const ArgArray &args)
{
    String arg0 = get_arg_string_opt(args.size() > 0 ? args[0] : Variant(), "file");
    String arg1 = get_arg_string_opt(args.size() > 1 ? args[1] : Variant(), "encoding");
    DocumentWindow *arg2 = get_object_arg_opt<DocumentWindow>(args.size() > 2 ? args[2] : Variant(), "window");
    return wrap_object(new_file(arg0, arg1, arg2));
}

Variant Editor::_set_active_view(const ArgArray &args)
{
    if (args.size() <= 0)
        Error::raise("argument 'view' missing");
    DocumentView &arg0 = get_object_arg<DocumentView>(args[0], "view");
    set_active_view(arg0);
    return Variant();
}

Variant Editor::_set_active_window(const ArgArray &args)
{
    if (args.size() <= 0)
        Error::raise("argument 'window' missing");
    DocumentWindow &arg0 = get_object_arg<DocumentWindow>(args[0], "window");
    set_active_window(arg0);
    return Variant();
}

Variant Editor::_active_window(const ArgArray &args)
{
    if (args.size() != 0)
        Error::raise("no arguments expected");
    return wrap_object(active_window());
}

Variant Editor::_active_view(const ArgArray &args)
{
    if (args.size() != 0)
        Error::raise("no arguments expected");
    return wrap_object(active_view());
}

Variant Editor::_reload(const ArgArray &args)
{
    if (args.size() <= 0)
        Error::raise("argument 'doc' missing");
    Document &arg0 = get_object_arg<Document>(args[0], "doc");
    reload(arg0);
    return Variant();
}

Variant Editor::_open_uris(const ArgArray &args)
{
    if (args.size() <= 0)
        Error::raise("argument 'uris' missing");
    VariantArray arg0 = get_arg_array(args[0], "uris");
    DocumentWindow *arg1 = get_object_arg_opt<DocumentWindow>(args.size() > 1 ? args[1] : Variant(), "window");
    open_uris(arg0, arg1);
    return Variant();
}

Variant Editor::_open_file(const ArgArray &args)
{
    if (args.size() <= 0)
        Error::raise("argument 'file' missing");
    String arg0 = get_arg_string(args[0], "file");
    String arg1 = get_arg_string_opt(args.size() > 1 ? args[1] : Variant(), "encoding");
    DocumentWindow *arg2 = get_object_arg_opt<DocumentWindow>(args.size() > 2 ? args[2] : Variant(), "window");
    return wrap_object(open_file(arg0, arg1, arg2));
}

Variant Editor::_close(const ArgArray &args)
{
    if (args.size() <= 0)
        Error::raise("argument 'doc' missing");
    Document &arg0 = get_object_arg<Document>(args[0], "doc");
    return wrap_bool(close(arg0));
}

Variant Editor::_open_files(const ArgArray &args)
{
    if (args.size() <= 0)
        Error::raise("argument 'files' missing");
    VariantArray arg0 = get_arg_array(args[0], "files");
    DocumentWindow *arg1 = get_object_arg_opt<DocumentWindow>(args.size() > 1 ? args[1] : Variant(), "window");
    open_files(arg0, arg1);
    return Variant();
}

Variant Editor::_save(const ArgArray &args)
{
    if (args.size() <= 0)
        Error::raise("argument 'doc' missing");
    Document &arg0 = get_object_arg<Document>(args[0], "doc");
    return wrap_bool(save(arg0));
}

Variant Editor::_open_uri(const ArgArray &args)
{
    if (args.size() <= 0)
        Error::raise("argument 'file' missing");
    String arg0 = get_arg_string(args[0], "file");
    String arg1 = get_arg_string_opt(args.size() > 1 ? args[1] : Variant(), "encoding");
    DocumentWindow *arg2 = get_object_arg_opt<DocumentWindow>(args.size() > 2 ? args[2] : Variant(), "window");
    return wrap_object(open_uri(arg0, arg1, arg2));
}

Variant Editor::_save_as(const ArgArray &args)
{
    if (args.size() <= 0)
        Error::raise("argument 'doc' missing");
    Document &arg0 = get_object_arg<Document>(args[0], "doc");
    String arg1 = get_arg_string_opt(args.size() > 1 ? args[1] : Variant(), "filename");
    return wrap_bool(save_as(arg0, arg1));
}

Variant DocumentWindow::_active_document(const ArgArray &args)
{
    if (args.size() != 0)
        Error::raise("no arguments expected");
    return wrap_object(active_document());
}

Variant DocumentWindow::_documents(const ArgArray &args)
{
    if (args.size() != 0)
        Error::raise("no arguments expected");
    return wrap_array(documents());
}

Variant DocumentWindow::_views(const ArgArray &args)
{
    if (args.size() != 0)
        Error::raise("no arguments expected");
    return wrap_array(views());
}

Variant DocumentWindow::_set_active_document(const ArgArray &args)
{
    if (args.size() <= 0)
        Error::raise("argument 'doc' missing");
    Document &arg0 = get_object_arg<Document>(args[0], "doc");
    set_active_document(arg0);
    return Variant();
}

Variant DocumentWindow::_is_active(const ArgArray &args)
{
    if (args.size() != 0)
        Error::raise("no arguments expected");
    return wrap_bool(is_active());
}

Variant DocumentWindow::_set_active(const ArgArray &args)
{
    if (args.size() != 0)
        Error::raise("no arguments expected");
    set_active();
    return Variant();
}

Variant DocumentWindow::_set_active_view(const ArgArray &args)
{
    if (args.size() <= 0)
        Error::raise("argument 'view' missing");
    DocumentView &arg0 = get_object_arg<DocumentView>(args[0], "view");
    set_active_view(arg0);
    return Variant();
}

Variant DocumentWindow::_editor(const ArgArray &args)
{
    if (args.size() != 0)
        Error::raise("no arguments expected");
    return wrap_object(editor());
}

Variant DocumentWindow::_active_view(const ArgArray &args)
{
    if (args.size() != 0)
        Error::raise("no arguments expected");
    return wrap_object(active_view());
}

} // namespace mom
