/*
 *   moouseractions.h
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

#ifndef __MOO_USER_ACTIONS_H__
#define __MOO_USER_ACTIONS_H__

#include <mooutils/moocommand.h>
#include <mooutils/moowindow.h>

G_BEGIN_DECLS


typedef void (*MooUserActionSetup)      (MooCommand *command,
                                         MooWindow  *window);

void moo_parse_user_actions (const char         *file,
                             MooUserActionSetup  setup);


G_END_DECLS

#endif /* __MOO_USER_ACTIONS_H__ */
