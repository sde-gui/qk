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
    Result() NOTHROW
        : m_success(true)
        , m_message()
    {
    }

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

    NORETURN static void raisef(const char *format, ...)
    {
        va_list args;
        va_start(args, format);
        char *msg = g_strdup_vprintf(format, args);
        va_end(args);
        String str(msg);
        g_free(msg);
        raise(str);
    }

private:
    String m_message;
};

class Callback : public moo::RefCounted<Callback>
{
public:
    virtual Variant run(const ArgList &args) = 0;
    virtual void on_connect() = 0;
    virtual void on_disconnect() = 0;
};

struct ArgSet
{
    ArgList pos;
    ArgDict kw;

    ArgSet() {}
    explicit ArgSet(const ArgList &pos) : pos(pos) {}
    ArgSet(const ArgList &pos, const ArgDict &kw) : pos(pos), kw(kw) {}
};

class Script
{
public:
    static HObject get_app_obj() NOTHROW;
    static Result call_method(HObject obj, const String &meth, const ArgSet &args, Variant &ret) NOTHROW;
    static Result connect_callback(HObject obj, const String &event, moo::SharedPtr<Callback> cb, gulong &id) NOTHROW;
    static Result disconnect_callback(HObject obj, gulong id) NOTHROW;
};

} // namespace mom

#endif // MOM_SCRIPT_API_H
