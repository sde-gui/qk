/*
 *   mooprintpreview.h
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

#ifndef MOOEDIT_COMPILATION
#error "This file may not be used directly"
#endif

#ifndef __MOO_PRINT_PREVIEW_H__
#define __MOO_PRINT_PREVIEW_H__

#include <mooedit/mootextprint.h>
#include <mooutils/mooglade.h>

G_BEGIN_DECLS


#define MOO_TYPE_PRINT_PREVIEW              (_moo_print_preview_get_type ())
#define MOO_PRINT_PREVIEW(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_PRINT_PREVIEW, MooPrintPreview))
#define MOO_PRINT_PREVIEW_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_PRINT_PREVIEW, MooPrintPreviewClass))
#define MOO_IS_PRINT_PREVIEW(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_PRINT_PREVIEW))
#define MOO_IS_PRINT_PREVIEW_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_PRINT_PREVIEW))
#define MOO_PRINT_PREVIEW_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_PRINT_PREVIEW, MooPrintPreviewClass))

typedef struct _MooPrintPreviewClass MooPrintPreviewClass;

#define MOO_PRINT_PREVIEW_RESPONSE_PRINT GTK_RESPONSE_APPLY

struct _MooPrintPreview
{
    GtkDialog base;
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

struct _MooPrintPreviewClass
{
    GtkDialogClass base_class;
};


GType        _moo_print_preview_get_type                (void) G_GNUC_CONST;

GtkWidget   *_moo_print_preview_new                     (MooPrintOperation          *op,
                                                         GtkPrintOperationPreview   *gtk_preview,
                                                         GtkPrintContext            *context);
void         _moo_print_preview_start                   (MooPrintPreview            *preview);
cairo_t     *_moo_print_preview_create_cairo_context    (MooPrintOperation          *op,
                                                         GtkPrintOperationPreview   *gtk_preview,
                                                         GtkPrintContext            *context);


G_END_DECLS

#endif /* __MOO_PRINT_PREVIEW_H__ */
