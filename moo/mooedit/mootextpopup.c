/*
 *   mootextpopup.c
 *
 *   Copyright (C) 2004-2006 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   See COPYING file that comes with this distribution.
 */

#include "mooedit/mootextpopup.h"
#include "mooutils/moomarshals.h"
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>


#define MAX_POPUP_LEN 10


struct _MooTextPopupPrivate {
    GtkTreeView *treeview;
    GtkTreeSelection *selection;
    GtkTreeViewColumn *column;
    GtkTreeModel *model;

    GtkTextView *doc;
    GtkTextBuffer *buffer;
    GtkTextMark *pos;

    int max_len;
    guint hide_on_activate : 1;

    GtkWidget *window;
    GtkScrolledWindow *scrolled_window;
    guint resize_idle;
    guint visible : 1;
    guint in_resize : 1;
};


static void     moo_text_popup_set_property (GObject        *object,
                                             guint           prop_id,
                                             const GValue   *value,
                                             GParamSpec     *pspec);
static void     moo_text_popup_get_property (GObject        *object,
                                             guint           prop_id,
                                             GValue         *value,
                                             GParamSpec     *pspec);
static void     moo_text_popup_dispose      (GObject        *object);

static void     moo_text_popup_show_real    (MooTextPopup   *popup);
static void     moo_text_popup_hide_real    (MooTextPopup   *popup);

static void     moo_text_popup_ensure_popup (MooTextPopup   *popup);
static gboolean moo_text_popup_empty        (MooTextPopup   *popup);
static void     moo_text_popup_resize       (MooTextPopup   *popup);
static void     moo_text_popup_connect      (MooTextPopup   *popup);
static void     moo_text_popup_disconnect   (MooTextPopup   *popup);
static void     moo_text_popup_emit_changed (MooTextPopup   *popup);
static void     moo_text_popup_select_first (MooTextPopup   *popup);


G_DEFINE_TYPE (MooTextPopup, moo_text_popup, G_TYPE_OBJECT)

enum {
    SHOW,
    HIDE,
    ACTIVATE,
    TEXT_CHANGED,
    NUM_SIGNALS
};

enum {
    PROP_0,
    PROP_HIDE_ON_ACTIVATE,
    PROP_MAX_LEN,
    PROP_POSITION,
    PROP_DOC,
    PROP_MODEL
};

static guint signals[NUM_SIGNALS];


static void
moo_text_popup_dispose (GObject *object)
{
    MooTextPopup *popup = MOO_TEXT_POPUP (object);

    if (popup->priv)
    {
        moo_text_popup_hide (popup);
        moo_text_popup_set_doc (popup, NULL);
        moo_text_popup_set_model (popup, NULL);

        if (popup->priv->window)
            gtk_widget_destroy (popup->priv->window);

        g_free (popup->priv);
        popup->priv = NULL;
    }

    G_OBJECT_CLASS(moo_text_popup_parent_class)->dispose (object);
}


