/*
 *   moofilewriter.c
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

#include <config.h>
#include "mooutils/moofilewriter-private.h"
#include "mooutils/mootype-macros.h"
#include "mooutils/mooutils-fs.h"
#include "mooutils/mooutils-misc.h"
#include "mooutils/mooi18n.h"
#include "mooutils/moocompat.h"
#include <mooutils/mooutils-tests.h>
#include <stdio.h>
#include <string.h>
#include <mooglib/moo-glib.h>
#include <gio/gio.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

using namespace moo;

/************************************************************************/
/* MooFileReader
 */

MooFileReader::MooFileReader()
    : file(nullptr)
{
}

void MooFileReader::close_file ()
{
    if (file)
    {
        mgw_fclose (file);
        file = nullptr;
    }
}

MooFileReader::~MooFileReader()
{
    close_file();
}

void
moo_file_reader_close (MooFileReader *reader)
{
    delete reader;
}

static MooFileReader *
moo_file_reader_new_real (const char  *filename,
                          gboolean     binary,
                          gerrp&       error)
{
    const char *mode = binary ? "rb" : "r";
    MGW_FILE *file;
    MooFileReader *reader;
    mgw_errno_t err;

    g_return_val_if_fail (filename != NULL, NULL);
    g_return_val_if_fail (!error, NULL);

    if (!(file = mgw_fopen (filename, mode, &err)))
    {
        g_set_error (&error, MOO_FILE_ERROR,
                     _moo_file_error_from_errno (err),
                     _("Could not open %s: %s"), filename,
                     mgw_strerror (err));
        return NULL;
    }

    reader = new MooFileReader();
    reader->file = file;

    return reader;
}

MooFileReader *
moo_file_reader_new (const char* filename,
                     gerrp&      error)
{
    return moo_file_reader_new_real (filename, TRUE, error);
}

MooFileReader *
moo_text_reader_new (const char* filename,
                     gerrp&      error)
{
    return moo_file_reader_new_real (filename, FALSE, error);
}

gboolean
moo_file_reader_read (MooFileReader  *reader,
                      char           *buf,
                      gsize           buf_size,
                      gsize          *size_read_p,
                      gerrp&          error)
{
    gsize size_read;
    mgw_errno_t err;

    g_return_val_if_fail (reader != nullptr, FALSE);
    g_return_val_if_fail (size_read_p != NULL, FALSE);
    g_return_val_if_fail (!error, FALSE);
    g_return_val_if_fail (reader->file != NULL, FALSE);
    g_return_val_if_fail (buf_size == 0 || buf != NULL, FALSE);

    if (buf_size == 0)
        return TRUE;

    size_read = mgw_fread (buf, 1, buf_size, reader->file, &err);

    if (size_read != buf_size && mgw_ferror (reader->file))
    {
        g_set_error (&error, MOO_FILE_ERROR,
                     _moo_file_error_from_errno (err),
                     "error reading file: %s",
                     mgw_strerror (err));
        return FALSE;
    }

    *size_read_p = size_read;
    return TRUE;
}


/************************************************************************/
/* MooFileWriter
 */

gboolean
moo_file_writer_write (MooFileWriter *writer,
                       const char    *data,
                       gssize         len)
{
    g_return_val_if_fail (writer != nullptr, FALSE);
    g_return_val_if_fail (data != NULL, FALSE);

    if (len < 0)
        len = strlen (data);
    if (!len)
        return TRUE;

    return writer->write (data, len);
}

gboolean
moo_file_writer_printf (MooFileWriter  *writer,
                        const char     *fmt,
                        ...)
{
    va_list args;
    gboolean ret;

    g_return_val_if_fail (writer != nullptr, FALSE);
    g_return_val_if_fail (fmt != NULL, FALSE);

    va_start (args, fmt);
    ret = writer->printf (fmt, args);
    va_end (args);

    return ret;
}

gboolean
moo_file_writer_printf_markup (MooFileWriter  *writer,
                               const char     *fmt,
                               ...)
{
    g_return_val_if_fail (writer != nullptr, FALSE);
    g_return_val_if_fail (fmt != NULL, FALSE);

    va_list args;
    va_start (args, fmt);
    gstr string = gstr::wrap_new (g_markup_vprintf_escaped (fmt, args));
    va_end (args);

    g_return_val_if_fail (!string.is_null(), FALSE);

    return moo_file_writer_write (writer, string, -1);
}

gboolean
moo_file_writer_close (MooFileWriter* writer,
                       gerrp&         error)
{
    gboolean ret;

    g_return_val_if_fail (writer != nullptr, FALSE);
    g_return_val_if_fail (!error, FALSE);

    ret = writer->close (error);

    delete writer;
    return ret;
}

gboolean
moo_file_writer_close (MooFileWriter  *writer,
                       GError        **errorp)
{
    gerrp error(errorp);
    return moo_file_writer_close (writer, error);
}


/************************************************************************/
/* MooLocalFileWriter
 */

MooLocalFileWriter::MooLocalFileWriter()
    : flags(MOO_FILE_WRITER_FLAGS_NONE)
{
}

