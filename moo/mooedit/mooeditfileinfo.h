/*
 *   mooeditfileinfo.h
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

#ifndef MOO_EDIT_FILE_INFO_H
#define MOO_EDIT_FILE_INFO_H

#include <gio/gio.h>
#include <mooedit/mooedittypes.h>

G_BEGIN_DECLS

#define MOO_TYPE_EDIT_OPEN_INFO                       (moo_edit_open_info_get_type ())
#define MOO_EDIT_OPEN_INFO(object)                    (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_EDIT_OPEN_INFO, MooEditOpenInfo))
#define MOO_EDIT_OPEN_INFO_CLASS(klass)               (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_EDIT_OPEN_INFO, MooEditOpenInfoClass))
#define MOO_IS_EDIT_OPEN_INFO(object)                 (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_EDIT_OPEN_INFO))
#define MOO_IS_EDIT_OPEN_INFO_CLASS(klass)            (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_EDIT_OPEN_INFO))
#define MOO_EDIT_OPEN_INFO_GET_CLASS(obj)             (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_EDIT_OPEN_INFO, MooEditOpenInfoClass))

#define MOO_TYPE_EDIT_SAVE_INFO                       (moo_edit_save_info_get_type ())
#define MOO_EDIT_SAVE_INFO(object)                    (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_EDIT_SAVE_INFO, MooEditSaveInfo))
#define MOO_EDIT_SAVE_INFO_CLASS(klass)               (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_EDIT_SAVE_INFO, MooEditSaveInfoClass))
#define MOO_IS_EDIT_SAVE_INFO(object)                 (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_EDIT_SAVE_INFO))
#define MOO_IS_EDIT_SAVE_INFO_CLASS(klass)            (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_EDIT_SAVE_INFO))
#define MOO_EDIT_SAVE_INFO_GET_CLASS(obj)             (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_EDIT_SAVE_INFO, MooEditSaveInfoClass))

#define MOO_TYPE_EDIT_RELOAD_INFO                       (moo_edit_reload_info_get_type ())
#define MOO_EDIT_RELOAD_INFO(object)                    (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_EDIT_RELOAD_INFO, MooEditReloadInfo))
#define MOO_EDIT_RELOAD_INFO_CLASS(klass)               (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_EDIT_RELOAD_INFO, MooEditReloadInfoClass))
#define MOO_IS_EDIT_RELOAD_INFO(object)                 (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_EDIT_RELOAD_INFO))
#define MOO_IS_EDIT_RELOAD_INFO_CLASS(klass)            (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_EDIT_RELOAD_INFO))
#define MOO_EDIT_RELOAD_INFO_GET_CLASS(obj)             (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_EDIT_RELOAD_INFO, MooEditReloadInfoClass))

typedef struct MooEditOpenInfoClass MooEditOpenInfoClass;
typedef struct MooEditSaveInfoClass MooEditSaveInfoClass;
typedef struct MooEditReloadInfoClass MooEditReloadInfoClass;

typedef enum {
    MOO_EDIT_OPEN_NEW_WINDOW = 1 << 0,
    MOO_EDIT_OPEN_NEW_TAB    = 1 << 1,
    MOO_EDIT_OPEN_RELOAD     = 1 << 2,
    MOO_EDIT_OPEN_CREATE_NEW = 1 << 3
} MooEditOpenFlags;

struct MooEditOpenInfo
{
    GObject parent;

    GFile *file;
    char *encoding;
    int line;
    MooEditOpenFlags flags;
};

struct MooEditOpenInfoClass
{
    GObjectClass parent_class;
};

struct MooEditReloadInfo {
    GObject parent;

    char *encoding;
    int line;
};

struct MooEditReloadInfoClass
{
    GObjectClass parent_class;
};

struct MooEditSaveInfo {
    GObject parent;

    GFile *file;
    char *encoding;
};

struct MooEditSaveInfoClass
{
    GObjectClass parent_class;
};

GType                moo_edit_open_info_get_type    (void) G_GNUC_CONST;
GType                moo_edit_reload_info_get_type  (void) G_GNUC_CONST;
GType                moo_edit_save_info_get_type    (void) G_GNUC_CONST;

MooEditOpenInfo     *moo_edit_open_info_new         (GFile              *file,
                                                     const char         *encoding);
MooEditOpenInfo     *moo_edit_open_info_new_path    (const char         *path,
                                                     const char         *encoding);
MooEditOpenInfo     *moo_edit_open_info_new_uri     (const char         *uri,
                                                     const char         *encoding);
MooEditOpenInfo     *moo_edit_open_info_dup         (MooEditOpenInfo    *info);

MooEditReloadInfo   *moo_edit_reload_info_new       (const char         *encoding);
MooEditReloadInfo   *moo_edit_reload_info_dup       (MooEditReloadInfo  *info);

MooEditSaveInfo     *moo_edit_save_info_new         (GFile              *file,
                                                     const char         *encoding);
MooEditSaveInfo     *moo_edit_save_info_new_path    (const char         *path,
                                                     const char         *encoding);
MooEditSaveInfo     *moo_edit_save_info_new_uri     (const char         *uri,
                                                     const char         *encoding);
MooEditSaveInfo     *moo_edit_save_info_dup         (MooEditSaveInfo    *info);

G_END_DECLS

#endif /* MOO_EDIT_FILE_INFO_H */
