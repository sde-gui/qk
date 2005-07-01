/*
 *   mooterm/mootermcursor.c
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
#include "mooterm/mooterm-private.h"
#include <gdk/gdkkeysyms.h>

#define MANY_CHARS 1024


enum {
    KEY_BACKSPACE = 0,
    KEY_TAB,
    KEY_BACK_TAB,
    KEY_LINEFEED,
    KEY_RETURN,
    KEY_ESCAPE,
    KEY_DELETE,
    KEY_HOME,
    KEY_LEFT,
    KEY_UP,
    KEY_RIGHT,
    KEY_DOWN,
    KEY_END,
    KEY_BEGIN,
    KEY_F1,
    KEY_F2,
    KEY_F3,
    KEY_F4,
    KEY_F5,
    KEY_F6,
    KEY_F7,
    KEY_F8,
    KEY_F9,
    KEY_F10,
    KEY_F11,
    KEY_F12,
    KEY_F13,
    KEY_F14,
    KEY_F15,
    KEY_F16,
    KEY_F17,
    KEY_F18,
    KEY_F19,
    KEY_F20,
    KEY_F21,
    KEY_F22,
    KEY_F23,
    KEY_F24,
    KEY_F25,
    KEY_F26,
    KEY_F27,
    KEY_F28,
    KEY_F29,
    KEY_F30,
    KEY_F31,
    KEY_F32,
    KEY_F33,
    KEY_F34,
    KEY_F35,
    KEY_MAX
};

typedef struct {
    const char *str;
    guint len;
} KeyString;

static KeyString xterm_keys[KEY_MAX] =
{
    { "\177" /* KEY_BACKSPACE = 0, */, 1},
    { "\t" /* KEY_TAB, */, 1},
    { "\033[Z" /* KEY_BACK_TAB, */, 3},
    { "\n" /* KEY_LINEFEED, */, 1},
    { "\n" /* KEY_RETURN, */, 1},
    { "\033" /* KEY_ESCAPE, */, 1},
    { "\033[P" /* KEY_DELETE, */, 3},
    { "\033OH" /* KEY_HOME, */, 3},
    { "\010" /* KEY_LEFT, */, 1},
    { "\033[A" /* KEY_UP, */, 3},
    { "\033[C" /* KEY_RIGHT, */, 3},
    { "\n" /* KEY_DOWN, */, 1},
    { "\033OF" /* KEY_END, */, 3},
    { "\033OH" /* KEY_BEGIN, */, 3},
        /* F# keys */
    { "\033OP", 3},
    { "\033OQ", 3},
    { "\033OR", 3},
    { "\033OS", 3},
    { "\033[15~", 5},
    { "\033[17~", 5},
    { "\033[18~", 5},
    { "\033[19~", 5},
    { "\033[20~", 5},
    { "\033[21~", 5},
        /* F11 */
    { "\033[23~", 5},
    { "\033[24~", 5},
    { "\033O2P", 4},
    { "\033O2Q", 4},
    { "\033O2R", 4},
    { "\033O2S", 4},
    { "\033[15;2~", 7},
    { "\033[17;2~", 7},
    { "\033[18;2~", 7},
    { "\033[19;2~", 7},
        /* F21 */
    { "\033[20;2~", 7},
    { "\033[21;2~", 7},
    { "\033[23;2~", 7},
    { "\033[24;2~", 7},
    { "\033O5P", 4},
    { "\033O5Q", 4},
    { "\033O5R", 4},
    { "\033O5S", 4},
    { "\033[15;5~", 7},
    { "\033[17;5~", 7},
        /* F31 */
    { "\033[18;5~", 7},
    { "\033[19;5~", 7},
    { "\033[20;5~", 7},
    { "\033[21;5~", 7},
    { "\033[23;5~", 7}
};


#define assign(num)                 \
    *string = xterm_keys[num].str;  \
    *len = xterm_keys[num].len;