MooLocalFileWriter::~MooLocalFileWriter()
{
}

static MooFileWriter *
moo_local_file_writer_new (g::File            file,
                           MooFileWriterFlags flags,
                           gerrp&             error)
{
    if (flags & MOO_FILE_WRITER_CONFIG_MODE)
    {
        mgw_errno_t err;

        gstr filename = file.get_path ();
        gstr dirname = !filename.empty() ? gstr::wrap_new (g_path_get_dirname (filename)) : gstr();

        if (!dirname.empty() && _moo_mkdir_with_parents (dirname, &err) != 0)
        {
            gstr display_name = gstr::wrap_new (g_filename_display_name (dirname));
            g_set_error (&error, G_FILE_ERROR,
                         mgw_file_error_from_errno (err),
                         _("Could not create folder %s: %s"),
                         display_name, mgw_strerror (err));
            return nullptr;
        }
    }

    g::FilePtr file_copy = file.dup ();
    g::FileOutputStreamPtr stream = file_copy->replace (NULL,
                                                        (flags & MOO_FILE_WRITER_SAVE_BACKUP) != 0,
                                                        G_FILE_CREATE_NONE,
                                                        NULL, error);

    if (!stream)
        return nullptr;

    auto writer = make_unique<MooLocalFileWriter>();

    writer->file = file_copy;
    writer->stream = stream;
    writer->flags = flags;

    return writer.release ();
}

MooFileWriter *
moo_file_writer_new (const char*        filename,
                     MooFileWriterFlags flags,
                     gerrp&             error)
{
    g_return_val_if_fail (filename != nullptr, nullptr);
    g_return_val_if_fail (!error, nullptr);

    g::FilePtr file = g::File::new_for_path (filename);
    g_return_val_if_fail (file != nullptr, nullptr);

    return moo_local_file_writer_new (*file, flags, error);
}

MooFileWriter *
moo_file_writer_new_for_file (g::File            file,
                              MooFileWriterFlags flags,
                              gerrp&             error)
{
    g_return_val_if_fail (!error, NULL);
    return moo_local_file_writer_new (file, flags, error);
}

MooFileWriter *
moo_config_writer_new (const char  *filename,
                       gboolean     save_backup,
                       gerrp&       error)
{
    MooFileWriterFlags flags;

    g_return_val_if_fail (filename != NULL, NULL);
    g_return_val_if_fail (!error, NULL);

    flags = MOO_FILE_WRITER_CONFIG_MODE | MOO_FILE_WRITER_TEXT_MODE;
    if (save_backup)
        flags |= MOO_FILE_WRITER_SAVE_BACKUP;

    return moo_file_writer_new (filename, flags, error);
}

MooFileWriter *
moo_config_writer_new (const char  *filename,
                       gboolean     save_backup,
                       GError     **errorp)
{
    gerrp error(errorp);
    return moo_config_writer_new(filename, save_backup, error);
}


bool MooLocalFileWriter::write (const char* data, gsize len)
{
    if (error)
        return FALSE;

    while (len > 0)
    {
        gsize chunk_len = len;
        gsize next_chunk = len;
        gsize bytes_written;
#ifdef __WIN32__
        gboolean need_le = FALSE;
#endif

#ifdef __WIN32__
        if (flags & MOO_FILE_WRITER_TEXT_MODE)
        {
            gsize le_start, le_len;
            if (moo_find_line_end (data, len, &le_start, &le_len))
            {
                need_le = TRUE;
                chunk_len = le_start;
                next_chunk = le_start + le_len;
            }
        }
#endif

        if (!stream->write_all (data, chunk_len,
                                &bytes_written, NULL,
                                error))
            return FALSE;

        data += next_chunk;
        len -= next_chunk;

#ifdef __WIN32__
        if (need_le && !stream->write_all ("\r\n", 2,
                                           &bytes_written, NULL,
                                           error))
            return FALSE;
#endif
    }

    return TRUE;
}

bool MooLocalFileWriter::printf (const char* fmt, va_list args)
{
    if (error)
        return FALSE;

    gstr text = gstr::wrap_new (g_strdup_vprintf (fmt, args));
    return write (text, strlen (text));
}

bool MooLocalFileWriter::close (gerrp& out_error)
{
    g_return_val_if_fail (stream != nullptr, FALSE);

    if (!error)
    {
        stream->flush (NULL, error);
        gerrp second;
        stream->close (NULL, error ? second : error);
        stream.reset ();
        file.reset ();
    }

    out_error = std::move (error);
    return !out_error;
}


/************************************************************************/
/* MooStringWriter
 */

bool MooStringWriter::write (const char* data, gsize len)
{
    g_string_append_len (string, data, len);
    return TRUE;
}

bool MooStringWriter::printf (const char* fmt, va_list args)
{
    gstrp buf;
    gint len = g_vasprintf (buf.pp(), fmt, args);

    if (len >= 0)
        g_string_append_len (string, buf.get(), len);

    return TRUE;
}

