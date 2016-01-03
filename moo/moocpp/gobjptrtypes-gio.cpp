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

FilePtr gobjref<GFile>::dup() const
{
    return FilePtr::wrap_new(g_file_dup(gobj()));
}

bool gobjref<GFile>::equal(GFile* file2) const
{
    return g_file_equal(gobj(), file2);
}

gstr gobjref<GFile>::get_basename() const
{
    return gstr::wrap_new(g_file_get_basename(gobj()));
}

gstr gobjref<GFile>::get_path() const
{
    return gstr::wrap_new(g_file_get_path(gobj()));
}

gstr gobjref<GFile>::get_uri() const
{
    return gstr::wrap_new(g_file_get_uri(gobj()));
}

gstr gobjref<GFile>::get_parse_name() const
{
    return gstr::wrap_new(g_file_get_parse_name(gobj()));
}

FilePtr gobjref<GFile>::get_parent() const
{
    return FilePtr::wrap_new(g_file_get_parent(gobj()));
}

bool gobjref<GFile>::has_parent(GFile* parent) const
{
    return g_file_has_parent(gobj(), parent);
}

FilePtr gobjref<GFile>::get_child(const char* name) const
{
    return FilePtr::wrap_new(g_file_get_child(gobj(), name));
}

FilePtr gobjref<GFile>::get_child_for_display_name(const char* display_name, GError** error) const
{
    return FilePtr::wrap_new(g_file_get_child_for_display_name(gobj(), display_name, error));
}

bool gobjref<GFile>::has_prefix(GFile* prefix) const
{
    return g_file_has_prefix(gobj(), prefix);
}

gstr gobjref<GFile>::get_relative_path(GFile* descendant) const
{
    return gstr::wrap_new(g_file_get_relative_path(gobj(), descendant));
}

FilePtr gobjref<GFile>::resolve_relative_path(const char *relative_path) const
{
    return FilePtr::wrap_new(g_file_resolve_relative_path(gobj(), relative_path));
}

bool gobjref<GFile>::is_native() const
{
    return g_file_is_native(gobj());
}

bool gobjref<GFile>::has_uri_scheme(const char *uri_scheme) const
{
    return g_file_has_uri_scheme(gobj(), uri_scheme);
}

gstr gobjref<GFile>::get_uri_scheme() const
{
    return gstr::wrap_new(g_file_get_uri_scheme(gobj()));
}

bool gobjref<GFile>::query_exists(GCancellable* cancellable) const
{
    return g_file_query_exists(gobj(), cancellable);
}

GFileType gobjref<GFile>::query_file_type(GFileQueryInfoFlags flags, GCancellable* cancellable) const
{
    return g_file_query_file_type(gobj(), flags, cancellable);
}

GFileInfo* gobjref<GFile>::query_info(const char *attributes, GFileQueryInfoFlags flags, GCancellable* cancellable, GError** error) const
{
    return g_file_query_info(gobj(), attributes, flags, cancellable, error);
}

GFileInfo* gobjref<GFile>::query_filesystem_info(const char *attributes, GCancellable* cancellable, GError** error) const
{
    return g_file_query_filesystem_info(gobj(), attributes, cancellable, error);
}

GFileEnumerator* gobjref<GFile>::enumerate_children(const char *attributes, GFileQueryInfoFlags flags, GCancellable* cancellable, GError** error) const
{
    return g_file_enumerate_children(gobj(), attributes, flags, cancellable, error);
}

FilePtr gobjref<GFile>::set_display_name(const char* display_name, GCancellable* cancellable, GError** error) const
{
    return FilePtr::wrap_new(g_file_set_display_name(gobj(), display_name, cancellable, error));
}

bool gobjref<GFile>::delete_(GCancellable* cancellable, GError** error) const
{
    return g_file_delete(gobj(), cancellable, error);
}

bool gobjref<GFile>::trash(GCancellable* cancellable, GError** error) const
{
    return g_file_trash(gobj(), cancellable, error);
}

bool gobjref<GFile>::copy(GFile* destination, GFileCopyFlags flags, GCancellable* cancellable, GFileProgressCallback progress_callback, gpointer progress_callback_data, GError** error) const
{
    return g_file_copy(gobj(), destination, flags, cancellable, progress_callback, progress_callback_data, error);
}

bool gobjref<GFile>::move(GFile* destination, GFileCopyFlags flags, GCancellable* cancellable, GFileProgressCallback progress_callback, gpointer progress_callback_data, GError** error) const
{
    return g_file_move(gobj(), destination, flags, cancellable, progress_callback, progress_callback_data, error);
}

bool gobjref<GFile>::make_directory(GCancellable* cancellable, GError** error) const
{
    return g_file_make_directory(gobj(), cancellable, error);
}

bool gobjref<GFile>::make_directory_with_parents(GCancellable* cancellable, GError** error) const
{
    return g_file_make_directory_with_parents(gobj(), cancellable, error);
}

bool gobjref<GFile>::make_symbolic_link(const char *symlink_value, GCancellable* cancellable, GError** error) const
{
    return g_file_make_symbolic_link(gobj(), symlink_value, cancellable, error);
}

bool gobjref<GFile>::load_contents(GCancellable* cancellable, char** contents, gsize* length, char** etag_out, GError** error) const
{
    return g_file_load_contents(gobj(), cancellable, contents, length, etag_out, error);
}

bool gobjref<GFile>::replace_contents(const char* contents, gsize length, const char* etag, gboolean make_backup, GFileCreateFlags flags, char** new_etag, GCancellable* cancellable, GError** error) const
{
    return g_file_replace_contents(gobj(), contents, length, etag, make_backup, flags, new_etag, cancellable, error);
}
