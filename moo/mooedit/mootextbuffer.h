/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
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

#include <gtk/gtktextbuffer.h>
#include <mooedit/moolang.h>
#include <mooedit/moolinemark.h>

G_BEGIN_DECLS


#define MOO_TYPE_TEXT_BUFFER              (moo_text_buffer_get_type ())
#define MOO_TEXT_BUFFER(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_TEXT_BUFFER, MooTextBuffer))
#define MOO_TEXT_BUFFER_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_TEXT_BUFFER, MooTextBufferClass))
#define MOO_IS_TEXT_BUFFER(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_TEXT_BUFFER))
#define MOO_IS_TEXT_BUFFER_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_TEXT_BUFFER))
#define MOO_TEXT_BUFFER_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_TEXT_BUFFER, MooTextBufferClass))


typedef struct _MooTextBufferPrivate  MooTextBufferPrivate;
typedef struct _MooTextBufferClass    MooTextBufferClass;

struct _MooTextBuffer
{
    GtkTextBuffer  parent;

    MooTextBufferPrivate *priv;
};

struct _MooTextBufferClass
{
    GtkTextBufferClass parent_class;

    void (*cursor_moved)        (MooTextBuffer      *buffer,
                                 const GtkTextIter  *iter);
    void (*selection_changed)   (MooTextBuffer      *buffer);

    void (*line_mark_added)     (MooTextBuffer      *buffer,
                                 MooLineMark        *mark);
};


GType       moo_text_buffer_get_type                    (void) G_GNUC_CONST;

GtkTextBuffer *moo_text_buffer_new                      (GtkTextTagTable    *table);

void        moo_text_buffer_set_lang                    (MooTextBuffer      *buffer,
                                                         MooLang            *lang);
MooLang    *moo_text_buffer_get_lang                    (MooTextBuffer      *buffer);

void        moo_text_buffer_set_highlight               (MooTextBuffer      *buffer,
                                                         gboolean            highlight);
gboolean    moo_text_buffer_get_highlight               (MooTextBuffer      *buffer);

void        moo_text_buffer_set_bracket_match_style     (MooTextBuffer      *buffer,
                                                         const MooTextStyle *style);
void        moo_text_buffer_set_bracket_mismatch_style  (MooTextBuffer      *buffer,
                                                         const MooTextStyle *style);
void        moo_text_buffer_set_brackets                (MooTextBuffer      *buffer,
                                                         const char         *brackets);

void        moo_text_buffer_freeze                      (MooTextBuffer      *buffer);
void        moo_text_buffer_thaw                        (MooTextBuffer      *buffer);
void        moo_text_buffer_begin_interactive_action    (MooTextBuffer      *buffer);
void        moo_text_buffer_end_interactive_action      (MooTextBuffer      *buffer);
void        moo_text_buffer_begin_non_interactive_action(MooTextBuffer      *buffer);
void        moo_text_buffer_end_non_interactive_action  (MooTextBuffer      *buffer);

gboolean    moo_text_buffer_has_text                    (MooTextBuffer      *buffer);
gboolean    moo_text_buffer_has_selection               (MooTextBuffer      *buffer);

void        moo_text_buffer_apply_scheme                (MooTextBuffer      *buffer,
                                                         MooTextStyleScheme *scheme);

gpointer    moo_text_buffer_get_undo_mgr                (MooTextBuffer      *buffer);

void        moo_text_buffer_add_line_mark               (MooTextBuffer      *buffer,
                                                         MooLineMark        *mark);
void        moo_text_buffer_remove_line_mark            (MooTextBuffer      *buffer,
                                                         MooLineMark        *mark);
void        moo_text_buffer_move_line_mark              (MooTextBuffer      *buffer,
                                                         MooLineMark        *mark,
                                                         int                 line);
GSList     *moo_text_buffer_get_line_marks_in_range     (MooTextBuffer      *buffer,
                                                         int                 first_line,
                                                         int                 last_line);
GSList     *moo_text_buffer_get_line_marks_at_line      (MooTextBuffer      *buffer,
                                                         int                 line);


G_END_DECLS

#endif /* __MOO_TEXT_BUFFER_H__ */
