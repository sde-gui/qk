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
            { MOO_TEXT_STYLE_STRIKETHROUGH, (char*) "MOO_TEXT_STYLE_STRIKETHROUGH", (char*) "strikethrough" }
        };

        type = g_flags_register_static ("MooTextStyleMask", values);
    }

    return type;
}


/****************************************************************************/
/* Style schemes
 */

static void fill_in_default_scheme (MooTextStyleScheme *scheme);


GType
moo_text_style_scheme_get_type (void)
{
    static GType type = 0;

    if (!type)
        type = g_boxed_type_register_static ("MooTextStyleScheme",
                                             (GBoxedCopyFunc) moo_text_style_scheme_ref,
                                             (GBoxedFreeFunc) moo_text_style_scheme_unref);

    return type;
}


MooTextStyleScheme*
moo_text_style_scheme_new_empty (const char *name)
{
    MooTextStyleScheme *scheme;

    g_return_val_if_fail (name && name[0], NULL);

    scheme = g_new0 (MooTextStyleScheme, 1);

    scheme->name = g_strdup (name);
    scheme->styles = g_hash_table_new_full (g_str_hash, g_str_equal,
                                            g_free,
                                            (GDestroyNotify) moo_text_style_free);
    scheme->ref_count = 1;
    scheme->use_theme_colors = TRUE;

    return scheme;
}


MooTextStyleScheme*
moo_text_style_scheme_new_default (void)
{
    MooTextStyleScheme *scheme;
    scheme = moo_text_style_scheme_new_empty (SCHEME_DEFAULT);
    fill_in_default_scheme (scheme);
    return scheme;
}


static void
copy_style (const char         *style_name,
            const MooTextStyle *style,
            GHashTable         *dest)
{
    g_hash_table_insert (dest, g_strdup (style_name),
                         moo_text_style_copy (style));
}

MooTextStyleScheme*
moo_text_style_scheme_copy (MooTextStyleScheme *scheme)
{
    MooTextStyleScheme *copy;

    g_return_val_if_fail (scheme != NULL, NULL);

    copy = moo_text_style_scheme_new_empty (scheme->name);

    copy->foreground = g_strdup (scheme->foreground);
    copy->background = g_strdup (scheme->background);
    copy->selected_foreground = g_strdup (scheme->selected_foreground);
    copy->selected_background = g_strdup (scheme->selected_background);
    copy->current_line = g_strdup (scheme->current_line);
    copy->use_theme_colors = scheme->use_theme_colors;
    copy->bracket_match = moo_text_style_copy (scheme->bracket_match);
    copy->bracket_mismatch = moo_text_style_copy (scheme->bracket_mismatch);

    g_hash_table_foreach (scheme->styles, (GHFunc) copy_style, scheme->styles);

    return copy;
}


void
moo_text_style_scheme_compose (MooTextStyleScheme *scheme,
                               const char         *language_name,
                               const char         *style_name,
                               const MooTextStyle *style)
{
    const char *key;
    char *freeme = NULL;
    MooTextStyle *old_style;

    g_return_if_fail (scheme != NULL);
    g_return_if_fail (style_name != NULL && style != NULL);

    if (language_name)
    {
        freeme = g_strdup_printf ("%s::%s", language_name, style_name);
        key = freeme;
    }
    else
    {
        key = style_name;
    }

    old_style = g_hash_table_lookup (scheme->styles, key);

    if (old_style)
        moo_text_style_compose (old_style, style);
    else
        g_hash_table_insert (scheme->styles, g_strdup (key),
                             moo_text_style_copy (style));

    g_free (freeme);
}


void
moo_text_style_scheme_set (MooTextStyleScheme *scheme,
                           const char         *language_name,
                           const char         *style_name,
                           const MooTextStyle *style)
{
    const char *key;
    char *freeme = NULL;

    g_return_if_fail (scheme != NULL);
    g_return_if_fail (style_name != NULL && style != NULL);

    if (language_name)
    {
        freeme = g_strdup_printf ("%s::%s", language_name, style_name);
        key = freeme;
    }
    else
    {
        key = style_name;
    }

    g_hash_table_insert (scheme->styles, g_strdup (key),
                         moo_text_style_copy (style));

    g_free (freeme);
}


