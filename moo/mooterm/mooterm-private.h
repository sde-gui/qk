/*
 *   mooterm/mooterm-private.h
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

#ifndef MOOTERM_MOOTERM_PRIVATE_H
#define MOOTERM_MOOTERM_PRIVATE_H

#include "mooterm/mooterm.h"
#include "mooterm/mootermbuffer.h"
#include "mooterm/mooterm-vt.h"

G_BEGIN_DECLS

#if 0
#define TERM_IMPLEMENT_ME_WARNING g_warning
#else
#define TERM_IMPLEMENT_ME_WARNING g_message
#endif

#define TERM_IMPLEMENT_ME TERM_IMPLEMENT_ME_WARNING ("%s: implement me", G_STRFUNC)


#define ADJUSTMENT_PRIORITY         G_PRIORITY_HIGH_IDLE
#define ADJUSTMENT_DELTA            30.0
#define EXPOSE_PRIORITY             G_PRIORITY_DEFAULT
#define EXPOSE_TIMEOUT              2

#define PT_WRITER_PRIORITY          G_PRIORITY_DEFAULT
#define PT_READER_PRIORITY          G_PRIORITY_DEFAULT

#define MIN_TERMINAL_WIDTH          8
#define MIN_TERMINAL_HEIGHT         4
#define MAX_TERMINAL_WIDTH          4096

#define DEFAULT_MONOSPACE_FONT      "Courier New 9"
#define DEFAULT_MONOSPACE_FONT2     "Monospace"

#define SCROLL_GRANULARITY          3


typedef enum {
    POINTER_NONE     = 0,
    POINTER_TEXT     = 1,
    POINTER_NORMAL   = 2,
    POINTERS_NUM     = 3
} TermPointerType;

enum {
    NORMAL  = 0,
    BOLD    = 1
};

typedef struct _TermFontInfo    TermFontInfo;

struct _MooTermPrivate {
    struct _MooTermPt       *pt;
    struct _MooTermParser   *parser;

    struct _MooTermBuffer   *buffer;
    struct _MooTermBuffer   *primary_buffer;
    struct _MooTermBuffer   *alternate_buffer;

    guint8          modes[MODE_MAX];
    guint8          saved_modes[MODE_MAX];

    gboolean        _scrolled;
    guint           _top_line;
    guint           width;
    guint           height;

    guint           cursor_row;
    guint           cursor_col;

    struct {
        guint           cursor_row, cursor_col;
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

    TermFontInfo   *font_info;

    GdkPixmap      *back_pixmap;
    GdkRegion      *changed_content; /* buffer coordinates, relative to top_line */
    GdkGC          *clip;
    gboolean        font_changed;
    PangoLayout    *layout;
    guint           pending_expose;
    gboolean        colors_inverted;
    gboolean        _cursor_visible;

    gboolean        _blink_cursor_visible;
    gboolean        _cursor_blinks;
    guint           _cursor_blink_time;
    guint           _cursor_blink_timeout_id;

    GdkGC          *color[2][MOO_TERM_COLOR_MAX];
    GdkGC          *fg[2];
    GdkGC          *bg;

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
        gboolean            hide_pointer_on_keypress;   /* = TRUE */
        gboolean            meta_sends_escape;          /* = TRUE */
        gboolean            scroll_on_keystroke;        /* = TRUE */
        MooTermEraseBinding backspace_binding;
        MooTermEraseBinding delete_binding;
        gboolean            allow_bold;
    } settings;
};

#define term_top_line(term)                     \
    ((term)->priv->_scrolled ?                  \
        (term)->priv->_top_line :               \
        buf_scrollback ((term)->priv->buffer))

void        moo_term_text_iface_init        (gpointer        iface);

void        moo_term_set_window_title       (MooTerm        *term,
                                             const char     *title);
void        moo_term_set_icon_name          (MooTerm        *term,
                                             const char     *title);

void        moo_term_set_alternate_buffer   (MooTerm        *term,
                                             gboolean        alternate);

void        moo_term_buf_content_changed    (MooTerm        *term,
                                             MooTermBuffer  *buf);
void        moo_term_cursor_moved           (MooTerm        *term,
                                             MooTermBuffer  *buf);

void        moo_term_size_changed           (MooTerm        *term);
void        moo_term_buf_size_changed       (MooTerm        *term);

void        moo_term_init_font_stuff        (MooTerm        *term);
void        moo_term_setup_palette          (MooTerm        *term);

void        moo_term_update_pointer         (MooTerm        *term);
void        moo_term_set_pointer_visible    (MooTerm        *term,
                                             gboolean        visible);
