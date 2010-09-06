#ifndef MOM_SCRIPT_API_H
#define MOM_SCRIPT_API_H

#include "mooscript-variant.h"

namespace mom {

struct Result
{
private:
    bool m_success;
    String m_message;

public:
    Result(bool success, const String &message = "unknown error") NOTHROW
        : m_success(success)
        , m_message(message)
    {
    }

    bool succeeded() const NOTHROW { return m_success; }
    const String &message() const NOTHROW { return m_message; }
};

class Error
{
protected:
    NOTHROW Error(const String &message) : m_message(message) {}

public:
    NOTHROW const String &message() const { return m_message; }

    NORETURN static void raise(const String &message = "unknown error")
    {
        throw Error(message);
    }

private:
    String m_message;
};

enum FieldKind
{
    FieldInvalid,
    FieldMethod,
    FieldProperty
};

// #pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
class IScript
{
public:
    virtual void ref() NOTHROW = 0;
    virtual void unref() NOTHROW = 0;
    virtual HObject get_app_obj() NOTHROW = 0;
    virtual Result call_method(HObject obj, const String &meth, const VariantArray &args, Variant &ret) NOTHROW = 0;
    virtual Result set_property(HObject obj, const String &prop, const Variant &val) NOTHROW = 0;
    virtual Result get_property(HObject obj, const String &prop, Variant &val) NOTHROW = 0;
    virtual FieldKind lookup_field(HObject obj, const String &field) NOTHROW = 0;
};
// #pragma GCC diagnostic pop

moo::SharedPtr<IScript> get_mom_script_instance();

} // namespace mom

#endif // MOM_SCRIPT_API_H
