#pragma once

#include "mooeditfileinfo.h"
#include "moocpp/strutils.h"
#include "moocpp/gobjectutils.h"

struct MooOpenInfo : public GObject
{
    MooOpenInfo()
        : line(- 1)
    {
    }

    moo::gobj_ptr<GFile> file;
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
    MooReloadInfo()
        : line(-1)
    {
    }

    moo::gstr encoding;
    int line;
};

struct MooReloadInfoClass
{
    GObjectClass parent_class;
};

struct MooSaveInfo : public GObject
{
    moo::gobj_ptr<GFile> file;
    moo::gstr encoding;
};

struct MooSaveInfoClass
{
    GObjectClass parent_class;
};
