/*
 *   mooeditbuffer-impl.h
 *
 *   Copyright (C) 2004-2010 by Yevgen Muntyan <emuntyan@sourceforge.net>
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

#ifndef MOOEDIT_COMPILATION
#error "This file may not be used"
#endif

#ifndef MOO_EDIT_BUFFER_IMPL_H
#define MOO_EDIT_BUFFER_IMPL_H

#include <mooedit/mooeditbuffer.h>

G_BEGIN_DECLS

void        _moo_edit_buffer_set_view   (MooEditBuffer  *buffer,
                                         MooEditView    *view);

G_END_DECLS

#endif /* MOO_EDIT_BUFFER_IMPL_H */
