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
#include <string>
#include <map>
#include <glib.h>
#include <mooutils/moocpp-refptr.h>

namespace moo {

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
    bool unref() const { mooCheck(refCount() > 0); return m_ref.unref(); }

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

template<typename T> struct CowWrapper;
template<typename T>
struct CowWrapper : public CowData<CowWrapper<T> >
{
    CowWrapper() : data() {}

    template<typename A1>
    CowWrapper(const A1 &a1) : data(a1) {}

    template<typename A1, typename A2>
    CowWrapper(const A1 &a1, const A2 &a2) : data(a1, a2) {}

    template<typename A1, typename A2, typename A3>
    CowWrapper(const A1 &a1, const A2 &a2, const A2 &a3) : data(a1, a2, a3) {}

    T data;
};

template<typename T>
class CowPtr
{
    typedef CowWrapper<T> TData;

public:
    CowPtr() : m_data(0)
    {
        TData *data = TData::getDefault();
        if (!data)
            data = new TData;
        m_data = data;
        m_data->ref();
        mooAssert(m_data->refCount() >= 1);
    }

    CowPtr(const CowPtr &cp)
        : m_data(0)
    {
        m_data = cp.m_data;
        m_data->ref();
        mooAssert(m_data->refCount() >= 2);
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
            mooAssert(m_data->refCount() >= 2);
        }
        return *this;
    }

    ~CowPtr()
    {
        mooAssert(m_data->refCount() > 0);
        if (m_data->unref())
            delete m_data;
    }

    template<typename A1>
    CowPtr(const A1 &a1)
        : m_data(new TData(a1))
    {
        m_data->ref();
        mooAssert(m_data->refCount() >= 1);
    }

    template<typename A1, typename A2>
    CowPtr(const A1 &a1, const A2 &a2)
        : m_data(new TData(a1, a2))
    {
        m_data->ref();
        mooAssert(m_data->refCount() >= 1);
    }

    template<typename A1, typename A2, typename A3>
    CowPtr(const A1 &a1, const A2 &a2, const A3 &a3)
        : m_data(new TData(a1, a2, a3))
    {
        m_data->ref();
        mooAssert(m_data->refCount() >= 1);
    }

    TData *get()
    {
        if (m_data->refCount() > 1)
        {
            TData *old_data = m_data;
            m_data = TData::dup(*old_data);
            m_data->ref();
            old_data->unref();
            mooAssert(m_data->refCount() > 0);
            mooAssert(old_data->refCount() > 0);
        }

        return m_data;
    }

    const TData *get() const { return m_data; }
    const TData *getConst() const { return m_data; }

    void swap(CowPtr &other)
    {
        std::swap(m_data, other.m_data);
    }

    MOO_IMPLEMENT_POINTER_TO_MEM(TData, get())

private:
    TData *m_data;
};

} // namespace util


template<typename T>
class Vector
{
private:
    util::CowPtr<std::vector<T> > m_impl;
    std::vector<T> &v() { return m_impl->data; }
    const std::vector<T> &v() const { return m_impl->data; }

public:
    class iterator : public std::vector<T>::iterator
    {
    private:
        typedef typename std::vector<T>::iterator std_iterator;
        typedef typename std::vector<T>::const_iterator std_const_iterator;

    public:
        iterator() : std_iterator() {}
        iterator(const std_iterator &si) : std_iterator(si) {}
        iterator &operator=(const std_iterator &si) { std_iterator::operator=(si); return *this; }
    };

    class const_iterator : public std::vector<T>::const_iterator
    {
    private:
        typedef typename std::vector<T>::iterator std_iterator;
        typedef typename std::vector<T>::const_iterator std_const_iterator;

    public:
        const_iterator() : std_const_iterator() {}
        const_iterator(const std_const_iterator &si) : std_const_iterator(si) {}
        const_iterator &operator=(const std_const_iterator &si) { std_const_iterator::operator=(si); return *this; }
    };

