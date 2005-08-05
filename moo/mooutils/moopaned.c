/*
 *   mooutils/moopaned.c
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

#include "mooutils/moopaned.h"
#include "mooutils/moostock.h"


static int MIN_PANE_SIZE = 10;
static int SPACING_IN_BUTTON = 4;


typedef struct {
    GtkWidget   *frame;
    GtkWidget   *child;
    GtkWidget   *button;
    GtkWidget   *sticky_button;
    GtkWidget   *close_button;
} Pane;


struct _MooPanedPrivate {
    GtkPositionType pane_position;

    GdkWindow   *handle_window;
    GdkWindow   *pane_window;

    Pane        *current_pane;
    GSList      *panes;

    gboolean     close_on_child_focus;

    int          position;

    gboolean     button_box_visible;
    int          button_box_size;
    gboolean     handle_visible;
    int          handle_size;
    gboolean     pane_widget_visible;
    int          pane_widget_size;
    gboolean     sticky;

    gboolean     handle_prelit;
    gboolean     in_drag;
    int          drag_start;
};


static void     moo_paned_finalize      (GObject        *object);
static void     moo_paned_set_property  (GObject        *object,
                                         guint           prop_id,
                                         const GValue   *value,
                                         GParamSpec     *pspec);
static void     moo_paned_get_property  (GObject        *object,
                                         guint           prop_id,
                                         GValue         *value,
                                         GParamSpec     *pspec);
static GObject *moo_paned_constructor   (GType                  type,
                                         guint                  n_construct_properties,
                                         GObjectConstructParam *construct_properties);

static void     moo_paned_realize       (GtkWidget      *widget);
static void     moo_paned_unrealize     (GtkWidget      *widget);
static void     moo_paned_map           (GtkWidget      *widget);

static void     moo_paned_set_focus_child (GtkContainer *container,
                                         GtkWidget      *widget);

static void     moo_paned_size_request  (GtkWidget      *widget,
                                         GtkRequisition *requisition);
static void     moo_paned_size_allocate (GtkWidget      *widget,
                                         GtkAllocation  *allocation);

static gboolean moo_paned_expose        (GtkWidget      *widget,
                                         GdkEventExpose *event);
static gboolean moo_paned_motion        (GtkWidget      *widget,
                                         GdkEventMotion *event);
static gboolean moo_paned_enter         (GtkWidget      *widget,
                                         GdkEventCrossing *event);
static gboolean moo_paned_leave         (GtkWidget      *widget,
                                         GdkEventCrossing *event);
static gboolean moo_paned_button_press  (GtkWidget      *widget,
                                         GdkEventButton *event);
static gboolean moo_paned_button_release(GtkWidget      *widget,
                                         GdkEventButton *event);

static void     moo_paned_forall        (GtkContainer   *container,
                                         gboolean        include_internals,
                                         GtkCallback     callback,
                                         gpointer        callback_data);
static void     moo_paned_remove        (GtkContainer   *container,
                                         GtkWidget      *widget);
static void     moo_paned_remove_pane   (MooPaned       *paned,
                                         GtkWidget      *pane_widget);

static void     realize_handle          (MooPaned       *paned);
static void     realize_pane            (MooPaned       *paned);
static void     draw_handle             (MooPaned       *paned);
static void     button_box_visible_notify (MooPaned     *paned);

static void     button_toggled          (GtkToggleButton *button,
                                         MooPaned       *paned);
static void     sticky_button_toggled   (GtkToggleButton *button,
                                         MooPaned       *paned);


/* MOO_TYPE_PANED */
G_DEFINE_TYPE (MooPaned, moo_paned, GTK_TYPE_BIN)

enum {
    PROP_0,
    PROP_PANE_POSITION,
    PROP_CLOSE_PANE_ON_CHILD_FOCUS,
    PROP_STICKY_PANE
};


