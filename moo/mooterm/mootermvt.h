/*
 *   mooterm/mootermvt.h
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

#ifndef MOOTERM_MOOTERMVT_H
#define MOOTERM_MOOTERMVT_H

#include "mooterm/mootermbuffer.h"

G_BEGIN_DECLS


#define MOO_TYPE_TERM_VT            (moo_term_vt_get_type ())
#define MOO_TYPE_TERM_VT_WIN        (moo_term_vt_win_get_type ())
#define MOO_TYPE_TERM_VT_UNIX       (moo_term_vt_unix_get_type ())

#define MOO_TERM_VT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), MOO_TYPE_TERM_VT, MooTermVt))
#define MOO_TERM_VT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  MOO_TYPE_TERM_VT, MooTermVtClass))
#define MOO_IS_TERM_VT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MOO_TYPE_TERM_VT))
#define MOO_IS_TERM_VT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  MOO_TYPE_TERM_VT))
#define MOO_TERM_VT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  MOO_TYPE_TERM_VT, MooTermVtClass))

typedef struct _MooTermVt           MooTermVt;
typedef struct _MooTermVtPrivate    MooTermVtPrivate;
typedef struct _MooTermVtClass      MooTermVtClass;


struct _MooTermVt {
    GObject           parent;
    MooTermVtPrivate *priv;
};

struct _MooTermVtClass {
    GObjectClass  parent_class;

    /* virtual methods */
    void        (*set_size)     (MooTermVt  *vt,
                                 gulong      width,
                                 gulong      height);
    gboolean    (*fork_command) (MooTermVt  *vt,
                                 const char *cmd,
                                 const char *working_dir,
                                 char      **envp);
    void        (*write)        (MooTermVt  *vt,
                                 const char *data,
                                 gssize      len);
    void        (*kill_child)   (MooTermVt  *vt);

    /* signals */
    void        (*child_died)   (MooTermVt  *vt);
};


GType           moo_term_vt_get_type        (void) G_GNUC_CONST;
GType           moo_term_vt_unix_get_type   (void) G_GNUC_CONST;
GType           moo_term_vt_win_get_type    (void) G_GNUC_CONST;

/* creates MooTermVtWin or MooTermVtUnix instance, depending on platform */
MooTermVt      *moo_term_vt_new             (void);

void            moo_term_vt_set_buffer      (MooTermVt      *vt,
                                             MooTermBuffer  *buffer);
MooTermBuffer  *moo_term_vt_get_buffer      (MooTermVt      *vt);

void            moo_term_vt_set_size        (MooTermVt      *vt,
                                             gulong          width,
                                             gulong          height);

gboolean        moo_term_vt_fork_command    (MooTermVt      *vt,
                                             const char     *cmd,
                                             const char     *working_dir,
                                             char          **envp);
void            moo_term_vt_kill_child      (MooTermVt      *vt);

void            moo_term_vt_write           (MooTermVt      *vt,
                                             const char     *data,
                                             gssize          len);


G_END_DECLS

#endif /* MOOTERM_MOOTERMVT_H */
