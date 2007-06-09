/*
 *   mootextstylescheme.h
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

#ifndef MOO_TEXT_STYLE_SCHEME_H
#define MOO_TEXT_STYLE_SCHEME_H

#include <mooedit/mootextstyle.h>
#include <gtk/gtkwidget.h>

G_BEGIN_DECLS


#define MOO_TYPE_TEXT_STYLE_SCHEME              (moo_text_style_scheme_get_type ())
#define MOO_TEXT_STYLE_SCHEME(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_TEXT_STYLE_SCHEME, MooTextStyleScheme))
#define MOO_IS_TEXT_STYLE_SCHEME(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_TEXT_STYLE_SCHEME))

typedef struct _MooTextStyleScheme MooTextStyleScheme;


GType               moo_text_style_scheme_get_type      (void) G_GNUC_CONST;

const char         *moo_text_style_scheme_get_id        (MooTextStyleScheme *scheme);
const char         *moo_text_style_scheme_get_name      (MooTextStyleScheme *scheme);

/* result must be freed with moo_text_style_free */
MooTextStyle       *_moo_text_style_scheme_get_bracket_match_style
                                                        (MooTextStyleScheme *scheme);
MooTextStyle       *_moo_text_style_scheme_get_bracket_mismatch_style
                                                        (MooTextStyleScheme *scheme);
void                _moo_text_style_scheme_apply        (MooTextStyleScheme *scheme,
                                                         GtkWidget          *widget);
/* result must be freed with moo_text_style_free */
MooTextStyle       *_moo_text_style_scheme_lookup_style (MooTextStyleScheme *scheme,
                                                         const char         *name);


G_END_DECLS

#endif /* MOO_TEXT_STYLE_SCHEME_H */
