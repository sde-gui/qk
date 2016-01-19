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
MOO_DEFINE_GOBJ_TYPE(GFileInfo, GObject, G_TYPE_FILE_INFO)

MOO_DEFINE_GOBJ_TYPE(GOutputStream, GObject, G_TYPE_OUTPUT_STREAM)
MOO_DEFINE_GOBJ_TYPE(GFileOutputStream, GOutputStream, G_TYPE_FILE_OUTPUT_STREAM)
MOO_DEFINE_GOBJ_TYPE(GInputStream, GObject, G_TYPE_INPUT_STREAM)
MOO_DEFINE_GOBJ_TYPE(GFileInputStream, GInputStream, G_TYPE_FILE_INPUT_STREAM)
MOO_DEFINE_GOBJ_TYPE(GIOStream, GObject, G_TYPE_IO_STREAM)
MOO_DEFINE_GOBJ_TYPE(GFileIOStream, GIOStream, G_TYPE_FILE_IO_STREAM)

MOO_DECLARE_CUSTOM_GOBJ_TYPE(GFile)
MOO_DECLARE_CUSTOM_GOBJ_TYPE(GOutputStream)

namespace moo {

namespace g {

MOO_GOBJ_TYPEDEFS(File, GFile);
MOO_GOBJ_TYPEDEFS(FileInfo, GFileInfo);
MOO_GOBJ_TYPEDEFS(OutputStream, GOutputStream);
MOO_GOBJ_TYPEDEFS(FileOutputStream, GFileOutputStream);
MOO_GOBJ_TYPEDEFS(InputStream, GInputStream);
MOO_GOBJ_TYPEDEFS(FileInputStream, GFileInputStream);
MOO_GOBJ_TYPEDEFS(IOStream, GIOStream);
MOO_GOBJ_TYPEDEFS(FileIOStream, GFileIOStream);

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
                                                         gerrp&                 error);
    bool                    has_prefix                  (g::File                prefix);
    gstr                    get_relative_path           (g::File                descendant);
    g::FilePtr              resolve_relative_path       (const char            *relative_path);
    bool                    is_native                   ();
    bool                    has_uri_scheme              (const char            *uri_scheme);
    gstr                    get_uri_scheme              ();
    g::FileInputStreamPtr   read                        (GCancellable*          cancellable,
                                                         gerrp&                 error);
    g::FileOutputStreamPtr  append_to                   (GFileCreateFlags       flags,
                                                         GCancellable*          cancellable,
							                             gerrp&                 error);
    g::FileOutputStreamPtr  create                      (GFileCreateFlags       flags,
                                                         GCancellable*          cancellable,
                                                         gerrp&                 error);
    g::FileOutputStreamPtr  replace                     (const char            *etag,
                                                         gboolean               make_backup,
                                                         GFileCreateFlags       flags,
                                                         GCancellable*          cancellable,
                                                         gerrp&                 error);
    g::FileIOStreamPtr      open_readwrite              (GCancellable*          cancellable,
                                                         gerrp&                 error);
    g::FileIOStreamPtr      create_readwrite            (GFileCreateFlags       flags,
                                                         GCancellable*          cancellable,
                                                         gerrp&                 error);
    g::FileIOStreamPtr      replace_readwrite           (const char            *etag,
                                                         gboolean               make_backup,
                                                         GFileCreateFlags       flags,
                                                         GCancellable*          cancellable,
                                                         gerrp&                 error);
    bool                    query_exists                (GCancellable*          cancellable);
    GFileType               query_file_type             (GFileQueryInfoFlags    flags,
                                                         GCancellable*          cancellable);
    g::FileInfoPtr          query_info                  (const char            *attributes,
                                                         GFileQueryInfoFlags    flags,
                                                         GCancellable*          cancellable,
                                                         gerrp&                 error);
    g::FileInfoPtr          query_filesystem_info       (const char            *attributes,
                                                         GCancellable*          cancellable,
                                                         gerrp&                 error);
    GFileEnumerator*        enumerate_children          (const char            *attributes,
                                                         GFileQueryInfoFlags    flags,
                                                         GCancellable*          cancellable,
                                                         gerrp&                 error);
    g::FilePtr              set_display_name            (const char*            display_name,
                                                         GCancellable*          cancellable,
                                                         gerrp&                 error);
    bool                    delete_                     (GCancellable*          cancellable,
                                                         gerrp&                 error);
    bool                    trash                       (GCancellable*          cancellable,
                                                         gerrp&                 error);
    bool                    copy                        (g::File                destination,
                                                         GFileCopyFlags         flags,
                                                         GCancellable*          cancellable,
                                                         GFileProgressCallback  progress_callback,
                                                         gpointer               progress_callback_data,
                                                         gerrp&                 error);
    bool                    move                        (g::File                destination,
                                                         GFileCopyFlags         flags,
                                                         GCancellable*          cancellable,
                                                         GFileProgressCallback  progress_callback,
                                                         gpointer               progress_callback_data,
                                                         gerrp&                 error);
    bool                    make_directory              (GCancellable*          cancellable,
                                                         gerrp&                 error);
    bool                    make_directory_with_parents (GCancellable*          cancellable,
                                                         gerrp&                 error);
    bool                    make_symbolic_link          (const char            *symlink_value,
                                                         GCancellable*          cancellable,
                                                         gerrp&                 error);

