#include "momscript-classes.h"
#include "mooapp/mooapp.h"

namespace mom {

static void check_no_args(const VariantArray &args)
{
    if (args.size() != 0)
        Error::raise("no arguments expected");
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


MOM_SINGLETON_DEFN(Global)

void Global::InitMetaObject(MetaObject &meta)
{
    meta.add_property("application", &Global::get_application, (PropertySetter) 0);
    meta.add_property("editor", &Global::get_editor, (PropertySetter) 0);
    meta.add_property("active_window", &Global::get_active_window, (PropertySetter) 0);
    meta.add_property("active_document", &Global::get_active_document, (PropertySetter) 0);
    meta.add_property("active_view", &Global::get_active_view, (PropertySetter) 0);
}

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

MOM_SINGLETON_DEFN(Application)

void Application::InitMetaObject(MetaObject &meta)
{
    meta.add_property("editor", &Application::get_editor, (PropertySetter) 0);
    meta.add_property("active_window", &Application::get_active_window, &Application::set_active_window);
    meta.add_property("windows", &Application::get_windows, (PropertySetter) 0);
    meta.add_method("quit", &Application::quit);
}

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

MOM_SINGLETON_DEFN(Editor)

void Editor::InitMetaObject(MetaObject &meta)
{
    meta.add_property("windows", &Editor::get_windows, (PropertySetter) 0);
    meta.add_property("documents", &Editor::get_documents, (PropertySetter) 0);
    meta.add_property("views", &Editor::get_views, (PropertySetter) 0);
    meta.add_property("active_window", &Editor::get_active_window, &Editor::set_active_window);
    meta.add_property("active_view", &Editor::get_active_view, &Editor::set_active_view);
    meta.add_property("active_document", &Editor::get_active_document, &Editor::set_active_document);
}

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

MOM_GOBJECT_DEFN(DocumentWindow)

void DocumentWindow::InitMetaObject(MetaObject &meta)
{
    meta.add_property("editor", &DocumentWindow::get_editor, (PropertySetter) 0);
    meta.add_property("active_view", &DocumentWindow::get_active_view,  &DocumentWindow::set_active_view);
    meta.add_property("active_document", &DocumentWindow::get_active_document, &DocumentWindow::set_active_document);
    meta.add_property("active", &DocumentWindow::get_active, (PropertySetter) 0);
    meta.add_property("views", &DocumentWindow::get_views, (PropertySetter) 0);

    meta.add_method("set_active", &DocumentWindow::set_active);
}

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

MOM_GOBJECT_DEFN(DocumentView)

void DocumentView::InitMetaObject(MetaObject &meta)
{
    meta.add_property("document", &DocumentView::get_document, (PropertySetter) 0);
    meta.add_property("window", &DocumentView::get_window, (PropertySetter) 0);
}

Variant DocumentView::get_document()
{
    return HObject(Document::wrap(gobj()));
}

Variant DocumentView::get_window()
{
    return HObject(DocumentWindow::wrap(moo_edit_get_window(gobj())));
}

MOM_GOBJECT_DEFN(Document)

void Document::InitMetaObject(MetaObject &meta)
{
    meta.add_property("views", &Document::get_views, (PropertySetter) 0);
    meta.add_property("active_view", &Document::get_active_view, (PropertySetter) 0);
}

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

} // namespace mom
