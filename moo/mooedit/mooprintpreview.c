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
#include "mooedit/mootextprint-private.h"
#include "mooedit/mooprintpreview-glade.h"
#include "mooutils/mooutils-gobject.h"
#include "mooutils/mooutils-misc.h"
#include <cairo/cairo-pdf.h>


struct _MooPrintPreviewPrivate {
    MooPrintOperation *op;
    GtkPrintContext *context;
    GtkPrintOperationPreview *gtk_preview;

    MooGladeXML *xml;
    GtkEntry *entry;
    GtkWidget *darea;
    GtkScrolledWindow *swin;

    GPtrArray *pages;
    guint n_pages;
    guint current_page;

    double page_width;
    double page_height;
    double screen_page_width;
    double screen_page_height;

    gboolean zoom_to_fit;
    guint zoom;
};

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
    preview->priv = g_new0 (MooPrintPreviewPrivate, 1);

    preview->priv->op = NULL;
    preview->priv->gtk_preview = NULL;

    preview->priv->xml = moo_glade_xml_new_empty (GETTEXT_PACKAGE);
    moo_glade_xml_fill_widget (preview->priv->xml, GTK_WIDGET (preview),
                               MOO_PRINT_PREVIEW_GLADE_XML, -1,
                               "dialog", NULL);

    preview->priv->context = NULL;
    preview->priv->gtk_preview = NULL;
    preview->priv->entry = NULL;
    preview->priv->darea = NULL;
    preview->priv->swin = NULL;
    preview->priv->pages = NULL;
    preview->priv->n_pages = 0;
    preview->priv->current_page = G_MAXUINT;
    preview->priv->page_width = 0;
    preview->priv->page_height = 0;
    preview->priv->screen_page_width = 0;
    preview->priv->screen_page_height = 0;

    preview->priv->zoom_to_fit = FALSE;
    preview->priv->zoom = ZOOM_100;
}


