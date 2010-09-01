#ifndef MOM_SCRIPT_CLASSES_H
#define MOM_SCRIPT_CLASSES_H

#include "momscript.h"
#include <mooedit/mooeditwindow.h>

namespace mom {

#define PROPERTY(Class,name,kind)           \
    Variant get_##name();                   \
    void set_##name(const Variant &value)

#define METHOD(Class,name)                  \
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

class Global : public _Singleton<Global>
{
public:
    PROPERTY(Global, application, read);
    PROPERTY(Global, editor, read);
    PROPERTY(Global, active_window, read);
    PROPERTY(Global, active_view, read);
    PROPERTY(Global, active_document, read);

private:
    MOM_SINGLETON_DECL(Global)
};

class Application : public _Singleton<Application>
{
public:
    PROPERTY(Application, editor, read);
    PROPERTY(Application, active_window, read-write);
    PROPERTY(Application, windows, read);

    METHOD(Application, quit);

private:
    MOM_SINGLETON_DECL(Application)
};

class Editor : public _Singleton<Editor>
{
public:
    PROPERTY(Editor, active_document, read-write);
    PROPERTY(Editor, active_window, read-write);
    PROPERTY(Editor, active_view, read-write);
    PROPERTY(Editor, documents, read);
    PROPERTY(Editor, views, read);
    PROPERTY(Editor, windows, read);

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

class DocumentWindow : public _GObjectWrapper<DocumentWindow, MooEditWindow>
{
public:
    PROPERTY(DocumentWindow, editor, read);
    PROPERTY(DocumentWindow, active_view, read-write);
    PROPERTY(DocumentWindow, active_document, read-write);
    PROPERTY(DocumentWindow, views, read);
    PROPERTY(DocumentWindow, documents, read);

    PROPERTY(DocumentWindow, active, read);
    METHOD(DocumentWindow, set_active);

private:
    MOM_GOBJECT_DECL(DocumentWindow, MooEditWindow)
};

class DocumentView : public _GObjectWrapper<DocumentView, MooEdit>
{
public:
    PROPERTY(DocumentView, document, read);
    PROPERTY(DocumentView, window, read);

    PROPERTY(DocumentView, line_wrap_mode, read-write);
    PROPERTY(DocumentView, overwrite_mode, read-write);
    PROPERTY(DocumentView, show_line_numbers, read-write);

private:
    MOM_GOBJECT_DECL(DocumentView, MooEdit)
};

class Document : public _GObjectWrapper<Document, MooEdit>
{
public:
    PROPERTY(Document, views, read);
    PROPERTY(Document, active_view, read);

    PROPERTY(Document, can_undo, read);
    PROPERTY(Document, can_redo, read);
    METHOD(Document, undo);
    METHOD(Document, redo);
    METHOD(Document, begin_not_undoable_action);
    METHOD(Document, end_not_undoable_action);

private:
    MOM_GOBJECT_DECL(Document, MooEdit)
};

#undef PROPERTY
#undef METHOD

} // namespace mom

#endif // MOM_SCRIPT_CLASSES_H