const MooTextStyle*
moo_text_style_scheme_get (MooTextStyleScheme *scheme,
                           const char         *language_name,
                           const char         *style_name)
{
    const MooTextStyle *style;
    const char *key;
    char *freeme = NULL;

    g_return_val_if_fail (scheme != NULL, NULL);
    g_return_val_if_fail (style_name != NULL, NULL);

    if (language_name)
    {
        freeme = g_strdup_printf ("%s::%s", language_name, style_name);
        key = freeme;
    }
    else
    {
        key = style_name;
    }

    style = g_hash_table_lookup (scheme->styles, key);

    g_free (freeme);
    return style;
}


MooTextStyleScheme*
moo_text_style_scheme_ref (MooTextStyleScheme *scheme)
{
    g_return_val_if_fail (scheme != NULL, NULL);
    scheme->ref_count++;
    return scheme;
}


void
moo_text_style_scheme_unref (MooTextStyleScheme *scheme)
{
    if (scheme && !(--scheme->ref_count))
    {
        g_free (scheme->name);
        g_free (scheme->foreground);
        g_free (scheme->background);
        g_free (scheme->selected_foreground);
        g_free (scheme->selected_background);
        g_free (scheme->current_line);
        g_hash_table_destroy (scheme->styles);
        moo_text_style_free (scheme->bracket_match);
        moo_text_style_free (scheme->bracket_mismatch);
        g_free (scheme);
    }
}


static MooTextStyle*
new_style (const char      *fg,
           const char      *bg,
           gboolean         italic,
           gboolean         bold,
           gboolean         underline,
           gboolean         strikethrough,
           MooTextStyleMask mask)
{
    GdkColor foreground, background;

    if (mask & MOO_TEXT_STYLE_FOREGROUND)
    {
        if (!gdk_color_parse (fg, &foreground))
        {
            g_warning ("could not parse color %s", fg);
            mask = mask & ~MOO_TEXT_STYLE_FOREGROUND;
        }
    }

    if (mask & MOO_TEXT_STYLE_BACKGROUND)
    {
        if (!gdk_color_parse (bg, &background))
        {
            g_warning ("could not parse color %s", bg);
            mask = mask & ~MOO_TEXT_STYLE_BACKGROUND;
        }
    }

    return moo_text_style_new (NULL, &foreground, &background,
                               bold, italic, underline, strikethrough,
                               mask, FALSE);
}


#define INSERT(name_,fg_,bg_,italic_,bold_,underline_,strikethrough_,mask_)     \
    g_hash_table_insert (scheme->styles, g_strdup (name_),                      \
                         new_style (fg_, bg_, italic_, bold_,                   \
                                    underline_, strikethrough_, mask_))

static void
fill_in_default_scheme (MooTextStyleScheme *scheme)
{
    scheme->current_line = g_strdup ("#EEF6FF");
    scheme->bracket_match = new_style (NULL, "#FFFF99", FALSE,
                                       FALSE, FALSE, FALSE,
                                       MOO_TEXT_STYLE_BACKGROUND);

    INSERT (DEF_STYLE_NORMAL,
            NULL, NULL, FALSE, FALSE, FALSE, FALSE, 0);

    INSERT (DEF_STYLE_PREPROCESSOR,
            "#008000", NULL, FALSE, FALSE, FALSE, FALSE,
            MOO_TEXT_STYLE_FOREGROUND);

    INSERT (DEF_STYLE_STRING,
            "#DD0000", NULL, FALSE, FALSE, FALSE, FALSE,
            MOO_TEXT_STYLE_FOREGROUND);

    INSERT (DEF_STYLE_COMMENT,
            "#808080", NULL, TRUE, FALSE, FALSE, FALSE,
            MOO_TEXT_STYLE_FOREGROUND | MOO_TEXT_STYLE_ITALIC);

    INSERT (DEF_STYLE_BASE_N,
            "#008080", NULL, FALSE, FALSE, FALSE, FALSE,
            MOO_TEXT_STYLE_FOREGROUND);

    INSERT (DEF_STYLE_KEYWORD,
            NULL, NULL, FALSE, TRUE, FALSE, FALSE,
            MOO_TEXT_STYLE_BOLD);

    INSERT (DEF_STYLE_DATA_TYPE,
            "#800000", NULL, FALSE, FALSE, FALSE, FALSE,
            MOO_TEXT_STYLE_FOREGROUND);

    INSERT (DEF_STYLE_DECIMAL,
            "#0000FF", NULL, FALSE, FALSE, FALSE, FALSE,
            MOO_TEXT_STYLE_FOREGROUND);

    INSERT (DEF_STYLE_FLOAT,
            "#800080", NULL, FALSE, FALSE, FALSE, FALSE,
            MOO_TEXT_STYLE_FOREGROUND);

    INSERT (DEF_STYLE_CHAR,
            "#FF00FF", NULL, FALSE, FALSE, FALSE, FALSE,
            MOO_TEXT_STYLE_FOREGROUND);

    INSERT (DEF_STYLE_ALERT,
            "#FFFFFF", "#FFCCCC", FALSE, TRUE, FALSE, FALSE,
            MOO_TEXT_STYLE_FOREGROUND | MOO_TEXT_STYLE_BACKGROUND |
                    MOO_TEXT_STYLE_BOLD | MOO_TEXT_STYLE_ITALIC);

    INSERT (DEF_STYLE_FUNCTION,
            "#000080", NULL, FALSE, FALSE, FALSE, FALSE,
            MOO_TEXT_STYLE_FOREGROUND);

    INSERT (DEF_STYLE_ERROR,
            "#FF0000", NULL, FALSE, FALSE, TRUE, FALSE,
            MOO_TEXT_STYLE_FOREGROUND | MOO_TEXT_STYLE_UNDERLINE);

    INSERT (DEF_STYLE_OTHERS,
            "#008000", NULL, FALSE, FALSE, FALSE, FALSE,
            MOO_TEXT_STYLE_FOREGROUND);
}
#undef INSERT


