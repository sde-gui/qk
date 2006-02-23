/*
 *   mootermpt.h
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

#ifndef MOOTERM_COMPILATION
#error "This file may not be included"
#endif

#ifndef __MOO_TERM_PT_H__
#define __MOO_TERM_PT_H__

#include <glib-object.h>

G_BEGIN_DECLS


#define MOO_TYPE_TERM_PT            (moo_term_pt_get_type ())
#define MOO_TYPE_TERM_PT_CYGWIN     (moo_term_pt_cyg_get_type ())
#define MOO_TYPE_TERM_PT_UNIX       (moo_term_pt_unix_get_type ())

#define MOO_TERM_PT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), MOO_TYPE_TERM_PT, MooTermPt))
#define MOO_TERM_PT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  MOO_TYPE_TERM_PT, MooTermPtClass))
#define MOO_IS_TERM_PT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MOO_TYPE_TERM_PT))
#define MOO_IS_TERM_PT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  MOO_TYPE_TERM_PT))
#define MOO_TERM_PT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  MOO_TYPE_TERM_PT, MooTermPtClass))

typedef struct _MooTermPt           MooTermPt;
typedef struct _MooTermPtPrivate    MooTermPtPrivate;
typedef struct _MooTermPtClass      MooTermPtClass;
struct _MooTerm;
struct _MooTermCommand;

struct _MooTermPt {
    GObject           parent;
    MooTermPtPrivate *priv;
};

struct _MooTermPtClass {
    GObjectClass  parent_class;

    /* virtual methods */
    void        (*set_size)             (MooTermPt  *pt,
                                         guint       width,
                                         guint       height);
    gboolean    (*fork_command)         (MooTermPt  *pt,
                                         const struct _MooTermCommand *cmd,
                                         const char *working_dir,
                                         char      **envp,
                                         GError    **error);
    void        (*write)                (MooTermPt  *pt,
                                         const char *data,
                                         gssize      len);
    void        (*kill_child)           (MooTermPt  *pt);

    /* signals */
    void        (*child_died)   (MooTermPt  *pt);
};


GType           moo_term_pt_get_type        (void) G_GNUC_CONST;
GType           moo_term_pt_unix_get_type   (void) G_GNUC_CONST;
GType           moo_term_pt_win_get_type    (void) G_GNUC_CONST;

/* creates MooTermPtWin or MooTermPtUnix instance, depending on platform */
MooTermPt      *_moo_term_pt_new            (struct _MooTerm *term);

void            _moo_term_pt_set_size       (MooTermPt      *pt,
                                             guint           width,
                                             guint           height);
void            _moo_term_pt_set_helper_directory
                                            (MooTermPt      *pt,
                                             const char     *dir);
char            _moo_term_pt_get_erase_char (MooTermPt      *pt);
void            _moo_term_pt_send_intr      (MooTermPt      *pt);

gboolean        _moo_term_pt_fork_command   (MooTermPt      *pt,
                                             const struct _MooTermCommand *cmd,
                                             const char     *working_dir,
                                             char          **envp,
                                             GError        **error);
void            _moo_term_pt_kill_child     (MooTermPt      *pt);

gboolean        _moo_term_pt_child_alive    (MooTermPt      *pt);

void            _moo_term_pt_write          (MooTermPt      *pt,
                                             const char     *data,
                                             gssize          len);

struct _MooTermCommand *_moo_term_get_default_shell(void);
gboolean        _moo_term_check_cmd         (struct _MooTermCommand *cmd,
                                             GError     **error);


G_END_DECLS

#endif /* __MOO_TERM_PT_H__ */
