/*
 *   mootextprint.c
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

#include "mooedit/mootextprint.h"


static GtkPageSetup *page_setup;
static GtkPrintSettings *print_settings;

static void load_default_settings   (void);

static void moo_print_operation_finalize    (GObject            *object);
static void moo_print_operation_set_property(GObject            *object,
                                             guint               prop_id,
                                             const GValue       *value,
                                             GParamSpec         *pspec);
static void moo_print_operation_get_property(GObject            *object,
                                             guint               prop_id,
                                             GValue             *value,
                                             GParamSpec         *pspec);

static void moo_print_operation_begin_print (GtkPrintOperation  *operation,
                                             GtkPrintContext    *context);
static void moo_print_operation_draw_page   (GtkPrintOperation  *operation,
                                             GtkPrintContext    *context,
                                             int                 page_nr);
static void moo_print_operation_end_print   (GtkPrintOperation  *operation,
                                             GtkPrintContext    *context);


G_DEFINE_TYPE(MooPrintOperation, moo_print_operation, GTK_TYPE_PRINT_OPERATION)

enum {
    PROP_0,
    PROP_DOC
};


static void
moo_print_operation_finalize (GObject *object)
{
    MooPrintOperation *print = MOO_PRINT_OPERATION (object);

    if (print->doc)
        g_object_unref (print->doc);
    if (print->buffer)
        g_object_unref (print->buffer);

    G_OBJECT_CLASS(moo_print_operation_parent_class)->finalize (object);
}


static void
moo_print_operation_set_property (GObject            *object,
                                  guint               prop_id,
                                  const GValue       *value,
                                  GParamSpec         *pspec)
{
    MooPrintOperation *print = MOO_PRINT_OPERATION (object);

    switch (prop_id)
    {
        case PROP_DOC:
            moo_print_operation_set_doc (print, g_value_get_object (value));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void
moo_print_operation_get_property (GObject            *object,
                                  guint               prop_id,
                                  GValue             *value,
                                  GParamSpec         *pspec)
{
    MooPrintOperation *print = MOO_PRINT_OPERATION (object);

    switch (prop_id)
    {
        case PROP_DOC:
            g_value_set_object (value, print->doc);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void
moo_print_operation_class_init (MooPrintOperationClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    GtkPrintOperationClass *print_class = GTK_PRINT_OPERATION_CLASS (klass);

    object_class->finalize = moo_print_operation_finalize;
    object_class->set_property = moo_print_operation_set_property;
    object_class->get_property = moo_print_operation_get_property;

    print_class->begin_print = moo_print_operation_begin_print;
    print_class->draw_page = moo_print_operation_draw_page;
    print_class->end_print = moo_print_operation_end_print;

    g_object_class_install_property (object_class,
                                     PROP_DOC,
                                     g_param_spec_object ("doc",
                                             "doc",
                                             "doc",
                                             GTK_TYPE_TEXT_VIEW,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
}


static void
moo_print_operation_init (MooPrintOperation *print)
{
    load_default_settings ();
    gtk_print_operation_set_print_settings (GTK_PRINT_OPERATION (print),
                                            print_settings);

    if (page_setup)
        gtk_print_operation_set_default_page_setup (GTK_PRINT_OPERATION (print),
                                                    page_setup);
}


void
moo_print_operation_set_doc (MooPrintOperation  *print,
                             GtkTextView        *doc)
{
    g_return_if_fail (MOO_IS_PRINT_OPERATION (print));
    g_return_if_fail (!doc || GTK_IS_TEXT_VIEW (doc));

    if (print->doc)
        g_object_unref (print->doc);

    print->doc = doc;

    if (print->doc)
    {
        g_object_ref (print->doc);
        print->buffer = gtk_text_view_get_buffer (print->doc);
    }
    else
    {
        print->buffer = NULL;
    }
}


static void
load_default_settings (void)
{
    if (!print_settings)
        print_settings = gtk_print_settings_new ();
}


void
moo_edit_page_setup (GtkTextView    *view,
                     GtkWidget      *parent)
{
    GtkPageSetup *new_page_setup;
    GtkWindow *parent_window = NULL;

    g_return_if_fail (!view || GTK_IS_TEXT_VIEW (view));

    load_default_settings ();

    if (!parent && view)
        parent = GTK_WIDGET (view);

    if (parent)
        parent = gtk_widget_get_toplevel (parent);

    if (parent && GTK_WIDGET_TOPLEVEL (parent))
        parent_window = GTK_WINDOW (parent);

    new_page_setup = gtk_print_run_page_setup_dialog (parent_window,
                                                      page_setup,
                                                      print_settings);

    if (page_setup)
        g_object_unref (page_setup);

    page_setup = new_page_setup;
}


static void
moo_print_operation_begin_print (GtkPrintOperation  *operation,
                                 GtkPrintContext    *context)
{
    MooPrintOperation *print = MOO_PRINT_OPERATION (operation);
    PangoFontDescription *desc;
    PangoLayoutLine *layout_line;
    double width, height;
    double page_height;
    GList *page_breaks;
    int num_lines;
    int line;

    g_return_if_fail (print->doc != NULL);

    width = gtk_print_context_get_width (context);
    height = gtk_print_context_get_height (context);

    g_object_get (print->buffer, "text", &print->text, NULL);
    print->layout = gtk_print_context_create_layout (context);

    desc = pango_font_description_from_string ("Courier New 12");
    pango_layout_set_font_description (print->layout, desc);
    pango_font_description_free (desc);

    pango_layout_set_width (print->layout, width * PANGO_SCALE);

    pango_layout_set_text (print->layout, print->text, -1);

    num_lines = pango_layout_get_line_count (print->layout);

    page_breaks = NULL;
    page_height = 0;

    for (line = 0; line < num_lines; line++)
    {
        PangoRectangle ink_rect, logical_rect;
        double line_height;

        layout_line = pango_layout_get_line (print->layout, line);
        pango_layout_line_get_extents (layout_line, &ink_rect, &logical_rect);

        line_height = logical_rect.height / 1024.0;

        if (page_height + line_height > height)
        {
            page_breaks = g_list_prepend (page_breaks, GINT_TO_POINTER (line));
            page_height = 0;
        }

        page_height += line_height;
    }

    page_breaks = g_list_reverse (page_breaks);
    gtk_print_operation_set_nr_of_pages (operation, g_list_length (page_breaks) + 1);

    print->page_breaks = page_breaks;
}


static void
moo_print_operation_draw_page (GtkPrintOperation  *operation,
                               GtkPrintContext    *context,
                               int                 page_nr)
{
    cairo_t *cr;
    GList *pagebreak;
    int start, end, i;
    PangoLayoutIter *iter;
    double start_pos;
    MooPrintOperation *print = MOO_PRINT_OPERATION (operation);

    g_return_if_fail (print->doc != NULL);

    if (page_nr == 0)
    {
        start = 0;
    }
    else
    {
        pagebreak = g_list_nth (print->page_breaks, page_nr - 1);
        start = GPOINTER_TO_INT (pagebreak->data);
    }

    pagebreak = g_list_nth (print->page_breaks, page_nr);

    if (pagebreak == NULL)
        end = pango_layout_get_line_count (print->layout);
    else
        end = GPOINTER_TO_INT (pagebreak->data);

    cr = gtk_print_context_get_cairo (context);

    cairo_set_source_rgb (cr, 0, 0, 0);

    i = 0;
    start_pos = 0;
    iter = pango_layout_get_iter (print->layout);

    do
    {
        PangoRectangle   logical_rect;
        PangoLayoutLine *line;
        int              baseline;

        if (i >= start)
        {
            line = pango_layout_iter_get_line (iter);

            pango_layout_iter_get_line_extents (iter, NULL, &logical_rect);
            baseline = pango_layout_iter_get_baseline (iter);

            if (i == start)
                start_pos = logical_rect.y / 1024.0;

            cairo_move_to (cr, logical_rect.x / 1024.0, baseline / 1024.0 - start_pos);

            pango_cairo_show_layout_line  (cr, line);
        }

        i++;
    }
    while (i < end && pango_layout_iter_next_line (iter));
}


static void
moo_print_operation_end_print (GtkPrintOperation  *operation,
                               GtkPrintContext    *context)
{
    MooPrintOperation *print = MOO_PRINT_OPERATION (operation);

    g_return_if_fail (print->doc != NULL);

    if (print->layout)
        g_object_unref (print->layout);
    g_free (print->text);
    g_list_free (print->page_breaks);

    print->layout = NULL;
    print->text = NULL;
    print->page_breaks = NULL;
}


void
moo_edit_print (GtkTextView *view,
                GtkWidget   *parent)
{
    GtkWindow *parent_window = NULL;
    GtkWidget *error_dialog;
    GtkPrintOperation *print;
    GtkPrintOperationResult res;
    GError *error = NULL;

    g_return_if_fail (GTK_IS_TEXT_VIEW (view));

    print = g_object_new (MOO_TYPE_PRINT_OPERATION, "doc", view, NULL);

    if (!parent)
        parent = GTK_WIDGET (view);
    parent = gtk_widget_get_toplevel (parent);
    if (GTK_WIDGET_TOPLEVEL (parent))
        parent_window = GTK_WINDOW (parent);

    res = gtk_print_operation_run (print, parent_window, &error);

    switch (res)
    {
        case GTK_PRINT_OPERATION_RESULT_ERROR:
            error_dialog = gtk_message_dialog_new (parent_window,
                                                   GTK_DIALOG_DESTROY_WITH_PARENT,
                                                   GTK_MESSAGE_ERROR,
                                                   GTK_BUTTONS_CLOSE,
                                                   "Error printing file:\n%s",
                                                   error ? error->message : "");
            gtk_dialog_run (GTK_DIALOG (error_dialog));
            gtk_widget_destroy (error_dialog);
            g_error_free (error);
            break;

        case GTK_PRINT_OPERATION_RESULT_APPLY:
            if (print_settings)
                g_object_unref (print_settings);
            print_settings = gtk_print_operation_get_print_settings (print);
            g_object_ref (print_settings);
            break;

        case GTK_PRINT_OPERATION_RESULT_CANCEL:
            break;
    }

    g_object_unref (print);
}
