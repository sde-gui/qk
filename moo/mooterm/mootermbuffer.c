/*
 *   mooterm/mootermbuffer.c
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

#define MOOTERM_COMPILATION
#include "mooterm/mootermbuffer-private.h"
#include "mooutils/moocompat.h"
#include "mooutils/moomarshals.h"


#define ACS_STERLING    "\302\243"      /* U+00A3 POUND SIGN */
#define ACS_DARROW      "\342\206\223"  /* U+2193 DOWNWARDS ARROW */
#define ACS_LARROW      "\342\206\220"  /* U+2190 LEFTWARDS ARROW */
#define ACS_RARROW      "\342\206\222"  /* U+2192 RIGHTWARDS ARROW */
#define ACS_UARROW      "\342\206\221"  /* U+2191 UPWARDS ARROW */
#define ACS_BOARD       "#"             /* ??? */
#define ACS_BULLET      "\342\200\242"  /* U+2022 BULLET */
#define ACS_CKBOARD     "\342\226\223"  /* U+2593 DARK SHADE */
#define ACS_DEGREE      "\302\260"      /* U+00B0 DEGREE SIGN */
#define ACS_DIAMOND     "\342\227\206"  /* U+25C6 BLACK DIAMOND */
#define ACS_GEQUAL      "\342\211\245"  /* U+2265 GREATER-THAN OR EQUAL TO */
#define ACS_PI          "\317\200"      /* U+03C0 GREEK SMALL LETTER PI */
#define ACS_HLINE       "\342\224\200"  /* U+2500 BOX DRAWINGS LIGHT HORIZONTAL */
#define ACS_LANTERN     "\347\201\257"  /* U+706F CJK UNIFIED IDEOGRAPH-706F ??? */
#define ACS_PLUS        "\342\224\274"  /* U+253C BOX DRAWINGS LIGHT VERTICAL AND HORIZONTAL */
#define ACS_LEQUAL      "\342\211\244"  /* U+2264 LESS-THAN OR EQUAL TO */
#define ACS_LLCORNER    "\342\224\224"  /* U+2514 BOX DRAWINGS LIGHT UP AND RIGHT */
#define ACS_LRCORNER    "\342\224\230"  /* U+2518 BOX DRAWINGS LIGHT UP AND LEFT */
#define ACS_NEQUAL      "\342\211\240"  /* U+2260 NOT EQUAL TO */
#define ACS_PLMINUS     "\302\261"      /* U+00B1 PLUS-MINUS SIGN */
#define ACS_S1          "\342\216\272"  /* U+23BA HORIZONTAL SCAN LINE-1 */
#define ACS_S3          "\342\216\273"  /* U+23BB HORIZONTAL SCAN LINE-3 */
#define ACS_S7          "\342\216\274"  /* U+23BC HORIZONTAL SCAN LINE-7 */
#define ACS_S9          "\342\216\275"  /* U+23BD HORIZONTAL SCAN LINE-9 */
#define ACS_BLOCK       "\342\226\210"  /* U+2588 FULL BLOCK */
#define ACS_TTEE        "\342\224\254"  /* U+252C BOX DRAWINGS LIGHT DOWN AND HORIZONTAL */
#define ACS_RTEE        "\342\224\244"  /* U+2524 BOX DRAWINGS LIGHT VERTICAL AND LEFT */
#define ACS_LTEE        "\342\224\234"  /* U+251C BOX DRAWINGS LIGHT VERTICAL AND RIGHT */
#define ACS_BTEE        "\342\224\264"  /* U+2534 BOX DRAWINGS LIGHT UP AND HORIZONTAL */
#define ACS_ULCORNER    "\342\224\214"  /* U+250C BOX DRAWINGS LIGHT DOWN AND RIGHT */
#define ACS_URCORNER    "\342\224\220"  /* U+2510 BOX DRAWINGS LIGHT DOWN AND LEFT */
#define ACS_VLINE       "\342\224\202"  /* U+2502 BOX DRAWINGS LIGHT VERTICAL */


/*
blank                                        " "                              " "                             _
diamond                                      ACS_DIAMOND                      "+"                            `
checker board (stipple)                      ACS_CKBOARD                      ":"                            a
horizontal tab                               "\t"                             "\t"                           b
form feed                                    "\014"                           "\014"                         c
carriage return                              "\r"                             "\r"                           d
line feed                                    "\n"                             "\n"                           e
degree symbol                                ACS_DEGREE                       "\"                            f
plus/minus                                   ACS_PLMINUS                      "#"                            g
board of squares                             ACS_BOARD                        "#"                            h
lantern symbol                               ACS_LANTERN                      "#"                            i
lower right corner                           ACS_LRCORNER                     "+"                            j
upper right corner                           ACS_URCORNER                     "+"                            k
upper left corner                            ACS_ULCORNER                     "+"                            l
lower left corner                            ACS_LLCORNER                     "+"                            m
large plus or crossover                      ACS_PLUS                         "+"                            n
scan line 1                                  ACS_S1                           "~"                            o
scan line 3                                  ACS_S3                           "-"                            p
horizontal line                              ACS_HLINE                        "-"                            q
scan line 7                                  ACS_S7                           "-"                            r
scan line 9                                  ACS_S9                           "_"                            s
tee pointing right                           ACS_LTEE                         "+"                            t
tee pointing left                            ACS_RTEE                         "+"                            u
tee pointing up                              ACS_BTEE                         "+"                            v
tee pointing down                            ACS_TTEE                         "+"                            w
vertical line                                ACS_VLINE                        "|"                            x
less-than-or-equal-to                        ACS_LEQUAL                       "<"                            y
greater-than-or-equal-to                     ACS_GEQUAL                       ">"                            z
greek pi                                     ACS_PI                           "*"                            {
not-equal                                    ACS_NEQUAL                       "!"                            |
UK pound sign                                ACS_STERLING                     "f"                            }
bullet                                       ACS_BULLET                       "o"                            ~
*/


