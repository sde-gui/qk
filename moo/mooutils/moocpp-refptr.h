/*
 *   moocpp-refptr.h
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

#ifndef MOO_CPP_REFPTR_H
#define MOO_CPP_REFPTR_H

#include <mooutils/moocpp-types.h>

namespace moo {

template<class T>
class RefCounted
{
protected:
    RefCounted() : m_ref(1) {}
    virtual ~RefCounted() {}

public:
    void ref()
    {
        m_ref.ref();
    }

    void unref()
    {
        if (m_ref.unref())
            delete this;
    }

private:
    MOO_DISABLE_COPY_AND_ASSIGN(RefCounted)

private:
    RefCount m_ref;
};

namespace impl
{

template<class T>
class DeleteObject
{
public:
    void operator() (T *p) { delete p; }

    template<class U>
    operator DeleteObject<U>& () { mooCheckCanCast(T, U); return *reinterpret_cast<DeleteObject<U>*>(this); }
};

template<>
inline void DeleteObject<char>::operator() (char *p) { g_free(p); }

} // namespace impl

template<class T>
class Pointer
{
public:
    virtual ~Pointer() {}

    virtual T *get() const = 0;

    operator T* () const { return get(); }
};

template<class T, class TDeleter = impl::DeleteObject<T> >
class OwningPtr : public Pointer<T>
{
public:
    OwningPtr(T *p = 0) : m_p(p) {}
    ~OwningPtr() { unset(); }

    OwningPtr(OwningPtr &op) : m_p(0) { set(op); }
    OwningPtr &operator=(OwningPtr &op) { set(op); }

    template<class U> OwningPtr(U &u) : m_p(0) { set(u); }
    template<class U> OwningPtr &operator=(U &u) { set(u); return *this; }

    template<class U, class V>
    operator OwningPtr<U, V>& ()
    {
        mooCheckCanCast(T, U);
        mooCheckCanCast(TDeleter, V);
        return *reinterpret_cast<OwningPtr<U, V>*>(this);
    }

    template<class U, class V>
    operator const OwningPtr<U, V>& () const
    {
        return const_cast<OwningPtr&>(this)->operator OwningPtr<U, V>&();
    }

    MOO_IMPLEMENT_POINTER(T, m_p)
    MOO_IMPLEMENT_BOOL(m_p)

    template<class U>
    void set(U *pu)
    {
        T *p = pu;
        if (m_p != p)
        {
            T *old = m_p;
            m_p = p;
            TDeleter()(old);
        }
    }

    template<class U, class V>
    void set(OwningPtr<U, V> &op)
    {
        mooCheckCanCast(U, T);
        T *p = op.steal();
        set(p);
    }

    void unset()
    {
        T *p = m_p;
        m_p = 0;
        TDeleter()(p);
    }

    virtual T *get() const { return m_p; }
    T *steal() { T *p = m_p; m_p = 0; return p; }

private:
    T *m_p;
};

namespace impl
{

template<class T>
class RefUnrefObject
{
public:
    static void ref(T *p) { p->ref(); }
    static void unref(T *p) { p->unref(); }

    template<class U>
    operator RefUnrefObject<U>& () { mooCheckCanCast(T, U); return *reinterpret_cast<RefUnrefObject<U>*>(this); }
};

}

template<class T, class TRefUnref = impl::RefUnrefObject<T> >
class SharedPtr : public Pointer<T>
{
private:
    inline static void ref(T *p) { if (p) TRefUnref::ref(p); }
    inline static void unref(T *p) { if (p) TRefUnref::unref(p); }

public:
    static SharedPtr take(T *p) { SharedPtr sp(p); unref(p); return sp; }

    SharedPtr(T *p = 0) : m_p(0) { set(p); }
    ~SharedPtr() { unset(); }

    SharedPtr(const SharedPtr &sp) : m_p(0) { set(sp.m_p); }
    SharedPtr &operator=(const SharedPtr &sp) { set(sp.m_p); return *this; }

    template<class U> SharedPtr(const U &u) : m_p(0) { set(u); }
    template<class U> SharedPtr &operator=(const U &u) { set(u); return *this; }

    template<class U, class V>
    operator SharedPtr<U, V>& ()
    {
        mooCheckCanCast(T, U);
        mooCheckCanCast(TRefUnref, V);
        return *reinterpret_cast<SharedPtr<U, V>*>(this);
    }

    template<class U, class V>
    operator const SharedPtr<U, V>& () const
    {
        return const_cast<SharedPtr*>(this)->operator SharedPtr<U, V>&();
    }

    MOO_IMPLEMENT_POINTER(T, m_p)
    MOO_IMPLEMENT_BOOL(m_p)

    template<class U, class V>
    void set(const SharedPtr<U, V> &sp)
    {
        mooCheckCanCast(U, T);
        set(sp.get());
    }

    template<class U, class V>
    void set(const OwningPtr<U, V> &op)
    {
        mooCheckCanCast(U, T);
        set(op.get());
    }

    template<class U>
    void set(U *pu)
    {
        if (m_p != pu)
        {
            T *old = m_p;
            m_p = pu;
            ref(m_p);
            unref(old);
        }
    }

    void unset()
    {
        if (m_p != 0)
        {
            T *p = m_p;
            m_p = 0;
            unref(p);
        }
    }

    virtual T *get() const { return m_p; }
    T *steal() const { T *p = m_p; m_p = 0; return p; }

private:
    T *m_p;
};

} // namespace moo

#endif /* MOO_CPP_REFPTR_H */
/* -%- strip:true -%- */