static void
moo_text_popup_class_init (MooTextPopupClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->dispose = moo_text_popup_dispose;
    gobject_class->set_property = moo_text_popup_set_property;
    gobject_class->get_property = moo_text_popup_get_property;

    klass->show = moo_text_popup_show_real;
    klass->hide = moo_text_popup_hide_real;

    g_object_class_install_property (gobject_class,
                                     PROP_HIDE_ON_ACTIVATE,
                                     g_param_spec_boolean ("hide-on-activate",
                                             "hide-on-activate",
                                             "hide-on-activate",
                                             TRUE,
                                             G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class,
                                     PROP_MAX_LEN,
                                     g_param_spec_int ("max-len",
                                             "max-len",
                                             "max-len",
                                             1, G_MAXINT, MAX_POPUP_LEN,
                                             G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class,
                                     PROP_POSITION,
                                     g_param_spec_boxed ("position",
                                             "position",
                                             "position",
                                             GTK_TYPE_TEXT_ITER,
                                             G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class,
                                     PROP_DOC,
                                     g_param_spec_object ("doc",
                                             "doc",
                                             "doc",
                                             GTK_TYPE_TEXT_VIEW,
                                             G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class,
                                     PROP_MODEL,
                                     g_param_spec_object ("model",
                                             "model",
                                             "model",
                                             GTK_TYPE_TREE_MODEL,
                                             G_PARAM_READWRITE));

    signals[SHOW] =
            g_signal_new ("show",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooTextPopupClass, show),
                          NULL, NULL,
                          _moo_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);

    signals[HIDE] =
            g_signal_new ("hide",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooTextPopupClass, hide),
                          NULL, NULL,
                          _moo_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);

    signals[ACTIVATE] =
            g_signal_new ("activate",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooTextPopupClass, activate),
                          NULL, NULL,
                          _moo_marshal_VOID__OBJECT_BOXED,
                          G_TYPE_NONE, 2,
                          GTK_TYPE_TREE_MODEL,
                          GTK_TYPE_TREE_ITER);

    signals[TEXT_CHANGED] =
            g_signal_new ("text-changed",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooTextPopupClass, text_changed),
                          NULL, NULL,
                          _moo_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);
}


static void
moo_text_popup_init (MooTextPopup *popup)
{
    popup->priv = g_new0 (MooTextPopupPrivate, 1);
    popup->column = popup->priv->column = gtk_tree_view_column_new ();
    popup->priv->max_len = MAX_POPUP_LEN;
    popup->priv->hide_on_activate = TRUE;
}


static void
moo_text_popup_ensure_popup (MooTextPopup *popup)
{
    if (!popup->priv->window)
    {
        GtkWidget *scrolled_window, *frame;

        popup->priv->window = gtk_window_new (GTK_WINDOW_POPUP);
        gtk_window_set_default_size (GTK_WINDOW (popup->priv->window), 1, 1);
        gtk_window_set_resizable (GTK_WINDOW (popup->priv->window), FALSE);
        gtk_widget_add_events (popup->priv->window, GDK_KEY_PRESS_MASK | GDK_BUTTON_PRESS_MASK);

        scrolled_window = gtk_scrolled_window_new (NULL, NULL);
        popup->priv->scrolled_window = GTK_SCROLLED_WINDOW (scrolled_window);
        gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                        GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
        gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_window),
                                             GTK_SHADOW_ETCHED_IN);
        /* a nasty hack to get the completions treeview to size nicely */
        gtk_widget_set_size_request (GTK_SCROLLED_WINDOW (scrolled_window)->vscrollbar, -1, 0);
        gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_window), GTK_SHADOW_NONE);

        frame = gtk_frame_new (NULL);
        gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_OUT);
        gtk_container_add (GTK_CONTAINER (frame), scrolled_window);
        gtk_container_add (GTK_CONTAINER (popup->priv->window), frame);

        popup->priv->treeview = GTK_TREE_VIEW (gtk_tree_view_new ());
        gtk_container_add (GTK_CONTAINER (scrolled_window),
                           GTK_WIDGET (popup->priv->treeview));
        gtk_tree_view_set_headers_visible (popup->priv->treeview, FALSE);
        gtk_tree_view_append_column (popup->priv->treeview,
                                     popup->priv->column);
#if GTK_CHECK_VERSION(2,6,0)
        gtk_tree_view_set_hover_selection (popup->priv->treeview, TRUE);
#endif

        gtk_widget_show_all (frame);

        popup->priv->selection = gtk_tree_view_get_selection (popup->priv->treeview);
        gtk_tree_selection_set_mode (popup->priv->selection, GTK_SELECTION_SINGLE);

        if (popup->priv->model)
            gtk_tree_view_set_model (popup->priv->treeview, popup->priv->model);
    }
}


