/*
 *   moousertools.h
 *
 *   Copyright (C) 2004-2006 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef __MOO_USER_TOOLS_H__
#define __MOO_USER_TOOLS_H__

#include <mooutils/moouixml.h>
#include <mooedit/moocommand.h>

G_BEGIN_DECLS


typedef enum {
    MOO_TOOL_FILE_TOOLS,
    MOO_TOOL_FILE_MENU
} MooToolFileType;

typedef enum {
    MOO_TOOL_POS_START,
    MOO_TOOL_POS_END
} MooToolPosition;

typedef enum {
    MOO_TOOL_UNIX,
    MOO_TOOL_WINDOWS
} MooToolOSType;

typedef struct {
    const char     *id;
    const char     *name;
    const char     *label;
    const char     *accel;
    const char     *menu;
    const char     *langs;
    const char     *options;
    MooToolPosition position;
    gboolean        enabled;
    MooToolOSType   os_type;
    const char     *cmd_type;
    MooCommandData *cmd_data;
    MooToolFileType type;
    const char     *file;
} MooToolLoadInfo;

void    _moo_edit_load_user_tools       (MooToolFileType         type,
                                         MooUIXML               *xml);

typedef void (*MooToolFileParseFunc)    (MooToolLoadInfo        *info,
                                         gpointer                data);

void    _moo_edit_parse_user_tools      (MooToolFileType         type,
                                         MooToolFileParseFunc    func,
                                         gpointer                data);
void    _moo_edit_save_user_tools       (MooToolFileType         type,
                                         MooMarkupDoc           *doc);


G_END_DECLS

#endif /* __MOO_USER_TOOLS_H__ */
