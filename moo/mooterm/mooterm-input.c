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
 *
 *   moo_term_key_press() code is taken from libvte vte.c,
 *   Copyright (C) 2001-2004 Red Hat, Inc.
 */

#define MOOTERM_COMPILATION
#include "mooterm/mooterm-keymap.h"
#include "mooterm/mootermpt.h"

#define META_MASK   GDK_MOD1_MASK


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


/* shamelessly taken from vte.c */
gboolean    moo_term_key_press          (GtkWidget      *widget,
                                         GdkEventKey    *event)
{
    MooTerm *term;
    char *string = NULL;
    gssize string_length = 0;
    int i;
    gboolean scrolled = FALSE;
    gboolean steal = FALSE;
    gboolean is_modifier = FALSE;
    gboolean handled;
    gboolean suppress_meta_esc = FALSE;
    guint keyval = 0;
    gunichar keychar = 0;
    char keybuf[6];  /* 6 bytes for UTF-8 character */
    GdkModifierType modifiers;

    term = MOO_TERM (widget);

    /* First, check if GtkWidget's behavior already does something with
     * this key. */
    if (widget_class()->key_press_event (widget, event))
    {
        return TRUE;
    }

    keyval = event->keyval;

    /* If it's a keypress, record that we got the event, in case the
     * input method takes the event from us. */
    term->priv->modifiers = modifiers = event->state;
    modifiers &= (GDK_SHIFT_MASK | GDK_CONTROL_MASK | META_MASK);

    /* Determine if this is just a modifier key. */
    is_modifier = key_is_modifier (keyval);

    /* Unless it's a modifier key, hide the pointer. */
    if (!is_modifier &&
         term->priv->settings.hide_pointer_on_keypress &&
         moo_term_pt_child_alive (term->priv->pt))
    {
        moo_term_set_pointer_visible (term, FALSE);
    }

    /* We steal many keypad keys here. */
    if (!term->priv->im_preedit_active)
    {
        switch (keyval)
        {
            CASE_GDK_KP_SOMETHING
                steal = TRUE;
        }

        if (modifiers & META_MASK)
        {
            steal = TRUE;
        }
    }

    /* Let the input method at this one first. */
    if (!steal)
    {
        if (gtk_im_context_filter_keypress(term->priv->im, event))
            return TRUE;
    }

    if (is_modifier)
        return FALSE;

    /* Now figure out what to send to the child. */
    handled = FALSE;

    switch (keyval)
    {
        case GDK_BackSpace:
            get_backspace_key (term, &string,
                               &string_length,
                               &suppress_meta_esc);
            handled = TRUE;
            break;

        case GDK_Delete:
            get_delete_key (term, &string,
                            &string_length,
                            &suppress_meta_esc);
            handled = TRUE;
            suppress_meta_esc = TRUE;
            break;

        case GDK_Insert:
            if (modifiers & GDK_SHIFT_MASK)
            {
                moo_term_paste_clipboard (term);
                handled = TRUE;
                suppress_meta_esc = TRUE;
            }
            else if (modifiers & GDK_CONTROL_MASK)
            {
                moo_term_copy_clipboard (term);
                handled = TRUE;
                suppress_meta_esc = TRUE;
            }
            break;

        case GDK_Page_Up:
        case GDK_KP_Page_Up:
            if (modifiers & GDK_SHIFT_MASK)
            {
                moo_term_scroll_pages (term, -1);
                scrolled = TRUE;
                handled = TRUE;
                suppress_meta_esc = TRUE;
            }
            break;

        case GDK_Page_Down:
        case GDK_KP_Page_Down:
            if (modifiers & GDK_SHIFT_MASK)
            {
                moo_term_scroll_pages (term, 1);
                scrolled = TRUE;
                handled = TRUE;
                suppress_meta_esc = TRUE;
            }
            break;

        case GDK_Home:
        case GDK_KP_Home:
            if (modifiers & GDK_SHIFT_MASK)
            {
                moo_term_scroll_to_top (term);
                scrolled = TRUE;
                handled = TRUE;
            }
            break;
        case GDK_End:
        case GDK_KP_End:
            if (modifiers & GDK_SHIFT_MASK)
            {
                moo_term_scroll_to_bottom (term);
                scrolled = TRUE;
                handled = TRUE;
            }
            break;
        case GDK_Up:
        case GDK_KP_Up:
            if (modifiers & GDK_SHIFT_MASK)
            {
                moo_term_scroll_lines (term, -1);
                scrolled = TRUE;
                handled = TRUE;
            }
            break;
        case GDK_Down:
        case GDK_KP_Down:
            if (modifiers & GDK_SHIFT_MASK)
            {
                moo_term_scroll_lines (term, 1);
                scrolled = TRUE;
                handled = TRUE;
            }
            break;

        case GDK_Break:
            moo_term_ctrl_c (term);
            handled = TRUE;
            break;
    }

    /* If the above switch statement didn't do the job, try mapping
     * it to a literal or capability name. */
    if (!handled)
    {
        if (!(modifiers & GDK_CONTROL_MASK))
            get_vt_key (term, keyval, &string, &string_length);
        else
            get_vt_ctl_key (term, keyval, &string, &string_length);

        /* If we found something this way, suppress
            * escape-on-meta. */
        if (string != NULL && string_length > 0)
            suppress_meta_esc = TRUE;
    }

    /* If we didn't manage to do anything, try to salvage a
        * printable string. */
    if (!handled && !string)
    {
        /* Convert the keyval to a gunichar. */
        keychar = gdk_keyval_to_unicode(keyval);
        string_length = 0;

        if (keychar != 0)
        {
            /* Convert the gunichar to a string. */
            string_length = g_unichar_to_utf8(keychar, keybuf);

            if (string_length)
            {
                string = g_malloc0 (string_length + 1);
                memcpy (string, keybuf, string_length);
            }
            else
            {
                string = NULL;
            }
        }

        if (string && (modifiers & GDK_CONTROL_MASK))
        {
            /* Replace characters which have "control"
                * counterparts with those counterparts. */
            for (i = 0; i < string_length; i++)
            {
                if ((((guint8)string[i]) >= 0x40) &&
                       (((guint8)string[i]) <  0x80))
                {
                    string[i] &= (~(0x60));
                }
            }
        }
    }

    /* If we got normal characters, send them to the child. */
    if (string)
    {
        if (moo_term_pt_child_alive (term->priv->pt))
        {
            if (term->priv->settings.meta_sends_escape &&
                !suppress_meta_esc &&
                string_length > 0 &&
                (modifiers & META_MASK))
            {
                moo_term_feed_child (term, VT_ESC_, 1);
            }

            if (string_length > 0)
            {
                moo_term_feed_child (term, string, string_length);
            }
        }

        /* Keep the cursor on-screen. */
        if (!scrolled && term->priv->settings.scroll_on_keystroke)
            moo_term_scroll_to_bottom (term);

        g_free(string);
    }

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


/****************************************************************************/
/*  Mouse handling
 */

static void     start_press_tracking    (MooTerm        *term);
static void     start_button_tracking   (MooTerm        *term);
static void     stop_mouse_tracking     (MooTerm        *term);

static gboolean button_press            (MooTerm        *term,
                                         GdkEventButton *event);
static gboolean button_press_or_release (MooTerm        *term,
                                         GdkEventButton *event);
static gboolean scroll_event            (MooTerm        *term,
                                         GdkEventScroll *event);


void        moo_term_set_mouse_tracking (MooTerm        *term,
                                         int             tracking_type)
{
    if (tracking_type != term->priv->tracking_mouse)
    {
        term->priv->tracking_mouse = tracking_type;
        moo_term_update_pointer (term);
        stop_mouse_tracking (term);
        term_selection_clear (term);

        switch (tracking_type)
        {
            case MODE_PRESS_TRACKING:
                start_press_tracking (term);
                break;

            case MODE_PRESS_AND_RELEASE_TRACKING:
                term->priv->tracking_mouse = TRUE;
                start_button_tracking (term);
                break;

            default:
                term->priv->tracking_mouse = FALSE;
                stop_mouse_tracking (term);
                break;
        }
    }
}


static void stop_mouse_tracking         (MooTerm    *term)
{
    if (term->priv->track_press_id)
        g_signal_handler_disconnect (term, term->priv->track_press_id);
    if (term->priv->track_release_id)
        g_signal_handler_disconnect (term, term->priv->track_release_id);
    if (term->priv->track_scroll_id)
        g_signal_handler_disconnect (term, term->priv->track_scroll_id);
    term->priv->track_press_id = 0;
    term->priv->track_release_id = 0;
    term->priv->track_scroll_id = 0;
}


static void start_press_tracking        (MooTerm    *term)
{
    term->priv->track_press_id =
            g_signal_connect (term, "button-press-event",
                              G_CALLBACK (button_press), NULL);
}


static void start_button_tracking       (MooTerm    *term)
{
    term->priv->track_press_id =
            g_signal_connect (term, "button-press-event",
                              G_CALLBACK (button_press_or_release), NULL);
    term->priv->track_release_id =
            g_signal_connect (term, "button-release-event",
                              G_CALLBACK (button_press_or_release), NULL);
    term->priv->track_scroll_id =
            g_signal_connect (term, "scroll-event",
                              G_CALLBACK (scroll_event), NULL);
}


static void     get_mouse_coordinates   (MooTerm        *term,
                                         GdkEventButton *event,
                                         int            *x,
                                         int            *y)
{
    guint char_width = term_char_width (term);
    guint char_height = term_char_height (term);

    *x = event->x / char_width;
    *x = CLAMP (*x, 0, (int)term->priv->width - 1);

    *y = event->y / char_height;
    *y = CLAMP (*y, 0, (int)term->priv->height - 1);
}


static gboolean button_press            (MooTerm        *term,
                                         GdkEventButton *event)
{
    int x, y;
    guchar button;
    char *string;

    if (event->type != GDK_BUTTON_PRESS  ||
        event->button < 1 || event->button > 5)
    {
        return TRUE;
    }

    if (event->button < 4)
        button = 040 + event->button - 1;
    else
        button = 0140 + event->button - 4;

    get_mouse_coordinates (term, event, &x, &y);

    string = g_strdup_printf (VT_CSI_ "M%c%c%c", button,
                              (guchar) (x + 1 + 040),
                              (guchar) (y + 1 + 040));

    moo_term_feed_child (term, string, 6);

    g_free (string);
    return TRUE;
}


static gboolean button_press_or_release (MooTerm        *term,
                                         GdkEventButton *event)
{
    int x, y;
    guint button;
    char *string;

    if ((event->type != GDK_BUTTON_PRESS && event->type != GDK_BUTTON_RELEASE) ||
        event->button < 1 || event->button > 5)
    {
        return TRUE;
    }

    get_mouse_coordinates (term, event, &x, &y);

    if (event->type == GDK_BUTTON_PRESS)
    {
        if (event->button < 4)
            button = event->button - 1;
        else
            button = 0100 + event->button - 4;
    }
    else
    {
        button = 3;
    }

    if (event->state & GDK_SHIFT_MASK)
        button |= 4;
    if (event->state & META_MASK)
        button |= 8;
    if (event->state & GDK_CONTROL_MASK)
        button |= 16;

    string = g_strdup_printf (VT_CSI_ "M%c%c%c",
                              (guchar) (button + 040),
                              (guchar) (x + 1 + 040),
                              (guchar) (y + 1 + 040));

    moo_term_feed_child (term, string, 6);

    g_free (string);
    return TRUE;
}


static gboolean scroll_event            (MooTerm        *term,
                                         GdkEventScroll *event)
{
    int x, y;
    guchar button;
    char *string;

    if (event->direction != GDK_SCROLL_UP &&
        event->direction != GDK_SCROLL_DOWN)
    {
        return TRUE;
    }

    get_mouse_coordinates (term, (GdkEventButton*)event, &x, &y);

    if (event->direction == GDK_SCROLL_UP)
        button = 0100;
    else
        button = 0101;

    if (event->state & GDK_SHIFT_MASK)
        button |= 4;
    if (event->state & META_MASK)
        button |= 8;
    if (event->state & GDK_CONTROL_MASK)
        button |= 16;

    string = g_strdup_printf (VT_CSI_ "M%c%c%c",
                              (guchar) (button + 040),
                              (guchar) (x + 1 + 040),
                              (guchar) (y + 1 + 040));

    moo_term_feed_child (term, string, 6);

    g_free (string);
    return TRUE;
}