gboolean
moo_text_popup_show (MooTextPopup      *popup,
                     const GtkTextIter *where)
{
    g_return_val_if_fail (MOO_IS_TEXT_POPUP (popup), FALSE);
    g_return_val_if_fail (where != NULL || popup->priv->pos != NULL, FALSE);
    g_return_val_if_fail (popup->priv->doc != NULL, FALSE);
    g_return_val_if_fail (popup->priv->model != NULL, FALSE);

    moo_text_popup_set_position (popup, where);

    if (!popup->priv->visible)
        g_signal_emit (popup, signals[SHOW], 0);

    return popup->priv->visible != 0;
}


void
moo_text_popup_update (MooTextPopup *popup)
{
    g_return_if_fail (MOO_IS_TEXT_POPUP (popup));

    if (!popup->priv->visible || popup->priv->in_resize)
        return;

    popup->priv->in_resize = TRUE;

    if (moo_text_popup_empty (popup))
    {
        moo_text_popup_hide (popup);
    }
    else
    {
        moo_text_popup_resize (popup);

        if (!moo_text_popup_get_selected (popup, NULL))
            moo_text_popup_select_first (popup);
    }

    popup->priv->in_resize = FALSE;
}


void
moo_text_popup_hide (MooTextPopup *popup)
{
    g_return_if_fail (MOO_IS_TEXT_POPUP (popup));

    if (popup->priv->visible)
        g_signal_emit (popup, signals[HIDE], 0);
}


void
moo_text_popup_activate (MooTextPopup *popup)
{
    GtkTreeIter iter;
    gboolean got_selected;

    g_return_if_fail (MOO_IS_TEXT_POPUP (popup));
    g_return_if_fail (popup->priv->visible);

    got_selected = gtk_tree_selection_get_selected (popup->priv->selection, NULL, &iter);

    if (got_selected)
        g_signal_emit (popup, signals[ACTIVATE], 0, popup->priv->model, &iter);

    if (popup->priv->hide_on_activate)
        moo_text_popup_hide (popup);
}


static gboolean
moo_text_popup_empty (MooTextPopup *popup)
{
    return !popup->priv->model ||
            gtk_tree_model_iter_n_children (popup->priv->model, NULL) == 0;
}


static void
moo_text_popup_show_real (MooTextPopup *popup)
{
    GtkWidget *toplevel;

    g_return_if_fail (popup->priv->doc != NULL);
    g_return_if_fail (GTK_WIDGET_REALIZED (popup->priv->doc));

    if (popup->priv->visible || moo_text_popup_empty (popup))
        return;

    moo_text_popup_ensure_popup (popup);

    toplevel = gtk_widget_get_toplevel (GTK_WIDGET (popup->priv->doc));
    g_return_if_fail (GTK_IS_WINDOW (toplevel));

    gtk_widget_realize (popup->priv->window);

    if (GTK_WINDOW (toplevel)->group)
        gtk_window_group_add_window (GTK_WINDOW (toplevel)->group,
                                     GTK_WINDOW (popup->priv->window));
    gtk_window_set_modal (GTK_WINDOW (popup->priv->window), TRUE);

    moo_text_popup_resize (popup);

    gtk_widget_show (popup->priv->window);
    popup->priv->visible = TRUE;

    gtk_widget_ensure_style (GTK_WIDGET (popup->priv->treeview));
    gtk_widget_modify_bg (GTK_WIDGET (popup->priv->treeview), GTK_STATE_ACTIVE,
                          &GTK_WIDGET(popup->priv->treeview)->style->base[GTK_STATE_SELECTED]);
    gtk_widget_modify_base (GTK_WIDGET (popup->priv->treeview), GTK_STATE_ACTIVE,
                            &GTK_WIDGET(popup->priv->treeview)->style->base[GTK_STATE_SELECTED]);

    gtk_grab_add (popup->priv->window);
    gdk_pointer_grab (popup->priv->window->window, TRUE,
                      GDK_BUTTON_PRESS_MASK |
                              GDK_BUTTON_RELEASE_MASK |
                              GDK_POINTER_MOTION_MASK,
                      NULL, NULL, GDK_CURRENT_TIME);

    if (!moo_text_popup_get_selected (popup, NULL))
        moo_text_popup_select_first (popup);

    moo_text_popup_connect (popup);
}


