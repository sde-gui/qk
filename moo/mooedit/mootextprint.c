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

    print->last_line = -1;
}


void
moo_print_operation_set_doc (MooPrintOperation  *print,
                             GtkTextView        *doc)
{
    g_return_if_fail (MOO_IS_PRINT_OPERATION (print));
    g_return_if_fail (!doc || GTK_IS_TEXT_VIEW (doc));

    if (print->doc)
        g_object_unref (print->doc);
    if (print->buffer)
        g_object_unref (print->buffer);

    print->doc = doc;

    if (print->doc)
    {
        g_object_ref (print->doc);
        print->buffer = gtk_text_view_get_buffer (print->doc);
        g_object_ref (print->buffer);
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
    PangoFontDescription *font = NULL;
    PangoLayoutLine *layout_line;
    double page_height;
    GList *page_breaks;
    int num_lines;
    int line;
    GtkTextIter iter;
    GTimer *timer;

    g_return_if_fail (print->doc != NULL);
    g_return_if_fail (print->first_line >= 0);
    g_return_if_fail (print->last_line < 0 || print->last_line >= print->first_line);
    g_return_if_fail (print->first_line < gtk_text_buffer_get_line_count (print->buffer));
    g_return_if_fail (print->last_line < gtk_text_buffer_get_line_count (print->buffer));

    timer = g_timer_new ();

    if (print->last_line < 0)
        print->last_line = gtk_text_buffer_get_line_count (print->buffer) - 1;

    print->page.x = print->page.y = 0;
    print->page.width = gtk_print_context_get_width (context);
    print->page.height = gtk_print_context_get_height (context);

    if (print->font)
        font = pango_font_description_from_string (print->font);

    if (!font)
    {
        PangoContext *widget_ctx;

        widget_ctx = gtk_widget_get_pango_context (GTK_WIDGET (print->doc));

        if (widget_ctx)
        {
            font = pango_context_get_font_description (widget_ctx);

            if (font)
                font = pango_font_description_copy_static (font);
        }
    }

    print->layout = gtk_print_context_create_layout (context);

    if (font)
    {
        pango_layout_set_font_description (print->layout, font);
        pango_font_description_free (font);
    }

    print->pages = g_array_new (FALSE, FALSE, sizeof (GtkTextIter));
    gtk_text_buffer_get_iter_at_line (print->buffer, &iter,
                                      print->first_line);
    g_array_append_val (print->pages, iter);
    page_height = 0;

    for (line = print->first_line; line <= print->last_line; line++)
    {
        GtkTextIter end;
        PangoRectangle line_rect;
        double line_height;
        char *text;

        end = iter;

        if (!gtk_text_iter_ends_line (&end))
            gtk_text_iter_forward_to_line_end (&end);

        text = gtk_text_buffer_get_text (print->buffer, &iter, &end, FALSE);
        pango_layout_set_text (print->layout, text, -1);
        g_free (text);

        pango_layout_get_pixel_extents (print->layout, NULL, &line_rect);

        if (page_height + line_rect.height > print->page.height)
        {
            g_array_append_val (print->pages, iter);
            page_height = 0;
        }

        page_height += line_rect.height;
        gtk_text_iter_forward_line (&iter);
    }

    gtk_print_operation_set_nr_of_pages (operation, print->pages->len);

    g_message ("begin_print: %d pages in %f s", print->pages->len,
               g_timer_elapsed (timer, NULL));
    g_timer_destroy (timer);
}


static void
moo_print_operation_draw_page (GtkPrintOperation  *operation,
                               GtkPrintContext    *context,
                               int                 page_nr)
{
    cairo_t *cr;
    GtkTextIter start, end;
    char *text;
    MooPrintOperation *print = MOO_PRINT_OPERATION (operation);
    GTimer *timer;

    g_return_if_fail (print->doc != NULL);
    g_return_if_fail (print->pages != NULL);
    g_return_if_fail (print->layout != NULL);
    g_return_if_fail (page_nr < print->pages->len);

    timer = g_timer_new ();

    cr = gtk_print_context_get_cairo (context);
    cairo_set_source_rgb (cr, 0, 0, 0);

    start = g_array_index (print->pages, GtkTextIter, page_nr);

    if (page_nr + 1 < print->pages->len)
        end = g_array_index (print->pages, GtkTextIter, page_nr + 1);
    else
        gtk_text_buffer_get_end_iter (print->buffer, &end);

    text = gtk_text_buffer_get_text (print->buffer, &start, &end, FALSE);
    pango_layout_set_text (print->layout, text, -1);
    g_free (text);

    cairo_move_to (cr, print->page.x, print->page.y);
    pango_cairo_show_layout (cr, print->layout);

    g_message ("page %d: %f s", page_nr, g_timer_elapsed (timer, NULL));
    g_timer_destroy (timer);
}


static void
moo_print_operation_end_print (GtkPrintOperation  *operation,
                               GtkPrintContext    *context)
{
    MooPrintOperation *print = MOO_PRINT_OPERATION (operation);

    g_return_if_fail (print->doc != NULL);

    g_object_unref (print->layout);
    g_array_free (print->pages, TRUE);

    print->layout = NULL;
    print->pages = NULL;
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

    {
        GtkTextIter start, end;
        GtkTextBuffer *buffer = gtk_text_view_get_buffer (view);
        gtk_text_buffer_get_bounds (buffer, &start, &end);
        g_assert (strlen (gtk_text_buffer_get_text (buffer, &start, &end, FALSE)) > 0);
        g_assert (MOO_PRINT_OPERATION(print)->last_line < 0);
    }

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
