/*
 *   mooutils-file.c
 *
 *   Copyright (C) 2004-2008 by Yevgen Muntyan <muntyan@tamu.edu>
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
#include "mooutils/mooutils-file-private.h"
#include "mooutils/mootype-macros.h"
#include "mooutils/mooutils-fs.h"
#include "mooutils/mooi18n.h"
#include <stdio.h>
#include <errno.h>
#include <glib/gstdio.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

/* Keep in sync with gio */
#define BACKUP_EXTENSION "~"

/************************************************************************/
/* MooFileReader
 */

struct MooFileReader {
    GObject base;
    FILE *file;
};

MOO_DEFINE_TYPE_STATIC (MooFileReader, moo_file_reader, G_TYPE_OBJECT)

static void
moo_file_reader_init (MooFileReader *reader)
{
    reader->file = NULL;
}

static void
moo_file_reader_close_file (MooFileReader *reader)
{
    if (reader->file)
    {
        fclose (reader->file);
        reader->file = NULL;
    }
}

static void
moo_file_reader_dispose (GObject *object)
{
    MooFileReader *reader = MOO_FILE_READER (object);
    moo_file_reader_close_file (reader);
    G_OBJECT_CLASS (moo_file_reader_parent_class)->dispose (object);
}

static void
moo_file_reader_class_init (MooFileReaderClass *klass)
{
    G_OBJECT_CLASS (klass)->dispose = moo_file_reader_dispose;
}

void
moo_file_reader_close (MooFileReader *reader)
{
    g_return_if_fail (MOO_IS_FILE_READER (reader));
    moo_file_reader_close_file (reader);
    g_object_unref (reader);
}

static MooFileReader *
moo_file_reader_new_real (const char  *filename,
                          gboolean     binary,
                          GError     **error)
{
    const char *mode = binary ? "rb" : "r";
    FILE *file;
    MooFileReader *reader;

    g_return_val_if_fail (filename != NULL, NULL);
    g_return_val_if_fail (!error || !*error, NULL);

    errno = 0;
    if (!(file = g_fopen (filename, mode)))
    {
        int err = errno;
        g_set_error (error, MOO_FILE_ERROR,
                     _moo_file_error_from_errno (err),
                     _("Could not open %s: %s"), filename,
                     g_strerror (err));
        return NULL;
    }

    reader = g_object_new (MOO_TYPE_FILE_READER, NULL);
    reader->file = file;

    return reader;
}

MooFileReader *
moo_file_reader_new (const char  *filename,
                     GError     **error)
{
    return moo_file_reader_new_real (filename, TRUE, error);
}

MooFileReader *
moo_text_reader_new (const char  *filename,
                     GError     **error)
{
    return moo_file_reader_new_real (filename, FALSE, error);
}

gboolean
moo_file_reader_read (MooFileReader  *reader,
                      char           *buf,
                      gsize           buf_size,
                      gsize          *size_read_p,
                      GError        **error)
{
    gsize size_read;

    g_return_val_if_fail (MOO_IS_FILE_READER (reader), FALSE);
    g_return_val_if_fail (size_read_p != NULL, FALSE);
    g_return_val_if_fail (!error || !*error, FALSE);
    g_return_val_if_fail (reader->file != NULL, FALSE);
    g_return_val_if_fail (buf_size == 0 || buf != NULL, FALSE);

    if (buf_size == 0)
        return TRUE;

    errno = 0;
    size_read = fread (buf, 1, buf_size, reader->file);

    if (size_read != buf_size && ferror (reader->file))
    {
        int err = errno;
        g_set_error (error, MOO_FILE_ERROR,
                     _moo_file_error_from_errno (err),
                     "error reading file: %s",
                     g_strerror (err));
        return FALSE;
    }

    *size_read_p = size_read;
    return TRUE;
}


/************************************************************************/
/* MooFileWriter
 */

MOO_DEFINE_TYPE_STATIC (MooFileWriter, moo_file_writer, G_TYPE_OBJECT)

static void
moo_file_writer_init (G_GNUC_UNUSED MooFileWriter *writer)
{
}

static void
moo_file_writer_class_init (G_GNUC_UNUSED MooFileWriterClass *klass)
{
}

