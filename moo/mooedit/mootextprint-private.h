/*
 *   mootextprint-private.h
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

#ifndef __MOO_TEXT_PRINT_PRIVATE_H__
#define __MOO_TEXT_PRINT_PRIVATE_H__

#include <mooedit/mootextprint.h>

G_BEGIN_DECLS


#define MOO_TYPE_PRINT_SETTINGS (_moo_print_settings_get_type ())


typedef enum {
    MOO_PRINT_WRAP       = 1 << 0,
    MOO_PRINT_ELLIPSIZE  = 1 << 1,
    MOO_PRINT_USE_STYLES = 1 << 2,
    MOO_PRINT_HEADER     = 1 << 3,
    MOO_PRINT_FOOTER     = 1 << 4
} MooPrintFlags;

typedef enum {
    MOO_PRINT_POS_LEFT,
    MOO_PRINT_POS_CENTER,
    MOO_PRINT_POS_RIGHT
} MooPrintPos;

typedef struct {
    gboolean do_print;
    PangoFontDescription *font;
    char *format[3];
    gboolean separator;

    PangoLayout *layout;
    double text_height;
    double separator_before;
    double separator_after;
    double separator_height;
    gpointer parsed_format[3];
} MooPrintHeaderFooter;

typedef struct {
    char *font;             /* overrides font set in the doc */
    MooPrintFlags flags;
    PangoWrapMode wrap_mode;
    MooPrintHeaderFooter *header;
    MooPrintHeaderFooter *footer;
} MooPrintSettings;


GType   _moo_print_settings_get_type            (void) G_GNUC_CONST;

void    _moo_print_operation_set_preview        (MooPrintOperation  *print,
                                                 MooPrintPreview    *preview);
GtkWindow *_moo_print_operation_get_parent      (MooPrintOperation  *print);
int     _moo_print_operation_get_n_pages        (MooPrintOperation  *print);


G_END_DECLS

#endif /* __MOO_TEXT_PRINT_PRIVATE_H__ */
