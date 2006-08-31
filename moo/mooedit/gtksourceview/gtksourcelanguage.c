/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*-
 *  gtksourcelanguage.c
 *
 *  Copyright (C) 2003 - Paolo Maggi <paolo.maggi@polito.it>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Library General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <string.h>
#include <fcntl.h>

#include <libxml/xmlreader.h>
#include <glib/gstdio.h>
#include "gtksourceview-i18n.h"
#include "gtksourcelanguage-private.h"
#include "gtksourcelanguage.h"
#include "gtksourceview-marshal.h"

#define DEFAULT_SECTION _("Others")

G_DEFINE_TYPE (GtkSourceLanguage, gtk_source_language, G_TYPE_OBJECT)


static void		  gtk_source_language_finalize 		(GObject 			*object);

static GtkSourceLanguage *process_language_node 		(xmlTextReaderPtr 		 reader,
								 const gchar 			*filename);

/* Signals */
enum {
	LAST_SIGNAL
};

// static guint 	 signals[LAST_SIGNAL] = { 0 };


static GSList *
slist_deep_copy (GSList *list)
{
	GSList *l, *copy = NULL;

	for (l = list; l != NULL; l = l->next)
		copy = g_slist_prepend (copy, g_strdup (l->data));

	return g_slist_reverse (copy);
}

static void
slist_deep_free (GSList *list)
{
	g_slist_foreach (list, (GFunc) g_free, NULL);
	g_slist_free (list);
}

GtkSourceLanguage *
_gtk_source_language_new_from_file (const gchar			*filename,
				    GtkSourceLanguagesManager	*lm)
{
	GtkSourceLanguage *lang = NULL;
	xmlTextReaderPtr reader = NULL;
	gint ret;
	gint fd;

	g_return_val_if_fail (filename != NULL, NULL);
	g_return_val_if_fail (lm != NULL, NULL);

	/*
	 * Use fd instead of filename so that it's utf8 safe on w32.
	 */
	fd = g_open (filename, O_RDONLY, 0);
	if (fd != -1)
		reader = xmlReaderForFd (fd, filename, NULL, 0);

	if (reader != NULL)
	{
        	ret = xmlTextReaderRead (reader);

        	while (ret == 1)
		{
			if (xmlTextReaderNodeType (reader) == 1)
			{
				xmlChar *name;

				name = xmlTextReaderName (reader);

				if (xmlStrcmp (name, BAD_CAST "language") == 0)
				{
					lang = process_language_node (reader, filename);
					ret = 0;
				}

				xmlFree (name);
			}

			if (ret == 1)
				ret = xmlTextReaderRead (reader);
		}

		xmlFreeTextReader (reader);
		close (fd);

		if (ret != 0)
		{
	            g_warning("Failed to parse '%s'", filename);
		    return NULL;
		}
        }
	else
	{
		g_warning("Unable to open '%s'", filename);

    	}

	if (lang != NULL)
		lang->priv->languages_manager = lm;

	return lang;
}

static void
gtk_source_language_class_init (GtkSourceLanguageClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	object_class->finalize	= gtk_source_language_finalize;
}

static void
gtk_source_language_init (GtkSourceLanguage *lang)
{
	lang->priv = g_new0 (GtkSourceLanguagePrivate, 1);
	lang->priv->styles = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
}

static void
gtk_source_language_finalize (GObject *object)
{
	GtkSourceLanguage *lang;

	lang = GTK_SOURCE_LANGUAGE (object);

	if (lang->priv != NULL)
	{
		g_free (lang->priv->lang_file_name);
		g_free (lang->priv->translation_domain);
		g_free (lang->priv->name);
		g_free (lang->priv->section);
		g_free (lang->priv->id);
		slist_deep_free (lang->priv->mime_types);
		slist_deep_free (lang->priv->globs);
		g_hash_table_destroy (lang->priv->styles);
		g_free (lang->priv->line_comment);
		g_free (lang->priv->block_comment_start);
		g_free (lang->priv->block_comment_end);
		g_free (lang->priv->brackets);
		g_free (lang->priv);
		lang->priv = NULL;
	}

	G_OBJECT_CLASS (gtk_source_language_parent_class)->finalize (object);
}

