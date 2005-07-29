/*
 *   mooedit/mooeditlang.c
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define MOOEDIT_COMPILATION
#include "mooedit/mooeditlang-private.h"
#include "mooedit/mooeditprefs.h"
#include "mooutils/moocompat.h"
#include <string.h>


static void moo_edit_lang_class_init    (MooEditLangClass   *klass);

static void moo_edit_lang_init          (MooEditLang        *lang);
static void moo_edit_lang_finalize      (GObject            *object);

static void moo_edit_lang_write_style_setting
                                        (MooEditLang        *lang,
                                         const char         *style_id,
                                         const GtkSourceTagStyle *style);

/* frees GPtrArray */
static void free_tag_array              (GPtrArray          *array);
static void remove_tag                  (GPtrArray          *array,
                                         GObject            *tag);
static void remove_weak_ref             (const char         *style_id,
                                         GPtrArray          *tags_array);

static GSList *copy_string_slist        (const GSList       *list);
static void free_string_slist           (GSList             *list);

static void add_string_pair             (const char         *id,
                                         const char         *name,
                                         GPtrArray          *array);


/* MOO_TYPE_EDIT_LANG */
G_DEFINE_TYPE (MooEditLang, moo_edit_lang, G_TYPE_OBJECT)


static void moo_edit_lang_class_init    (MooEditLangClass   *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    gobject_class->finalize = moo_edit_lang_finalize;
}


static void moo_edit_lang_init          (MooEditLang        *lang)
{
    lang->priv = g_new0 (MooEditLangPrivate, 1);

    lang->priv->tag_id_to_style_id =
        g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
    lang->priv->style_id_to_tags =
        g_hash_table_new_full (g_str_hash, g_str_equal, g_free,
                               (GDestroyNotify)free_tag_array);
    lang->priv->style_id_to_style =
        g_hash_table_new_full (g_str_hash, g_str_equal, g_free,
                               (GDestroyNotify)gtk_source_tag_style_free);
    lang->priv->style_id_to_style_name =
        g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
}


static void moo_edit_lang_finalize      (GObject            *object)
{
    MooEditLang *lang = MOO_EDIT_LANG (object);

    g_free (lang->priv->id);
    g_free (lang->priv->name);
    g_free (lang->priv->section);
    g_free (lang->priv->author);
    g_free (lang->priv->filename);

    free_string_slist (lang->priv->mime_types);
    free_string_slist (lang->priv->extensions);

    g_free (lang->priv->brackets);
    g_free (lang->priv->word_chars);

    g_hash_table_destroy (lang->priv->tag_id_to_style_id);
    g_hash_table_destroy (lang->priv->style_id_to_style);
    g_hash_table_destroy (lang->priv->style_id_to_style_name);

    g_hash_table_foreach (lang->priv->style_id_to_tags,
                          (GHFunc) remove_weak_ref, NULL);
    g_hash_table_destroy (lang->priv->style_id_to_tags);

    g_slist_foreach (lang->priv->tags, (GFunc) g_object_unref, NULL);
    g_slist_free (lang->priv->tags);

    g_free (lang->priv);
    lang->priv = NULL;
}


MooEditLang         *moo_edit_lang_new              (const char     *id)
{
    MooEditLang *lang;

    g_return_val_if_fail (id != NULL, NULL);

    lang = MOO_EDIT_LANG (g_object_new (MOO_TYPE_EDIT_LANG, NULL));
    lang->priv->id = g_strdup (id);

    lang->priv->loaded = TRUE;

    return lang;
}


MooEditLang         *moo_edit_lang_new_from_file    (const char     *filename)
{
    MooEditLang *lang;

    g_return_val_if_fail (filename != NULL, NULL);

    lang = MOO_EDIT_LANG (g_object_new (MOO_TYPE_EDIT_LANG, NULL));
    lang->priv->filename = g_strdup (filename);
    if (!moo_edit_lang_load_description (lang)) {
        g_object_unref (lang);
        return NULL;
    }
    return lang;
}


const char          *moo_edit_lang_get_id           (MooEditLang    *lang)
{
    g_return_val_if_fail (MOO_IS_EDIT_LANG (lang), NULL);
    return lang->priv->id;
}

const char          *moo_edit_lang_get_name         (MooEditLang    *lang)
{
    g_return_val_if_fail (MOO_IS_EDIT_LANG (lang), NULL);
    return lang->priv->name;
}

