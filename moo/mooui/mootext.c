/*
 *   mooui/mootext.c
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

#include "mooui/mootext.h"


typedef struct _MooTextData     MooTextData;
struct _MooTextData {
    guint           drag_scroll_timeout;
    GdkEventType    drag_button;
    MooTextDragType drag_type;
    int             drag_start_x;
    int             drag_start_y;
    guint           drag_moved                      : 1;
    guint           double_click_selects_brackets   : 1;
    guint           double_click_selects_inside     : 1;
};


static const GQuark *get_quark              (void) G_GNUC_CONST;
static MooTextData  *moo_text_data_new      (void);
static MooTextData  *moo_text_data_clear    (MooTextData    *data);
static void          moo_text_data_free     (MooTextData    *data);
static MooTextData  *moo_text_get_data      (MooText        *obj);

#define MOO_TEXT_DATA_QUARK     (get_quark()[0])
#define GET_DATA(obj)           (moo_text_get_data (obj))

typedef gboolean (*ExtendSelectionFunc)     (MooText            *obj,
                                             MooTextSelectionType type,
                                             GtkTextIter        *insert,
                                             GtkTextIter        *selection_bound);
typedef void     (*StartSelectionDndFunc)   (MooText            *obj,
                                             const GtkTextIter  *iter,
                                             GdkEventMotion     *event);


static MooTextData *moo_text_data_new   (void)
{
    return moo_text_data_clear (g_new0 (MooTextData, 1));
}

static MooTextData  *moo_text_data_clear    (MooTextData    *data)
{
    data->drag_moved = FALSE;
    data->drag_type = MOO_TEXT_DRAG_NONE;
    data->drag_start_x = -1;
    data->drag_start_y = -1;
    data->drag_button = GDK_BUTTON_RELEASE;

    return data;
}

static void         moo_text_data_free  (MooTextData    *data)
{
    g_free (data);
}


/**************************************************************************/
/* mouse functionality
 */

#define get_impl(func) MOO_TEXT_GET_IFACE(obj)->func

static gboolean can_drag_selection              (MooText            *obj)
{
    return MOO_TEXT_GET_IFACE(obj)->start_selection_dnd != NULL;
}


static void     start_drag_scroll               (MooText *obj);
static void     stop_drag_scroll                (MooText *obj);
static gboolean drag_scroll_timeout_func        (MooText *obj);
static const int SCROLL_TIMEOUT = 100;


gboolean    moo_text_button_press_event     (GtkWidget          *widget,
                                             GdkEventButton     *event)
{
    MooText *obj;
    GtkTextIter iter;
    MooTextData *data;
    int x, y;

    obj = MOO_TEXT (widget);
    data = moo_text_get_data (obj);

    if (!GTK_WIDGET_HAS_FOCUS (widget))
        gtk_widget_grab_focus (widget);

    get_impl(window_to_buffer_coords) (obj, event->x, event->y, &x, &y);
    get_impl(get_iter_at_location) (obj, &iter, x, y);

    if (event->type == GDK_BUTTON_PRESS)
    {
        if (event->button == 1)
        {
            GtkTextIter sel_start, sel_end;

            data->drag_button = GDK_BUTTON_PRESS;
            data->drag_start_x = x;
            data->drag_start_y = y;

            /* if clicked in selected, start drag */
            if (get_impl(get_selection_bounds) (obj, &sel_start, &sel_end))
            {
                get_impl(iter_order) (&sel_start, &sel_end);
                if (get_impl(iter_in_range) (&iter, &sel_start, &sel_end)
                    && can_drag_selection (obj))
                {
                    /* clicked inside of selection,
                    * set up drag and return */
                    data->drag_type = MOO_TEXT_DRAG_DRAG;
                    return TRUE;
                }
            }

            /* otherwise, clear selection, and position cursor at clicked point */
            if (event->state & GDK_SHIFT_MASK)
            {
                get_impl(place_selection_end) (obj, &iter);
            }
            else
            {
                get_impl(select_range) (obj, &iter, &iter);
            }

            data->drag_type = MOO_TEXT_DRAG_SELECT;
        }
        else if (event->button == 2)
        {
            get_impl(middle_button_click) (obj, event);
        }
        else if (event->button == 3)
        {
            get_impl(right_button_click) (obj, event);
        }
        else
        {
            g_warning ("got button %d in button_press callback", event->button);
        }
    }
    else if (event->type == GDK_2BUTTON_PRESS && event->button == 1)
    {
        GtkTextIter bound;

        if (get_impl(get_selection_bounds) (obj, NULL, NULL))
        {
            /* it may happen sometimes, if you click fast enough */
            get_impl(select_range) (obj, &iter, &iter);
        }

        data->drag_button = GDK_2BUTTON_PRESS;
        data->drag_start_x = x;
        data->drag_start_y = y;
        data->drag_type = MOO_TEXT_DRAG_SELECT;

        bound = iter;

        if (get_impl(extend_selection) (obj, MOO_TEXT_SELECT_WORDS, &iter, &bound))
            get_impl(select_range) (obj, &bound, &iter);
    }
    else if (event->type == GDK_3BUTTON_PRESS && event->button == 1)
    {
        GtkTextIter bound;

        data->drag_button = GDK_3BUTTON_PRESS;
        data->drag_start_x = x;
        data->drag_start_y = y;
        data->drag_type = MOO_TEXT_DRAG_SELECT;

        bound = iter;

        if (get_impl(extend_selection) (obj, MOO_TEXT_SELECT_LINES, &iter, &bound))
            get_impl(select_range) (obj, &bound, &iter);
    }

    return TRUE;
}


