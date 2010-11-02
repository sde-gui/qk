#include "mooscript-classes.h"
#include "mooscript-classes-util.h"
#include "mooapp/mooapp.h"
#include "mooutils/mooutils-misc.h"
#include <string.h>
#include <sstream>

namespace mom {

// static void check_no_args(const VariantArray &args)
// {
//     if (args.size() != 0)
//         Error::raise("no arguments expected");
// }
//
// static void check_1_arg(const VariantArray &args)
// {
//     if (args.size() != 1)
//         Error::raise("exactly one argument expected");
// }
//
// template<typename T>
// static moo::SharedPtr<T> get_object(const Variant &val, bool null_ok = false)
// {
//     if (null_ok && val.vt() == VtVoid)
//         return moo::SharedPtr<T>();
//
//     if (val.vt() != VtObject)
//         Error::raise("object expected");
//
//     HObject h = val.value<VtObject>();
//     if (null_ok && h.id() == 0)
//         return moo::SharedPtr<T>();
//
//     moo::SharedPtr<Object> obj = Object::lookup_object(h);
//
//     if (!obj)
//         Error::raise("bad object");
//     if (&obj->meta() != &T::class_meta())
//         Error::raise("bad object");
//
//     return moo::SharedPtr<T>(static_cast<T*>(obj.get()));
// }
//
// static bool get_bool(const Variant &val)
// {
//     if (val.vt() != VtBool)
//         Error::raise("boolean expected");
//     return val.value<VtBool>();
// }

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
    VariantArray ar;
    if (val.vt() == VtArray)
        ar = val.value<VtArray>();
    else if (val.vt() == VtArgList)
        ar = val.value<VtArgList>();
    else
        Error::raise("list expected");
    moo::Vector<String> ret;
    for (int i = 0, c = ar.size(); i < c; ++i)
        ret.append(get_string(ar[i]));
    return ret;
}