static void moo_paned_class_init (MooPanedClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
    GtkContainerClass *container_class = GTK_CONTAINER_CLASS (klass);

    moo_create_stock_items ();

    gobject_class->finalize = moo_paned_finalize;
    gobject_class->set_property = moo_paned_set_property;
    gobject_class->get_property = moo_paned_get_property;
    gobject_class->constructor = moo_paned_constructor;

    widget_class->realize = moo_paned_realize;
    widget_class->unrealize = moo_paned_unrealize;
    widget_class->map = moo_paned_map;
    widget_class->expose_event = moo_paned_expose;
    widget_class->size_request = moo_paned_size_request;
    widget_class->size_allocate = moo_paned_size_allocate;
    widget_class->motion_notify_event = moo_paned_motion;
    widget_class->enter_notify_event = moo_paned_enter;
    widget_class->leave_notify_event = moo_paned_leave;
    widget_class->button_press_event = moo_paned_button_press;
    widget_class->button_release_event = moo_paned_button_release;

    container_class->forall = moo_paned_forall;
    container_class->set_focus_child = moo_paned_set_focus_child;
    container_class->remove = moo_paned_remove;

    g_object_class_install_property (gobject_class,
                                     PROP_PANE_POSITION,
                                     g_param_spec_enum ("pane-position",
                                             "pane-position",
                                             "pane-position",
                                             GTK_TYPE_POSITION_TYPE,
                                             GTK_POS_LEFT,
                                             G_PARAM_READWRITE |
                                                     G_PARAM_CONSTRUCT_ONLY));

    g_object_class_install_property (gobject_class,
                                     PROP_CLOSE_PANE_ON_CHILD_FOCUS,
                                     g_param_spec_boolean ("close-pane-on-child-focus",
                                             "close-pane-on-child-focus",
                                             "close-pane-on-child-focus",
                                             TRUE,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_STICKY_PANE,
                                     g_param_spec_boolean ("sticky-pane",
                                             "sticky-pane",
                                             "sticky-pane",
                                             FALSE,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    gtk_widget_class_install_style_property (widget_class,
                                             g_param_spec_int ("handle-size",
                                                     "handle-size",
                                                     "handle-size",
                                                     0,
                                                     G_MAXINT,
                                                     5,
                                                     G_PARAM_READABLE));

    gtk_widget_class_install_style_property (widget_class,
                                             g_param_spec_int ("button-spacing",
                                                     "button-spacing",
                                                     "button-spacing",
                                                     0,
                                                     G_MAXINT,
                                                     0,
                                                     G_PARAM_READABLE));
}


static void moo_paned_init      (MooPaned *paned)
{
    paned->priv = g_new0 (MooPanedPrivate, 1);

    paned->button_box = NULL;

    paned->priv->pane_position = -1;
    paned->priv->handle_window = NULL;
    paned->priv->pane_window = NULL;
    paned->priv->current_pane = NULL;
    paned->priv->panes = NULL;
    paned->priv->button_box_visible = FALSE;
    paned->priv->button_box_size = 0;
    paned->priv->handle_visible = FALSE;
    paned->priv->handle_size = 0;
    paned->priv->pane_widget_visible = FALSE;
    paned->priv->pane_widget_size = 0;
    paned->priv->sticky = FALSE;
    paned->priv->position = -1;
    paned->priv->handle_prelit = FALSE;
    paned->priv->in_drag = FALSE;
    paned->priv->drag_start = -1;
}


static GObject *moo_paned_constructor   (GType                  type,
                                         guint                  n_construct_properties,
                                         GObjectConstructParam *construct_properties)
{
    GObject *object;
    MooPaned *paned;
    int button_spacing;

    object = G_OBJECT_CLASS(moo_paned_parent_class)->constructor (type,
                                n_construct_properties, construct_properties);
    paned = MOO_PANED (object);

    gtk_widget_style_get (GTK_WIDGET (paned),
                          "button-spacing", &button_spacing, NULL);

    switch (paned->priv->pane_position)
    {
        case GTK_POS_LEFT:
        case GTK_POS_RIGHT:
            paned->button_box = gtk_vbox_new (FALSE, button_spacing);
            break;
        case GTK_POS_TOP:
        case GTK_POS_BOTTOM:
            paned->button_box = gtk_hbox_new (FALSE, button_spacing);
            break;
        default:
            g_warning ("%s: invalid 'pane-position' property value '%d',"
                       "falling back to GTK_POS_LEFT", G_STRLOC,
                       paned->priv->pane_position);
            paned->priv->pane_position = GTK_POS_LEFT;
            paned->button_box = gtk_vbox_new (FALSE, button_spacing);
            break;
    }

    gtk_object_sink (gtk_object_ref (GTK_OBJECT (paned->button_box)));
    gtk_widget_set_parent (paned->button_box, GTK_WIDGET (paned));
    gtk_widget_show (paned->button_box);
    g_signal_connect_swapped (paned->button_box, "notify::visible",
                              G_CALLBACK (button_box_visible_notify),
                              paned);

    return object;
}


static void     moo_paned_set_property  (GObject        *object,
                                         guint           prop_id,
                                         const GValue   *value,
                                         GParamSpec     *pspec)
{
    MooPaned *paned = MOO_PANED (object);

    switch (prop_id)
    {
        case PROP_PANE_POSITION:
            paned->priv->pane_position = g_value_get_enum (value);
            break;

        case PROP_CLOSE_PANE_ON_CHILD_FOCUS:
            paned->priv->close_on_child_focus =
                    g_value_get_boolean (value);
            g_object_notify (object, "close-pane-on-child-focus");
            break;

        case PROP_STICKY_PANE:
            moo_paned_set_sticky_pane (paned,
                                       g_value_get_boolean (value));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void     moo_paned_get_property  (GObject        *object,
                                         guint           prop_id,
                                         GValue         *value,
                                         GParamSpec     *pspec)
{
    MooPaned *paned = MOO_PANED (object);

    switch (prop_id)
    {
        case PROP_PANE_POSITION:
            g_value_set_enum (value, paned->priv->pane_position);
            break;

        case PROP_CLOSE_PANE_ON_CHILD_FOCUS:
            g_value_set_boolean (value, paned->priv->close_on_child_focus);
            break;

        case PROP_STICKY_PANE:
            g_value_set_boolean (value, paned->priv->sticky);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void moo_paned_finalize  (GObject      *object)
{
    MooPaned *paned = MOO_PANED (object);
    g_free (paned->priv);
    G_OBJECT_CLASS (moo_paned_parent_class)->finalize (object);
}


GtkWidget   *moo_paned_new              (GtkPositionType pane_position)
{
    return GTK_WIDGET (g_object_new (MOO_TYPE_PANED,
                       "pane-position", pane_position,
                       NULL));
}


static void moo_paned_realize      (GtkWidget       *widget)
{
    static GdkWindowAttr attributes;
    gint attributes_mask;
    MooPaned *paned;

    paned = MOO_PANED (widget);

    GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);

    attributes.x = widget->allocation.x;
    attributes.y = widget->allocation.y;
    attributes.width = widget->allocation.width;
    attributes.height = widget->allocation.height;
    attributes.window_type = GDK_WINDOW_CHILD;
    attributes.event_mask = gtk_widget_get_events (widget)
            | GDK_EXPOSURE_MASK;

    attributes.visual = gtk_widget_get_visual (widget);
    attributes.colormap = gtk_widget_get_colormap (widget);
    attributes.wclass = GDK_INPUT_OUTPUT;

    attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;

    widget->window = gdk_window_new (gtk_widget_get_parent_window (widget),
                                     &attributes, attributes_mask);
    gdk_window_set_user_data (widget->window, widget);

    widget->style = gtk_style_attach (widget->style, widget->window);
    gtk_style_set_background (widget->style, widget->window, GTK_STATE_NORMAL);

    realize_pane (paned);
}


static void realize_handle (MooPaned *paned)
{
    static GdkWindowAttr attributes;
    gint attributes_mask;
    GtkWidget *widget = GTK_WIDGET (paned);

    switch (paned->priv->pane_position)
    {
        case GTK_POS_LEFT:
        case GTK_POS_RIGHT:
            attributes.y = 0;
            attributes.width = paned->priv->handle_size;
            attributes.height = widget->allocation.height;
            break;
        case GTK_POS_TOP:
        case GTK_POS_BOTTOM:
            attributes.x = 0;
            attributes.width = widget->allocation.width;
            attributes.height = paned->priv->handle_size;
            break;
    }

    switch (paned->priv->pane_position)
    {
        case GTK_POS_LEFT:
            attributes.x = paned->priv->pane_widget_size;
            break;
        case GTK_POS_RIGHT:
            attributes.x = 0;
            break;
        case GTK_POS_TOP:
            attributes.y = paned->priv->pane_widget_size;
            break;
        case GTK_POS_BOTTOM:
            attributes.y = 0;
            break;
    }

    attributes.window_type = GDK_WINDOW_CHILD;
    attributes.event_mask = gtk_widget_get_events (widget)
            | GDK_POINTER_MOTION_HINT_MASK
            | GDK_POINTER_MOTION_MASK
            | GDK_BUTTON_PRESS_MASK
            | GDK_BUTTON_RELEASE_MASK
            | GDK_EXPOSURE_MASK
            | GDK_ENTER_NOTIFY_MASK
            | GDK_LEAVE_NOTIFY_MASK;

    attributes.visual = gtk_widget_get_visual (widget);
    attributes.colormap = gtk_widget_get_colormap (widget);
    attributes.wclass = GDK_INPUT_OUTPUT;

    switch (paned->priv->pane_position)
    {
        case GTK_POS_LEFT:
        case GTK_POS_RIGHT:
            attributes.cursor = gdk_cursor_new (GDK_SB_H_DOUBLE_ARROW);
            break;
        case GTK_POS_TOP:
        case GTK_POS_BOTTOM:
            attributes.cursor = gdk_cursor_new (GDK_SB_V_DOUBLE_ARROW);
            break;
    }

    attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL |
            GDK_WA_COLORMAP | GDK_WA_CURSOR;

    paned->priv->handle_window = gdk_window_new (paned->priv->pane_window,
            &attributes, attributes_mask);
    gdk_window_set_user_data (paned->priv->handle_window, widget);

    gtk_style_set_background (widget->style,
                              paned->priv->handle_window,
                              GTK_STATE_NORMAL);

    gdk_cursor_unref (attributes.cursor);
}


static void realize_pane (MooPaned *paned)
{
    static GdkWindowAttr attributes;
    gint attributes_mask;
    GtkWidget *widget = GTK_WIDGET (paned);

    switch (paned->priv->pane_position)
    {
        case GTK_POS_LEFT:
        case GTK_POS_RIGHT:
            attributes.y = 0;
            attributes.height = widget->allocation.height;
            break;
        case GTK_POS_TOP:
        case GTK_POS_BOTTOM:
            attributes.x = 0;
            attributes.width = widget->allocation.width;
            break;
    }

    switch (paned->priv->pane_position)
    {
        case GTK_POS_LEFT:
            attributes.x = paned->priv->button_box_size;
            attributes.width = paned->priv->pane_widget_size +
                    paned->priv->handle_size;
            break;
        case GTK_POS_RIGHT:
            attributes.width = paned->priv->pane_widget_size +
                    paned->priv->handle_size;
            attributes.x = widget->allocation.width -
                               paned->priv->button_box_size -
                               attributes.width;
            break;
        case GTK_POS_TOP:
            attributes.y = paned->priv->button_box_size;
            attributes.height = paned->priv->pane_widget_size +
                    paned->priv->handle_size;
            break;
        case GTK_POS_BOTTOM:
            attributes.height = paned->priv->pane_widget_size +
                    paned->priv->handle_size;
            attributes.y = widget->allocation.height -
                               paned->priv->button_box_size -
                               attributes.height;
            break;
    }

    attributes.window_type = GDK_WINDOW_CHILD;
    attributes.event_mask = gtk_widget_get_events (widget)
            | GDK_EXPOSURE_MASK;

    attributes.visual = gtk_widget_get_visual (widget);
    attributes.colormap = gtk_widget_get_colormap (widget);
    attributes.wclass = GDK_INPUT_OUTPUT;
    attributes.cursor = gdk_cursor_new (GDK_SB_H_DOUBLE_ARROW);

    attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL |
            GDK_WA_COLORMAP;

    paned->priv->pane_window =
            gdk_window_new (widget->window, &attributes, attributes_mask);
    gdk_window_set_user_data (paned->priv->pane_window, widget);

    gtk_style_set_background (widget->style,
                              paned->priv->pane_window,
                              GTK_STATE_NORMAL);

    realize_handle (paned);
}


static void     moo_paned_unrealize     (GtkWidget      *widget)
{
    MooPaned *paned = MOO_PANED (widget);

    if (GTK_WIDGET_CLASS (moo_paned_parent_class)->unrealize)
        GTK_WIDGET_CLASS (moo_paned_parent_class)->unrealize (widget);

    if (paned->priv->handle_window)
    {
        gdk_window_set_user_data (paned->priv->handle_window, NULL);
        gdk_window_destroy (paned->priv->handle_window);
        paned->priv->handle_window = NULL;
        paned->priv->handle_visible = FALSE;
        paned->priv->handle_size = 0;
    }

    if (paned->priv->pane_window)
    {
        gdk_window_set_user_data (paned->priv->pane_window, NULL);
        gdk_window_destroy (paned->priv->pane_window);
        paned->priv->pane_window = NULL;
        paned->priv->pane_widget_visible = FALSE;
        paned->priv->pane_widget_size = 0;
    }
}


static void add_button_box_requisition (MooPaned *paned,
                                        GtkRequisition *requisition,
                                        GtkRequisition *child_requisition)
{
    switch (paned->priv->pane_position)
    {
        case GTK_POS_LEFT:
        case GTK_POS_RIGHT:
            requisition->width += child_requisition->width;
            requisition->height = MAX (child_requisition->height, requisition->height);
            paned->priv->button_box_size = child_requisition->width;
            break;
        case GTK_POS_TOP:
        case GTK_POS_BOTTOM:
            requisition->height += child_requisition->height;
            requisition->width = MAX (child_requisition->width, requisition->width);
            paned->priv->button_box_size = child_requisition->height;
            break;
    }
}


static void add_handle_requisition (MooPaned       *paned,
                                    GtkRequisition *requisition)
{
    gtk_widget_style_get (GTK_WIDGET (paned),
                          "handle_size", &paned->priv->handle_size,
                          NULL);

    switch (paned->priv->pane_position)
    {
        case GTK_POS_LEFT:
        case GTK_POS_RIGHT:
            requisition->width += paned->priv->handle_size;
            break;
        case GTK_POS_TOP:
        case GTK_POS_BOTTOM:
            requisition->height += paned->priv->handle_size;
            break;
    }
}


static void add_pane_widget_requisition (MooPaned       *paned,
                                         GtkRequisition *requisition,
                                         GtkRequisition *child_requisition)
{
    switch (paned->priv->pane_position)
    {
        case GTK_POS_LEFT:
        case GTK_POS_RIGHT:
            requisition->height = MAX (child_requisition->height, requisition->height);

            if (paned->priv->sticky)
            {
                requisition->width += child_requisition->width;
            }
            else
            {
                requisition->width = MAX (child_requisition->width +
                        paned->priv->button_box_size +
                        paned->priv->handle_size, requisition->width);
            }

            break;

        case GTK_POS_TOP:
        case GTK_POS_BOTTOM:
            requisition->width = MAX (child_requisition->width, requisition->width);

            if (paned->priv->sticky)
            {
                requisition->height += child_requisition->height;
            }
            else
            {
                requisition->height = MAX (child_requisition->height +
                        paned->priv->button_box_size +
                        paned->priv->handle_size, requisition->height);
            }

            break;
    }

    switch (paned->priv->pane_position)
    {
        case GTK_POS_LEFT:
        case GTK_POS_RIGHT:
            paned->priv->pane_widget_size = MAX (paned->priv->position,
                    child_requisition->width);
            paned->priv->position = paned->priv->pane_widget_size;
            break;
        case GTK_POS_TOP:
        case GTK_POS_BOTTOM:
            paned->priv->pane_widget_size = MAX (paned->priv->position,
                    child_requisition->height);
            paned->priv->position = paned->priv->pane_widget_size;
            break;
    }
}


static void moo_paned_size_request (GtkWidget      *widget,
                                    GtkRequisition *requisition)
{
    GtkBin *bin = GTK_BIN (widget);
    MooPaned *paned = MOO_PANED (widget);
    GtkRequisition child_requisition;

    requisition->width = 0;
    requisition->height = 0;

    if (bin->child && GTK_WIDGET_VISIBLE (bin->child))
    {
        gtk_widget_size_request (bin->child, &child_requisition);
        requisition->width += child_requisition.width;
        requisition->height += child_requisition.height;
    }

    if (paned->priv->button_box_visible)
    {
        gtk_widget_size_request (paned->button_box, &child_requisition);
        add_button_box_requisition (paned, requisition, &child_requisition);
    }
    else
    {
        paned->priv->button_box_size = 0;
    }

    if (paned->priv->handle_visible)
        add_handle_requisition (paned, requisition);
    else
        paned->priv->handle_size = 0;

    if (paned->priv->pane_widget_visible)
    {
        gtk_widget_size_request (paned->priv->current_pane->frame, &child_requisition);
        add_pane_widget_requisition (paned, requisition, &child_requisition);
    }
    else
    {
        paned->priv->pane_widget_size = 0;
    }
}


static void get_pane_widget_allocation (MooPaned        *paned,
                                        GtkAllocation   *allocation)
{
    switch (paned->priv->pane_position)
    {
        case GTK_POS_LEFT:
            allocation->x = 0;
            allocation->y = 0;
            allocation->width = paned->priv->pane_widget_size;
            allocation->height = GTK_WIDGET(paned)->allocation.height;
            break;
        case GTK_POS_RIGHT:
            allocation->x = paned->priv->handle_size;
            allocation->y = 0;
            allocation->width = paned->priv->pane_widget_size;
            allocation->height = GTK_WIDGET(paned)->allocation.height;
            break;
        case GTK_POS_TOP:
            allocation->x = 0;
            allocation->y = 0;
            allocation->width = GTK_WIDGET(paned)->allocation.width;
            allocation->height = paned->priv->pane_widget_size;
            break;
        case GTK_POS_BOTTOM:
            allocation->x = 0;
            allocation->y = paned->priv->handle_size;
            allocation->width = GTK_WIDGET(paned)->allocation.width;
            allocation->height = paned->priv->pane_widget_size;
            break;
    }
}


static void get_button_box_allocation (MooPaned        *paned,
                                       GtkAllocation   *allocation)
{
    switch (paned->priv->pane_position)
    {
        case GTK_POS_LEFT:
        case GTK_POS_RIGHT:
            allocation->y = 0;
            allocation->height = GTK_WIDGET(paned)->allocation.height;
            allocation->width = paned->priv->button_box_size;
            break;
        case GTK_POS_TOP:
        case GTK_POS_BOTTOM:
            allocation->x = 0;
            allocation->width = GTK_WIDGET(paned)->allocation.width;
            allocation->height = paned->priv->button_box_size;
            break;
    }

    switch (paned->priv->pane_position)
    {
        case GTK_POS_LEFT:
            allocation->x = 0;
            break;
        case GTK_POS_RIGHT:
            allocation->x = GTK_WIDGET(paned)->allocation.width -
                                allocation->width;
            break;
        case GTK_POS_TOP:
            allocation->y = 0;
            break;
        case GTK_POS_BOTTOM:
            allocation->y = GTK_WIDGET(paned)->allocation.height -
                                allocation->height;
            break;
    }
}


static void get_bin_child_allocation (MooPaned        *paned,
                                      GtkAllocation   *allocation)
{
    switch (paned->priv->pane_position)
    {
        case GTK_POS_LEFT:
        case GTK_POS_RIGHT:
            allocation->y = 0;
            allocation->height = GTK_WIDGET(paned)->allocation.height;
            allocation->width = GTK_WIDGET(paned)->allocation.width -
                                    paned->priv->button_box_size -
                                    paned->priv->handle_size;
            break;
        case GTK_POS_TOP:
        case GTK_POS_BOTTOM:
            allocation->x = 0;
            allocation->width = GTK_WIDGET(paned)->allocation.width;
            allocation->height = GTK_WIDGET(paned)->allocation.height -
                                    paned->priv->button_box_size -
                                    paned->priv->handle_size;
            break;
    }

    switch (paned->priv->pane_position)
    {
        case GTK_POS_LEFT:
            allocation->x = paned->priv->button_box_size +
                    paned->priv->handle_size;
            break;
        case GTK_POS_RIGHT:
            allocation->x = 0;
            break;
        case GTK_POS_TOP:
            allocation->y = paned->priv->button_box_size +
                    paned->priv->handle_size;
            break;
        case GTK_POS_BOTTOM:
            allocation->y = 0;
            break;
    }

    if (paned->priv->sticky)
    {
        switch (paned->priv->pane_position)
        {
            case GTK_POS_LEFT:
                allocation->x += paned->priv->pane_widget_size;
                allocation->width -= paned->priv->pane_widget_size;
                break;
            case GTK_POS_RIGHT:
                allocation->width -= paned->priv->pane_widget_size;
                break;
            case GTK_POS_TOP:
                allocation->y += paned->priv->pane_widget_size;
                allocation->height -= paned->priv->pane_widget_size;
                break;
            case GTK_POS_BOTTOM:
                allocation->height -= paned->priv->pane_widget_size;
                break;
        }
    }
}


static void clamp_handle_size (MooPaned *paned)
{
    switch (paned->priv->pane_position)
    {
        case GTK_POS_LEFT:
        case GTK_POS_RIGHT:
            paned->priv->handle_size = CLAMP (paned->priv->handle_size, 0,
                                              GTK_WIDGET(paned)->allocation.width);
            break;
        case GTK_POS_TOP:
        case GTK_POS_BOTTOM:
            paned->priv->handle_size = CLAMP (paned->priv->handle_size, 0,
                                              GTK_WIDGET(paned)->allocation.height);
            break;
    }
}


static void clamp_button_box_size (MooPaned *paned)
{
    switch (paned->priv->pane_position)
    {
        case GTK_POS_LEFT:
        case GTK_POS_RIGHT:
            paned->priv->button_box_size = CLAMP (paned->priv->button_box_size, 0,
                    GTK_WIDGET(paned)->allocation.width -
                            paned->priv->handle_size);
            break;
        case GTK_POS_TOP:
        case GTK_POS_BOTTOM:
            paned->priv->button_box_size = CLAMP (paned->priv->button_box_size, 0,
                    GTK_WIDGET(paned)->allocation.height -
                            paned->priv->handle_size);
            break;
    }
}


static void clamp_child_requisition (MooPaned *paned,
                                     GtkRequisition *requisition)
{
    switch (paned->priv->pane_position)
    {
        case GTK_POS_LEFT:
        case GTK_POS_RIGHT:
            requisition->width = CLAMP (requisition->width, 0,
                                        GTK_WIDGET(paned)->allocation.width -
                                                paned->priv->handle_size -
                                                paned->priv->button_box_size);
            break;
        case GTK_POS_TOP:
        case GTK_POS_BOTTOM:
            requisition->height = CLAMP (requisition->height, 0,
                                        GTK_WIDGET(paned)->allocation.height -
                                                paned->priv->handle_size -
                                                paned->priv->button_box_size);
            break;
    }
}


static void clamp_pane_widget_size (MooPaned       *paned,
                                    GtkRequisition *child_requisition)
{
    int min_size, max_size;

    switch (paned->priv->pane_position)
    {
        case GTK_POS_LEFT:
        case GTK_POS_RIGHT:
            max_size = GTK_WIDGET(paned)->allocation.width -
                                      paned->priv->handle_size -
                                      paned->priv->button_box_size;
            if (paned->priv->sticky)
                max_size -= child_requisition->width;
            break;
        case GTK_POS_TOP:
        case GTK_POS_BOTTOM:
            max_size = GTK_WIDGET(paned)->allocation.height -
                                      paned->priv->handle_size -
                                      paned->priv->button_box_size;
            if (paned->priv->sticky)
                max_size -= child_requisition->height;
            break;
    }

    min_size = CLAMP (MIN_PANE_SIZE, 0, max_size);

    paned->priv->pane_widget_size =
        CLAMP (paned->priv->pane_widget_size, min_size, max_size);
}


static void get_pane_window_rect (MooPaned      *paned,
                                  GdkRectangle  *rect)
{
    switch (paned->priv->pane_position)
    {
        case GTK_POS_LEFT:
        case GTK_POS_RIGHT:
            rect->width = paned->priv->pane_widget_size + paned->priv->handle_size;
            rect->height = GTK_WIDGET(paned)->allocation.height;
            rect->y = 0;
            break;
        case GTK_POS_TOP:
        case GTK_POS_BOTTOM:
            rect->height = paned->priv->pane_widget_size + paned->priv->handle_size;
            rect->width = GTK_WIDGET(paned)->allocation.width;
            rect->x = 0;
            break;
    }

    switch (paned->priv->pane_position)
    {
        case GTK_POS_LEFT:
            rect->x = paned->priv->button_box_size;
            break;
        case GTK_POS_RIGHT:
            rect->x = GTK_WIDGET(paned)->allocation.width -
                          rect->width -
                          paned->priv->button_box_size;
            break;
        case GTK_POS_TOP:
            rect->y = paned->priv->button_box_size;
            break;
        case GTK_POS_BOTTOM:
            rect->y = GTK_WIDGET(paned)->allocation.height -
                          rect->height -
                          paned->priv->button_box_size;
            break;
    }
}


static void get_handle_window_rect (MooPaned      *paned,
                                    GdkRectangle  *rect)
{
    switch (paned->priv->pane_position)
    {
        case GTK_POS_LEFT:
        case GTK_POS_RIGHT:
            rect->y = 0;
            rect->width = paned->priv->handle_size;
            rect->height = GTK_WIDGET(paned)->allocation.height;
            break;
        case GTK_POS_TOP:
        case GTK_POS_BOTTOM:
            rect->x = 0;
            rect->height = paned->priv->handle_size;
            rect->width = GTK_WIDGET(paned)->allocation.width;
            break;
    }

    switch (paned->priv->pane_position)
    {
        case GTK_POS_LEFT:
            rect->x = paned->priv->pane_widget_size;
            break;
        case GTK_POS_RIGHT:
            rect->x = 0;
            break;
        case GTK_POS_TOP:
            rect->y = paned->priv->pane_widget_size;
            break;
        case GTK_POS_BOTTOM:
            rect->y = 0;
            break;
    }
}


static void moo_paned_size_allocate (GtkWidget     *widget,
                                     GtkAllocation *allocation)
{
    GtkBin *bin;
    MooPaned *paned;
    GtkAllocation child_allocation;
    GtkRequisition child_requisition = {0, 0};

    widget->allocation = *allocation;
    bin = GTK_BIN (widget);
    paned = MOO_PANED (widget);

    if (bin->child && GTK_WIDGET_VISIBLE (bin->child))
        gtk_widget_get_child_requisition (bin->child, &child_requisition);

    if (paned->priv->handle_visible)
        clamp_handle_size (paned);

    if (paned->priv->button_box_visible)
        clamp_button_box_size (paned);

    clamp_child_requisition (paned, &child_requisition);

    if (paned->priv->pane_widget_visible)
    {
        clamp_pane_widget_size (paned, &child_requisition);
        paned->priv->position = paned->priv->pane_widget_size;
    }

    if (GTK_WIDGET_REALIZED (widget))
    {
        GdkRectangle rect;

        gdk_window_move_resize (widget->window,
                                allocation->x,
                                allocation->y,
                                allocation->width,
                                allocation->height);

        if (paned->priv->pane_window)
        {
            get_pane_window_rect (paned, &rect);
            gdk_window_move_resize (paned->priv->pane_window,
                                    rect.x, rect.y,
                                    rect.width, rect.height);
        }

        if (paned->priv->handle_visible)
        {
            get_handle_window_rect (paned, &rect);
            gdk_window_move_resize (paned->priv->handle_window,
                                    rect.x, rect.y,
                                    rect.width, rect.height);
        }
    }

    if (paned->priv->button_box_visible)
    {
        get_button_box_allocation (paned, &child_allocation);
        gtk_widget_size_allocate (paned->button_box, &child_allocation);
    }

    if (bin->child)
    {
        get_bin_child_allocation (paned, &child_allocation);
        gtk_widget_size_allocate (bin->child, &child_allocation);
    }

    if (paned->priv->pane_widget_visible)
    {
        get_pane_widget_allocation (paned, &child_allocation);
        gtk_widget_size_allocate (paned->priv->current_pane->frame,
                                  &child_allocation);
    }
}


static void moo_paned_map (GtkWidget *widget)
{
    MooPaned *paned = MOO_PANED (widget);

    gdk_window_show (widget->window);

    (* GTK_WIDGET_CLASS (moo_paned_parent_class)->map) (widget);

    if (paned->priv->handle_visible)
    {
        gdk_window_show (paned->priv->pane_window);
        gdk_window_show (paned->priv->handle_window);
        gdk_window_raise (paned->priv->pane_window);
    }
}


static void moo_paned_forall        (GtkContainer   *container,
                                     gboolean        include_internals,
                                     GtkCallback     callback,
                                     gpointer        callback_data)
{
    MooPaned *paned = MOO_PANED (container);
    GtkBin *bin = GTK_BIN (container);
    GSList *l;

    if (bin->child)
        callback (bin->child, callback_data);

    if (include_internals)
    {
        callback (paned->button_box, callback_data);

        for (l = paned->priv->panes; l != NULL; l = l->next)
            callback (((Pane*)l->data)->frame, callback_data);
    }
}


static gboolean moo_paned_expose    (GtkWidget      *widget,
                                     GdkEventExpose *event)
{
    MooPaned *paned = MOO_PANED (widget);

    if (paned->priv->button_box_visible)
        gtk_container_propagate_expose (GTK_CONTAINER (paned),
                                        paned->button_box, event);

    if (GTK_WIDGET_DRAWABLE (GTK_BIN(paned)->child))
        gtk_container_propagate_expose (GTK_CONTAINER (paned),
                                        GTK_BIN(paned)->child, event);

    if (paned->priv->pane_widget_visible)
        gtk_container_propagate_expose (GTK_CONTAINER (paned),
                                        paned->priv->current_pane->frame,
                                        event);

    if (paned->priv->handle_visible)
        draw_handle (paned);

    return FALSE;
}


static void draw_handle             (MooPaned       *paned)
{
    GtkWidget *widget = GTK_WIDGET (paned);
    GtkStateType state;

    GdkRectangle area;

    area.x = 0;
    area.y = 0;

    switch (paned->priv->pane_position)
    {
        case GTK_POS_LEFT:
        case GTK_POS_RIGHT:
            area.width = paned->priv->handle_size;
            area.height = widget->allocation.height;
            break;
        case GTK_POS_TOP:
        case GTK_POS_BOTTOM:
            area.width = widget->allocation.width;
            area.height = paned->priv->handle_size;
            break;
    }

    if (gtk_widget_is_focus (widget))
        state = GTK_STATE_SELECTED;
    else if (paned->priv->handle_prelit)
        state = GTK_STATE_PRELIGHT;
    else
        state = GTK_WIDGET_STATE (widget);

    gtk_paint_handle (widget->style,
                      paned->priv->handle_window,
                      state,
                      GTK_SHADOW_NONE,
                      &area,
                      widget,
                      "paned",
                      area.x, area.y, area.width, area.height,
                      GTK_ORIENTATION_VERTICAL);
}


static void button_toggled          (GtkToggleButton *button,
                                     MooPaned       *paned)
{
    Pane *pane;

    if (!gtk_toggle_button_get_active (button))
    {
        if (paned->priv->current_pane &&
            paned->priv->current_pane->button == GTK_WIDGET (button))
        {
            gtk_widget_hide (paned->priv->current_pane->frame);
            paned->priv->current_pane = NULL;
            paned->priv->pane_widget_visible = FALSE;
            paned->priv->pane_widget_size = 0;
            gtk_widget_queue_resize (GTK_WIDGET (paned));
        }

        return;
    }

    if (paned->priv->current_pane)
    {
        Pane *old_pane = paned->priv->current_pane;
        paned->priv->current_pane = NULL;
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (old_pane->button), FALSE);
        gtk_widget_hide (old_pane->frame);
    }

    pane = g_object_get_data (G_OBJECT (button), "moo-pane");
    g_return_if_fail (pane != NULL);

    if (GTK_WIDGET_REALIZED (paned))
    {
        gtk_widget_set_parent_window (pane->frame,
                                      paned->priv->pane_window);
        gtk_widget_queue_resize (GTK_WIDGET (paned));
    }

    paned->priv->current_pane = pane;
    gtk_widget_show (pane->frame);

    paned->priv->handle_visible = TRUE;
    gtk_widget_style_get (GTK_WIDGET (paned),
                          "handle_size", &paned->priv->handle_size,
                          NULL);
    paned->priv->pane_widget_visible = TRUE;
    if (paned->priv->position > 0)
        paned->priv->pane_widget_size = paned->priv->position;
}


void         moo_paned_set_sticky_pane  (MooPaned   *paned,
                                         gboolean    sticky)
{
    GSList *l;

    if (sticky)
        sticky = TRUE;

    g_return_if_fail (MOO_IS_PANED (paned));
    if (paned->priv->sticky != sticky && GTK_WIDGET_REALIZED (paned))
        gtk_widget_queue_resize (GTK_WIDGET (paned));
    paned->priv->sticky = sticky;

    for (l = paned->priv->panes; l != NULL; l = l->next)
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (((Pane*)l->data)->sticky_button),
                                      sticky);

    g_object_notify (G_OBJECT (paned), "sticky-pane");
}


void        moo_paned_hide_pane         (MooPaned   *paned)
{
    GtkToggleButton *btn;
    g_return_if_fail (MOO_IS_PANED (paned));
    if (paned->priv->current_pane)
    {
        btn = GTK_TOGGLE_BUTTON (paned->priv->current_pane->button);
        gtk_toggle_button_set_active (btn, FALSE);
    }
}


void        moo_paned_open_pane         (MooPaned   *paned,
                                         guint       index)
{
    Pane *pane;
    g_return_if_fail (MOO_IS_PANED (paned));
    pane = g_slist_nth_data (paned->priv->panes, index);
    g_return_if_fail (pane != NULL);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pane->button),
                                  TRUE);
}


GtkWidget  *moo_paned_get_nth_pane      (MooPaned   *paned,
                                         guint       n)
{
    Pane *pane;
    g_return_val_if_fail (MOO_IS_PANED (paned), NULL);
    pane = g_slist_nth_data (paned->priv->panes, n);
    g_return_val_if_fail (pane != NULL, NULL);
    return pane->child;
}


static gboolean moo_paned_motion    (GtkWidget      *widget,
                                     G_GNUC_UNUSED GdkEventMotion *event)
{
    MooPaned *paned = MOO_PANED (widget);

    if (paned->priv->in_drag)
    {
        int size;
        GtkRequisition requisition;

        gtk_widget_get_child_requisition (paned->priv->current_pane->frame,
                                          &requisition);

        switch (paned->priv->pane_position)
        {
            case GTK_POS_LEFT:
                gtk_widget_get_pointer (widget, &size, NULL);
                size -= (paned->priv->drag_start + paned->priv->button_box_size);
                size = CLAMP (size, requisition.width,
                              widget->allocation.width - paned->priv->button_box_size -
                                      paned->priv->handle_size);
                break;
            case GTK_POS_RIGHT:
                gtk_widget_get_pointer (widget, &size, NULL);
                size = widget->allocation.width - size;
                size -= (paned->priv->drag_start + paned->priv->button_box_size);
                size = CLAMP (size, requisition.width,
                              widget->allocation.width - paned->priv->button_box_size -
                                      paned->priv->handle_size);
                break;
            case GTK_POS_TOP:
                gtk_widget_get_pointer (widget, NULL, &size);
                size -= (paned->priv->drag_start + paned->priv->button_box_size);
                size = CLAMP (size, requisition.height,
                              widget->allocation.height - paned->priv->button_box_size -
                                      paned->priv->handle_size);
                break;
            case GTK_POS_BOTTOM:
                gtk_widget_get_pointer (widget, NULL, &size);
                size = widget->allocation.height - size;
                size -= (paned->priv->drag_start + paned->priv->button_box_size);
                size = CLAMP (size, requisition.height,
                              widget->allocation.height - paned->priv->button_box_size -
                                      paned->priv->handle_size);
                break;
        }

        if (size != paned->priv->pane_widget_size)
            moo_paned_set_pane_size (paned, size);
    }

    return FALSE;
}


static void get_handle_rect (MooPaned     *paned,
                             GdkRectangle *rect)
{
    rect->x = rect->y = 0;

    switch (paned->priv->pane_position)
    {
        case GTK_POS_LEFT:
        case GTK_POS_RIGHT:
            rect->width = paned->priv->handle_size;
            rect->height = GTK_WIDGET(paned)->allocation.height;
            break;
        case GTK_POS_TOP:
        case GTK_POS_BOTTOM:
            rect->height = paned->priv->handle_size;
            rect->width = GTK_WIDGET(paned)->allocation.width;
            break;
    }
}


static gboolean moo_paned_enter     (GtkWidget      *widget,
                                     GdkEventCrossing *event)
{
    MooPaned *paned = MOO_PANED (widget);
    GdkRectangle rect;

    if (event->window == paned->priv->handle_window &&
        !paned->priv->in_drag)
    {
        paned->priv->handle_prelit = TRUE;
        get_handle_rect (paned, &rect);
        gdk_window_invalidate_rect (paned->priv->handle_window,
                                         &rect, FALSE);
        return TRUE;
    }

    return FALSE;
}


static gboolean moo_paned_leave     (GtkWidget      *widget,
                                     GdkEventCrossing *event)
{
    MooPaned *paned = MOO_PANED (widget);
    GdkRectangle rect;

    if (event->window == paned->priv->handle_window &&
        !paned->priv->in_drag)
    {
        paned->priv->handle_prelit = FALSE;
        get_handle_rect (paned, &rect);
        gdk_window_invalidate_rect (paned->priv->handle_window,
                                         &rect, FALSE);
        return TRUE;
    }

    return FALSE;
}


static gboolean moo_paned_button_press  (GtkWidget      *widget,
                                         GdkEventButton *event)
{
    MooPaned *paned = MOO_PANED (widget);

    if (!paned->priv->in_drag &&
         (event->window == paned->priv->handle_window) &&
         (event->button == 1) &&
         paned->priv->pane_widget_visible)
    {
        paned->priv->in_drag = TRUE;

        /* This is copied from gtkpaned.c */
        gdk_pointer_grab (paned->priv->handle_window, FALSE,
                          GDK_POINTER_MOTION_HINT_MASK
                                  | GDK_BUTTON1_MOTION_MASK
                                  | GDK_BUTTON_RELEASE_MASK
                                  | GDK_ENTER_NOTIFY_MASK
                                  | GDK_LEAVE_NOTIFY_MASK,
                          NULL, NULL,
                          event->time);

        switch (paned->priv->pane_position)
        {
            case GTK_POS_LEFT:
            case GTK_POS_RIGHT:
                paned->priv->drag_start = event->x;
                break;
            case GTK_POS_TOP:
            case GTK_POS_BOTTOM:
                paned->priv->drag_start = event->y;
                break;
        }

        return TRUE;
    }

    return FALSE;
}


static gboolean moo_paned_button_release(GtkWidget      *widget,
                                         GdkEventButton *event)
{
    MooPaned *paned = MOO_PANED (widget);

    if (paned->priv->in_drag && (event->button == 1))
    {
        paned->priv->in_drag = FALSE;
        paned->priv->drag_start = -1;
        gdk_display_pointer_ungrab (gtk_widget_get_display (widget),
                                    event->time);
        return TRUE;
    }

    return FALSE;
}


void         moo_paned_set_pane_size    (MooPaned   *paned,
                                         int         size)
{
    g_return_if_fail (MOO_IS_PANED (paned));

    if (GTK_WIDGET_REALIZED (paned))
    {
        GtkWidget *widget = GTK_WIDGET (paned);

        switch (paned->priv->pane_position)
        {
            case GTK_POS_LEFT:
            case GTK_POS_RIGHT:
                size = CLAMP (size, 0,
                              widget->allocation.width - paned->priv->button_box_size -
                                      paned->priv->handle_size);
                break;
            case GTK_POS_TOP:
            case GTK_POS_BOTTOM:
                size = CLAMP (size, 0,
                              widget->allocation.height - paned->priv->button_box_size -
                                      paned->priv->handle_size);
                break;
        }

        if (size != paned->priv->position)
        {
            paned->priv->position = size;
            if (paned->priv->pane_widget_visible)
                gtk_widget_queue_resize (widget);
        }
    }
    else
    {
        paned->priv->position = size;
    }
}


static void     button_box_visible_notify (MooPaned     *paned)
{
    gboolean visible = GTK_WIDGET_VISIBLE (paned->button_box);

    if (paned->priv->button_box_visible == visible)
        return;

    if (paned->priv->panes)
        paned->priv->button_box_visible = visible;

    if (GTK_WIDGET_REALIZED (paned))
        gtk_widget_queue_resize (GTK_WIDGET (paned));
}


void         moo_paned_add_pane     (MooPaned   *paned,
                                     GtkWidget  *pane_widget,
                                     const char *button_label,
                                     const char *button_stock_id)
{
    GtkWidget *box;
    GtkWidget *label = NULL;
    GtkWidget *icon = NULL;

    g_return_if_fail (MOO_IS_PANED (paned));
    g_return_if_fail (GTK_IS_WIDGET (pane_widget));
    g_return_if_fail (pane_widget->parent == NULL);

    switch (paned->priv->pane_position)
    {
        case GTK_POS_LEFT:
        case GTK_POS_RIGHT:
            box = gtk_vbox_new (FALSE, SPACING_IN_BUTTON);
            break;
        default:
            box = gtk_hbox_new (FALSE, SPACING_IN_BUTTON);
            break;
    }

    gtk_widget_show (box);

    if (button_label)
    {
        label = gtk_label_new (button_label);
        gtk_widget_show (label);

        switch (paned->priv->pane_position)
        {
            case GTK_POS_LEFT:
                gtk_label_set_angle (GTK_LABEL (label), 90);
                break;
            case GTK_POS_RIGHT:
                gtk_label_set_angle (GTK_LABEL (label), 270);
                break;
            default:
                break;
        }
    }

    if (button_stock_id)
    {
        icon = gtk_image_new_from_stock (button_stock_id,
                                         GTK_ICON_SIZE_MENU);
        gtk_widget_show (icon);
    }

    switch (paned->priv->pane_position)
    {
        case GTK_POS_LEFT:
            if (label)
                gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);
            if (icon)
                gtk_box_pack_start (GTK_BOX (box), icon, FALSE, FALSE, 0);
            break;
        default:
            if (icon)
                gtk_box_pack_start (GTK_BOX (box), icon, FALSE, FALSE, 0);
            if (label)
                gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);
            break;
    }

    moo_paned_insert_pane (paned, pane_widget, box, -1);
}