static GSList *
parse_mime_types_or_globs (xmlTextReaderPtr reader,
			   const char      *attr_name)
{
	xmlChar *attr;
	gchar **mtl;
	gint i;
	GSList *list = NULL;

	attr = xmlTextReaderGetAttribute (reader, BAD_CAST attr_name);
	if (attr == NULL)
		return NULL;

	mtl = g_strsplit_set ((gchar*) attr, ";,", 0);

	for (i = 0; mtl[i] != NULL; i++)
		/* steal the strings from the array */
		list = g_slist_prepend (list, mtl[i]);

	g_free (mtl);
	xmlFree (attr);

	return g_slist_reverse (list);
}

static gboolean
string_to_bool (const gchar *string)
{
	if (!g_ascii_strcasecmp (string, "yes") ||
	    !g_ascii_strcasecmp (string, "true") ||
	    !g_ascii_strcasecmp (string, "1"))
		return TRUE;
	else if (!g_ascii_strcasecmp (string, "no") ||
		 !g_ascii_strcasecmp (string, "false") ||
		 !g_ascii_strcasecmp (string, "0"))
		return FALSE;
	else
		g_return_val_if_reached (FALSE);
}

static void
process_brackets_node (xmlTextReaderPtr   reader,
		       GtkSourceLanguage *language)
{
	xmlNode *node;

	node = xmlTextReaderCurrentNode (reader);
	g_return_if_fail (node != NULL);
	g_return_if_fail (node->children != NULL);

	language->priv->brackets = g_strdup ((gchar*) node->children->content);
}

static gboolean
get_attribute (xmlTextReaderPtr   reader,
	       const gchar       *element,
	       const gchar       *attribute,
	       gchar            **dest)
{
	xmlChar *tmp = xmlTextReaderGetAttribute (reader, BAD_CAST attribute);

	if (!tmp)
	{
		g_warning ("missing %s attribute in %s element",
			   attribute, element);
		return FALSE;
	}
	else
	{
		*dest = g_strdup ((gchar*) tmp);
		xmlFree (tmp);
		return TRUE;
	}
}

static void
process_brackets_and_comments (xmlTextReaderPtr   reader,
			       GtkSourceLanguage *language)
{
	gint ret;
	gboolean brackets_done = FALSE;
	gboolean line_comment_done = FALSE;
	gboolean block_comment_done = FALSE;

	ret = xmlTextReaderRead (reader);

	while (ret == 1)
	{
		if (xmlTextReaderNodeType (reader) == 1)
		{
			xmlChar *name;

			name = xmlTextReaderName (reader);

			if (!xmlStrcmp (name, BAD_CAST "brackets"))
			{
				if (brackets_done)
				{
					g_warning ("duplicated %s element", name);
					ret = 0;
				}
				else
				{
					process_brackets_node (reader, language);
					brackets_done = TRUE;
				}
			}
			else if (!xmlStrcmp (name, BAD_CAST "line-comment"))
			{
				if (line_comment_done)
				{
					g_warning ("duplicated %s element", name);
					ret = 0;
				}
				else
				{
					if (!get_attribute (reader, "line-comment", "start",
							    &language->priv->line_comment))
						ret = 0;
					else
						line_comment_done = TRUE;
				}
			}
			else if (!xmlStrcmp (name, BAD_CAST "block-comment"))
			{
				if (block_comment_done)
				{
					g_warning ("duplicated %s element", name);
					ret = 0;
				}
				else
				{
					if (!get_attribute (reader, "block-comment", "start",
							    &language->priv->block_comment_start))
						ret = 0;
					else if (!get_attribute (reader, "block-comment", "end",
								 &language->priv->block_comment_end))
						ret = 0;
					else
						block_comment_done = TRUE;
				}
			}

			if (brackets_done && line_comment_done && block_comment_done)
				ret = 0;

			xmlFree (name);
		}

		if (ret == 1)
			ret = xmlTextReaderRead (reader);
	}
}

