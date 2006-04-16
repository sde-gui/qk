/*
 *   moocompletion.c
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

#include "mooedit/moocompletion.h"
#include "mooutils/moomarshals.h"
#include <gdk/gdkkeysyms.h>


#define MAX_POPUP_LEN 10


struct _MooCompletionPrivate {
    GtkTreeView *treeview;
    GtkTreeSelection *selection;
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell;
    GtkTreeModel *model;
    int text_column;

    GtkTextView *doc;
    GtkTextBuffer *buffer;
    GtkTextMark *start;
    GtkTextMark *end;

    GtkWidget *popup;
    GtkScrolledWindow *scrolled_window;
    guint working : 1;
    guint visible : 1;
    guint in_update : 1;
};


static void     moo_completion_dispose         (GObject         *object);

static gboolean moo_completion_complete_real   (MooCompletion   *cmpl,
                                                GtkTreeModel    *model,
                                                GtkTreeIter     *iter);

static void     moo_completion_ensure_popup    (MooCompletion   *cmpl);
static gboolean moo_completion_empty           (MooCompletion   *cmpl);
static void     moo_completion_popup           (MooCompletion   *cmpl);
static void     moo_completion_popdown         (MooCompletion   *cmpl);
static void     moo_completion_resize_popup    (MooCompletion   *cmpl);
static void     moo_completion_connect_popup   (MooCompletion   *cmpl);
static void     moo_completion_disconnect_popup(MooCompletion   *cmpl);


G_DEFINE_TYPE (MooCompletion, moo_completion, G_TYPE_OBJECT)

enum {
    UPDATE,
    COMPLETE,
    NUM_SIGNALS
};

static guint signals[NUM_SIGNALS];


static void
moo_completion_dispose (GObject *object)
{
    MooCompletion *cmpl = MOO_COMPLETION (object);

    if (cmpl->priv)
    {
        moo_completion_hide (cmpl);
        moo_completion_set_doc (cmpl, NULL);
        moo_completion_set_model (cmpl, NULL, 0);

        if (cmpl->priv->popup)
            gtk_widget_destroy (cmpl->priv->popup);

        g_free (cmpl->priv);
        cmpl->priv = NULL;
    }

    G_OBJECT_CLASS(moo_completion_parent_class)->dispose (object);
}


static void
moo_completion_class_init (MooCompletionClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->dispose = moo_completion_dispose;
    klass->complete = moo_completion_complete_real;

    signals[UPDATE] =
            g_signal_new ("update",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooCompletionClass, update),
                          NULL, NULL,
                          _moo_marshal_VOID__BOXED_BOXED,
                          G_TYPE_NONE, 2,
                          GTK_TYPE_TEXT_ITER,
                          GTK_TYPE_TEXT_ITER);

    signals[COMPLETE] =
            g_signal_new ("complete",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooCompletionClass, complete),
                          g_signal_accumulator_true_handled, NULL,
                          _moo_marshal_BOOLEAN__OBJECT_BOXED,
                          G_TYPE_BOOLEAN, 2,
                          GTK_TYPE_TREE_MODEL,
                          GTK_TYPE_TREE_ITER);
}


static void
moo_completion_init (MooCompletion *cmpl)
{
    cmpl->priv = g_new0 (MooCompletionPrivate, 1);
    cmpl->priv->model = GTK_TREE_MODEL (gtk_list_store_new (1, G_TYPE_STRING));
}


static void
moo_completion_ensure_popup (MooCompletion *cmpl)
{
    if (!cmpl->priv->popup)
    {
        GtkWidget *scrolled_window, *frame;

        cmpl->priv->popup = gtk_window_new (GTK_WINDOW_POPUP);
        gtk_widget_set_size_request (cmpl->priv->popup, -1, -1);
        gtk_window_set_default_size (GTK_WINDOW (cmpl->priv->popup), 1, 1);
        gtk_window_set_resizable (GTK_WINDOW (cmpl->priv->popup), FALSE);
        gtk_widget_add_events (cmpl->priv->popup, GDK_KEY_PRESS_MASK | GDK_BUTTON_PRESS_MASK);

        scrolled_window = gtk_scrolled_window_new (NULL, NULL);
        cmpl->priv->scrolled_window = GTK_SCROLLED_WINDOW (scrolled_window);
        gtk_widget_set_size_request (scrolled_window, -1, -1);
        gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                        GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
        gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_window),
                                             GTK_SHADOW_ETCHED_IN);
        /* a nasty hack to get the completions treeview to size nicely */
        gtk_widget_set_size_request (GTK_SCROLLED_WINDOW (scrolled_window)->vscrollbar, -1, 0);
        gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_window), GTK_SHADOW_NONE);

        frame = gtk_frame_new (NULL);
        gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_OUT);
        gtk_container_add (GTK_CONTAINER (frame), scrolled_window);
        gtk_container_add (GTK_CONTAINER (cmpl->priv->popup), frame);

        cmpl->priv->treeview = GTK_TREE_VIEW (gtk_tree_view_new ());
        gtk_widget_set_size_request (GTK_WIDGET (cmpl->priv->treeview), -1, -1);
        gtk_container_add (GTK_CONTAINER (scrolled_window),
                           GTK_WIDGET (cmpl->priv->treeview));
        gtk_tree_view_set_headers_visible (cmpl->priv->treeview, FALSE);
