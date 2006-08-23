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


#define MOO_TYPE_USER_TOOL_INFO (_moo_user_tool_info_get_type ())

typedef enum {
    MOO_USER_TOOL_MENU,
    MOO_USER_TOOL_CONTEXT
} MooUserToolType;

typedef enum {
    MOO_USER_TOOL_POS_END,
    MOO_USER_TOOL_POS_START
} MooUserToolPosition;

typedef enum {
    MOO_USER_TOOL_UNIX,
    MOO_USER_TOOL_WIN32
} MooUserToolOSType;

#ifdef G_OS_WIN32
#define MOO_USER_TOOL_THIS_OS MOO_USER_TOOL_WIN32
#else
#define MOO_USER_TOOL_THIS_OS MOO_USER_TOOL_UNIX
#endif

typedef struct {
    char                *id;
    char                *name;
    char                *accel;
    char                *menu;
    char                *langs;
    char                *options;
    MooUserToolPosition  position;
    gboolean             enabled;
    MooUserToolOSType    os_type;
    MooCommandType      *cmd_type;
    MooCommandData      *cmd_data;
    MooUserToolType      type;
    char                *file;
    guint                ref_count;
} MooUserToolInfo;

GType _moo_user_tool_info_get_type (void) G_GNUC_CONST;

MooUserToolInfo *_moo_user_tool_info_new    (void);
MooUserToolInfo *_moo_user_tool_info_ref    (MooUserToolInfo *info);
void             _moo_user_tool_info_unref  (MooUserToolInfo *info);

void    _moo_edit_load_user_tools       (MooUserToolType         type);

typedef void (*MooToolFileParseFunc)    (MooUserToolInfo        *info,
                                         gpointer                data);

/* caller must free the list and unref() the contents */
GSList *_moo_edit_parse_user_tools      (MooUserToolType         type);
void    _moo_edit_save_user_tools       (MooUserToolType         type,
                                         const GSList           *info);


G_END_DECLS

#endif /* __MOO_USER_TOOLS_H__ */
