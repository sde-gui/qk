/*
 *   mootextstylescheme.h
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

#ifndef __MOO_TEXT_STYLE_SCHEME_H__
#define __MOO_TEXT_STYLE_SCHEME_H__

#include <mooedit/mootextstyle.h>

G_BEGIN_DECLS


#define MOO_TYPE_TEXT_STYLE_SCHEME              (moo_text_style_scheme_get_type ())
#define MOO_TEXT_STYLE_SCHEME(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_TEXT_STYLE_SCHEME, MooTextStyleScheme))
#define MOO_TEXT_STYLE_SCHEME_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_TEXT_STYLE_SCHEME, MooTextStyleSchemeClass))
#define MOO_IS_TEXT_STYLE_SCHEME(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_TEXT_STYLE_SCHEME))
#define MOO_IS_TEXT_STYLE_SCHEME_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_TEXT_STYLE_SCHEME))
#define MOO_TEXT_STYLE_SCHEME_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_TEXT_STYLE_SCHEME, MooTextStyleSchemeClass))

typedef struct _MooTextStyleScheme MooTextStyleScheme;
typedef struct _MooTextStyleSchemeClass MooTextStyleSchemeClass;

typedef enum {
    MOO_TEXT_COLOR_FG = 0,
    MOO_TEXT_COLOR_BG,
    MOO_TEXT_COLOR_SEL_FG,
    MOO_TEXT_COLOR_SEL_BG,
    MOO_TEXT_COLOR_CUR_LINE,
    MOO_TEXT_NUM_COLORS
} MooTextColor;


struct _MooTextStyleScheme
{
    GObject parent;

    MooTextStyleScheme *base_scheme;
    char *name;
    char *text_colors[MOO_TEXT_NUM_COLORS];
    MooTextStyle *bracket_match;
    MooTextStyle *bracket_mismatch;
    GHashTable *styles; /* char* -> MooTextStyle* */
};

struct _MooTextStyleSchemeClass
{
    GObjectClass parent_class;

    void (*changed) (MooTextStyleScheme *scheme);
};


GType               moo_text_style_scheme_get_type      (void) G_GNUC_CONST;

MooTextStyleScheme *moo_text_style_scheme_new_empty     (const char         *name,
                                                         MooTextStyleScheme *base);
MooTextStyleScheme *moo_text_style_scheme_new_default   (void);

MooTextStyleScheme *moo_text_style_scheme_copy          (MooTextStyleScheme *scheme);

GSList             *moo_text_style_scheme_list_default  (MooTextStyleScheme *scheme);

void                moo_text_style_scheme_compose       (MooTextStyleScheme *scheme,
                                                         const char         *language_name,
                                                         const char         *style_name,
                                                         const MooTextStyle *style);
void                moo_text_style_scheme_set           (MooTextStyleScheme *scheme,
                                                         const char         *language_name,
                                                         const char         *style_name,
                                                         const MooTextStyle *style);
const MooTextStyle *moo_text_style_scheme_get           (MooTextStyleScheme *scheme,
                                                         const char         *language_name,
                                                         const char         *style_name);

void                _moo_text_style_scheme_get_color    (MooTextStyleScheme *scheme,
                                                         MooTextColor        color,
                                                         GdkColor           *dest,
                                                         GdkScreen          *screen);


G_END_DECLS

#endif /* __MOO_TEXT_STYLE_SCHEME_H__ */
