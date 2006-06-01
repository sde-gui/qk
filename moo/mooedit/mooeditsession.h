/*
 *   mooeditsession.h
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

#ifndef __MOO_EDIT_SESSION_H__
#define __MOO_EDIT_SESSION_H__

#include <mooedit/mooeditwindow.h>

G_BEGIN_DECLS


#define MOO_TYPE_EDIT_SESSION (moo_edit_session_get_type ())
typedef struct _MooEditSession MooEditSession;


#define MOO_EDIT_SESSION_ELM_SESSION        "session"
#define MOO_EDIT_SESSION_ELM_WINDOW         "window"
#define MOO_EDIT_SESSION_ELM_DOC            "doc"
#define MOO_EDIT_SESSION_PROP_ENCODING      "encoding"
#define MOO_EDIT_SESSION_PROP_LINE          "line"
#define MOO_EDIT_SESSION_PROP_ACTIVE_DOC    "active-doc"


GType            moo_edit_session_get_type      (void) G_GNUC_CONST;

MooEditSession  *moo_edit_session_copy          (MooEditSession *session);
void             moo_edit_session_free          (MooEditSession *session);

MooEditSession  *moo_edit_session_parse_markup  (const char     *markup);
MooEditSession  *moo_edit_session_parse_node    (MooMarkupNode  *node);
char            *moo_edit_session_get_markup    (MooEditSession *session);


G_END_DECLS

#endif /* __MOO_EDIT_SESSION_H__ */
