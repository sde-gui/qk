/*
 *   mooedit-fileops.h
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

#ifndef __cplusplus
#error "This is a C++ header"
#endif

#include "mooedit/mooedit.h"
#include <gio/gio.h>
#include "moocpp/utils.h"
#include "moocpp/strutils.h"

G_BEGIN_DECLS
const char *_moo_get_default_encodings (void);
G_END_DECLS

typedef enum {
    MOO_EDIT_SAVE_FLAGS_NONE = 0,
    MOO_EDIT_SAVE_BACKUP = 1 << 0
} MooEditSaveFlags;

#define MOO_EDIT_FILE_ERROR (_moo_edit_file_error_quark ())

enum {
    MOO_EDIT_FILE_ERROR_ENCODING = 1,
    MOO_EDIT_FILE_ERROR_FAILED,
    MOO_EDIT_FILE_ERROR_NOT_IMPLEMENTED,
    MOO_EDIT_FILE_ERROR_NOENT,
    MOO_EDIT_FILE_ERROR_CANCELLED
};
MOO_DEFINE_FLAGS(MooEditSaveFlags)

GQuark           _moo_edit_file_error_quark     (void) G_GNUC_CONST;

bool             _moo_is_file_error_cancelled   (GError*                error);

bool             _moo_edit_file_is_new          (const moo::g::File&    file);
bool             _moo_edit_load_file            (Edit&                  edit,
                                                 const moo::g::File&    file,
                                                 const moo::gstr&       init_encoding,
                                                 const moo::gstr&       init_cached_encoding,
                                                 GError**               error);
bool             _moo_edit_reload_file          (Edit                   edit,
                                                 const char*            encoding,
                                                 GError**               error);
bool             _moo_edit_save_file            (Edit&                  edit,
                                                 const moo::g::File&    floc,
                                                 const char*            encoding,
                                                 MooEditSaveFlags       flags,
                                                 GError**               error);
bool             _moo_edit_save_file_copy       (Edit                   edit,
                                                 const moo::g::File&    file,
                                                 const char*            encoding,
                                                 MooEditSaveFlags       flags,
                                                 GError**               error);
