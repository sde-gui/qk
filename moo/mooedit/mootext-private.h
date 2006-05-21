/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   mootext-private.h
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
#error "This file may not be included"
#endif

#ifndef __MOO_TEXT_PRIVATE_H__
#define __MOO_TEXT_PRIVATE_H__

#include "mooedit/moofold.h"
#include "mooedit/moolinebuffer.h"

G_BEGIN_DECLS


#define MOO_PLACEHOLDER_TAG "moo-placeholder-tag"


Line       *_moo_line_mark_get_line                 (MooLineMark        *mark);
void        _moo_line_mark_set_line                 (MooLineMark        *mark,
                                                     Line               *line,
                                                     int                 line_no,
                                                     guint               stamp);
void        _moo_line_mark_set_buffer               (MooLineMark        *mark,
                                                     MooTextBuffer      *buffer,
                                                     LineBuffer         *line_buf);
void        _moo_line_mark_deleted                  (MooLineMark        *mark);
void        _moo_line_mark_set_pretty               (MooLineMark        *mark,
                                                     gboolean            pretty);
gboolean    _moo_line_mark_get_pretty               (MooLineMark        *mark);

void        _moo_line_mark_realize                  (MooLineMark        *mark,
                                                     GtkWidget          *widget);
void        _moo_line_mark_unrealize                (MooLineMark        *mark);

void        _moo_line_mark_set_fold                 (MooLineMark        *mark,
                                                     MooFold            *fold);
MooFold    *_moo_line_mark_get_fold                 (MooLineMark        *mark);

void        _moo_text_buffer_ensure_highlight       (MooTextBuffer      *buffer,
                                                     int                 first_line,
                                                     int                 last_line);
void        _moo_text_buffer_apply_syntax_tag       (MooTextBuffer      *buffer,
                                                     GtkTextTag         *tag,
                                                     const GtkTextIter  *start,
                                                     const GtkTextIter  *end);
void        _moo_text_buffer_highlighting_changed   (MooTextBuffer      *buffer,
                                                     int                 first,
                                                     int                 last);
void        _moo_text_buffer_tags_changed           (MooTextBuffer      *buffer,
                                                     int                 first,
                                                     int                 last);
gpointer    _moo_text_buffer_get_undo_stack         (MooTextBuffer      *buffer);


G_END_DECLS

#endif /* __MOO_TEXT_PRIVATE_H__ */
