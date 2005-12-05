/*
 *   moocellrenderercolor.c
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

#include "mooutils/moocellrenderercolor.h"
#include "mooutils/moomarshals.h"
#include "mooutils/moodialogs.h"
#include <gtk/gtkcolorseldialog.h>

#define CELL_MIN_WIDTH 20
#define CELL_MIN_HEIGHT 12
#define CELL_BORDER_WIDTH 1


static void     moo_cell_renderer_color_get_property    (GObject              *object,
                                                         guint                 param_id,
                                                         GValue               *value,
                                                         GParamSpec           *pspec);
static void     moo_cell_renderer_color_set_property    (GObject              *object,
                                                         guint                 param_id,
                                                         const GValue         *value,
                                                         GParamSpec           *pspec);
static void     moo_cell_renderer_color_get_size        (GtkCellRenderer      *cell,
                                                         GtkWidget            *widget,
                                                         GdkRectangle         *rectangle,
                                                         gint                 *x_offset,
                                                         gint                 *y_offset,
                                                         gint                 *width,
                                                         gint                 *height);
static void     moo_cell_renderer_color_render          (GtkCellRenderer      *cell,
                                                         GdkDrawable          *window,
                                                         GtkWidget            *widget,
                                                         GdkRectangle         *background_area,
                                                         GdkRectangle         *cell_area,
                                                         GdkRectangle         *expose_area,
                                                         GtkCellRendererState  flags);
static gboolean moo_cell_renderer_color_activate        (GtkCellRenderer      *cell,
                                                         GdkEvent             *event,
                                                         GtkWidget            *widget,
                                                         const gchar          *path,
                                                         GdkRectangle         *background_area,
                                                         GdkRectangle         *cell_area,
                                                         GtkCellRendererState  flags);


enum {
    PROP_0,
    PROP_COLOR,
    PROP_COLOR_SET,
    PROP_ACTIVATABLE
};

enum {
    COLOR_SET,
    NUM_SIGNALS
};

static guint signals[NUM_SIGNALS];

G_DEFINE_TYPE (MooCellRendererColor, moo_cell_renderer_color, GTK_TYPE_CELL_RENDERER)


static void
moo_cell_renderer_color_class_init (MooCellRendererColorClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    GtkCellRendererClass *cell_class = GTK_CELL_RENDERER_CLASS (klass);

    object_class->get_property = moo_cell_renderer_color_get_property;
    object_class->set_property = moo_cell_renderer_color_set_property;

    cell_class->get_size = moo_cell_renderer_color_get_size;
    cell_class->render = moo_cell_renderer_color_render;
    cell_class->activate = moo_cell_renderer_color_activate;

    g_object_class_install_property (object_class,
                                     PROP_COLOR,
                                     g_param_spec_boxed ("color",
                                             "color",
                                             "color",
                                             GDK_TYPE_COLOR,
                                             G_PARAM_READWRITE));

    g_object_class_install_property (object_class,
                                     PROP_ACTIVATABLE,
                                     g_param_spec_boolean ("activatable",
                                             "activatable",
                                             "activatable",
                                             FALSE,
                                             G_PARAM_READWRITE));

    g_object_class_install_property (object_class,
                                     PROP_COLOR_SET,
                                     g_param_spec_boolean ("color-set",
                                             "color-set",
                                             "color-set",
                                             FALSE,
                                             G_PARAM_READWRITE));

    signals[COLOR_SET] =
            g_signal_new ("color-set",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooCellRendererColorClass, color_set),
                          NULL, NULL,
                          _moo_marshal_VOID__BOXED,
                          G_TYPE_NONE, 1,
                          GDK_TYPE_COLOR);
}


static void
moo_cell_renderer_color_init (MooCellRendererColor *cell)
{
    cell->color_set = FALSE;
    cell->activatable = FALSE;
    GTK_CELL_RENDERER(cell)->mode = GTK_CELL_RENDERER_MODE_ACTIVATABLE;
    GTK_CELL_RENDERER(cell)->xpad = 2;
    GTK_CELL_RENDERER(cell)->ypad = 2;
}


static void
moo_cell_renderer_color_get_property (GObject        *object,
                                      guint           param_id,
                                      GValue         *value,
                                      GParamSpec     *pspec)
{
    MooCellRendererColor *cell = MOO_CELL_RENDERER_COLOR (object);

    switch (param_id)
    {
        case PROP_COLOR:
            g_value_set_boxed (value, &cell->color);
            break;

        case PROP_COLOR_SET:
            g_value_set_boolean (value, cell->color_set ? TRUE : FALSE);
            break;

        case PROP_ACTIVATABLE:
            g_value_set_boolean (value, cell->activatable ? TRUE : FALSE);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
            break;
    }
}


static void
moo_cell_renderer_color_set_property (GObject      *object,
                                      guint         param_id,
                                      const GValue *value,
                                      GParamSpec   *pspec)
{
    GdkColor *color;
    MooCellRendererColor *cell = MOO_CELL_RENDERER_COLOR (object);

    switch (param_id)
    {
        case PROP_COLOR:
            color = g_value_get_boxed (value);
            if (color)
            {
                cell->color = *color;
                cell->color_set = TRUE;
            }
            else
            {
                cell->color_set = FALSE;
            }
            break;

        case PROP_COLOR_SET:
            cell->color_set = g_value_get_boolean (value) ? TRUE : FALSE;
            break;

        case PROP_ACTIVATABLE:
            cell->activatable = g_value_get_boolean (value) ? TRUE : FALSE;
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
            break;
    }
}


GtkCellRenderer *
moo_cell_renderer_color_new (void)
{
    return g_object_new (MOO_TYPE_CELL_RENDERER_COLOR, NULL);
}


static void
moo_cell_renderer_color_get_size (GtkCellRenderer *gtkcell,
                                  GtkWidget       *widget,
                                  GdkRectangle    *cell_area,
                                  gint            *x_offset,
                                  gint            *y_offset,
                                  gint            *width,
                                  gint            *height)
{
    int border_x = CELL_BORDER_WIDTH;
    int border_y = CELL_BORDER_WIDTH;

    if (widget->style)
    {
        border_x = widget->style->xthickness;
        border_y = widget->style->ythickness;
    }

    if (x_offset)
        *x_offset = 0;
    if (y_offset)
        *y_offset = 0;
    if (width)
        *width = 2 * gtkcell->xpad + 2 * border_x + CELL_MIN_WIDTH;
    if (height)
        *height = 2 * gtkcell->ypad + 2 * border_y + CELL_MIN_HEIGHT;
}


static void
moo_cell_renderer_color_render (GtkCellRenderer      *gtkcell,
                                GdkWindow            *window,
                                GtkWidget            *widget,
                                GdkRectangle         *background_area,
                                GdkRectangle         *cell_area,
                                GdkRectangle         *expose_area,
                                GtkCellRendererState  flags)

{
    MooCellRendererColor *cell = MOO_CELL_RENDERER_COLOR (gtkcell);
    GdkRectangle rect;
    GdkGC *gc;

    rect = *cell_area;
    rect.x += gtkcell->xpad;
    rect.y += gtkcell->ypad;
    rect.width  -= gtkcell->xpad * 2;
    rect.height -= gtkcell->ypad * 2;

    if (cell->color_set)
    {
        static GdkGCValues values;
        GdkColormap *colormap;

        colormap = gtk_widget_get_colormap (widget);
        g_return_if_fail (colormap != NULL);
        gdk_colormap_alloc_color (colormap, &cell->color, TRUE, TRUE);

        values.foreground = cell->color;

        gc = gdk_gc_new_with_values (window, &values, GDK_GC_FOREGROUND);
        gdk_draw_rectangle (window, gc, TRUE, rect.x, rect.y,
                            rect.width, rect.height);
        g_object_unref (gc);
    }

    if (flags & GTK_CELL_RENDERER_SELECTED)
        gc = widget->style->text_gc[GTK_STATE_SELECTED];
    else if (flags & GTK_CELL_RENDERER_INSENSITIVE)
        gc = widget->style->text_gc[GTK_STATE_INSENSITIVE];
    else
        gc = widget->style->text_gc[GTK_STATE_NORMAL];

    gdk_draw_rectangle (window, gc, FALSE, rect.x, rect.y,
                        rect.width - 1, rect.height - 1);

    if (!cell->color_set)
        gdk_draw_line (window, gc,
                       rect.x,
                       rect.y + rect.height - 1,
                       rect.x + rect.width - 1,
                       rect.y);

}


static gboolean
moo_cell_renderer_color_activate (GtkCellRenderer      *gtkcell,
                                  GdkEvent             *event,
                                  GtkWidget            *widget,
                                  const gchar          *path,
                                  GdkRectangle         *background_area,
                                  GdkRectangle         *cell_area,
                                  GtkCellRendererState  flags)
{
    MooCellRendererColor *cell = MOO_CELL_RENDERER_COLOR (gtkcell);

    if (cell->activatable)
    {
        GtkWidget *dialog;
        GtkColorSelectionDialog *color_dialog;
        GtkColorSelection *colorsel;
        GdkColor color;
        int response;

        dialog = gtk_color_selection_dialog_new (NULL);
        color_dialog = GTK_COLOR_SELECTION_DIALOG (dialog);
        colorsel = GTK_COLOR_SELECTION (color_dialog->colorsel);

        gtk_color_selection_set_current_color (colorsel, &cell->color);

        moo_position_window (dialog, widget, TRUE, FALSE, 0, 0);
        response = gtk_dialog_run (GTK_DIALOG (dialog));

        if (response == GTK_RESPONSE_OK)
            gtk_color_selection_get_current_color (colorsel, &color);

        gtk_widget_destroy (dialog);

        if (response == GTK_RESPONSE_OK)
            g_signal_emit (cell, signals[COLOR_SET], 0, &color);

        return TRUE;
    }

    return FALSE;
}