gboolean
moo_file_writer_write (MooFileWriter *writer,
                       const char    *data,
                       gssize         len)
{
    g_return_val_if_fail (MOO_IS_FILE_WRITER (writer), FALSE);
    g_return_val_if_fail (data != NULL, FALSE);

    if (len < 0)
        len = strlen (data);
    if (!len)
        return TRUE;

    return MOO_FILE_WRITER_GET_CLASS (writer)->meth_write (writer, data, len);
}

gboolean
moo_file_writer_printf (MooFileWriter  *writer,
                        const char     *fmt,
                        ...)
{
    va_list args;
    gboolean ret;

    g_return_val_if_fail (MOO_IS_FILE_WRITER (writer), FALSE);
    g_return_val_if_fail (fmt != NULL, FALSE);

    va_start (args, fmt);
    ret = MOO_FILE_WRITER_GET_CLASS (writer)->meth_printf (writer, fmt, args);
    va_end (args);

    return ret;
}

gboolean
moo_file_writer_close (MooFileWriter  *writer,
                       GError        **error)
{
    gboolean ret;

    g_return_val_if_fail (MOO_IS_FILE_WRITER (writer), FALSE);
    g_return_val_if_fail (!error || !*error, FALSE);

    ret = MOO_FILE_WRITER_GET_CLASS (writer)->meth_close (writer, error);

    g_object_unref (writer);
    return ret;
}


/************************************************************************/
/* MooLocalFileWriter
 */

struct MooLocalFileWriter {
    MooFileWriter base;

    FILE *file;
    char *filename;
    char *temp_filename;
    MooFileWriterFlags flags;
    GError *error;
};

MOO_DEFINE_TYPE_STATIC (MooLocalFileWriter, moo_local_file_writer, MOO_TYPE_FILE_WRITER)

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
        g_mapped_file_free (file1);
    if (file2)
        g_mapped_file_free (file2);
    return equal;
}

static MooFileWriter *
moo_local_file_writer_new (const char          *filename,
                           MooFileWriterFlags   flags,
                           GError             **error)
{
    int fd;
    FILE *file;
    char *temp_filename = NULL;
    char *dirname = NULL;
    MooLocalFileWriter *writer = NULL;

    g_return_val_if_fail (filename != NULL, NULL);

    dirname = g_path_get_dirname (filename);

    if (_moo_mkdir_with_parents (dirname) != 0)
    {
        int err = errno;
        char *display_name = g_filename_display_name (dirname);
        g_set_error (error, G_FILE_ERROR,
                     g_file_error_from_errno (err),
                     _("Could not create folder %s: %s"),
                     display_name, g_strerror (err));
        g_free (display_name);
        goto out;
    }

    temp_filename = g_strdup_printf ("%s" G_DIR_SEPARATOR_S ".cfg-tmp-XXXXXX", dirname);
    errno = 0;
    fd = g_mkstemp (temp_filename);

    if (fd == -1)
    {
        int err = errno;
        char *display_name = g_filename_display_name (temp_filename);
        g_set_error (error, G_FILE_ERROR,
                     g_file_error_from_errno (err),
                     _("Could not create temporary file %s: %s"),
                     display_name, g_strerror (err));
        g_free (display_name);
        goto out;
    }

    close (fd);

    if (!(file = g_fopen (temp_filename,
                          (flags & MOO_FILE_WRITER_TEXT_MODE) ? "w" : "wb")))
    {
        int err = errno;
        char *display_name = g_filename_display_name (temp_filename);
        g_set_error (error, G_FILE_ERROR,
                     g_file_error_from_errno (err),
                     _("Could not create temporary file %s: %s"),
                     display_name, g_strerror (err));
        g_free (display_name);
        g_unlink (temp_filename);
        goto out;
    }

    writer = g_object_new (MOO_TYPE_LOCAL_FILE_WRITER, NULL);
    writer->file = file;
    writer->temp_filename = temp_filename;
    temp_filename = NULL;
    writer->filename = g_strdup (filename);
    writer->error = NULL;
    writer->flags = flags;

out:
    g_free (temp_filename);
    g_free (dirname);
    return MOO_FILE_WRITER (writer);
}

MooFileWriter *
moo_file_writer_new (const char          *filename,
                     MooFileWriterFlags   flags,
                     GError             **error)
{
    g_return_val_if_fail (filename != NULL, NULL);
    g_return_val_if_fail (!error || !*error, NULL);
    return moo_local_file_writer_new (filename, flags, error);
}

