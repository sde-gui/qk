/*
 *   mooprintpreview.c
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define MOOEDIT_COMPILATION
#include "mooedit/mooprintpreview.h"
#include "mooedit/mooprintpreview-glade.h"
#include "mooutils/mooutils-gobject.h"
#include <cairo/cairo-pdf.h>


enum {
    ZOOM_100,
    ZOOM_LAST
};

static double zoom_factors[ZOOM_LAST] = {
    1.
};

static const char *zoom_factor_names[ZOOM_LAST] = {
    "100%"
};


G_DEFINE_TYPE(MooPrintPreview, _moo_print_preview, GTK_TYPE_DIALOG)


static void
_moo_print_preview_init (MooPrintPreview *preview)
{
    preview->op = NULL;
    preview->gtk_preview = NULL;

    preview->xml = moo_glade_xml_new_empty (GETTEXT_PACKAGE);
    moo_glade_xml_fill_widget (preview->xml, GTK_WIDGET (preview),
                               MOO_PRINT_PREVIEW_GLADE_XML, -1,
                               "dialog", NULL);
}


static void
moo_print_preview_finalize (GObject *object)
{
    MooPrintPreview *preview = MOO_PRINT_PREVIEW (object);

    g_object_unref (preview->op);
    g_object_unref (preview->gtk_preview);
    g_object_unref (preview->xml);

    G_OBJECT_CLASS (_moo_print_preview_parent_class)->finalize (object);
}


static void
_moo_print_preview_class_init (MooPrintPreviewClass *klass)
{
    G_OBJECT_CLASS (klass)->finalize = moo_print_preview_finalize;
}


static cairo_status_t
dummy_write_func (G_GNUC_UNUSED gpointer      closure,
                  G_GNUC_UNUSED const guchar *data,
                  G_GNUC_UNUSED guint         length)
{
    return CAIRO_STATUS_SUCCESS;
}

static cairo_surface_t *
create_pdf_surface (MooPrintPreview *preview)
{
    return cairo_pdf_surface_create_for_stream (dummy_write_func, NULL,
                                                preview->page_width,
                                                preview->page_height);
}


GtkWidget *
_moo_print_preview_new (MooPrintOperation        *op,
                        GtkPrintOperationPreview *gtk_preview,
                        GtkPrintContext          *context)
{
    MooPrintPreview *preview;
    cairo_surface_t *pdf;
    cairo_t *cr;

    g_return_val_if_fail (MOO_IS_PRINT_OPERATION (op), NULL);
    g_return_val_if_fail (GTK_IS_PRINT_OPERATION_PREVIEW (gtk_preview), NULL);

    preview = g_object_new (MOO_TYPE_PRINT_PREVIEW, NULL);
    preview->op = g_object_ref (op);
    preview->op->preview = preview;
    preview->gtk_preview = g_object_ref (gtk_preview);
    preview->context = g_object_ref (context);

    gtk_window_set_transient_for (GTK_WINDOW (preview), op->parent);
    gtk_window_set_modal (GTK_WINDOW (preview), TRUE);

    pdf = create_pdf_surface (preview);
    cr = cairo_create (pdf);
    gtk_print_context_set_cairo_context (context, cr, 72, 72);
    cairo_destroy (cr);
    cairo_surface_destroy (pdf);

    return GTK_WIDGET (preview);
}


static void
moo_print_preview_set_page (MooPrintPreview *preview,
                            guint            n)
{
    char *text;

    g_return_if_fail (n < preview->n_pages);

    if (preview->current_page == n)
        return;

    preview->current_page = n;
    gtk_widget_queue_draw (preview->darea);

    gtk_widget_set_sensitive (moo_glade_xml_get_widget (preview->xml, "next"),
                              n + 1 < preview->n_pages);
    gtk_widget_set_sensitive (moo_glade_xml_get_widget (preview->xml, "prev"),
                              n > 0);

    text = g_strdup_printf ("%d", n + 1);
    gtk_entry_set_text (preview->entry, text);
    g_free (text);
}


static void
calc_size (double *width,
           double *height,
           double  alloc_width,
           double  alloc_height,
           double  page_width,
           double  page_height)
{
        if (alloc_width / alloc_height > page_width / page_height)
        {
            *height = alloc_height;
            *width = alloc_height * (page_width / page_height);
        }
        else
        {
            *width = alloc_width;
            *height = alloc_width * (page_height / page_width);
        }
}

static void
moo_print_preview_set_zoom (MooPrintPreview *preview,
                            gboolean         zoom_to_fit,
                            guint            zoom)
{
    g_return_if_fail (MOO_IS_PRINT_PREVIEW (preview));
    g_return_if_fail (zoom_to_fit || zoom < ZOOM_LAST);

    if (zoom_to_fit)
        zoom = ZOOM_100;

    if (!zoom_to_fit == !preview->zoom_to_fit && zoom == preview->zoom)
        return;

    preview->zoom_to_fit = zoom_to_fit != 0;
    preview->zoom = zoom;

    if (preview->zoom_to_fit)
    {
        gtk_scrolled_window_set_policy (preview->swin, GTK_POLICY_NEVER, GTK_POLICY_NEVER);

        calc_size (&preview->screen_page_width,
                   &preview->screen_page_height,
                   preview->darea->allocation.width,
                   preview->darea->allocation.height,
                   preview->page_width,
                   preview->page_height);

        gtk_widget_set_size_request (preview->darea, -1, -1);
    }
    else
    {
        gtk_scrolled_window_set_policy (preview->swin, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
        preview->screen_page_width = zoom_factors[zoom] * preview->page_width;
        preview->screen_page_height = zoom_factors[zoom] * preview->page_height;
        gtk_widget_set_size_request (preview->darea, preview->screen_page_width, preview->screen_page_height);
    }

    gtk_widget_queue_draw (preview->darea);

    if (zoom_to_fit)
        g_print ("zoom_to_fit\n");
    else
        g_print ("zoom: %s\n", zoom_factor_names[zoom]);
}

static void
size_allocate (MooPrintPreview *preview)
{
    if (preview->zoom_to_fit)
        calc_size (&preview->screen_page_width,
                   &preview->screen_page_height,
                   preview->darea->allocation.width,
                   preview->darea->allocation.height,
                   preview->page_width,
                   preview->page_height);
}


static void
moo_print_preview_zoom_in (MooPrintPreview *preview,
                           int              change)
{
    g_return_if_fail (MOO_IS_PRINT_PREVIEW (preview));
    g_return_if_fail (change == -1 || change == 1);

    if (change < 0)
        g_print ("zoom out\n");
    else
        g_print ("zoom in\n");
}


static cairo_surface_t *
get_pdf (MooPrintPreview *preview)
{
    cairo_surface_t *pdf;
    cairo_t *cr;

    g_return_val_if_fail (preview->current_page < preview->n_pages, NULL);

    if (preview->pages->pdata[preview->current_page])
        return preview->pages->pdata[preview->current_page];

    pdf = create_pdf_surface (preview);
    g_return_val_if_fail (pdf != NULL, NULL);

    cr = cairo_create (pdf);
    gtk_print_context_set_cairo_context (preview->context, cr, 72, 72);
    gtk_print_operation_preview_render_page (preview->gtk_preview, preview->current_page);
    cairo_destroy (cr);

    preview->pages->pdata[preview->current_page] = pdf;
    return pdf;
}


static gboolean
expose_event (MooPrintPreview *preview,
              GdkEventExpose  *event,
              GtkWidget       *widget)
{
    cairo_surface_t *pdf;
    cairo_t *cr;
    double scale;
#if 0
    GTimer *timer;
#endif

    pdf = get_pdf (preview);
    g_return_val_if_fail (pdf != NULL, FALSE);

    cr = gdk_cairo_create (widget->window);

    gdk_cairo_rectangle (cr, &event->area);
    cairo_clip (cr);

    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_rectangle (cr, 0, 0, preview->screen_page_width, preview->screen_page_height);
    cairo_fill (cr);

#if 0
    timer = g_timer_new ();
#endif

    scale = preview->screen_page_width / preview->page_width;
    cairo_scale (cr, scale, scale);
    cairo_set_source_surface (cr, pdf, 0, 0);
    cairo_paint (cr);

    cairo_destroy (cr);

#if 0
    g_print ("%f s\n", g_timer_elapsed (timer, NULL));
    g_timer_destroy (timer);
#endif

    return FALSE;
}


static void
zoom_100_clicked (MooPrintPreview *preview)
{
    moo_print_preview_set_zoom (preview, FALSE, ZOOM_100);
}

static void
zoom_to_fit_clicked (MooPrintPreview *preview)
{
    moo_print_preview_set_zoom (preview, TRUE, 0);
}

static void
zoom_in_clicked (MooPrintPreview *preview)
{
    moo_print_preview_zoom_in (preview, 1);
}

static void
zoom_out_clicked (MooPrintPreview *preview)
{
    moo_print_preview_zoom_in (preview, -1);
}

static void
next_clicked (MooPrintPreview *preview)
{
    if (preview->current_page + 1 < preview->n_pages)
        moo_print_preview_set_page (preview, preview->current_page + 1);
}

static void
prev_clicked (MooPrintPreview *preview)
{
    if (preview->current_page > 0)
        moo_print_preview_set_page (preview, preview->current_page - 1);
}

static void
entry_activated (MooPrintPreview *preview)
{
    int page = _moo_convert_string_to_int (gtk_entry_get_text (preview->entry),
                                           preview->current_page + 1) - 1;
    page = CLAMP (page, 0, (int) preview->n_pages - 1);
    moo_print_preview_set_page (preview, page);
}

void
_moo_print_preview_start (MooPrintPreview *preview)
{
    GtkToolItem *entry_item;
    MooGladeXML *xml;
    GtkWidget *box;
    GtkPageSetup *setup;
    char *text;

    g_signal_connect_swapped (moo_glade_xml_get_widget (preview->xml, "zoom_100"),
                              "clicked", G_CALLBACK (zoom_100_clicked), preview);
    g_signal_connect_swapped (moo_glade_xml_get_widget (preview->xml, "zoom_to_fit"),
                              "clicked", G_CALLBACK (zoom_to_fit_clicked), preview);
    g_signal_connect_swapped (moo_glade_xml_get_widget (preview->xml, "zoom_in"),
                              "clicked", G_CALLBACK (zoom_in_clicked), preview);
    g_signal_connect_swapped (moo_glade_xml_get_widget (preview->xml, "zoom_out"),
                              "clicked", G_CALLBACK (zoom_out_clicked), preview);
    g_signal_connect_swapped (moo_glade_xml_get_widget (preview->xml, "next"),
                              "clicked", G_CALLBACK (next_clicked), preview);
    g_signal_connect_swapped (moo_glade_xml_get_widget (preview->xml, "prev"),
                              "clicked", G_CALLBACK (prev_clicked), preview);

    preview->swin = moo_glade_xml_get_widget (preview->xml, "swin");
    preview->darea = moo_glade_xml_get_widget (preview->xml, "darea");
    g_signal_connect_swapped (preview->darea, "expose-event",
                              G_CALLBACK (expose_event), preview);
    g_signal_connect_swapped (preview->darea, "size-allocate",
                              G_CALLBACK (size_allocate), preview);

    entry_item = moo_glade_xml_get_widget (preview->xml, "entry_item");
    xml = moo_glade_xml_new_from_buf (MOO_PRINT_PREVIEW_GLADE_XML, -1,
                                      "entry_hbox", GETTEXT_PACKAGE, NULL);
    box = moo_glade_xml_get_widget (xml, "entry_hbox");
    gtk_container_add (GTK_CONTAINER (entry_item), box);

    preview->entry = moo_glade_xml_get_widget (xml, "entry");
    g_signal_connect_swapped (preview->entry, "activate", G_CALLBACK (entry_activated), preview);

    setup = gtk_print_context_get_page_setup (preview->context);
    preview->page_width = gtk_page_setup_get_paper_width (setup, GTK_UNIT_INCH) * 72.;
    preview->page_height = gtk_page_setup_get_paper_height (setup, GTK_UNIT_INCH) * 72.;

    preview->n_pages = preview->op->pages->len;
    preview->pages = g_ptr_array_sized_new (preview->n_pages);
    g_ptr_array_set_size (preview->pages, preview->n_pages);

    preview->current_page = G_MAXUINT;
    moo_print_preview_set_page (preview, 0);

    preview->zoom_to_fit = TRUE;
    moo_print_preview_set_zoom (preview, FALSE, ZOOM_100);

    text = g_strdup_printf ("of %d", preview->n_pages);
    gtk_label_set_text (moo_glade_xml_get_widget (xml, "label"), text);
    g_free (text);

    g_object_unref (xml);
}
