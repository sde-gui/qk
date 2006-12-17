/*
 *   mooencodings.h
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

#ifndef __MOO_ENCODINGS_H__
#define __MOO_ENCODINGS_H__

#include <gtk/gtkwidget.h>

G_BEGIN_DECLS


#define MOO_ENCODING_LOCALE "locale"
#define MOO_ENCODING_AUTO   "auto"
#define MOO_ENCODING_UTF8   "UTF-8"

void         _moo_encodings_attach_combo    (GtkWidget  *dialog,
                                             GtkWidget  *box,
                                             gboolean    save_mode,
                                             const char *encoding);
void         _moo_encodings_sync_combo      (GtkWidget  *dialog,
                                             gboolean    save_mode);
const char  *_moo_encodings_combo_get       (GtkWidget  *dialog,
                                             gboolean    save_mode);


G_END_DECLS

#endif /* __MOO_ENCODINGS_H__ */