#if GTK_CHECK_VERSION(2,6,0)
        gtk_tree_view_set_hover_selection (cmpl->priv->treeview, TRUE);
#endif

        gtk_widget_show_all (frame);

        cmpl->priv->selection = gtk_tree_view_get_selection (cmpl->priv->treeview);
        gtk_tree_selection_set_mode (cmpl->priv->selection, GTK_SELECTION_SINGLE);

        cmpl->priv->column = gtk_tree_view_column_new ();
        cmpl->priv->cell = gtk_cell_renderer_text_new ();

        gtk_tree_view_column_pack_start (cmpl->priv->column,
                                         cmpl->priv->cell,
                                         TRUE);
        gtk_tree_view_column_set_attributes (cmpl->priv->column,
                                             cmpl->priv->cell,
                                             "text", cmpl->priv->text_column,
                                             NULL);

        gtk_tree_view_append_column (cmpl->priv->treeview,
                                     cmpl->priv->column);

        if (cmpl->priv->model)
            gtk_tree_view_set_model (cmpl->priv->treeview, cmpl->priv->model);
    }
}


gboolean
moo_completion_show (MooCompletion      *cmpl,
                     const GtkTextIter  *start,
                     const GtkTextIter  *end)
{
    g_return_val_if_fail (MOO_IS_COMPLETION (cmpl), FALSE);

    if (cmpl->priv->working)
    {
        moo_completion_set_region (cmpl, start, end);
        return cmpl->priv->working;
    }

    g_return_val_if_fail (cmpl->priv->doc != NULL, FALSE);
    g_return_val_if_fail (cmpl->priv->model != NULL, FALSE);

    moo_completion_set_region (cmpl, start, end);

    if (!moo_completion_empty (cmpl))
    {
        cmpl->priv->working = TRUE;
        moo_completion_popup (cmpl);
        return TRUE;
    }

    return FALSE;
}


void
moo_completion_update (MooCompletion *cmpl)
{
    GtkTextIter start, end;

    g_return_if_fail (MOO_IS_COMPLETION (cmpl));
    g_return_if_fail (cmpl->priv->working);

    if (cmpl->priv->in_update)
        return;

    cmpl->priv->in_update = TRUE;

    gtk_text_buffer_get_iter_at_mark (cmpl->priv->buffer,
                                      &start, cmpl->priv->start);
    gtk_text_buffer_get_iter_at_mark (cmpl->priv->buffer,
                                      &end, cmpl->priv->end);

    g_signal_emit (cmpl, signals[UPDATE], 0, cmpl->priv->model);

    if (moo_completion_empty (cmpl))
        moo_completion_hide (cmpl);
    else if (cmpl->priv->visible)
        moo_completion_resize_popup (cmpl);

    cmpl->priv->in_update = FALSE;
}