static GtkSourceLanguage *
process_language_node (xmlTextReaderPtr reader, const gchar *filename)
{
	xmlChar *version;
	xmlChar *tmp;
	xmlChar *untranslated_name;
	GtkSourceLanguage *lang;

	lang = g_object_new (GTK_TYPE_SOURCE_LANGUAGE, NULL);

	lang->priv->lang_file_name = g_strdup (filename);

	tmp = xmlTextReaderGetAttribute (reader, BAD_CAST "translation-domain");
	if (tmp != NULL)
		lang->priv->translation_domain = g_strdup ((gchar*) tmp);
	else
		lang->priv->translation_domain = g_strdup (GETTEXT_PACKAGE);
	xmlFree (tmp);

	tmp = xmlTextReaderGetAttribute (reader, BAD_CAST "hidden");
	if (tmp != NULL)
		lang->priv->hidden = string_to_bool ((gchar*) tmp);
	else
		lang->priv->hidden = FALSE;
	xmlFree (tmp);

	tmp = xmlTextReaderGetAttribute (reader, BAD_CAST "_name");
	if (tmp == NULL)
	{
		tmp = xmlTextReaderGetAttribute (reader, BAD_CAST "name");

		if (tmp == NULL)
		{
			g_warning ("Impossible to get language name from file '%s'",
				   filename);
			g_object_unref (lang);
			return NULL;
		}

		lang->priv->name = g_strdup ((char*) tmp);
		untranslated_name = tmp;
	}
	else
	{
		lang->priv->name = g_strdup (dgettext (lang->priv->translation_domain, (gchar*) tmp));
		untranslated_name = tmp;
	}

	tmp = xmlTextReaderGetAttribute (reader, BAD_CAST "id");
	if (tmp != NULL)
	{
		lang->priv->id = g_ascii_strdown ((gchar*) tmp, -1);
	}
	else
	{
		lang->priv->id = g_ascii_strdown ((gchar*) untranslated_name, -1);
	}
	xmlFree (tmp);
	xmlFree (untranslated_name);

	tmp = xmlTextReaderGetAttribute (reader, BAD_CAST "_section");
	if (tmp == NULL)
	{
		tmp = xmlTextReaderGetAttribute (reader, BAD_CAST "section");

		if (tmp == NULL)
			lang->priv->section = g_strdup (DEFAULT_SECTION);
		else
			lang->priv->section = g_strdup ((char*) tmp);

		xmlFree (tmp);
	}
	else
	{
		lang->priv->section = g_strdup (dgettext (lang->priv->translation_domain, (gchar*) tmp));
		xmlFree (tmp);
	}

	version = xmlTextReaderGetAttribute (reader, BAD_CAST "version");

	if (version == NULL)
	{
		g_warning ("Impossible to get version number from file '%s'",
			   filename);
		g_object_unref (lang);
		return NULL;
	}

	if (xmlStrcmp (version , BAD_CAST "1.0") == 0)
	{
		lang->priv->version = GTK_SOURCE_LANGUAGE_VERSION_1_0;
	}
	else if (xmlStrcmp (version, BAD_CAST "2.0") == 0)
	{
		lang->priv->version = GTK_SOURCE_LANGUAGE_VERSION_2_0;
	}
	else
	{
		g_warning ("Usupported language spec version '%s' in file '%s'",
			   (gchar*) version, filename);
		xmlFree (version);
		g_object_unref (lang);
		return NULL;
	}

	xmlFree (version);

	lang->priv->mime_types = parse_mime_types_or_globs (reader, "mimetypes");
	lang->priv->globs = parse_mime_types_or_globs (reader, "globs");

	if (lang->priv->version == GTK_SOURCE_LANGUAGE_VERSION_2_0)
		process_brackets_and_comments (reader, lang);

	return lang;
}

/**
 * gtk_source_language_get_id:
 * @language: a #GtkSourceLanguage.
 *
 * Returns the ID of the language. The ID is not locale-dependent.
 *
 * Return value: the ID of @language, it must be freed it with g_free.
 **/
gchar *
gtk_source_language_get_id (GtkSourceLanguage *language)
{
	g_return_val_if_fail (GTK_IS_SOURCE_LANGUAGE (language), NULL);
	g_return_val_if_fail (language->priv->id != NULL, NULL);

	return g_strdup (language->priv->id);
}

/**
 * gtk_source_language_get_name:
 * @language: a #GtkSourceLanguage.
 *
 * Returns the localized name of the language.
 *
 * Return value: the name of @language.
 **/
gchar *
gtk_source_language_get_name (GtkSourceLanguage *language)
{
	g_return_val_if_fail (GTK_IS_SOURCE_LANGUAGE (language), NULL);
	g_return_val_if_fail (language->priv->name != NULL, NULL);

	return g_strdup (language->priv->name);
}

