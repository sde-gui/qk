/*
 *   moocpp/gutil.h
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

#pragma once

#ifdef __cplusplus

#include <memory>
#include <utility>
#include <moocpp/utils.h>
#include <moocpp/strutils.h>
#include <mooglib/moo-glib.h>

namespace moo {
namespace g {

gstr build_filename(const char* c1, const char* c2, const char* c3 = nullptr, const char* c4 = nullptr, const char* c5 = nullptr);
gstr filename_to_uri(const char* filename);
gstr filename_from_uri(const char *uri);
gstr path_get_dirname (const char* filename);
gstr path_get_basename (const char* filename);
gstr filename_display_name (const char* filename);
gstr filename_display_basename (const char* filename);
bool file_get_contents (const char* path, gstr& data, gsize& data_len, gerrp& error);
gstr uri_escape_string(const char* unescaped, const char* reserved_chars_allowed = nullptr, bool allow_utf8 = false);
gstr filename_to_utf8 (const char* opsysstring);
gstr filename_from_utf8(const gchar* utf8string);

gstr get_current_dir();

gstr locale_to_utf8(const char* str);
gstr convert(const char* str, gssize len, const char* to_codeset, const char* from_codeset, gsize& bytes_read, gsize& bytes_written, gerrp& error);

gstr utf8_normalize(const char* str, GNormalizeMode mode);
gstr utf8_strup(const char* str);
gstr utf8_strdown(const char* str);

gstr markup_vprintf_escaped(const char* fmt, va_list args) G_GNUC_PRINTF (1, 0);

template<typename ...Args>
inline gstr markup_printf_escaped (const char* format, Args&& ...args) G_GNUC_PRINTF (1, 2)
{
    return wrap_new (printf_helper::callv (g_markup_printf_escaped, format, std::forward<Args> (args)...));
}

#ifdef __WIN32__
gstr win32_error_message(guint32 code);
gstr utf16_to_utf8(const wchar_t* str);
#endif // __WIN32__

} // namespace g
} // namespace moo

#endif // __cplusplus