void
moo_completion_hide (MooCompletion *cmpl)
{
    g_return_if_fail (MOO_IS_COMPLETION (cmpl));

    if (cmpl->priv->working)
    {
        moo_completion_popdown (cmpl);
        cmpl->priv->working = FALSE;
    }
}


void
moo_completion_complete (MooCompletion *cmpl)
{
    GtkTreeIter iter;
    gboolean got_selected, retval;

    g_return_if_fail (MOO_IS_COMPLETION (cmpl));
    g_return_if_fail (cmpl->priv->visible);

    got_selected = gtk_tree_selection_get_selected (cmpl->priv->selection, NULL, &iter);

    moo_completion_popdown (cmpl);

    if (got_selected)
        g_signal_emit (cmpl, signals[COMPLETE], 0, cmpl->priv->model, &iter, &retval);

    moo_completion_hide (cmpl);
}


static gboolean
moo_completion_empty (MooCompletion *cmpl)
{
    return !cmpl->priv->model ||
            gtk_tree_model_iter_n_children (cmpl->priv->model, NULL) == 0;
}


static void
moo_completion_popup (MooCompletion *cmpl)
{
    GtkWidget *toplevel;

    if (cmpl->priv->visible)
        return;

    g_return_if_fail (cmpl->priv->doc != NULL);

    moo_completion_ensure_popup (cmpl);

    toplevel = gtk_widget_get_toplevel (GTK_WIDGET (cmpl->priv->doc));
    g_return_if_fail (GTK_IS_WINDOW (toplevel));

    gtk_widget_realize (cmpl->priv->popup);

    if (GTK_WINDOW (toplevel)->group)
        gtk_window_group_add_window (GTK_WINDOW (toplevel)->group,
                                     GTK_WINDOW (cmpl->priv->popup));
    gtk_window_set_modal (GTK_WINDOW (cmpl->priv->popup), TRUE);

    moo_completion_resize_popup (cmpl);

    gtk_widget_show (cmpl->priv->popup);
    cmpl->priv->visible = TRUE;

    gtk_widget_ensure_style (GTK_WIDGET (cmpl->priv->treeview));
    gtk_widget_modify_bg (GTK_WIDGET (cmpl->priv->treeview), GTK_STATE_ACTIVE,
                          &GTK_WIDGET(cmpl->priv->treeview)->style->base[GTK_STATE_SELECTED]);
    gtk_widget_modify_base (GTK_WIDGET (cmpl->priv->treeview), GTK_STATE_ACTIVE,
                            &GTK_WIDGET(cmpl->priv->treeview)->style->base[GTK_STATE_SELECTED]);

    gtk_grab_add (cmpl->priv->popup);
    gdk_pointer_grab (cmpl->priv->popup->window, TRUE,
                      GDK_BUTTON_PRESS_MASK |
                              GDK_BUTTON_RELEASE_MASK |
                              GDK_POINTER_MOTION_MASK,
                      NULL, NULL, GDK_CURRENT_TIME);

    moo_completion_connect_popup (cmpl);
}


static void
moo_completion_popdown (MooCompletion *cmpl)
{
    if (cmpl->priv->visible)
    {
        cmpl->priv->visible = FALSE;

        moo_completion_disconnect_popup (cmpl);

        gdk_pointer_ungrab (GDK_CURRENT_TIME);
        gtk_grab_remove (cmpl->priv->popup);
        gtk_widget_hide (cmpl->priv->popup);
    }
}


