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
#include "moocpp/strutils.h"

MOO_DEFINE_GOBJ_TYPE(GFile, GObject, G_TYPE_FILE)
MOO_DECLARE_CUSTOM_GOBJ_TYPE(GFile)

namespace moo {

namespace g {

MOO_GOBJ_TYPEDEFS(File, GFile);

} // namespace g

template<>
class gobj_ref<GFile> : public gobj_ref_parent<GFile>
{
public:
    MOO_DEFINE_GOBJREF_METHODS(GFile)

    static g::FilePtr       new_for_path                (const char* path);
    static g::FilePtr       new_for_uri                 (const char* uri);
    static g::FilePtr       new_for_commandline_arg     (const char* arg);
    static g::FilePtr       parse_name                  (const char* parse_name);

    g::FilePtr              dup                         ();

    bool                    equal                       (g::File                file2);
    gstr                    get_basename                ();
    gstr                    get_path                    ();
    gstr                    get_uri                     ();
    gstr                    get_parse_name              ();
    g::FilePtr              get_parent                  ();
    bool                    has_parent                  (g::File                parent);
    g::FilePtr              get_child                   (const char*            name);
    g::FilePtr              get_child_for_display_name  (const char*            display_name,
                                                         GError**               error);
    bool                    has_prefix                  (g::File                prefix);
    gstr                    get_relative_path           (g::File                descendant);
    g::FilePtr              resolve_relative_path       (const char            *relative_path);
    bool                    is_native                   ();
    bool                    has_uri_scheme              (const char            *uri_scheme);
    gstr                    get_uri_scheme              ();
    GFileInputStream*       read                        (GCancellable*          cancellable,
                                                         GError**               error);
    GFileOutputStream*      append_to                   (GFileCreateFlags       flags,
                                                         GCancellable*          cancellable,
							                             GError**               error);
    GFileOutputStream*      create                      (GFileCreateFlags       flags,
                                                         GCancellable*          cancellable,
                                                         GError**               error);
    GFileOutputStream*      replace                     (const char            *etag,
                                                         gboolean               make_backup,
                                                         GFileCreateFlags       flags,
                                                         GCancellable*          cancellable,
                                                         GError**               error);
    GFileIOStream*          open_readwrite              (GCancellable*          cancellable,
                                                         GError**               error);
    GFileIOStream*          create_readwrite            (GFileCreateFlags       flags,
                                                         GCancellable*          cancellable,
                                                         GError**               error);
    GFileIOStream*          replace_readwrite           (const char            *etag,
                                                         gboolean               make_backup,
                                                         GFileCreateFlags       flags,
                                                         GCancellable*          cancellable,
                                                         GError**               error);
    bool                    query_exists                (GCancellable*          cancellable);
    GFileType               query_file_type             (GFileQueryInfoFlags    flags,
                                                         GCancellable*          cancellable);
    GFileInfo*              query_info                  (const char            *attributes,
                                                         GFileQueryInfoFlags    flags,
                                                         GCancellable*          cancellable,
                                                         GError**               error);
    GFileInfo*              query_filesystem_info       (const char            *attributes,
                                                         GCancellable*          cancellable,
                                                         GError**               error);
    GFileEnumerator*        enumerate_children          (const char            *attributes,
                                                         GFileQueryInfoFlags    flags,
                                                         GCancellable*          cancellable,
                                                         GError**               error);
    g::FilePtr              set_display_name            (const char*            display_name,
                                                         GCancellable*          cancellable,
                                                         GError**               error);
    bool                    delete_                     (GCancellable*          cancellable,
                                                         GError**               error);
    bool                    trash                       (GCancellable*          cancellable,
                                                         GError**               error);
    bool                    copy                        (g::File                destination,
                                                         GFileCopyFlags         flags,
                                                         GCancellable*          cancellable,
                                                         GFileProgressCallback  progress_callback,
                                                         gpointer               progress_callback_data,
                                                         GError**               error);
    bool                    move                        (g::File                destination,
                                                         GFileCopyFlags         flags,
                                                         GCancellable*          cancellable,
                                                         GFileProgressCallback  progress_callback,
                                                         gpointer               progress_callback_data,
                                                         GError**               error);
    bool                    make_directory              (GCancellable*          cancellable,
                                                         GError**               error);
    bool                    make_directory_with_parents (GCancellable*          cancellable,
                                                         GError**               error);
    bool                    make_symbolic_link          (const char            *symlink_value,
                                                         GCancellable*          cancellable,
                                                         GError**               error);

    bool                    load_contents               (GCancellable*          cancellable,
                                                         char**                 contents,
                                                         gsize*                 length,
                                                         char**                 etag_out,
                                                         GError**               error);
    bool                    replace_contents            (const char*            contents,
                                                         gsize                  length,
                                                         const char*            etag,
                                                         gboolean               make_backup,
                                                         GFileCreateFlags       flags,
                                                         char**                 new_etag,
                                                         GCancellable*          cancellable,
                                                         GError**               error);
};

} // namespace moo

MOO_REGISTER_CUSTOM_GOBJ_TYPE(GFile)

#endif // __cplusplus
