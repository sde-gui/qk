/*
 *   mooterm-text.h
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

#ifndef MOO_TERM_TEXT_H
#define MOO_TERM_TEXT_H

#include <mooterm/mooterm.h>
#include <mooterm/mootermtag.h>
#include <mooterm/mootermline.h>


#define MOO_TYPE_TERM_ITER (moo_term_iter_get_type())

typedef struct _MooTermIter MooTermIter;

struct _MooTermIter {
    MooTerm *term;
    MooTermLine *line;
    gpointer buffer;
    int row;
    int col;
    int width;
    int stamp;
};


GType        moo_term_iter_get_type             (void) G_GNUC_CONST;

MooTermIter *moo_term_iter_copy                 (const MooTermIter  *iter);
void         moo_term_iter_free                 (MooTermIter        *iter);

guint        moo_term_get_line_count            (MooTerm            *term);
MooTermLine *moo_term_get_line                  (MooTerm            *term,
                                                 guint               line_no);

gboolean     moo_term_get_selection_bounds      (MooTerm            *term,
                                                 MooTermIter        *start,
                                                 MooTermIter        *end);

void         moo_term_set_line_data             (MooTerm            *term,
                                                 MooTermLine        *line,
                                                 const char         *key,
                                                 gpointer            data);
void         moo_term_set_line_data_full        (MooTerm            *term,
                                                 MooTermLine        *line,
                                                 const char         *key,
                                                 gpointer            data,
                                                 GDestroyNotify      destroy);
gpointer     moo_term_get_line_data             (MooTerm            *term,
                                                 MooTermLine        *line,
                                                 const char         *key);

MooTermTag  *moo_term_create_tag                (MooTerm            *term,
                                                 const char         *name);
MooTermTag  *moo_term_get_tag                   (MooTerm            *term,
                                                 const char         *name);
void         moo_term_delete_tag                (MooTerm            *term,
                                                 MooTermTag         *tag);

MooTermTagTable *moo_term_get_tag_table         (MooTerm            *term);


gboolean     moo_term_get_iter_at_line          (MooTerm            *term,
                                                 MooTermIter        *iter,
                                                 guint               line);
gboolean     moo_term_get_iter_at_line_offset   (MooTerm            *term,
                                                 MooTermIter        *iter,
                                                 guint               line,
                                                 guint               offset);
void         moo_term_get_iter_at_cursor        (MooTerm            *term,
                                                 MooTermIter        *iter);

void         moo_term_iter_forward_to_line_end  (MooTermIter        *iter);

void         moo_term_apply_tag                 (MooTerm            *term,
                                                 MooTermTag         *tag,
                                                 MooTermIter        *start,
                                                 MooTermIter        *end);
void         moo_term_remove_tag                (MooTerm            *term,
                                                 MooTermTag         *tag,
                                                 MooTermIter        *start,
                                                 MooTermIter        *end);

gboolean     moo_term_iter_has_tag              (MooTermIter        *iter,
                                                 MooTermTag         *tag);
gboolean     moo_term_iter_get_tag_start        (MooTermIter        *iter,
                                                 MooTermTag         *tag);
gboolean     moo_term_iter_get_tag_end          (MooTermIter        *iter,
                                                 MooTermTag         *tag);

void         moo_term_window_to_buffer_coords   (MooTerm            *term,
                                                 int                 window_x,
                                                 int                 window_y,
                                                 int                *buffer_x,
                                                 int                *buffer_y);
void         moo_term_get_iter_at_location      (MooTerm            *term,
                                                 MooTermIter        *iter,
                                                 int                 x,
                                                 int                 y);
void         moo_term_get_iter_at_pos           (MooTerm            *term,
                                                 MooTermIter        *iter,
                                                 int                 x,
                                                 int                 y);

char        *moo_term_get_text                  (MooTermIter        *start,
                                                 MooTermIter        *end);


#endif /* MOO_TERM_TEXT_H */