/**
 * gtk_source_language_get_section:
 * @language: a #GtkSourceLanguage.
 *
 * Returns the localized section of the language.
 * Each language belong to a section (ex. HTML belogs to the
 * Markup section).
 *
 * Return value: the section of @language.
 **/
gchar *
gtk_source_language_get_section	(GtkSourceLanguage *language)
{
	g_return_val_if_fail (GTK_IS_SOURCE_LANGUAGE (language), NULL);
	g_return_val_if_fail (language->priv->section != NULL, NULL);

	return g_strdup (language->priv->section);
}

gint
gtk_source_language_get_version (GtkSourceLanguage *language)
{
	g_return_val_if_fail (GTK_IS_SOURCE_LANGUAGE (language), 0);

	return language->priv->version;
}

/**
 * gtk_source_language_get_mime_types:
 * @language: a #GtkSourceLanguage.
 *
 * Returns a list of mime types for the given @language.  After usage you should
 * free each element of the list as well as the list itself.
 *
 * Return value: a list of mime types (strings).
 **/
GSList *
gtk_source_language_get_mime_types (GtkSourceLanguage *language)
{
	g_return_val_if_fail (GTK_IS_SOURCE_LANGUAGE (language), NULL);

	return slist_deep_copy (language->priv->mime_types);
}

/**
 * gtk_source_language_get_globs:
 * @language: a #GtkSourceLanguage.
 *
 * Returns a list of globs for the given @language.  After usage you should
 * free each element of the list as well as the list itself.
 *
 * Return value: a list of globs (strings).
 **/
GSList *
gtk_source_language_get_globs (GtkSourceLanguage *language)
{
	g_return_val_if_fail (GTK_IS_SOURCE_LANGUAGE (language), NULL);

	return slist_deep_copy (language->priv->globs);
}

/**
 * _gtk_source_language_get_languages_manager:
 * @language: a #GtkSourceLanguage.
 *
 * Returns the #GtkSourceLanguagesManager for the #GtkSourceLanguage.
 *
 * Return value: #GtkSourceLanguagesManager for @language.
 **/
GtkSourceLanguagesManager *
_gtk_source_language_get_languages_manager (GtkSourceLanguage *language)
{
	g_return_val_if_fail (GTK_IS_SOURCE_LANGUAGE (language), NULL);
	g_return_val_if_fail (language->priv->id != NULL, NULL);

	return language->priv->languages_manager;
}

/* Highlighting engine creation ------------------------------------------ */

void
_gtk_source_language_define_language_styles (GtkSourceLanguage *lang)
{
#define ADD_ALIAS(style,mapto)								\
	g_hash_table_insert (lang->priv->styles, g_strdup (style), g_strdup (mapto))

	ADD_ALIAS ("Base-N Integer", "def:base-n-integer");
	ADD_ALIAS ("Character", "def:character");
	ADD_ALIAS ("Comment", "def:comment");
	ADD_ALIAS ("Function", "def:function");
	ADD_ALIAS ("Decimal", "def:decimal");
	ADD_ALIAS ("Floating Point", "def:floating-point");
	ADD_ALIAS ("Keyword", "def:keyword");
	ADD_ALIAS ("Preprocessor", "def:preprocessor");
	ADD_ALIAS ("String", "def:string");
	ADD_ALIAS ("Specials", "def:specials");
	ADD_ALIAS ("Data Type", "def:data-type");

#undef ADD_ALIAS
}

GtkSourceEngine *
_gtk_source_language_create_engine (GtkSourceLanguage *language)
{
	GtkSourceContextEngine *ce;
	gboolean success = FALSE;

	ce = _gtk_source_context_engine_new (language);

	switch (language->priv->version)
	{
		case GTK_SOURCE_LANGUAGE_VERSION_1_0:
			success = _gtk_source_language_file_parse_version1 (language, ce);
			break;

		case GTK_SOURCE_LANGUAGE_VERSION_2_0:
			success = _gtk_source_language_file_parse_version2 (language, ce);
			break;
	}

	if (!success)
	{
		g_object_unref (ce);
		ce = NULL;
	}

	return ce ? GTK_SOURCE_ENGINE (ce) : NULL;
}
