/*
 *   mootextprint.h
 *
 *   Copyright (C) 2004-2010 by Yevgen Muntyan <emuntyan@sourceforge.net>
 *
 *   This file is part of medit.  medit is free software; you can
 *   redistribute it and/or modify it under the terms of the
 *   GNU Lesser General Public License as published by the
 *   Free Software Foundation; either version 2.1 of the License,
 *   or (at your option) any later version.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with medit.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MOOEDIT_COMPILATION
#error "This file may not be used directly"
#endif

#ifndef MOO_TEXT_PRINT_H
#define MOO_TEXT_PRINT_H

#include <gtk/gtk.h>

G_BEGIN_DECLS


#define MOO_TYPE_PRINT_OPERATION              (_moo_print_operation_get_type ())
#define MOO_PRINT_OPERATION(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_PRINT_OPERATION, MooPrintOperation))
#define MOO_PRINT_OPERATION_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_PRINT_OPERATION, MooPrintOperationClass))
#define MOO_IS_PRINT_OPERATION(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_PRINT_OPERATION))
#define MOO_IS_PRINT_OPERATION_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_PRINT_OPERATION))
#define MOO_PRINT_OPERATION_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_PRINT_OPERATION, MooPrintOperationClass))

typedef struct _MooPrintOperation         MooPrintOperation;
typedef struct _MooPrintOperationPrivate  MooPrintOperationPrivate;
typedef struct _MooPrintOperationClass    MooPrintOperationClass;

struct _MooPrintOperation
{
    GtkPrintOperation base;
    MooPrintOperationPrivate *priv;
};

struct _MooPrintOperationClass
{
    GtkPrintOperationClass base_class;
};


GType   _moo_print_operation_get_type           (void) G_GNUC_CONST;

void    _moo_edit_page_setup                    (GtkWidget          *parent);
void    _moo_edit_print                         (GtkTextView        *view,
                                                 GtkWidget          *parent);
void    _moo_edit_print_preview                 (GtkTextView        *view,
                                                 GtkWidget          *parent);
void    _moo_edit_export_pdf                    (GtkTextView        *view,
                                                 const char         *filename);


G_END_DECLS

#endif /* MOO_TEXT_PRINT_H */
