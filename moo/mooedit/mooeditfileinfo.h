/*
 *   mooeditfileinfo.h
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

#include <mooedit/mooedittypes.h>
#include <moocpp/moocpp.h>

G_BEGIN_DECLS

#define MOO_TYPE_OPEN_INFO                       (moo_open_info_get_type ())
#define MOO_TYPE_SAVE_INFO                       (moo_save_info_get_type ())
#define MOO_TYPE_RELOAD_INFO                     (moo_reload_info_get_type ())

MooOpenInfo         *moo_open_info_new          (const char         *path,
                                                 const char         *encoding,
                                                 int                 line,
                                                 MooOpenFlags        flags);
MooOpenInfo         *moo_open_info_new_file     (GFile              *file,
                                                 const char         *encoding,
                                                 int                 line,
                                                 MooOpenFlags        flags);
MooOpenInfo         *moo_open_info_new_uri      (const char         *uri,
                                                 const char         *encoding,
                                                 int                 line,
                                                 MooOpenFlags        flags);
MooOpenInfo         *moo_open_info_dup          (MooOpenInfo        *info);

char                *moo_open_info_get_filename (MooOpenInfo        *info);
char                *moo_open_info_get_uri      (MooOpenInfo        *info);
GFile               *moo_open_info_get_file     (MooOpenInfo        *info);
char                *moo_open_info_get_uri      (MooOpenInfo        *info);
const char          *moo_open_info_get_encoding (MooOpenInfo        *info);
void                 moo_open_info_set_encoding (MooOpenInfo        *info,
                                                 const char         *encoding);
int                  moo_open_info_get_line     (MooOpenInfo        *info);
void                 moo_open_info_set_line     (MooOpenInfo        *info,
                                                 int                 line);
MooOpenFlags         moo_open_info_get_flags    (MooOpenInfo        *info);
void                 moo_open_info_set_flags    (MooOpenInfo        *info,
                                                 MooOpenFlags        flags);
void                 moo_open_info_add_flags    (MooOpenInfo        *info,
                                                 MooOpenFlags        flags);

MooReloadInfo       *moo_reload_info_new        (const char         *encoding,
                                                 int                 line);
MooReloadInfo       *moo_reload_info_dup        (MooReloadInfo      *info);

int                  moo_reload_info_get_line   (MooReloadInfo      *info);
void                 moo_reload_info_set_line   (MooReloadInfo      *info,
                                                 int                 line);

MooSaveInfo         *moo_save_info_new          (const char         *path,
                                                 const char         *encoding);
MooSaveInfo         *moo_save_info_new_file     (GFile              *file,
                                                 const char         *encoding);
MooSaveInfo         *moo_save_info_new_uri      (const char         *uri,
                                                 const char         *encoding);
MooSaveInfo         *moo_save_info_dup          (MooSaveInfo        *info);

G_END_DECLS
