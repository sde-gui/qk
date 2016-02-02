/*
 *   mooutils-file.h
 *
 *   Copyright (C) 2004-2010 by Yevgen Muntyan <emuntyan@users.sourceforge.net>
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

#include <mooutils/mooutils-misc.h>

#ifdef __cplusplus

#include <moocpp/moocpp.h>

struct MooFileReader;
struct MooFileWriter;

void g_object_ref(MooFileReader*) = delete;
void g_object_unref(MooFileReader*) = delete;
void g_object_ref(MooFileWriter*) = delete;
void g_object_unref(MooFileWriter*) = delete;

MooFileReader  *moo_file_reader_new             (const char     *filename,
                                                 moo::gerrp&     error);
MooFileReader  *moo_text_reader_new             (const char     *filename,
                                                 moo::gerrp&     error);
gboolean        moo_file_reader_read            (MooFileReader  *reader,
                                                 char           *buf,
                                                 gsize           buf_size,
                                                 gsize          *size_read,
                                                 moo::gerrp&     error);
void            moo_file_reader_close           (MooFileReader  *reader);

typedef enum /*< flags >*/
{
    MOO_FILE_WRITER_FLAGS_NONE  = 0,
    MOO_FILE_WRITER_SAVE_BACKUP = 1 << 0,
    MOO_FILE_WRITER_CONFIG_MODE = 1 << 1,
    MOO_FILE_WRITER_TEXT_MODE   = 1 << 2
} MooFileWriterFlags;

MOO_DEFINE_FLAGS(MooFileWriterFlags);

MooFileWriter  *moo_file_writer_new             (const char     *filename,
                                                 MooFileWriterFlags flags,
                                                 moo::gerrp&     error);
MooFileWriter  *moo_file_writer_new_for_file    (moo::g::File    file,
                                                 MooFileWriterFlags flags,
                                                 moo::gerrp&     error);
MooFileWriter  *moo_config_writer_new           (const char     *filename,
                                                 gboolean        save_backup,
                                                 moo::gerrp&     error);
MooFileWriter  *moo_string_writer_new           (void);

gboolean        moo_file_writer_close           (MooFileWriter  *writer,
                                                 moo::gerrp&     error);


#endif // __cplusplus

G_BEGIN_DECLS

typedef struct MooFileWriter MooFileWriter;

MooFileWriter  *moo_config_writer_new           (const char     *filename,
                                                 gboolean        save_backup,
                                                 GError        **error);
gboolean        moo_file_writer_write           (MooFileWriter  *writer,
                                                 const char     *data,
                                                 gssize          len MOO_CPP_DFLT_PARAM(-1));
gboolean        moo_file_writer_printf_c        (MooFileWriter  *writer,
                                                 const char     *fmt,
                                                 ...) G_GNUC_PRINTF (2, 3);
gboolean        moo_file_writer_printf_markup_c (MooFileWriter  *writer,
                                                 const char     *fmt,
                                                 ...) G_GNUC_PRINTF (2, 3);
gboolean        moo_file_writer_close           (MooFileWriter  *writer,
                                                 GError        **error);

#ifndef __cplusplus

#define moo_file_writer_printf moo_file_writer_printf_c
#define moo_file_writer_printf_markup moo_file_writer_printf_markup_c

#endif // !__cplusplus

G_END_DECLS

#ifdef __cplusplus

template<typename ...Args>
inline bool moo_file_writer_printf (MooFileWriter *writer, const char* fmt, Args&& ...args) G_GNUC_PRINTF (2, 3)
{
    gstr s = gstr::printf (fmt, std::forward<Args> (args)...);
    return moo_file_writer_write (writer, s);
}

template<typename ...Args>
inline bool moo_file_writer_printf_markup (MooFileWriter  *writer,
                                           const char     *fmt,
                                           Args&&... args) G_GNUC_PRINTF (2, 3)
{
    gstr s = g::markup_printf_escaped (fmt, std::forward<Args> (args)...);
    return moo_file_writer_write (writer, s);
}

#endif // __cplusplus
