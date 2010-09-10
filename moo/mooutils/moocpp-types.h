/*
 *   moocpp-types.h
 *
 *   Copyright (C) 2004-2009 by Yevgen Muntyan <muntyan@tamu.edu>
 *
 *   This file is part of medit.  medit is free software; you can
 *   redistribute it and/or modify it under the terms of the
 *   GNU Lesser General Public License as published by the
 *   Free Software Foundation; either version 2.1 of the License,
 *   or (at your option) any later version.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with medit.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MOO_CPP_TYPES_H
#define MOO_CPP_TYPES_H

#include <mooutils/moocpp-exc.h>

namespace moo {

#define MOO_IMPLEMENT_POINTER(Class, get_ptr_expr)                          \
    Class *operator->() const { return get_ptr_expr; }                      \
    Class &operator*() const { return *mooCheckPtr(get_ptr_expr); }

#define MOO_IMPLEMENT_POINTER_TO_MEM(Class, get_ptr_expr)                   \
    Class *operator->() { return get_ptr_expr; }                            \
    const Class *operator->() const { return get_ptr_expr; }                \
    Class &operator*() { return *mooCheckPtr(get_ptr_expr); }               \
    const Class &operator*() const { return *mooCheckPtr(get_ptr_expr); }

#define MOO_IMPLEMENT_BOOL(get_bool_expr)                                   \
    operator bool() const { return get_bool_expr; }                         \
    bool operator !() const { return !(get_bool_expr); }

#define MOO_IMPLEMENT_SCALAR(Class, val)                                    \
    bool operator==(const Class &other) const { return val == other.val; }  \
    bool operator!=(const Class &other) const { return !(*this == other); } \
    bool operator<(const Class &other) const { return val < other.val; }

template<typename T>
class CheckedPtr
{
public:
    CheckedPtr(T *p) : m_p(p) { mooThrowIfFalse(p); }
    CheckedPtr(T *p, const char *msg, const MooCodeLoc &loc) : m_p(p) { if (!p) ExcUnexpected::raise(msg, loc); }

    NOTHROW operator T*() const { return m_p; }
    T & NOTHROW operator*() const { return *m_p; }
    T * NOTHROW operator->() const { return m_p; }

private:
    T *m_p;
};

template<typename T>
inline T *_mooCheckPtrImpl(T *p, const char *msg, const MooCodeLoc &loc)
{
    if (!p)
        ExcUnexpected::raise(msg, loc);
    return p;
}

#define mooCheckPtr(p) (moo::_mooCheckPtrImpl(p, #p " is null", MOO_CODE_LOC))

class RefCount
{
public:
    RefCount(int count) : m_count(count) { mooCheck(count >= 0); }

    operator int() const
    {
        return g_atomic_int_get(&m_count);
    }

    void ref()
    {
        g_atomic_int_inc(&m_count);
    }

    bool unref()
    {
        mooAssert(m_count > 0);
        return g_atomic_int_dec_and_test(&m_count) != 0;
    }

private:
    MOO_DISABLE_COPY_AND_ASSIGN(RefCount)

private:
    int m_count;
};

template<typename T>
class ValueRestorer
{
public:
    ValueRestorer(T &val)
        : m_ref(val)
        , m_val(val)
    {
    }

    ~ValueRestorer()
    {
        m_ref = m_val;
    }

    MOO_DISABLE_COPY_AND_ASSIGN(ValueRestorer)

private:
    T &m_ref;
    T m_val;
};

template<typename BaseEnum>
class FlagsEnum
{
    struct NonExistent;
    BaseEnum m_val;

public:
    FlagsEnum(BaseEnum val) : m_val(val) {}
    FlagsEnum(NonExistent * = 0) : m_val(BaseEnum(0)) {}

    bool operator==(BaseEnum other) { return m_val == other; }
    bool operator!=(BaseEnum other) { return m_val != other; }

    // no operator bool()
    bool operator!() const { return !m_val; }

    operator BaseEnum() const { return m_val; }

    FlagsEnum &operator&=(BaseEnum other) { m_val &= other; return *this; }
    FlagsEnum &operator|=(BaseEnum other) { m_val |= other; return *this; }
    FlagsEnum &operator^=(BaseEnum other) { m_val ^= other; return *this; }

    FlagsEnum operator&(BaseEnum other) const { return FlagsEnum(BaseEnum(m_val & other)); }
    FlagsEnum operator|(BaseEnum other) const { return FlagsEnum(BaseEnum(m_val | other)); }
    FlagsEnum operator^(BaseEnum other) const { return FlagsEnum(BaseEnum(m_val ^ other)); }

    FlagsEnum operator~() const { return BaseEnum(~m_val); }
};

#define MOO_DECLARE_FLAGS(Flags, Flag) typedef moo::FlagsEnum<Flag> Flags

} // namespace moo

#endif /* MOO_CPP_TYPES_H */
/* -%- strip:true -%- */
