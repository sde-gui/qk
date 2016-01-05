/**
* boxed:MooOpenInfo: information for opening a file
 *
 * Object which contains filename, character encoding, line
 * number, and options to use in moo_editor_open_file().
 **/

/**
 * boxed:MooSaveInfo: information for saving a file
 *
 * Object which contains a filename and character encoding to
 * use in moo_editor_save() and moo_editor_save_as().
 **/

/**
 * boxed:MooReloadInfo: information for reloading a file
 *
 * Object which contains character encoding and line number to
 * use in moo_editor_reload().
 **/

#include "mooeditfileinfo-impl.h"
#include <mooutils/mooutils-misc.h>
#include <moocpp/gboxed.h>

using namespace moo;

MOO_DEFINE_BOXED_CPP_TYPE(MooOpenInfo, moo_open_info)
MOO_DEFINE_BOXED_CPP_TYPE(MooSaveInfo, moo_save_info)
MOO_DEFINE_BOXED_CPP_TYPE(MooReloadInfo, moo_reload_info)

MOO_DEFINE_PTR_ARRAY_FULL(MooOpenInfoArray, moo_open_info_array, MooOpenInfo,
                          gboxed_helper<MooOpenInfo>::array_elm_copy,
                          gboxed_helper<MooOpenInfo>::array_elm_free)

/**
 * moo_open_info_new_file: (static-method-of MooOpenInfo) (moo-kwargs)
 *
 * @file:
 * @encoding: (type const-utf8) (allow-none) (default NULL)
 * @line: (type index) (default -1)
 * @flags: (default 0)
 *
 * Returns: (transfer full)
 **/
MooOpenInfo *
moo_open_info_new_file (GFile       *file,
                        const char  *encoding,
                        int          line,
                        MooOpenFlags flags)
{
    g_return_val_if_fail (G_IS_FILE (file), nullptr);
    return new MooOpenInfo(file, encoding, line, flags);
}

/**
 * moo_open_info_new: (constructor-of MooOpenInfo) (moo-kwargs)
 *
 * @path: (type const-filename)
 * @encoding: (type const-utf8) (allow-none) (default NULL)
 * @line: (type index) (default -1)
 * @flags: (default 0)
 *
 * Returns: (transfer full)
 **/
MooOpenInfo *
moo_open_info_new (const char  *path,
                   const char  *encoding,
                   int          line,
                   MooOpenFlags flags)
{
    g::FilePtr file = g::File::new_for_path(path);
    return moo_open_info_new_file (file.gobj(), encoding, line, flags);
}

/**
 * moo_open_info_new_uri: (static-method-of MooOpenInfo) (moo-kwargs)
 *
 * @uri: (type const-utf8)
 * @encoding: (type const-utf8) (allow-none) (default NULL)
 * @line: (type index) (default -1)
 * @flags: (default 0)
 *
 * Returns: (transfer full)
 **/
MooOpenInfo *
moo_open_info_new_uri (const char  *uri,
                       const char  *encoding,
                       int          line,
                       MooOpenFlags flags)
{
    g::FilePtr file = g::File::new_for_uri(uri);
    return moo_open_info_new_file (file.gobj(), encoding, line, flags);
}

/**
 * moo_open_info_dup:
 *
 * Returns: (transfer full)
 **/
MooOpenInfo *
moo_open_info_dup (MooOpenInfo *info)
{
    g_return_val_if_fail(info != nullptr, nullptr);
    return new MooOpenInfo(*info);
}


/**
 * moo_open_info_get_filename: (moo.private 1)
 *
 * Returns: (type filename)
 **/
char *
moo_open_info_get_filename (MooOpenInfo *info)
{
    g_return_val_if_fail(info != nullptr, nullptr);
    return info->file->get_path().release_owned();
}

/**
 * moo_open_info_get_uri: (moo.private 1)
 *
 * Returns: (type utf8)
 **/
char *
moo_open_info_get_uri (MooOpenInfo *info)
{
    g_return_val_if_fail(info != nullptr, nullptr);
    return info->file->get_uri().release_owned();
}

/**
 * moo_open_info_get_file: (moo.private 1)
 *
 * Returns: (transfer full)
 **/
GFile *
moo_open_info_get_file (MooOpenInfo *info)
{
    g_return_val_if_fail(info != nullptr, nullptr);
    return info->file->dup().release();
}

/**
 * moo_open_info_get_encoding: (moo.private 1)
 *
 * Returns: (type const-utf8)
 **/
