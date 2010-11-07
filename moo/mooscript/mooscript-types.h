#ifndef MOM_SCRIPT_TYPES_H
#define MOM_SCRIPT_TYPES_H

#include <glib.h>
#include "moocpp-cont.h"

namespace mom {

using moo::String;

class Object;

class HObject
{
public:
    HObject(guint id = 0) NOTHROW : m_id(id) {}
    HObject(const Object &obj) NOTHROW;
    guint id() const NOTHROW { return m_id; }
    bool is_null() const NOTHROW { return m_id == 0; }
private:
    guint m_id;
};

class Index
{
public:
    Index(int value = 0) : m_value(value) {}

    int get() const throw() { return m_value; };
    int get_base1() const throw() { return m_value + 1; };

private:
    int m_value;
};

class Base1Int
{
public:
    Base1Int(int value = 0) throw() : m_value(value) {}

    int get() const throw() { return m_value; }
    Index get_index() const throw() { return m_value - 1; }

private:
    int m_value;
};

} // namespace mom

#endif // MOM_SCRIPT_TYPES_H
