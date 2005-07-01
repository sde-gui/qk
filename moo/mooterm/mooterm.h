/*
 *   mooterm/mooterm.h
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

#ifndef MOOTERM_MOOTERM_H
#define MOOTERM_MOOTERM_H

#include "mooterm/mootermbuffer.h"
#include <gtk/gtk.h>

G_BEGIN_DECLS


#define MOO_TYPE_TERM              (moo_term_get_type ())
#define MOO_TERM(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_TERM, MooTerm))
#define MOO_TERM_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_TERM, MooTermClass))
#define MOO_IS_TERM(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_TERM))
#define MOO_IS_TERM_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_TERM))
#define MOO_TERM_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_TERM, MooTermClass))

typedef struct _MooTerm         MooTerm;
typedef struct _MooTermPrivate  MooTermPrivate;
typedef struct _MooTermClass    MooTermClass;


struct _MooTerm
{
    GtkWidget       parent;
    MooTermPrivate *priv;
};

struct _MooTermClass
{
    GtkWidgetClass parent_class;

    void (* set_scroll_adjustments) (GtkWidget      *widget,
                                     GtkAdjustment  *hadjustment,
                                     GtkAdjustment  *vadjustment);
};


GType            moo_term_get_type          (void) G_GNUC_CONST;

void             moo_term_set_buffer        (MooTerm        *term,
                                             MooTermBuffer  *buffer);
MooTermBuffer   *moo_term_get_buffer        (MooTerm        *term);
void             moo_term_set_adjustment    (MooTerm        *term,
                                             GtkAdjustment  *vadj);

gboolean         moo_term_fork_command      (MooTerm        *term,
                                             const char     *cmd,
                                             const char     *working_dir,
                                             char          **envp);
void             moo_term_feed_child        (MooTerm        *term,
                                             const char     *string,
                                             gssize          len);

void             moo_term_scroll_to_top     (MooTerm        *term);
void             moo_term_scroll_to_bottom  (MooTerm        *term);
void             moo_term_scroll_lines      (MooTerm        *term,
                                             int             lines);
void             moo_term_scroll_pages      (MooTerm        *term,
                                             int             pages);

void             moo_term_copy_clipboard    (MooTerm        *term);
void             moo_term_paste_clipboard   (MooTerm        *term);

void             moo_term_ctrl_c            (MooTerm        *term);


G_END_DECLS

#endif /* MOOTERM_MOOTERM_H */
