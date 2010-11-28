/*
 *   mooeditbuffer.h
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

#ifndef MOO_EDIT_BUFFER_H
#define MOO_EDIT_BUFFER_H

#include <mooedit/mootextbuffer.h>
#include <mooedit/mooedittypes.h>

G_BEGIN_DECLS


#define MOO_TYPE_EDIT_BUFFER                       (moo_edit_buffer_get_type ())
#define MOO_EDIT_BUFFER(object)                    (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_EDIT_BUFFER, MooEditBuffer))
#define MOO_EDIT_BUFFER_CLASS(klass)               (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_EDIT_BUFFER, MooEditBufferClass))
#define MOO_IS_EDIT_BUFFER(object)                 (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_EDIT_BUFFER))
#define MOO_IS_EDIT_BUFFER_CLASS(klass)            (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_EDIT_BUFFER))
#define MOO_EDIT_BUFFER_GET_CLASS(obj)             (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_EDIT_BUFFER, MooEditBufferClass))

typedef struct _MooEditBufferPrivate  MooEditBufferPrivate;
typedef struct _MooEditBufferClass    MooEditBufferClass;

struct _MooEditBuffer
{
    MooTextBuffer parent;
    MooEditBufferPrivate *priv;
};

struct _MooEditBufferClass
{
    MooTextBufferClass parent_class;
};


GType            moo_edit_buffer_get_type       (void) G_GNUC_CONST;

MooEditView     *moo_edit_buffer_get_view       (MooEditBuffer    *buffer);
MooEdit         *moo_edit_buffer_get_doc        (MooEditBuffer    *buffer);


G_END_DECLS

#endif /* MOO_EDIT_BUFFER_H */
