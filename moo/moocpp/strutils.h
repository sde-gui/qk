/*
 *   moocpp/strutils.h
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

#ifdef __cplusplus

#include <mooglib/moo-glib.h>
#include <moocpp/memutils.h>
#include <moocpp/utils.h>
#include <algorithm>
#include <utility>
#include <functional>

namespace moo {

// Replacement for raw char*
class strp
{
public:
    strp() : m_p(nullptr) {}
    ~strp() { ::g_free(m_p); }

    strp(strp&& s) : strp() { std::swap(m_p, s.m_p); }
    strp& operator=(strp&& s) { std::swap(m_p, s.m_p); }

    strp(const strp&) = delete;
    strp& operator=(const strp&) = delete;

    char* release() { char *p = m_p; m_p = nullptr; return p; }
    const char* get() { return m_p; }

    char*& p() { return m_p; }
    char** pp() { return &m_p; }
    operator char*() = delete;
    char** operator&() = delete;

    operator bool() const { return m_p != nullptr; }
    bool operator !() const { return m_p == nullptr; }

private:
    char* m_p;
};

template<typename T>
struct mg_get_string
{
    static const char* get_string(const T& obj) { return static_cast<const char*>(obj); }
};

template<typename Self, typename GetString = mg_get_string<Self>>
class gstr_methods_mixin
{
public:
    char *strdup() const { return g_strdup(c_str()); }

    bool empty() const { const char* s = c_str(); return !s || !*s; }

    // These must not be called, to avoid ambiguity between an empty string and null
    operator bool() const = delete;
    bool operator!() const = delete;

private:
    Self& self() { return static_cast<Self&>(*this); }
    const Self& self() const { return static_cast<const Self&>(*this); }
    const char* c_str() const { return GetString::get_string(static_cast<const Self&>(*this)); }
};

class gstr : public gstr_methods_mixin<gstr>
{
public:
    gstr();
    ~gstr();
    gstr(const char* s, mem_transfer mt);

    gstr(const gstr&);
    gstr& operator=(const gstr&);
    gstr(gstr&&);
    gstr& operator=(gstr&&);

    gstr(nullptr_t) : gstr() {}
    gstr& operator=(nullptr_t) { clear(); return *this; }

    void set(const gstr& s) = delete;
    static gstr wrap(const gstr& s) = delete;

    void set(const char *s)                 { assign(s, mem_transfer::make_copy); }
    void set_new(char *s)                   { assign(s, mem_transfer::take_ownership); }
    void set_const(const char *s)           { assign(s, mem_transfer::borrow); }
    static gstr wrap(const char *s)         { return gstr(s, mem_transfer::make_copy); }
    static gstr wrap_new(char *s)           { return gstr(s, mem_transfer::take_ownership); }
    static gstr wrap_const(const char *s)   { return gstr(s, mem_transfer::borrow); }

    bool is_null() const;

    operator const char*() const;
    const char* get() const { return static_cast<const char*>(*this); }

    char* get_mutable();
    char* release_owned();
    void clear();
    void reset() { clear(); }

private:
    void assign(const char* s, mem_transfer mt);

private:
    void *m_p; // either char* or Data*
    bool m_is_inline;
    bool m_is_const;
};


bool operator==(const gstr& s1, const char* s2);
bool operator==(const char* s1, const gstr& s2);
bool operator==(const gstr& s1, const gstr& s2);
bool operator!=(const gstr& s1, const char* s2);
bool operator!=(const char* s1, const gstr& s2);
bool operator!=(const gstr& s1, const gstr& s2);


class gerrp
{
public:
    explicit gerrp(GError** errp = nullptr) : m_errp(errp ? errp : &m_local), m_local(nullptr) {}

    ~gerrp()
    {
        if (m_errp != &m_local)
            clear();
    }

    operator bool() const { return (*m_errp) != nullptr; }
    bool operator!() const { return (*m_errp) == nullptr; }

    GError* get() const { return (*m_errp); }
    GError* operator->() const { return (*m_errp); }
    GError** operator&() { return m_errp; }

    //void propagate(GError** dest) { g_propagate_error(dest, m_err); m_err = nullptr; }
    void clear() { if (*m_errp) g_error_free(*m_errp); *m_errp = nullptr; m_local = nullptr; }

    MOO_DISABLE_COPY_OPS(gerrp);

    gerrp(gerrp&& other) = delete;

    gerrp& operator=(gerrp&& other)
    {
        clear();
        if (other)
            g_propagate_error (m_errp, other.get());
        other.m_errp = &other.m_local;
        other.m_local = nullptr;
        return *this;
    }

private:
    GError** m_errp;
    GError* m_local;
};


} // namespace moo

namespace std {

template<>
struct hash<moo::gstr>
{
    const size_t operator()(const moo::gstr& s) const;
};

} // namespace std

#endif // __cplusplus
