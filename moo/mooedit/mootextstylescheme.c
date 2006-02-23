/*
 *   mootextstylescheme.c
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

#define MOOEDIT_COMPILATION
#include "mooedit/mootextstylescheme.h"
#include "mooedit/moolang-strings.h"
#include "mooedit/mootextview.h"
#include "mooedit/mootextbuffer.h"
#include "mooutils/moomarshals.h"
#include <string.h>


static void moo_text_style_scheme_finalize  (GObject            *object);
static void fill_in_default_scheme          (MooTextStyleScheme *scheme);


/* MOO_TYPE_TEXT_STYLE_SCHEME */
G_DEFINE_TYPE (MooTextStyleScheme, moo_text_style_scheme, G_TYPE_OBJECT)

enum {
    CHANGED,
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];

static void
moo_text_style_scheme_class_init (MooTextStyleSchemeClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = moo_text_style_scheme_finalize;

    signals[CHANGED] =
            g_signal_new ("changed",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooTextStyleSchemeClass, changed),
                          NULL, NULL,
                          _moo_marshal_VOID__VOID,
                          G_TYPE_NONE,0);
}


static void
moo_text_style_scheme_init (MooTextStyleScheme *scheme)
{
    scheme->base_scheme = NULL;
    scheme->name = NULL;
    memset (scheme->text_colors, 0, MOO_TEXT_NUM_COLORS * sizeof(char*));
    scheme->bracket_match = NULL;
    scheme->bracket_mismatch = NULL;
    scheme->styles = g_hash_table_new_full (g_str_hash, g_str_equal,
                                            g_free,
                                            (GDestroyNotify) moo_text_style_free);
}


static void
moo_text_style_scheme_finalize (GObject *object)
{
    guint i;
    MooTextStyleScheme *scheme = MOO_TEXT_STYLE_SCHEME (object);

    if (scheme->base_scheme)
        g_object_unref (scheme->base_scheme);

    g_free (scheme->name);

    for (i = 0; i < MOO_TEXT_NUM_COLORS; ++i)
        g_free (scheme->text_colors[i]);

    g_hash_table_destroy (scheme->styles);
    moo_text_style_free (scheme->bracket_match);
    moo_text_style_free (scheme->bracket_mismatch);

    G_OBJECT_CLASS(moo_text_style_scheme_parent_class)->finalize (object);
}


MooTextStyleScheme*
moo_text_style_scheme_new_empty (const char         *name,
                                 MooTextStyleScheme *base)
{
    MooTextStyleScheme *scheme;

    g_return_val_if_fail (name && name[0], NULL);

    scheme = g_object_new (MOO_TYPE_TEXT_STYLE_SCHEME, NULL);

    scheme->base_scheme = base ? g_object_ref (base) : NULL;
    scheme->name = g_strdup (name);

    return scheme;
}


MooTextStyleScheme*
moo_text_style_scheme_new_default (void)
{
    MooTextStyleScheme *scheme;
    scheme = moo_text_style_scheme_new_empty (SCHEME_DEFAULT, NULL);
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
    guint i;

    g_return_val_if_fail (scheme != NULL, NULL);

    copy = moo_text_style_scheme_new_empty (scheme->name, scheme->base_scheme);

    for (i = 0; i < MOO_TEXT_NUM_COLORS; ++i)
        copy->text_colors[i] = g_strdup (scheme->text_colors[i]);

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
    scheme->text_colors[MOO_TEXT_COLOR_CUR_LINE] = g_strdup ("#EEF6FF");
    scheme->bracket_match = new_style (NULL, "#FFFF99",
                                       FALSE, TRUE, FALSE, FALSE,
                                       MOO_TEXT_STYLE_BACKGROUND |
                                               MOO_TEXT_STYLE_BOLD);

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
_moo_text_style_scheme_get_color (MooTextStyleScheme *scheme,
                                  MooTextColor        color,
                                  GdkColor           *dest,
                                  GdkScreen          *screen)
{
    GtkSettings *settings;
    GtkStyle *style;

    g_return_if_fail (scheme != NULL);
    g_return_if_fail (dest != NULL);
    g_return_if_fail (GDK_IS_SCREEN (screen));
    g_return_if_fail (color < MOO_TEXT_NUM_COLORS);

    if (scheme->text_colors[color])
    {
        if (gdk_color_parse (scheme->text_colors[color], dest))
            return;
        else
            g_warning ("%s: could not parse color '%s'", G_STRLOC,
                       scheme->text_colors[color]);
    }

    settings = gtk_settings_get_for_screen (screen);

    style = gtk_rc_get_style_by_paths (settings, NULL, NULL, MOO_TYPE_TEXT_VIEW);

    if (!style)
        style = gtk_style_new ();
    else
        g_object_ref (style);

    switch (color)
    {
        case MOO_TEXT_COLOR_FG:
            *dest = style->text[GTK_STATE_NORMAL];
            break;

        case MOO_TEXT_COLOR_BG:
        case MOO_TEXT_COLOR_CUR_LINE:
            *dest = style->base[GTK_STATE_NORMAL];
            break;

        case MOO_TEXT_COLOR_SEL_FG:
            *dest = style->text[GTK_STATE_SELECTED];
            break;

        case MOO_TEXT_COLOR_SEL_BG:
            *dest = style->base[GTK_STATE_SELECTED];
            break;

        case MOO_TEXT_NUM_COLORS:
            g_assert_not_reached ();
    }

    g_object_unref (style);
}


static void
prepend_default_style (const char *style_name,
                       G_GNUC_UNUSED gpointer whatever,
                       GSList **list)
{
    if (!strstr (style_name, "::"))
        *list = g_slist_prepend (*list, g_strdup (style_name));
}

GSList*
moo_text_style_scheme_list_default (MooTextStyleScheme *scheme)
{
    GSList *list = NULL;
    g_return_val_if_fail (MOO_IS_TEXT_STYLE_SCHEME (scheme), NULL);
    g_hash_table_foreach (scheme->styles, (GHFunc) prepend_default_style, &list);
    return g_slist_reverse (list);
}