static GtkWidget   *create_frame_widget (MooPaned   *paned,
                                         Pane       *pane)
{
    GtkWidget *frame, *hbox, *button, *icon;
    GtkTooltips *tooltips;

    frame = gtk_frame_new (NULL);
    gtk_frame_set_label_align (GTK_FRAME (frame), 1.0, 0.5);
    gtk_frame_set_shadow_type (GTK_FRAME (frame),
                               GTK_SHADOW_NONE);
//     gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

    hbox = gtk_hbox_new (TRUE, 0);

    tooltips = gtk_tooltips_new ();

    button = gtk_toggle_button_new ();
    pane->sticky_button = button;
    g_object_set_data (G_OBJECT (button), "moo-pane", pane);
    gtk_button_set_focus_on_click (GTK_BUTTON (button), FALSE);
    gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
    gtk_tooltips_set_tip (tooltips, button, "Sticky", "Sticky");
    icon = gtk_image_new_from_stock (MOO_STOCK_STICKY,
                                     MOO_ICON_SIZE_REAL_SMALL);
    gtk_container_add (GTK_CONTAINER (button), icon);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button),
                                  paned->priv->sticky);
    gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);

    button = gtk_button_new ();
    pane->close_button = button;
    g_object_set_data (G_OBJECT (button), "moo-pane", pane);
    gtk_button_set_focus_on_click (GTK_BUTTON (button), FALSE);
    gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
    gtk_tooltips_set_tip (tooltips, button, "Hide pane", "Hide pane");
    icon = gtk_image_new_from_stock (MOO_STOCK_CLOSE,
                                     MOO_ICON_SIZE_REAL_SMALL);
    gtk_container_add (GTK_CONTAINER (button), icon);
    gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);

    gtk_widget_show_all (hbox);
    gtk_frame_set_label_widget (GTK_FRAME (frame), hbox);

    return frame;
}