static void
moo_text_popup_hide_real (MooTextPopup *popup)
{
    if (popup->priv->visible)
    {
        popup->priv->visible = FALSE;

        moo_text_popup_disconnect (popup);

        gdk_pointer_ungrab (GDK_CURRENT_TIME);
        gtk_grab_remove (popup->priv->window);
        gtk_widget_hide (popup->priv->window);
    }
}


static gboolean
resize_in_idle (MooTextPopup *popup)
{
    moo_text_popup_resize (popup);
    return FALSE;
}


static void
window_size_request (MooTextPopup *popup)
{
    if (!popup->priv->resize_idle)
        popup->priv->resize_idle = g_idle_add_full (GDK_PRIORITY_REDRAW + 20,
                                                    (GSourceFunc) resize_in_idle,
                                                    popup, NULL);
}


static void
disconnect_window (MooTextPopup *popup)
{
    g_signal_handlers_disconnect_by_func (popup->priv->window,
                                          (gpointer) window_size_request,
                                          popup);
    if (popup->priv->resize_idle)
        g_source_remove (popup->priv->resize_idle);
    popup->priv->resize_idle = 0;
}


static void
moo_text_popup_resize (MooTextPopup *popup)
{
    GtkWidget *widget = GTK_WIDGET (popup->priv->doc);
    int x, y, width;
    int total_items, items, height;
    GdkScreen *screen;
    int monitor_num;
    GdkRectangle monitor, iter_rect;
    GtkRequisition popup_req;
    int vert_separator;
    GtkTextIter iter;

    g_return_if_fail (GTK_WIDGET_REALIZED (widget));
    g_return_if_fail (popup->priv->pos);

    disconnect_window (popup);

    moo_text_popup_get_position (popup, &iter);
    gtk_text_view_get_iter_location (popup->priv->doc, &iter, &iter_rect);
    gtk_text_view_buffer_to_window_coords (popup->priv->doc, GTK_TEXT_WINDOW_WIDGET,
                                           iter_rect.x, iter_rect.y,
                                           &iter_rect.x, &iter_rect.y);
    gdk_window_get_origin (widget->window, &x, &y);
    x += iter_rect.x;
    y += iter_rect.y;

    total_items = gtk_tree_model_iter_n_children (popup->priv->model, NULL);
    items = MIN (total_items, popup->priv->max_len);

    gtk_tree_view_column_cell_get_size (popup->priv->column, NULL,
                                        NULL, NULL, &width, &height);

    screen = gtk_widget_get_screen (widget);
    monitor_num = gdk_screen_get_monitor_at_window (screen, widget->window);
    gdk_screen_get_monitor_geometry (screen, monitor_num, &monitor);

    gtk_widget_style_get (GTK_WIDGET (popup->priv->treeview),
                          "vertical-separator", &vert_separator,
                          NULL);

    width = MAX (width, 100);
    width = MIN (monitor.width, width);

    gtk_widget_set_size_request (GTK_WIDGET (popup->priv->treeview),
                                 -1, items * (height + vert_separator));

    gtk_widget_size_request (popup->priv->window, &popup_req);

    if (x < monitor.x)
        x = monitor.x;
    else if (x + popup_req.width > monitor.x + monitor.width)
        x = monitor.x + monitor.width - popup_req.width;

    if (y + iter_rect.height + popup_req.height <= monitor.y + monitor.height)
        y += iter_rect.height;
    else
        y -= popup_req.height;

    gtk_window_resize (GTK_WINDOW (popup->priv->window),
                       popup_req.width, popup_req.height);
//     g_print ("resizing to %d, %d\n", popup_req.width, popup_req.height);
    gtk_window_move (GTK_WINDOW (popup->priv->window), x, y);

//     g_signal_connect_swapped (popup->priv->window, "size-request",
//                               G_CALLBACK (window_size_request), popup);
}


