/*
 *   moolang.c
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
#include "config.h"
#include "mooedit/moolang-private.h"
#include "mooutils/mooi18n.h"
#include "gtksourceview/gtksourcelanguage-private.h"
#include <libxml/xmlreader.h>
#include <glib/gstdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif


struct _MooLangPrivate {
    char *version;      /* not NULL; "" by default */
    char *author;       /* not NULL; "" by default */

    char *brackets;
    char *line_comment;
    char *block_comment_start;
    char *block_comment_end;

    guint hidden : 1;
};


G_DEFINE_TYPE (MooLang, moo_lang, GTK_TYPE_SOURCE_LANGUAGE)


static void
moo_lang_init (MooLang *lang)
{
    lang->priv = G_TYPE_INSTANCE_GET_PRIVATE (lang, MOO_TYPE_LANG, MooLangPrivate);
    lang->priv->version = g_strdup ("");
    lang->priv->author = g_strdup ("");
}


static void
moo_lang_finalize (GObject *object)
{
    MooLang *lang = MOO_LANG (object);

    g_free (lang->priv->version);
    g_free (lang->priv->author);
    g_free (lang->priv->brackets);
    g_free (lang->priv->line_comment);
    g_free (lang->priv->block_comment_start);
    g_free (lang->priv->block_comment_end);

    G_OBJECT_CLASS (moo_lang_parent_class)->finalize (object);
}


static void
moo_lang_class_init (MooLangClass *klass)
{
    G_OBJECT_CLASS (klass)->finalize = moo_lang_finalize;
    g_type_class_add_private (klass, sizeof (MooLangPrivate));
}


GtkSourceEngine *
_moo_lang_get_engine (MooLang *lang)
{
    g_return_val_if_fail (MOO_IS_LANG (lang), NULL);
    return _gtk_source_language_create_engine (GTK_SOURCE_LANGUAGE (lang));
}


MooLangMgr *
_moo_lang_get_mgr (MooLang *lang)
{
    g_return_val_if_fail (MOO_IS_LANG (lang), NULL);
    return MOO_LANG_MGR (_gtk_source_language_get_languages_manager (GTK_SOURCE_LANGUAGE (lang)));
}


const char *
_moo_lang_id (MooLang *lang)
{
    g_return_val_if_fail (!lang || MOO_IS_LANG (lang), NULL);

    if (lang)
        return GTK_SOURCE_LANGUAGE(lang)->priv->id;
    else
        return MOO_LANG_NONE;
}


const char *
_moo_lang_display_name (MooLang *lang)
{
    g_return_val_if_fail (!lang || MOO_IS_LANG (lang), NULL);

    if (lang)
        return GTK_SOURCE_LANGUAGE(lang)->priv->name;
    else
        return _(MOO_LANG_NONE_NAME);
}


const char *
_moo_lang_get_line_comment (MooLang *lang)
{
    g_return_val_if_fail (MOO_IS_LANG (lang), NULL);
    return g_hash_table_lookup (GTK_SOURCE_LANGUAGE(lang)->priv->properties, "line-comment-start");
}


const char *
_moo_lang_get_block_comment_start (MooLang *lang)
{
    g_return_val_if_fail (MOO_IS_LANG (lang), NULL);
    return g_hash_table_lookup (GTK_SOURCE_LANGUAGE(lang)->priv->properties, "block-comment-start");
}


const char *
_moo_lang_get_block_comment_end (MooLang *lang)
{
    g_return_val_if_fail (MOO_IS_LANG (lang), NULL);
    return g_hash_table_lookup (GTK_SOURCE_LANGUAGE(lang)->priv->properties, "block-comment-end");
}


const char *
_moo_lang_get_brackets (MooLang *lang)
{
    g_return_val_if_fail (MOO_IS_LANG (lang), NULL);
    return NULL;
}


const char *
_moo_lang_get_section (MooLang *lang)
{
    g_return_val_if_fail (MOO_IS_LANG (lang), NULL);
    return GTK_SOURCE_LANGUAGE(lang)->priv->section;
}


gboolean
_moo_lang_get_hidden (MooLang *lang)
{
    g_return_val_if_fail (MOO_IS_LANG (lang), TRUE);
    return GTK_SOURCE_LANGUAGE(lang)->priv->hidden != 0;
}


char *
_moo_lang_id_from_name (const char *whatever)
{
    if (!whatever || !g_ascii_strcasecmp (whatever, MOO_LANG_NONE))
        return g_strdup (MOO_LANG_NONE);
    else
        return g_strstrip (g_ascii_strdown (whatever, -1));
}