const char *
moo_open_info_get_encoding (MooOpenInfo *info)
{
    g_return_val_if_fail(info != nullptr, nullptr);
    return info->encoding;
}

/**
 * moo_open_info_set_encoding: (moo.private 1)
 *
 * @info:
 * @encoding: (type const-utf8) (allow-none)
 **/
void
moo_open_info_set_encoding (MooOpenInfo *info,
                            const char  *encoding)
{
    g_return_if_fail(info != nullptr);
    info->encoding.copy(encoding);
}

/**
 * moo_open_info_get_line:
 *
 * Returns: (type index)
 **/
int
moo_open_info_get_line (MooOpenInfo *info)
{
    g_return_val_if_fail(info != nullptr, -1);
    return info->line;
}

/**
 * moo_open_info_set_line:
 *
 * @info:
 * @line: (type index)
 **/
void
moo_open_info_set_line (MooOpenInfo *info,
                        int          line)
{
    g_return_if_fail(info != nullptr);
    info->line = line;
}

/**
 * moo_open_info_get_flags:
 **/
MooOpenFlags
moo_open_info_get_flags (MooOpenInfo *info)
{
    g_return_val_if_fail(info != nullptr, MOO_OPEN_FLAGS_NONE);
    return info->flags;
}

/**
 * moo_open_info_set_flags:
 **/
void
moo_open_info_set_flags(MooOpenInfo  *info,
                        MooOpenFlags  flags)
{
    g_return_if_fail(info != nullptr);
    info->flags = flags;
}

/**
 * moo_open_info_add_flags:
 **/
void
moo_open_info_add_flags(MooOpenInfo  *info,
                        MooOpenFlags  flags)
{
    g_return_if_fail(info != nullptr);
    info->flags |= flags;
}


/**
 * moo_save_info_new_file: (static-method-of MooSaveInfo)
 *
 * @file:
 * @encoding: (type const-utf8) (allow-none) (default NULL)
 *
 * Returns: (transfer full)
 **/
MooSaveInfo *
moo_save_info_new_file(GFile      *file,
                       const char *encoding)
{
    g_return_val_if_fail(G_IS_FILE(file), nullptr);
    return new MooSaveInfo(file, encoding);
}

/**
 * moo_save_info_new: (constructor-of MooSaveInfo)
 *
 * @path: (type const-filename)
 * @encoding: (type const-utf8) (allow-none) (default NULL)
 *
 * Returns: (transfer full)
 **/
MooSaveInfo *
moo_save_info_new(const char *path,
                  const char *encoding)
{
    auto file = g::File::new_for_path(path);
    MooSaveInfo *info = moo_save_info_new_file(file.gobj(), encoding);
    return info;
}

/**
 * moo_save_info_new_uri: (static-method-of MooSaveInfo)
 *
 * @uri: (type const-utf8)
 * @encoding: (type const-utf8) (allow-none) (default NULL)
 *
 * Returns: (transfer full)
 **/
MooSaveInfo *
moo_save_info_new_uri (const char *uri,
                       const char *encoding)
{
    auto file = g::File::new_for_uri(uri);
    MooSaveInfo *info = moo_save_info_new_file(file.gobj(), encoding);
    return info;
}

/**
 * moo_save_info_dup:
 *
 * Returns: (transfer full)
 **/
MooSaveInfo *
moo_save_info_dup (MooSaveInfo *info)
{
    g_return_val_if_fail(info != nullptr, nullptr);
    return new MooSaveInfo(*info);
}


/**
 * moo_reload_info_new: (constructor-of MooReloadInfo)
 *
 * @encoding: (type const-utf8) (allow-none) (default NULL)
 * @line: (type index) (default -1)
 **/
MooReloadInfo *
moo_reload_info_new (const char *encoding,
                     int         line)
{
    return new MooReloadInfo(encoding, line);
}

/**
 * moo_reload_info_dup:
 *
 * Returns: (transfer full)
 **/
MooReloadInfo *
moo_reload_info_dup (MooReloadInfo *info)
{
    g_return_val_if_fail(info != nullptr, nullptr);
    return new MooReloadInfo(*info);
}

/**
 * moo_reload_info_get_line:
 *
 * Returns: (type index)
 **/
int
moo_reload_info_get_line (MooReloadInfo *info)
{
    g_return_val_if_fail (info != nullptr, -1);
    return info->line;
}

/**
 * moo_reload_info_set_line:
 *
 * @info:
 * @line: (type index)
 **/
void
moo_reload_info_set_line (MooReloadInfo *info,
                          int            line)
{
    g_return_if_fail (info != nullptr);
    info->line = line;
}
