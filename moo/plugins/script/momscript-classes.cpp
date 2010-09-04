#include "momscript-classes.h"
#include "mooapp/mooapp.h"
#include "mooutils/mooutils-misc.h"
#include <string.h>

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

static String get_string(const Variant &val, bool null_ok = false)
{
    if (null_ok && val.vt() == VtVoid)
        return String();

    if (val.vt() != VtString)
        Error::raise("string expected");

    return val.value<VtString>();
}

static moo::Vector<String> get_string_list(const Variant &val)
{
    if (val.vt() != VtArray)
        Error::raise("list expected");
    const VariantArray &ar = val.value<VtArray>();
    moo::Vector<String> ret;
    for (int i = 0, c = ar.size(); i < c; ++i)
        ret.append(get_string(ar[i]));
    return ret;
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
// Application
//

Variant Application::editor(const VariantArray &args)
{
    check_no_args(args);
    return HObject(Editor::get_instance());
}

Variant Application::active_window(const VariantArray &args)
{
    return Editor::get_instance().active_window(args);
}

Variant Application::set_active_window(const VariantArray &args)
{
    return Editor::get_instance().set_active_window(args);
}

Variant Application::active_document(const VariantArray &args)
{
    return Editor::get_instance().active_document(args);
}

Variant Application::active_view(const VariantArray &args)
{
    return Editor::get_instance().active_view(args);
}

Variant Application::windows(const VariantArray &args)
{
    return Editor::get_instance().windows(args);
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

Variant Editor::active_document(const VariantArray &args)
{
    check_no_args(args);
    return HObject(Document::wrap(moo_editor_get_active_doc(moo_editor_instance())));
}

Variant Editor::set_active_document(const VariantArray &args)
{
    check_1_arg(args);
    moo::SharedPtr<Document> doc = get_object<Document>(args[0]);
    moo_editor_set_active_doc(moo_editor_instance(), doc->gobj());
    return Variant();
}

Variant Editor::active_window(const VariantArray &args)
{
    check_no_args(args);
    return HObject(DocumentWindow::wrap(moo_editor_get_active_window(moo_editor_instance())));
}

Variant Editor::set_active_window(const VariantArray &args)
{
    check_1_arg(args);
    moo::SharedPtr<DocumentWindow> window = get_object<DocumentWindow>(args[0]);
    moo_editor_set_active_window(moo_editor_instance(), window->gobj());
    return Variant();
}

Variant Editor::active_view(const VariantArray &args)
{
    check_no_args(args);
    return HObject(DocumentView::wrap(moo_editor_get_active_doc(moo_editor_instance())));
}

Variant Editor::set_active_view(const VariantArray &args)
{
    check_1_arg(args);
    moo::SharedPtr<DocumentView> view = get_object<DocumentView>(args[0]);
    moo_editor_set_active_doc(moo_editor_instance(), view->gobj());
    return Variant();
}

Variant Editor::documents(const VariantArray &args)
{
    check_no_args(args);
    GSList *docs = moo_editor_list_docs(moo_editor_instance());
    return wrap_gslist<MooEdit, Document>(docs);
}

Variant Editor::views(const VariantArray &args)
{
    check_no_args(args);
    GSList *docs = moo_editor_list_docs(moo_editor_instance());
    return wrap_gslist<MooEdit, DocumentView>(docs);
}

Variant Editor::windows(const VariantArray &args)
{
    check_no_args(args);
    GSList *windows = moo_editor_list_windows(moo_editor_instance());
    return wrap_gslist<MooEditWindow, DocumentWindow>(windows);
}

Variant Editor::get_document_by_path(const VariantArray &args)
{
    check_1_arg(args);
    String path = get_string(args[0]);
    MooEdit *doc = moo_editor_get_doc(moo_editor_instance(), path);
    return doc ? HObject(Document::wrap(doc)) : HObject();
}

Variant Editor::get_document_by_uri(const VariantArray &args)
{
    check_1_arg(args);
    String uri = get_string(args[0]);
    MooEdit *doc = moo_editor_get_doc_for_uri(moo_editor_instance(), uri);
    return doc ? HObject(Document::wrap(doc)) : HObject();
}

static GSList *get_file_info_list(const Variant &val, bool uri)
{
    moo::Vector<String> filenames = get_string_list(val);

    if (filenames.empty())
        return NULL;

    GSList *files = NULL;

    for (int i = 0, c = filenames.size(); i < c; ++i)
    {
        MooEditFileInfo *fi = uri ?
                moo_edit_file_info_new_uri(filenames[i], NULL) :
                moo_edit_file_info_new_path(filenames[i], NULL);
        if (!fi)
            goto error;
        files = g_slist_prepend(files, fi);
    }

    return g_slist_reverse(files);

error:
    g_slist_foreach(files, (GFunc) moo_edit_file_info_free, NULL);
    g_slist_free(files);
    Error::raise("error");
}

static Variant open_files_or_uris(const VariantArray &args, bool uri)
{
    if (args.size() == 0 || args.size() > 2)
        Error::raise("expected one or two arguments");

    moo::SharedPtr<DocumentWindow> window;
    if (args.size() >= 2)
        window = get_object<DocumentWindow>(args[1], true);

    GSList *files = get_file_info_list(args[0], uri);
    moo_editor_open(moo_editor_instance(), window ? window->gobj() : NULL, NULL, files);
    g_slist_foreach(files, (GFunc) moo_edit_file_info_free, NULL);
    g_slist_free(files);

    return Variant();
}

Variant Editor::open_files(const VariantArray &args)
{
    return open_files_or_uris(args, false);
}

Variant Editor::open_uris(const VariantArray &args)
{
    return open_files_or_uris(args, true);
}

static Variant open_file_or_uri(const VariantArray &args, bool uri, bool new_file)
{
    if (args.size() == 0)
        Error::raise("at least one argument expected");

    String file = get_string(args[0]);

    String encoding;
    if (args.size() > 1)
        encoding = get_string(args[1], true);

    moo::SharedPtr<DocumentWindow> window;
    if (args.size() > 2)
        window = get_object<DocumentWindow>(args[2], true);

    MooEdit *doc = NULL;

    if (new_file)
    {
        if (uri)
        {
            moo_assert_not_reached();
            Error::raise("error");
        }
        else
        {
            doc = moo_editor_new_file(moo_editor_instance(), window ? window->gobj() : NULL, NULL, file, encoding);
        }
    }
    else
    {
        if (uri)
            doc = moo_editor_open_uri(moo_editor_instance(), window ? window->gobj() : NULL, NULL, file, encoding);
        else
            doc = moo_editor_open_file(moo_editor_instance(), window ? window->gobj() : NULL, NULL, file, encoding);
    }

    return doc ? HObject(Document::wrap(doc)) : HObject();
}

Variant Editor::open_file(const VariantArray &args)
{
    return open_file_or_uri(args, false, false);
}

Variant Editor::open_uri(const VariantArray &args)
{
    return open_file_or_uri(args, true, false);
}

Variant Editor::new_file(const VariantArray &args)
{
    return open_file_or_uri(args, false, true);
}

Variant Editor::reload(const VariantArray &args)
{
    check_1_arg(args);
    moo::SharedPtr<Document> doc = get_object<Document>(args[0]);
    moo_edit_reload(doc->gobj(), NULL, NULL);
    return Variant();
}

Variant Editor::save(const VariantArray &args)
{
    check_1_arg(args);
    moo::SharedPtr<Document> doc = get_object<Document>(args[0]);
    return bool(moo_edit_save(doc->gobj(), NULL));
}

Variant Editor::save_as(const VariantArray &args)
{
    if (args.size() == 0)
        Error::raise("at least one argument expected");

    moo::SharedPtr<Document> doc = get_object<Document>(args[0]);

    String filename;
    if (args.size() > 0)
        filename = get_string(args[1], true);

    return bool(moo_edit_save_as(doc->gobj(),
                                 filename.empty() ? NULL : (const char*) filename,
                                 NULL,
                                 NULL));
}

Variant Editor::close(const VariantArray &args)
{
    check_1_arg(args);
    return bool(moo_edit_close(get_object<Document>(args[0])->gobj(), TRUE));
}


///////////////////////////////////////////////////////////////////////////////
//
// DocumentWindow
//

Variant DocumentWindow::editor(const VariantArray &args)
{
    check_no_args(args);
    return HObject(Editor::get_instance());
}

Variant DocumentWindow::active_view(const VariantArray &args)
{
    check_no_args(args);
    return HObject(DocumentView::wrap(moo_edit_window_get_active_doc(gobj())));
}

Variant DocumentWindow::set_active_view(const VariantArray &args)
{
    check_1_arg(args);
    moo::SharedPtr<DocumentView> view = get_object<DocumentView>(args[0]);
    moo_edit_window_set_active_doc(gobj(), view->gobj());
    return Variant();
}

Variant DocumentWindow::active_document(const VariantArray &args)
{
    check_no_args(args);
    return HObject(Document::wrap(moo_edit_window_get_active_doc(gobj())));
}

Variant DocumentWindow::set_active_document(const VariantArray &args)
{
    check_1_arg(args);
    moo::SharedPtr<Document> doc = get_object<Document>(args[0]);
    moo_edit_window_set_active_doc(gobj(), doc->gobj());
    return Variant();
}

Variant DocumentWindow::views(const VariantArray &args)
{
    check_no_args(args);
    GSList *docs = moo_edit_window_list_docs(gobj());
    return wrap_gslist<MooEdit, DocumentView>(docs);
}

Variant DocumentWindow::documents(const VariantArray &args)
{
    check_no_args(args);
    GSList *docs = moo_edit_window_list_docs(gobj());
    return wrap_gslist<MooEdit, Document>(docs);
}

Variant DocumentWindow::is_active(const VariantArray &args)
{
    check_no_args(args);
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

Variant DocumentView::document(const VariantArray &args)
{
    check_no_args(args);
    return HObject(Document::wrap(gobj()));
}

Variant DocumentView::window(const VariantArray &args)
{
    check_no_args(args);
    return HObject(DocumentWindow::wrap(moo_edit_get_window(gobj())));
}

Variant DocumentView::line_wrap_mode(const VariantArray &args)
{
    check_no_args(args);
    GtkWrapMode mode;
    g_object_get(gobj(), "wrap-mode", &mode, (char*) NULL);
    return mode != GTK_WRAP_NONE;
}

Variant DocumentView::set_line_wrap_mode(const VariantArray &args)
{
    check_1_arg(args);
    moo_edit_ui_set_line_wrap_mode(gobj(), get_bool(args[0]));
    return Variant();
}

Variant DocumentView::overwrite_mode(const VariantArray &args)
{
    check_no_args(args);
    gboolean overwrite;
    g_object_get(gobj(), "overwrite", &overwrite, (char*) NULL);
    return bool(overwrite);
}

Variant DocumentView::set_overwrite_mode(const VariantArray &args)
{
    check_1_arg(args);
    g_object_set(gobj(), "overwrite", gboolean(get_bool(args[0])), (char*) NULL);
    return Variant();
}

Variant DocumentView::show_line_numbers(const VariantArray &args)
{
    check_no_args(args);
    gboolean show;
    g_object_get(gobj(), "show-line-numbers", &show, (char*) NULL);
    return bool(show);
}

Variant DocumentView::set_show_line_numbers(const VariantArray &args)
{
    check_1_arg(args);
    moo_edit_ui_set_show_line_numbers(gobj(), get_bool(args[0]));
    return Variant();
}


///////////////////////////////////////////////////////////////////////////////
//
// Document
//

Variant Document::active_view(const VariantArray &args)
{
    check_no_args(args);
    return HObject(DocumentView::wrap(gobj()));
}

Variant Document::views(const VariantArray &args)
{
    check_no_args(args);
    VariantArray views;
    views.append(HObject(DocumentView::wrap(gobj())));
    return views;
}

Variant Document::filename(const VariantArray &args)
{
    check_no_args(args);
    char *filename = moo_edit_get_utf8_filename(gobj());
    return filename ? String::take_utf8(filename) : String::Null();
}

Variant Document::uri(const VariantArray &args)
{
    check_no_args(args);
    char *uri = moo_edit_get_uri(gobj());
    return uri ? String::take_utf8(uri) : String::Null();
}

Variant Document::basename(const VariantArray &args)
{
    check_no_args(args);
    return String(moo_edit_get_display_basename(gobj()));
}

Variant Document::reload(const VariantArray &args)
{
    VariantArray editor_args;
    editor_args.append(HObject(*this));
    editor_args.append(args);
    return Editor::get_instance().reload(editor_args);
}

Variant Document::save(const VariantArray &args)
{
    VariantArray editor_args;
    editor_args.append(HObject(*this));
    editor_args.append(args);
    return Editor::get_instance().save(editor_args);
}

Variant Document::save_as(const VariantArray &args)
{
    VariantArray editor_args;
    editor_args.append(HObject(*this));
    editor_args.append(args);
    return Editor::get_instance().save_as(editor_args);
}

Variant Document::encoding(const VariantArray &args)
{
    check_no_args(args);
    return String(moo_edit_get_encoding(gobj()));
}

Variant Document::set_encoding(const VariantArray &args)
{
    check_1_arg(args);
    String enc = get_string(args);
    moo_edit_set_encoding(gobj(), enc);
    return Variant();
}

Variant Document::line_endings(const VariantArray &args)
{
    check_no_args(args);

    switch (moo_edit_get_line_end_type(gobj()))
    {
        case MOO_LE_UNIX:
            return String("unix");
        case MOO_LE_WIN32:
            return String("win32");
        case MOO_LE_MAC:
            return String("mac");
        case MOO_LE_MIX:
            return String("mix");
        case MOO_LE_NONE:
            return String("none");
    }

    moo_assert_not_reached();
    Error::raise("error");
}

Variant Document::set_line_endings(const VariantArray &args)
{
    check_1_arg(args);

    MooLineEndType le = MOO_LE_NONE;
    String str_le = get_string(args[0]);

    if (str_le == "unix")
        le = MOO_LE_UNIX;
    else if (str_le == "win32")
        le = MOO_LE_WIN32;
    else if (str_le == "mac")
        le = MOO_LE_MAC;
    else if (str_le == "mix")
        le = MOO_LE_MIX;
    else if (str_le == "none")
        le = MOO_LE_NONE;
    else
        Error::raise("invalid line ending type");

    moo_edit_set_line_end_type(gobj(), le);

    return Variant();
}

Variant Document::can_undo(const VariantArray &args)
{
    check_no_args(args);
    return moo_text_view_can_undo(MOO_TEXT_VIEW(gobj()));
}

Variant Document::can_redo(const VariantArray &args)
{
    check_no_args(args);
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

Variant Document::start_pos(const VariantArray &args)
{
    check_no_args(args);
    return Index(0);
}

Variant Document::end_pos(const VariantArray &args)
{
    check_no_args(args);
    return Index(gtk_text_buffer_get_char_count(buffer()));
}

Variant Document::cursor_pos(const VariantArray &args)
{
    check_no_args(args);
    GtkTextBuffer *buf = buffer();
    GtkTextIter iter;
    gtk_text_buffer_get_iter_at_mark(buf, &iter, gtk_text_buffer_get_insert(buf));
    return Index(gtk_text_iter_get_offset(&iter));
}

Variant Document::set_cursor_pos(const VariantArray &args)
{
    check_1_arg(args);
    GtkTextBuffer *buf = buffer();
    GtkTextIter iter;
    get_iter(args[0], buf, &iter);
    gtk_text_buffer_place_cursor(buf, &iter);
    return Variant();
}

Variant Document::selection_bound(const VariantArray &args)
{
    check_no_args(args);
    GtkTextBuffer *buf = buffer();
    GtkTextIter iter;
    gtk_text_buffer_get_iter_at_mark(buf, &iter, gtk_text_buffer_get_selection_bound(buf));
    return Index(gtk_text_iter_get_offset(&iter));
}

Variant Document::selection(const VariantArray &args)
{
    check_no_args(args);
    GtkTextBuffer *buf = buffer();
    GtkTextIter start, end;
    gtk_text_buffer_get_selection_bounds(buf, &start, &end);
    return make_pair(Index(gtk_text_iter_get_offset(&start)),
                     Index(gtk_text_iter_get_offset(&end)));
}

Variant Document::set_selection(const VariantArray &args)
{
    GtkTextBuffer *buf = buffer();
    GtkTextIter start, end;

    if (args.size() == 1)
    {
        get_iter_pair(args[0], buf, &start, &end);
    }
    else if (args.size() == 2)
    {
        get_iter(args[0], buf, &start);
        get_iter(args[1], buf, &end);
    }
    else
    {
        Error::raise("exactly one or two arguments expected");
    }

    gtk_text_buffer_select_range(buf, &start, &end);
    return Variant();
}

Variant Document::has_selection(const VariantArray &args)
{
    check_no_args(args);
    GtkTextBuffer *buf = buffer();
    GtkTextIter start, end;
    gtk_text_buffer_get_selection_bounds(buf, &start, &end);
    return bool(gtk_text_iter_equal(&start, &end));
}

Variant Document::char_count(const VariantArray &args)
{
    check_no_args(args);
    return gtk_text_buffer_get_char_count(buffer());
}

Variant Document::line_count(const VariantArray &args)
{
    check_no_args(args);
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

Variant Document::selected_text(const VariantArray &args)
{
    check_no_args(args);
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

Variant Document::selected_lines(const VariantArray &args)
{
    check_no_args(args);
    GtkTextIter start, end;
    GtkTextBuffer *buf = buffer();
    get_selected_lines_bounds(buf, &start, &end);
    char *text = gtk_text_buffer_get_slice(buf, &start, &end, TRUE);
    char **lines = moo_splitlines(text);
    if (text && text[0] && lines && *lines && text[strlen(text) - 1] == '\n')
    {
        int n_lines = g_strv_length(lines);
        g_free(lines[n_lines - 1]);
        lines[n_lines - 1] = NULL;
    }
    VariantArray ar;
    for (char **p = lines; p && *p; ++p)
        ar.append(String::take_utf8(*p));
    g_free(lines);
    g_free(text);
    return ar;
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