void
moo_text_popup_set_doc (MooTextPopup *popup,
                        GtkTextView   *doc)
{
    g_return_if_fail (MOO_IS_TEXT_POPUP (popup));
    g_return_if_fail (!doc || GTK_IS_TEXT_VIEW (doc));

    if (popup->priv->doc == doc)
        return;

    moo_text_popup_hide (popup);

    if (popup->priv->doc)
    {
        if (popup->priv->pos)
        {
            gtk_text_buffer_delete_mark (popup->priv->buffer, popup->priv->pos);
            popup->priv->pos = NULL;
        }

        g_object_unref (popup->priv->buffer);
        g_object_unref (popup->priv->doc);
        popup->priv->doc = NULL;
        popup->priv->buffer = NULL;
    }

    if (doc)
    {
        popup->priv->doc = g_object_ref (doc);
        popup->priv->buffer = g_object_ref (gtk_text_view_get_buffer (doc));
    }

    g_object_notify (G_OBJECT (popup), "doc");
}


GtkTextView *
moo_text_popup_get_doc (MooTextPopup *popup)
{
    g_return_val_if_fail (MOO_IS_TEXT_POPUP (popup), NULL);
    return popup->priv->doc;
}


void
moo_text_popup_set_model (MooTextPopup *popup,
                          GtkTreeModel *model)
{
    g_return_if_fail (MOO_IS_TEXT_POPUP (popup));
    g_return_if_fail (!model || GTK_IS_TREE_MODEL (model));

    if (model == popup->priv->model)
        return;

    if (popup->priv->model)
        g_object_unref (popup->priv->model);

    popup->priv->model = model ? g_object_ref (model) : NULL;

    if (popup->priv->treeview)
    {
        gtk_tree_view_set_model (popup->priv->treeview, model);
        moo_text_popup_update (popup);
    }

    g_object_notify (G_OBJECT (popup), "model");
}


GtkTreeModel *
moo_text_popup_get_model (MooTextPopup *popup)
{
    g_return_val_if_fail (MOO_IS_TEXT_POPUP (popup), NULL);
    return popup->priv->model;
}


gboolean
moo_text_popup_get_position (MooTextPopup *popup,
                             GtkTextIter  *iter)
{
    g_return_val_if_fail (MOO_IS_TEXT_POPUP (popup), FALSE);

    if (popup->priv->pos)
    {
        if (iter)
            gtk_text_buffer_get_iter_at_mark (popup->priv->buffer, iter,
                                              popup->priv->pos);
        return TRUE;
    }

    return FALSE;
}


void
moo_text_popup_set_position (MooTextPopup      *popup,
                             const GtkTextIter *where)
{
    g_return_if_fail (MOO_IS_TEXT_POPUP (popup));
    g_return_if_fail (where);
    g_return_if_fail (popup->priv->buffer != NULL);

    if (!popup->priv->pos)
    {
        popup->priv->pos =
                gtk_text_buffer_create_mark (popup->priv->buffer,
                                             NULL, where, TRUE);
    }
    else
    {
        gtk_text_buffer_move_mark (popup->priv->buffer,
                                   popup->priv->pos, where);
    }

    moo_text_popup_update (popup);
}


/***************************************************************************/
/* Popup events
 */

static gboolean
doc_focus_out (MooTextPopup *popup)
{
    moo_text_popup_hide (popup);
    return FALSE;
}


