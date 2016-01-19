/*
 *   moofilewriter-private.h
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

#include "mooutils/moofilewriter.h"

struct MooFileReader
{
    MooFileReader();
    ~MooFileReader();

    void close_file();

    MOO_DISABLE_COPY_OPS(MooFileReader);

    MGW_FILE *file;
};

struct MooFileWriter
{
    virtual ~MooFileWriter() {}

    virtual bool write  (const char*    data,
                         gsize          len) = 0;
    virtual bool printf (const char*    fmt,
                         va_list        args) G_GNUC_PRINTF (2, 0) = 0;
    virtual bool close  (moo::gerrp&    error) = 0;
};

struct MooLocalFileWriter : public MooFileWriter
{
    MooLocalFileWriter();
    ~MooLocalFileWriter();

    bool write  (const char*    data,
                 gsize          len) override;
    bool printf (const char*    fmt,
                 va_list        args) G_GNUC_PRINTF (2, 0) override;
    bool close  (moo::gerrp&    error) override;

    MOO_DISABLE_COPY_OPS(MooLocalFileWriter);

    moo::g::FilePtr file;
    moo::g::FileOutputStreamPtr stream;
    MooFileWriterFlags flags;
    moo::gerrp error;
};

struct MooStringWriter : public MooFileWriter
{
    MooStringWriter();
    ~MooStringWriter();

    bool write  (const char*    data,
                 gsize          len) override;
    bool printf (const char*    fmt,
                 va_list        args) G_GNUC_PRINTF (2, 0) override;
    bool close  (moo::gerrp&    error) override;

    MOO_DISABLE_COPY_OPS(MooStringWriter);

    GString *string;
};
