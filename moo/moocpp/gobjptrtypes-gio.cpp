/*
 *   moocpp/gobjptrtypes-gio.cpp
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

#include "moocpp/gobjptrtypes-gio.h"
#include "moocpp/strutils.h"

using namespace moo;

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// gobjref<GFile>
//

using FileRef = gobjref<GFile>;
using FilePtr = gobjptr<GFile>;

FilePtr FileRef::new_for_path(const char* path)
{
    return wrap_new(g_file_new_for_path(path));
}

FilePtr FileRef::new_for_uri(const char* uri)
{
    return wrap_new(g_file_new_for_uri(uri));
}

FilePtr FileRef::new_for_commandline_arg(const char* arg)
{
    return wrap_new(g_file_new_for_commandline_arg(arg));
}

FilePtr FileRef::parse_name(const char* parse_name)
{
    return wrap_new(g_file_parse_name(parse_name));
}

FilePtr FileRef::dup() const
{
    return FilePtr::wrap_new(g_file_dup(g()));
}

bool FileRef::equal(GFile* file2) const
{
    return g_file_equal(g(), file2);
}

gstr FileRef::get_basename() const
{
    return gstr::wrap_new(g_file_get_basename(g()));
}

gstr FileRef::get_path() const
{
    return gstr::wrap_new(g_file_get_path(g()));
}

gstr FileRef::get_uri() const
{
    return gstr::wrap_new(g_file_get_uri(g()));
}

gstr FileRef::get_parse_name() const
{
    return gstr::wrap_new(g_file_get_parse_name(g()));
}

FilePtr FileRef::get_parent() const
{
    return FilePtr::wrap_new(g_file_get_parent(g()));
}

bool FileRef::has_parent(GFile* parent) const
{
    return g_file_has_parent(g(), parent);
}

FilePtr FileRef::get_child(const char* name) const
{
    return FilePtr::wrap_new(g_file_get_child(g(), name));
}

FilePtr FileRef::get_child_for_display_name(const char* display_name, GError** error) const
{
    return FilePtr::wrap_new(g_file_get_child_for_display_name(g(), display_name, error));
}

bool FileRef::has_prefix(GFile* prefix) const
{
    return g_file_has_prefix(g(), prefix);
}

gstr FileRef::get_relative_path(GFile* descendant) const
{
    return gstr::wrap_new(g_file_get_relative_path(g(), descendant));
}

FilePtr FileRef::resolve_relative_path(const char *relative_path) const
{
    return FilePtr::wrap_new(g_file_resolve_relative_path(g(), relative_path));
}

bool FileRef::is_native() const
{
    return g_file_is_native(g());
}

bool FileRef::has_uri_scheme(const char *uri_scheme) const
{
    return g_file_has_uri_scheme(g(), uri_scheme);
}

gstr FileRef::get_uri_scheme() const
{
    return gstr::wrap_new(g_file_get_uri_scheme(g()));
}

bool FileRef::query_exists(GCancellable* cancellable) const
{
    return g_file_query_exists(g(), cancellable);
}

GFileType FileRef::query_file_type(GFileQueryInfoFlags flags, GCancellable* cancellable) const
{
    return g_file_query_file_type(g(), flags, cancellable);
}

GFileInfo* FileRef::query_info(const char *attributes, GFileQueryInfoFlags flags, GCancellable* cancellable, GError** error) const
{
    return g_file_query_info(g(), attributes, flags, cancellable, error);
}

GFileInfo* FileRef::query_filesystem_info(const char *attributes, GCancellable* cancellable, GError** error) const
{
    return g_file_query_filesystem_info(g(), attributes, cancellable, error);
}

GFileEnumerator* FileRef::enumerate_children(const char *attributes, GFileQueryInfoFlags flags, GCancellable* cancellable, GError** error) const
{
    return g_file_enumerate_children(g(), attributes, flags, cancellable, error);
}

FilePtr FileRef::set_display_name(const char* display_name, GCancellable* cancellable, GError** error) const
{
    return FilePtr::wrap_new(g_file_set_display_name(g(), display_name, cancellable, error));
}

bool FileRef::delete_(GCancellable* cancellable, GError** error) const
{
    return g_file_delete(g(), cancellable, error);
}

bool FileRef::trash(GCancellable* cancellable, GError** error) const
{
    return g_file_trash(g(), cancellable, error);
}

bool FileRef::copy(GFile* destination, GFileCopyFlags flags, GCancellable* cancellable, GFileProgressCallback progress_callback, gpointer progress_callback_data, GError** error) const
{
    return g_file_copy(g(), destination, flags, cancellable, progress_callback, progress_callback_data, error);
}

bool FileRef::move(GFile* destination, GFileCopyFlags flags, GCancellable* cancellable, GFileProgressCallback progress_callback, gpointer progress_callback_data, GError** error) const
{
    return g_file_move(g(), destination, flags, cancellable, progress_callback, progress_callback_data, error);
}

bool FileRef::make_directory(GCancellable* cancellable, GError** error) const
{
    return g_file_make_directory(g(), cancellable, error);
}

bool FileRef::make_directory_with_parents(GCancellable* cancellable, GError** error) const
{
    return g_file_make_directory_with_parents(g(), cancellable, error);
}

bool FileRef::make_symbolic_link(const char *symlink_value, GCancellable* cancellable, GError** error) const
{
    return g_file_make_symbolic_link(g(), symlink_value, cancellable, error);
}

bool FileRef::load_contents(GCancellable* cancellable, char** contents, gsize* length, char** etag_out, GError** error) const
{
    return g_file_load_contents(g(), cancellable, contents, length, etag_out, error);
}

bool FileRef::replace_contents(const char* contents, gsize length, const char* etag, gboolean make_backup, GFileCreateFlags flags, char** new_etag, GCancellable* cancellable, GError** error) const
{
    return g_file_replace_contents(g(), contents, length, etag, make_backup, flags, new_etag, cancellable, error);
}