void         moo_paned_insert_pane      (MooPaned   *paned,
                                         GtkWidget  *pane_widget,
                                         GtkWidget  *button_widget,
                                         int         position)
{
    GtkWidget *button;
    Pane *pane;

    g_return_if_fail (MOO_IS_PANED (paned));
    g_return_if_fail (GTK_IS_WIDGET (pane_widget));
    g_return_if_fail (GTK_IS_WIDGET (button_widget));
    g_return_if_fail (pane_widget->parent == NULL);
    g_return_if_fail (button_widget->parent == NULL);

    button = gtk_toggle_button_new ();
    gtk_widget_show (button);
    gtk_container_add (GTK_CONTAINER (button), button_widget);
    gtk_widget_show (button_widget);

    if (position < 0 || position > (int) moo_paned_n_panes (paned))
        position = moo_paned_n_panes (paned);

    gtk_container_add_with_properties (GTK_CONTAINER (paned->button_box),
                                       button,
                                       "expand", FALSE,
                                       "fill", FALSE,
                                       "pack-type", GTK_PACK_START,
                                       "position", position,
                                       NULL);

    pane = g_new (Pane, 1);

    pane->frame = create_frame_widget (paned, pane);
    gtk_object_sink (gtk_object_ref (GTK_OBJECT (pane->frame)));
    gtk_widget_set_parent (pane->frame, GTK_WIDGET (paned));

    pane->child = pane_widget;
    gtk_widget_show (pane->child);
    gtk_container_add (GTK_CONTAINER (pane->frame), pane->child);

    pane->button = button;
    paned->priv->panes = g_slist_insert (paned->priv->panes,
                                         pane, position);

    g_object_set_data (G_OBJECT (button), "moo-pane", pane);
    g_object_set_data (G_OBJECT (pane->child), "moo-pane", pane);
    g_object_set_data (G_OBJECT (pane->frame), "moo-pane", pane);

    g_signal_connect (button, "toggled",
                      G_CALLBACK (button_toggled), paned);
    g_signal_connect_swapped (pane->close_button, "clicked",
                              G_CALLBACK (moo_paned_hide_pane), paned);
    g_signal_connect (pane->sticky_button, "toggled",
                      G_CALLBACK (sticky_button_toggled), paned);

    if (GTK_WIDGET_VISIBLE (paned->button_box))
        paned->priv->button_box_visible = TRUE;

    paned->priv->handle_visible = TRUE;
    gtk_widget_style_get (GTK_WIDGET (paned),
                          "handle_size", &paned->priv->handle_size,
                          NULL);

    if (GTK_WIDGET_VISIBLE (paned))
        gtk_widget_queue_resize (GTK_WIDGET (paned));

    if (GTK_WIDGET_REALIZED (paned))
        gtk_widget_set_parent_window (pane->frame,
                                      paned->priv->pane_window);
}


