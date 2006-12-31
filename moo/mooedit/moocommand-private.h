/*
 *   moocommand-private.h
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
#error "This file may not be used"
#endif

#ifndef __MOO_COMMAND_PRIVATE_H__
#define __MOO_COMMAND_PRIVATE_H__

#include <mooedit/mookeyfile.h>
#include <mooedit/moocommand.h>

G_BEGIN_DECLS


void            _moo_command_init                   (void);
MooCommandData *_moo_command_parse_item             (MooKeyFileItem     *item,
                                                     const char         *name,
                                                     const char         *filename,
                                                     MooCommandType    **type,
                                                     char              **options);
void            _moo_command_format_item            (MooKeyFileItem     *item,
                                                     MooCommandData     *data,
                                                     MooCommandType     *type,
                                                     char               *options);

GtkWidget      *_moo_command_type_create_widget     (MooCommandType     *type);
void            _moo_command_type_load_data         (MooCommandType     *type,
                                                     GtkWidget          *widget,
                                                     MooCommandData     *data);
gboolean        _moo_command_type_save_data         (MooCommandType     *type,
                                                     GtkWidget          *widget,
                                                     MooCommandData     *data);
gboolean        _moo_command_type_data_equal        (MooCommandType     *type,
                                                     MooCommandData     *data1,
                                                     MooCommandData     *data2);


G_END_DECLS

#endif /* __MOO_COMMAND_PRIVATE_H__ */
