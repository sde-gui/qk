/*
 *   mooui/mootext.h
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

#ifndef MOOUI_MOOTEXT_H
#define MOOUI_MOOTEXT_H

#include <gtk/gtk.h>

G_BEGIN_DECLS


#define MOO_TYPE_TEXT             (moo_text_get_type ())
#define MOO_TEXT(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), MOO_TYPE_TEXT, MooText))
#define MOO_IS_TEXT(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MOO_TYPE_TEXT))
#define MOO_TEXT_GET_IFACE(inst)  (G_TYPE_INSTANCE_GET_INTERFACE ((inst), MOO_TYPE_TEXT, MooTextIface))


typedef struct _MooText         MooText;
typedef struct _MooTextIface    MooTextIface;

typedef enum {
    MOO_TEXT_DRAG_NONE = 0,
    MOO_TEXT_DRAG_SELECT,
    MOO_TEXT_DRAG_DRAG
} MooTextDragType;

typedef enum {
    MOO_TEXT_SELECT_CHARS,
    MOO_TEXT_SELECT_WORDS,
    MOO_TEXT_SELECT_LINES
} MooTextSelectionType;

struct _MooTextIface {
    GTypeInterface  parent;

    void     (*middle_button_click)     (MooText            *obj,
                                         GdkEventButton     *event);
    void     (*right_button_click)      (MooText            *obj,
                                         GdkEventButton     *event);

    void     (*start_selection_dnd)     (MooText            *obj,
                                         const GtkTextIter  *iter,
                                         GdkEventMotion     *event);

    gboolean (*extend_selection)        (MooText            *obj,
                                         MooTextSelectionType type,
                                         GtkTextIter        *insert,
                                         GtkTextIter        *selection_bound);
    void     (*window_to_buffer_coords) (MooText            *obj,
                                         int                 window_x,
                                         int                 window_y,
                                         int                *buffer_x,
                                         int                *buffer_y);
    void     (*get_iter_at_location)    (MooText            *obj,
                                         GtkTextIter        *iter,
                                         int                 x,
                                         int                 y);
    gboolean (*get_selection_bounds)    (MooText            *obj,
                                         GtkTextIter        *sel_start,
                                         GtkTextIter        *sel_end);
    void     (*iter_order)              (GtkTextIter        *first,
                                         GtkTextIter        *second);
    gboolean (*iter_in_range)           (const GtkTextIter  *iter,
                                         const GtkTextIter  *start,
                                         const GtkTextIter  *end);
    void     (*place_selection_end)     (MooText            *obj,
                                         const GtkTextIter  *where);
    void     (*select_range)            (MooText            *obj,
                                         const GtkTextIter  *start,
                                         const GtkTextIter  *end);
    void     (*scroll_selection_end_onscreen) (MooText        *obj);

    GdkWindow* (*get_window)            (MooText            *obj);
    void     (*get_visible_rect)        (MooText            *obj,
                                         GdkRectangle       *rect);
};


GType        moo_text_get_type                  (void);

gboolean     moo_text_button_press_event        (GtkWidget          *widget,
                                                 GdkEventButton     *event);
gboolean     moo_text_button_release_event      (GtkWidget          *widget,
                                                 GdkEventButton     *event);
gboolean     moo_text_motion_event              (GtkWidget          *widget,
                                                 GdkEventMotion     *event);


G_END_DECLS

#endif /* MOOUI_MOOTEXT_H */
