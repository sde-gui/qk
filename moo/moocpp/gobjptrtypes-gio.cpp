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
using namespace moo::g;

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// gobj_ref<GFile>
//

FilePtr File::new_for_path(const char* path)
{
    return wrap_new(g_file_new_for_path(path));
}

FilePtr File::new_for_uri(const char* uri)
{
    return wrap_new(g_file_new_for_uri(uri));
}

FilePtr File::new_for_commandline_arg(const char* arg)
{
    return wrap_new(g_file_new_for_commandline_arg(arg));
}

FilePtr File::parse_name(const char* parse_name)
{
    return wrap_new(g_file_parse_name(parse_name));
}

FilePtr File::dup() const
{
    return FilePtr::wrap_new(g_file_dup(g()));
}

bool File::equal(GFile* file2) const
{
    return g_file_equal(g(), file2);
}

gstr File::get_basename() const
{
    return gstr::wrap_new(g_file_get_basename(g()));
}

gstr File::get_path() const
{
    return gstr::wrap_new(g_file_get_path(g()));
}

gstr File::get_uri() const
{
    return gstr::wrap_new(g_file_get_uri(g()));
}

gstr File::get_parse_name() const
{
    return gstr::wrap_new(g_file_get_parse_name(g()));
}

FilePtr File::get_parent() const
{
    return FilePtr::wrap_new(g_file_get_parent(g()));
}

bool File::has_parent(GFile* parent) const
{
    return g_file_has_parent(g(), parent);
}

FilePtr File::get_child(const char* name) const
{
    return FilePtr::wrap_new(g_file_get_child(g(), name));
}

FilePtr File::get_child_for_display_name(const char* display_name, GError** error) const
{
    return FilePtr::wrap_new(g_file_get_child_for_display_name(g(), display_name, error));
}

bool File::has_prefix(GFile* prefix) const
{
    return g_file_has_prefix(g(), prefix);
}

gstr File::get_relative_path(GFile* descendant) const
{
    return gstr::wrap_new(g_file_get_relative_path(g(), descendant));
}

FilePtr File::resolve_relative_path(const char *relative_path) const
{
    return FilePtr::wrap_new(g_file_resolve_relative_path(g(), relative_path));
}

bool File::is_native() const
{
    return g_file_is_native(g());
}

bool File::has_uri_scheme(const char *uri_scheme) const
{
    return g_file_has_uri_scheme(g(), uri_scheme);
}

gstr File::get_uri_scheme() const
{
    return gstr::wrap_new(g_file_get_uri_scheme(g()));
}

bool File::query_exists(GCancellable* cancellable) const
{
    return g_file_query_exists(g(), cancellable);
}

GFileType File::query_file_type(GFileQueryInfoFlags flags, GCancellable* cancellable) const
{
    return g_file_query_file_type(g(), flags, cancellable);
}

GFileInfo* File::query_info(const char *attributes, GFileQueryInfoFlags flags, GCancellable* cancellable, GError** error) const
{
    return g_file_query_info(g(), attributes, flags, cancellable, error);
}

GFileInfo* File::query_filesystem_info(const char *attributes, GCancellable* cancellable, GError** error) const
{
    return g_file_query_filesystem_info(g(), attributes, cancellable, error);
}

GFileEnumerator* File::enumerate_children(const char *attributes, GFileQueryInfoFlags flags, GCancellable* cancellable, GError** error) const
{
    return g_file_enumerate_children(g(), attributes, flags, cancellable, error);
}

FilePtr File::set_display_name(const char* display_name, GCancellable* cancellable, GError** error) const
{
    return FilePtr::wrap_new(g_file_set_display_name(g(), display_name, cancellable, error));
}

bool File::delete_(GCancellable* cancellable, GError** error) const
{
    return g_file_delete(g(), cancellable, error);
}

bool File::trash(GCancellable* cancellable, GError** error) const
{
    return g_file_trash(g(), cancellable, error);
}

bool File::copy(GFile* destination, GFileCopyFlags flags, GCancellable* cancellable, GFileProgressCallback progress_callback, gpointer progress_callback_data, GError** error) const
{
    return g_file_copy(g(), destination, flags, cancellable, progress_callback, progress_callback_data, error);
}

bool File::move(GFile* destination, GFileCopyFlags flags, GCancellable* cancellable, GFileProgressCallback progress_callback, gpointer progress_callback_data, GError** error) const
{
    return g_file_move(g(), destination, flags, cancellable, progress_callback, progress_callback_data, error);
}

bool File::make_directory(GCancellable* cancellable, GError** error) const
{
    return g_file_make_directory(g(), cancellable, error);
}

bool File::make_directory_with_parents(GCancellable* cancellable, GError** error) const
{
    return g_file_make_directory_with_parents(g(), cancellable, error);
}

bool File::make_symbolic_link(const char *symlink_value, GCancellable* cancellable, GError** error) const
{
    return g_file_make_symbolic_link(g(), symlink_value, cancellable, error);
}

bool File::load_contents(GCancellable* cancellable, char** contents, gsize* length, char** etag_out, GError** error) const
{
    return g_file_load_contents(g(), cancellable, contents, length, etag_out, error);
}

bool File::replace_contents(const char* contents, gsize length, const char* etag, gboolean make_backup, GFileCreateFlags flags, char** new_etag, GCancellable* cancellable, GError** error) const
{
    return g_file_replace_contents(g(), contents, length, etag, make_backup, flags, new_etag, cancellable, error);
}
