/*
 *   mooterm.h
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef __MOO_TERM_H__
#define __MOO_TERM_H__

#include <gtk/gtkwidget.h>
#include <mooterm/mootermpt.h>

G_BEGIN_DECLS


#define MOO_TYPE_TERM               (moo_term_get_type ())
#define MOO_TERM(object)            (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_TERM, MooTerm))
#define MOO_TERM_CLASS(klass)       (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_TERM, MooTermClass))
#define MOO_IS_TERM(object)         (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_TERM))
#define MOO_IS_TERM_CLASS(klass)    (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_TERM))
#define MOO_TERM_GET_CLASS(obj)     (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_TERM, MooTermClass))

typedef struct _MooTerm             MooTerm;
typedef struct _MooTermPrivate      MooTermPrivate;
typedef struct _MooTermClass        MooTermClass;


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

    void (*bell)                    (MooTerm        *term);
    void (*set_window_title)        (MooTerm        *term,
                                     const char     *title);
    void (*set_icon_name)           (MooTerm        *term,
                                     const char     *icon);
    void (*child_died)              (MooTerm        *term);

    void (*populate_popup)          (MooTerm        *term,
                                     GtkWidget      *menu);

    void (*apply_settings)          (MooTerm        *term);

    void (*reset)                   (MooTerm        *term);
    void (*new_line)                (MooTerm        *term);
};

typedef enum {
    MOO_TERM_ERROR_FAILED,
    MOO_TERM_ERROR_INVAL
} MooTermError;

#define     MOO_TERM_ERROR                  (moo_term_error_quark())
GQuark      moo_term_error_quark            (void) G_GNUC_CONST;

GType       moo_term_get_type               (void) G_GNUC_CONST;

void        moo_term_set_adjustment         (MooTerm        *term,
                                             GtkAdjustment  *vadj);

gboolean    moo_term_fork_command           (MooTerm        *term,
                                             const MooTermCommand *cmd,
                                             GError        **error);
gboolean    moo_term_fork_command_line      (MooTerm        *term,
                                             const char     *cmd_line,
                                             const char     *working_dir,
                                             char          **envp,
                                             GError        **error);
gboolean    moo_term_fork_argv              (MooTerm        *term,
                                             char          **argv,
                                             const char     *working_dir,
                                             char          **envp,
                                             GError        **error);
gboolean    moo_term_child_alive            (MooTerm        *term);
void        moo_term_kill_child             (MooTerm        *term);

void        moo_term_feed                   (MooTerm        *term,
                                             const char     *data,
                                             int             len);
void        moo_term_feed_child             (MooTerm        *term,
                                             const char     *string,
                                             int             len);
void        moo_term_ctrl_c                 (MooTerm        *term);

void        moo_term_get_screen_size        (MooTerm        *term,
                                             guint          *columns,
                                             guint          *rows);

void        moo_term_scroll_to_top          (MooTerm        *term);
void        moo_term_scroll_to_bottom       (MooTerm        *term);
void        moo_term_scroll_lines           (MooTerm        *term,
                                             int             lines);
void        moo_term_scroll_pages           (MooTerm        *term,
                                             int             pages);

void        moo_term_copy_clipboard         (MooTerm        *term,
                                             GdkAtom         selection);
void        moo_term_paste_clipboard        (MooTerm        *term,
                                             GdkAtom         selection);
void        moo_term_select_all             (MooTerm        *term);
char       *moo_term_get_selection          (MooTerm        *term);
char       *moo_term_get_content            (MooTerm        *term);

void        moo_term_set_pointer_visible    (MooTerm        *term,
                                             gboolean        visible);
void        moo_term_set_font_from_string   (MooTerm        *term,
                                             const char     *font);
void        moo_term_set_cursor_blink_time  (MooTerm        *term,
                                             guint           ms);

void        moo_term_set_fd                 (MooTerm        *term,
                                             int             master);

void        moo_term_reset                  (MooTerm        *term);
void        moo_term_soft_reset             (MooTerm        *term);

guint       moo_term_char_height            (MooTerm        *term);
guint       moo_term_char_width             (MooTerm        *term);

gboolean    moo_term_start_default_shell    (MooTerm        *term,
                                             GError        **error);

void        moo_term_set_colors             (MooTerm        *term,
                                             GdkColor       *colors,
                                             guint           n_colors);

void        moo_term_apply_settings         (MooTerm        *term);


G_END_DECLS

#endif /* __MOO_TERM_H__ */