    inline void checkNonEmpty() const { mooThrowIfFalse(!empty()); }
    inline void checkIndex(int i) const { mooThrowIfFalse((i) < size()); }
    inline void checkIndexE(int i) const { mooThrowIfFalse((i) <= size()); }

public:
    Vector() {}

    template<typename U>
    explicit Vector(const Vector<U> &v)
    {
        insert(end(), v.begin(), v.end());
    }

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

    bool empty() const { return v().empty(); }
    int size() const { return v().size(); }

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
            std::pair<const_iterator, const_iterator> itr = std::mismatch(begin(), end(), other.begin());
            return itr.first == end();
        }
        else
        {
            return false;
        }
    }

    bool operator!=(const Vector &other) const { return !(*this == other); }
};


class String
{
public:
    String() {}
    String(const char *str) { if (str && *str) s() = validateUtf8(str); }
    String(const char *str, gsize len) { if (len != 0) s().assign(validateUtf8(str, len), len); }
    String(gsize len, char c) : m_impl(len, validateUtf8(c)) {}
    String(char c) : m_impl(1, validateUtf8(c)) {}
    ~String() {}

    String(const std::string &ss) : m_impl(validateUtf8(ss)) {}
    String &operator=(const std::string &ss) { s() = validateUtf8(ss); return *this; }
    String &operator=(const char *str) { s() = str ? validateUtf8(str) : ""; return *this; }

    operator const char* () const { return utf8(); }
    const char *utf8() const { return s().c_str(); }

    bool empty() const { return s().empty(); }
    void clear() { if (!empty()) s().clear(); }

    bool operator==(const String &other) const { return s() == other.s(); }
    bool operator!=(const String &other) const { return s() != other.s(); }
    bool operator< (const String &other) const { return s() <  other.s(); }
    bool operator> (const String &other) const { return s() >  other.s(); }
    bool operator<=(const String &other) const { return s() <= other.s(); }
    bool operator>=(const String &other) const { return s() >= other.s(); }

    String operator+(const String &other) const { if (!other.empty()) { return String(s() + other.s(), true); } else { return *this; } }
    String operator+(const char *str) const { if (str && *str) return String(s() + validateUtf8(str), true); else return *this; }
    String operator+(char c) const { return String(s() + validateUtf8(c), true); }
    String &operator+=(const String &other) { if (!other.empty()) s() += other.s(); return *this; }
    String &operator+=(const char *str) { if (str && *str) { s() += validateUtf8(str); } return *this; }
    String &operator+=(char c) { s() += validateUtf8(c); return *this; }

    template<typename T>
    String &append(const T &t) { return *this += t; }

    void swap(String &other) { m_impl.swap(other.m_impl); }

private:
    String(const std::string &ss, bool /*nCheck*/) : m_impl(ss) {}

    static const std::string &validateUtf8(const std::string &ss) { mooThrowIfFalse(g_utf8_validate(ss.c_str(), ss.size(), NULL)); return ss; }
    static std::string &validateUtf8(std::string &ss) { mooThrowIfFalse(g_utf8_validate(ss.c_str(), ss.size(), NULL)); return ss; }
    static const char *validateUtf8(const char *str, int len = -1) { mooThrowIfFalse(g_utf8_validate(str, len, NULL)); return str; }
    static char validateUtf8(char c) { mooThrowIfFalse(!((guchar)c & 0x80)); return c; }

private:
    util::CowPtr<std::string> m_impl;
    std::string &s() { return m_impl->data; }
    const std::string &s() const { return m_impl->data; }
    const std::string &cs() const { return m_impl->data; }
};

} // namespace moo

inline moo::String operator+(const std::string &ss, const moo::String &ms) { return moo::String(ss) + ms; }
inline moo::String operator+(const char *str, const moo::String &ms) { return moo::String(str) + ms; }
inline moo::String operator+(char c, const moo::String &ms) { return moo::String(c) + ms; }

namespace moo {

namespace impl {

class WeakPtrBase
{
public:
    WeakPtrBase() : m_p(0) {}
    ~WeakPtrBase() { mooAssert(m_p == 0); }

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
        mooCheckCanCast(T, U);
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
        mooCheckCanCast(U, T);
        T *p = wp.get();
        set(p);
    }