gboolean    moo_text_button_release_event   (GtkWidget          *widget,
                                             G_GNUC_UNUSED GdkEventButton     *event)
{
    MooText *obj = MOO_TEXT (widget);
    MooTextData *data = moo_text_get_data (obj);
    GtkTextIter iter;

    switch (data->drag_type)
    {
        case MOO_TEXT_DRAG_NONE:
            /* it may happen after right-click, or clicking outside
            * of widget or something like that
            * everything has been taken care of, so do nothing */
            break;

        case MOO_TEXT_DRAG_SELECT:
            /* everything should be done already in button_press and
            * motion_notify handlers */
            stop_drag_scroll (obj);
            break;

        case MOO_TEXT_DRAG_DRAG:
            /* if we were really dragging, drop it
            * otherwise, it was just a single click in selected text */
            g_assert (!data->drag_moved); /* parent should handle drag */ /* TODO ??? */

            get_impl(get_iter_at_location) (obj, &iter,
                                            data->drag_start_x,
                                            data->drag_start_y);
            get_impl(select_range) (obj, &iter, &iter);
            break;

        default:
            g_assert_not_reached ();
    }

    moo_text_data_clear (data);
    return TRUE;
}


#define outside(x,y,rect)               \
    ((x) < (rect).x ||                  \
     (y) < (rect).y ||                  \
     (x) >= (rect).x + (rect).width ||  \
     (y) >= (rect).y + (rect).height)

gboolean    moo_text_motion_event           (GtkWidget          *widget,
                                             GdkEventMotion     *event)
{
    MooText *obj = MOO_TEXT (widget);
    MooTextData *data = moo_text_get_data (obj);
    int x, y, event_x, event_y;
    GtkTextIter iter;

    if (!data->drag_type)
        return FALSE;

    if (event->is_hint)
    {
        gdk_window_get_pointer (event->window, &event_x, &event_y, NULL);
    }
    else {
        event_x = (int)event->x;
        event_y = (int)event->y;
    }

    get_impl(window_to_buffer_coords) (obj, event_x, event_y, &x, &y);
    get_impl(get_iter_at_location) (obj, &iter, x, y);

    if (data->drag_type == MOO_TEXT_DRAG_SELECT)
    {
        GdkRectangle rect;
        GtkTextIter start;
        MooTextSelectionType t;

        data->drag_moved = TRUE;
        get_impl(get_visible_rect) (obj, &rect);

        if (outside (x, y, rect))
        {
            start_drag_scroll (obj);
            return TRUE;
        }
        else
        {
            stop_drag_scroll (obj);
        }

        get_impl(get_iter_at_location) (obj, &start,
                                        data->drag_start_x,
                                        data->drag_start_y);

        switch (data->drag_button)
        {
            case GDK_BUTTON_PRESS:
                t = MOO_TEXT_SELECT_CHARS;
                break;
            case GDK_2BUTTON_PRESS:
                t = MOO_TEXT_SELECT_WORDS;
                break;
            default:
                t = MOO_TEXT_SELECT_LINES;
        }

        if (get_impl(extend_selection) (obj, t, &iter, &start))
            get_impl(select_range) (obj, &start, &iter);
        else
            get_impl(select_range) (obj, &iter, &iter);
    }
    else
    {
        /* this piece is from gtktextview.c */
        int x, y;

        gdk_window_get_pointer (get_impl(get_window) (obj), &x, &y, NULL);

        if (gtk_drag_check_threshold (widget,
                                      data->drag_start_x,
                                      data->drag_start_y,
                                      x, y))
        {
            GtkTextIter iter;
            int buffer_x, buffer_y;

            get_impl(window_to_buffer_coords) (obj,
                                               data->drag_start_x,
                                               data->drag_start_y,
                                               &buffer_x,
                                               &buffer_y);

            get_impl(get_iter_at_location) (obj, &iter, buffer_x, buffer_y);

            data->drag_type = MOO_TEXT_DRAG_NONE;
            get_impl(start_selection_dnd) (obj, &iter, event);
        }
    }

    return TRUE;
}


