#pragma once

#include "mooeditfileinfo.h"
#include "moocpp/strutils.h"
#include "moocpp/gobjptrtypes.h"

struct MooOpenInfo : public GObject
{
    moo::gobjptr<GFile> file;
    moo::gstr encoding;
    int line;
    MooOpenFlags flags;
};

struct MooOpenInfoClass
{
    GObjectClass parent_class;
};

struct MooReloadInfo : public GObject
{
    moo::gstr encoding;
    int line;
};

struct MooReloadInfoClass
{
    GObjectClass parent_class;
};

struct MooSaveInfo : public GObject
{
    moo::gobjptr<GFile> file;
    moo::gstr encoding;
};

struct MooSaveInfoClass
{
    GObjectClass parent_class;
};