gboolean    moo_term_button_press           (GtkWidget      *widget,
                                             GdkEventButton *event);
gboolean    moo_term_button_release         (GtkWidget      *widget,
                                             GdkEventButton *event);
gboolean    moo_term_motion_notify          (GtkWidget      *widget,
                                             GdkEventMotion *event);
void        moo_term_do_popup_menu          (MooTerm        *term,
                                             GdkEventButton *event);

gboolean    moo_term_key_press              (GtkWidget      *widget,
                                             GdkEventKey    *event);
gboolean    moo_term_key_release            (GtkWidget      *widget,
                                             GdkEventKey    *event);
void        moo_term_im_commit              (GtkIMContext   *imcontext,
                                             gchar          *arg,
                                             MooTerm        *term);
void        moo_term_im_preedit_start       (MooTerm        *term);
void        moo_term_im_preedit_end         (MooTerm        *term);

void        moo_term_init_back_pixmap       (MooTerm        *term);
void        moo_term_resize_back_pixmap     (MooTerm        *term);
void        moo_term_update_back_pixmap     (MooTerm        *term);
void        moo_term_invalidate_content_all (MooTerm        *term);
void        moo_term_invalidate_content_rect(MooTerm        *term,
                                             GdkRectangle   *rect);

gboolean    moo_term_expose_event           (GtkWidget      *widget,
                                             GdkEventExpose *event);
void        moo_term_invalidate_rect        (MooTerm        *term,
                                             GdkRectangle   *rect);
void        moo_term_force_update           (MooTerm        *term);
void        moo_term_invalidate_all         (MooTerm        *term);

void        moo_term_release_selection      (MooTerm        *term);
void        moo_term_grab_selection         (MooTerm        *term);

/* in mooterm-draw.c */
void        moo_term_pause_cursor_blinking  (MooTerm        *term);
void        moo_term_set_cursor_blinks      (MooTerm        *term,
                                             gboolean        blinks);


/*************************************************************************/
/* vt commands
 */

void        moo_term_reset                  (MooTerm    *term);
void        moo_term_soft_reset             (MooTerm    *term);

void        moo_term_bell                   (MooTerm    *term);
void        moo_term_decid                  (MooTerm    *term);
void        moo_term_set_dec_modes          (MooTerm    *term,
                                             int        *modes,
                                             guint       n_modes,
                                             gboolean    set);
void        moo_term_save_dec_modes         (MooTerm    *term,
                                             int        *modes,
                                             guint       n_modes);
void        moo_term_restore_dec_modes      (MooTerm    *term,
                                             int        *modes,
                                             guint       n_modes);
void        moo_term_set_ansi_modes         (MooTerm    *term,
                                             int        *modes,
                                             guint       n_modes,
                                             gboolean    set);
void        moo_term_set_mode               (MooTerm    *term,
                                             int         mode,
                                             gboolean    set);
void        moo_term_set_ca_mode            (MooTerm    *term,
                                             gboolean    set);
void        moo_term_decsc                  (MooTerm    *term);
void        moo_term_decrc                  (MooTerm    *term);

/* these two are in mootermdraw.c */
void        moo_term_invert_colors          (MooTerm    *term,
                                             gboolean    invert);
void        moo_term_set_cursor_visible      (MooTerm    *term,
                                             gboolean    visible);
/* this one is in mooterminput.c, tracking_type == -1 means turn it off */
void        moo_term_set_mouse_tracking     (MooTerm    *term,
                                             int         tracking_type);

void        moo_term_da1                    (MooTerm    *term);
void        moo_term_da2                    (MooTerm    *term);
void        moo_term_da3                    (MooTerm    *term);

void        moo_term_setting_request        (MooTerm    *term,
                                             int         setting);
void        moo_term_dsr                    (MooTerm    *term,
                                             int         type,
                                             int         arg,
                                             gboolean    extended);


/*************************************************************************/
/* font info
 */

struct _TermFontInfo {
    PangoContext   *ctx;
    guint           width;
    guint           height;
    guint           ascent;
};

void            moo_term_font_info_calculate(TermFontInfo           *info);
void            moo_term_font_info_set_font (TermFontInfo           *info,
                                             PangoFontDescription   *font_desc);
TermFontInfo   *moo_term_font_info_new      (PangoContext           *ctx);
void            moo_term_font_info_free     (TermFontInfo           *info);

#define term_char_width(term)   ((term)->priv->font_info->width)
#define term_char_height(term)  ((term)->priv->font_info->height)


G_END_DECLS

#endif /* MOOTERM_MOOTERM_PRIVATE_H */
