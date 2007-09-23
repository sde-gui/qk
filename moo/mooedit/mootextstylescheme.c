/*
 *   mootextstylescheme.c
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

#define MOOEDIT_COMPILATION
#include "mooedit/mootextstylescheme.h"
#include "mooedit/mootextview.h"
#include "mooutils/mooi18n.h"
#include "mooedit/gtksourceview/gtksourceview-api.h"

#define STYLE_HAS_FOREGROUND(s) ((s) && ((s)->mask & GTK_SOURCE_STYLE_USE_FOREGROUND))
#define STYLE_HAS_BACKGROUND(s) ((s) && ((s)->mask & GTK_SOURCE_STYLE_USE_BACKGROUND))

#define STYLE_BRACKET_MATCH	"bracket-match"
#define STYLE_BRACKET_MISMATCH	"bracket-mismatch"
#define STYLE_CURRENT_LINE      "current-line"
#define STYLE_RIGHT_MARGIN      "right-margin"

GType
moo_text_style_scheme_get_type (void)
{
    static GType type = 0;

    if (G_UNLIKELY (!type))
        type = GTK_TYPE_SOURCE_STYLE_SCHEME;

    return type;
}


GType
moo_text_style_get_type (void)
{
    static GType type = 0;

    if (G_UNLIKELY (!type))
        type = GTK_TYPE_SOURCE_STYLE;

    return type;
}


const char *
moo_text_style_scheme_get_id (MooTextStyleScheme *scheme)
{
    g_return_val_if_fail (MOO_IS_TEXT_STYLE_SCHEME (scheme), NULL);
    return gtk_source_style_scheme_get_id (GTK_SOURCE_STYLE_SCHEME (scheme));
}


const char *
moo_text_style_scheme_get_name (MooTextStyleScheme *scheme)
{
    g_return_val_if_fail (MOO_IS_TEXT_STYLE_SCHEME (scheme), NULL);
    return gtk_source_style_scheme_get_name (GTK_SOURCE_STYLE_SCHEME (scheme));
}


MooTextStyle *
_moo_text_style_scheme_get_bracket_match_style (MooTextStyleScheme *scheme)
{
    g_return_val_if_fail (MOO_IS_TEXT_STYLE_SCHEME (scheme), NULL);
    return (MooTextStyle*) gtk_source_style_scheme_get_style (GTK_SOURCE_STYLE_SCHEME (scheme),
                                                              STYLE_BRACKET_MATCH);
}


MooTextStyle *
_moo_text_style_scheme_get_bracket_mismatch_style (MooTextStyleScheme *scheme)
{
    g_return_val_if_fail (MOO_IS_TEXT_STYLE_SCHEME (scheme), NULL);
    return (MooTextStyle*) gtk_source_style_scheme_get_style (GTK_SOURCE_STYLE_SCHEME (scheme),
                                                              STYLE_BRACKET_MISMATCH);
}


MooTextStyle *
_moo_text_style_scheme_lookup_style (MooTextStyleScheme *scheme,
                                     const char         *name)
{
    g_return_val_if_fail (MOO_IS_TEXT_STYLE_SCHEME (scheme), NULL);
    g_return_val_if_fail (name != NULL, NULL);
    return (MooTextStyle*) gtk_source_style_scheme_get_style (GTK_SOURCE_STYLE_SCHEME (scheme),
                                                              name);
}


static const char *
get_color (GtkSourceStyleScheme *scheme,
           const char           *name)
{
    GtkSourceStyle *style;

    style = gtk_source_style_scheme_get_style (scheme, name);

    if (style && (style->mask & GTK_SOURCE_STYLE_USE_BACKGROUND))
        return style->background;
    else
        return NULL;
}

void
_moo_text_style_scheme_apply (MooTextStyleScheme *scheme,
                              GtkWidget          *widget)
{
    g_return_if_fail (MOO_IS_TEXT_STYLE_SCHEME (scheme));
    g_return_if_fail (GTK_IS_WIDGET (widget));

    _gtk_source_style_scheme_apply (GTK_SOURCE_STYLE_SCHEME (scheme), widget);

    if (MOO_IS_TEXT_VIEW (widget))
    {
        const char *color;

        color = get_color (GTK_SOURCE_STYLE_SCHEME (scheme), STYLE_CURRENT_LINE);
        moo_text_view_set_current_line_color (MOO_TEXT_VIEW (widget), color);

        color = get_color (GTK_SOURCE_STYLE_SCHEME (scheme), STYLE_RIGHT_MARGIN);
        moo_text_view_set_right_margin_color (MOO_TEXT_VIEW (widget), color);
    }
}


void
_moo_text_style_apply_to_tag (const MooTextStyle *style,
                              GtkTextTag         *tag)
{
    _gtk_source_style_apply (NULL, tag);
    _gtk_source_style_apply ((GtkSourceStyle*) style, tag);
}
