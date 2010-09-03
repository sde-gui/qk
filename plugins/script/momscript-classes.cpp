#include "momscript-classes.h"
#include "mooapp/mooapp.h"

namespace mom {

#include "momscript-classes-meta.h"

static void check_no_args(const VariantArray &args)
{
    if (args.size() != 0)
        Error::raise("no arguments expected");
}

static void check_1_arg(const VariantArray &args)
{
    if (args.size() != 1)
        Error::raise("exactly one argument expected");
}

template<typename T>
static moo::SharedPtr<T> get_object(const Variant &val, bool null_ok = false)
{
    if (null_ok && val.vt() == VtVoid)
        return moo::SharedPtr<T>();

    if (val.vt() != VtObject)
        Error::raise("object expected");

    HObject h = val.value<VtObject>();
    if (null_ok && h.id() == 0)
        return moo::SharedPtr<T>();

    moo::SharedPtr<Object> obj = Object::lookup_object(h);

    if (!obj)
        Error::raise("bad object");
    if (&obj->meta() != &T::class_meta())
        Error::raise("bad object");

    return moo::SharedPtr<T>(static_cast<T*>(obj.get()));
}

static bool get_bool(const Variant &val)
{
    if (val.vt() != VtBool)
        Error::raise("boolean expected");
    return val.value<VtBool>();
}

static String get_string(const Variant &val)
{
    if (val.vt() != VtString)
        Error::raise("string expected");

    return val.value<VtString>();
}

template<typename GClass, typename Class>
static Variant wrap_gslist(GSList *list)
{
    VariantArray array;
    while (list)
    {
        array.append(HObject(Class::wrap((GClass*)list->data)));
        list = list->next;
    }
    return array;
}

static Index get_index(const Variant &val)
{
    switch (val.vt())
    {
        case VtIndex:
            return val.value<VtIndex>();
        case VtInt:
            return Index(val.value<VtInt>());
        case VtBase1:
            return val.value<VtBase1>().get_index();
        default:
            Error::raise("index expected");
    }
}

static void get_iter(const Variant &val, GtkTextBuffer *buf, GtkTextIter *iter)
{
    Index idx = get_index(val);
    if (idx.get() > gtk_text_buffer_get_char_count(buf) || idx.get() < 0)
        Error::raise("invalid offset");
    gtk_text_buffer_get_iter_at_offset(buf, iter, idx.get());
}

static void get_pair(const Variant &val, Variant &elm1, Variant &elm2)
{
    if (val.vt() != VtArray)
        Error::raise("pair of values expected");
    const VariantArray &ar = val.value<VtArray>();
    if (ar.size() != 2)
        Error::raise("pair of values expected");
    elm1 = ar[0];
    elm2 = ar[1];
}

static Variant make_pair(const Variant &elm1, const Variant &elm2)
{
    VariantArray ar;
    ar.append(elm1);
    ar.append(elm2);
    return ar;
}

static void get_iter_pair(const Variant &val, GtkTextBuffer *buf, GtkTextIter *iter1, GtkTextIter *iter2)
{
    Variant elm1, elm2;
    get_pair(val, elm1, elm2);
    get_iter(elm1, buf, iter1);
    get_iter(elm2, buf, iter2);
}

///////////////////////////////////////////////////////////////////////////////
//
// Global
//

Variant Global::get_application()
{
    return HObject(Application::get_instance());
}

Variant Global::get_editor()
{
    return HObject(Editor::get_instance());
}

Variant Global::get_active_window()
{
    return Application::get_instance().get_active_window();
}

Variant Global::get_active_view()
{
    return Editor::get_instance().get_active_view();
}

Variant Global::get_active_document()
{
    return Editor::get_instance().get_active_document();
}


///////////////////////////////////////////////////////////////////////////////
//
// Application
//

Variant Application::get_editor()
{
    return HObject(Editor::get_instance());
}

Variant Application::get_active_window()
{
    return Editor::get_instance().get_active_window();
}

void Application::set_active_window(const Variant &val)
{
    Editor::get_instance().set_active_window(val);
}

Variant Application::get_windows()
{
    return Editor::get_instance().get_windows();
}

Variant Application::quit(const VariantArray &args)
{
    check_no_args(args);
    moo_app_quit(moo_app_get_instance());
    return Variant();
}


///////////////////////////////////////////////////////////////////////////////
//
// Editor
//

Variant Editor::get_active_document()
{
    return HObject(Document::wrap(moo_editor_get_active_doc(moo_editor_instance())));
}

void Editor::set_active_document(const Variant &val)
{
    moo::SharedPtr<Document> doc = get_object<Document>(val);
    moo_editor_set_active_doc(moo_editor_instance(), doc->gobj());
}

Variant Editor::get_active_window()
{
    return HObject(DocumentWindow::wrap(moo_editor_get_active_window(moo_editor_instance())));
}

void Editor::set_active_window(const Variant &val)
{
    moo::SharedPtr<DocumentWindow> window = get_object<DocumentWindow>(val);
    moo_editor_set_active_window(moo_editor_instance(), window->gobj());
}

Variant Editor::get_active_view()
{
    return HObject(DocumentView::wrap(moo_editor_get_active_doc(moo_editor_instance())));
}

void Editor::set_active_view(const Variant &val)
{
    moo::SharedPtr<DocumentView> view = get_object<DocumentView>(val);
    moo_editor_set_active_doc(moo_editor_instance(), view->gobj());
    g_print("Editor::set_active_view\n");
}

Variant Editor::get_documents()
{
    GSList *docs = moo_editor_list_docs(moo_editor_instance());
    return wrap_gslist<MooEdit, Document>(docs);
}

Variant Editor::get_views()
{
    GSList *docs = moo_editor_list_docs(moo_editor_instance());
    return wrap_gslist<MooEdit, DocumentView>(docs);
}

Variant Editor::get_windows()
{
    GSList *windows = moo_editor_list_windows(moo_editor_instance());
    return wrap_gslist<MooEditWindow, DocumentWindow>(windows);
}


///////////////////////////////////////////////////////////////////////////////
//
// DocumentWindow
//

Variant DocumentWindow::get_editor()
{
    return HObject(Editor::get_instance());
}

Variant DocumentWindow::get_active_view()
{
    return HObject(DocumentView::wrap(moo_edit_window_get_active_doc(gobj())));
}

void DocumentWindow::set_active_view(const Variant &val)
{
    moo::SharedPtr<DocumentView> view = get_object<DocumentView>(val);
    moo_edit_window_set_active_doc(gobj(), view->gobj());
}

Variant DocumentWindow::get_active_document()
{
    return HObject(Document::wrap(moo_edit_window_get_active_doc(gobj())));
}

void DocumentWindow::set_active_document(const Variant &val)
{
    moo::SharedPtr<Document> doc = get_object<Document>(val);
    moo_edit_window_set_active_doc(gobj(), doc->gobj());
}

Variant DocumentWindow::get_views()
{
    GSList *docs = moo_edit_window_list_docs(gobj());
    return wrap_gslist<MooEdit, DocumentView>(docs);
}

Variant DocumentWindow::get_documents()
{
    GSList *docs = moo_edit_window_list_docs(gobj());
    return wrap_gslist<MooEdit, Document>(docs);
}

Variant DocumentWindow::get_active()
{
    return gobj() == moo_editor_get_active_window(moo_editor_instance());
}

Variant DocumentWindow::set_active(const VariantArray &args)
{
    check_no_args(args);
    moo_editor_set_active_window(moo_editor_instance(), gobj());
    return Variant();
}


///////////////////////////////////////////////////////////////////////////////
//
// DocumentView
//

Variant DocumentView::get_document()
{
    return HObject(Document::wrap(gobj()));
}

Variant DocumentView::get_window()
{
    return HObject(DocumentWindow::wrap(moo_edit_get_window(gobj())));
}

Variant DocumentView::get_line_wrap_mode()
{
    GtkWrapMode mode;
    g_object_get(gobj(), "wrap-mode", &mode, (char*) NULL);
    return mode != GTK_WRAP_NONE;
}

void DocumentView::set_line_wrap_mode(const Variant &val)
{
    moo_edit_ui_set_line_wrap_mode(gobj(), get_bool(val));
}

Variant DocumentView::get_overwrite_mode()
{
    gboolean overwrite;
    g_object_get(gobj(), "overwrite", &overwrite, (char*) NULL);
    return bool(overwrite);
}

void DocumentView::set_overwrite_mode(const Variant &val)
{
    g_object_set(gobj(), "overwrite", gboolean(get_bool(val)), (char*) NULL);
}

Variant DocumentView::get_show_line_numbers()
{
    gboolean show;
    g_object_get(gobj(), "show-line-numbers", &show, (char*) NULL);
    return bool(show);
}

void DocumentView::set_show_line_numbers(const Variant &val)
{
    moo_edit_ui_set_show_line_numbers(gobj(), get_bool(val));
}


///////////////////////////////////////////////////////////////////////////////
//
// Document
//

Variant Document::get_active_view()
{
    return HObject(DocumentView::wrap(gobj()));
}

Variant Document::get_views()
{
    VariantArray views;
    views.append(HObject(DocumentView::wrap(gobj())));
    return views;
}

Variant Document::get_filename()
{
    char *filename = moo_edit_get_utf8_filename(gobj());
    return filename ? String::take_utf8(filename) : String::Null();
}

Variant Document::get_uri()
{
    char *uri = moo_edit_get_uri(gobj());
    return uri ? String::take_utf8(uri) : String::Null();
}

Variant Document::get_basename()
{
    return String(moo_edit_get_display_basename(gobj()));
}

Variant Document::get_can_undo()
{
    return moo_text_view_can_undo(MOO_TEXT_VIEW(gobj()));
}

Variant Document::get_can_redo()
{
    return moo_text_view_can_redo(MOO_TEXT_VIEW(gobj()));
}

Variant Document::undo(const VariantArray &args)
{
    check_no_args(args);
    moo_text_view_undo(MOO_TEXT_VIEW(gobj()));
    return Variant();
}

Variant Document::redo(const VariantArray &args)
{
    check_no_args(args);
    moo_text_view_redo(MOO_TEXT_VIEW(gobj()));
    return Variant();
}

Variant Document::begin_not_undoable_action(const VariantArray &args)
{
    check_no_args(args);
    moo_text_view_begin_not_undoable_action(MOO_TEXT_VIEW(gobj()));
    return Variant();
}

Variant Document::end_not_undoable_action(const VariantArray &args)
{
    check_no_args(args);
    moo_text_view_end_not_undoable_action(MOO_TEXT_VIEW(gobj()));
    return Variant();
}

Variant Document::get_start()
{
    return Index(0);
}

Variant Document::get_end()
{
    return Index(gtk_text_buffer_get_char_count(buffer()));
}

Variant Document::get_cursor()
{
    GtkTextBuffer *buf = buffer();
    GtkTextIter iter;
    gtk_text_buffer_get_iter_at_mark(buf, &iter, gtk_text_buffer_get_insert(buf));
    return Index(gtk_text_iter_get_offset(&iter));
}

void Document::set_cursor(const Variant &value)
{
    GtkTextBuffer *buf = buffer();
    GtkTextIter iter;
    get_iter(value, buf, &iter);
    gtk_text_buffer_place_cursor(buf, &iter);
}

Variant Document::get_selection_bound()
{
    GtkTextBuffer *buf = buffer();
    GtkTextIter iter;
    gtk_text_buffer_get_iter_at_mark(buf, &iter, gtk_text_buffer_get_selection_bound(buf));
    return Index(gtk_text_iter_get_offset(&iter));
}

Variant Document::get_selection()
{
    GtkTextBuffer *buf = buffer();
    GtkTextIter start, end;
    gtk_text_buffer_get_selection_bounds(buf, &start, &end);
    return make_pair(Index(gtk_text_iter_get_offset(&start)),
                     Index(gtk_text_iter_get_offset(&end)));
}

void Document::set_selection(const Variant &value)
{
    GtkTextBuffer *buf = buffer();
    GtkTextIter start, end;
    get_iter_pair(value, buf, &start, &end);
    gtk_text_buffer_select_range(buf, &start, &end);
}

Variant Document::get_has_selection()
{
    GtkTextBuffer *buf = buffer();
    GtkTextIter start, end;
    gtk_text_buffer_get_selection_bounds(buf, &start, &end);
    return bool(gtk_text_iter_equal(&start, &end));
}

Variant Document::get_char_count()
{
    return gtk_text_buffer_get_char_count(buffer());
}

Variant Document::get_line_count()
{
    return gtk_text_buffer_get_line_count(buffer());
}

Variant Document::line_at_pos(const VariantArray &args)
{
    check_1_arg(args);
    GtkTextBuffer *buf = buffer();
    GtkTextIter iter;
    get_iter(args[0], buf, &iter);
    return Index(gtk_text_iter_get_line(&iter));
}

Variant Document::pos_at_line(const VariantArray &args)
{
    check_1_arg(args);
    Index idx = get_index(args[0]);
    GtkTextBuffer *buf = buffer();
    if (idx.get() < 0 || idx.get() >= gtk_text_buffer_get_line_count(buf))
        Error::raise("invalid line");
    GtkTextIter iter;
    gtk_text_buffer_get_iter_at_line(buf, &iter, idx.get());
    return Index(gtk_text_iter_get_offset(&iter));
}

Variant Document::pos_at_line_end(const VariantArray &args)
{
    check_1_arg(args);
    Index idx = get_index(args[0]);
    GtkTextBuffer *buf = buffer();
    if (idx.get() < 0 || idx.get() >= gtk_text_buffer_get_line_count(buf))
        Error::raise("invalid line");
    GtkTextIter iter;
    gtk_text_buffer_get_iter_at_line(buf, &iter, idx.get());
    gtk_text_iter_forward_to_line_end(&iter);
    return Index(gtk_text_iter_get_offset(&iter));
}

Variant Document::char_at_pos(const VariantArray &args)
{
    check_1_arg(args);
    GtkTextBuffer *buf = buffer();
    GtkTextIter iter;
    get_iter(args[0], buf, &iter);
    if (gtk_text_iter_is_end(&iter))
        Error::raise("can't get text at end of buffer");
    gunichar c = gtk_text_iter_get_char(&iter);
    char b[7];
    b[g_unichar_to_utf8(c, b)] = 0;
    return String(b);
}

Variant Document::text(const VariantArray &args)
{
    if (args.size() == 0)
    {
        GtkTextBuffer *buf = buffer();
        GtkTextIter start, end;
        gtk_text_buffer_get_bounds(buf, &start, &end);
        return String::take_utf8(gtk_text_buffer_get_slice(buf, &start, &end, TRUE));
    }

    if (args.size() != 2)
        Error::raise("either 0 or 2 arguments expected");

    GtkTextBuffer *buf = buffer();
    GtkTextIter start, end;
    get_iter(args[0], buf, &start);
    get_iter(args[1], buf, &end);
    return String::take_utf8(gtk_text_buffer_get_slice(buf, &start, &end, TRUE));
}

Variant Document::insert_text(const VariantArray &args)
{
    String text;
    GtkTextIter iter;
    GtkTextBuffer *buf = buffer();

    if (args.size() == 1)
    {
        text = get_string(args[0]);
        gtk_text_buffer_get_iter_at_mark(buf, &iter, gtk_text_buffer_get_insert(buf));
    }
    else if (args.size() == 2)
    {
        get_iter(args[0], buf, &iter);
        text = get_string(args[1]);
    }
    else
    {
        Error::raise("one or two arguments expected");
    }

    gtk_text_buffer_insert(buf, &iter, text.utf8(), -1);

    return Variant();
}

Variant Document::replace_text(const VariantArray &args)
{
    String text;
    GtkTextIter start, end;
    GtkTextBuffer *buf = buffer();

    if (args.size() != 3)
        Error::raise("exactly three arguments expected");

    get_iter(args[0], buf, &start);
    get_iter(args[1], buf, &end);
    text = get_string(args[2]);

    gtk_text_buffer_delete(buf, &start, &end);
    gtk_text_buffer_insert(buf, &start, text.utf8(), -1);

    return Variant();
}

Variant Document::delete_text(const VariantArray &args)
{
    GtkTextIter start, end;
    GtkTextBuffer *buf = buffer();

    if (args.size() != 2)
        Error::raise("exactly two arguments expected");

    get_iter(args[0], buf, &start);
    get_iter(args[1], buf, &end);

    gtk_text_buffer_delete(buf, &start, &end);

    return Variant();
}

Variant Document::append_text(const VariantArray &args)
{
    String text;
    GtkTextIter iter;
    GtkTextBuffer *buf = buffer();

    if (args.size() != 1)
        Error::raise("exactly one argument expected");

    text = get_string(args[0]);

    gtk_text_buffer_get_end_iter(buf, &iter);
    gtk_text_buffer_insert(buf, &iter, text.utf8(), -1);

    return Variant();
}

Variant Document::clear(const VariantArray &args)
{
    check_no_args(args);
    GtkTextBuffer *buf = buffer();
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(buf, &start, &end);
    gtk_text_buffer_delete(buf, &start, &end);
    return Variant();
}

Variant Document::copy(const VariantArray &args)
{
    check_no_args(args);
    g_signal_emit_by_name(gobj(), "copy-clipboard");
    return Variant();
}

Variant Document::cut(const VariantArray &args)
{
    check_no_args(args);
    g_signal_emit_by_name(gobj(), "cut-clipboard");
    return Variant();
}

Variant Document::paste(const VariantArray &args)
{
    check_no_args(args);
    g_signal_emit_by_name(gobj(), "paste-clipboard");
    return Variant();
}

Variant Document::select_text(const VariantArray &args)
{
    GtkTextIter start, end;
    GtkTextBuffer *buf = buffer();

    if (args.size() == 2)
    {
        get_iter(args[0], buf, &start);
        get_iter(args[1], buf, &end);
    }
    else if (args.size() == 1)
    {
        get_iter_pair(args[0], buf, &start, &end);
    }
    else
    {
        Error::raise("exactly one or two arguments expected");
    }

    gtk_text_buffer_select_range(buf, &start, &end);

    return Variant();
}

Variant Document::select_lines(const VariantArray &args)
{
    Index first_line, last_line;

    if (args.size() == 1)
    {
        first_line = last_line = get_index(args[0]);
    }
    else if (args.size() == 2)
    {
        first_line = get_index(args[0]);
        last_line = get_index(args[1]);
    }
    else
    {
        Error::raise("exactly one or two arguments expected");
    }

    if (first_line.get() > last_line.get())
        std::swap(first_line, last_line);

    GtkTextBuffer *buf = buffer();

    if (first_line.get() < 0 || first_line.get() >= gtk_text_buffer_get_line_count(buf))
        Error::raise("invalid line");
    if (last_line.get() < 0 || last_line.get() >= gtk_text_buffer_get_line_count(buf))
        Error::raise("invalid line");

    GtkTextIter start, end;
    gtk_text_buffer_get_iter_at_line(buf, &start, first_line.get());
    gtk_text_buffer_get_iter_at_line(buf, &end, last_line.get());
    gtk_text_iter_forward_line(&end);
    gtk_text_buffer_select_range(buf, &start, &end);

    return Variant();
}

static void get_select_lines_range(const VariantArray &args, GtkTextBuffer *buf, GtkTextIter *start, GtkTextIter *end)
{
    if (args.size() == 1)
    {
        get_iter(args[0], buf, start);
        *end = *start;
    }
    else if (args.size() == 2)
    {
        get_iter(args[0], buf, start);
        get_iter(args[1], buf, end);
    }
    else
    {
        Error::raise("exactly one or two arguments expected");
    }

    gtk_text_iter_order(start, end);
    gtk_text_iter_forward_line(end);
}

Variant Document::select_lines_at_pos(const VariantArray &args)
{
    GtkTextBuffer *buf = buffer();
    GtkTextIter start, end;
    get_select_lines_range(args, buf, &start, &end);
    gtk_text_buffer_select_range(buf, &start, &end);
    return Variant();
}

Variant Document::select_all(const VariantArray &args)
{
    check_no_args(args);
    GtkTextBuffer *buf = buffer();
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(buf, &start, &end);
    gtk_text_buffer_select_range(buf, &start, &end);
    return Variant();
}

Variant Document::get_selected_text()
{
    GtkTextBuffer *buf = buffer();
    GtkTextIter start, end;
    gtk_text_buffer_get_selection_bounds(buf, &start, &end);
    return String::take_utf8(gtk_text_buffer_get_slice(buf, &start, &end, TRUE));
}

static void get_selected_lines_bounds(GtkTextBuffer *buf, GtkTextIter *start, GtkTextIter *end)
{
    gtk_text_buffer_get_selection_bounds(buf, start, end);
    if (!gtk_text_iter_starts_line(end) || (gtk_text_iter_equal(start, end) && !gtk_text_iter_ends_line(start)))
        gtk_text_iter_forward_line(end);
    gtk_text_iter_set_line_offset(start, 0);
}

Variant Document::get_selected_lines()
{
    GtkTextIter start, end;
    GtkTextBuffer *buf = buffer();
    get_selected_lines_bounds(buf, &start, &end);
    return String::take_utf8(gtk_text_buffer_get_slice(buf, &start, &end, TRUE));
}

Variant Document::delete_selected_text(const VariantArray &args)
{
    check_no_args(args);
    GtkTextBuffer *buf = buffer();
    GtkTextIter start, end;
    gtk_text_buffer_get_selection_bounds(buf, &start, &end);
    gtk_text_buffer_delete(buf, &start, &end);
    return Variant();
}

Variant Document::delete_selected_lines(const VariantArray &args)
{
    check_no_args(args);
    GtkTextIter start, end;
    GtkTextBuffer *buf = buffer();
    get_selected_lines_bounds(buf, &start, &end);
    gtk_text_buffer_delete(buf, &start, &end);
    return Variant();
}

Variant Document::replace_selected_text(const VariantArray &args)
{
    check_1_arg(args);
    String text = get_string(args[0]);
    GtkTextBuffer *buf = buffer();
    GtkTextIter start, end;
    gtk_text_buffer_get_selection_bounds(buf, &start, &end);
    gtk_text_buffer_delete(buf, &start, &end);
    gtk_text_buffer_insert(buf, &start, text.utf8(), -1);
    return Variant();
}

Variant Document::replace_selected_lines(const VariantArray &args)
{
    check_1_arg(args);
    String text = get_string(args[0]);
    GtkTextBuffer *buf = buffer();
    GtkTextIter start, end;
    get_selected_lines_bounds(buf, &start, &end);
    gtk_text_buffer_delete(buf, &start, &end);
    gtk_text_buffer_insert(buf, &start, text.utf8(), -1);
    return Variant();
}


} // namespace mom
