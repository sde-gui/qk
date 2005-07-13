/*
 *   mooterm/mooterminput.c
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
#include "mooterm/mooterm-keymap.h"
#include "mooterm/mootermpt.h"

/* must be enough to fit '^' + one unicode character + 0 byte */
#define MANY_CHARS 16


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
    if (moo_term_pt_child_alive (term->priv->pt))
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
    else if (ignore (key))
    {
        handled = TRUE;
    }
    else if (!mods)
    {
        if (get_vt_key (term, event->keyval, &string, &len))
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
            case GDK_Up:
                moo_term_scroll_lines (term, -1);
                handled = TRUE;
                break;
            case GDK_Down:
                moo_term_scroll_lines (term, 1);
                handled = TRUE;
                break;

            case GDK_Insert:
                moo_term_paste_clipboard (term);
                handled = TRUE;
                scroll = TRUE;
                clear_selection = TRUE;
                break;

            default:
                /* ignore Shift key*/
                if (get_vt_key (term, event->keyval, &string, &len))
                {
                    handled = TRUE;
                    scroll = TRUE;
                    clear_selection = TRUE;
                }
        }
    }
    else if (mods == GDK_CONTROL_MASK)
    {
        if (get_vt_ctl_key (term, event->keyval, &string, &len))
        {
            handled = TRUE;
            scroll = TRUE;
            clear_selection = TRUE;
        }
        else
        {
            switch (key)
            {
                case GDK_Insert:
                    moo_term_copy_clipboard (term);
                    handled = TRUE;
                    break;

                case GDK_Break:
                    get_vt_ctl_key (term, GDK_C, &string, &len);
                    handled = TRUE;
                    scroll = TRUE;
                    clear_selection = TRUE;
                    break;
            }
        }
    }
    else if (mods == (GDK_SHIFT_MASK | GDK_CONTROL_MASK))
    {
    }


    if (!handled)
    {
        gunichar c = gdk_keyval_to_unicode (event->keyval);

        if (c && g_unichar_isgraph (c))
        {
            len = 0;

            if (mods & GDK_CONTROL_MASK)
                buffer[len++] = '^';

            len += g_unichar_to_utf8 (c, &buffer[len]);
            buffer[len] = 0;
            string = buffer;

            handled = TRUE;
            scroll = TRUE;
            clear_selection = TRUE;
        }
    }

    if (clear_selection)
        term_selection_clear (term);

    if (string && moo_term_pt_child_alive (term->priv->pt))
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


void        moo_term_set_mouse_tracking     (MooTerm    *term,
                                             int         tracking_type)
{
    term_implement_me ();
}