const char          *moo_edit_lang_get_section      (MooEditLang    *lang)
{
    g_return_val_if_fail (MOO_IS_EDIT_LANG (lang), NULL);
    return lang->priv->section;
}


/* The list must be freed and the tags unref'ed */
GSList              *moo_edit_lang_get_tags         (MooEditLang    *lang)
{
    GSList *list = NULL, *l = NULL;

    g_return_val_if_fail (MOO_IS_EDIT_LANG (lang), NULL);

    if (!lang->priv->loaded) moo_edit_lang_load_full (lang);

    for (l = lang->priv->tags; l != NULL; l = l->next) {
        GtkTextTag *copy;
        char *style_id;
        char *tag_id;

        copy = gtk_source_tag_copy (GTK_SOURCE_TAG (l->data));
        list = g_slist_prepend (list, copy);

        tag_id = gtk_source_tag_get_id (GTK_SOURCE_TAG (l->data));
        style_id = g_hash_table_lookup (lang->priv->tag_id_to_style_id, tag_id);
        if (style_id) {
            GPtrArray *tags;
            GtkSourceTagStyle *style;

            tags = g_hash_table_lookup (lang->priv->style_id_to_tags, style_id);
            if (!tags) {
                tags = g_ptr_array_new ();
                g_hash_table_insert (lang->priv->style_id_to_tags,
                                     g_strdup (style_id), tags);
            }

            g_ptr_array_add (tags, copy);
            g_object_weak_ref (G_OBJECT (copy), (GWeakNotify) remove_tag, tags);

            style = g_hash_table_lookup (lang->priv->style_id_to_style, style_id);
            if (!style) g_critical ("no style with id '%s'", style_id);
            else gtk_source_tag_set_style (GTK_SOURCE_TAG (copy), style);
        }
        g_free (tag_id);
    }

    list = g_slist_reverse (list);
    return list;
}


GtkSourceTag        *moo_edit_lang_get_tag          (MooEditLang    *lang,
                                                     const char     *tag_id)
{
    GSList *l;
    GtkSourceTag *tag;

    g_return_val_if_fail (MOO_IS_EDIT_LANG (lang) && tag_id != NULL, NULL);

    if (!lang->priv->loaded) moo_edit_lang_load_full (lang);

    for (l = lang->priv->tags; l != NULL; l = l->next) {
        char *id;
        tag = GTK_SOURCE_TAG (l->data);
        id = gtk_source_tag_get_id (tag);
        if (!strcmp (id, tag_id)) {
            g_free (id);
            return tag;
        }
        g_free (id);
    }

    return NULL;
}


gunichar             moo_edit_lang_get_escape_char  (MooEditLang    *lang)
{
    g_return_val_if_fail (MOO_IS_EDIT_LANG (lang), 0);

    if (!lang->priv->loaded) moo_edit_lang_load_full (lang);

    return lang->priv->escape_char;
}


void                 moo_edit_lang_set_escape_char  (MooEditLang    *lang,
                                                     gunichar        escape_char)
{
    g_return_if_fail (MOO_IS_EDIT_LANG (lang));

    if (!lang->priv->loaded) moo_edit_lang_load_full (lang);

    lang->priv->escape_char = escape_char;
}


/* Should free the list (and free each string in it also). */
GSList              *moo_edit_lang_get_mime_types   (MooEditLang    *lang)
{
    g_return_val_if_fail (MOO_IS_EDIT_LANG (lang), NULL);

    if (!lang->priv->loaded) moo_edit_lang_load_full (lang);

    return copy_string_slist (lang->priv->mime_types);
}


void                 moo_edit_lang_set_mime_types   (MooEditLang    *lang,
                                                     const GSList   *mime_types)
{
    g_return_if_fail (MOO_IS_EDIT_LANG (lang));

    if (!lang->priv->loaded) moo_edit_lang_load_full (lang);

    free_string_slist (lang->priv->mime_types);
    lang->priv->mime_types = copy_string_slist (mime_types);
}


GSList              *moo_edit_lang_get_extensions   (MooEditLang    *lang)
{
    g_return_val_if_fail (MOO_IS_EDIT_LANG (lang), NULL);

    if (!lang->priv->loaded) moo_edit_lang_load_full (lang);

    return copy_string_slist (lang->priv->extensions);
}