static gboolean
popup_button_press (MooTextPopup  *popup,
                    GdkEventButton *event)
{
    if (event->window == popup->priv->window->window)
    {
        int width, height;

        gdk_drawable_get_size (GDK_DRAWABLE (event->window),
                               &width, &height);

        if (event->x < 0 || event->x >= width ||
            event->y < 0 || event->y >= height)
        {
            moo_text_popup_hide (popup);
        }

        return TRUE;
    }
    else
    {
        moo_text_popup_hide (popup);
        return FALSE;
    }
}


static void
popup_move_selection (MooTextPopup *popup,
                      GdkEventKey   *event)
{
    int n_items, current_item, new_item = -1;
    GtkTreeIter iter;
    GtkTreeModel *model;

    n_items = gtk_tree_model_iter_n_children (popup->priv->model, NULL);
    g_return_if_fail (n_items != 0);

    if (gtk_tree_selection_get_selected (popup->priv->selection, &model, &iter))
    {
        GtkTreePath *path = gtk_tree_model_get_path (model, &iter);
        current_item = gtk_tree_path_get_indices (path) [0];
        gtk_tree_path_free (path);
    }
    else
    {
        current_item = -1;
    }

    switch (event->keyval)
    {
        case GDK_Page_Down:
        case GDK_KP_Page_Down:
            new_item = current_item + popup->priv->max_len - 1;
            if (new_item >= n_items)
                new_item = n_items - 1;
            break;

        case GDK_Page_Up:
        case GDK_KP_Page_Up:
            new_item = current_item - popup->priv->max_len + 1;
            if (new_item < 0)
                new_item = 0;
            break;

        case GDK_Tab:
        case GDK_KP_Tab:
        case GDK_Down:
        case GDK_KP_Down:
            if (current_item < n_items - 1)
                new_item = current_item + 1;
            else
                new_item = 0;
            break;

        case GDK_ISO_Left_Tab:
        case GDK_Up:
        case GDK_KP_Up:
            if (current_item <= 0)
                new_item = n_items - 1;
            else
                new_item = current_item - 1;
            break;

        default:
            g_return_if_reached ();
    }

    if (new_item >= 0 && new_item < n_items)
    {
        GtkTreePath *path = gtk_tree_path_new_from_indices (new_item, -1);
        gtk_tree_view_set_cursor (popup->priv->treeview, path, NULL, FALSE);
        gtk_tree_view_scroll_to_cell (popup->priv->treeview, path, NULL, FALSE, 0, 0);
        gtk_tree_path_free (path);
    }
    else
    {
        gtk_tree_selection_unselect_all (popup->priv->selection);
    }
}


static gboolean
popup_key_press (MooTextPopup *popup,
                 GdkEventKey   *event)
{
    switch (event->keyval)
    {
        case GDK_Down:
        case GDK_Up:
        case GDK_KP_Down:
        case GDK_KP_Up:
        case GDK_Page_Down:
        case GDK_Page_Up:
        case GDK_Tab:
        case GDK_KP_Tab:
        case GDK_ISO_Left_Tab:
            popup_move_selection (popup, event);
            return TRUE;

        case GDK_Escape:
            moo_text_popup_hide (popup);
            return TRUE;

        case GDK_Return:
        case GDK_ISO_Enter:
        case GDK_KP_Enter:
            moo_text_popup_activate (popup);
            return TRUE;

        default:
            return gtk_widget_event (GTK_WIDGET (popup->priv->doc), (GdkEvent*) event);
    }
}


static gboolean
list_button_press (MooTextPopup  *popup,
                   GdkEventButton *event)
{
    GtkTreePath *path;
    GtkTreeIter iter;

    if (gtk_tree_view_get_path_at_pos (popup->priv->treeview,
                                       event->x, event->y,
                                       &path, NULL, NULL, NULL))
    {
        gtk_tree_model_get_iter (popup->priv->model, &iter, path);
        gtk_tree_selection_select_iter (popup->priv->selection, &iter);
        moo_text_popup_activate (popup);
        gtk_tree_path_free (path);
        return TRUE;
    }

    return FALSE;
}


