#ifndef MOM_SCRIPT_CLASSES_H
#define MOM_SCRIPT_CLASSES_H

#include "momscript.h"
#include <mooedit/mooeditwindow.h>

namespace mom {

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
    Variant get_application();
    Variant get_editor();
    Variant get_active_window();
    Variant get_active_view();
    Variant get_active_document();

private:
    MOM_SINGLETON_DECL(Global)
};

class Application : public _Singleton<Application>
{
public:
    Variant get_editor();
    Variant get_active_window();
    void set_active_window(const Variant &val);
    Variant get_windows();

    Variant quit(const VariantArray &args);

private:
    MOM_SINGLETON_DECL(Application)
};

class Editor : public _Singleton<Editor>
{
public:
    Variant get_active_document();
    void set_active_document(const Variant &val);
    Variant get_active_window();
    void set_active_window(const Variant &val);
    Variant get_active_view();
    void set_active_view(const Variant &val);
    Variant get_documents();
    Variant get_views();
    Variant get_windows();

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
    Variant get_editor();
    Variant get_active_view();
    void set_active_view(const Variant &val);
    Variant get_active_document();
    void set_active_document(const Variant &val);
    Variant get_views();
    Variant get_documents();

    Variant get_active();
    Variant set_active(const VariantArray &args);

private:
    MOM_GOBJECT_DECL(DocumentWindow, MooEditWindow)
};

class DocumentView : public _GObjectWrapper<DocumentView, MooEdit>
{
public:
    Variant get_document();
    Variant get_window();

private:
    MOM_GOBJECT_DECL(DocumentView, MooEdit)
};

class Document : public _GObjectWrapper<Document, MooEdit>
{
public:
    Variant get_views();
    Variant get_active_view();

private:
    MOM_GOBJECT_DECL(Document, MooEdit)
};

} // namespace mom

#endif // MOM_SCRIPT_CLASSES_H