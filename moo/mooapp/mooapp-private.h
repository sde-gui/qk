/*
 *   mooapp/mooapp-private.h
 *
 *   Copyright (C) 2004-2008 by Yevgen Muntyan <muntyan@tamu.edu>
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

#ifndef MOO_APP_COMPILATION
#error "This file may not be used"
#endif

#ifndef MOO_APP_PRIVATE_H
#define MOO_APP_PRIVATE_H

#include "mooapp.h"

G_BEGIN_DECLS


typedef enum
{
    MOO_APP_CMD_ZERO = 0,
    MOO_APP_CMD_PYTHON_STRING,
    MOO_APP_CMD_PYTHON_FILE,
    MOO_APP_CMD_SCRIPT,
    MOO_APP_CMD_OPEN_FILE,
    MOO_APP_CMD_OPEN_URIS,
    MOO_APP_CMD_QUIT,
    MOO_APP_CMD_DIE,
    MOO_APP_CMD_PRESENT,
    MOO_APP_CMD_LAST
} MooAppCmdCode;


#if defined(WANT_MOO_APP_CMD_STRINGS) || defined(WANT_MOO_APP_CMD_CHARS)

/* 'g' is taken by ggap */
#define CMD_ZERO            "\0"
#define CMD_PYTHON_STRING   "p"
#define CMD_PYTHON_FILE     "P"
#define CMD_SCRIPT          "s"
#define CMD_OPEN_FILE       "f"
#define CMD_OPEN_URIS       "u"
#define CMD_QUIT            "q"
#define CMD_DIE             "d"
#define CMD_PRESENT         "r"

#endif

#ifdef WANT_MOO_APP_CMD_CHARS

static const char *moo_app_cmd_chars =
    CMD_ZERO
    CMD_PYTHON_STRING
    CMD_PYTHON_FILE
    CMD_SCRIPT
    CMD_OPEN_FILE
    CMD_OPEN_URIS
    CMD_QUIT
    CMD_DIE
    CMD_PRESENT
;

#endif /* WANT_MOO_APP_CMD_CHARS */


GtkWidget       *_moo_app_create_prefs_dialog   (MooApp     *app);


G_END_DECLS

#endif /* MOO_APP_PRIVATE_H */
