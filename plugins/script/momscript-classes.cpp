#include "momscript-classes.h"
#include "mooapp/mooapp.h"

namespace mom {

#include "momscript-classes-meta.h"

static void check_no_args(const VariantArray &args)
{
    if (args.size() != 0)
        Error::raise("no arguments expected");
}

// static void check_1_arg(const VariantArray &args)
// {
//     if (args.size() != 1)
//         Error::raise("exactly one argument expected");
// }

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
    moo_edit_set_line_wrap_mode(gobj(), get_bool(val));
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
    moo_edit_set_show_line_numbers(gobj(), get_bool(val));
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


} // namespace mom
