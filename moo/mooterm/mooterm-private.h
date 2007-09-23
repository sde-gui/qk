/*
 *   mooterm-private.h
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef MOOTERM_COMPILATION
#error "This file may not be included"
#endif

#ifndef MOO_TERM_PRIVATE_H
#define MOO_TERM_PRIVATE_H

#include "mooterm/mooterm.h"
#include "mooterm/mootermtag.h"
#include "mooterm/mootermbuffer.h"
#include "mooterm/mooterm-vt.h"
#include <gtk/gtk.h>

G_BEGIN_DECLS

#if 0
#define TERM_IMPLEMENT_ME_WARNING g_warning
#else
#define TERM_IMPLEMENT_ME_WARNING g_message
#endif

#define TERM_IMPLEMENT_ME TERM_IMPLEMENT_ME_WARNING ("%s: implement me", G_STRLOC)

#define PT_WRITER_PRIORITY          G_PRIORITY_DEFAULT
#define PT_READER_PRIORITY          G_PRIORITY_DEFAULT

#define ADJUSTMENT_PRIORITY         G_PRIORITY_DEFAULT_IDLE
#define ADJUSTMENT_DELTA            30.0

#define UPDATE_TIMEOUT              10
#define UPDATE_REPEAT_TIMEOUT       40
#define PROCESS_INCOMING_TIMEOUT    10
#define INPUT_CHUNK_SIZE            4096

#define MIN_TERMINAL_WIDTH          8
#define MIN_TERMINAL_HEIGHT         4
#define MAX_TERMINAL_WIDTH          4096

#define SCROLL_GRANULARITY          3

#ifdef MOO_DEBUG_ENABLED
#define DEFAULT_MONOSPACE_FONT "Courier New 11"
#else
#define DEFAULT_MONOSPACE_FONT "Monospace"
#endif


typedef enum {
    POINTER_NONE     = 0,
    POINTER_TEXT     = 1,
    POINTER_NORMAL   = 2,
    POINTERS_NUM     = 3
} TermPointerType;

enum {
    COLOR_NORMAL    = 0,
    COLOR_BOLD      = 1,
    COLOR_MAX       = 16
};

typedef enum {
    DRAG_NONE = 0,
    DRAG_SELECT,
    DRAG_DRAG
} DragType;

typedef struct _MooTermFont MooTermFont;

typedef enum {
    MOO_TERM_ERASE_AUTO,
    MOO_TERM_ERASE_ASCII_BACKSPACE,
    MOO_TERM_ERASE_ASCII_DELETE,
    MOO_TERM_ERASE_DELETE_SEQUENCE
} MooTermEraseBinding;

struct _MooTermCommand {
    char    *cmd_line;
    char   **argv;
    char    *working_dir;
    char   **envp;
};

struct _MooTermPrivate {
    struct _MooTermPt       *pt;
    struct _MooTermParser   *parser;
    GString                 *incoming;
    guint                    process_timeout;

    struct _MooTermBuffer   *buffer;
    struct _MooTermBuffer   *primary_buffer;
    struct _MooTermBuffer   *alternate_buffer;

    guint8          modes[MODE_MAX];
    guint8          saved_modes[MODE_MAX];

    gboolean        scrolled;
    guint           top_line;
    guint           width;
    guint           height;

    guint           cursor_row;
    guint           cursor_col;

    struct {
        guint           cursor_row, cursor_col; /* these are real cursor coordinates in buffer
                                                   (it may be different from what's displayed in AWM mode) */
        MooTermTextAttr attr;
        int             GL, GR;
        gboolean        autowrap;
        gboolean        decom;
        guint           top_margin, bottom_margin;
        /* TODO: Selective erase attribute ??? */
        int             single_shift;
    } saved_cursor;

    gpointer        selection;
    gboolean        owns_selection;

    MooTermFont    *font;

    GdkRegion      *changed; /* screen coordinates */
    guint           cursor_row_old; /* cursor has been here, and it's been invalidated */
    guint           cursor_col_old;
    guint           update_timer;

    GdkGC          *clip;
    gboolean        font_changed;
    PangoLayout    *layout;
    gboolean        colors_inverted;
    gboolean        cursor_visible;

    gboolean        blink_cursor_visible;
    gboolean        cursor_blinks;
    guint           cursor_blink_time;
    guint           cursor_blink_timeout_id;

    GdkGC          *color[COLOR_MAX];
    GdkGC          *fg[2];
    GdkGC          *bg;
    GdkColor        palette[COLOR_MAX];
    GdkColor        fg_color[2];
    GdkColor        bg_color;

    GdkCursor      *pointer[POINTERS_NUM];
    gboolean        pointer_visible;
    int             tracking_mouse;
    gulong          track_press_id;
    gulong          track_release_id;
    gulong          track_scroll_id;

    GtkIMContext   *im;
    gboolean        im_preedit_active;
    GdkModifierType modifiers;

    GtkAdjustment  *adjustment;
    guint           pending_adjustment_changed;
    guint           pending_adjustment_value_changed;

    struct {
        MooTermEraseBinding backspace_binding;
        MooTermEraseBinding delete_binding;
        guint               hide_pointer_on_keypress : 1;   /* = TRUE */
        guint               meta_sends_escape : 1;          /* = TRUE */
        guint               scroll_on_input : 1;            /* = TRUE */
        guint               bold_pango : 1;
        guint               bold_offset : 1;
    } settings;

    struct {
        guint           drag_scroll_timeout;
        GdkEventType    drag_button;
        DragType        drag_type;
        int             drag_start_x;
        int             drag_start_y;
        guint           drag_moved : 1;
    } mouse_stuff;
};

