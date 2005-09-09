/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *   mootextbuffer.h
 *
 *   Copyright (C) 2004-2005 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef __MOO_TEXT_BUFFER_H__
#define __MOO_TEXT_BUFFER_H__

#include <gtksourceview/gtksourcebuffer.h>
#include "mooedit/mooeditlang.h"

G_BEGIN_DECLS


#define MOO_TYPE_TEXT_BUFFER              (moo_text_buffer_get_type ())
#define MOO_TEXT_BUFFER(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_TEXT_BUFFER, MooTextBuffer))
#define MOO_TEXT_BUFFER_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_TEXT_BUFFER, MooTextBufferClass))
#define MOO_IS_TEXT_BUFFER(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_TEXT_BUFFER))
#define MOO_IS_TEXT_BUFFER_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_TEXT_BUFFER))
#define MOO_TEXT_BUFFER_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_TEXT_BUFFER, MooTextBufferClass))


typedef struct _MooTextBuffer         MooTextBuffer;
typedef struct _MooTextBufferPrivate  MooTextBufferPrivate;
typedef struct _MooTextBufferClass    MooTextBufferClass;

struct _MooTextBuffer
{
    GtkSourceBuffer  parent;

    MooTextBufferPrivate *priv;
};

struct _MooTextBufferClass
{
    GtkSourceBufferClass parent_class;
    
    void (*cursor_moved) (MooTextBuffer      *buffer,
                          const GtkTextIter  *iter);
};


GType           moo_text_buffer_get_type        (void) G_GNUC_CONST;

MooTextBuffer  *moo_text_buffer_new             (void);

void            moo_text_buffer_set_lang        (MooTextBuffer  *buffer,
                                                 MooEditLang    *lang);

void            moo_text_buffer_set_bracket_match_style 
                                                (MooTextBuffer  *buffer,
                                                 const GtkSourceTagStyle *style);
void            moo_text_buffer_set_brackets    (MooTextBuffer  *buffer, 
                                                 const char     *brackets);
void            moo_text_buffer_set_check_brackets
                                                (MooTextBuffer  *buffer, 
                                                 gboolean        check);

gboolean        moo_text_buffer_has_text        (MooTextBuffer  *buffer);
gboolean        moo_text_buffer_has_selection   (MooTextBuffer  *buffer);

G_END_DECLS

#endif /* __MOO_TEXT_BUFFER_H__ */
/* kate: space-indent on; indent-width 4; replace-tabs on; */
