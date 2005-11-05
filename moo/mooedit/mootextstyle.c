/*
 *   mootextstyle.c
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

#define MOOEDIT_COMPILATION
#include "mooedit/mootextstyle.h"
#include "mooedit/moolang-strings.h"
#include "mooedit/mootextview.h"
#include "mooedit/mootextbuffer.h"
#include <string.h>


static GMemChunk *style_chunk__ = NULL;

inline static void
init_style_chunk__ (void)
{
    if (!style_chunk__)
        style_chunk__ = g_mem_chunk_create (MooTextStyle, 128, G_ALLOC_AND_FREE);
}

inline static MooTextStyle*
style_new (void)
{
    init_style_chunk__ ();
    return g_chunk_new (MooTextStyle, style_chunk__);
}

inline static MooTextStyle*
style_new0 (void)
{
    init_style_chunk__ ();
    return g_chunk_new0 (MooTextStyle, style_chunk__);
}

inline static void
style_free (MooTextStyle *style)
{
    g_chunk_free (style, style_chunk__);
}


MooTextStyle*
moo_text_style_new (const char      *default_style,
                    const GdkColor  *foreground,
                    const GdkColor  *background,
                    gboolean         bold,
                    gboolean         italic,
                    gboolean         underline,
                    gboolean         strikethrough,
                    MooTextStyleMask mask,
                    gboolean         modified)
{
    MooTextStyle *style;

    g_return_val_if_fail (!(mask & MOO_TEXT_STYLE_FOREGROUND) || foreground, NULL);
    g_return_val_if_fail (!(mask & MOO_TEXT_STYLE_BACKGROUND) || background, NULL);

    style = style_new ();

    style->default_style = g_strdup (default_style);
    style->bold = bold;
    style->italic = italic;
    style->underline = underline;
    style->strikethrough = strikethrough;
    style->mask = mask;
    style->modified = modified;

    if (mask & MOO_TEXT_STYLE_FOREGROUND)
        style->foreground = *foreground;
    if (mask & MOO_TEXT_STYLE_BACKGROUND)
        style->background = *background;

    return style;
}


MooTextStyle*
moo_text_style_copy (const MooTextStyle *style)
{
    MooTextStyle *copy;

    g_return_val_if_fail (style != NULL, NULL);

    copy = style_new ();
    copy->default_style = NULL;
    moo_text_style_copy_content (copy, style);

    return copy;
}


void
moo_text_style_copy_content (MooTextStyle       *dest,
                             const MooTextStyle *src)
{
    g_return_if_fail (dest != NULL && src != NULL && dest != src);
    g_free (dest->default_style);
    memcpy (dest, src, sizeof (MooTextStyle));
    dest->default_style = g_strdup (src->default_style);
}


void
moo_text_style_compose (MooTextStyle       *dest,
                        const MooTextStyle *src)
{
    g_return_if_fail (dest != NULL && src != NULL && dest != src);

    if (src->default_style)
    {
        g_free (dest->default_style);
        dest->default_style = g_strdup (src->default_style);
    }

#define MAYBE_COPY(mask__,what__)       \
    if (src->mask & mask__)             \
    {                                   \
        dest->what__ = src->what__;     \
        dest->mask |= mask__;           \
    }

    MAYBE_COPY (MOO_TEXT_STYLE_FOREGROUND, foreground);
    MAYBE_COPY (MOO_TEXT_STYLE_BACKGROUND, background);
    MAYBE_COPY (MOO_TEXT_STYLE_BOLD, bold);
    MAYBE_COPY (MOO_TEXT_STYLE_ITALIC, italic);
    MAYBE_COPY (MOO_TEXT_STYLE_UNDERLINE, underline);
    MAYBE_COPY (MOO_TEXT_STYLE_STRIKETHROUGH, strikethrough);

    dest->modified |= src->modified;
#undef MAYBE_COPY
}


void
moo_text_style_free (MooTextStyle *style)
{
    if (style)
    {
        g_free (style->default_style);
        style_free (style);
    }
}


GType
moo_text_style_get_type (void)
{
    static GType type = 0;

    if (!type)
        type = g_boxed_type_register_static ("MooTextStyle",
                                             (GBoxedCopyFunc) moo_text_style_copy,
                                             (GBoxedFreeFunc) moo_text_style_free);

    return type;
}


GType
moo_text_style_mask_get_type (void)
{
    static GType type = 0;

    if (!type)
    {
        static GFlagsValue values[] = {
            { MOO_TEXT_STYLE_FOREGROUND, (char*) "MOO_TEXT_STYLE_FOREGROUND", (char*) "foreground" },
            { MOO_TEXT_STYLE_BACKGROUND, (char*) "MOO_TEXT_STYLE_BACKGROUND", (char*) "background" },
            { MOO_TEXT_STYLE_BOLD, (char*) "MOO_TEXT_STYLE_BOLD", (char*) "bold" },
            { MOO_TEXT_STYLE_ITALIC, (char*) "MOO_TEXT_STYLE_ITALIC", (char*) "italic" },
            { MOO_TEXT_STYLE_UNDERLINE, (char*) "MOO_TEXT_STYLE_UNDERLINE", (char*) "underline" },
            { MOO_TEXT_STYLE_STRIKETHROUGH, (char*) "MOO_TEXT_STYLE_STRIKETHROUGH", (char*) "strikethrough" },
            { 0, NULL, NULL }
        };

        type = g_flags_register_static ("MooTextStyleMask", values);
    }

    return type;
}