#define popup_get_cursor(popup, iter)                                                   \
    gtk_text_buffer_get_iter_at_mark (popup->priv->buffer, iter,                        \
                                      gtk_text_buffer_get_insert (popup->priv->buffer))


static void
buffer_delete_range (MooTextPopup *popup,
                     GtkTextIter  *start,
                     GtkTextIter  *end)
{
    GtkTextIter iter;
    int offset;

    if (gtk_text_iter_compare (start, end) > 0)
    {
        GtkTextIter *tmp = start;
        start = end;
        end = tmp;
    }

    popup_get_cursor (popup, &iter);

    if (gtk_text_iter_compare (&iter, end))
        goto hide;

    offset = gtk_text_iter_get_offset (end) - gtk_text_iter_get_offset (start);

    if (ABS (offset) > 1)
        goto hide;

    return;

hide:
    moo_text_popup_hide (popup);
}


static void
buffer_insert_text (MooTextPopup *popup,
                    GtkTextIter  *iter,
                    const char   *text,
                    int           len)
{
    GtkTextIter cursor;
    gunichar ch;

    popup_get_cursor (popup, &cursor);

    if (gtk_text_iter_compare (&cursor, iter))
        goto hide;

    if (len < 0)
        len = g_utf8_strlen (text, -1);

    if (len > 1)
        goto hide;

    ch = g_utf8_get_char (text);

    if (g_unichar_isspace (ch))
    {
        switch (ch)
        {
            case ' ':
            case '\t':
                break;

            default:
                goto hide;
        }
    }

    return;

hide:
    moo_text_popup_hide (popup);
}


static void
moo_text_popup_connect (MooTextPopup *popup)
{
    g_return_if_fail (popup->priv->doc && popup->priv->treeview && popup->priv->window);

    g_signal_connect_swapped (popup->priv->doc, "focus-out-event",
                              G_CALLBACK (doc_focus_out), popup);

    g_signal_connect_swapped (popup->priv->window, "button-press-event",
                              G_CALLBACK (popup_button_press), popup);
    g_signal_connect_swapped (popup->priv->window, "key-press-event",
                              G_CALLBACK (popup_key_press), popup);

    g_signal_connect_swapped (popup->priv->treeview, "button-press-event",
                              G_CALLBACK (list_button_press), popup);

    g_signal_connect_swapped (popup->priv->buffer, "delete-range",
                              G_CALLBACK (buffer_delete_range), popup);
    g_signal_connect_swapped (popup->priv->buffer, "insert-text",
                              G_CALLBACK (buffer_insert_text), popup);
    g_signal_connect_data (popup->priv->buffer, "delete-range",
                           G_CALLBACK (moo_text_popup_emit_changed), popup,
                           NULL, G_CONNECT_AFTER | G_CONNECT_SWAPPED);
    g_signal_connect_data (popup->priv->buffer, "insert-text",
                           G_CALLBACK (moo_text_popup_emit_changed), popup,
                           NULL, G_CONNECT_AFTER | G_CONNECT_SWAPPED);
}


static void
moo_text_popup_disconnect (MooTextPopup *popup)
{
    disconnect_window (popup);

    g_signal_handlers_disconnect_by_func (popup->priv->doc,
                                          (gpointer) doc_focus_out,
                                          popup);

    g_signal_handlers_disconnect_by_func (popup->priv->window,
                                          (gpointer) popup_button_press,
                                          popup);
    g_signal_handlers_disconnect_by_func (popup->priv->window,
                                          (gpointer) popup_key_press,
                                          popup);

    g_signal_handlers_disconnect_by_func (popup->priv->treeview,
                                          (gpointer) list_button_press,
                                          popup);

    g_signal_handlers_disconnect_by_func (popup->priv->buffer,
                                          (gpointer) buffer_delete_range,
                                          popup);
    g_signal_handlers_disconnect_by_func (popup->priv->buffer,
                                          (gpointer) buffer_insert_text,
                                          popup);
    g_signal_handlers_disconnect_by_func (popup->priv->buffer,
                                          (gpointer) moo_text_popup_emit_changed,
                                          popup);
}


