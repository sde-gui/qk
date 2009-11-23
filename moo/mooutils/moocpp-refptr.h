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
#include <mooutils/moocpp-cont.h>

namespace moo {

template<class T> class List;

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
    operator DeleteObject<U>& () { mCanCast(T, U); return *reinterpret_cast<DeleteObject<U>*>(this); }
};

} // namespace impl

template<class T, class TDeleter = impl::DeleteObject<T> >
class OwningPtr
{
public:
    OwningPtr(T *p = 0) : m_p(p) {}
    ~OwningPtr() { unset(); }

    OwningPtr(const OwningPtr &op) : m_p(0) { set(op); }
    OwningPtr &operator=(const OwningPtr &op) { set(op); }

    template<class U> OwningPtr(const U &u) : m_p(0) { set(u); }
    template<class U> OwningPtr &operator=(const U &u) { set(u); return *this; }

    template<class U, class V>
    operator OwningPtr<U, V>& ()
    {
        mCanCast(T, U);
        mCanCast(TDeleter, V);
        return *reinterpret_cast<OwningPtr<U, V>*>(this);
    }

    template<class U, class V>
    operator const OwningPtr<U, V>& () const
    {
        return const_cast<OwningPtr*>(this)->operator OwningPtr<U, V>&();
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
    void set(const OwningPtr<U, V> &op)
    {
        mCanCast(U, T);
        T *p = op.steal();
        set(p);
    }

    void unset()
    {
        T *p = m_p;
        m_p = 0;
        TDeleter()(p);
    }

    T *get() const { return m_p; }
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
    operator RefUnrefObject<U>& () { mCanCast(T, U); return *reinterpret_cast<RefUnrefObject<U>*>(this); }
};

}

template<class T, class TRefUnref = impl::RefUnrefObject<T> >
class SharedPtr
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
        mCanCast(T, U);
        mCanCast(TRefUnref, V);
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
        mCanCast(U, T);
        set(sp.get());
    }

    template<class U, class V>
    void set(const OwningPtr<U, V> &op)
    {
        mCanCast(U, T);
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

    T *get() const { return m_p; }
    T *steal() const { T *p = m_p; m_p = 0; return p; }

private:
    T *m_p;
};

namespace impl {

class WeakPtrBase
{
public:
    WeakPtrBase() : m_p(0) {}
    ~WeakPtrBase() { mAssert(m_p == 0); }

    void unsetPtrNoNotify()
    {
        m_p = 0;
    }

protected:
    void set(void *p) { m_p = p; }

protected:
    void *m_p;
};

} // namespace impl

template<class T>
class WeakPtr : public impl::WeakPtrBase
{
public:
    WeakPtr(T *p = 0) { set(p); }
    ~WeakPtr() { unset(); }

    WeakPtr(const WeakPtr &op) { set(op); }
    WeakPtr &operator=(const WeakPtr &op) { set(op); }

    template<class U> WeakPtr(const U &u) { set(u); }
    template<class U> WeakPtr &operator=(const U &u) { set(u); return *this; }

    template<class U>
    operator WeakPtr<U>& ()
    {
        mCanCast(T, U);
        return *reinterpret_cast<WeakPtr<U>*>(this);
    }

    template<class U>
    operator const WeakPtr<U>& () const
    {
        return const_cast<WeakPtr*>(this)->operator WeakPtr<U>&();
    }

    MOO_IMPLEMENT_POINTER(T, get())
    MOO_IMPLEMENT_BOOL(get())

    template<class U>
    void set(U *u)
    {
        T *p = u;
        T *old = get();
        if (old != p)
        {
            if (old)
                old->removeWeakPtr(*this);
            m_p = p;
            if (p)
                p->addWeakPtr(*this);
        }
    }

    template<class U>
    void set(const WeakPtr<U> &wp)
    {
        mCanCast(U, T);
        T *p = wp.get();
        set(p);
    }

    template<class U, class V>
    void set(const SharedPtr<U, V> &sp)
    {
        mCanCast(U, T);
        T *p = sp.get();
        set(p);
    }

    template<class U, class V>
    void set(const OwningPtr<U, V> &op)
    {
        mCanCast(U, T);
        T *p = op.get();
        set(p);
    }

    void unset()
    {
        T *old = get();
        if (old)
            old->removeWeakPtr(*this);
        m_p = 0;
    }

    T *get() const { return static_cast<T*>(m_p); }
};

template<class T>
class WeakRefd
{
protected:
    WeakRefd()
    {
    }

    virtual ~WeakRefd()
    {
        notify();
    }

    void notify()
    {
        for (int i = 0, c = m_ptrs.size(); i < c; ++i)
            m_ptrs[i]->unsetPtrNoNotify();
        m_ptrs.clear();
    }

public:
    template<class U>
    void addWeakPtr(WeakPtr<U> &wp)
    {
        mCanCast(U, T);
        mAssert(!m_ptrs.contains(&wp));
        m_ptrs.append(&wp);
    }

    template<class U>
    void removeWeakPtr(WeakPtr<U> &wp)
    {
        mCanCast(U, T);
        mAssert(m_ptrs.contains(&wp));
        m_ptrs.removeAll(&wp);
    }

private:
    MOO_DISABLE_COPY_AND_ASSIGN(WeakRefd)

private:
    List<impl::WeakPtrBase*> m_ptrs;
};

} // namespace moo

#endif /* MOO_CPP_REFPTR_H */
/* -%- strip:true -%- */
