/*
 *   mootermpt.h
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef MOO_TERM_PT_H
#define MOO_TERM_PT_H

#include <glib-object.h>

G_BEGIN_DECLS


#define MOO_TYPE_TERM_COMMAND       (moo_term_command_get_type ())

#define MOO_TYPE_TERM_PT            (moo_term_pt_get_type ())
#define MOO_TERM_PT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), MOO_TYPE_TERM_PT, MooTermPt))
#define MOO_TERM_PT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  MOO_TYPE_TERM_PT, MooTermPtClass))
#define MOO_IS_TERM_PT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MOO_TYPE_TERM_PT))
#define MOO_IS_TERM_PT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  MOO_TYPE_TERM_PT))
#define MOO_TERM_PT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  MOO_TYPE_TERM_PT, MooTermPtClass))

typedef struct _MooTermCommand      MooTermCommand;

typedef struct _MooTermPt           MooTermPt;
typedef struct _MooTermPtClass      MooTermPtClass;

typedef void  (*MooTermIOFunc)     (const char *buf,
                                    gsize       len,
                                    gpointer    data);
typedef gsize (*MooTermIOSizeFunc) (gpointer    data);


GType            moo_term_pt_get_type       (void) G_GNUC_CONST;
GType            moo_term_command_get_type  (void) G_GNUC_CONST;

/* creates MooTermPtCyg or MooTermPtUnix instance, depending on platform */
MooTermPt       *moo_term_pt_new                (MooTermIOFunc   func,
                                                 gpointer        data);

void            _moo_term_pt_set_io_size_func   (MooTermPt      *pt,
                                                 MooTermIOSizeFunc size_func,
                                                 gpointer        data);

void             moo_term_pt_set_priority       (MooTermPt      *pt,
                                                 int             priority);
void             moo_term_pt_set_size           (MooTermPt      *pt,
                                                 guint           width,
                                                 guint           height);
void             moo_term_pt_set_echo_input     (MooTermPt      *pt,
                                                 gboolean        echo);
void            _moo_term_pt_set_helper_directory
                                                (MooTermPt      *pt,
                                                 const char     *dir);
char            _moo_term_pt_get_erase_char     (MooTermPt      *pt);
void             moo_term_pt_send_intr          (MooTermPt      *pt);

gboolean         moo_term_pt_fork_command       (MooTermPt      *pt,
                                                 const MooTermCommand *cmd,
                                                 GError        **error);
void             moo_term_pt_kill_child         (MooTermPt      *pt);
gboolean         moo_term_pt_child_alive        (MooTermPt      *pt);
gboolean        _moo_term_pt_alive              (MooTermPt      *pt);

gboolean        _moo_term_pt_set_fd             (MooTermPt      *pt,
                                                 int             master);

void             moo_term_pt_write              (MooTermPt      *pt,
                                                 const char     *data,
                                                 gssize          len);

/* needed for cygwin */
void             moo_term_set_helper_directory  (const char     *dir);

MooTermCommand *_moo_term_get_default_shell     (void);
gboolean        _moo_term_check_cmd             (MooTermCommand *cmd,
                                                 GError        **error);
MooTermCommand  *moo_term_command_new_argv      (char          **argv,
                                                 const char     *working_dir,
                                                 char          **envp);
MooTermCommand  *moo_term_command_new_command_line (const char *cmd_line,
                                                 const char     *working_dir,
                                                 char          **envp);
MooTermCommand  *moo_term_command_copy          (const MooTermCommand *cmd);
void             moo_term_command_free          (MooTermCommand *cmd);


G_END_DECLS

#endif /* MOO_TERM_PT_H */