    bool                    load_contents               (GCancellable*          cancellable,
                                                         char**                 contents,
                                                         gsize*                 length,
                                                         char**                 etag_out,
                                                         gerrp&                 error);
    bool                    replace_contents            (const char*            contents,
                                                         gsize                  length,
                                                         const char*            etag,
                                                         gboolean               make_backup,
                                                         GFileCreateFlags       flags,
                                                         char**                 new_etag,
                                                         GCancellable*          cancellable,
                                                         gerrp&                 error);
};


template<>
class gobj_ref<GOutputStream> : public gobj_ref_parent<GOutputStream>
{
public:
    MOO_DEFINE_GOBJREF_METHODS(GOutputStream)

    gssize  write           (const void                *buffer,
                             gsize                      count,
                             GCancellable              *cancellable,
                             gerrp&                     error);
    bool    write_all       (const void                *buffer,
                             gsize                      count,
                             gsize                     *bytes_written,
                             GCancellable              *cancellable,
                             gerrp&                     error);
    gssize  splice          (g::InputStream             source,
                             GOutputStreamSpliceFlags   flags,
                             GCancellable              *cancellable,
                             gerrp&                     error);
    bool    flush           (GCancellable              *cancellable,
                             gerrp&                     error);
    bool    close           (GCancellable              *cancellable,
                             gerrp&                     error);
    void    write_async     (const void                *buffer,
                             gsize                      count,
                             int                        io_priority,
                             GCancellable              *cancellable,
                             GAsyncReadyCallback        callback,
                             gpointer                   user_data);
    gssize  write_finish    (GAsyncResult              *result,
                             gerrp&                     error);
    void    splice_async    (g::InputStream             source,
                             GOutputStreamSpliceFlags   flags,
                             int                        io_priority,
                             GCancellable              *cancellable,
                             GAsyncReadyCallback        callback,
                             gpointer                   user_data);
    gssize  splice_finish   (GAsyncResult              *result,
                             gerrp&                     error);
    void    flush_async     (int                        io_priority,
                             GCancellable              *cancellable,
                             GAsyncReadyCallback        callback,
                             gpointer                   user_data);
    bool    flush_finish    (GAsyncResult              *result,
                             gerrp&                     error);
    void    close_async     (int                        io_priority,
                             GCancellable              *cancellable,
                             GAsyncReadyCallback        callback,
                             gpointer                   user_data);
    bool    close_finish    (GAsyncResult              *result,
                             gerrp&                     error);

    bool    is_closed       ();
    bool    is_closing      ();
    bool    has_pending     ();
    bool    set_pending     (gerrp&                     error);
    void    clear_pending   ();
};


} // namespace moo

MOO_REGISTER_CUSTOM_GOBJ_TYPE(GFile)
MOO_REGISTER_CUSTOM_GOBJ_TYPE(GOutputStream)

#endif // __cplusplus
