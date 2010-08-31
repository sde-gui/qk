#include "momscript-classes.h"

namespace mom {

MetaObject Object::s_meta(Object::InitMetaObject);
moo::Dict<guint, moo::SharedPtr<Object> > Object::s_objects;
guint Object::s_last_id;

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

Variant Object::call_method(const String &name, const VariantArray &args)
{
    moo::SharedPtr<Method> meth = m_meta.lookup_method(name);
    if (!meth)
        Error::raise("invalid method");
    return meth->call(*this, args);
}

void Object::set_property(const String &name, const Variant &val)
{
    moo::SharedPtr<Property> prop = m_meta.lookup_property(name);
    if (!prop)
        Error::raise("invalid property");
    prop->set(*this, val);
}

Variant Object::get_property(const String &name)
{
    moo::SharedPtr<Property> prop = m_meta.lookup_property(name);
    if (!prop)
        Error::raise("invalid property");
    return prop->get(*this);
}

HObject Script::get_global_obj()
{
    try
    {
        return Global::get_instance();
    }
    catch (...)
    {
        return HObject();
    }
}

FieldKind Script::lookup_field(HObject h, const String &field)
{
    try
    {
        moo::SharedPtr<Object> obj = Object::lookup_object(h);
        if (!obj)
            return FieldInvalid;
        else
            return obj->meta().lookup_field(field);
    }
    catch (...)
    {
        return FieldInvalid;
    }
}

Result Script::call_method(HObject h, const String &meth, const VariantArray &args, Variant &ret)
{
    try
    {
        moo::SharedPtr<Object> obj = Object::lookup_object(h);
        if (!obj)
            Error::raise("invalid object");
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

Result Script::set_property(HObject h, const String &prop, const Variant &val)
{
    try
    {
        moo::SharedPtr<Object> obj = Object::lookup_object(h);
        if (!obj)
            Error::raise("invalid object");
        obj->set_property(prop, val);
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

Result Script::get_property(HObject h, const String &prop, Variant &val)
{
    try
    {
        moo::SharedPtr<Object> obj = Object::lookup_object(h);
        if (!obj)
            Error::raise("invalid object");
        val = obj->get_property(prop);
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

moo::SharedPtr<IScript> get_mom_script_instance()
{
    return new Script;
}

} // namespace mom