static void
moo_print_preview_finalize (GObject *object)
{
    guint i;
    MooPrintPreview *preview = MOO_PRINT_PREVIEW (object);

    g_object_unref (preview->priv->op);
    g_object_unref (preview->priv->gtk_preview);
    g_object_unref (preview->priv->context);
    g_object_unref (preview->priv->xml);

    if (preview->priv->pages)
    {
        for (i = 0; i < preview->priv->pages->len; ++i)
            if (preview->priv->pages->pdata[i])
                cairo_surface_destroy (preview->priv->pages->pdata[i]);
        g_ptr_array_free (preview->priv->pages, TRUE);
    }

    g_free (preview->priv);

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
    GtkPageSetup *page_setup;
    double width, height;

    page_setup = gtk_print_context_get_page_setup (preview->priv->context);

    switch (gtk_page_setup_get_orientation (page_setup))
    {
        case GTK_PAGE_ORIENTATION_LANDSCAPE:
        case GTK_PAGE_ORIENTATION_REVERSE_LANDSCAPE:
            width = preview->priv->page_height;
            height = preview->priv->page_width;
            break;

        default:
            width = preview->priv->page_width;
            height = preview->priv->page_height;
            break;
    }

    return cairo_pdf_surface_create_for_stream (dummy_write_func, NULL,
                                                width, height);
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
    preview->priv->op = g_object_ref (op);
    _moo_print_operation_set_preview (op, preview);

    preview->priv->gtk_preview = g_object_ref (gtk_preview);
    preview->priv->context = g_object_ref (context);

    gtk_window_set_transient_for (GTK_WINDOW (preview),
                                  _moo_print_operation_get_parent (op));
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

    g_return_if_fail (n < preview->priv->n_pages);

    if (preview->priv->current_page == n)
        return;

    preview->priv->current_page = n;
    gtk_widget_queue_draw (preview->priv->darea);

    gtk_widget_set_sensitive (moo_glade_xml_get_widget (preview->priv->xml, "next"),
                              n + 1 < preview->priv->n_pages);
    gtk_widget_set_sensitive (moo_glade_xml_get_widget (preview->priv->xml, "prev"),
                              n > 0);

    text = g_strdup_printf ("%d", n + 1);
    gtk_entry_set_text (preview->priv->entry, text);
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

    if (!zoom_to_fit == !preview->priv->zoom_to_fit && zoom == preview->priv->zoom)
        return;

    preview->priv->zoom_to_fit = zoom_to_fit != 0;
    preview->priv->zoom = zoom;

    if (preview->priv->zoom_to_fit)
    {
        gtk_scrolled_window_set_policy (preview->priv->swin, GTK_POLICY_NEVER, GTK_POLICY_NEVER);

        calc_size (&preview->priv->screen_page_width,
                   &preview->priv->screen_page_height,
                   preview->priv->darea->allocation.width,
                   preview->priv->darea->allocation.height,
                   preview->priv->page_width,
                   preview->priv->page_height);

        gtk_widget_set_size_request (preview->priv->darea, -1, -1);
    }
    else
    {
        gtk_scrolled_window_set_policy (preview->priv->swin, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
        preview->priv->screen_page_width = zoom_factors[zoom] * preview->priv->page_width;
        preview->priv->screen_page_height = zoom_factors[zoom] * preview->priv->page_height;
        gtk_widget_set_size_request (preview->priv->darea,
                                     preview->priv->screen_page_width,
                                     preview->priv->screen_page_height);
    }

    gtk_widget_queue_draw (preview->priv->darea);

    if (zoom_to_fit)
        _moo_message ("zoom_to_fit\n");
    else
        _moo_message ("zoom: %s\n", zoom_factor_names[zoom]);
}

static void
size_allocate (MooPrintPreview *preview)
{
    if (preview->priv->zoom_to_fit)
        calc_size (&preview->priv->screen_page_width,
                   &preview->priv->screen_page_height,
                   preview->priv->darea->allocation.width,
                   preview->priv->darea->allocation.height,
                   preview->priv->page_width,
                   preview->priv->page_height);
}


static void
moo_print_preview_zoom_in (MooPrintPreview *preview,
                           int              change)
{
    g_return_if_fail (MOO_IS_PRINT_PREVIEW (preview));
    g_return_if_fail (change == -1 || change == 1);

    if (change < 0)
        _moo_message ("zoom out\n");
    else
        _moo_message ("zoom in\n");
}


static cairo_surface_t *
get_pdf (MooPrintPreview *preview)
{
    GPtrArray *pages = preview->priv->pages;
    guint current_page = preview->priv->current_page;

    g_return_val_if_fail (current_page < preview->priv->n_pages, NULL);

    if (!pages->pdata[current_page])
    {
        cairo_t *cr;
        cairo_surface_t *pdf;

        pdf  = create_pdf_surface (preview);
        g_return_val_if_fail (pdf != NULL, NULL);

        cr = cairo_create (pdf);
        gtk_print_context_set_cairo_context (preview->priv->context, cr, 72, 72);
        gtk_print_operation_preview_render_page (preview->priv->gtk_preview, current_page);
        cairo_destroy (cr);

        pages->pdata[current_page] = pdf;
    }

    return pages->pdata[current_page];
}


static gboolean
expose_event (MooPrintPreview *preview,
              GdkEventExpose  *event,
              GtkWidget       *widget)
{
    cairo_surface_t *pdf;
    cairo_t *cr;
    cairo_matrix_t matrix;
    double scale;
    double width, height;
    GtkPageSetup *page_setup;

    pdf = get_pdf (preview);
    g_return_val_if_fail (pdf != NULL, FALSE);

    cr = gdk_cairo_create (widget->window);

    gdk_cairo_rectangle (cr, &event->area);
    cairo_clip (cr);

    width = preview->priv->page_width;
    height = preview->priv->page_height;

    scale = preview->priv->screen_page_width / width;
    cairo_scale (cr, scale, scale);

    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_rectangle (cr, 0, 0, width, height);
    cairo_fill (cr);

    page_setup = gtk_print_context_get_page_setup (preview->priv->context);
    switch (gtk_page_setup_get_orientation (page_setup))
    {
        case GTK_PAGE_ORIENTATION_LANDSCAPE:
            cairo_matrix_init (&matrix,
                               0, -1,
                               1,  0,
                               0,  0);
            cairo_transform (cr, &matrix);
            cairo_translate (cr, -height, 0);
            break;

        case GTK_PAGE_ORIENTATION_REVERSE_PORTRAIT:
            cairo_matrix_init (&matrix,
                              -1,  0,
                               0, -1,
                               0,  0);
            cairo_transform (cr, &matrix);
            cairo_translate (cr, -width, -height);
            break;

        case GTK_PAGE_ORIENTATION_REVERSE_LANDSCAPE:
            cairo_matrix_init (&matrix,
                               0,  1,
                              -1,  0,
                               0,  0);
            cairo_transform (cr, &matrix);
            cairo_translate (cr, 0, -width);
            break;

        default: /* GTK_PAGE_ORIENTATION_PORTRAIT */
          break;
    }

    cairo_set_source_surface (cr, pdf, 0, 0);
    cairo_paint (cr);

    cairo_destroy (cr);

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
    if (preview->priv->current_page + 1 < preview->priv->n_pages)
        moo_print_preview_set_page (preview, preview->priv->current_page + 1);
}

static void
prev_clicked (MooPrintPreview *preview)
{
    if (preview->priv->current_page > 0)
        moo_print_preview_set_page (preview, preview->priv->current_page - 1);
}

static void
entry_activated (MooPrintPreview *preview)
{
    int page = _moo_convert_string_to_int (gtk_entry_get_text (preview->priv->entry),
                                           preview->priv->current_page + 1) - 1;
    page = CLAMP (page, 0, (int) preview->priv->n_pages - 1);
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

    g_signal_connect_swapped (moo_glade_xml_get_widget (preview->priv->xml, "zoom_100"),
                              "clicked", G_CALLBACK (zoom_100_clicked), preview);
    g_signal_connect_swapped (moo_glade_xml_get_widget (preview->priv->xml, "zoom_to_fit"),
                              "clicked", G_CALLBACK (zoom_to_fit_clicked), preview);
    g_signal_connect_swapped (moo_glade_xml_get_widget (preview->priv->xml, "zoom_in"),
                              "clicked", G_CALLBACK (zoom_in_clicked), preview);
    g_signal_connect_swapped (moo_glade_xml_get_widget (preview->priv->xml, "zoom_out"),
                              "clicked", G_CALLBACK (zoom_out_clicked), preview);
    g_signal_connect_swapped (moo_glade_xml_get_widget (preview->priv->xml, "next"),
                              "clicked", G_CALLBACK (next_clicked), preview);
    g_signal_connect_swapped (moo_glade_xml_get_widget (preview->priv->xml, "prev"),
                              "clicked", G_CALLBACK (prev_clicked), preview);

    preview->priv->swin = moo_glade_xml_get_widget (preview->priv->xml, "swin");
    preview->priv->darea = moo_glade_xml_get_widget (preview->priv->xml, "darea");
    g_signal_connect_swapped (preview->priv->darea, "expose-event",
                              G_CALLBACK (expose_event), preview);
    g_signal_connect_swapped (preview->priv->darea, "size-allocate",
                              G_CALLBACK (size_allocate), preview);

    entry_item = moo_glade_xml_get_widget (preview->priv->xml, "entry_item");
    xml = moo_glade_xml_new_from_buf (MOO_PRINT_PREVIEW_GLADE_XML, -1,
                                      "entry_hbox", GETTEXT_PACKAGE, NULL);
    box = moo_glade_xml_get_widget (xml, "entry_hbox");
    gtk_container_add (GTK_CONTAINER (entry_item), box);

    preview->priv->entry = moo_glade_xml_get_widget (xml, "entry");
    g_signal_connect_swapped (preview->priv->entry, "activate", G_CALLBACK (entry_activated), preview);

    setup = gtk_print_context_get_page_setup (preview->priv->context);
    preview->priv->page_width = gtk_page_setup_get_paper_width (setup, GTK_UNIT_INCH) * 72.;
    preview->priv->page_height = gtk_page_setup_get_paper_height (setup, GTK_UNIT_INCH) * 72.;

    preview->priv->n_pages = _moo_print_operation_get_n_pages (preview->priv->op);
    preview->priv->pages = g_ptr_array_sized_new (preview->priv->n_pages);
    g_ptr_array_set_size (preview->priv->pages, preview->priv->n_pages);

    preview->priv->current_page = G_MAXUINT;
    moo_print_preview_set_page (preview, 0);

    preview->priv->zoom_to_fit = TRUE;
    moo_print_preview_set_zoom (preview, FALSE, ZOOM_100);

    text = g_strdup_printf ("of %d", preview->priv->n_pages);
    gtk_label_set_text (moo_glade_xml_get_widget (xml, "label"), text);
    g_free (text);

    g_object_unref (xml);
}


GtkPrintOperationPreview *
_moo_print_preview_get_gtk_preview (MooPrintPreview *preview)
{
    g_return_val_if_fail (MOO_IS_PRINT_PREVIEW (preview), NULL);
    return preview->priv->gtk_preview;
}