void                 moo_edit_lang_set_extensions   (MooEditLang    *lang,
                                                     const GSList   *extensions)
{
    g_return_if_fail (MOO_IS_EDIT_LANG (lang));

    if (!lang->priv->loaded) moo_edit_lang_load_full (lang);

    free_string_slist (lang->priv->extensions);
    lang->priv->extensions = copy_string_slist (extensions);
}


/* Allocates new null-terminated array; even indexes are style ids, odd are style names */
GPtrArray           *moo_edit_lang_get_style_names  (MooEditLang    *lang)
{
    GPtrArray *result;

    g_return_val_if_fail (MOO_IS_EDIT_LANG (lang), NULL);

    if (!lang->priv->loaded) moo_edit_lang_load_full (lang);

    result = g_ptr_array_new ();
    g_hash_table_foreach (lang->priv->style_id_to_style_name,
                          (GHFunc) add_string_pair, result);
    g_ptr_array_add (result, NULL);

    return result;
}


const GtkSourceTagStyle *moo_edit_lang_get_style    (MooEditLang    *lang,
                                                     const char     *style_id)
{
    g_return_val_if_fail (MOO_IS_EDIT_LANG (lang) && style_id != NULL, NULL);

    if (!lang->priv->loaded) moo_edit_lang_load_full (lang);

    return g_hash_table_lookup (lang->priv->style_id_to_style, style_id);
}


void                 moo_edit_lang_set_style        (MooEditLang    *lang,
                                                     const char     *style_id,
                                                     const GtkSourceTagStyle *style)
{
    GPtrArray *tags;
    GSList *m;

    g_return_if_fail (MOO_IS_EDIT_LANG (lang));
    g_return_if_fail (style_id != NULL && style != NULL);

    if (!lang->priv->loaded) moo_edit_lang_load_full (lang);

    g_return_if_fail (g_hash_table_lookup (lang->priv->style_id_to_style, style_id) != NULL);

    moo_edit_lang_write_style_setting (lang, style_id, style);

    g_hash_table_insert (lang->priv->style_id_to_style,
                         g_strdup (style_id), gtk_source_tag_style_copy (style));

    tags = g_hash_table_lookup (lang->priv->style_id_to_tags, style_id);
    if (tags) {
        guint i;
        for (i = 0; i < tags->len; ++i)
            gtk_source_tag_set_style (GTK_SOURCE_TAG (g_ptr_array_index(tags, i)),
                                      style);
    }

    /* TODO TODO something to all this stuff */
    /* 'master copies' of tags are not stored in lang->priv->style_id_to_tags
        because of weak_ref issues */
    m = g_slist_find_custom (lang->priv->tags, style_id, (GCompareFunc) strcmp);
    g_return_if_fail (m != NULL);
    gtk_source_tag_set_style (GTK_SOURCE_TAG (m->data), style);
}


#ifndef USE_XML
gboolean    moo_edit_lang_load_description  (G_GNUC_UNUSED MooEditLang *lang)
{
    g_warning ("XML support disabled, can't load language description");
    return FALSE;
}


gboolean    moo_edit_lang_load_full         (MooEditLang *lang)
{
    g_return_val_if_fail (MOO_IS_EDIT_LANG (lang), FALSE);
    g_return_val_if_fail (!lang->priv->loaded, FALSE);

    lang->priv->loaded = TRUE;

    g_warning ("XML support disabled, can't load language");

    return FALSE;
}
#endif /* USE_XML */


static void free_tag_array              (GPtrArray          *array)
{
    g_ptr_array_free (array, TRUE);
}


static void remove_tag                  (GPtrArray          *array,
                                         GObject            *tag)
{
    g_return_if_fail (g_ptr_array_remove_fast (array, tag));
}


static void remove_weak_ref             (G_GNUC_UNUSED const char *style_id,
                                         GPtrArray          *tags_array)
{
    guint i;

    for (i = 0; i < tags_array->len; ++i)
        g_object_weak_unref (G_OBJECT (g_ptr_array_index (tags_array, i)),
                             (GWeakNotify) remove_tag, tags_array);
}


static GSList *copy_string_slist        (const GSList       *list)
{
    GSList *copy = NULL;
    const GSList *l;

    for (l = list; l != NULL; l = l->next)
        copy = g_slist_prepend (copy, g_strdup ((char*)l->data));
    copy = g_slist_reverse (copy);

    return copy;
}


static void free_string_slist           (GSList             *list)
{
    GSList *l;

    for (l = list; l != NULL; l = l->next)
        g_free (l->data);

    g_slist_free (list);
}


