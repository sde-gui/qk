#ifndef MOM_SCRIPT_CLASSES_H
#define MOM_SCRIPT_CLASSES_H

#include "momscript.h"
#include <mooedit/mooeditwindow.h>

namespace mom {

#define SINGLETON_CLASS(Name)  class Name : public _Singleton<Name>
#define GOBJECT_CLASS(Name, GName) class Name : public _GObjectWrapper<Name, GName>

#define PROPERTY(name,kind)                 \
    Variant get_##name();                   \
    void set_##name(const Variant &value)

#define METHOD(name)                        \
    Variant name(const VariantArray &args)

template<typename T>
class _Singleton : public Object
{
public:
    static T &get_instance()
    {
        if (!s_instance)
            s_instance = new T;
        return *s_instance;
    }

    static void cleanup() NOTHROW
    {
        if (s_instance)
            s_instance->remove();
        moo_assert(s_instance == 0);
    }

protected:
    _Singleton(MetaObject &meta)
        : Object(meta)
    {
        mooThrowIfFalse(s_instance == 0);
        s_instance = static_cast<T*>(this);
    }

    ~_Singleton() NOTHROW
    {
        mooThrowIfFalse(s_instance == 0 || s_instance == this);
        s_instance = 0;
    }

private:
    static T *s_instance;
};

#define MOM_SINGLETON_DECL(Class)       \
private:                                \
    MOM_OBJECT_DECL(Class)              \
    friend class _Singleton<Class>;     \
    Class()                             \
        : _Singleton<Class>(s_meta)     \
    {                                   \
    }

#define MOM_SINGLETON_DEFN(Class)       \
    MOM_OBJECT_DEFN(Class)              \
    template<> Class *_Singleton<Class>::s_instance = 0;

SINGLETON_CLASS(Application)
{
public:
    METHOD(editor);
    METHOD(active_view);
    METHOD(active_document);

    METHOD(active_window);
    METHOD(set_active_window);
    METHOD(windows);

    METHOD(quit);

private:
    MOM_SINGLETON_DECL(Application)
};

///////////////////////////////////////////////////////////////////////////////
///
/// ==== Editor ====[mom-script-editor]
///
SINGLETON_CLASS(Editor)
{
public:
    /// - ``Editor.active_document()``: returns current active document or null
    /// if there are no open documents
    METHOD(active_document);
    /// - ``Editor.set_active_document(doc)``: makes ``doc`` active
    METHOD(set_active_document);
    /// - ``Editor.active_window()``: returns current active window
    METHOD(active_window);
    /// - ``Editor.set_active_window(window)``: makes ``window`` active
    METHOD(set_active_window);
    /// - ``Editor.active_view()``: returns current active document view
    METHOD(active_view);
    /// - ``Editor.set_active_view(view)``: makes ``view`` active
    METHOD(set_active_view);

    /// - ``Editor.documents()``: returns list of all open documents
    METHOD(documents);
    /// - ``Editor.documents()``: returns list of all open document views
    METHOD(views);
    /// - ``Editor.documents()``: returns list of all document windows
    METHOD(windows);

//     /// - ``Editor.get_document_by_path(path)``: returns document with path
//     /// ``path`` or null.
//     METHOD(get_document_by_path);
//     /// - ``Editor.get_document_by_uri(path)``: returns document with uri
//     /// ``uri`` or null.
//     METHOD(get_document_by_uri);
//
//     /// - ``Editor.open_files(files, window=null)``: open files. If ``window`` is
//     /// given then open files in that window, otherwise in an existing window.
//     METHOD(open_files);
//     /// - ``Editor.open_files(uris, window=null)``: open files. If ``window`` is
//     /// given then open files in that window, otherwise in an existing window.
//     METHOD(open_uris);
//     /// - ``Editor.open_file(file, encoding=null, window=null)``: open file.
//     /// If ``encoding`` is null or "auto" then pick character encoding automatically,
//     /// otherwise use ``encoding``.
//     /// If ``window`` is given then open files in that window, otherwise in an existing window.
//     METHOD(open_file);
//     /// - ``Editor.open_uri(uri, encoding=null, window=null)``: open file.
//     /// If ``encoding`` is null or "auto" then pick character encoding automatically,
//     /// otherwise use ``encoding``.
//     /// If ``window`` is given then open files in that window, otherwise in an existing window.
//     METHOD(open_uri);
//     /// - ``Editor.reload(doc)``: reload document.
//     METHOD(reload);
//     /// - ``Editor.save(doc)``: save document.
//     METHOD(save);
//     /// - ``Editor.save_as(doc, new_filename=null)``: save document as ``new_filename``.
//     /// If ``new_filename`` is not given then first ask user for new filename.
//     METHOD(save_as);
//     /// - ``Editor.save_as_uri(doc, new_uri=null)``: save document as ``new_uri``.
//     /// If ``new_uri`` is not given then first ask user for new filename.
//     METHOD(save_as_uri);
//     /// ``Editor.close(doc)``: close document.
//     METHOD(close);

private:
    MOM_SINGLETON_DECL(Editor)
};

template<typename T, typename GT>
class _GObjectWrapper : public Object
{
public:
    GT *gobj() const throw() { return m_gobj; }