template<typename GClass, typename Class>
static VariantArray wrap_gslist(GSList *list)
{
    VariantArray array;
    while (list)
    {
        GClass *gobj = (GClass*) list->data;
        array.append(HObject(gobj ? Class::wrap(gobj)->id() : 0));
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

static void get_iter(gint64 pos, GtkTextBuffer *buf, GtkTextIter *iter)
{
    if (pos > gtk_text_buffer_get_char_count(buf) || pos < 0)
        Error::raise("invalid offset");
    gtk_text_buffer_get_iter_at_offset(buf, iter, pos);
}

static void get_iter(const Variant &val, GtkTextBuffer *buf, GtkTextIter *iter)
{
    get_iter(get_index(val).get(), buf, iter);
}

static void get_pair(const Variant &val, Variant &elm1, Variant &elm2)
{
    VariantArray ar;
    if (val.vt() == VtArray)
        ar = val.value<VtArray>();
    else if (val.vt() == VtArgList)
        ar = val.value<VtArgList>();
    else
        Error::raise("pair of values expected");
    if (ar.size() != 2)
        Error::raise("pair of values expected");
    elm1 = ar[0];
    elm2 = ar[1];
}

static VariantArray make_pair(const Variant &elm1, const Variant &elm2)
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

///
/// @node Application object
/// @section Application object
/// @helpsection{SCRIPT_APPLICATION}
/// @table @method
///

/// @item Application.editor()
/// returns Editor object.
Editor *Application::editor()
{
    return &Editor::get_instance();
}

/// @item Application.quit()
/// quit @medit{}.
void Application::quit()
{
    moo_app_quit(moo_app_get_instance());
}

struct DialogOptions
{
    String title;
    String dialog_id;
    String icon;
    String buttons;
    gint64 width;
    gint64 height;
    gint64 timeout;
};

struct FileDialogOptions
{
    String filename;
    bool multiple;
    bool directory;
    bool save;
};

struct ListDialogOptions
{
    moo::Vector<String> columns;
    moo::Vector<String> data;
    bool checklist;
    bool radiolist;
    bool editable;
    bool show_header;
    String return_column;
};

struct MessageDialogOptions
{
    String kind;
    String text;
};

struct EntryDialogOptions
{
    String text;
    String entry_text;
    bool hide;
};

struct TextDialogOptions
{
    String text;
    String info_text;
    String filename;
    bool editable;
};

static Variant show_file_dialog(G_GNUC_UNUSED const FileDialogOptions &dopts, G_GNUC_UNUSED const DialogOptions &opts)
{
    Error::raise("not implemented");
}

static Variant show_list_dialog(G_GNUC_UNUSED const ListDialogOptions &dopts, G_GNUC_UNUSED const DialogOptions &opts)
{
    Error::raise("not implemented");
}

static Variant show_message_dialog(G_GNUC_UNUSED const MessageDialogOptions &dopts, G_GNUC_UNUSED const DialogOptions &opts)
{
    Error::raise("not implemented");
}

static Variant show_entry_dialog(G_GNUC_UNUSED const EntryDialogOptions &dopts, G_GNUC_UNUSED const DialogOptions &opts)
{
    Error::raise("not implemented");
}

static Variant show_text_dialog(G_GNUC_UNUSED const TextDialogOptions &dopts, G_GNUC_UNUSED const DialogOptions &opts)
{
    Error::raise("not implemented");
}

/// @item Application.dialog()
/// show a dialog.
Variant Application::dialog(const ArgSet &args)
{
    if (!args.pos.empty())
        Error::raisef("in function %s, no positional arguments expected",
                          (const char*) current_func().name);

    DialogOptions opts;
    opts.title = get_kwarg_string_opt(args, "title");
    opts.dialog_id = get_kwarg_string_opt(args, "id");
    opts.icon = get_kwarg_string_opt(args, "icon");
    opts.buttons = get_kwarg_string_opt(args, "buttons");
    opts.width = get_kwarg_int_opt(args, "width", -1);
    opts.height = get_kwarg_int_opt(args, "height", -1);
    opts.timeout = get_kwarg_int_opt(args, "timeout", -1);

    String kind = get_kwarg_string(args, "kind");

    if (kind == "file" || kind == "file-selection")
    {
        FileDialogOptions dopts;
        dopts.filename = get_kwarg_string_opt(args, "filename");
        dopts.multiple = get_kwarg_bool_opt(args, "multiple");
        dopts.directory = get_kwarg_bool_opt(args, "directory");
        dopts.save = get_kwarg_bool_opt(args, "save");
        return show_file_dialog(dopts, opts);
    }

    if (kind == "list")
    {
        ListDialogOptions dopts;
        dopts.columns = get_string_list(args.kw.value("column"));
        dopts.data = get_string_list(args.kw.value("data"));
        dopts.checklist = get_kwarg_bool_opt(args, "checklist");
        dopts.radiolist = get_kwarg_bool_opt(args, "radiolist");
        dopts.editable = get_kwarg_bool_opt(args, "editable");
        dopts.show_header = get_kwarg_bool_opt(args, "show_header");
        dopts.return_column = get_kwarg_string_opt(args, "return_column");
        return show_list_dialog(dopts, opts);
    }

    if (kind == "error" || kind == "warning" || kind == "question" || kind == "information")
    {
        MessageDialogOptions dopts;
        dopts.kind = kind;
        dopts.text = get_kwarg_string(args, "text");
        return show_message_dialog(dopts, opts);
    }

    if (kind == "entry" || kind == "text-entry")
    {
        EntryDialogOptions dopts;
        dopts.text = get_kwarg_string_opt(args, "text");
        dopts.entry_text = get_kwarg_string_opt(args, "entry_text");
        dopts.hide = get_kwarg_bool_opt(args, "hide");
        return show_entry_dialog(dopts, opts);
    }

    if (kind == "text" || kind == "text-info")
    {
        TextDialogOptions dopts;
        dopts.text = get_kwarg_string_opt(args, "text");
        dopts.info_text = get_kwarg_string_opt(args, "info_text");
        dopts.filename = get_kwarg_string_opt(args, "filename");
        dopts.editable = get_kwarg_bool_opt(args, "editable");
        return show_text_dialog(dopts, opts);
    }

    Error::raisef("in function %s, invalid dialog kind '%s'",
                      (const char*) current_func().name, (const char*) kind);
}

///
/// @end table
///

///////////////////////////////////////////////////////////////////////////////
//
// Editor
//

///
/// @node Editor object
/// @section Editor object
/// @helpsection{SCRIPT_EDITOR}
/// @table @method
///

/// @item Editor.active_document()
/// returns current active document or @null{} if there are no open documents
Document *Editor::active_document()
{
    return Document::wrap(moo_editor_get_active_doc(moo_editor_instance()));
}

/// @item Editor.set_active_document(doc)
/// makes @param{doc} active
void Editor::set_active_document(Document &doc)
{
    moo_editor_set_active_doc(moo_editor_instance(), doc.gobj());
}

/// @item Editor.active_window()
/// returns current active window
DocumentWindow *Editor::active_window()
{
    return DocumentWindow::wrap(moo_editor_get_active_window(moo_editor_instance()));
}

/// @item Editor.set_active_window(window)
/// makes @param{window} active
void Editor::set_active_window(DocumentWindow &window)
{
    moo_editor_set_active_window(moo_editor_instance(), window.gobj());
}

/// @item Editor.active_view()
/// returns current active document view
DocumentView *Editor::active_view()
{
    return DocumentView::wrap(moo_editor_get_active_doc(moo_editor_instance()));
}

/// @item Editor.set_active_view(view)
/// makes @param{view} active
void Editor::set_active_view(DocumentView &view)
{
    moo_editor_set_active_doc(moo_editor_instance(), view.gobj());
}

/// @item Editor.documents()
/// returns list of all open documents
VariantArray Editor::documents()
{
    GSList *docs = moo_editor_list_docs(moo_editor_instance());
    return wrap_gslist<MooEdit, Document>(docs);
}

/// @item Editor.views()
/// returns list of all open document views
VariantArray Editor::views()
{
    GSList *docs = moo_editor_list_docs(moo_editor_instance());
    return wrap_gslist<MooEdit, DocumentView>(docs);
}

/// @item Editor.windows()
/// returns list of all document windows
VariantArray Editor::windows()
{
    GSList *windows = moo_editor_list_windows(moo_editor_instance());
    return wrap_gslist<MooEditWindow, DocumentWindow>(windows);
}

/// @item Editor.new_document(window=null)
/// creates new document in specified window. If window is @null{} then
/// the document is created in an existing window
Document *Editor::new_document(DocumentWindow *window)
{
    return Document::wrap(moo_editor_new_doc(moo_editor_instance(), window ? window->gobj() : NULL));
}

/// @item Editor.new_window()
/// creates new document window
DocumentWindow *Editor::new_window()
{
    return DocumentWindow::wrap(moo_editor_new_window(moo_editor_instance()));
}

/// @item Editor.get_document_by_path(path)
/// returns document with path @param{path} or @null{}.
Document *Editor::get_document_by_path(const String &path)
{
    return Document::wrap(moo_editor_get_doc(moo_editor_instance(), path));
}

/// @item Editor.get_document_by_uri(path)
/// returns document with uri @param{uri} or @null{}.
Document *Editor::get_document_by_uri(const String &uri)
{
    return Document::wrap(moo_editor_get_doc_for_uri(moo_editor_instance(), uri));
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

static void open_files_or_uris(const VariantArray &file_array, DocumentWindow *window, bool uri)
{
    GSList *files = get_file_info_list(file_array, uri);
    moo_editor_open(moo_editor_instance(), window ? window->gobj() : NULL, NULL, files);
    g_slist_foreach(files, (GFunc) moo_edit_file_info_free, NULL);
    g_slist_free(files);
}

/// @item Editor.open_files(files, window=null)
/// open files. If @param{window} is given then open files in that window,
/// otherwise in an existing window.
void Editor::open_files(const VariantArray &files, DocumentWindow *window)
{
    open_files_or_uris(files, window, false);
}

/// @item Editor.open_uris(uris, window=null)
/// open files. If @param{window} is given then open files in that window,
/// otherwise in an existing window.
void Editor::open_uris(const VariantArray &uris, DocumentWindow *window)
{
    open_files_or_uris(uris, window, true);
}

static Document *open_file_or_uri(const String &file, const String &encoding, DocumentWindow *window, bool uri, bool new_file)
{
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

    return Document::wrap(doc);
}

/// @item Editor.open_file(file, encoding=null, window=null)
/// open file. If @param{encoding} is @null{} or "auto" then pick character
/// encoding automatically, otherwise use @param{encoding}. If @param{window}
/// is given then open files in that window, otherwise in an existing window.
Document *Editor::open_file(const String &file, const String &encoding, DocumentWindow *window)
{
    return open_file_or_uri(file, encoding, window, false, false);
}

/// @item Editor.open_uri(uri, encoding=null, window=null)
/// open file. If @param{encoding} is @null{} or "auto" then pick character
/// encoding automatically, otherwise use @param{encoding}. If @param{window}
/// is given then open files in that window, otherwise in an existing window.
Document *Editor::open_uri(const String &uri, const String &encoding, DocumentWindow *window)
{
    return open_file_or_uri(uri, encoding, window, true, false);
}

/// @item Editor.new_file(file, encoding=null, window=null)
/// open file if it exists on disk or create a new one. If @param{encoding} is
/// @null{} or "auto" then pick character encoding automatically, otherwise use
/// @param{encoding}. If @param{window} is given then open file in that window,
/// otherwise in an existing window.
Document *Editor::new_file(const String &file, const String &encoding, DocumentWindow *window)
{
    return open_file_or_uri(file, encoding, window, false, true);
}

/// @item Editor.reload(doc)
/// reload document.
void Editor::reload(Document &doc)
{
    moo_edit_reload(doc.gobj(), NULL, NULL);
}

/// @item Editor.save(doc)
/// save document.
bool Editor::save(Document &doc)
{
    return moo_edit_save(doc.gobj(), NULL);
}

/// @item Editor.save_as(doc, filename=null)
/// save document as @param{filename}. If @param{filename} is not given then
/// first ask user for new filename.
bool Editor::save_as(Document &doc, const String &filename)
{
    return moo_edit_save_as(doc.gobj(),
                            filename.empty() ? NULL : (const char*) filename,
                            NULL,
                            NULL);
}

/// @item Editor.close(doc)
/// close document.
bool Editor::close(Document &doc)
{
    return moo_edit_close(doc.gobj(), TRUE);
}

/// @item Editor.close_documents(docs)
/// close all documents in the list @param{docs}.
bool Editor::close_documents(const VariantArray &arr)
{
    moo::Vector<MooEdit*> pvec;

    for (int i = 0, c = arr.size(); i < c; ++i)
        pvec.append(get_object_arg<Document>(arr[i], "doc").gobj());

    GSList *docs = NULL;
    for (int i = 0, c = pvec.size(); i < c; ++i)
        docs = g_slist_prepend (docs, pvec[i]);
    docs = g_slist_reverse (docs);

    bool retval = moo_editor_close_docs (moo_editor_instance(), docs, TRUE);
    g_slist_free (docs);
    return retval;
}

/// @item Editor.close_window(window)
/// close window.
bool Editor::close_window(DocumentWindow &window)
{
    return moo_editor_close_window(moo_editor_instance(), window.gobj(), TRUE);
}

///
/// @end table
///

///////////////////////////////////////////////////////////////////////////////
//
// DocumentWindow
//

///
/// @node DocumentWindow object
/// @section DocumentWindow object
/// @helpsection{SCRIPT_DOCUMENT_WINDOW}
/// @table @method
///

/// @item DocumentWindow.editor()
/// returns Editor object.
Editor *DocumentWindow::editor()
{
    return &Editor::get_instance();
}

/// @item DocumentWindow.active_view()
/// returns current active document view in this window.
DocumentView *DocumentWindow::active_view()
{
    return DocumentView::wrap(moo_edit_window_get_active_doc(gobj()));
}

/// @item DocumentWindow.set_active_view(view)
/// makes @param{view} active, i.e. switches to its tab.
void DocumentWindow::set_active_view(DocumentView &view)
{
    moo_edit_window_set_active_doc(gobj(), view.gobj());
}

/// @item DocumentWindow.active_document()
/// returns current active document in this window, that is the document
/// whose view is the active one.
Document *DocumentWindow::active_document()
{
    return Document::wrap(moo_edit_window_get_active_doc(gobj()));
}

/// @item DocumentWindow.set_active_document(doc)
/// makes active a view of document @param{doc}. It picks arbitrary view
/// of @param{doc} if there are more than one in this window.
void DocumentWindow::set_active_document(Document &doc)
{
    moo_edit_window_set_active_doc(gobj(), doc.gobj());
}

/// @item DocumentWindow.views()
/// returns list of all document views in this window.
VariantArray DocumentWindow::views()
{
    GSList *docs = moo_edit_window_list_docs(gobj());
    return wrap_gslist<MooEdit, DocumentView>(docs);
}

/// @item DocumentWindow.documents()
/// returns list of all documents in this window.
VariantArray DocumentWindow::documents()
{
    GSList *docs = moo_edit_window_list_docs(gobj());
    return wrap_gslist<MooEdit, Document>(docs);
}

/// @item DocumentWindow.is_active()
/// returns whether this window is the active one.
bool DocumentWindow::is_active()
{
    return gobj() == moo_editor_get_active_window(moo_editor_instance());
}

/// @item DocumentWindow.set_active()
/// makes this window active.
void DocumentWindow::set_active()
{
    moo_editor_set_active_window(moo_editor_instance(), gobj());
}

///
/// @end table
///

///////////////////////////////////////////////////////////////////////////////
//
// DocumentView
//

///
/// @node DocumentView object
/// @section DocumentView object
/// @helpsection{DOCUMENT_VIEW}
/// @table @method
///

/// @item DocumentView.document()
/// returns document to which this view belongs.
Document *DocumentView::document()
{
    return Document::wrap(gobj());
}

/// @item DocumentView.window()
/// returns window which contains this view.
DocumentWindow *DocumentView::window()
{
    return DocumentWindow::wrap(moo_edit_get_window(gobj()));
}

/// @item DocumentView.line_wrap_mode()
/// returns whether line wrapping is enabled.
bool DocumentView::line_wrap_mode()
{
    GtkWrapMode mode;
    g_object_get(gobj(), "wrap-mode", &mode, (char*) NULL);
    return mode != GTK_WRAP_NONE;
}

/// @item DocumentView.set_line_wrap_mode(enabled)
/// enables or disables line wrapping.
void DocumentView::set_line_wrap_mode(bool wrap)
{
    moo_edit_ui_set_line_wrap_mode(gobj(), wrap);
}

/// @item DocumentView.overwrite_mode()
/// returns whether overwrite mode is on.
bool DocumentView::overwrite_mode()
{
    gboolean overwrite;
    g_object_get(gobj(), "overwrite", &overwrite, (char*) NULL);
    return overwrite;
}

/// @item DocumentView.set_overwrite_mode(enabled)
/// enables or disables overwrite mode.
void DocumentView::set_overwrite_mode(bool overwrite)
{
    g_object_set(gobj(), "overwrite", gboolean(overwrite), (char*) NULL);
}

/// @item DocumentView.show_line_numbers()
/// returns whether line numbers are displayed.
bool DocumentView::show_line_numbers()
{
    gboolean show;
    g_object_get(gobj(), "show-line-numbers", &show, (char*) NULL);
    return show;
}

/// @item DocumentView.set_show_line_numbers(show)
/// shows or hides line numbers.
void DocumentView::set_show_line_numbers(bool show)
{
    moo_edit_ui_set_show_line_numbers(gobj(), show);
}

///
/// @end table
///

///////////////////////////////////////////////////////////////////////////////
//
// Document
//

///
/// @node Document object
/// @section Document object
/// @helpsection{DOCUMENT}
///

/// @table @method

static GtkTextBuffer *buffer(Document *doc)
{
    return gtk_text_view_get_buffer(GTK_TEXT_VIEW(doc->gobj()));
}

/// @item Document.views()
/// returns list of views which display this document.
VariantArray Document::views()
{
    VariantArray views;
    views.append(HObject(*DocumentView::wrap(gobj())));
    return views;
}

/// @item Document.active_view()
/// returns active view of this document. If the document has a single
/// view, then that is returned; otherwise if the current active view
/// belongs to this document, then that view is returned; otherwise
/// a random view is picked.
DocumentView *Document::active_view()
{
    return DocumentView::wrap(gobj());
}

/// @item Document.filename()
/// returns full file path of the document or @null{} if the document
/// has not been saved yet or if it can't be represented with a local
/// path (e.g. if it is in a remote location like a web site).
/// @itemize @minus
/// @item Untitled => @null{}
/// @item @file{/home/user/example.txt} => @code{"/home/user/example.txt"}
/// @item @file{http://example.com/index.html} => @null{}
/// @end itemize
String Document::filename()
{
    char *filename = moo_edit_get_utf8_filename(gobj());
    return filename ? String::take_utf8(filename) : String::Null();
}

/// @item Document.uri()
/// returns URI of the document or @null{} if the document has not been
/// saved yet.
String Document::uri()
{
    char *uri = moo_edit_get_uri(gobj());
    return uri ? String::take_utf8(uri) : String::Null();
}

/// @item Document.basename()
/// returns basename of the document, that is the full path minus directory
/// part. If the document has not been saved yet, then it returns the name
/// shown in the titlebar, e.g. "Untitled".
String Document::basename()
{
    return String(moo_edit_get_display_basename(gobj()));
}

/// @item Document.is_modified()
/// returns whether the document is modified.
bool Document::is_modified()
{
    return MOO_EDIT_IS_MODIFIED(gobj());
}

/// @item Document.set_modified(modified)
/// sets modification state of the document.
void Document::set_modified(bool modified)
{
    moo_edit_set_modified(gobj(), modified);
}

/// @item Document.encoding()
/// returns character encoding of the document.
String Document::encoding()
{
    return moo_edit_get_encoding(gobj());
}

/// @item Document.set_encoding(encoding)
/// set character encoding of the document, it will be used when the document
/// is saved.
void Document::set_encoding(const String &enc)
{
    moo_edit_set_encoding(gobj(), enc);
}

String Document::line_endings()
{
    switch (moo_edit_get_line_end_type(gobj()))
    {
        case MOO_LE_UNIX:
            return "unix";
        case MOO_LE_WIN32:
            return "win32";
        case MOO_LE_MAC:
            return "mac";
        case MOO_LE_MIX:
            return "mix";
        case MOO_LE_NONE:
            return "none";
    }

    moo_assert_not_reached();
    Error::raise("error");
}

void Document::set_line_endings(const String &str_le)
{
    MooLineEndType le = MOO_LE_NONE;

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
}

/// @item Document.reload()
/// reload the document.
void Document::reload()
{
    Editor::get_instance().reload(*this);
}

/// @item Document.save()
/// save the document.
bool Document::save()
{
    return Editor::get_instance().save(*this);
}

/// @item Document.save_as(filename=null)
/// save the document as @param{filename}. If @param{filename} is @null{} then
/// @uilabel{Save As} will be shown to choose new filename.
bool Document::save_as(const String &filename)
{
    return Editor::get_instance().save_as(*this, filename);
}

/// @item Document.can_undo()
/// returns whether undo action is available.
bool Document::can_undo()
{
    return moo_text_view_can_undo(MOO_TEXT_VIEW(gobj()));
}

/// @item Document.can_redo()
/// returns whether redo action is available.
bool Document::can_redo()
{
    return moo_text_view_can_redo(MOO_TEXT_VIEW(gobj()));
}

/// @item Document.undo()
/// undo.
void Document::undo()
{
    moo_text_view_undo(MOO_TEXT_VIEW(gobj()));
}

/// @item Document.redo()
/// redo.
void Document::redo()
{
    moo_text_view_redo(MOO_TEXT_VIEW(gobj()));
}

/// @item Document.begin_not_undoable_action()
/// mark the beginning of a non-undoable operation. Undo stack will be erased
/// and undo will not be recorded until @method{end_not_undoable_action()} call.
void Document::begin_not_undoable_action()
{
    moo_text_view_begin_not_undoable_action(MOO_TEXT_VIEW(gobj()));
}

/// @item Document.end_not_undoable_action()
/// end the non-undoable operation started with @method{begin_not_undoable_action()}.
void Document::end_not_undoable_action()
{
    moo_text_view_end_not_undoable_action(MOO_TEXT_VIEW(gobj()));
}

/// @end table

/// @table @method

/// @item Document.start_pos()
/// position at the beginning of the document (0 in Python, 1 in Lua, etc.)
gint64 Document::start_pos()
{
    return 0;
}

/// @item Document.end_pos()
/// position at the end of the document. This is the position past the last
/// character: it points to no character, but it is a valid position for
/// text insertion, cursor may be put there, etc.
gint64 Document::end_pos()
{
    return gtk_text_buffer_get_char_count(buffer(this));
}

/// @item Document.cursor_pos()
/// position at the cursor.
gint64 Document::cursor_pos()
{
    GtkTextBuffer *buf = buffer(this);
    GtkTextIter iter;
    gtk_text_buffer_get_iter_at_mark(buf, &iter, gtk_text_buffer_get_insert(buf));
    return gtk_text_iter_get_offset(&iter);
}

/// @item Document.set_cursor_pos(pos)
/// move cursor to position @param{pos}.
void Document::set_cursor_pos(gint64 pos)
{
    GtkTextBuffer *buf = buffer(this);
    GtkTextIter iter;
    get_iter(pos, buf, &iter);
    gtk_text_buffer_place_cursor(buf, &iter);
}

/// @item Document.selection()
/// returns selection bounds as a list of two items, start and end. Returned
/// list is always sorted, use @method{cursor()} and @method{selection_bound()}
/// if you need to distinguish beginning and end of selection. If no text is
/// is selected, then it returns pair @code{[cursor, cursor]}.
VariantArray Document::selection()
{
    GtkTextBuffer *buf = buffer(this);
    GtkTextIter start, end;
    gtk_text_buffer_get_selection_bounds(buf, &start, &end);
    return make_pair(Index(gtk_text_iter_get_offset(&start)),
                     Index(gtk_text_iter_get_offset(&end)));
}

/// @item Document.set_selection(bounds_as_list)
/// @item Document.set_selection(start, end)
/// select text.
void Document::set_selection(const ArgList &args)
{
    GtkTextBuffer *buf = buffer(this);
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
}

/// @item Document.selection_bound()
/// returns the selection bound other than cursor position. Selection is
/// either [cursor, selection_bound) or [selection_bound, cursor), depending
/// on direction user dragged the mouse (or on @method{set_selection}
/// arguments).
gint64 Document::selection_bound()
{
    GtkTextBuffer *buf = buffer(this);
    GtkTextIter iter;
    gtk_text_buffer_get_iter_at_mark(buf, &iter, gtk_text_buffer_get_selection_bound(buf));
    return gtk_text_iter_get_offset(&iter);
}

/// @item Document.has_selection()
/// whether any text is selected.
bool Document::has_selection()
{
    return gtk_text_buffer_get_selection_bounds(buffer(this), NULL, NULL);
}

/// @item Document.char_count()
/// character count.
gint64 Document::char_count()
{
    return gtk_text_buffer_get_char_count(buffer(this));
}

/// @item Document.line_count()
/// line count.
gint64 Document::line_count()
{
    return gtk_text_buffer_get_line_count(buffer(this));
}

/// @item Document.line_at_pos(pos)
/// returns index of the line which contains position @param{pos}.
gint64 Document::line_at_pos(gint64 pos)
{
    GtkTextBuffer *buf = buffer(this);
    GtkTextIter iter;
    get_iter(pos, buf, &iter);
    return gtk_text_iter_get_line(&iter);
}

/// @item Document.pos_at_line(line)
/// returns position at the beginning of line @param{line}.
gint64 Document::pos_at_line(gint64 line)
{
    GtkTextBuffer *buf = buffer(this);
    if (line < 0 || line >= gtk_text_buffer_get_line_count(buf))
        Error::raise("invalid line");
    GtkTextIter iter;
    gtk_text_buffer_get_iter_at_line(buf, &iter, line);
    return gtk_text_iter_get_offset(&iter);
}

/// @item Document.pos_at_line_end(line)
/// returns position at the end of line @param{line}.
gint64 Document::pos_at_line_end(gint64 line)
{
    GtkTextBuffer *buf = buffer(this);
    if (line < 0 || line >= gtk_text_buffer_get_line_count(buf))
        Error::raise("invalid line");
    GtkTextIter iter;
    gtk_text_buffer_get_iter_at_line(buf, &iter, line);
    gtk_text_iter_forward_to_line_end(&iter);
    return gtk_text_iter_get_offset(&iter);
}

/// @item Document.char_at_pos(pos)
/// returns character at position @param{pos} as string.
String Document::char_at_pos(gint64 pos)
{
    GtkTextBuffer *buf = buffer(this);
    GtkTextIter iter;
    get_iter(pos, buf, &iter);
    if (gtk_text_iter_is_end(&iter))
        Error::raise("can't get text at end of buffer");
    gunichar c = gtk_text_iter_get_char(&iter);
    char b[7];
    b[g_unichar_to_utf8(c, b)] = 0;
    return String(b);
}

/// @item Document.text()
/// returns whole document contents.
/// @item Document.text(start, end)
/// returns text in the range [@param{start}, @param{end}), @param{end} not
/// included. Example: @code{doc.text(doc.start_pos(), doc.end_pos())} is
/// equivalent @code{to doc.text()}.
String Document::text(const ArgList &args)
{
    if (args.size() == 0)
    {
        GtkTextBuffer *buf = buffer(this);
        GtkTextIter start, end;
        gtk_text_buffer_get_bounds(buf, &start, &end);
        return String::take_utf8(gtk_text_buffer_get_slice(buf, &start, &end, TRUE));
    }

    if (args.size() != 2)
        Error::raise("either 0 or 2 arguments expected");

    GtkTextBuffer *buf = buffer(this);
    GtkTextIter start, end;
    get_iter(args[0], buf, &start);
    get_iter(args[1], buf, &end);
    return String::take_utf8(gtk_text_buffer_get_slice(buf, &start, &end, TRUE));
}

/// @item Document.insert_text(text)
/// @item Document.insert_text(pos, text)
/// insert text into the document. If @param{pos} is not given, insert at
/// cursor position.
void Document::insert_text(const ArgList &args)
{
    String text;
    GtkTextIter iter;
    GtkTextBuffer *buf = buffer(this);

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

}

/// @item Document.replace_text(start, end, text)
/// replace text in the region [@param{start}, @param{end}). Equivalent to
/// @code{delete_text(start, end), insert_text(start, text)}.
void Document::replace_text(gint64 start_pos, gint64 end_pos, const String &text)
{
    GtkTextIter start, end;
    GtkTextBuffer *buf = buffer(this);

    get_iter(start_pos, buf, &start);
    get_iter(end_pos, buf, &end);

    gtk_text_buffer_delete(buf, &start, &end);
    gtk_text_buffer_insert(buf, &start, text.utf8(), -1);
}

/// @item Document.delete_text(start, end)
/// delete text in the region [@param{start}, @param{end}). Example:
/// @code{doc.delete_text(doc.start(), doc.end())} will delete all text in
/// @code{doc}.
void Document::delete_text(gint64 start_pos, gint64 end_pos)
{
    GtkTextIter start, end;
    GtkTextBuffer *buf = buffer(this);

    get_iter(start_pos, buf, &start);
    get_iter(end_pos, buf, &end);

    gtk_text_buffer_delete(buf, &start, &end);
}

/// @item Document.append_text(text)
/// append text. Equivalent to @code{doc.insert_text(doc.end(), text)}.
void Document::append_text(const String &text)
{
    GtkTextIter iter;
    GtkTextBuffer *buf = buffer(this);
    gtk_text_buffer_get_end_iter(buf, &iter);
    gtk_text_buffer_insert(buf, &iter, text.utf8(), -1);
}

/// @item Document.clear()
/// delete all text in the document.
void Document::clear()
{
    GtkTextBuffer *buf = buffer(this);
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(buf, &start, &end);
    gtk_text_buffer_delete(buf, &start, &end);
}

/// @item Document.copy()
/// copy selected text to clipboard. If no text is selected then nothing
/// will happen, same as Ctrl-C key combination.
void Document::copy()
{
    g_signal_emit_by_name(gobj(), "copy-clipboard");
}

/// @item Document.cut()
/// cut selected text to clipboard. If no text is selected then nothing
/// will happen, same as Ctrl-X key combination.
void Document::cut()
{
    g_signal_emit_by_name(gobj(), "cut-clipboard");
}

/// @item Document.paste()
/// paste text from clipboard. It has the same effect as Ctrl-V key combination:
/// nothing happens if clipboard is empty, and selected text is replaced with
/// clipboard contents otherwise.
void Document::paste()
{
    g_signal_emit_by_name(gobj(), "paste-clipboard");
}

/// @item Document.select_text(bounds_as_list)
/// @item Document.select_text(start, end)
/// select text, same as @method{set_selection()}.
void Document::select_text(const ArgList &args)
{
    GtkTextIter start, end;
    GtkTextBuffer *buf = buffer(this);

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
}

/// @item Document.select_lines(line)
/// select a line.
/// @item Document.select_lines(first, last)
/// select lines from @param{first} to @param{last}, @emph{including}
/// @param{last}.
void Document::select_lines(const ArgList &args)
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

    GtkTextBuffer *buf = buffer(this);

    if (first_line.get() < 0 || first_line.get() >= gtk_text_buffer_get_line_count(buf))
        Error::raise("invalid line");
    if (last_line.get() < 0 || last_line.get() >= gtk_text_buffer_get_line_count(buf))
        Error::raise("invalid line");

    GtkTextIter start, end;
    gtk_text_buffer_get_iter_at_line(buf, &start, first_line.get());
    gtk_text_buffer_get_iter_at_line(buf, &end, last_line.get());
    gtk_text_iter_forward_line(&end);
    gtk_text_buffer_select_range(buf, &start, &end);
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

/// @item Document.select_lines_at_pos(bounds_as_list)
/// @item Document.select_lines_at_pos(start, end)
/// select lines: similar to @method{select_text}, but select whole lines.
void Document::select_lines_at_pos(const ArgList &args)
{
    GtkTextBuffer *buf = buffer(this);
    GtkTextIter start, end;
    get_select_lines_range(args, buf, &start, &end);
    gtk_text_buffer_select_range(buf, &start, &end);
}

/// @item Document.select_all()
/// select all.
void Document::select_all()
{
    GtkTextBuffer *buf = buffer(this);
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(buf, &start, &end);
    gtk_text_buffer_select_range(buf, &start, &end);
}

/// @item Document.selected_text()
/// returns selected text.
String Document::selected_text()
{
    GtkTextBuffer *buf = buffer(this);
    GtkTextIter start, end;
    gtk_text_buffer_get_selection_bounds(buf, &start, &end);
    return String::take_utf8(gtk_text_buffer_get_slice(buf, &start, &end, TRUE));
}

static void get_selected_lines_bounds(GtkTextBuffer *buf, GtkTextIter *start, GtkTextIter *end, bool *cursor_at_next_line = 0)
{
    if (cursor_at_next_line)
        *cursor_at_next_line = false;

    gtk_text_buffer_get_selection_bounds(buf, start, end);

    gtk_text_iter_set_line_offset(start, 0);

    if (gtk_text_iter_starts_line(end) && !gtk_text_iter_equal(start, end))
    {
        gtk_text_iter_backward_line(end);
        if (cursor_at_next_line)
            *cursor_at_next_line = true;
    }

    if (!gtk_text_iter_ends_line(end))
        gtk_text_iter_forward_to_line_end(end);

}

/// @item Document.selected_lines()
/// returns selected lines as a list of strings, one string for each line,
/// line terminator characters not included. If nothing is selected, then
/// line at cursor is returned.
VariantArray Document::selected_lines()
{
    GtkTextIter start, end;
    GtkTextBuffer *buf = buffer(this);
    get_selected_lines_bounds(buf, &start, &end);
    char *text = gtk_text_buffer_get_slice(buf, &start, &end, TRUE);
    char **lines = moo_splitlines(text);
    VariantArray ar;
    for (char **p = lines; p && *p; ++p)
        ar.append(String::take_utf8(*p));
    g_free(lines);
    g_free(text);
    return ar;
}

/// @item Document.delete_selected_text()
/// delete selected text, equivalent to @code{doc.delete_text(doc.cursor(),
/// doc.selection_bound())}.
void Document::delete_selected_text()
{
    GtkTextBuffer *buf = buffer(this);
    GtkTextIter start, end;
    gtk_text_buffer_get_selection_bounds(buf, &start, &end);
    gtk_text_buffer_delete(buf, &start, &end);
}

/// @item Document.delete_selected_lines()
/// delete selected lines. Similar to @method{delete_selected_text()} but
/// selection is extended to include whole lines. If nothing is selected,
/// then line at cursor is deleted.
void Document::delete_selected_lines()
{
    GtkTextIter start, end;
    GtkTextBuffer *buf = buffer(this);
    get_selected_lines_bounds(buf, &start, &end);
    gtk_text_iter_forward_line(&end);
    gtk_text_buffer_delete(buf, &start, &end);
}

/// @item Document.replace_selected_text(text)
/// replace selected text with @param{text}. If nothing is selected,
/// @param{text} is inserted at cursor.
void Document::replace_selected_text(const String &text)
{
    GtkTextBuffer *buf = buffer(this);
    GtkTextIter start, end;
    gtk_text_buffer_get_selection_bounds(buf, &start, &end);
    gtk_text_buffer_delete(buf, &start, &end);
    gtk_text_buffer_insert(buf, &start, text.utf8(), -1);
    gtk_text_buffer_place_cursor(buf, &start);
}

static String join(const moo::Vector<String> &list, const String &sep)
{
    std::stringstream ss;
    for (int i = 0, c = list.size(); i < c; ++i)
    {
        if (i != 0)
            ss << sep.utf8();
        ss << list[i].utf8();
    }
    return ss.str();
}

/// @item Document.replace_selected_lines(text)
/// @item Document.replace_selected_lines(lines)
/// replace selected lines with @param{text}. Similar to
/// @method{replace_selected_text()}, but selection is extended to include
/// whole lines. If nothing is selected, then line at cursor is replaced.
void Document::replace_selected_lines(const Variant &repl)
{
    String text;

    switch (repl.vt())
    {
        case VtString:
            text = repl.value<VtString>();
            break;
        case VtArray:
            {
                moo::Vector<String> lines = get_string_list(repl);
                text = join(lines, "\n");
            }
            break;
        default:
            Error::raise("string or list of strings expected");
            break;
    }

    GtkTextBuffer *buf = buffer(this);
    GtkTextIter start, end;
    bool cursor_at_next_line;
    get_selected_lines_bounds(buf, &start, &end, &cursor_at_next_line);
    gtk_text_buffer_delete(buf, &start, &end);
    gtk_text_buffer_insert(buf, &start, text.utf8(), -1);

    if (cursor_at_next_line)
    {
        gtk_text_iter_forward_line(&start);
        gtk_text_buffer_place_cursor(buf, &start);
    }
}

/// @end table

} // namespace mom
