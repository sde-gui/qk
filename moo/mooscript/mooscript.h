#ifndef MOM_SCRIPT_H
#define MOM_SCRIPT_H

#include "mooscript-api.h"

namespace mom {

using moo::String;

class Object;

class Method : public moo::RefCounted<Method>
{
public:
    virtual Variant call (Object &obj, const ArgArray &args) = 0;
};

class Signal : public moo::RefCounted<Signal>
{
};

class MetaObject
{
public:
    typedef void (*Init) (MetaObject &meta);
    MetaObject(Init init)
    {
        init(*this);
    }

    moo::SharedPtr<Method> lookup_method(const String &meth) const NOTHROW { return m_methods.value(meth); }
    moo::SharedPtr<Signal> lookup_signal(const String &sig) const NOTHROW { return m_signals.value(sig); }

    template<typename TObject>
    void add_method(const String &meth, Variant (TObject::*impl)(const ArgArray &args))
    {
        class MethodImpl : public Method
        {
        public:
            typedef Variant (TObject::*CallFunc)(const ArgArray &args);

            MethodImpl(CallFunc impl)
                : m_impl(impl)
            {
            }

            Variant call(Object &obj, const ArgArray &args)
            {
                TObject &tobj = static_cast<TObject&>(obj);
                return (tobj.*m_impl)(args);
            }

        private:
            CallFunc m_impl;
        };

        m_methods[meth] = new MethodImpl(impl);
    }

    void add_signal(const String &sig)
    {
        m_signals[sig] = new Signal;
    }

private:
    MOO_DISABLE_COPY_AND_ASSIGN(MetaObject)

private:
    moo::Dict<String, moo::SharedPtr<Method> > m_methods;
    moo::Dict<String, moo::SharedPtr<Signal> > m_signals;
};

#define MOM_OBJECT_DECL(Class) \
public: \
    static String class_name() { return #Class; } \
    static const MetaObject &class_meta() { return s_meta; } \
protected: \
    static MetaObject s_meta; \
    static void InitMetaObject(MetaObject &meta); \
    static void _InitMetaObjectFull(MetaObject &meta);

#define MOM_OBJECT_DEFN(Class) \
    MetaObject Class::s_meta(Class::_InitMetaObjectFull); \
    void Class::_InitMetaObjectFull(MetaObject &meta) \
    { \
        Object::_InitMetaObjectFull(meta); \
        Class::InitMetaObject(meta); \
    }

class Object : public moo::RefCounted<Object>
{
public:
    Variant call_method(const String &meth, const ArgArray &args);

    gulong connect_callback(const String &name, moo::SharedPtr<Callback> cb);
    void disconnect_callback(gulong id);
    moo::Vector<moo::SharedPtr<Callback> > list_callbacks(const String &name) const;
    bool has_callbacks(const String &name) const;

private:
    void disconnect_callback(gulong id, bool notify);

public:
    static moo::SharedPtr<Object> lookup_object (const HObject &h) NOTHROW
    {
        return s_objects.value(h.id());
    }

    guint id() const NOTHROW { return m_id; }
    const MetaObject &meta() const NOTHROW { return m_meta; }

    MOM_OBJECT_DECL(Object)

protected:
    Object(MetaObject &meta)
        : m_meta(meta)
        , m_id(++s_last_id)
    {
        s_objects[m_id] = moo::SharedPtr<Object>(this);
    }

    ~Object() NOTHROW
    {
        moo_assert(s_objects.value(m_id) == 0);
    }

    void remove() NOTHROW
    {
        s_objects.remove(m_id);
    }

private:
    MOO_DISABLE_COPY_AND_ASSIGN(Object)

private:
    typedef moo::Dict<guint, moo::SharedPtr<Object> > ObjectMap;
    static ObjectMap s_objects;
    static guint s_last_id;
    MetaObject &m_meta;
    guint m_id;

    struct CallbackInfo {
        gulong id;
        moo::SharedPtr<Callback> cb;
    };
    typedef moo::Dict<String, moo::Vector<CallbackInfo> > CallbackMap;
    CallbackMap m_callbacks;
    static gulong s_last_callback_id;
};

} // namespace mom

#endif // MOM_SCRIPT_H
