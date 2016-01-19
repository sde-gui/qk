/*
 *   moocpp/gobjtypes-gio.cpp
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

#include "moocpp/gobjtypes-gio.h"
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

FilePtr File::dup()
{
    return FilePtr::wrap_new(g_file_dup(nc_gobj()));
}

bool File::equal(File file2)
{
    return g_file_equal(nc_gobj(), file2.gobj());
}

gstr File::get_basename()
{
    return gstr::wrap_new(g_file_get_basename(nc_gobj()));
}

gstr File::get_path()
{
    return gstr::wrap_new(g_file_get_path(nc_gobj()));
}

gstr File::get_uri()
{
    return gstr::wrap_new(g_file_get_uri(nc_gobj()));
}

gstr File::get_parse_name()
{
    return gstr::wrap_new(g_file_get_parse_name(nc_gobj()));
}

FilePtr File::get_parent()
{
    return FilePtr::wrap_new(g_file_get_parent(nc_gobj()));
}

bool File::has_parent(File parent)
{
    return g_file_has_parent(nc_gobj(), parent.gobj());
}

FilePtr File::get_child(const char* name)
{
    return FilePtr::wrap_new(g_file_get_child(nc_gobj(), name));
}

FilePtr File::get_child_for_display_name(const char* display_name, GError** error)
{
    return FilePtr::wrap_new(g_file_get_child_for_display_name(nc_gobj(), display_name, error));
}

bool File::has_prefix(File prefix)
{
    return g_file_has_prefix(nc_gobj(), prefix.gobj());
}

gstr File::get_relative_path(File descendant)
{
    return gstr::wrap_new(g_file_get_relative_path(nc_gobj(), descendant.gobj()));
}

FilePtr File::resolve_relative_path(const char *relative_path)
{
    return FilePtr::wrap_new(g_file_resolve_relative_path(nc_gobj(), relative_path));
}

bool File::is_native()
{
    return g_file_is_native(nc_gobj());
}

bool File::has_uri_scheme(const char *uri_scheme)
{
    return g_file_has_uri_scheme(nc_gobj(), uri_scheme);
}

gstr File::get_uri_scheme()
{
    return gstr::wrap_new(g_file_get_uri_scheme(nc_gobj()));
}

bool File::query_exists(GCancellable* cancellable)
{
    return g_file_query_exists(nc_gobj(), cancellable);
}

GFileType File::query_file_type(GFileQueryInfoFlags flags, GCancellable* cancellable)
{
    return g_file_query_file_type(nc_gobj(), flags, cancellable);
}

GFileInfo* File::query_info(const char *attributes, GFileQueryInfoFlags flags, GCancellable* cancellable, GError** error)
{
    return g_file_query_info(nc_gobj(), attributes, flags, cancellable, error);
}

GFileInfo* File::query_filesystem_info(const char *attributes, GCancellable* cancellable, GError** error)
{
    return g_file_query_filesystem_info(nc_gobj(), attributes, cancellable, error);
}

GFileEnumerator* File::enumerate_children(const char *attributes, GFileQueryInfoFlags flags, GCancellable* cancellable, GError** error)
{
    return g_file_enumerate_children(nc_gobj(), attributes, flags, cancellable, error);
}

FilePtr File::set_display_name(const char* display_name, GCancellable* cancellable, GError** error)
{
    return FilePtr::wrap_new(g_file_set_display_name(nc_gobj(), display_name, cancellable, error));
}

bool File::delete_(GCancellable* cancellable, GError** error)
{
    return g_file_delete(nc_gobj(), cancellable, error);
}

bool File::trash(GCancellable* cancellable, GError** error)
{
    return g_file_trash(nc_gobj(), cancellable, error);
}

bool File::copy(File destination, GFileCopyFlags flags, GCancellable* cancellable, GFileProgressCallback progress_callback, gpointer progress_callback_data, GError** error)
{
    return g_file_copy(nc_gobj(), destination.gobj(), flags, cancellable, progress_callback, progress_callback_data, error);
}

bool File::move(File destination, GFileCopyFlags flags, GCancellable* cancellable, GFileProgressCallback progress_callback, gpointer progress_callback_data, GError** error)
{
    return g_file_move(nc_gobj(), destination.gobj(), flags, cancellable, progress_callback, progress_callback_data, error);
}

bool File::make_directory(GCancellable* cancellable, GError** error)
{
    return g_file_make_directory(nc_gobj(), cancellable, error);
}

bool File::make_directory_with_parents(GCancellable* cancellable, GError** error)
{
    return g_file_make_directory_with_parents(nc_gobj(), cancellable, error);
}

bool File::make_symbolic_link(const char *symlink_value, GCancellable* cancellable, GError** error)
{
    return g_file_make_symbolic_link(nc_gobj(), symlink_value, cancellable, error);
}

bool File::load_contents(GCancellable* cancellable, char** contents, gsize* length, char** etag_out, GError** error)
{
    return g_file_load_contents(nc_gobj(), cancellable, contents, length, etag_out, error);
}

bool File::replace_contents(const char* contents, gsize length, const char* etag, gboolean make_backup, GFileCreateFlags flags, char** new_etag, GCancellable* cancellable, GError** error)
{
    return g_file_replace_contents(nc_gobj(), contents, length, etag, make_backup, flags, new_etag, cancellable, error);
}
