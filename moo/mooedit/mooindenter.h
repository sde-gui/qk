/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   mooindenter.h
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

#ifndef __MOO_INDENTER_H__
#define __MOO_INDENTER_H__

#include <gtk/gtktextbuffer.h>

G_BEGIN_DECLS


#define MOO_TYPE_INDENTER            (moo_indenter_get_type ())
#define MOO_INDENTER(object)         (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_INDENTER, MooIndenter))
#define MOO_INDENTER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_INDENTER, MooIndenterClass))
#define MOO_IS_INDENTER(object)      (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_INDENTER))
#define MOO_IS_INDENTER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_INDENTER))
#define MOO_INDENTER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_INDENTER, MooIndenterClass))


typedef struct _MooIndenter         MooIndenter;
typedef struct _MooIndenterClass    MooIndenterClass;

struct _MooIndenter
{
    GObject parent;

    gpointer doc; /* MooEdit* */
    gboolean use_tabs;
    guint tab_width;
    guint indent;
};

struct _MooIndenterClass
{
    GObjectClass parent_class;

    void    (*config_changed)   (MooIndenter    *indenter,
                                 guint           setting_id);
    void    (*character)        (MooIndenter    *indenter,
                                 gunichar        inserted_char,
                                 GtkTextIter    *where);
};


GType        moo_indenter_get_type              (void) G_GNUC_CONST;

MooIndenter *moo_indenter_new                   (gpointer        doc,
                                                 const char     *name);

char        *moo_indenter_make_space            (MooIndenter    *indenter,
                                                 guint           len,
                                                 guint           start);

void         moo_indenter_character             (MooIndenter    *indenter,
                                                 gunichar        inserted_char,
                                                 GtkTextIter    *where);
void         moo_indenter_tab                   (MooIndenter    *indenter,
                                                 GtkTextBuffer  *buffer);
void         moo_indenter_shift_lines           (MooIndenter    *indenter,
                                                 GtkTextBuffer  *buffer,
                                                 guint           first_line,
                                                 guint           last_line,
                                                 int             direction);

/* computes offset of start and returns offset or -1 if there are
   non-whitespace characters before start */
int          moo_iter_get_blank_offset          (const GtkTextIter  *iter,
                                                 guint               tab_width);

/* computes where cursor should jump when backspace is pressed

<-- result -->
              blah blah blah
                      blah
                     | offset
*/
guint        moo_text_iter_get_prev_stop        (const GtkTextIter *start,
                                                 guint              tab_width,
                                                 guint              offset,
                                                 gboolean           same_line);


G_END_DECLS

#endif /* __MOO_INDENTER_H__ */