static void add_string_pair             (const char         *id,
                                         const char         *name,
                                         GPtrArray          *array)
{
    g_ptr_array_add (array, g_strdup (id));
    g_ptr_array_add (array, g_strdup (name));
}


/****************************************************************************/
/* Default styles                                                           */
/***/

typedef struct {
    char                *id;
    char                *name;
    GtkSourceTagStyle   *style;
} Style;

/* char* -> Style* */
static GHashTable *default_styles = NULL;


static void      create_default_styles  (void);
static Style    *new_style              (const char         *id,
                                         const char         *name,
                                         GtkSourceTagStyle  *style);
static GtkSourceTagStyle *new_tag_style (const char         *fg,
                                         const char         *bg,
                                         gboolean            italic,
                                         gboolean            bold,
                                         gboolean            underline,
                                         gboolean            strikethrough);
static void      free_style             (Style *s);
static void      add_id_name_pair       (const char         *id,
                                         const Style        *style,
                                         GPtrArray          *array);


static Style    *new_style              (const char         *id,
                                         const char         *name,
                                         GtkSourceTagStyle *style)
{
    Style *s = g_new (Style, 1);
    s->id = g_strdup (id);
    s->name = g_strdup (name);
    s->style = style;
    return s;
}

static GtkSourceTagStyle *new_tag_style (const char *fg,
                                         const char *bg,
                                         gboolean    italic,
                                         gboolean    bold,
                                         gboolean    underline,
                                         gboolean    strikethrough)
{
    GtkSourceTagStyle *s = gtk_source_tag_style_new ();
    GdkColor color;

    s->is_default = TRUE;
    s->italic = italic;
    s->bold = bold;
    s->underline = underline;
    s->strikethrough = strikethrough;

    s->mask = 0;

    if (fg) {
        if (!gdk_color_parse (fg, &color))
            g_warning ("could not parse color %s", fg);
        else {
            s->foreground = color;
            s->mask |= GTK_SOURCE_TAG_STYLE_USE_FOREGROUND;
        }
    }
    if (bg) {
        if (!gdk_color_parse (bg, &color))
            g_warning ("could not parse color %s", bg);
        else {
            s->background = color;
            s->mask |= GTK_SOURCE_TAG_STYLE_USE_BACKGROUND;
        }
    }

    return s;
}

static void      free_style             (Style *s)
{
    g_free (s->id);
    g_free (s->name);
    gtk_source_tag_style_free (s->style);
    g_free (s);
}

static void      add_id_name_pair       (const char         *id,
                                         const Style        *style,
                                         GPtrArray          *array)
{
    g_ptr_array_add (array, g_strdup (id));
    g_ptr_array_add (array, g_strdup (style->name));
}