static void     moo_paned_remove        (GtkContainer   *container,
                                         GtkWidget      *widget)
{
    MooPaned *paned = MOO_PANED (container);

    if (widget == GTK_BIN(paned)->child)
        GTK_CONTAINER_CLASS(moo_paned_parent_class)->remove (container, widget);
    else
        moo_paned_remove_pane (paned, widget);
}


static void moo_paned_remove_pane       (MooPaned   *paned,
                                         GtkWidget  *pane_widget)
{
    Pane *pane;

    g_return_if_fail (MOO_IS_PANED (paned));
    g_return_if_fail (GTK_IS_WIDGET (pane_widget));

    pane = g_object_get_data (G_OBJECT (pane_widget), "moo-pane");
    g_return_if_fail (pane != NULL);
    g_return_if_fail (pane->child == pane_widget);

    if (paned->priv->current_pane && paned->priv->current_pane == pane)
        moo_paned_hide_pane (paned);

    g_object_set_data (G_OBJECT (pane->button), "moo-pane", NULL);
    g_object_set_data (G_OBJECT (pane->child), "moo-pane", NULL);
    g_object_set_data (G_OBJECT (pane->frame), "moo-pane", NULL);
    g_signal_handlers_disconnect_by_func (pane->button,
                                          (gpointer) button_toggled,
                                          paned);
    g_signal_handlers_disconnect_by_func (pane->sticky_button,
                                          (gpointer) sticky_button_toggled,
                                          paned);
    g_signal_handlers_disconnect_by_func (pane->close_button,
                                          (gpointer) moo_paned_hide_pane,
                                          paned);

    gtk_container_remove (GTK_CONTAINER (paned->button_box), pane->button);
    paned->priv->panes = g_slist_remove (paned->priv->panes, pane);

    gtk_container_remove (GTK_CONTAINER (pane->frame), pane->child);
    gtk_widget_unparent (pane->frame);

    if (!moo_paned_n_panes (paned))
    {
        paned->priv->handle_visible = FALSE;
        paned->priv->handle_size = 0;
        if (paned->priv->pane_window)
            gdk_window_hide (paned->priv->pane_window);
        gtk_widget_hide (paned->button_box);
    }

    if (GTK_WIDGET_VISIBLE (paned))
        gtk_widget_queue_resize (GTK_WIDGET (paned));
}


guint       moo_paned_n_panes           (MooPaned   *paned)
{
    g_return_val_if_fail (MOO_IS_PANED (paned), 0);
    return g_slist_length (paned->priv->panes);
}


static void     moo_paned_set_focus_child   (GtkContainer   *container,
                                             GtkWidget      *widget)
{
    MooPaned *paned = MOO_PANED (container);

    GTK_CONTAINER_CLASS(moo_paned_parent_class)->set_focus_child (container, widget);

    if (widget == GTK_BIN(paned)->child &&
        paned->priv->close_on_child_focus &&
        !paned->priv->sticky)
    {
        moo_paned_hide_pane (paned);
    }
}


static void     sticky_button_toggled   (GtkToggleButton *button,
                                         MooPaned       *paned)
{
    gboolean active = gtk_toggle_button_get_active (button);
    if (active != paned->priv->sticky)
        moo_paned_set_sticky_pane (paned, active);
}