static void
moo_text_popup_emit_changed (MooTextPopup *popup)
{
    g_signal_emit (popup, signals[TEXT_CHANGED], 0);
}


MooTextPopup *
moo_text_popup_new (GtkTextView *doc)
{
    g_return_val_if_fail (!doc || GTK_IS_TEXT_VIEW (doc), NULL);
    return g_object_new (MOO_TYPE_TEXT_POPUP, "doc", doc, NULL);
}


static void
moo_text_popup_set_property (GObject        *object,
                             guint           prop_id,
                             const GValue   *value,
                             GParamSpec     *pspec)
{
    MooTextPopup *popup = MOO_TEXT_POPUP (object);

    switch (prop_id)
    {
        case PROP_HIDE_ON_ACTIVATE:
            popup->priv->hide_on_activate = g_value_get_boolean (value) != 0;
            g_object_notify (object, "hide-on-activate");
            break;

        case PROP_MAX_LEN:
            popup->priv->max_len = g_value_get_int (value);
            g_object_notify (object, "max-len");
            break;

        case PROP_POSITION:
            moo_text_popup_set_position (popup, g_value_get_boxed (value));
            break;

        case PROP_DOC:
            moo_text_popup_set_doc (popup, g_value_get_object (value));
            break;

        case PROP_MODEL:
            moo_text_popup_set_model (popup, g_value_get_object (value));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void
moo_text_popup_get_property (GObject        *object,
                             guint           prop_id,
                             GValue         *value,
                             GParamSpec     *pspec)
{
    MooTextPopup *popup = MOO_TEXT_POPUP (object);
    GtkTextIter iter;

    switch (prop_id)
    {
        case PROP_HIDE_ON_ACTIVATE:
            g_value_set_boolean (value, popup->priv->hide_on_activate != 0);
            break;

        case PROP_MAX_LEN:
            g_value_set_int (value, popup->priv->max_len);
            break;

        case PROP_POSITION:
            if (moo_text_popup_get_position (popup, &iter))
                g_value_set_boxed (value, &iter);
            else
                g_value_set_boxed (value, NULL);
            break;

        case PROP_DOC:
            g_value_set_object (value, popup->priv->doc);
            break;

        case PROP_MODEL:
            g_value_set_object (value, popup->priv->model);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


gboolean
moo_text_popup_get_selected (MooTextPopup *popup,
                             GtkTreeIter  *iter)
{
    g_return_val_if_fail (MOO_IS_TEXT_POPUP (popup), FALSE);
    g_return_val_if_fail (popup->priv->model != NULL, FALSE);
    return gtk_tree_selection_get_selected (popup->priv->selection, NULL, iter);
}


void
moo_text_popup_select (MooTextPopup *popup,
                       GtkTreeIter  *iter)
{
    g_return_if_fail (MOO_IS_TEXT_POPUP (popup));
    g_return_if_fail (popup->priv->model != NULL);

    gtk_tree_selection_select_iter (popup->priv->selection, iter);

    if (popup->priv->visible)
    {
        GtkTreePath *path;
        path = gtk_tree_model_get_path (popup->priv->model, iter);
        gtk_tree_view_scroll_to_cell (popup->priv->treeview, path,
                                      NULL, FALSE, 0, 0);
        gtk_tree_path_free (path);
    }
}


static void
moo_text_popup_select_first (MooTextPopup *popup)
{
    GtkTreeIter iter;
    gtk_tree_model_get_iter_first (popup->priv->model, &iter);
    moo_text_popup_select (popup, &iter);
}
