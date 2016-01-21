#pragma once

#include "mooeditfileinfo.h"
#include "moocpp/strutils.h"
#include "moocpp/gobjtypes.h"

using namespace moo;

struct MooOpenInfo
{
    g::FilePtr file;
    gstr encoding;
    int line;
    MooOpenFlags flags;

    MooOpenInfo(GFile* file, const char* encoding, int line, MooOpenFlags flags)
        : file(wrap_new(g_file_dup(file)))
        , encoding(gstr::wrap(encoding))
        , line(line)
        , flags(flags)
    {
    }

    MooOpenInfo(const MooOpenInfo& other)
        : file(other.file->dup())
        , encoding(other.encoding)
        , line(other.line)
        , flags(other.flags)
    {
    }

    MooOpenInfo& operator=(const MooOpenInfo&) = delete;
    MooOpenInfo(MooOpenInfo&&) = delete;
    MooOpenInfo& operator=(MooOpenInfo&&) = delete;
};

struct MooReloadInfo : public GObject
{
    MooReloadInfo(const char* encoding, int line)
        : encoding(gstr::wrap(encoding))
        , line(line)
    {
    }

    MooReloadInfo(const MooReloadInfo& other)
        : encoding(other.encoding)
        , line(other.line)
    {
    }

    gstr encoding;
    int line;

    MooReloadInfo(MooReloadInfo&&) = delete;
};

struct MooSaveInfo : public GObject
{
    g::FilePtr file;
    gstr encoding;

    MooSaveInfo(GFile* file, const char* encoding)
        : file(wrap_new(g_file_dup(file)))
        , encoding(gstr::wrap(encoding))
    {
    }

    MooSaveInfo(const MooSaveInfo& other)
        : file(other.file->dup())
        , encoding(other.encoding)
    {
    }

    MooSaveInfo(MooSaveInfo&&) = delete;
};
