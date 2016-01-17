/*
 *   moocpp/gobjtypes-gio.h
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

#include <gio/gio.h>
#include <mooglib/moo-glib.h>

#include "moocpp/gobjtypes-glib.h"

MOO_DEFINE_GOBJ_TYPE(GFile, GObject, G_TYPE_FILE)

namespace moo {

class gstr;

template<>
class gobj_ref<GFile> : public gobj_ref_parent<GFile>
{
public:
    MOO_DEFINE_GOBJREF_METHODS(GFile)

    static gobj_ptr<GFile>  new_for_path                (const char* path);
    static gobj_ptr<GFile>  new_for_uri                 (const char* uri);
    static gobj_ptr<GFile>  new_for_commandline_arg     (const char* arg);
    static gobj_ptr<GFile>  parse_name                  (const char* parse_name);

    gobj_ptr<GFile>         dup                         () const;

    bool                    equal                       (GFile*                 file2) const;
    gstr                    get_basename                () const;
    gstr                    get_path                    () const;
    gstr                    get_uri                     () const;
    gstr                    get_parse_name              () const;
    gobj_ptr<GFile>         get_parent                  () const;
    bool                    has_parent                  (GFile*                 parent) const;
    gobj_ptr<GFile>         get_child                   (const char*            name) const;
    gobj_ptr<GFile>         get_child_for_display_name  (const char*            display_name,
                                                         GError**               error) const;
    bool                    has_prefix                  (GFile*                 prefix) const;
    gstr                    get_relative_path           (GFile*                 descendant) const;
    gobj_ptr<GFile>         resolve_relative_path       (const char            *relative_path) const;
    bool                    is_native                   () const;
    bool                    has_uri_scheme              (const char            *uri_scheme) const;
    gstr                    get_uri_scheme              () const;
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
    gobj_ptr<GFile>         set_display_name            (const char*            display_name,
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
};

namespace g {

MOO_GOBJ_TYPEDEFS(File, GFile);

} // namespace g

} // namespace moo

#endif // __cplusplus