/* drawing chars are _`a-z{|}~ -- 0x5F - 0x7E */
#define MAX_GRAPH 126

static const char *DRAWING_SET_STRINGS[MAX_GRAPH + 1] = {
    /* 95 nulls */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,

    /* do we need control characters here? */
    " ", ACS_DIAMOND, ACS_CKBOARD, "?", "?", "?", "?", ACS_DEGREE, ACS_PLMINUS, ACS_BOARD,
    ACS_LANTERN, ACS_LRCORNER, ACS_URCORNER, ACS_ULCORNER, ACS_LLCORNER, ACS_PLUS, ACS_S1, ACS_S3,
    ACS_HLINE, ACS_S7, ACS_S9, ACS_LTEE, ACS_RTEE, ACS_BTEE, ACS_TTEE, ACS_VLINE, ACS_LEQUAL,
    ACS_GEQUAL, ACS_PI, ACS_NEQUAL, ACS_STERLING, ACS_BULLET
};

static gunichar ASCII_DRAWING_SET[MAX_GRAPH + 1] = {
    /* 95 zeros */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

    ' ', '+', ':', '?', '?', '?', '?', '\\', '#', '#', '#', '+', '+', '+', '+', '+', '~', '-',
    '-', '-', '_', '+', '+', '+', '+', '|', '<', '>', '*', '!', 'f', 'o'
};

static gunichar DRAWING_SET[MAX_GRAPH + 1];

static void init_drawing_sets (void);


#define MIN_WIDTH   (10)
#define MIN_HEIGHT  (10)

static void     moo_term_buffer_set_property    (GObject        *object,
                                                 guint           prop_id,
                                                 const GValue   *value,
                                                 GParamSpec     *pspec);
static void     moo_term_buffer_get_property    (GObject        *object,
                                                 guint           prop_id,
                                                 GValue         *value,
                                                 GParamSpec     *pspec);
static GObject *moo_term_buffer_constructor     (GType                  type,
                                                 guint                  n_construct_properties,
                                                 GObjectConstructParam *construct_param);
static void     moo_term_buffer_finalize        (GObject        *object);


/* MOO_TYPE_TERM_BUFFER */
G_DEFINE_TYPE (MooTermBuffer, moo_term_buffer, G_TYPE_OBJECT)

enum {
    CHANGED,
    SCREEN_SIZE_CHANGED,
    CURSOR_MOVED,
    BELL,
    SET_WINDOW_TITLE,
    SET_ICON_NAME,
    FEED_CHILD,
    FULL_RESET,
    LAST_SIGNAL
};

enum {
    PROP_0,
    PROP_SCREEN_WIDTH,
    PROP_SCREEN_HEIGHT,
    PROP_SCROLLBACK,
    PROP_MAX_SCROLLBACK,
    PROP_CURSOR_VISIBLE,
    PROP_MODE_IRM,
    PROP_MODE_LNM,
    PROP_MODE_DECCKM,
    PROP_MODE_DECSCNM,
    PROP_MODE_DECOM,
    PROP_MODE_DECAWM,
    PROP_MODE_MOUSE_TRACKING,
    PROP_MODE_HILITE_MOUSE_TRACKING,
    PROP_MODE_KEYPAD_NUMERIC,
    PROP_SCROLLING_REGION_SET
};

static guint signals[LAST_SIGNAL];