    static T &wrap(GT *gobj)
    {
        mooThrowIfFalse(G_IS_OBJECT(gobj));
        void *pwrapper = g_object_get_qdata(G_OBJECT(gobj), wrapper_quark());
        if (pwrapper)
            return *static_cast<T*>(pwrapper);
        else
            return *new T(gobj);
    }

protected:
    static GQuark wrapper_quark() G_GNUC_CONST
    {
        static GQuark q;
        if (G_UNLIKELY(!q))
            q = g_quark_from_string(T::class_name() + "-mom-script-wrapper");
        return q;
    }

    _GObjectWrapper(GT *gobj, MetaObject &meta)
        : Object(meta)
        , m_gobj(gobj)
    {
        mooThrowIfFalse(G_IS_OBJECT(gobj));
        g_object_set_qdata(G_OBJECT(gobj), wrapper_quark(), this);
        g_object_weak_ref(G_OBJECT(gobj), (GWeakNotify) gobject_weak_notify, this);
    }

    ~_GObjectWrapper()
    {
        if (m_gobj)
        {
            g_object_weak_unref(G_OBJECT(m_gobj), (GWeakNotify) gobject_weak_notify, this);
            g_object_set_qdata(G_OBJECT(m_gobj), wrapper_quark(), 0);
        }
    }

private:
    static void gobject_weak_notify(_GObjectWrapper *wrapper)
    {
        wrapper->m_gobj = 0;
    }

private:
    MOO_DISABLE_COPY_AND_ASSIGN(_GObjectWrapper)

private:
    GT *m_gobj;
};

#define MOM_GOBJECT_DECL(Class, GClass)                 \
protected:                                              \
    friend class _GObjectWrapper<Class, GClass>;        \
    Class(GClass *gobj) :                               \
        _GObjectWrapper<Class, GClass>(gobj, s_meta)    \
    {                                                   \
    }                                                   \
    MOM_OBJECT_DECL(Class)

#define MOM_GOBJECT_DEFN(Class)                         \
    MOM_OBJECT_DEFN(Class)

GOBJECT_CLASS(DocumentWindow, MooEditWindow)
{
public:
    METHOD(editor);
    METHOD(active_view);
    METHOD(set_active_view);
    METHOD(active_document);
    METHOD(set_active_document);
    METHOD(views);
    METHOD(documents);

    METHOD(is_active);
    METHOD(set_active);

private:
    MOM_GOBJECT_DECL(DocumentWindow, MooEditWindow)
};

GOBJECT_CLASS(DocumentView, MooEdit)
{
public:
    METHOD(document);
    METHOD(window);

    METHOD(line_wrap_mode);
    METHOD(set_line_wrap_mode);
    METHOD(overwrite_mode);
    METHOD(set_overwrite_mode);
    METHOD(show_line_numbers);
    METHOD(set_show_line_numbers);

private:
    MOM_GOBJECT_DECL(DocumentView, MooEdit)
};

GOBJECT_CLASS(Document, MooEdit)
{
public:
    METHOD(views);
    METHOD(active_view);

    METHOD(filename);
    METHOD(uri);
    METHOD(basename);

//     METHOD(encoding);
//     METHOD(set_encoding);
//     METHOD(line_endings);
//     METHOD(set_line_endings);

//     METHOD(reload);
//     METHOD(save);
//     METHOD(save_as);
//     METHOD(save_as_uri);

    METHOD(can_undo);
    METHOD(can_redo);
    METHOD(undo);
    METHOD(redo);
    METHOD(begin_not_undoable_action);
    METHOD(end_not_undoable_action);

    METHOD(start_pos);
    METHOD(end_pos);
    METHOD(cursor_pos);
    METHOD(set_cursor_pos);
    METHOD(selection);
    METHOD(set_selection);
    METHOD(selection_bound);
    METHOD(has_selection);

    METHOD(char_count);
    METHOD(line_count);

    METHOD(line_at_pos);
    METHOD(pos_at_line);
    METHOD(pos_at_line_end);
    METHOD(char_at_pos);
    METHOD(text);

    METHOD(insert_text);
    METHOD(replace_text);
    METHOD(delete_text);
    METHOD(append_text);

    METHOD(clear);
    METHOD(copy);
    METHOD(cut);
    METHOD(paste);

    METHOD(select_text);
    METHOD(select_lines);
    METHOD(select_lines_at_pos);
    METHOD(select_all);

    METHOD(selected_text);
    METHOD(selected_lines);
    METHOD(delete_selected_text);
    METHOD(delete_selected_lines);
    METHOD(replace_selected_text);
    METHOD(replace_selected_lines);

public:
    GtkTextBuffer *buffer() { return gtk_text_view_get_buffer(GTK_TEXT_VIEW(gobj())); }

private:
    MOM_GOBJECT_DECL(Document, MooEdit)
};

#undef PROPERTY
#undef METHOD

} // namespace mom

#endif // MOM_SCRIPT_CLASSES_H