static void
moo_completion_resize_popup (MooCompletion *cmpl)
{
    GtkWidget *widget = GTK_WIDGET (cmpl->priv->doc);
    int x, y, width;
    int total_items, items, height;
    GdkScreen *screen;
    int monitor_num;
    GdkRectangle monitor, iter_rect;
    GtkRequisition popup_req;
    int vert_separator, horiz_separator;
    GtkTextIter iter;

    g_return_if_fail (GTK_WIDGET_REALIZED (widget));
    g_return_if_fail (cmpl->priv->start && cmpl->priv->end);

    moo_completion_get_region (cmpl, &iter, NULL);
    gtk_text_view_get_iter_location (cmpl->priv->doc, &iter, &iter_rect);
    gtk_text_view_buffer_to_window_coords (cmpl->priv->doc, GTK_TEXT_WINDOW_TEXT,
                                           iter_rect.x, iter_rect.y,
                                           &iter_rect.x, &iter_rect.y);
    gdk_window_get_origin (widget->window, &x, &y);
    x += iter_rect.x;
    y += iter_rect.y;

    total_items = gtk_tree_model_iter_n_children (cmpl->priv->model, NULL);
    items = MIN (total_items, MAX_POPUP_LEN);

    gtk_tree_view_column_cell_get_size (cmpl->priv->column, NULL,
                                        NULL, NULL, NULL, &height);

    screen = gtk_widget_get_screen (widget);
    monitor_num = gdk_screen_get_monitor_at_window (screen, widget->window);
    gdk_screen_get_monitor_geometry (screen, monitor_num, &monitor);

    gtk_widget_style_get (GTK_WIDGET (cmpl->priv->treeview),
                          "vertical-separator", &vert_separator,
                          "horizontal-separator", &horiz_separator,
                          NULL);

    gtk_widget_size_request (GTK_WIDGET (cmpl->priv->treeview), &popup_req);
    width = popup_req.width;

    if (total_items > items)
    {
        GtkRequisition scrollbar_req;
        gtk_widget_size_request (cmpl->priv->scrolled_window->vscrollbar,
                                 &scrollbar_req);
        width += scrollbar_req.width;
    }

    width = MAX (width, 100);
    width = MIN (monitor.width, width);

    gtk_widget_set_size_request (GTK_WIDGET (cmpl->priv->treeview),
                                 width, items * (height + vert_separator));

    gtk_widget_set_size_request (cmpl->priv->popup, -1, -1);
    gtk_widget_size_request (cmpl->priv->popup, &popup_req);

    if (x < monitor.x)
        x = monitor.x;
    else if (x + popup_req.width > monitor.x + monitor.width)
        x = monitor.x + monitor.width - popup_req.width;

    if (y + iter_rect.height + popup_req.height <= monitor.y + monitor.height)
    {
        y += iter_rect.height;
    }
    else
    {
        y -= popup_req.height;
    }

    gtk_window_move (GTK_WINDOW (cmpl->priv->popup), x, y);
}


void
moo_completion_set_doc (MooCompletion *cmpl,
                        GtkTextView   *doc)
{
    g_return_if_fail (MOO_IS_COMPLETION (cmpl));
    g_return_if_fail (!doc || GTK_IS_TEXT_VIEW (doc));

    if (cmpl->priv->doc == doc)
        return;

    moo_completion_hide (cmpl);

    if (cmpl->priv->doc)
    {
        if (cmpl->priv->start)
        {
            gtk_text_buffer_delete_mark (cmpl->priv->buffer, cmpl->priv->start);
            gtk_text_buffer_delete_mark (cmpl->priv->buffer, cmpl->priv->end);
            cmpl->priv->start = NULL;
            cmpl->priv->end = NULL;
        }

        g_object_unref (cmpl->priv->doc);
        cmpl->priv->doc = NULL;
        cmpl->priv->buffer = NULL;
    }

    if (doc)
    {
        cmpl->priv->doc = g_object_ref (doc);
        cmpl->priv->buffer = gtk_text_view_get_buffer (doc);
    }
}


