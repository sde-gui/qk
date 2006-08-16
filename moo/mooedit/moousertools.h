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


/* file == NULL means find one in data dirs */
void    moo_edit_load_user_tools        (const char *file,
                                         MooUIXML   *xml);
void    moo_edit_load_user_menu         (const char *file,
                                         MooUIXML   *xml);


G_END_DECLS

#endif /* __MOO_USER_TOOLS_H__ */
