/*
 *   mootextprint.h
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

#ifndef __MOO_TEXT_PRINT_H__
#define __MOO_TEXT_PRINT_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS


#define MOO_TYPE_PRINT_OPERATION              (moo_print_operation_get_type ())
#define MOO_PRINT_OPERATION(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_PRINT_OPERATION, MooPrintOperation))
#define MOO_PRINT_OPERATION_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_PRINT_OPERATION, MooPrintOperationClass))
#define MOO_IS_PRINT_OPERATION(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_PRINT_OPERATION))
#define MOO_IS_PRINT_OPERATION_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_PRINT_OPERATION))
#define MOO_PRINT_OPERATION_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_PRINT_OPERATION, MooPrintOperationClass))

typedef struct _MooPrintOperation         MooPrintOperation;
typedef struct _MooPrintOperationClass    MooPrintOperationClass;

struct _MooPrintOperation
{
    GtkPrintOperation base;

    GtkTextView *doc;
    GtkTextBuffer *buffer;

    /* print settings */
    int first_line;
    int last_line;          /* -1 to print everything after first_line */
    char *font;             /* overrides font set in the doc */
    gboolean wrap;
    PangoWrapMode wrap_mode;
    gboolean ellipsize;
    gboolean use_styles;

    /* aux stuff */
    GArray *pages;          /* GtkTextIter's pointing to pages start */
    PangoLayout *layout;

    struct {
        double x;
        double y;
        double width;
        double height;
    } page;                 /* text area */
};

struct _MooPrintOperationClass
{
    GtkPrintOperationClass base_class;
};


GType   moo_print_operation_get_type    (void) G_GNUC_CONST;

void    moo_print_operation_set_doc     (MooPrintOperation  *print,
                                         GtkTextView        *doc);
void    moo_print_operation_set_buffer  (MooPrintOperation  *print,
                                         GtkTextBuffer      *buffer);

void    moo_edit_page_setup             (GtkTextView    *view,
                                         GtkWidget      *parent);
void    moo_edit_print                  (GtkTextView    *view,
                                         GtkWidget      *parent);


G_END_DECLS

#endif /* __MOO_TEXT_PRINT_H__ */