MooFileWriter *
moo_config_writer_new (const char  *filename,
                       gboolean     save_backup,
                       GError     **error)
{
    MooFileWriterFlags flags;

    g_return_val_if_fail (filename != NULL, NULL);
    g_return_val_if_fail (!error || !*error, NULL);

    flags = MOO_FILE_WRITER_TEXT_MODE | MOO_FILE_WRITER_LAZY;
    if (save_backup)
        flags |= MOO_FILE_WRITER_SAVE_BACKUP;

    return moo_file_writer_new (filename, flags, error);
}


static gboolean
moo_local_file_writer_write (MooFileWriter *fwriter,
                             const char    *data,
                             gsize          len)
{
    MooLocalFileWriter *writer = (MooLocalFileWriter*) fwriter;

    if (writer->error)
        return FALSE;

    while (len > 0)
    {
        gsize written;

        errno = 0;
        written = fwrite (data, 1, len, writer->file);

        if (written < len && ferror (writer->file))
        {
            int err = errno;
            g_set_error (&writer->error, G_FILE_ERROR,
                         g_file_error_from_errno (err),
                         "%s", g_strerror (err));
            break;
        }

        len -= written;
        data += written;
    }

    return writer->error == NULL;
}

G_GNUC_PRINTF (2, 0)
static gboolean
moo_local_file_writer_printf (MooFileWriter *fwriter,
                              const char    *fmt,
                              va_list        args)
{
    MooLocalFileWriter *writer = (MooLocalFileWriter*) fwriter;

    if (writer->error)
        return FALSE;

    errno = 0;

    if (vfprintf (writer->file, fmt, args) < 0)
    {
        int err = errno;
        g_set_error (&writer->error, G_FILE_ERROR,
                     g_file_error_from_errno (err),
                     "%s", g_strerror (err));
    }

    return writer->error == NULL;
}

static gboolean
moo_local_file_writer_close (MooFileWriter *fwriter,
                             GError       **error)
{
    MooLocalFileWriter *writer = (MooLocalFileWriter*) fwriter;
    gboolean retval;

    if (!writer->error)
    {
        errno = 0;

        if (fclose (writer->file) != 0)
        {
            int err = errno;
            g_set_error (&writer->error, G_FILE_ERROR,
                         g_file_error_from_errno (err),
                         "%s", g_strerror (err));
        }

        writer->file = NULL;
    }

    if (!writer->error &&
        (!(writer->flags & MOO_FILE_WRITER_LAZY) ||
         !same_content (writer->filename, writer->temp_filename)))
    {
        if (writer->flags & MOO_FILE_WRITER_SAVE_BACKUP)
        {
            char *bak_file;

            bak_file = g_strdup_printf ("%s" BACKUP_EXTENSION, writer->filename);

            if (g_file_test (writer->filename, G_FILE_TEST_EXISTS))
            {
                g_unlink (bak_file);
                _moo_rename_file (writer->filename, bak_file, NULL);
            }

            g_free (bak_file);
        }

#ifdef __WIN32__
        _moo_unlink (writer->filename);
#endif
        _moo_rename_file (writer->temp_filename,
                          writer->filename,
                          &writer->error);
    }

    g_unlink (writer->temp_filename);
    g_free (writer->temp_filename);
    g_free (writer->filename);
    writer->temp_filename = NULL;
    writer->filename = NULL;

    retval = writer->error == NULL;

    if (writer->error)
        g_propagate_error (error, writer->error);

    writer->error = NULL;
    return retval;
}


static void
moo_local_file_writer_class_init (MooLocalFileWriterClass *klass)
{
    MooFileWriterClass *writer_class = MOO_FILE_WRITER_CLASS (klass);
    writer_class->meth_write = moo_local_file_writer_write;
    writer_class->meth_printf = moo_local_file_writer_printf;
    writer_class->meth_close = moo_local_file_writer_close;
}

static void
moo_local_file_writer_init (MooLocalFileWriter *writer)
{
    writer->file = NULL;
    writer->temp_filename = NULL;
    writer->filename = NULL;
    writer->error = NULL;
}


/************************************************************************/
/* MooStringWriter
 */

struct MooStringWriter {
    MooFileWriter base;
    GString *string;
};

