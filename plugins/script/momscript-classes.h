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
    PROPERTY(editor, read);
    PROPERTY(active_window, read-write);
    PROPERTY(windows, read);

    PROPERTY(active_view, read);
    PROPERTY(active_document, read);

    METHOD(quit);

private:
    MOM_SINGLETON_DECL(Application)
};

SINGLETON_CLASS(Editor)
{
public:
    PROPERTY(active_document, read-write);
    PROPERTY(active_window, read-write);
    PROPERTY(active_view, read-write);
    PROPERTY(documents, read);
    PROPERTY(views, read);
    PROPERTY(windows, read);

//     METHOD(open);
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
    PROPERTY(editor, read);
    PROPERTY(active_view, read-write);
    PROPERTY(active_document, read-write);
    PROPERTY(views, read);
    PROPERTY(documents, read);

    PROPERTY(active, read);
    METHOD(set_active);

private:
    MOM_GOBJECT_DECL(DocumentWindow, MooEditWindow)
};

GOBJECT_CLASS(DocumentView, MooEdit)
{
public:
    PROPERTY(document, read);
    PROPERTY(window, read);

    PROPERTY(line_wrap_mode, read-write);
    PROPERTY(overwrite_mode, read-write);
    PROPERTY(show_line_numbers, read-write);

private:
    MOM_GOBJECT_DECL(DocumentView, MooEdit)
};

GOBJECT_CLASS(Document, MooEdit)
{
public:
    PROPERTY(views, read);
    PROPERTY(active_view, read);

    PROPERTY(filename, read);
    PROPERTY(uri, read);
    PROPERTY(basename, read);

//     METHOD(reload);
//     METHOD(save);
//     METHOD(save_as);

    PROPERTY(can_undo, read);
    PROPERTY(can_redo, read);
    METHOD(undo);
    METHOD(redo);
    METHOD(begin_not_undoable_action);
    METHOD(end_not_undoable_action);

    PROPERTY(start, read);
    PROPERTY(end, read);
    PROPERTY(cursor, read-write);
    PROPERTY(selection, read-write);
    PROPERTY(selection_bound, read);
    PROPERTY(has_selection, read);

    PROPERTY(char_count, read);
    PROPERTY(line_count, read);

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

    PROPERTY(selected_text, read);
    PROPERTY(selected_lines, read);
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