GtkTextView *
moo_completion_get_doc (MooCompletion *cmpl)
{
    g_return_val_if_fail (MOO_IS_COMPLETION (cmpl), NULL);
    return cmpl->priv->doc;
}


void
moo_completion_set_model (MooCompletion  *cmpl,
                          GtkTreeModel   *model,
                          int             text_column)
{
    g_return_if_fail (MOO_IS_COMPLETION (cmpl));
    g_return_if_fail (!model || GTK_IS_TREE_MODEL (model));

    if (model && gtk_tree_model_get_column_type (model, text_column) != G_TYPE_STRING)
    {
        g_critical ("%s: invalid text column", G_STRLOC);
        return;
    }

    moo_completion_hide (cmpl);

    cmpl->priv->text_column = text_column;

    if (model != cmpl->priv->model)
    {
        if (cmpl->priv->model)
            g_object_unref (cmpl->priv->model);
        cmpl->priv->model = model;
        if (cmpl->priv->model)
            g_object_ref (cmpl->priv->model);
    }

    if (cmpl->priv->treeview)
    {
        gtk_tree_view_set_model (cmpl->priv->treeview, model);

        if (model)
            gtk_tree_view_column_set_attributes (cmpl->priv->column,
                                                 cmpl->priv->cell,
                                                 "text", cmpl->priv->text_column,
                                                 NULL);
    }
}


GtkTreeModel *
moo_completion_get_model (MooCompletion  *cmpl,
                          int            *text_column)
{
    g_return_val_if_fail (MOO_IS_COMPLETION (cmpl), NULL);

    if (text_column && cmpl->priv->model)
        *text_column = cmpl->priv->text_column;

    return cmpl->priv->model;
}


gboolean
moo_completion_get_region (MooCompletion  *cmpl,
                           GtkTextIter    *start,
                           GtkTextIter    *end)
{
    g_return_val_if_fail (MOO_IS_COMPLETION (cmpl), FALSE);

    if (cmpl->priv->start)
    {
        if (start)
            gtk_text_buffer_get_iter_at_mark (cmpl->priv->buffer, start,
                                              cmpl->priv->start);
        if (end)
            gtk_text_buffer_get_iter_at_mark (cmpl->priv->buffer, end,
                                              cmpl->priv->end);
        return TRUE;
    }

    return FALSE;
}


void
moo_completion_set_region (MooCompletion      *cmpl,
                           const GtkTextIter  *start,
                           const GtkTextIter  *end)
{
    g_return_if_fail (MOO_IS_COMPLETION (cmpl));
    g_return_if_fail (start && end);
    g_return_if_fail (cmpl->priv->buffer != NULL);

    if (gtk_text_iter_compare (start, end) > 0)
    {
        const GtkTextIter *tmp = start;
        start = end;
        end = tmp;
    }

    if (!cmpl->priv->start)
    {
        cmpl->priv->start =
                gtk_text_buffer_create_mark (cmpl->priv->buffer,
                                             NULL, start, TRUE);
        cmpl->priv->end =
                gtk_text_buffer_create_mark (cmpl->priv->buffer,
                                             NULL, end, FALSE);
    }
    else
    {
        gtk_text_buffer_move_mark (cmpl->priv->buffer,
                                   cmpl->priv->start, start);
        gtk_text_buffer_move_mark (cmpl->priv->buffer,
                                   cmpl->priv->end, end);
    }

    if (cmpl->priv->working)
        moo_completion_update (cmpl);
}


/***************************************************************************/
/* Popup events
 */

static gboolean
doc_focus_out (MooCompletion *cmpl)
{
    moo_completion_hide (cmpl);
    return FALSE;
}


