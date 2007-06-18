/*
 *   mootextstyle.h
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

#ifndef MOO_TEXT_STYLE_H
#define MOO_TEXT_STYLE_H

#include <gdk/gdkcolor.h>
#include <gtk/gtktexttag.h>

G_BEGIN_DECLS


#define MOO_TYPE_TEXT_STYLE         (moo_text_style_get_type ())
#define MOO_TEXT_STYLE(obj)         (G_TYPE_CHECK_INSTANCE_CAST ((obj), MOO_TYPE_TEXT_STYLE, MooTextStyle))
#define MOO_IS_TEXT_STYLE(obj)      (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MOO_TYPE_TEXT_STYLE))

typedef struct _MooTextStyle MooTextStyle;

GType            moo_text_style_get_type        (void) G_GNUC_CONST;

MooTextStyle    *moo_text_style_new             (void);
MooTextStyle    *moo_text_style_copy            (const MooTextStyle *style);
void             moo_text_style_free            (MooTextStyle       *style);

void             _moo_text_style_apply_to_tag   (const MooTextStyle *style,
                                                 GtkTextTag         *tag);


G_END_DECLS

#endif /* MOO_TEXT_STYLE_H */
