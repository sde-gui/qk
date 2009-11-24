/*
 *   moocpp-cont.h
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

#ifndef MOO_CPP_CONT_H
#define MOO_CPP_CONT_H

#include <algorithm>
#include <vector>
#include <mooutils/moocpp-macros.h>

namespace moo {

#ifdef MOO_USE_EXCEPTIONS
#define MOO_CONTAINER_CHECK(what)       \
do {                                    \
    if (what)                           \
        ;                               \
    else                                \
        throw std::runtime_error();     \
} while (0)
#else
#define MOO_CONTAINER_CHECK mCheck
#endif

namespace util {

template<typename T>
class CowData
{
public:
    CowData() : m_ref(0) {}
    CowData(const CowData&) : m_ref(0) {}
    virtual ~CowData() {}

    int refCount() const { return m_ref; }
    void ref() const { m_ref.ref(); }
    bool unref() const { mCheck(refCount() > 0); return m_ref.unref(); }

    static T *getDefault()
    {
        static T obj;
        if (obj.refCount() == 0)
            obj.ref();
        return &obj;
    }

    static T *dup(const T &data) { return new T(data); }

private:
    CowData &operator=(const CowData&);

private:
    mutable RefCount m_ref;
};

template<typename TData>
class CowPtr
{
public:
    CowPtr(TData *data = 0)
        : m_data(0)
    {
        if (!data)
            data = TData::getDefault();
        if (!data)
            data = new TData;
        m_data = data;
        m_data->ref();
        mAssert(m_data->refCount() >= 1);
    }

    CowPtr(const CowPtr &cp)
        : m_data(0)
    {
        m_data = cp.m_data;
        m_data->ref();
        mAssert(m_data->refCount() >= 2);
    }

    CowPtr &operator=(const CowPtr &cp)
    {
        if (m_data != cp.m_data)
        {
            TData *data = m_data;
            m_data = cp.m_data;
            m_data->ref();
            if (data->unref())
                delete data;
            mAssert(m_data->refCount() >= 2);
        }
        return *this;
    }

    ~CowPtr()
    {
        mAssert(m_data->refCount() > 0);
        if (m_data->unref())
            delete m_data;
    }

    TData *get()
    {
        if (m_data->refCount() > 1)
        {
            TData *old_data = m_data;
            m_data = TData::dup(*old_data);
            m_data->ref();
            old_data->unref();
            mAssert(m_data->refCount() > 0);
            mAssert(old_data->refCount() > 0);
        }

        return m_data;
    }

    const TData *get() const { return m_data; }
    const TData *getConst() const { return m_data; }

    MOO_IMPLEMENT_POINTER_TO_MEM(TData, get())

private:
    TData *m_data;
};

} // namespace util


template<typename T>
class Vector
{
private:
    typedef typename std::vector<T> std_vector;
    typedef typename std::vector<T>::iterator std_iterator;
    typedef typename std::vector<T>::const_iterator std_const_iterator;

    struct Impl : public util::CowData<Impl>
    {
        std_vector v;
    };

    util::CowPtr<Impl> m_impl;

    std_vector &v() { return m_impl->v; }
    const std_vector &v() const { return m_impl->v; }

public:
    class iterator;
    class const_iterator;
    friend class iterator;
    friend class const_iterator;

    class iterator
    {
    private:
        friend class Vector;
        typedef Vector::std_iterator std_iterator;
        typedef Vector::std_const_iterator std_const_iterator;

        std_iterator m_si;

        iterator(const std_iterator &si) : m_si(si) {}

        operator std_iterator&() { return m_si; }
        operator const std_iterator&() const { return m_si; }

    public:
        T &operator*() const { return *m_si; }
    };

    class const_iterator
    {
    private:
        friend class Vector;
        typedef Vector::std_iterator std_iterator;
        typedef Vector::std_const_iterator std_const_iterator;

        std_const_iterator m_si;

        const_iterator(const std_iterator &si) : m_si(si) {}
        const_iterator(const std_const_iterator &si) : m_si(si) {}

        operator std_const_iterator&() { return m_si; }
        operator const std_const_iterator&() const { return m_si; }

    public:
        const T &operator*() const { return *m_si; }
    };

    inline void checkNonEmpty() const { MOO_CONTAINER_CHECK(!empty()); }
    inline void checkIndex(int i) const { MOO_CONTAINER_CHECK((i) < size()); }
    inline void checkIndexE(int i) const { MOO_CONTAINER_CHECK((i) <= size()); }

public:
    Vector() {}

    iterator begin() { return v().begin(); }
    const_iterator begin() const { return v().begin(); }
    iterator end() { return v().end(); }
    const_iterator end() const { return v().end(); }
    const_iterator constBegin() const { return begin(); }
    const_iterator constEnd() const { return end(); }

    T &at(int i) { checkIndex(i); return v().at(i); }
    const T &at(int i) const { checkIndex(i); return v().at(i); }

    T &operator[](int i) { return at(i); }
    const T &operator[](int i) const { return at(i); }

    T &first() { checkNonEmpty(); return at(0); }
    const T &first() const { checkNonEmpty(); return at(0); }
    T &last() { checkNonEmpty(); return at(size() - 1); }
    const T &last() const { checkNonEmpty(); return at(size() - 1); }
    T &back() { checkNonEmpty(); return last(); }
    const T &back() const { checkNonEmpty(); return last(); }
    T &front() { checkNonEmpty(); return first(); }
    const T &front() const { checkNonEmpty(); return first(); }

    Vector mid(int pos, int length = -1) const;

    T value(int i) const { return value(i, T()); }
    T value(int i, const T &defaultValue) const { return i >= 0 && i < size() ? v()[i] : defaultValue; }

    bool contains(const T &value) const { return indexOf(value) >= 0; }
    bool endsWith(const T &value) const { int c = v().size(); return c > 0 && v()[c - 1] == value; }
    int indexOf(const T &value, int from = 0) const { for (int i = from, c = v().size(); i < c; ++i) if (v()[i] == value) return i; return -1; }
    int lastIndexOf(const T &value, int from = -1) const { for (int c = v().size(), i = (from >= 0 ? from : c - 1); i >= 0 && i < c; --i) if (v()[i] == value) return i; return -1; }
    bool startsWith(const T &value) const { return v().size() > 0 && v()[0] == value; }
    int count(const T &value) const { return std::count(v().begin(), v().end(), value); }

    int count() const { return v().size(); }
    bool isEmpty() const { return v().size() == 0; }

    bool empty() const { return isEmpty(); }
    int length() const { return count(); }
    int size() const { return count(); }

    void append(const T &value) { v().push_back(value); }
    void append(const Vector &other) { v().insert(v().end(), other.v().begin(), other.v().end()); }
    void clear() { v().erase(v().begin(), v().end()); }
    iterator erase(iterator pos) { v().erase(pos); }
    iterator erase(iterator begin, iterator end) { v().erase(begin, end);  }

    void insert(int i, const T &value) { checkIndexE(i); v().insert(v().begin() + i, value); }
    iterator insert(iterator before, const T &value) { v().insert(before, value); }
    template<class InputIterator>
    void insert(iterator before, InputIterator first, InputIterator last) { return v().insert(before, first, last); }

    void pop_back() { checkNonEmpty(); v().pop_back(); }
    void pop_front() { checkNonEmpty(); v().pop_front(); }
    void prepend(const T &value) { v().push_front(value); }
    void push_back(const T &value) { v().push_back(value); }
    void push_front(const T &value) { v().push_front(value); }
    int removeAll(const T &value)
    {
        int sizeBefore = v().size();
        if (sizeBefore > 0)
            std::remove(v().begin(), v().end(), value);
        return sizeBefore - v().size();
    }
    void removeAt(int i) { checkIndex(i); v().erase(v().begin() + i); }
    void removeFirst() { checkNonEmpty(); v().erase(v().begin()); }
    void removeLast() { checkNonEmpty(); v().erase(v().end() - 1); }
    bool removeOne(const T &value) { int idx = indexOf(value); if (idx >= 0) { removeAt(idx); return true; } else { return false; } }
    void replace(int i, const T &value) { checkIndex(i); (*this)[i] = value; }
    void swap(int i, int j) { checkIndex(i); checkIndex(j); std::swap((*this)[i], (*this)[j]); }

    T takeAt(int i) { T value = value(i); removeAt(i); return value; }
    T takeFirst() { return takeAt(0); }
    T takeLast() { return takeAt(size() - 1); }

    Vector operator+(const Vector &other) const { Vector result = *this; result += other; return result; }
    Vector &operator+=(const Vector &other) { append(other); return *this; }
    Vector &operator+=(const T &value) { append(value); return *this; }
    Vector &operator<<(const Vector &other) { append(other); return *this; }
    Vector &operator<<(const T &value) { append(value); return *this; }

    bool operator==(const Vector &other) const
    {
        if (size() == other.size())
        {
            std::pair<std_const_iterator, std_const_iterator> itr = std::mismatch(v().begin(), v().end(), other.v().begin());
            return itr.first == v().end();
        }
        else
        {
            return false;
        }
    }

    bool operator!=(const Vector &other) const { return !(*this == other); }
};


} // namespace moo

#endif /* MOO_CPP_CONT_H */
/* -%- strip:true -%- */