static gboolean
popup_button_press (MooCompletion  *cmpl,
                    GdkEventButton *event)
{
    if (event->window == cmpl->priv->popup->window)
    {
        int width, height;

        gdk_drawable_get_size (GDK_DRAWABLE (event->window),
                               &width, &height);

        if (event->x < 0 || event->x >= width ||
            event->y < 0 || event->y >= height)
        {
            moo_completion_hide (cmpl);
        }

        return TRUE;
    }
    else
    {
        moo_completion_hide (cmpl);
        return FALSE;
    }
}


static void
popup_move_selection (MooCompletion *cmpl,
                      GdkEventKey   *event)
{
    int n_items, current_item, new_item = -1;
    GtkTreeIter iter;
    GtkTreeModel *model;

    n_items = gtk_tree_model_iter_n_children (cmpl->priv->model, NULL);
    g_return_if_fail (n_items != 0);

    if (gtk_tree_selection_get_selected (cmpl->priv->selection, &model, &iter))
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
        case GDK_Down:
        case GDK_KP_Down:
            if (current_item < n_items - 1)
                new_item = current_item + 1;
            else
                new_item = -1;
            break;

        case GDK_Up:
        case GDK_KP_Up:
            if (current_item < 0)
                new_item = n_items - 1;
            else
                new_item = current_item - 1;
            break;

        case GDK_Page_Down:
        case GDK_KP_Page_Down:
            new_item = current_item + MAX_POPUP_LEN - 1;
            if (new_item >= n_items)
                new_item = n_items - 1;
            break;

        case GDK_Page_Up:
        case GDK_KP_Page_Up:
            new_item = current_item - MAX_POPUP_LEN + 1;
            if (new_item < 0)
                new_item = 0;
            break;

        case GDK_Tab:
        case GDK_KP_Tab:
            if (current_item < n_items - 1)
                new_item = current_item + 1;
            else
                new_item = 0;
            break;

        case GDK_ISO_Left_Tab:
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
        gtk_tree_view_set_cursor (cmpl->priv->treeview, path, NULL, FALSE);
        gtk_tree_view_scroll_to_cell (cmpl->priv->treeview, path, NULL, FALSE, 0, 0);
        gtk_tree_path_free (path);
    }
    else
    {
        gtk_tree_selection_unselect_all (cmpl->priv->selection);
    }
}


static gboolean
popup_key_press (MooCompletion *cmpl,
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
            popup_move_selection (cmpl, event);
            return TRUE;

        case GDK_Escape:
            moo_completion_hide (cmpl);
            return TRUE;

        case GDK_Return:
        case GDK_ISO_Enter:
        case GDK_KP_Enter:
            moo_completion_complete (cmpl);
            return TRUE;

        default:
            return gtk_widget_event (GTK_WIDGET (cmpl->priv->doc), (GdkEvent*) event);
    }
}


static gboolean
list_button_press (MooCompletion  *cmpl,
                   GdkEventButton *event)
{
    GtkTreePath *path;
    GtkTreeIter iter;

    if (gtk_tree_view_get_path_at_pos (cmpl->priv->treeview,
                                       event->x, event->y,
                                       &path, NULL, NULL, NULL))
    {
        gtk_tree_model_get_iter (cmpl->priv->model, &iter, path);
        gtk_tree_selection_select_iter (cmpl->priv->selection, &iter);
        moo_completion_complete (cmpl);
        gtk_tree_path_free (path);
        return TRUE;
    }

    return FALSE;
}


static void
buffer_delete_range (MooCompletion *cmpl,
                     GtkTextIter   *start,
                     GtkTextIter   *end)
{
    GtkTextIter region_end;
    int offset;

    if (gtk_text_iter_compare (start, end) > 0)
    {
        GtkTextIter *tmp = start;
        start = end;
        end = tmp;
    }

    moo_completion_get_region (cmpl, NULL, &region_end);

    if (gtk_text_iter_compare (&region_end, end))
        goto hide;

    offset = gtk_text_iter_get_offset (end) - gtk_text_iter_get_offset (&region_end);

    if (ABS (offset) > 1)
        goto hide;

    return;

hide:
    moo_completion_hide (cmpl);
}