static void get_xterm_key   (guint          key,
                             const char   **string,
                             guint         *len)
{
    switch (key)
    {
        case GDK_BackSpace:
            assign (KEY_BACKSPACE); break;
        case GDK_Tab:
            assign (KEY_TAB); break;
        case GDK_Linefeed:
            assign (KEY_LINEFEED); break;
        case GDK_Return:
            assign (KEY_RETURN); break;
        case GDK_Escape:
            assign (KEY_ESCAPE); break;
        case GDK_Delete:
            assign (KEY_DELETE); break;
        case GDK_Home:
            assign (KEY_HOME); break;
        case GDK_Left:
            assign (KEY_LEFT); break;
        case GDK_Up:
            assign (KEY_UP); break;
        case GDK_Right:
            assign (KEY_RIGHT); break;
        case GDK_Down:
            assign (KEY_DOWN); break;
        case GDK_End:
            assign (KEY_END); break;
        case GDK_Begin:
            assign (KEY_BEGIN); break;
        case GDK_F1:
            assign (KEY_F1); break;
        case GDK_F2:
            assign (KEY_F2); break;
        case GDK_F3:
            assign (KEY_F3); break;
        case GDK_F4:
            assign (KEY_F4); break;
        case GDK_F5:
            assign (KEY_F5); break;
        case GDK_F6:
            assign (KEY_F6); break;
        case GDK_F7:
            assign (KEY_F7); break;
        case GDK_F8:
            assign (KEY_F8); break;
        case GDK_F9:
            assign (KEY_F9); break;
        case GDK_F10:
            assign (KEY_F10); break;
        case GDK_F11:
            assign (KEY_F11); break;
        case GDK_F12:
            assign (KEY_F12); break;
        case GDK_F13:
            assign (KEY_F13); break;
        case GDK_F14:
            assign (KEY_F14); break;
        case GDK_F15:
            assign (KEY_F15); break;
        case GDK_F16:
            assign (KEY_F16); break;
        case GDK_F17:
            assign (KEY_F17); break;
        case GDK_F18:
            assign (KEY_F18); break;
        case GDK_F19:
            assign (KEY_F19); break;
        case GDK_F20:
            assign (KEY_F20); break;
        case GDK_F21:
            assign (KEY_F21); break;
        case GDK_F22:
            assign (KEY_F22); break;
        case GDK_F23:
            assign (KEY_F23); break;
        case GDK_F24:
            assign (KEY_F24); break;
        case GDK_F25:
            assign (KEY_F25); break;
        case GDK_F26:
            assign (KEY_F26); break;
        case GDK_F27:
            assign (KEY_F27); break;
        case GDK_F28:
            assign (KEY_F28); break;
        case GDK_F29:
            assign (KEY_F29); break;
        case GDK_F30:
            assign (KEY_F30); break;
        case GDK_F31:
            assign (KEY_F31); break;
        case GDK_F32:
            assign (KEY_F32); break;
        case GDK_F33:
            assign (KEY_F33); break;
        case GDK_F34:
            assign (KEY_F34); break;
        case GDK_F35:
            assign (KEY_F35); break;
        default:
            *string = NULL;
            *len = 0;
    }
}


static GtkWidgetClass *widget_class (void)
{
    static GtkWidgetClass *klass = NULL;
    if (!klass)
        klass = GTK_WIDGET_CLASS (g_type_class_peek (GTK_TYPE_WIDGET));
    return klass;
}


void        moo_term_im_commit          (G_GNUC_UNUSED GtkIMContext   *imcontext,
                                         gchar          *arg,
                                         MooTerm        *term)
{
    moo_term_feed_child (term, arg, -1);
}


gboolean    moo_term_key_press          (GtkWidget      *widget,
                                         GdkEventKey    *event)
{
    MooTerm *term = MOO_TERM (widget);
    gboolean handled = FALSE;
    gboolean scroll = FALSE;
    gboolean clear_selection = FALSE;
    char buffer[MANY_CHARS];
    const char *string = NULL;
    char *freeme = NULL;
    guint len = 0;
    guint key = event->keyval;
    GdkModifierType mods = event->state & (GDK_SHIFT_MASK | GDK_CONTROL_MASK | GDK_MOD1_MASK);

    if (gtk_im_context_filter_keypress (term->priv->im, event))
    {
        handled = TRUE;
        scroll = TRUE;
        clear_selection = TRUE;
    }

    if (!mods)
    {
        get_xterm_key (key, &string, &len);

        if (string)
        {
            handled = TRUE;
            scroll = TRUE;
            clear_selection = TRUE;
        }
    }
    else if (mods == GDK_SHIFT_MASK)
    {
        switch (key)
        {
            case GDK_Begin:
            case GDK_Home:
                moo_term_scroll_to_top (term);
                handled = TRUE;
                break;
            case GDK_Up:
                moo_term_scroll_lines (term, -1);
                handled = TRUE;
                break;
            case GDK_Down:
                moo_term_scroll_lines (term, 1);
                handled = TRUE;
                break;
            case GDK_Page_Up:
                moo_term_scroll_pages (term, -1);
                handled = TRUE;
                break;
            case GDK_Page_Down:
                moo_term_scroll_pages (term, 1);
                handled = TRUE;
                break;
            case GDK_End:
                moo_term_scroll_to_bottom (term);
                handled = TRUE;
                break;
            case GDK_Insert:
                moo_term_paste_clipboard (term);
                handled = TRUE;
                break;
        }
    }
    else if (mods == GDK_CONTROL_MASK)
    {
        switch (key)
        {
            case GDK_Insert:
                moo_term_copy_clipboard (term);
                handled = TRUE;
                break;
            case GDK_Break:
            case GDK_C:
            case GDK_c:
                moo_term_ctrl_c (term);
                handled = TRUE;
                break;
        }
    }
    else if (mods == (GDK_SHIFT_MASK | GDK_CONTROL_MASK))
    {
    }

    if (clear_selection)
        term_selection_clear (term);
    if (string)
        moo_term_feed_child (term, string, len);
    if (handled && term->priv->scroll_on_keystroke && scroll)
        moo_term_scroll_to_bottom (term);

    g_free (freeme);

    if (!handled)
        return widget_class()->key_press_event (widget, event);
    else
        return TRUE;
}


gboolean    moo_term_key_release        (GtkWidget      *widget,
                                         GdkEventKey    *event)
{
    MooTerm *term = MOO_TERM (widget);
    if (!gtk_im_context_filter_keypress (term->priv->im, event))
        return widget_class()->key_release_event (widget, event);
    else
        return TRUE;
}
