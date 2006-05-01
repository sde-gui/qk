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

G_BEGIN_DECLS


#define MOO_USER_TOOLS_FILE             "tools.cfg"
#define MOO_USER_MENU_FILE              "menu.cfg"

#define MOO_USER_TOOL_KEY_OS            "os"
#define MOO_USER_TOOL_KEY_ACTION        "action"
#define MOO_USER_TOOL_KEY_LABEL         "label"
#define MOO_USER_TOOL_KEY_ACCEL         "accel"
#define MOO_USER_TOOL_KEY_POSITION      "position"
#define MOO_USER_TOOL_KEY_OPTIONS       "options"
#define MOO_USER_TOOL_KEY_COMMAND       "command"
#define MOO_USER_TOOL_KEY_LANG          "lang"
#define MOO_USER_TOOL_KEY_ENABLED       "enabled"
#define MOO_USER_TOOL_KEY_MENU          "menu"

#define MOO_USER_TOOL_POSITION_START    "start"
#define MOO_USER_TOOL_POSITION_END      "end"

#define MOO_USER_TOOL_OPTION_NEED_SAVE  "need-save"
#define MOO_USER_TOOL_OPTION_NEED_FILE  "need-file"
#define MOO_USER_TOOL_OPTION_NEED_DOC   "need-doc"
#define MOO_USER_TOOL_OPTION_SILENT     "silent"


void    moo_edit_get_user_tools_files   (char     ***default_files,
                                         guint      *n_files,
                                         char      **user_file);
void    moo_edit_load_user_tools        (char      **default_files,
                                         guint       n_files,
                                         char       *user_file,
                                         MooUIXML   *xml);

void    moo_edit_get_user_menu_files    (char     ***default_files,
                                         guint      *n_files,
                                         char      **user_file);
void    moo_edit_load_user_menu         (char      **default_files,
                                         guint       n_files,
                                         char       *user_file,
                                         MooUIXML   *xml);


G_END_DECLS

#endif /* __MOO_USER_TOOLS_H__ */