static void
buffer_insert_text (MooCompletion *cmpl,
                    GtkTextIter   *iter,
                    const char    *text,
                    int            len)
{
    GtkTextIter region_end;
    gunichar ch;

    moo_completion_get_region (cmpl, NULL, &region_end);

    if (gtk_text_iter_compare (&region_end, iter))
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
    moo_completion_hide (cmpl);
}


static void
moo_completion_connect_popup (MooCompletion *cmpl)
{
    g_return_if_fail (cmpl->priv->doc && cmpl->priv->treeview && cmpl->priv->popup);

    g_signal_connect_swapped (cmpl->priv->doc, "focus-out-event",
                              G_CALLBACK (doc_focus_out), cmpl);

    g_signal_connect_swapped (cmpl->priv->popup, "button-press-event",
                              G_CALLBACK (popup_button_press), cmpl);
    g_signal_connect_swapped (cmpl->priv->popup, "key-press-event",
                              G_CALLBACK (popup_key_press), cmpl);

    g_signal_connect_swapped (cmpl->priv->treeview, "button-press-event",
                              G_CALLBACK (list_button_press), cmpl);

    g_signal_connect_swapped (cmpl->priv->buffer, "delete-range",
                              G_CALLBACK (buffer_delete_range), cmpl);
    g_signal_connect_swapped (cmpl->priv->buffer, "insert-text",
                              G_CALLBACK (buffer_insert_text), cmpl);
    g_signal_connect_data (cmpl->priv->buffer, "delete-range",
                           G_CALLBACK (moo_completion_update), cmpl,
                           NULL, G_CONNECT_AFTER | G_CONNECT_SWAPPED);
    g_signal_connect_data (cmpl->priv->buffer, "insert-text",
                           G_CALLBACK (moo_completion_update), cmpl,
                           NULL, G_CONNECT_AFTER | G_CONNECT_SWAPPED);
}


static void
moo_completion_disconnect_popup (MooCompletion *cmpl)
{
    g_signal_handlers_disconnect_by_func (cmpl->priv->doc,
                                          (gpointer) doc_focus_out,
                                          cmpl);

    g_signal_handlers_disconnect_by_func (cmpl->priv->popup,
                                          (gpointer) popup_button_press,
                                          cmpl);
    g_signal_handlers_disconnect_by_func (cmpl->priv->popup,
                                          (gpointer) popup_key_press,
                                          cmpl);

    g_signal_handlers_disconnect_by_func (cmpl->priv->treeview,
                                          (gpointer) list_button_press,
                                          cmpl);

    g_signal_handlers_disconnect_by_func (cmpl->priv->buffer,
                                          (gpointer) buffer_delete_range,
                                          cmpl);
    g_signal_handlers_disconnect_by_func (cmpl->priv->buffer,
                                          (gpointer) buffer_insert_text,
                                          cmpl);
    g_signal_handlers_disconnect_by_func (cmpl->priv->buffer,
                                          (gpointer) moo_completion_update,
                                          cmpl);
}


static gboolean
moo_completion_complete_real (MooCompletion  *cmpl,
                              GtkTreeModel   *model,
                              GtkTreeIter    *iter)
{
    char *text = NULL;
    GtkTextIter start, end;

    gtk_tree_model_get (model, iter, cmpl->priv->text_column, &text, -1);
    g_return_val_if_fail (text != NULL, FALSE);

    moo_completion_get_region (cmpl, &start, &end);
    gtk_text_buffer_delete (cmpl->priv->buffer, &start, &end);
    gtk_text_buffer_insert (cmpl->priv->buffer, &start, text, -1);

    g_free (text);
    return TRUE;
}


MooCompletion *
moo_completion_new (void)
{
    return g_object_new (MOO_TYPE_COMPLETION, NULL);
}