bool MooStringWriter::close (G_GNUC_UNUSED gerrp& error)
{
    g_string_free (string, TRUE);
    string = NULL;
    return TRUE;
}

MooStringWriter::MooStringWriter()
    : string (g_string_new (NULL))
{
}

MooStringWriter::~MooStringWriter()
{
    g_assert (!string);
}

MooFileWriter *
moo_string_writer_new (void)
{
    return new MooStringWriter();
}


static gboolean
same_content (const char *filename1,
              const char *filename2)
{
    GMappedFile *file1, *file2;
    char *content1, *content2;
    gsize len;
    gboolean equal = FALSE;

    file1 = g_mapped_file_new (filename1, FALSE, NULL);
    file2 = g_mapped_file_new (filename2, FALSE, NULL);

    if (!file1 || !file2 ||
        g_mapped_file_get_length (file1) != g_mapped_file_get_length (file2))
            goto out;

    len = g_mapped_file_get_length (file1);
    content1 = g_mapped_file_get_contents (file1);
    content2 = g_mapped_file_get_contents (file2);

    if (memcmp (content1, content2, len) == 0)
        equal = TRUE;

out:
    if (file1)
        g_mapped_file_unref (file1);
    if (file2)
        g_mapped_file_unref (file2);
    return equal;
}

static gboolean
check_file_contents (const char *filename,
                     const char *contents)
{
    gboolean equal;
    char *real_contents;

    if (!g_file_get_contents (filename, &real_contents, NULL, NULL))
        return FALSE;

    equal = strcmp (real_contents, contents) == 0;

    g_free (real_contents);
    return equal;
}

static void
test_moo_file_writer (void)
{
    const char *dir;
    char *my_dir, *filename, *bak_filename;
    MooFileWriter *writer;
    gerrp error;

    dir = moo_test_get_working_dir ();
    my_dir = g_build_filename (dir, "cfg-writer", NULL);
    filename = g_build_filename (my_dir, "configfile", NULL);
    bak_filename = g_strdup_printf ("%s~", filename);

    writer = moo_config_writer_new (filename, TRUE, &error);
    TEST_ASSERT_MSG (writer != NULL,
                     "moo_cfg_writer_new failed: %s",
                     moo_error_message (error));
    error.clear ();

    if (writer)
    {
        moo_file_writer_write (writer, "first line\n", -1);
        moo_file_writer_printf (writer, "second line #%d\n", 2);
        moo_file_writer_write (writer, "third\nlalalala\n", 6);
        TEST_ASSERT_MSG (moo_file_writer_close (writer, &error),
                         "moo_file_writer_close failed: %s",
                         moo_error_message (error));
        error.clear ();

#ifdef __WIN32__
#define LE "\r\n"
#else
#define LE "\n"
#endif

        TEST_ASSERT (g_file_test (filename, G_FILE_TEST_EXISTS));
        TEST_ASSERT (!g_file_test (bak_filename, G_FILE_TEST_EXISTS));
        TEST_ASSERT (check_file_contents (filename, "first line" LE "second line #2" LE "third" LE));
    }

    writer = moo_config_writer_new (filename, TRUE, &error);
    TEST_ASSERT_MSG (writer != NULL,
                     "moo_config_writer_new failed: %s",
                     moo_error_message (error));
    if (writer)
    {
        moo_file_writer_write (writer, "First line\n", -1);
        moo_file_writer_printf (writer, "Second line #%d\n", 2);
        moo_file_writer_write (writer, "Third\nlalalala\n", 6);
        TEST_ASSERT_MSG (moo_file_writer_close (writer, &error),
                         "moo_file_writer_close failed: %s",
                         moo_error_message (error));
        error.clear ();

        TEST_ASSERT (g_file_test (filename, G_FILE_TEST_EXISTS));
        TEST_ASSERT (g_file_test (bak_filename, G_FILE_TEST_EXISTS));
        TEST_ASSERT (check_file_contents (filename, "First line" LE "Second line #2" LE "Third" LE));
        TEST_ASSERT (check_file_contents (bak_filename, "first line" LE "second line #2" LE "third" LE));
        TEST_ASSERT (!same_content (bak_filename, filename));
    }

    TEST_ASSERT (_moo_remove_dir (my_dir, TRUE, NULL));

#ifndef __WIN32__
    writer = moo_config_writer_new ("/usr/test-mooutils-fs", TRUE, &error);
#else
    writer = moo_config_writer_new ("K:\\nowayyouhaveit\\file.ini", TRUE, &error);
#endif
    TEST_ASSERT (writer == NULL);
    TEST_ASSERT (error);

    g_free (bak_filename);
    g_free (filename);
    g_free (my_dir);
}

void
moo_test_moo_file_writer (void)
{
    MooTestSuite *suite;

    suite = moo_test_suite_new ("MooFileWriter", "MooFileWriter tests", NULL, NULL, NULL);

    moo_test_suite_add_test (suite, "MooFileWriter", "MooFileWriter tests",
                             (MooTestFunc) test_moo_file_writer, NULL);
}
