#pragma once

#include "mooeditfileinfo.h"
#include "moocpp/strutils.h"
#include "moocpp/gobjptrtypes.h"

struct MooOpenInfo : public GObject
{
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

MOO_GOBJ_TYPEDEFS(OpenInfo, MooOpenInfo);
MOO_GOBJ_TYPEDEFS(ReloadInfo, MooReloadInfo);
MOO_GOBJ_TYPEDEFS(SaveInfo, MooSaveInfo);