static void moo_term_buffer_class_init (MooTermBufferClass *klass)
{
    GObjectClass   *gobject_class = G_OBJECT_CLASS (klass);

    init_drawing_sets ();

    gobject_class->set_property = moo_term_buffer_set_property;
    gobject_class->get_property = moo_term_buffer_get_property;
    gobject_class->constructor = moo_term_buffer_constructor;
    gobject_class->finalize = moo_term_buffer_finalize;

    klass->changed = buf_changed_clear;

    signals[CHANGED] =
            g_signal_new ("changed",
                          G_OBJECT_CLASS_TYPE (gobject_class),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooTermBufferClass, changed),
                          NULL, NULL,
                          _moo_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);

    signals[CURSOR_MOVED] =
            g_signal_new ("cursor-moved",
                          G_OBJECT_CLASS_TYPE (gobject_class),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooTermBufferClass, cursor_moved),
                          NULL, NULL,
                          _moo_marshal_VOID__UINT_UINT,
                          G_TYPE_NONE, 2,
                          G_TYPE_UINT, G_TYPE_UINT);

    signals[BELL] =
            g_signal_new ("bell",
                          G_OBJECT_CLASS_TYPE (gobject_class),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooTermBufferClass, bell),
                          NULL, NULL,
                          _moo_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);

    signals[SCREEN_SIZE_CHANGED] =
            g_signal_new ("screen-size-changed",
                          G_OBJECT_CLASS_TYPE (gobject_class),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooTermBufferClass, screen_size_changed),
                          NULL, NULL,
                          _moo_marshal_VOID__UINT_UINT,
                          G_TYPE_NONE, 2,
                          G_TYPE_UINT, G_TYPE_UINT);

    signals[SET_WINDOW_TITLE] =
            g_signal_new ("set-window-title",
                          G_OBJECT_CLASS_TYPE (gobject_class),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooTermBufferClass, set_window_title),
                          NULL, NULL,
                          _moo_marshal_VOID__STRING,
                          G_TYPE_NONE, 1,
                          G_TYPE_STRING | G_SIGNAL_TYPE_STATIC_SCOPE);

    signals[SET_ICON_NAME] =
            g_signal_new ("set-icon-name",
                          G_OBJECT_CLASS_TYPE (gobject_class),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooTermBufferClass, set_icon_name),
                          NULL, NULL,
                          _moo_marshal_VOID__STRING,
                          G_TYPE_NONE, 1,
                          G_TYPE_STRING | G_SIGNAL_TYPE_STATIC_SCOPE);

    signals[FEED_CHILD] =
            g_signal_new ("feed-child",
                          G_OBJECT_CLASS_TYPE (gobject_class),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooTermBufferClass, feed_child),
                          NULL, NULL,
                          _moo_marshal_VOID__STRING_INT,
                          G_TYPE_NONE, 2,
                          G_TYPE_STRING | G_SIGNAL_TYPE_STATIC_SCOPE,
                          G_TYPE_INT);

    signals[FULL_RESET] =
            g_signal_new ("full-reset",
                          G_OBJECT_CLASS_TYPE (gobject_class),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooTermBufferClass, full_reset),
                          NULL, NULL,
                          _moo_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);

    g_object_class_install_property (gobject_class,
                                     PROP_SCREEN_WIDTH,
                                     g_param_spec_uint ("screen-width",
                                             "screen-width",
                                             "screen-width",
                                             0, 1000000, 80,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_SCREEN_HEIGHT,
                                     g_param_spec_uint ("screen-height",
                                             "screen-height",
                                             "screen-height",
                                             0, 1000000, 24,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_MAX_SCROLLBACK,
                                     g_param_spec_int ("max-scrollback",
                                             "max-scrollback",
                                             "max-scrollback",
                                             -1, 1000000, -1,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_SCROLLBACK,
                                     g_param_spec_uint ("scrollback",
                                             "scrollback",
                                             "scrollback",
                                             0, 1000000, 0,
                                             G_PARAM_READABLE));

    g_object_class_install_property (gobject_class,
                                     PROP_CURSOR_VISIBLE,
                                     g_param_spec_boolean ("cursor-visible",
                                             "cursor-visible",
                                             "cursor-visible",
                                             TRUE,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_MODE_IRM,
                                     g_param_spec_boolean ("mode-IRM",
                                             "mode-IRM",
                                             "Insertion-Replacement Mode",
                                             FALSE,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_MODE_LNM,
                                     g_param_spec_boolean ("mode-LNM",
                                             "mode-LNM",
                                             "Linefeed/New Line Mode",
                                             FALSE,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_MODE_DECCKM,
                                     g_param_spec_boolean ("mode-DECCKM",
                                             "mode-DECCKM",
                                             "Cursor Key Mode",
                                             FALSE,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_MODE_DECSCNM,
                                     g_param_spec_boolean ("mode-DECSCNM",
                                             "mode-DECSCNM",
                                             "Screen Mode",
                                             FALSE,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_MODE_DECOM,
                                     g_param_spec_boolean ("mode-DECOM",
                                             "mode-DECOM",
                                             "Origin Mode",
                                             FALSE,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_MODE_DECAWM,
                                     g_param_spec_boolean ("mode-DECAWM",
                                             "mode-DECAWM",
                                             "Auto Wrap Mode",
                                             TRUE,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_MODE_MOUSE_TRACKING,
                                     g_param_spec_boolean ("mode-MOUSE-TRACKING",
                                             "mode-MOUSE_TRACKING",
                                             "Send mouse X & Y on button press and release",
                                             FALSE,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_MODE_HILITE_MOUSE_TRACKING,
                                     g_param_spec_boolean ("mode-HILITE-MOUSE-TRACKING",
                                             "mode-HILITE_MOUSE_TRACKING",
                                             "Use Hilite Mouse Tracking",
                                             FALSE,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_MODE_KEYPAD_NUMERIC,
                                     g_param_spec_boolean ("mode-KEYPAD-NUMERIC",
                                             "mode-KEYPAD-NUMERIC",
                                             "mode-KEYPAD-NUMERIC",
                                             TRUE,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_SCROLLING_REGION_SET,
                                     g_param_spec_boolean ("scrolling-region-set",
                                             "scrolling-region-set",
                                             "scrolling-region-set",
                                             FALSE,
                                             G_PARAM_READABLE));
}


static void     moo_term_buffer_init            (MooTermBuffer      *buf)
{
    buf->priv = g_new0 (MooTermBufferPrivate, 1);

    buf->priv->lines = g_ptr_array_new ();
    buf->priv->changed = NULL;
    buf->priv->changed_all = FALSE;
    buf->priv->parser = moo_term_parser_new (buf);

    buf->priv->single_shift = -1;
    buf->priv->graph_sets[0] = buf->priv->graph_sets[1] =
            buf->priv->graph_sets[2] = buf->priv->graph_sets[3] = NULL;
    buf->priv->current_graph_set = NULL;
}


static void     moo_term_buffer_finalize        (GObject            *object)
{
    guint i;
    MooTermBuffer *buf = MOO_TERM_BUFFER (object);

    for (i = 0; i < buf->priv->lines->len; ++i)
        term_line_free (buf_line (buf, i));
    g_ptr_array_free (buf->priv->lines, TRUE);

    g_list_free (buf->priv->tab_stops);

    if (buf->priv->changed)
        buf_region_destroy (buf->priv->changed);

    moo_term_parser_free (buf->priv->parser);

    g_free (buf->priv);

    G_OBJECT_CLASS (moo_term_buffer_parent_class)->finalize (object);
}


#define set_mode(modes, m, val) \
{                               \
    if (val)                    \
        modes |= m;             \
    else                        \
        modes &= ~m;            \
}


static void     moo_term_buffer_set_property    (GObject        *object,
                                                 guint           prop_id,
                                                 const GValue   *value,
                                                 GParamSpec     *pspec)
{
    MooTermBuffer *buf = MOO_TERM_BUFFER (object);

    switch (prop_id) {
        case PROP_SCREEN_WIDTH:
            if (buf->priv->constructed)
                moo_term_buffer_set_screen_size (buf, g_value_get_uint (value), 0);
            else
                buf->priv->screen_width = g_value_get_uint (value);
            break;

        case PROP_SCREEN_HEIGHT:
            if (buf->priv->constructed)
                moo_term_buffer_set_screen_size (buf, 0, g_value_get_uint (value));
            else
                buf->priv->screen_height = g_value_get_uint (value);
            break;

        case PROP_MAX_SCROLLBACK:
            if (buf->priv->constructed)
                moo_term_buffer_set_max_scrollback (buf, g_value_get_int (value));
            else
                buf->priv->max_height = g_value_get_int (value);
            break;

        case PROP_CURSOR_VISIBLE:
            moo_term_buffer_set_cursor_visible (buf, g_value_get_boolean (value));
            break;

        case PROP_MODE_IRM:
            set_mode (buf->priv->modes, IRM, g_value_get_boolean (value));
            g_object_notify (G_OBJECT (buf), "mode-IRM");
            break;

        case PROP_MODE_LNM:
            set_mode (buf->priv->modes, LNM, g_value_get_boolean (value));
            g_object_notify (G_OBJECT (buf), "mode-LNM");
            break;

        case PROP_MODE_DECCKM:
            set_mode (buf->priv->modes, DECCKM, g_value_get_boolean (value));
            g_object_notify (G_OBJECT (buf), "mode-DECCKM");
            break;

        case PROP_MODE_DECSCNM:
            set_mode (buf->priv->modes, DECSCNM, g_value_get_boolean (value));
            g_object_notify (G_OBJECT (buf), "mode-DECSCNM");
            break;

        case PROP_MODE_DECOM:
            set_mode (buf->priv->modes, DECOM, g_value_get_boolean (value));
            g_object_notify (G_OBJECT (buf), "mode-DECOM");
            break;

        case PROP_MODE_DECAWM:
            set_mode (buf->priv->modes, DECAWM, g_value_get_boolean (value));
            g_object_notify (G_OBJECT (buf), "mode-DECAWM");
            break;

        case PROP_MODE_MOUSE_TRACKING:
            set_mode (buf->priv->modes, MOUSE_TRACKING, g_value_get_boolean (value));
            g_object_notify (G_OBJECT (buf), "mode-MOUSE-TRACKING");
            break;

        case PROP_MODE_HILITE_MOUSE_TRACKING:
            set_mode (buf->priv->modes, HILITE_MOUSE_TRACKING, g_value_get_boolean (value));
            g_object_notify (G_OBJECT (buf), "mode-HILITE-MOUSE-TRACKING");
            break;

        case PROP_MODE_KEYPAD_NUMERIC:
            set_mode (buf->priv->modes, KEYPAD_NUMERIC, g_value_get_boolean (value));
            g_object_notify (G_OBJECT (buf), "mode-KEYPAD-NUMERIC");
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}


static void     moo_term_buffer_get_property    (GObject        *object,
                                                 guint           prop_id,
                                                 GValue         *value,
                                                 GParamSpec     *pspec)
{
    MooTermBuffer *buf = MOO_TERM_BUFFER (object);

    switch (prop_id) {
        case PROP_SCREEN_WIDTH:
            g_value_set_uint (value, buf->priv->screen_width);
            break;

        case PROP_SCREEN_HEIGHT:
            g_value_set_uint (value, buf->priv->screen_height);
            break;

        case PROP_SCROLLBACK:
            g_value_set_uint (value, buf_screen_offset (buf));
            break;

        case PROP_MAX_SCROLLBACK:
            g_value_set_int (value, buf->priv->max_height);
            break;

        case PROP_CURSOR_VISIBLE:
            g_value_set_boolean (value, buf->priv->cursor_visible);
            break;

        case PROP_MODE_IRM:
            g_value_set_boolean (value, buf->priv->modes & IRM);
            break;

        case PROP_MODE_LNM:
            g_value_set_boolean (value, buf->priv->modes & LNM);
            break;

        case PROP_MODE_DECCKM:
            g_value_set_boolean (value, buf->priv->modes & DECCKM);
            break;

        case PROP_MODE_DECSCNM:
            g_value_set_boolean (value, buf->priv->modes & DECSCNM);
            break;

        case PROP_MODE_DECOM:
            g_value_set_boolean (value, buf->priv->modes & DECOM);
            break;

        case PROP_MODE_DECAWM:
            g_value_set_boolean (value, buf->priv->modes & DECAWM);
            break;

        case PROP_MODE_MOUSE_TRACKING:
            g_value_set_boolean (value, buf->priv->modes & MOUSE_TRACKING);
            break;

        case PROP_MODE_HILITE_MOUSE_TRACKING:
            g_value_set_boolean (value, buf->priv->modes & HILITE_MOUSE_TRACKING);
            break;

        case PROP_MODE_KEYPAD_NUMERIC:
            g_value_set_boolean (value, buf->priv->modes & KEYPAD_NUMERIC);
            break;

        case PROP_SCROLLING_REGION_SET:
            g_value_set_boolean (value, buf->priv->scrolling_region_set);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}


static GObject *moo_term_buffer_constructor     (GType                  type,
                                                 guint                  n_props,
                                                 GObjectConstructParam *props)
{
    MooTermBuffer *buf;
    GObject *object;
    guint i;

    object = G_OBJECT_CLASS(moo_term_buffer_parent_class)->constructor (type, n_props, props);
    buf = MOO_TERM_BUFFER (object);

    buf->priv->constructed = TRUE;

    if (buf->priv->max_height <= MIN_HEIGHT)
        buf->priv->max_height = -1;

    buf->priv->current_attr.mask = 0;

    if (buf->priv->screen_width < MIN_WIDTH)
        buf->priv->screen_width = MIN_WIDTH;
    if (buf->priv->screen_height < MIN_HEIGHT)
        buf->priv->screen_height = MIN_HEIGHT;
    buf->priv->screen_offset = 0;

    buf->priv->top_margin = 0;
    buf->priv->bottom_margin = buf->priv->screen_height - 1;
    buf->priv->scrolling_region_set = FALSE;

    for (i = 0; i < buf->priv->screen_height; ++i)
    {
        g_ptr_array_add (buf->priv->lines,
                         term_line_new (buf->priv->screen_width));
    }

    return object;
}


void    moo_term_buffer_set_cursor_visible  (MooTermBuffer  *buf,
                                             gboolean        visible)
{
    buf->priv->cursor_visible = visible;
    g_object_notify (G_OBJECT (buf), "cursor-visible");
}


void    moo_term_buffer_bell            (MooTermBuffer  *buf)
{
    g_signal_emit (buf, signals[BELL], 0);
}


void    moo_term_buffer_changed         (MooTermBuffer  *buf)
{
    if (!buf->priv->freeze_changed_notify)
        g_signal_emit (buf, signals[CHANGED], 0);
}

void    moo_term_buffer_scrollback_changed  (MooTermBuffer  *buf)
{
    g_object_notify (G_OBJECT (buf), "scrollback");
}


static void buf_set_screen_width    (MooTermBuffer  *buf,
                                     guint           width)
{
    buf->priv->screen_width = width;

    if (buf->priv->cursor_col >= width)
        moo_term_buffer_cursor_move_to (buf, -1, width - 1);

    g_object_notify (G_OBJECT (buf), "screen-width");
    g_signal_emit (buf, signals[SCREEN_SIZE_CHANGED], 0,
                   width,
                   buf_screen_height (buf));

    moo_term_buffer_reset_tab_stops (buf);

    buf_changed_set_all (buf);
    moo_term_buffer_changed (buf);
}


static void buf_set_screen_height   (MooTermBuffer  *buf,
                                     guint           height)
{
    guint old_height = buf_screen_height (buf);
    guint width = buf_screen_width (buf);

    buf->priv->screen_height = height;

    if (height > old_height)
    {
        if (buf->priv->screen_offset < height - old_height)
        {
            guint add = height - old_height - buf->priv->screen_offset;
            guint i;

            for (i = 0; i < add; ++i)
                g_ptr_array_add (buf->priv->lines,
                                 term_line_new (width));

            if (buf->priv->screen_offset)
            {
                guint cursor_row =
                        buf->priv->screen_offset + buf->priv->cursor_row;

                buf->priv->screen_offset = 0;

                moo_term_buffer_cursor_move_to (buf, cursor_row, -1);
                moo_term_buffer_scrollback_changed (buf);
            }
        }
        else
        {
            buf->priv->screen_offset -= (height - old_height);
            moo_term_buffer_cursor_move_to (buf,
                                            buf->priv->cursor_row + height - old_height, -1);
            moo_term_buffer_scrollback_changed (buf);
        }
    }
    else
    {
        buf->priv->screen_offset += (old_height - height);

        if (buf->priv->cursor_row > old_height - height)
            moo_term_buffer_cursor_move_to (buf,
                                            buf->priv->cursor_row + height - old_height, -1);
        else
            moo_term_buffer_cursor_move_to (buf, 0, -1);

        moo_term_buffer_scrollback_changed (buf);
    }

    g_object_notify (G_OBJECT (buf), "screen-height");
    g_signal_emit (buf, signals[SCREEN_SIZE_CHANGED], 0,
                   buf_screen_width (buf),
                   height);

    if (buf->priv->scrolling_region_set)
    {
        if (buf->priv->top_margin >= height)
        {
            buf->priv->top_margin = 0;
            buf->priv->bottom_margin = height - 1;
            buf->priv->scrolling_region_set = FALSE;
            g_object_notify (G_OBJECT (buf), "scrolling-region-set");
        }
        else if (buf->priv->bottom_margin >= height)
        {
            buf->priv->bottom_margin = height - 1;
        }
    }

    buf_changed_set_all (buf);
    moo_term_buffer_changed (buf);
}


void    moo_term_buffer_set_screen_size (MooTermBuffer  *buf,
                                         guint           width,
                                         guint           height)
{
    if (height >= MIN_HEIGHT && height != buf_screen_height (buf))
        buf_set_screen_height (buf, height);

    if (width >= MIN_WIDTH && width != buf_screen_width (buf))
        buf_set_screen_width (buf, width);
}


void    moo_term_buffer_set_max_scrollback      (G_GNUC_UNUSED MooTermBuffer  *buf,
                                                 G_GNUC_UNUSED int             lines)
{
    g_warning ("%s: implement me", G_STRLOC);
}


void    moo_term_buffer_cursor_move     (MooTermBuffer  *buf,
                                         int             rows,
                                         int             cols)
{
    int width = buf_screen_width (buf);
    int height = buf_screen_height (buf);
    int cursor_row = buf_cursor_row (buf);
    int cursor_col = buf_cursor_col (buf);

    if (rows && cursor_row + rows >= 0 && cursor_row + rows < height)
        cursor_row += rows;
    if (cols && cursor_col + cols >= 0 && cursor_col + cols < width)
        cursor_col += cols;

    moo_term_buffer_cursor_move_to (buf, cursor_row, cursor_col);
}


static void moo_term_buffer_cursor_moved (MooTermBuffer  *buf,
                                          guint           old_row,
                                          guint           old_col)
{
    if (!buf->priv->freeze_cursor_notify)
        g_signal_emit (buf, signals[CURSOR_MOVED], 0,
                       old_row, old_col);
}


void    moo_term_buffer_cursor_move_to  (MooTermBuffer  *buf,
                                         int             row,
                                         int             col)
{
    guint old_row = buf_cursor_row (buf);
    guint old_col = buf_cursor_col (buf);

    g_return_if_fail (row < (int) buf_screen_height (buf));
    g_return_if_fail (col < (int) buf_screen_width (buf));

    if (row < 0)
        row = old_row;
    if (col < 0)
        col = old_col;

    buf->priv->cursor_row = row;
    buf->priv->cursor_col = col;

    moo_term_buffer_cursor_moved (buf, old_row, old_col);
}


void    moo_term_buffer_write           (MooTermBuffer  *buf,
                                         const char     *data,
                                         int             len)
{
    guint old_cursor_row = buf_cursor_row (buf);
    guint old_cursor_col = buf_cursor_col (buf);

    buf_freeze_changed_notify (buf);
    buf_freeze_cursor_notify (buf);

    moo_term_parser_parse (buf->priv->parser, data, len);

    buf_thaw_changed_notify (buf);
    buf_thaw_cursor_notify (buf);

    moo_term_buffer_changed (buf);
    moo_term_buffer_cursor_moved (buf, old_cursor_row, old_cursor_col);
}


/* chars must be valid unicode string */
static void buf_print_chars_real (MooTermBuffer *buf,
                                  const char    *chars,
                                  guint          len)
{
    guint width = buf_screen_width (buf);
    guint cursor_row = buf_cursor_row (buf);

    MooTermTextAttr *attr =
            buf->priv->current_attr.mask ? &buf->priv->current_attr : NULL;

    const char *p = chars;
    const char *end = chars + len;

    g_assert (p < end);

    for (p = chars; p < end; p = g_utf8_next_char (p))
    {
        gunichar c = g_utf8_get_char (p);

        if (c <= MAX_GRAPH)
        {
            if (buf->priv->single_shift >= 0)
            {
                if (buf->priv->graph_sets[buf->priv->single_shift])
                {
                    if (buf->priv->graph_sets[buf->priv->single_shift][c])
                        c = buf->priv->graph_sets[buf->priv->single_shift][c];
                    else
                        g_warning ("%s: using regular character while in "
                                   "graphics mode", G_STRLOC);
                }
                buf->priv->single_shift = -1;
            }
            else if (buf->priv->current_graph_set)
            {
                if (buf->priv->current_graph_set[c])
                    c = buf->priv->current_graph_set[c];
                else
                    g_warning ("%s: using regular character while in "
                               "graphics mode", G_STRLOC);
            }
        }

        if (buf->priv->modes & IRM)
        {
            term_line_insert_unichar (buf_screen_line (buf, cursor_row),
                                      buf->priv->cursor_col++,
                                      c, 1, attr, width);
            buf_changed_add_range (buf, cursor_row,
                                   buf->priv->cursor_col - 1,
                                   width - buf->priv->cursor_col + 1);
        }
        else
        {
            term_line_set_unichar (buf_screen_line (buf, cursor_row),
                                   buf->priv->cursor_col++,
                                   c, 1, attr, width);
            buf_changed_add_range (buf, cursor_row,
                                   buf->priv->cursor_col - 1, 1);
        }

        if (buf->priv->cursor_col == width)
        {
            buf->priv->cursor_col--;
            if (buf->priv->modes & DECAWM)
                moo_term_buffer_new_line (buf);
        }
    }
}


/* chars must be valid unicode string */
void    moo_term_buffer_print_chars     (MooTermBuffer  *buf,
                                         const char     *chars,
                                         int             len)
{
    const char *p = chars;
    const char *s;

    g_return_if_fail (len != 0 && chars != NULL);

    while ((len > 0 && p != chars + len) || (len < 0 && *p != 0))
    {
        for (s = p; ((len > 0 && s != chars + len) || (len < 0 && *s != 0))
             && *s && (*s & 0x80 || (' ' <= *s && *s <= '~')); ++s) ;

        if (s != p)
        {
            buf_print_chars_real (buf, p, s - p);
            p = s;
        }
        else if (!*s)
        {
            ++p;
        }
        else if (*s == 0x7F)
        {
            buf_print_chars_real (buf, "~", -1);
            ++p;
        }
        else
        {
            static char ar[2] = {'^'};
            ar[1] = *s + 0x40;
            buf_print_chars_real (buf, ar, 2);
        }
    }
}


MooTermBuffer  *moo_term_buffer_new         (guint width,
                                             guint height)
{
    return MOO_TERM_BUFFER (g_object_new (MOO_TYPE_TERM_BUFFER,
                                          "screen-width", width,
                                          "screen-height", height,
                                          NULL));
}


void moo_term_buffer_set_window_title   (MooTermBuffer  *buf,
                                         const char     *title)
{
    g_signal_emit (buf, signals[SET_WINDOW_TITLE], 0, title);
}

void moo_term_buffer_set_icon_name      (MooTermBuffer  *buf,
                                         const char     *icon)
{
    g_signal_emit (buf, signals[SET_ICON_NAME], 0, icon);
}


void    moo_term_buffer_index           (MooTermBuffer  *buf)
{
    guint cursor_row = buf_cursor_row (buf);
    guint screen_height = buf_screen_height (buf);

    g_assert (cursor_row < screen_height);

    if (cursor_row == screen_height - 1)
    {
        _buf_add_lines (buf, 1);

        buf_changed_set_all (buf);

        moo_term_buffer_changed (buf);
        moo_term_buffer_scrollback_changed (buf);
    }
    else
    {
        moo_term_buffer_cursor_move (buf, 1, 0);
    }
}


void    moo_term_buffer_new_line        (MooTermBuffer  *buf)
{
    moo_term_buffer_index (buf);
    moo_term_buffer_cursor_move_to (buf, -1, 0);
}


void    moo_term_buffer_full_reset      (MooTermBuffer  *buf)
{
    g_signal_emit (buf, signals[FULL_RESET], 0);

    buf->priv->top_margin = 0;
    buf->priv->bottom_margin = buf->priv->screen_height - 1;
    buf->priv->scrolling_region_set = FALSE;
    g_object_notify (G_OBJECT (buf), "scrolling-region-set");

    moo_term_buffer_erase_display (buf);
    moo_term_buffer_cursor_move_to (buf, 0, 0);

    g_object_set (buf,
                  "mode-IRM", FALSE,
                  "mode-LNM", FALSE,
                  "mode-DECCKM", FALSE,
                  "mode-DECSCNM", FALSE,
                  "mode-DECOM", FALSE,
                  "mode-DECAWM", TRUE,
                  "mode-MOUSE-TRACKING", FALSE,
                  "mode-HILITE-MOUSE-TRACKING", FALSE,
                  "mode-KEYPAD-NUMERIC", TRUE,
                  NULL);
}


void    moo_term_buffer_feed_child      (MooTermBuffer  *buf,
                                         const char     *string,
                                         int             len)
{
    g_signal_emit (buf, signals[FEED_CHILD], 0, string, len);
}


void    moo_term_buffer_erase_display   (MooTermBuffer  *buf)
{
    guint screen_height = buf_screen_height (buf);
    guint i;

    for (i = 0; i < screen_height; ++i)
        term_line_erase (buf_screen_line (buf, i));

    buf_changed_set_all (buf);
    moo_term_buffer_changed (buf);
}


void    moo_term_buffer_set_keypad_numeric  (MooTermBuffer  *buf,
                                             gboolean        setting)
{
    if (setting)
        buf->priv->modes |= KEYPAD_NUMERIC;
    else
        buf->priv->modes &= ~KEYPAD_NUMERIC;

    g_object_notify (G_OBJECT (buf), "mode-KEYPAD-NUMERIC");
}


void    moo_term_buffer_reset_tab_stops     (MooTermBuffer  *buf)
{
    guint i;
    guint width = buf_screen_width (buf);

    g_list_free (buf->priv->tab_stops);
    buf->priv->tab_stops = NULL;

    for (i = 0; i < (width + 7) / 8; ++i)
        buf->priv->tab_stops = g_list_append (buf->priv->tab_stops,
                                              GUINT_TO_POINTER (8 * i));
}

guint   moo_term_buffer_next_tab_stop       (MooTermBuffer  *buf,
                                             guint           current)
{
    GList *l;

    for (l = buf->priv->tab_stops;
         l != NULL && GPOINTER_TO_UINT (l->data) <= current;
         l = l->next) ;

    if (l && GPOINTER_TO_UINT (l->data) > current)
        return GPOINTER_TO_UINT (l->data);
    else
        return buf_screen_width (buf) - 1;
}

guint   moo_term_buffer_prev_tab_stop       (MooTermBuffer  *buf,
                                             guint           current)
{
    GList *l;

    for (l = buf->priv->tab_stops;
         l != NULL && GPOINTER_TO_UINT (l->data) < current;
         l = l->next) ;

    if (l && l->prev && GPOINTER_TO_UINT (l->prev->data) < current)
        return GPOINTER_TO_UINT (l->prev->data);
    else
        return 0;
}

void    moo_term_buffer_clear_tab_stop      (MooTermBuffer  *buf)
{
    buf->priv->tab_stops =
            g_list_remove (buf->priv->tab_stops,
                           GUINT_TO_POINTER (buf_cursor_col (buf)));
}

static int cmp_guints (gconstpointer a, gconstpointer b)
{
    if (GPOINTER_TO_UINT (a) < GPOINTER_TO_UINT (b))
        return -1;
    else if (a == b)
        return 0;
    else
        return 1;
}

void    moo_term_buffer_set_tab_stop        (MooTermBuffer  *buf)
{
    guint cursor = buf_cursor_col (buf);

    if (!g_list_find (buf->priv->tab_stops, GUINT_TO_POINTER (cursor)))
        buf->priv->tab_stops =
                g_list_insert_sorted (buf->priv->tab_stops,
                                      GUINT_TO_POINTER (cursor),
                                      cmp_guints);
}


static void init_drawing_sets (void)
{
    guint i;

    for (i = 0; i <= MAX_GRAPH; ++i)
    {
        if (DRAWING_SET_STRINGS[i])
            DRAWING_SET[i] = g_utf8_get_char (DRAWING_SET_STRINGS[i]);
    }
}


void    moo_term_buffer_select_charset  (MooTermBuffer  *buf,
                                         guint           set_num,
                                         guint           charset)
{
    g_return_if_fail (set_num < 4 && charset < 5);

    switch (charset)
    {
        case 0:
            if (buf->priv->use_ascii_graphics)
                buf->priv->graph_sets[set_num] = ASCII_DRAWING_SET;
            else
                buf->priv->graph_sets[set_num] = DRAWING_SET;
            break;

        case 2:
            g_warning ("%s: choosing graphics instead of"
                       "Alternate Character ROM Special Set", G_STRLOC);
            if (buf->priv->use_ascii_graphics)
                buf->priv->graph_sets[set_num] = ASCII_DRAWING_SET;
            else
                buf->priv->graph_sets[set_num] = DRAWING_SET;
            break;

        case 1:
            g_warning ("%s: choosing regular charset instead of"
                    "Alternate Character ROM Standard Set", G_STRLOC);
            buf->priv->graph_sets[set_num] = NULL;
            break;

        case 3:
            g_warning ("%s: choosing regular charset instead of"
                       "United Kingdom", G_STRLOC);
            buf->priv->graph_sets[set_num] = NULL;
            break;

        case 4:
            buf->priv->graph_sets[set_num] = NULL;
            break;
    }
}


void    moo_term_buffer_shift           (MooTermBuffer  *buf,
                                         guint           set)
{
    g_return_if_fail (set < 4);
    buf->priv->current_graph_set = buf->priv->graph_sets[set];
}


void    moo_term_buffer_single_shift    (MooTermBuffer  *buf,
                                         guint           set)
{
    g_return_if_fail (set < 4);
    buf->priv->single_shift = set;
}
