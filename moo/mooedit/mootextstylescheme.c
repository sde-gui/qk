/*
 *   mootextstylescheme.c
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

#define MOOEDIT_COMPILATION
#include "mooedit/mootextstylescheme.h"
#include "mooedit/mootextview.h"
#include "mooutils/mooi18n.h"
#include "mooedit/gtksourceview/gtksourceview-api.h"

#define STYLE_HAS_FOREGROUND(s) ((s) && ((s)->mask & GTK_SOURCE_STYLE_USE_FOREGROUND))
#define STYLE_HAS_BACKGROUND(s) ((s) && ((s)->mask & GTK_SOURCE_STYLE_USE_BACKGROUND))

#define STYLE_TEXT		"text"
#define STYLE_SELECTED		"text-selected"
#define STYLE_BRACKET_MATCH	"bracket-match"
#define STYLE_BRACKET_MISMATCH	"bracket-mismatch"
#define STYLE_CURSOR		"cursor"
#define STYLE_CURRENT_LINE	"current-line"

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

MooTextStyle *
moo_text_style_new (MooTextStyleMask mask)
{
    return (MooTextStyle*) gtk_source_style_new (mask);
}

MooTextStyle *
moo_text_style_copy (const MooTextStyle *style)
{
    return style ? (MooTextStyle*) gtk_source_style_copy ((GtkSourceStyle*) style) : NULL;
}

void
moo_text_style_free (MooTextStyle *style)
{
    if (style)
        gtk_source_style_free ((GtkSourceStyle*) style);
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


static void
set_text_style (GtkWidget      *widget,
		GtkSourceStyle *style,
		GtkStateType    state)
{
	GdkColor *color;

	if (STYLE_HAS_BACKGROUND (style))
		color = &style->background;
	else
		color = NULL;

	gtk_widget_modify_base (widget, state, color);

	if (STYLE_HAS_FOREGROUND (style))
		color = &style->foreground;
	else
		color = NULL;

	gtk_widget_modify_text (widget, state, color);
}

void
_moo_text_style_scheme_apply (MooTextStyleScheme *scheme,
                              GtkWidget          *widget)
{
	GtkSourceStyle *style;

	g_return_if_fail (MOO_IS_TEXT_STYLE_SCHEME (scheme));
	g_return_if_fail (GTK_IS_WIDGET (widget));

	gtk_widget_ensure_style (widget);

	style = gtk_source_style_scheme_get_style (GTK_SOURCE_STYLE_SCHEME (scheme), STYLE_TEXT);
	set_text_style (widget, style, GTK_STATE_NORMAL);
	set_text_style (widget, style, GTK_STATE_ACTIVE);
	set_text_style (widget, style, GTK_STATE_PRELIGHT);
	set_text_style (widget, style, GTK_STATE_INSENSITIVE);
	gtk_source_style_free (style);

	style = gtk_source_style_scheme_get_style (GTK_SOURCE_STYLE_SCHEME (scheme), STYLE_SELECTED);
	set_text_style (widget, style, GTK_STATE_SELECTED);
	gtk_source_style_free (style);

        if (MOO_IS_TEXT_VIEW (widget))
        {
            style = gtk_source_style_scheme_get_style (GTK_SOURCE_STYLE_SCHEME (scheme), STYLE_CURSOR);
            moo_text_view_set_cursor_color (MOO_TEXT_VIEW (widget), style ? &style->foreground : NULL);
            gtk_source_style_free (style);

            style = gtk_source_style_scheme_get_style (GTK_SOURCE_STYLE_SCHEME (scheme), STYLE_CURRENT_LINE);
            moo_text_view_set_current_line_color (MOO_TEXT_VIEW (widget), style ? &style->foreground : NULL);
            gtk_source_style_free (style);
        }
}


void
_moo_text_style_apply_to_tag (const MooTextStyle *style,
                              GtkTextTag         *tag)
{
    _gtk_source_style_apply (NULL, tag);
    _gtk_source_style_apply ((GtkSourceStyle*) style, tag);
}
