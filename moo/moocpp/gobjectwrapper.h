/*
 *   moocpp/gobjectwrapper.h
 *
 *   Copyright (C) 2004-2015 by Yevgen Muntyan <emuntyan@users.sourceforge.net>
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

#pragma once

#include <algorithm>
#include <memory>
#include <vector>

#include "mooedit/mooedit.h"

namespace moo {

template<typename ObjClass>
class ObjRefUnref;

template<typename ObjClass, typename ObjRefUnrefHelper = ObjRefUnref<ObjClass>>
class RefPtr
{
public:
    explicit RefPtr(ObjClass* obj = nullptr)
        : RefPtr(obj, false)
    {
    }

    ~RefPtr()
    {
        release();
    }

    void set(ObjClass* obj)
    {
        assign(obj, false);
    }

    void take(ObjClass* obj)
    {
        assign(obj, true);
    }

    void release()
    {
        auto* tmp = m_obj;
        m_obj = nullptr;
        if (tmp)
            ObjRefUnrefHelper::unref(m_obj);
    }

    ObjClass* get() const { return m_obj; }

    operator bool() const { return m_obj != nullptr; }
    bool operator!() const { return m_obj == nullptr; }

    template<typename X>
    bool operator==(const X* other) const
    {
        return get() == other;
    }

    template<typename X, typename Y>
    bool operator==(const RefPtr<X, Y>& other) const
    {
        return get() == other.get();
    }

    template<typename X>
    bool operator!=(const X& anything) const
    {
        return !(*this == anything);
    }

    RefPtr(const RefPtr& other)
        : RefPtr(other.get())
    {
    }

    RefPtr(RefPtr&& other)
        : RefPtr(other.get(), true)
    {
        other.m_obj = nullptr;
    }

    RefPtr& operator=(const RefPtr& other)
    {
        assign(other.get(), false);
        return *this;
    }

    RefPtr& operator=(RefPtr&& other)
    {
        if (m_obj != other.m_obj)
        {
            assign(other.m_obj, false);
            other.m_obj = nullptr;
        }
        
        return *this;
    }

private:
    RefPtr(ObjClass* obj, bool newObject)
        : m_obj(nullptr)
    {
        assign(obj, newObject);
    }

    void assign(ObjClass* obj, bool newObject)
    {
        if (m_obj != obj)
        {
            ObjClass* tmp = m_obj;
            m_obj = obj;
            if (m_obj && !newObject)
                ObjRefUnrefHelper::ref(m_obj);
            if (tmp)
                ObjRefUnrefHelper::unref(tmp);
        }
    }

private:
    ObjClass* m_obj;
};

class GObjRefUnref
{
public:
    static void ref(gpointer obj) { g_object_ref(obj); }
    static void unref(gpointer obj) { g_object_unref(obj); }
};

template<typename GObjClass>
class GObjRefPtr : public RefPtr<GObjClass, GObjRefUnref>
{
    typedef RefPtr<GObjClass, GObjRefUnref> base;

public:
    explicit GObjRefPtr(GObjClass* obj = nullptr)
        : base(obj)
    {
    }

    GObject* gobj() const { return this->get() ? G_OBJECT(this->get()) : nullptr; }
    GTypeInstance* g_type_instance() const { return gobj() ? &gobj()->g_type_instance : nullptr; }
};

template<typename T, typename U>
auto find(const std::vector<T>& vec, const U& elm) -> decltype(vec.begin())
{
    return std::find(vec.begin(), vec.end(), elm);
}

template<typename T, typename U>
auto find(std::vector<T>& vec, const U& elm) -> decltype(vec.begin())
{
    return std::find(vec.begin(), vec.end(), elm);
}

template<typename T, typename U>
bool contains(const std::vector<T>& vec, const U& elm)
{
    return find(vec, elm) != vec.end();
}

template<typename T, typename U>
void remove(std::vector<T>& vec, const U& elm)
{
    auto itr = find(vec, elm);
    g_assert (itr != vec.end());
    vec.erase(itr);
}

class GObjectWrapper
{
public:
    GObjectWrapper(GObject* obj);

protected:
    GObjectWrapper(GObject* obj, bool takeReference);
    ~GObjectWrapper();

    GObjectWrapper(const GObjectWrapper&) = delete;
    GObjectWrapper& operator=(const GObjectWrapper&) = delete;

private:
    GObject* m_gobj;
    bool m_ownReference;
};

} // namespace moo
