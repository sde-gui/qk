/*
 *   moocpp/gutil.cpp
 *
 *   Copyright (C) 2004-2016 by Yevgen Muntyan <emuntyan@users.sourceforge.net>
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

#include "moocpp/gutil.h"

namespace moo {
namespace g {

gstr filename_to_utf8 (const char* opsysstring)
{
    return wrap_new (g_filename_to_utf8 (opsysstring, -1, nullptr, nullptr, nullptr));
}

gstr filename_from_utf8 (const gchar* utf8string)
{
    return wrap_new (g_filename_from_utf8 (utf8string, -1, nullptr, nullptr, nullptr));
}

gstr build_filename (const char* c1, const char* c2, const char* c3, const char* c4, const char* c5)
{
    return wrap_new (g_build_filename (c1, c2, c3, c4, c5, nullptr));
}

gstr filename_to_uri (const char* filename)
{
    return wrap_new (g_filename_to_uri (filename, nullptr, nullptr));
}

gstr filename_from_uri (const char *uri)
{
    return wrap_new (g_filename_from_uri (uri, nullptr, nullptr));
}

gstr path_get_dirname (const char* filename)
{
    return wrap_new (g_path_get_dirname (filename));
}

gstr filename_display_name (const char* filename)
{
    return wrap_new (g_filename_display_name (filename));
}

gstr filename_display_basename (const char* filename)
{
    return wrap_new (g_filename_display_basename (filename));
}

bool file_get_contents (const char* path, gstr& data, gsize& data_len, gerrp& error)
{
    gstrp contents;

    if (!g_file_get_contents (path, contents.pp (), &data_len, &error))
    {
        data.reset ();
        return false;
    }

    data = wrap_new (contents.release ());
    return true;
}

gstr uri_escape_string (const char* unescaped, const char* reserved_chars_allowed, bool allow_utf8)
{
    return wrap_new (g_uri_escape_string (unescaped, reserved_chars_allowed, allow_utf8));
}

gstr get_current_dir ()
{
    return wrap_new (g_get_current_dir ());
}

gstr locale_to_utf8 (const char* str)
{
    return wrap_new (g_locale_to_utf8 (str, -1, nullptr, nullptr, nullptr));
}

gstr convert (const char* str, gssize len, const char* to_codeset, const char* from_codeset, gsize& bytes_read, gsize& bytes_written, gerrp& error)
{
    return wrap_new (g_convert (str, len, to_codeset, from_codeset, &bytes_read, &bytes_written, &error));
}

gstr utf8_normalize (const char* str, GNormalizeMode mode)
{
    return wrap_new (g_utf8_normalize (str, -1, mode));
}

gstr utf8_strup (const char* str)
{
    return wrap_new (g_utf8_strup (str, -1));
}

gstr utf8_strdown (const char* str)
{
    return wrap_new (g_utf8_strdown (str, -1));
}

gstr markup_vprintf_escaped (const char* fmt, va_list args)
{
    return wrap_new (g_markup_vprintf_escaped (fmt, args));
}

#ifdef __WIN32__

gstr win32_error_message (DWORD code)
{
    return wrap_new (g_win32_error_message (code));
}

gstr utf16_to_utf8 (const wchar_t* str)
{
    return wrap_new (g_utf16_to_utf8 (reinterpret_cast<const gunichar2*>(str), -1, nullptr, nullptr, nullptr));
}

#endif // __WIN32__

} // namespace g
} // namespace moo
