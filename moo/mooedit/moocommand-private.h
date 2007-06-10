/*
 *   moocommand-private.h
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
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

#ifndef MOO_COMMAND_PRIVATE_H
#define MOO_COMMAND_PRIVATE_H

#include <mooedit/mookeyfile.h>
#include <mooedit/moocommand.h>

G_BEGIN_DECLS


void            _moo_command_init                   (void);
MooCommandData *_moo_command_parse_item             (MooKeyFileItem     *item,
                                                     const char         *name,
                                                     const char         *filename,
                                                     MooCommandFactory **factory,
                                                     char              **options);
void            _moo_command_format_item            (MooKeyFileItem     *item,
                                                     MooCommandData     *data,
                                                     MooCommandFactory  *factory,
                                                     char               *options);

GtkWidget      *_moo_command_factory_create_widget  (MooCommandFactory  *factory);
void            _moo_command_factory_load_data      (MooCommandFactory  *factory,
                                                     GtkWidget          *widget,
                                                     MooCommandData     *data);
gboolean        _moo_command_factory_save_data      (MooCommandFactory  *factory,
                                                     GtkWidget          *widget,
                                                     MooCommandData     *data);
gboolean        _moo_command_factory_data_equal     (MooCommandFactory  *factory,
                                                     MooCommandData     *data1,
                                                     MooCommandData     *data2);


G_END_DECLS

#endif /* MOO_COMMAND_PRIVATE_H */