static void create_default_styles       (void)
{
    if (default_styles) return;

    default_styles = g_hash_table_new_full (g_str_hash,
                                            g_str_equal,
                                            g_free,
                                            (GDestroyNotify)free_style);

    g_hash_table_insert (default_styles,
                         g_strdup (MOO_EDIT_STYLE_BASE_N),
                         new_style (MOO_EDIT_STYLE_BASE_N,
                                    "Base-N Integer",
                                    new_tag_style ("#008080", NULL, FALSE, FALSE, FALSE, FALSE)));

    g_hash_table_insert (default_styles,
                         g_strdup (MOO_EDIT_STYLE_KEYWORD),
                         new_style (MOO_EDIT_STYLE_KEYWORD,
                                    "Keyword",
                                    new_tag_style (NULL, NULL, FALSE, TRUE, FALSE, FALSE)));

    g_hash_table_insert (default_styles,
                         g_strdup (MOO_EDIT_STYLE_DATA_TYPE),
                         new_style (MOO_EDIT_STYLE_DATA_TYPE,
                                    "Data Type",
                                    new_tag_style ("#800000", NULL, FALSE, FALSE, FALSE, FALSE)));

    g_hash_table_insert (default_styles,
                         g_strdup (MOO_EDIT_STYLE_DECIMAL),
                         new_style (MOO_EDIT_STYLE_DECIMAL,
                                    "Decimal",
                                    new_tag_style ("#0000FF", NULL, FALSE, FALSE, FALSE, FALSE)));

    g_hash_table_insert (default_styles,
                         g_strdup (MOO_EDIT_STYLE_FLOAT),
                         new_style (MOO_EDIT_STYLE_FLOAT,
                                    "Float",
                                    new_tag_style ("#800080", NULL, FALSE, FALSE, FALSE, FALSE)));

    g_hash_table_insert (default_styles,
                         g_strdup (MOO_EDIT_STYLE_CHAR),
                         new_style (MOO_EDIT_STYLE_CHAR,
                                    "Char",
                                    new_tag_style ("#FF00FF", NULL, FALSE, FALSE, FALSE, FALSE)));

    g_hash_table_insert (default_styles,
                         g_strdup (MOO_EDIT_STYLE_STRING),
                         new_style (MOO_EDIT_STYLE_STRING,
                                    "String",
                                    new_tag_style ("#FF0000", NULL, FALSE, FALSE, FALSE, FALSE)));

    g_hash_table_insert (default_styles,
                         g_strdup (MOO_EDIT_STYLE_COMMENT),
                         new_style (MOO_EDIT_STYLE_COMMENT,
                                    "Comment",
                                    new_tag_style ("#808080", NULL, TRUE, FALSE, FALSE, FALSE)));

    g_hash_table_insert (default_styles,
                         g_strdup (MOO_EDIT_STYLE_OTHERS),
                         new_style (MOO_EDIT_STYLE_OTHERS,
                                    "Others",
                                    new_tag_style ("#008000", NULL, FALSE, FALSE, FALSE, FALSE)));

    g_hash_table_insert (default_styles,
                         g_strdup (MOO_EDIT_STYLE_ALERT),
                         new_style (MOO_EDIT_STYLE_ALERT,
                                    "Alert",
                                    new_tag_style ("#FFFFFF", "#FF0000", FALSE, TRUE, FALSE, FALSE)));

    g_hash_table_insert (default_styles,
                         g_strdup (MOO_EDIT_STYLE_FUNCTION),
                         new_style (MOO_EDIT_STYLE_FUNCTION,
                                    "Function",
                                    new_tag_style ("#000080", NULL, FALSE, FALSE, FALSE, FALSE)));

    g_hash_table_insert (default_styles,
                         g_strdup (MOO_EDIT_STYLE_ERROR),
                         new_style (MOO_EDIT_STYLE_ERROR,
                                    "Error",
                                    new_tag_style ("#FF0000", NULL, FALSE, FALSE, TRUE, FALSE)));
}


/* Allocates new null-terminated array; even indexes are style ids, odd are style names */
GPtrArray           *moo_edit_lang_get_default_style_names (void)
{
    GPtrArray *array;

    create_default_styles ();

    array = g_ptr_array_new ();
    g_hash_table_foreach (default_styles, (GHFunc) add_id_name_pair, array);
    g_ptr_array_add (array, NULL);

    return array;
}


GtkSourceTagStyle   *moo_edit_lang_get_default_style    (const char *style_id)
{
    Style *s;

    g_return_val_if_fail (style_id != NULL, NULL);

    create_default_styles ();

    s = g_hash_table_lookup (default_styles, style_id);
    if (s)
        return gtk_source_tag_style_copy (s->style);
    else
        return NULL;
}


void        moo_edit_style_load             (const char     *lang_id,
                                             const char     *style_id,
                                             GtkSourceTagStyle *style)
{
    const char *s;
    const GdkColor *color;

    g_return_if_fail (lang_id != NULL && style_id != NULL && style != NULL);

    create_default_styles ();

    s = moo_edit_style_setting (lang_id, style_id, MOO_EDIT_PREFS_FOREGROUND);
    color = moo_prefs_get_color (s);
    if (color) {
        style->foreground = *color;
        style->mask |= GTK_SOURCE_TAG_STYLE_USE_FOREGROUND;
    }

    s = moo_edit_style_setting (lang_id, style_id, MOO_EDIT_PREFS_BACKGROUND);
    color = moo_prefs_get_color (s);
    if (color) {
        style->background = *color;
        style->mask |= GTK_SOURCE_TAG_STYLE_USE_BACKGROUND;
    }

    s = moo_edit_style_setting (lang_id, style_id, MOO_EDIT_PREFS_BOLD);
    if (moo_prefs_get (s))
        style->bold = moo_prefs_get_bool (s);

    s = moo_edit_style_setting (lang_id, style_id, MOO_EDIT_PREFS_ITALIC);
    if (moo_prefs_get (s))
        style->italic = moo_prefs_get_bool (s);

    s = moo_edit_style_setting (lang_id, style_id, MOO_EDIT_PREFS_UNDERLINE);
    if (moo_prefs_get (s))
        style->underline = moo_prefs_get_bool (s);

    s = moo_edit_style_setting (lang_id, style_id, MOO_EDIT_PREFS_STRIKETHROUGH);
    if (moo_prefs_get (s))
        style->strikethrough = moo_prefs_get_bool (s);
}


