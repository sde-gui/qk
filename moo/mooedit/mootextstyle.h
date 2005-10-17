/*
 *   mootextstyle.h
 *
 *   Copyright (C) 2004-2005 by Yevgen Muntyan <muntyan@math.tamu.edu>
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

G_BEGIN_DECLS


#define MOO_TYPE_TEXT_STYLE         (moo_text_style_get_type ())
#define MOO_TYPE_TEXT_STYLE_MASK    (moo_text_style_mask_get_type ())

typedef struct _MooTextStyle MooTextStyle;
typedef struct _MooTextStyleArray MooTextStyleArray;

typedef enum  {
    MOO_TEXT_STYLE_FOREGROUND       = 1 << 0,
    MOO_TEXT_STYLE_BACKGROUND       = 1 << 1,
    MOO_TEXT_STYLE_BOLD             = 1 << 2,
    MOO_TEXT_STYLE_ITALIC           = 1 << 3,
    MOO_TEXT_STYLE_UNDERLINE        = 1 << 4,
    MOO_TEXT_STYLE_STRIKETHROUGH    = 1 << 5
} MooTextStyleMask;

struct _MooTextStyle {
    char *default_style;
    GdkColor foreground;
    GdkColor background;
    guint bold : 1;
    guint italic : 1;
    guint underline : 1;
    guint strikethrough : 1;
    guint modified : 1;
    MooTextStyleMask mask;
};

struct _MooTextStyleArray {
    MooTextStyle **data;
    guint len;
};


GType               moo_text_style_get_type         (void) G_GNUC_CONST;
GType               moo_text_style_mask_get_type    (void) G_GNUC_CONST;

MooTextStyle       *moo_text_style_new              (const char         *default_style,
                                                     const GdkColor     *foreground,
                                                     const GdkColor     *background,
                                                     gboolean            bold,
                                                     gboolean            italic,
                                                     gboolean            underline,
                                                     gboolean            strikethrough,
                                                     MooTextStyleMask    mask,
                                                     gboolean            modified);
MooTextStyle       *moo_text_style_copy             (const MooTextStyle *style);
void                moo_text_style_copy_content     (MooTextStyle       *dest,
                                                     const MooTextStyle *src);
void                moo_text_style_compose          (MooTextStyle       *dest,
                                                     const MooTextStyle *src);
void                moo_text_style_free             (MooTextStyle       *style);


G_END_DECLS

#endif /* __MOO_TEXT_STYLE_H__ */
