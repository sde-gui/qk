/*
 *   mooeditfiltersettings.h
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef MOOEDIT_COMPILATION
#error "This file may not be used"
#endif

#ifndef MOO_EDIT_FILTER_SETTINGS_H
#define MOO_EDIT_FILTER_SETTINGS_H

#include <mooedit/mooedit.h>

G_BEGIN_DECLS


typedef struct _MooEditFilter MooEditFilter;

MooEditFilter  *_moo_edit_filter_new                    (const char     *string);
MooEditFilter  *_moo_edit_filter_new_langs              (const char     *string);
MooEditFilter  *_moo_edit_filter_new_regex              (const char     *string);
MooEditFilter  *_moo_edit_filter_new_globs              (const char     *string);
void            _moo_edit_filter_free                   (MooEditFilter  *filter);
gboolean        _moo_edit_filter_match                  (MooEditFilter  *filter,
                                                         MooEdit        *doc);

void            _moo_edit_filter_settings_load          (void);

GSList         *_moo_edit_filter_settings_get_strings   (void);
void            _moo_edit_filter_settings_set_strings   (GSList     *strings);

char           *_moo_edit_filter_settings_get_for_file  (const char *filename);


G_END_DECLS

#endif /* MOO_EDIT_FILTER_SETTINGS_H */