const char *moo_edit_style_setting          (const char     *lang_id,
                                             const char     *style_id,
                                             const char     *setting)
{
    static char *s = NULL;

    g_return_val_if_fail (lang_id != NULL && style_id != NULL && setting != NULL, NULL);

    g_free (s);
    return s = g_strdup_printf (MOO_EDIT_PREFS_PREFIX "/syntax/%s/%s/%s", lang_id, style_id, setting);
}


#define color_differ(c1,c2) \
    ((c1)->red != (c2)->red || \
     (c1)->green != (c2)->green || \
     (c1)->blue != (c2)->blue)

static void moo_edit_lang_write_style_setting
                                        (MooEditLang        *lang,
                                         const char         *style_id,
                                         const GtkSourceTagStyle *style)
{
    const char *lang_id;
    const char *setting;
    const GdkColor *color;
    const GtkSourceTagStyle *old_style;

    old_style = g_hash_table_lookup (lang->priv->style_id_to_style, style_id);
    g_return_if_fail (old_style != NULL);

    lang_id = lang->priv->id;

    if ((old_style->mask & GTK_SOURCE_TAG_STYLE_USE_BACKGROUND) !=
        (style->mask & GTK_SOURCE_TAG_STYLE_USE_BACKGROUND) ||
        ((old_style->mask & GTK_SOURCE_TAG_STYLE_USE_BACKGROUND) &&
         color_differ (&(old_style->background), &(style->background))))
    {
        setting = moo_edit_style_setting (lang_id, style_id, MOO_EDIT_PREFS_BACKGROUND);
        if (style->mask & GTK_SOURCE_TAG_STYLE_USE_BACKGROUND) {
            color = moo_prefs_get_color (setting);
            if (!color || color_differ (color, &(style->background)))
                moo_prefs_set_color (setting, &(style->background));
        }
        else if (moo_prefs_get (setting))
            moo_prefs_set (setting, NULL);
    }

    if ((old_style->mask & GTK_SOURCE_TAG_STYLE_USE_FOREGROUND) !=
        (style->mask & GTK_SOURCE_TAG_STYLE_USE_FOREGROUND) ||
        ((old_style->mask & GTK_SOURCE_TAG_STYLE_USE_FOREGROUND) &&
         color_differ (&(old_style->foreground), &(style->foreground))))
    {
        setting = moo_edit_style_setting (lang_id, style_id, MOO_EDIT_PREFS_FOREGROUND);
        if (style->mask & GTK_SOURCE_TAG_STYLE_USE_FOREGROUND) {
            color = moo_prefs_get_color (setting);
            if (!color || color_differ (color, &(style->foreground)))
                moo_prefs_set_color (setting, &(style->foreground));
        }
        else if (moo_prefs_get (setting))
            moo_prefs_set (setting, NULL);
    }

    if (old_style->bold != style->bold) {
        setting = moo_edit_style_setting (lang_id, style_id, MOO_EDIT_PREFS_BOLD);
        if (moo_prefs_get_bool (setting) != style->bold)
            moo_prefs_set_bool (setting, style->bold);
    }

    if (old_style->italic != style->italic) {
        setting = moo_edit_style_setting (lang_id, style_id, MOO_EDIT_PREFS_ITALIC);
        if (moo_prefs_get_bool (setting) != style->italic)
            moo_prefs_set_bool (setting, style->italic);
    }

    if (old_style->underline != style->underline) {
        setting = moo_edit_style_setting (lang_id, style_id, MOO_EDIT_PREFS_UNDERLINE);
        if (moo_prefs_get_bool (setting) != style->underline)
            moo_prefs_set_bool (setting, style->underline);
    }

    if (old_style->strikethrough != style->strikethrough) {
        setting = moo_edit_style_setting (lang_id, style_id, MOO_EDIT_PREFS_STRIKETHROUGH);
        if (moo_prefs_get_bool (setting) != style->strikethrough)
            moo_prefs_set_bool (setting, style->strikethrough);
    }
}


const char          *moo_edit_lang_get_brackets     (MooEditLang    *lang)
{
    g_return_val_if_fail (MOO_IS_EDIT_LANG (lang), NULL);
    return lang->priv->brackets;
}
