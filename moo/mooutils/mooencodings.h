/*
 *   mooencodings.h
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef MOO_ENCODINGS_H
#define MOO_ENCODINGS_H

#include <gtk/gtk.h>

G_BEGIN_DECLS


#define MOO_ENCODING_AUTO   "auto"
#define MOO_ENCODING_UTF8   "UTF-8"

typedef enum {
    MOO_ENCODING_COMBO_OPEN,
    MOO_ENCODING_COMBO_SAVE
} MooEncodingComboType;

void         _moo_encodings_combo_init      (GtkComboBox            *combo,
                                             MooEncodingComboType    type,
					     gboolean		     use_separators);
void         _moo_encodings_combo_set_enc   (GtkComboBox            *combo,
                                             const char             *enc,
                                             MooEncodingComboType    type);
const char  *_moo_encodings_combo_get_enc   (GtkComboBox            *combo,
                                             MooEncodingComboType    type);

void         _moo_encodings_attach_combo    (GtkWidget              *dialog,
                                             GtkWidget              *box,
                                             gboolean                save_mode,
                                             const char             *encoding);
const char  *_moo_encodings_combo_get       (GtkWidget              *dialog,
                                             gboolean                save_mode);

typedef void (*MooEncodingsMenuFunc)        (const char             *encoding,
                                             gpointer                data);
GtkAction   *_moo_encodings_menu_action_new (const char             *id,
                                             const char             *label,
                                             MooEncodingsMenuFunc    func,
                                             gpointer                data);
void         _moo_encodings_menu_action_set_current (GtkAction      *action,
                                             const char             *enc);

const char  *_moo_encoding_locale           (void);
gboolean     _moo_encodings_equal           (const char             *enc1,
                                             const char             *enc2);
const char  *_moo_encoding_get_display_name (const char             *enc);


G_END_DECLS

#endif /* MOO_ENCODINGS_H */
