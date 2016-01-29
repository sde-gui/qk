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

#include <mooglib/moo-glib.h>
#include <moocpp/memutils.h>
#include <moocpp/utils.h>
#include <algorithm>
#include <utility>
#include <functional>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <stdarg.h>

namespace moo {

// Replacement for raw char*
class gstrp : public gbuf<char>
{
public:
    gstrp(char* p = nullptr) : gbuf(p) {}

    gstrp(gstrp&& s) : gbuf(std::move(s)) {}
    gstrp& operator=(gstrp&& s) { static_cast<gbuf<char>&>(*this) = std::move(s); return *this; }

    char*& p() { return _get(); }
    char** pp() { return &_get(); }

    MOO_DISABLE_COPY_OPS(gstrp);

private:
    char* m_p;
};

class gstr
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
    void set_const(const gstr& s) = delete;
    void set_new(const gstr& s) = delete;
    static gstr wrap(const gstr& s) = delete;
    static gstr wrap_const(const gstr& s) = delete;
    static gstr wrap_new(const gstr& s) = delete;

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

    char *strdup() const { return g_strdup(*this); }
    bool empty() const { const char* s = *this; return !s || !*s; }

    // These must not be called, to avoid ambiguity between an empty string and null
    operator bool() const = delete;
    bool operator!() const = delete;

private:
    void assign(const char* s, mem_transfer mt);

private:
    void *m_p; // either char* or Data*
    bool m_is_inline;
    bool m_is_const;
};

using gstrvec = std::vector<gstr>;
using gstrset = std::unordered_set<gstr>;
using gstrmap = std::unordered_map<gstr, gstr>;

bool operator==(const gstr& s1, const char* s2);
bool operator==(const char* s1, const gstr& s2);
bool operator==(const gstr& s1, const gstr& s2);
bool operator!=(const gstr& s1, const char* s2);
bool operator!=(const char* s1, const gstr& s2);
bool operator!=(const gstr& s1, const gstr& s2);


class gstrv
{
public:
    gstrv(char** p = nullptr) : m_p(p) {}
    ~gstrv() { ::g_strfreev(m_p); }

    void set(char** p) { if (m_p != p) { ::g_strfreev(m_p); m_p = p; } }
    void reset(char** p = nullptr) { set(p); }
    char** get() const { return m_p; }

    static gstrv convert(gstrvec v);

    gsize size() const { return m_p ? g_strv_length (m_p) : 0; }

    operator char**() const = delete;
    char*** operator&() = delete;

    char** release() { char** p = m_p; m_p = nullptr; return p; }

    const char* operator[] (gsize i) const { return m_p[i]; }

    MOO_DISABLE_COPY_OPS(gstrv);

    gstrv(gstrv&& other) : gstrv() { *this = std::move(other); }
    gstrv& operator=(gstrv&& other) { std::swap(m_p, other.m_p); return *this; }

    gstrv& operator=(char** p) { set(p); return *this; }

    bool operator==(nullptr_t) const { return m_p == nullptr; }
    bool operator!=(nullptr_t) const { return m_p != nullptr; }
    operator bool() const { return m_p != nullptr; }
    bool operator !() const { return m_p == nullptr; }

private:
    char** m_p;
};

gstrvec convert(gstrv v);

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


class strbuilder
{
public:
    strbuilder(const char *init = nullptr, gssize len = -1);
    strbuilder(gsize reserve);
    ~strbuilder();

    gstrp release();
    const char* get() const;

    MOO_DISABLE_COPY_OPS(strbuilder);

    void truncate(gsize len);
    void set_size(gsize len);
    void append(const char* val, gssize len = -1);
    void append(char c);
    void append(gunichar wc);
    void prepend(const char* val, gssize len = -1);
    void prepend(char c);
    void prepend(gunichar wc);
    void insert(gssize pos, const char* val, gssize len = -1);
    void insert(gssize pos, char c);
    void insert(gssize pos, gunichar wc);
    void overwrite(gsize pos, const char* val, gssize len = -1);
    void erase(gssize pos = 0, gssize len = -1);
    void ascii_down();
    void ascii_up();
    void vprintf(const char* format, va_list args);
    void printf(const char* format, ...) G_GNUC_PRINTF(1, 2);
    void append_vprintf(const char* format, va_list args);
    void append_printf(const char* format, ...) G_GNUC_PRINTF(1, 2);
    void append_uri_escaped(const char* unescaped, const char* reserved_chars_allowed = nullptr, bool allow_utf8 = false);

private:
    mutable GString* m_buf;
    mutable gstrp m_result;
};

//gstrvec convert(gstrv v);

} // namespace moo

void g_free(const moo::gstr&) = delete;
void g_free(const moo::gstrp&) = delete;
void g_free(const moo::gstrv&) = delete;
void g_strfreev(const moo::gstrv&) = delete;

namespace std {

template<>
struct hash<moo::gstr>
{
    const size_t operator()(const moo::gstr& s) const;
};

} // namespace std