void
_moo_text_style_scheme_apply (MooTextStyleScheme *scheme,
                              gpointer            view)
{
    GdkColor color;
    GdkColor *color_ptr;
    MooTextBuffer *buffer;

    g_return_if_fail (scheme != NULL);
    g_return_if_fail (MOO_IS_TEXT_VIEW (view));

    buffer = MOO_TEXT_BUFFER (gtk_text_view_get_buffer (view));

    gtk_widget_ensure_style (view);

    color_ptr = NULL;
    if (scheme->foreground)
    {
        if (gdk_color_parse (scheme->foreground, &color))
            color_ptr = &color;
        else
            g_warning ("%s: could not parse color '%s'", G_STRLOC, scheme->foreground);
    }
    gtk_widget_modify_text (view, GTK_STATE_NORMAL, color_ptr);
    gtk_widget_modify_text (view, GTK_STATE_ACTIVE, color_ptr);
    gtk_widget_modify_text (view, GTK_STATE_PRELIGHT, color_ptr);
    gtk_widget_modify_text (view, GTK_STATE_INSENSITIVE, color_ptr);
    moo_text_view_set_cursor_color (view, color_ptr);

    color_ptr = NULL;
    if (scheme->background)
    {
        if (gdk_color_parse (scheme->background, &color))
            color_ptr = &color;
        else
            g_warning ("%s: could not parse color '%s'", G_STRLOC, scheme->background);
    }
    gtk_widget_modify_base (view, GTK_STATE_NORMAL, color_ptr);
    gtk_widget_modify_base (view, GTK_STATE_ACTIVE, color_ptr);
    gtk_widget_modify_base (view, GTK_STATE_PRELIGHT, color_ptr);
    gtk_widget_modify_base (view, GTK_STATE_INSENSITIVE, color_ptr);

    color_ptr = NULL;
    if (scheme->selected_foreground)
    {
        if (gdk_color_parse (scheme->selected_foreground, &color))
            color_ptr = &color;
        else
            g_warning ("%s: could not parse color '%s'", G_STRLOC, scheme->selected_foreground);
    }
    gtk_widget_modify_text (view, GTK_STATE_SELECTED, color_ptr);

    color_ptr = NULL;
    if (scheme->selected_background)
    {
        if (gdk_color_parse (scheme->selected_background, &color))
            color_ptr = &color;
        else
            g_warning ("%s: could not parse color '%s'", G_STRLOC, scheme->selected_background);
    }
    gtk_widget_modify_base (view, GTK_STATE_SELECTED, color_ptr);

    color_ptr = NULL;
    if (scheme->current_line)
    {
        if (gdk_color_parse (scheme->current_line, &color))
            color_ptr = &color;
        else
            g_warning ("%s: could not parse color '%s'", G_STRLOC, scheme->current_line);
    }
    moo_text_view_set_current_line_color (view, color_ptr);
    moo_text_view_set_highlight_current_line (view, color_ptr != NULL);

    moo_text_buffer_set_bracket_match_style (buffer, scheme->bracket_match);
    moo_text_buffer_set_bracket_mismatch_style (buffer, scheme->bracket_mismatch);
}