#define MOO_TYPE_TERM_ERASE_BINDING (_moo_term_erase_binding_get_type ())
GType       _moo_term_erase_binding_get_type (void) G_GNUC_CONST;

#define term_top_line(term)                     \
    ((term)->priv->scrolled ?                   \
        (term)->priv->top_line :                \
        buf_scrollback ((term)->priv->buffer))

void        _moo_term_init_settings         (void);
void        _moo_term_set_window_title      (MooTerm        *term,
                                             const char     *title);
void        _moo_term_set_icon_name         (MooTerm        *term,
                                             const char     *title);

void        _moo_term_buf_content_changed   (MooTerm        *term,
                                             MooTermBuffer  *buf);
void        _moo_term_cursor_moved          (MooTerm        *term,
                                             MooTermBuffer  *buf);
void        _moo_term_buffer_scrolled       (MooTermBuffer  *buf,
                                             guint           lines,
                                             MooTerm        *term);

void        _moo_term_update_size           (MooTerm        *term,
                                             gboolean        force);

void        _moo_term_init_font_stuff       (MooTerm        *term);
void        _moo_term_init_palette          (MooTerm        *term);
void        _moo_term_update_palette        (MooTerm        *term);
void        _moo_term_style_set             (GtkWidget      *widget,
                                             GtkStyle       *previous_style);

void        _moo_term_update_pointer        (MooTerm        *term);
void        _moo_term_set_pointer_visible   (MooTerm        *term,
                                             gboolean        visible);
gboolean    _moo_term_button_press          (GtkWidget      *widget,
                                             GdkEventButton *event);
gboolean    _moo_term_button_release        (GtkWidget      *widget,
                                             GdkEventButton *event);
gboolean    _moo_term_motion_notify         (GtkWidget      *widget,
                                             GdkEventMotion *event);
void        _moo_term_do_popup_menu         (MooTerm        *term,
                                             GdkEventButton *event);

gboolean    _moo_term_key_press             (GtkWidget      *widget,
                                             GdkEventKey    *event);
gboolean    _moo_term_key_release           (GtkWidget      *widget,
                                             GdkEventKey    *event);
void        _moo_term_im_commit             (GtkIMContext   *imcontext,
                                             gchar          *arg,
                                             MooTerm        *term);

gboolean    _moo_term_expose_event          (GtkWidget      *widget,
                                             GdkEventExpose *event);

/* (row, col) coordinates relative to visible part of buffer,
   i.e. _moo_term_invalidate is equivalent to
   _moo_term_invalidate_screen_rect ({0, 0, width, height}) */
void        _moo_term_invalidate_screen_rect(MooTerm        *term,
                                             GdkRectangle   *rect);
/* invalidate whole window */
void        _moo_term_invalidate            (MooTerm        *term);

void        _moo_term_release_selection     (MooTerm        *term);
void        _moo_term_grab_selection        (MooTerm        *term);
void        _moo_term_clear_selection       (MooTerm        *term);

/* in mooterm-draw.c */
void        _moo_term_pause_cursor_blinking (MooTerm        *term);
void        _moo_term_set_cursor_blinks     (MooTerm        *term,
                                             gboolean        blinks);


/*************************************************************************/
/* vt commands
 */

void        _moo_term_bell                  (MooTerm    *term);
void        _moo_term_decid                 (MooTerm    *term);
void        _moo_term_set_dec_modes         (MooTerm    *term,
                                             int        *modes,
                                             guint       n_modes,
                                             gboolean    set);
void        _moo_term_save_dec_modes        (MooTerm    *term,
                                             int        *modes,
                                             guint       n_modes);
void        _moo_term_restore_dec_modes     (MooTerm    *term,
                                             int        *modes,
                                             guint       n_modes);
void        _moo_term_set_ansi_modes        (MooTerm    *term,
                                             int        *modes,
                                             guint       n_modes,
                                             gboolean    set);
void        _moo_term_set_mode              (MooTerm    *term,
                                             int         mode,
                                             gboolean    set);
void        _moo_term_decsc                 (MooTerm    *term);
void        _moo_term_decrc                 (MooTerm    *term);

/* these two are in mootermdraw.c */
void        _moo_term_invert_colors         (MooTerm    *term,
                                             gboolean    invert);
void        _moo_term_set_cursor_visible    (MooTerm    *term,
                                             gboolean    visible);
/* this one is in mooterminput.c, tracking_type == -1 means turn it off */
void        _moo_term_set_mouse_tracking    (MooTerm    *term,
                                             int         tracking_type);

void        _moo_term_da1                   (MooTerm    *term);
void        _moo_term_da2                   (MooTerm    *term);
void        _moo_term_da3                   (MooTerm    *term);

void        _moo_term_setting_request       (MooTerm    *term,
                                             int         setting);
void        _moo_term_dsr                   (MooTerm    *term,
                                             int         type,
                                             int         arg,
                                             gboolean    extended);


/*************************************************************************/
/* font info
 */

struct _MooTermFont {
    PangoContext   *ctx;
    guint           width;
    guint           height;
    guint           ascent;
};

void            _moo_term_font_free         (MooTermFont            *info);

#define term_char_width(term)   ((term)->priv->font->width)
#define term_char_height(term)  ((term)->priv->font->height)


G_END_DECLS

#endif /* MOO_TERM_PRIVATE_H */