    template<class U, class V>
    void set(const SharedPtr<U, V> &sp)
    {
        mooCheckCanCast(U, T);
        T *p = sp.get();
        set(p);
    }

    template<class U, class V>
    void set(const OwningPtr<U, V> &op)
    {
        mooCheckCanCast(U, T);
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
        mooCheckCanCast(U, T);
        mooAssert(!m_ptrs.contains(&wp));
        m_ptrs.append(&wp);
    }

    template<class U>
    void removeWeakPtr(WeakPtr<U> &wp)
    {
        mooCheckCanCast(U, T);
        mooAssert(m_ptrs.contains(&wp));
        m_ptrs.removeAll(&wp);
    }

private:
    MOO_DISABLE_COPY_AND_ASSIGN(WeakRefd)

private:
    Vector<impl::WeakPtrBase*> m_ptrs;
};


template<typename Key, typename Value>
class Dict
{
private:
    typedef typename std::map<Key, Value> std_map;
    typedef typename std_map::iterator std_iterator;
    typedef typename std_map::const_iterator std_const_iterator;
    typedef typename std_map::value_type std_value_type;

    util::CowPtr<std_map> m_impl;
    std_map &m() { return m_impl->data; }
    const std_map &m() const { return m_impl->data; }

public:
    class iterator;
    class const_iterator;
    friend class iterator;
    friend class const_iterator;

    class iterator : public std_iterator
    {
    public:
        iterator() : std_iterator() {}
        iterator(const std_iterator &si) : std_iterator(si) {}
        iterator &operator=(const std_iterator &si) { std_iterator::operator=(si); return *this; }

        const Key &key() const { return static_cast<const std_iterator&>(*this)->first; }
        Value &value() const { return static_cast<const std_iterator&>(*this)->second; }
    };

    class const_iterator : public std_const_iterator
    {
    public:
        const_iterator() : std_const_iterator() {}
        const_iterator(const std_const_iterator &si) : std_const_iterator(si) {}
        const_iterator &operator=(const std_const_iterator &si) { std_const_iterator::operator=(si); return *this; }
        const_iterator(const iterator &iter) : std_const_iterator(iter) {}
        const_iterator &operator=(const iterator &iter) { std_const_iterator::operator=(iter); return *this; }

        const Key &key() const { return static_cast<const std_const_iterator&>(*this)->first; }
        const Value &value() const { return static_cast<const std_const_iterator&>(*this)->second; }
    };

public:
    Dict() {}
    ~Dict() {}

    iterator begin() { return m().begin(); }
    const_iterator begin() const { return m().begin(); }
    const_iterator cbegin() const { return m().begin(); }
    iterator end() { return m().end(); }
    const_iterator end() const { return m().end(); }
    const_iterator cend() const { return m().end(); }

    bool empty() const { return m().empty(); }
    int size() const { return m().size(); }

    Value &operator[] (const Key &key) { return m()[key]; }

    std::pair<iterator, bool> insert(const Key &key, const Value &value)
    {
        return m().insert(std_value_type(key, value));
    }

    template <class InputIterator>
    void insert(InputIterator first, InputIterator last)
    {
        m().insert(first, last);
    }

    void erase(iterator pos) { m().erase(pos); }
    int erase(const Key &key) { return m().erase(key); }

    template<typename T>
    void remove(const T &t) { erase(t); }

    void swap(Dict &other) { m_impl.swap(other.m_impl); }

    void clear() { m().clear(); }

    iterator find(const Key &key) { return m().find(key); }
    const_iterator find(const Key &key) const { return m().find(key); }
    const_iterator cfind(const Key &key) const { return m().find(key); }

    bool contains(const Key &key) const { return find(key) != end(); }

    const Value &value(const Key &key, const Value &value = Value()) const
    {
        const_iterator iter = find(key);
        if (iter != end())
            return iter->second;
        else
            return value;
    }
};

} // namespace moo

#endif /* MOO_CPP_CONT_H */
/* -%- strip:true -%- */
