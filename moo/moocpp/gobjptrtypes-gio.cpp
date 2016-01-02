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

using FilePtr = gobjptr<GFile>;

static void test()
{
    {
    FilePtr x;
    x.g_file();
    }

    {
    FilePtr x;
    x.new_for_path("");
    FilePtr f = FilePtr::new_for_path("");
    f->set("foo", 1, nullptr);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// mg_gobjptr_methods<GFile>
//

const FilePtr& mg_gobjptr_methods<GFile>::self() const
{
    return static_cast<const FilePtr&>(*this);
}

GFile* mg_gobjptr_methods<GFile>::g_file() const
{
    return self().get();
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// gobjref<GFile>
//

const FilePtr& gobjref<GFile>::self() const
{
    return FilePtr::from_gobjref(*this);
}

GFile* gobjref<GFile>::g() const
{
    return self().get();
}

FilePtr gobjref<GFile>::dup() const
{
    return FilePtr::wrap_new(g_file_dup(g()));
}

bool gobjref<GFile>::equal(GFile* file2) const
{
    return g_file_equal(g(), file2);
}

mg_str gobjref<GFile>::get_basename() const
{
    return mg_str::wrap_new(g_file_get_basename(g()));
}

mg_str gobjref<GFile>::get_path() const
{
    return mg_str::wrap_new(g_file_get_path(g()));
}

mg_str gobjref<GFile>::get_uri() const
{
    return mg_str::wrap_new(g_file_get_uri(g()));
}

mg_str gobjref<GFile>::get_parse_name() const
{
    return mg_str::wrap_new(g_file_get_parse_name(g()));
}

FilePtr gobjref<GFile>::get_parent() const
{
    return FilePtr::wrap_new(g_file_get_parent(g()));
}

bool gobjref<GFile>::has_parent(GFile* parent) const
{
    return g_file_has_parent(g(), parent);
}

FilePtr gobjref<GFile>::get_child(const char* name) const
{
    return FilePtr::wrap_new(g_file_get_child(g(), name));
}

FilePtr gobjref<GFile>::get_child_for_display_name(const char* display_name, GError** error) const
{
    return FilePtr::wrap_new(g_file_get_child_for_display_name(g(), display_name, error));
}

bool gobjref<GFile>::has_prefix(GFile* prefix) const
{
    return g_file_has_prefix(g(), prefix);
}

mg_str gobjref<GFile>::get_relative_path(GFile* descendant) const
{
    return mg_str::wrap_new(g_file_get_relative_path(g(), descendant));
}

FilePtr gobjref<GFile>::resolve_relative_path(const char *relative_path) const
{
    return FilePtr::wrap_new(g_file_resolve_relative_path(g(), relative_path));
}

bool gobjref<GFile>::is_native() const
{
    return g_file_is_native(g());
}

bool gobjref<GFile>::has_uri_scheme(const char *uri_scheme) const
{
    return g_file_has_uri_scheme(g(), uri_scheme);
}

mg_str gobjref<GFile>::get_uri_scheme() const
{
    return mg_str::wrap_new(g_file_get_uri_scheme(g()));
}

bool gobjref<GFile>::query_exists(GCancellable* cancellable) const
{
    return g_file_query_exists(g(), cancellable);
}

GFileType gobjref<GFile>::query_file_type(GFileQueryInfoFlags flags, GCancellable* cancellable) const
{
    return g_file_query_file_type(g(), flags, cancellable);
}

GFileInfo* gobjref<GFile>::query_info(const char *attributes, GFileQueryInfoFlags flags, GCancellable* cancellable, GError** error) const
{
    return g_file_query_info(g(), attributes, flags, cancellable, error);
}

GFileInfo* gobjref<GFile>::query_filesystem_info(const char *attributes, GCancellable* cancellable, GError** error) const
{
    return g_file_query_filesystem_info(g(), attributes, cancellable, error);
}

GFileEnumerator* gobjref<GFile>::enumerate_children(const char *attributes, GFileQueryInfoFlags flags, GCancellable* cancellable, GError** error) const
{
    return g_file_enumerate_children(g(), attributes, flags, cancellable, error);
}

FilePtr gobjref<GFile>::set_display_name(const char* display_name, GCancellable* cancellable, GError** error) const
{
    return FilePtr::wrap_new(g_file_set_display_name(g(), display_name, cancellable, error));
}

bool gobjref<GFile>::delete_(GCancellable* cancellable, GError** error) const
{
    return g_file_delete(g(), cancellable, error);
}

bool gobjref<GFile>::trash(GCancellable* cancellable, GError** error) const
{
    return g_file_trash(g(), cancellable, error);
}

bool gobjref<GFile>::copy(GFile* destination, GFileCopyFlags flags, GCancellable* cancellable, GFileProgressCallback progress_callback, gpointer progress_callback_data, GError** error) const
{
    return g_file_copy(g(), destination, flags, cancellable, progress_callback, progress_callback_data, error);
}

bool gobjref<GFile>::move(GFile* destination, GFileCopyFlags flags, GCancellable* cancellable, GFileProgressCallback progress_callback, gpointer progress_callback_data, GError** error) const
{
    return g_file_move(g(), destination, flags, cancellable, progress_callback, progress_callback_data, error);
}

bool gobjref<GFile>::make_directory(GCancellable* cancellable, GError** error) const
{
    return g_file_make_directory(g(), cancellable, error);
}

bool gobjref<GFile>::make_directory_with_parents(GCancellable* cancellable, GError** error) const
{
    return g_file_make_directory_with_parents(g(), cancellable, error);
}

bool gobjref<GFile>::make_symbolic_link(const char *symlink_value, GCancellable* cancellable, GError** error) const
{
    return g_file_make_symbolic_link(g(), symlink_value, cancellable, error);
}

bool gobjref<GFile>::load_contents(GCancellable* cancellable, char** contents, gsize* length, char** etag_out, GError** error) const
{
    return g_file_load_contents(g(), cancellable, contents, length, etag_out, error);
}

bool gobjref<GFile>::replace_contents(const char* contents, gsize length, const char* etag, gboolean make_backup, GFileCreateFlags flags, char** new_etag, GCancellable* cancellable, GError** error) const
{
    return g_file_replace_contents(g(), contents, length, etag, make_backup, flags, new_etag, cancellable, error);
}