MOO_DEFINE_TYPE_STATIC (MooStringWriter, moo_string_writer, MOO_TYPE_FILE_WRITER)

static gboolean
moo_string_writer_write (MooFileWriter *fwriter,
                         const char    *data,
                         gsize          len)
{
    MooStringWriter *writer = (MooStringWriter*) fwriter;
    g_string_append_len (writer->string, data, len);
    return TRUE;
}

G_GNUC_PRINTF (2, 0)
static gboolean
moo_string_writer_printf (MooFileWriter *fwriter,
                          const char    *fmt,
                          va_list        args)
{
    MooStringWriter *writer = (MooStringWriter*) fwriter;
    g_string_append_vprintf (writer->string, fmt, args);
    return TRUE;
}

static gboolean
moo_string_writer_close (MooFileWriter *fwriter,
                         G_GNUC_UNUSED GError **error)
{
    MooStringWriter *writer = (MooStringWriter*) fwriter;
    g_string_free (writer->string, TRUE);
    writer->string = NULL;
    return TRUE;
}

static void
moo_string_writer_class_init (MooStringWriterClass *klass)
{
    MooFileWriterClass *writer_class = MOO_FILE_WRITER_CLASS (klass);
    writer_class->meth_write = moo_string_writer_write;
    writer_class->meth_printf = moo_string_writer_printf;
    writer_class->meth_close = moo_string_writer_close;
}

static void
moo_string_writer_init (MooStringWriter *writer)
{
    writer->string = g_string_new (NULL);
}

MooFileWriter *
moo_string_writer_new (void)
{
    return g_object_new (MOO_TYPE_STRING_WRITER, NULL);
}

const char *
moo_string_writer_get_string (MooFileWriter *fwriter,
                              gsize         *len)
{
    MooStringWriter *writer = (MooStringWriter*) fwriter;

    g_return_val_if_fail (G_TYPE_CHECK_INSTANCE_TYPE ((writer), MOO_TYPE_STRING_WRITER), NULL);
    g_return_val_if_fail (writer->string != NULL, NULL);

    if (len)
        *len = writer->string->len;

    return writer->string->str;
}


#ifdef MOO_ENABLE_UNIT_TESTS

#include <mooutils/mooutils-tests.h>

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
    GError *error = NULL;

    dir = moo_test_get_working_dir ();
    my_dir = g_build_filename (dir, "cfg-writer", NULL);
    filename = g_build_filename (my_dir, "configfile", NULL);
    bak_filename = g_strdup_printf ("%s" BACKUP_EXTENSION, filename);

    writer = moo_config_writer_new (filename, TRUE, &error);
    TEST_ASSERT_MSG (writer != NULL,
                     "moo_cfg_writer_new failed: %s",
                     error ? error->message : "");
    if (error)
    {
        g_error_free (error);
        error = NULL;
    }

    if (writer)
    {
        moo_file_writer_write (writer, "first line\n", -1);
        moo_file_writer_printf (writer, "second line #%d\n", 2);
        moo_file_writer_write (writer, "third\nlalalala\n", 6);
        TEST_ASSERT_MSG (moo_file_writer_close (writer, &error),
                         "moo_file_writer_close failed: %s",
                         error ? error->message : "");
        if (error)
        {
            g_error_free (error);
            error = NULL;
        }

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
                     error ? error->message : "");
    if (writer)
    {
        moo_file_writer_write (writer, "First line\n", -1);
        moo_file_writer_printf (writer, "Second line #%d\n", 2);
        moo_file_writer_write (writer, "Third\nlalalala\n", 6);
        TEST_ASSERT_MSG (moo_file_writer_close (writer, &error),
                         "moo_file_writer_close failed: %s",
                         error ? error->message : "");
        if (error)
        {
            g_error_free (error);
            error = NULL;
        }

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
    TEST_ASSERT (error != NULL);

    if (error)
        g_error_free (error);
    error = NULL;

    g_free (bak_filename);
    g_free (filename);
    g_free (my_dir);
}

void
moo_test_moo_file_writer (void)
{
    MooTestSuite *suite;

    suite = moo_test_suite_new ("MooFileWriter", NULL, NULL, NULL);

    moo_test_suite_add_test (suite, "test of MooFileWriter",
                             (MooTestFunc) test_moo_file_writer, NULL);
}

#endif /* MOO_ENABLE_UNIT_TESTS */
