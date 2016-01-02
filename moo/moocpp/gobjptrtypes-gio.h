/*
 *   moocpp/gobjptrtypes-gio.h
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

#include <gio/gio.h>
#include <mooglib/moo-glib.h>

#include "moocpp/gobjptrtypes-glib.h"

namespace moo {

class mg_str;

///////////////////////////////////////////////////////////////////////////////////////////
//
// mg_gobj_handle
//

template<>
struct gobjref<GFile> : public gobjref<GObject>
{
    gobjptr<GFile>          dup                         () const;

    bool                    equal                       (GFile*                 file2) const;
    mg_str                  get_basename                () const;
    mg_str                  get_path                    () const;
    mg_str                  get_uri                     () const;
    mg_str                  get_parse_name              () const;
    gobjptr<GFile>          get_parent                  () const;
    bool                    has_parent                  (GFile*                 parent) const;
    gobjptr<GFile>          get_child                   (const char*            name) const;
    gobjptr<GFile>          get_child_for_display_name  (const char*            display_name,
                                                         GError**               error) const;
    bool                    has_prefix                  (GFile*                 prefix) const;
    mg_str                  get_relative_path           (GFile*                 descendant) const;
    gobjptr<GFile>          resolve_relative_path       (const char            *relative_path) const;
    bool                    is_native                   () const;
    bool                    has_uri_scheme              (const char            *uri_scheme) const;
    mg_str                  get_uri_scheme              () const;
    GFileInputStream*       read                        (GCancellable*          cancellable,
                                                         GError**               error) const;
    GFileOutputStream*      append_to                   (GFileCreateFlags       flags,
                                                         GCancellable*          cancellable,
							                             GError**               error) const;
    GFileOutputStream*      create                      (GFileCreateFlags       flags,
                                                         GCancellable*          cancellable,
                                                         GError**               error) const;
    GFileOutputStream*      replace                     (const char            *etag,
                                                         gboolean               make_backup,
                                                         GFileCreateFlags       flags,
                                                         GCancellable*          cancellable,
                                                         GError**               error) const;
    GFileIOStream*          open_readwrite              (GCancellable*          cancellable,
                                                         GError**               error) const;
    GFileIOStream*          create_readwrite            (GFileCreateFlags       flags,
                                                         GCancellable*          cancellable,
                                                         GError**               error) const;
    GFileIOStream*          replace_readwrite           (const char            *etag,
                                                         gboolean               make_backup,
                                                         GFileCreateFlags       flags,
                                                         GCancellable*          cancellable,
                                                         GError**               error) const;
    bool                    query_exists                (GCancellable*          cancellable) const;
    GFileType               query_file_type             (GFileQueryInfoFlags    flags,
                                                         GCancellable*          cancellable) const;
    GFileInfo*              query_info                  (const char            *attributes,
                                                         GFileQueryInfoFlags    flags,
                                                         GCancellable*          cancellable,
                                                         GError**               error) const;
    GFileInfo*              query_filesystem_info       (const char            *attributes,
                                                         GCancellable*          cancellable,
                                                         GError**               error) const;
    GFileEnumerator*        enumerate_children          (const char            *attributes,
                                                         GFileQueryInfoFlags    flags,
                                                         GCancellable*          cancellable,
                                                         GError**               error) const;
    gobjptr<GFile>          set_display_name            (const char*            display_name,
                                                         GCancellable*          cancellable,
                                                         GError**               error) const;
    bool                    delete_                     (GCancellable*          cancellable,
                                                         GError**               error) const;
    bool                    trash                       (GCancellable*          cancellable,
                                                         GError**               error) const;
    bool                    copy                        (GFile*                 destination,
                                                         GFileCopyFlags         flags,
                                                         GCancellable*          cancellable,
                                                         GFileProgressCallback  progress_callback,
                                                         gpointer               progress_callback_data,
                                                         GError**               error) const;
    bool                    move                        (GFile*                 destination,
                                                         GFileCopyFlags         flags,
                                                         GCancellable*          cancellable,
                                                         GFileProgressCallback  progress_callback,
                                                         gpointer               progress_callback_data,
                                                         GError**               error) const;
    bool                    make_directory              (GCancellable*          cancellable,
                                                         GError**               error) const;
    bool                    make_directory_with_parents (GCancellable*          cancellable,
                                                         GError**               error) const;
    bool                    make_symbolic_link          (const char            *symlink_value,
                                                         GCancellable*          cancellable,
                                                         GError**               error) const;

    bool                    load_contents               (GCancellable*          cancellable,
                                                         char**                 contents,
                                                         gsize*                 length,
                                                         char**                 etag_out,
                                                         GError**               error) const;
    bool                    replace_contents            (const char*            contents,
                                                         gsize                  length,
                                                         const char*            etag,
                                                         gboolean               make_backup,
                                                         GFileCreateFlags       flags,
                                                         char**                 new_etag,
                                                         GCancellable*          cancellable,
                                                         GError**               error) const;

    GFile*                  g() const;
    const gobjptr<GFile>&   self() const;
};

///////////////////////////////////////////////////////////////////////////////////////////
//
// mg_gobjptr_methods
//

template<>
struct mg_gobjptr_methods<GFile> : public mg_gobjptr_methods<GObject>
{
    GFile*  g_file() const;

    static gobjptr<GFile>   new_for_path                (const char* path);
    static gobjptr<GFile>   new_for_uri                 (const char* uri);
    static gobjptr<GFile>   new_for_commandline_arg     (const char* arg);
    static gobjptr<GFile>   parse_name                  (const char* parse_name);

private:
    const gobjptr<GFile>& self() const; //{ return static_cast<const gobjptr<GFile>&>(*this); }
};

//template<typename Self>
//struct mg_gobjptr_handle<Self, GFile>
//    : public mg_gobjptr_handle<Self, GObject>
//{
//    gobjptr<GFile>  dup() const                                         { return Self(self_impl().dup(), ref_transfer::take_ownership); }
//
//    void blah_instance() const;
//    static void blah_static();
//
//    static Self     new_for_path            (const char* path)          { return Self(impl::new_for_path(path), ref_transfer::take_ownership); }
//    static Self     new_for_uri             (const char* uri)           { return Self(impl::new_for_uri(path), ref_transfer::take_ownership); }
//    static Self     new_for_commandline_arg (const char* arg)           { return Self(impl::new_for_commandline_arg(path), ref_transfer::take_ownership); }
//
//    static Self     parse_name              (const char* parse_name)    { return Self(impl::parse_name(parse_name)); }
//
//protected:
//    using impl = mg_gobj_handle<GFile>;
//    const impl& self_impl() const { return static_cast<const impl&>(self()); }
//};

} // namespace moo
