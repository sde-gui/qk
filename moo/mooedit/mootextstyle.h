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

#ifndef __MOO_TEXT_STYLE_H__
#define __MOO_TEXT_STYLE_H__

#include <gdk/gdkcolor.h>
#include <gtk/gtktexttag.h>

G_BEGIN_DECLS


#define MOO_TYPE_TEXT_STYLE         (moo_text_style_get_type ())

typedef struct _MooTextStyle MooTextStyle;

/* must be the same as GtkSourceStyleMask */
typedef enum
{
    MOO_TEXT_STYLE_USE_BACKGROUND    = 1 << 0,	/*< nick=use_background >*/
    MOO_TEXT_STYLE_USE_FOREGROUND    = 1 << 1,	/*< nick=use_foreground >*/
    MOO_TEXT_STYLE_USE_ITALIC        = 1 << 2,	/*< nick=use_italic >*/
    MOO_TEXT_STYLE_USE_BOLD          = 1 << 3,	/*< nick=use_bold >*/
    MOO_TEXT_STYLE_USE_UNDERLINE     = 1 << 4,	/*< nick=use_underline >*/
    MOO_TEXT_STYLE_USE_STRIKETHROUGH = 1 << 5	/*< nick=use_strikethrough >*/
} MooTextStyleMask;

/* must be the same as GtkSourceStyle */
struct _MooTextStyle
{
	MooTextStyleMask mask;

	GdkColor foreground;
	GdkColor background;

	guint italic : 1;
	guint bold : 1;
	guint underline : 1;
	guint strikethrough : 1;
};


GType            moo_text_style_get_type        (void) G_GNUC_CONST;

MooTextStyle    *moo_text_style_new             (MooTextStyleMask    mask);
MooTextStyle    *moo_text_style_copy            (const MooTextStyle *style);
void             moo_text_style_free            (MooTextStyle       *style);

void             _moo_text_style_apply_to_tag   (const MooTextStyle *style,
                                                 GtkTextTag         *tag);


G_END_DECLS

#endif /* __MOO_TEXT_STYLE_H__ */
