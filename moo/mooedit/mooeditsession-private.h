/*
 *   mooeditsession-private.h
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

#ifndef MOOEDIT_COMPILATION
#error "This file may not be used directly"
#endif

#ifndef __MOO_EDIT_SESSION_PRIVATE_H__
#define __MOO_EDIT_SESSION_PRIVATE_H__

#include "mooedit/mooeditor.h"

G_BEGIN_DECLS


MooEditSession  *_moo_edit_session_new          (void);
void             _moo_edit_session_add_window   (MooEditWindow  *window);
gboolean         _moo_edit_session_load         (MooEditSession *session,
                                                 MooEditor      *editor,
                                                 MooEditWindow  *window);


G_END_DECLS

#endif /* __MOO_EDIT_SESSION_PRIVATE_H__ */
