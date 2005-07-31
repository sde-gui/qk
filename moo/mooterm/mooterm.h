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

#include <gtk/gtk.h>

G_BEGIN_DECLS


#define MOO_TYPE_TERM              (moo_term_get_type ())
#define MOO_TYPE_TERM_PROFILE      (moo_term_profile_get_type ())
#define MOO_TERM(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_TERM, MooTerm))
#define MOO_TERM_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_TERM, MooTermClass))
#define MOO_IS_TERM(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_TERM))
#define MOO_IS_TERM_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_TERM))
#define MOO_TERM_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_TERM, MooTermClass))

typedef struct _MooTerm             MooTerm;
typedef struct _MooTermPrivate      MooTermPrivate;
typedef struct _MooTermClass        MooTermClass;
typedef struct _MooTermProfile      MooTermProfile;
typedef struct _MooTermProfileArray MooTermProfileArray;


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
                                     GtkMenu        *menu);

    void (*apply_settings)          (MooTerm        *term);
};

typedef enum {
    MOO_TERM_ERASE_AUTO,
    MOO_TERM_ERASE_ASCII_BACKSPACE,
    MOO_TERM_ERASE_ASCII_DELETE,
    MOO_TERM_ERASE_DELETE_SEQUENCE
} MooTermEraseBinding;

struct _MooTermProfile {
    char    *name;
    char    *cmd_line;
    char   **argv;
    char   **envp;
    char    *working_dir;
};

struct _MooTermProfileArray {
    MooTermProfile **data;
    guint            len;
};


GType       moo_term_get_type               (void) G_GNUC_CONST;
GType       moo_term_profile_get_type       (void) G_GNUC_CONST;
GType       moo_term_profile_array_get_type (void) G_GNUC_CONST;
GType       moo_term_erase_binding_get_type (void) G_GNUC_CONST;

void        moo_term_set_adjustment         (MooTerm        *term,
                                             GtkAdjustment  *vadj);

gboolean    moo_term_fork_command           (MooTerm        *term,
                                             const char     *cmd,
                                             const char     *working_dir,
                                             char          **envp);
gboolean    moo_term_fork_argv              (MooTerm        *term,
                                             char          **argv,
                                             const char     *working_dir,
                                             char          **envp);
gboolean    moo_term_child_alive            (MooTerm        *term);
void        moo_term_kill_child             (MooTerm        *term);

/* makes sense only for win32 */
void        moo_term_set_helper_directory   (const char     *dir);

void        moo_term_feed                   (MooTerm        *term,
                                             const char     *data,
                                             int             len);
void        moo_term_feed_child             (MooTerm        *term,
                                             const char     *string,
                                             int             len);

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
void        moo_term_ctrl_c                 (MooTerm        *term);

void        moo_term_set_pointer_visible    (MooTerm        *term,
                                             gboolean        visible);
void        moo_term_set_font_from_string   (MooTerm        *term,
                                             const char     *font);
void        moo_term_set_cursor_blink_time  (MooTerm        *term,
                                             guint           ms);


MooTermProfile      *moo_term_profile_new       (const char     *name,
                                                 const char     *cmd_line,
                                                 char          **argv,
                                                 char          **envp,
                                                 const char     *working_dir);
MooTermProfile      *moo_term_profile_copy      (const MooTermProfile *profile);
void                 moo_term_profile_free      (MooTermProfile *profile);

MooTermProfileArray *moo_term_profile_array_new (void);
void                 moo_term_profile_array_add (MooTermProfileArray *array,
                                                 const MooTermProfile *profile);
MooTermProfileArray *moo_term_profile_array_copy(MooTermProfileArray *array);
void                 moo_term_profile_array_free(MooTermProfileArray *array);

MooTermProfileArray *moo_term_get_profiles      (MooTerm        *term);
MooTermProfile      *moo_term_get_profile       (MooTerm        *term,
                                                 guint           index);

void        moo_term_set_profile            (MooTerm        *term,
                                             guint           index,
                                             const MooTermProfile *profile);
void        moo_term_add_profile            (MooTerm        *term,
                                             const MooTermProfile *profile);
void        moo_term_remove_profile         (MooTerm        *term,
                                             guint           index);

void        moo_term_set_default_profile    (MooTerm        *term,
                                             int             profile);
int         moo_term_get_default_profile    (MooTerm        *term);
void        moo_term_start_default_profile  (MooTerm        *term);
void        moo_term_start_profile          (MooTerm        *term,
                                             int             profile);


G_END_DECLS

#endif /* MOOTERM_MOOTERM_H */