static void start_drag_scroll (MooText *obj)
{
    MooTextData *data = moo_text_get_data (obj);

    if (!data->drag_scroll_timeout)
        data->drag_scroll_timeout =
                g_timeout_add (SCROLL_TIMEOUT,
                               (GSourceFunc)drag_scroll_timeout_func,
                               obj);

    drag_scroll_timeout_func (obj);
}


static void stop_drag_scroll (MooText *obj)
{
    MooTextData *data = moo_text_get_data (obj);

    if (data->drag_scroll_timeout)
        g_source_remove (data->drag_scroll_timeout);

    data->drag_scroll_timeout = 0;
}


static gboolean drag_scroll_timeout_func (MooText *obj)
{
    int x, y, px, py;
    GtkTextIter iter;
    GtkTextIter start;
    MooTextSelectionType t;
    MooTextData *data = moo_text_get_data (obj);

    g_assert (data->drag_type == MOO_TEXT_DRAG_SELECT);

    gdk_window_get_pointer (get_impl(get_window) (obj), &px, &py, NULL);
    get_impl(window_to_buffer_coords) (obj, px, py, &x, &y);
    get_impl(get_iter_at_location) (obj, &iter, x, y);

    get_impl(get_iter_at_location) (obj, &start,
                                    data->drag_start_x,
                                    data->drag_start_y);

    switch (data->drag_button)
    {
        case GDK_BUTTON_PRESS:
            t = MOO_TEXT_SELECT_CHARS;
            break;
        case GDK_2BUTTON_PRESS:
            t = MOO_TEXT_SELECT_WORDS;
            break;
        default:
            t = MOO_TEXT_SELECT_LINES;
    }

    if (get_impl(extend_selection) (obj, t, &iter, &start))
        get_impl(select_range) (obj, &start, &iter);
    else
        get_impl(select_range) (obj, &iter, &iter);

    get_impl(scroll_selection_end_onscreen) (obj);

    return TRUE;
}


/**************************************************************************/
/* GInterface stuff
 */

static void     moo_text_iface_init     (gpointer       g_iface);


GType moo_text_get_type (void)
{
    static GType type = 0;

    if (!type)
    {
        static const GTypeInfo info =
        {
            sizeof (MooTextIface),  /* class_size */
            moo_text_iface_init,    /* base_init */
            NULL,                   /* base_finalize */
            NULL,                   /* class_init */
            0, 0, 0, 0, 0, 0
        };

        type = g_type_register_static (G_TYPE_INTERFACE,
                                       "MooText",
                                       &info, (GTypeFlags)0);
    }

    return type;
}


static void    moo_text_iface_init    (G_GNUC_UNUSED gpointer g_iface)
{
}


static const GQuark *get_quark (void)
{
    static GQuark q[1] = {0};

    if (!q[0])
    {
        q[0] = g_quark_from_static_string ("moo_text_data");
    }

    return q;
}


static MooTextData *moo_text_get_data   (MooText        *obj)
{
    MooTextData *data;

    g_return_val_if_fail (MOO_IS_TEXT (obj), NULL);

    data = g_object_get_qdata (G_OBJECT (obj), MOO_TEXT_DATA_QUARK);

    if (!data)
    {
        data = moo_text_data_new ();
        g_object_set_qdata_full (G_OBJECT (obj),
                                 MOO_TEXT_DATA_QUARK,
                                 data,
                                 (GDestroyNotify) moo_text_data_free);
    }

    return data;
}



/**************************************************************************/
/* GInterface stuff
 */

