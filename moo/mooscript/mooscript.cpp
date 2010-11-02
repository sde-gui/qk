#include "mooscript-classes.h"

namespace mom {

MetaObject Object::s_meta("Object", Object::InitMetaObject);
moo::Dict<guint, moo::SharedPtr<Object> > Object::s_objects;
guint Object::s_last_id;
gulong Object::s_last_callback_id;

HObject::HObject(const Object &obj)
    : m_id(obj.id())
{
}

void Object::InitMetaObject(MetaObject &)
{
}

void Object::_InitMetaObjectFull(MetaObject &meta)
{
    InitMetaObject(meta);
}

Variant Object::call_method(const String &name, const ArgSet &args)
{
    moo::SharedPtr<Method> meth = m_meta.lookup_method(name);
    if (!meth)
        Error::raisef("no method '%s' in class '%s'", (const char*) name, (const char*) m_meta.name());
    return meth->call(*this, args);
}

gulong Object::connect_callback(const String &name, moo::SharedPtr<Callback> cb)
{
    if (!m_meta.lookup_signal(name))
        Error::raisef("no signal '%s' in class '%s'", (const char*) name, (const char*) m_meta.name());

    CallbackInfo info = { ++s_last_callback_id, cb };

    if (m_callbacks.contains(name))
    {
        m_callbacks[name].append(info);
    }
    else
    {
        moo::Vector<CallbackInfo> vec;
        vec.append(info);
        m_callbacks[name] = vec;
    }

    try
    {
        cb->on_connect();
    }
    catch (...)
    {
        disconnect_callback(info.id, false);
        throw;
    }

    return info.id;
}

void Object::disconnect_callback(gulong id, bool notify)
{
    moo::SharedPtr<Callback> cb;
    for (CallbackMap::iterator iter = m_callbacks.begin(); !cb && iter != m_callbacks.end(); ++iter)
    {
        moo::Vector<CallbackInfo> &vec = iter.value();
        for (int i = 0, c = vec.size(); i < c; ++i)
        {
            if (vec[i].id == id)
            {
                cb = vec[i].cb;
                vec.removeAt(i);
                break;
            }
        }
    }
    if (cb && notify)
        cb->on_disconnect();
}

void Object::disconnect_callback(gulong id)
{
    disconnect_callback(id, true);
}

moo::Vector<moo::SharedPtr<Callback> > Object::list_callbacks(const String &name) const
{
    if (!m_meta.lookup_signal(name))
        Error::raisef("no signal '%s' in class '%s'", (const char*) name, (const char*) m_meta.name());

    moo::Vector<moo::SharedPtr<Callback> > callbacks;
    CallbackMap::const_iterator iter = m_callbacks.find(name);

    if (iter != m_callbacks.end())
    {
        const moo::Vector<CallbackInfo> &info = iter.value();
        callbacks.resize(info.size());
        for (int i = 0, c = info.size(); i < c; ++i)
            callbacks[i] = info[i].cb;
    }

    return callbacks;
}

bool Object::has_callbacks(const String &name) const
{
    if (!m_meta.lookup_signal(name))
        Error::raisef("no signal '%s' in class '%s'", (const char*) name, (const char*) m_meta.name());

    return m_callbacks.find(name) != m_callbacks.end();
}

HObject Script::get_app_obj()
{
    try
    {
        return Application::get_instance();
    }
    catch (...)
    {
        return HObject();
    }
}

Result Script::call_method(HObject h, const String &meth, const ArgSet &args, Variant &ret)
{
    try
    {
        moo::SharedPtr<Object> obj = Object::lookup_object(h);
        if (!obj)
            Error::raisef("no object with id %lu", h.id());
        ret = obj->call_method(meth, args);
        return true;
    }
    catch (const Error &err)
    {
        return Result(false, err.message());
    }
    catch (...)
    {
        return false;
    }
}

Result Script::connect_callback(HObject h, const String &event, moo::SharedPtr<Callback> cb, gulong &id)
{
    try
    {
        id = 0;
        moo::SharedPtr<Object> obj = Object::lookup_object(h);
        if (!obj)
            Error::raisef("no object with id %lu", h.id());
        id = obj->connect_callback(event, cb);
        return true;
    }
    catch (const Error &err)
    {
        return Result(false, err.message());
    }
    catch (...)
    {
        return false;
    }
}

Result Script::disconnect_callback(HObject h, gulong id)
{
    try
    {
        moo::SharedPtr<Object> obj = Object::lookup_object(h);
        if (!obj)
            Error::raisef("no object with id %lu", h.id());
        obj->disconnect_callback(id);
        return true;
    }
    catch (const Error &err)
    {
        return Result(false, err.message());
    }
    catch (...)
    {
        return false;
    }
}

} // namespace mom
