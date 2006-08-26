/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*-
 *  gtksourcelanguagesmanager.c
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

#include <string.h>
#include <libxml/xmlreader.h>
#include "gtksourceview-i18n.h"
#include "gtksourcelanguagesmanager.h"
#include "gtksourcelanguage-private.h"
#include "gtksourcelanguage.h"

#define RNG_SCHEMA_FILE		"language2.rng"
#define SOURCEVIEW_DIR		"gtksourceview-2.0"
#define LANGUAGE_DIR		"language-specs"
#define STYLE_DIR		"styles"

enum {
	PROP_0,
	PROP_LANG_SPECS_DIRS,
	PROP_STYLE_SCHEMES_DIRS
};

struct _GtkSourceLanguagesManagerPrivate
{
	GSList 		*available_languages;
	GSList		*language_specs_directories;
	GSList 		*style_schemes;
	GSList		*style_schemes_directories;
	char		*rng_file;
};


G_DEFINE_TYPE (GtkSourceLanguagesManager, gtk_source_languages_manager, G_TYPE_OBJECT)


static void	 gtk_source_languages_manager_finalize	 	(GObject 		   *object);

static void	 slist_deep_free 				(GSList 		   *list);
static GSList 	*get_lang_files 				(GtkSourceLanguagesManager *lm);

static void	 gtk_source_languages_manager_set_property 	(GObject 		   *object,
					   			 guint 	 		    prop_id,
			    		   			 const GValue 		   *value,
					   			 GParamSpec		   *pspec);
static void	 gtk_source_languages_manager_get_property 	(GObject 		   *object,
					   			 guint 	 		    prop_id,
			    		   			 GValue 		   *value,
					   			 GParamSpec		   *pspec);
static void	 gtk_source_languages_manager_set_specs_dirs	(GtkSourceLanguagesManager *lm,
								 const GSList 		   *dirs);
static void	 gtk_source_languages_manager_set_styles_dirs	(GtkSourceLanguagesManager *lm,
								 const GSList              *dirs);


static void
gtk_source_languages_manager_class_init (GtkSourceLanguagesManagerClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize	= gtk_source_languages_manager_finalize;

	object_class->set_property = gtk_source_languages_manager_set_property;
	object_class->get_property = gtk_source_languages_manager_get_property;

	g_object_class_install_property (object_class,
					 PROP_LANG_SPECS_DIRS,
					 g_param_spec_pointer ("lang_files_dirs",
						 	       _("Language specification directories"),
							       _("List of directories where the "
								 "language specification files (.lang) "
								 "are located"),
							       G_PARAM_READWRITE));

	g_object_class_install_property (object_class,
					 PROP_STYLE_SCHEMES_DIRS,
					 g_param_spec_pointer ("style_schemes_dirs",
						 	       _("Style schemes directories"),
							       _("List of directories where the "
								 "style scheme files (.styles) "
								 "are located"),
							       G_PARAM_READWRITE));
}

static void
gtk_source_languages_manager_set_property (GObject 	*object,
					   guint 	 prop_id,
			    		   const GValue *value,
					   GParamSpec	*pspec)
{
	GtkSourceLanguagesManager *lm;

	lm = GTK_SOURCE_LANGUAGES_MANAGER (object);

	switch (prop_id)
	{
	    case PROP_LANG_SPECS_DIRS:
		gtk_source_languages_manager_set_specs_dirs (lm, g_value_get_pointer (value));
		break;

	    case PROP_STYLE_SCHEMES_DIRS:
		gtk_source_languages_manager_set_styles_dirs (lm, g_value_get_pointer (value));
		break;

	    default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gtk_source_languages_manager_get_property (GObject 	*object,
					   guint 	 prop_id,
			    		   GValue 	*value,
					   GParamSpec	*pspec)
{
	GtkSourceLanguagesManager *lm;

	lm = GTK_SOURCE_LANGUAGES_MANAGER (object);

	switch (prop_id)
	{
	    case PROP_LANG_SPECS_DIRS:
		    g_value_set_pointer (value, lm->priv->language_specs_directories);
		    break;

	    case PROP_STYLE_SCHEMES_DIRS:
		    g_value_set_pointer (value, lm->priv->style_schemes_directories);
		    break;

	    default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gtk_source_languages_manager_init (GtkSourceLanguagesManager *lm)
{
	lm->priv = g_new0 (GtkSourceLanguagesManagerPrivate, 1);
	/* initialize dirs list with default value */
	gtk_source_languages_manager_set_specs_dirs (lm, NULL);
	gtk_source_languages_manager_set_styles_dirs (lm, NULL);
}

/**
 * gtk_source_languages_manager_new:
 *
 * Creates a new language manager.
 *
 * Return value: a #GtkSourceLanguagesManager.
 **/
GtkSourceLanguagesManager *
gtk_source_languages_manager_new (void)
{
	return g_object_new (GTK_TYPE_SOURCE_LANGUAGES_MANAGER, NULL);
}

static void
gtk_source_languages_manager_finalize (GObject *object)
{
	GtkSourceLanguagesManager *lm;

	lm = GTK_SOURCE_LANGUAGES_MANAGER (object);

	g_slist_foreach (lm->priv->available_languages, (GFunc) g_object_unref, NULL);
	g_slist_free (lm->priv->available_languages);
	slist_deep_free (lm->priv->language_specs_directories);
	slist_deep_free (lm->priv->style_schemes_directories);
	g_free (lm->priv->rng_file);
	g_free (lm->priv);

	G_OBJECT_CLASS (gtk_source_languages_manager_parent_class)->finalize (object);
}

static GSList *
slist_deep_copy (const GSList *list)
{
	GSList *copy = NULL;

	while (list != NULL)
	{
		copy = g_slist_prepend (copy, g_strdup (list->data));
		list = list->next;
	}

	return g_slist_reverse (copy);
}

static GSList *
copy_data_dirs_list (const GSList              *list,
		     const gchar               *basename)
{
	const gchar * const *xdg_dirs;
	GSList *new_list = NULL;

	if (list != NULL)
		return slist_deep_copy (list);

	/* Don't be tricked by the list order: what we want
	 * is [xdg2, xdg1, ~], in other words it should be
	 * in reverse order. This is due to the fact that
	 * in get_lang_files() we walk the list, read each
	 * dir and prepend the files, so the files in last
	 * dir of this list will be first in the files list.
	 */

	new_list = g_slist_prepend (new_list,
				    g_build_filename (g_get_user_config_dir (),
						      SOURCEVIEW_DIR,
						      basename,
						      NULL));

	/* fetch specs in XDG_DATA_DIRS */
	for (xdg_dirs = g_get_system_data_dirs (); xdg_dirs && *xdg_dirs; xdg_dirs++)
		new_list = g_slist_prepend (new_list,
					    g_build_filename (*xdg_dirs,
							      SOURCEVIEW_DIR,
							      basename,
							      NULL));

	return new_list;
}

static void
gtk_source_languages_manager_set_specs_dirs (GtkSourceLanguagesManager *lm,
					     const GSList              *dirs)
{
	g_return_if_fail (GTK_IS_SOURCE_LANGUAGES_MANAGER (lm));
	g_return_if_fail (lm->priv->available_languages == NULL);

	if (lm->priv->language_specs_directories != NULL)
		slist_deep_free (lm->priv->language_specs_directories);

	lm->priv->language_specs_directories = copy_data_dirs_list (dirs, LANGUAGE_DIR);
}

static void
gtk_source_languages_manager_set_styles_dirs (GtkSourceLanguagesManager *lm,
					      const GSList              *dirs)
{
	g_return_if_fail (GTK_IS_SOURCE_LANGUAGES_MANAGER (lm));
	g_return_if_fail (lm->priv->available_languages == NULL);

	if (lm->priv->style_schemes_directories != NULL)
		slist_deep_free (lm->priv->style_schemes_directories);

	lm->priv->style_schemes_directories = copy_data_dirs_list (dirs, STYLE_DIR);
}

/**
 * gtk_source_languages_manager_get_lang_files_dirs:
 * @lm: a #GtkSourceLanguagesManager.
 *
 * Gets a list of language files directories for the given language manager.
 *
 * Return value: a list of language files directories (as strings).
 **/
const GSList *
gtk_source_languages_manager_get_lang_files_dirs (GtkSourceLanguagesManager *lm)
{
	g_return_val_if_fail (GTK_IS_SOURCE_LANGUAGES_MANAGER (lm), NULL);

	return lm->priv->language_specs_directories;
}

/**
 * gtk_source_languages_manager_get_style_schemes_dirs:
 * @lm: a #GtkSourceLanguagesManager.
 *
 * Gets a list of style schemes directories for the given language manager.
 *
 * Return value: a list of style schemes directories (as strings).
 **/
const GSList *
gtk_source_languages_manager_get_style_schemes_dirs (GtkSourceLanguagesManager *lm)
{
	g_return_val_if_fail (GTK_IS_SOURCE_LANGUAGES_MANAGER (lm), NULL);

	return lm->priv->style_schemes_directories;
}

/**
 * _gtk_source_languages_manager_get_rng_file:
 * @lm: a #GtkSourceLanguagesManager.
 *
 * Returns location of the RNG schema file for lang files version 2.
 *
 * Returns: path to RNG file. It belongs to %lm and must not be freed or modified.
 **/
const char *
_gtk_source_languages_manager_get_rng_file (GtkSourceLanguagesManager *lm)
{
	g_return_val_if_fail (GTK_IS_SOURCE_LANGUAGES_MANAGER (lm), NULL);

	if (!lm->priv->rng_file)
	{
		const GSList *dirs;

		dirs = gtk_source_languages_manager_get_lang_files_dirs (lm);

		while (dirs != NULL)
		{
			char *file = g_build_filename (dirs->data, RNG_SCHEMA_FILE, NULL);

			if (g_file_test (file, G_FILE_TEST_EXISTS))
			{
				lm->priv->rng_file = file;
				break;
			}

			g_free (file);

			dirs = dirs->next;
		}
	}

	return lm->priv->rng_file;
}

static void
prepend_lang (gchar *id, GtkSourceLanguage *lang, GtkSourceLanguagesManager *lm)
{
	lm->priv->available_languages =
		g_slist_prepend (lm->priv->available_languages, lang);
}

/**
 * gtk_source_languages_manager_get_available_languages:
 * @lm: a #GtkSourceLanguagesManager.
 *
 * Gets a list of available languages for the given language manager.
 * This function returns a pointer to a internal list, so there is no need to
 * free it after usage.
 *
 * Return value: a list of #GtkSourceLanguage.
 **/
const GSList *
gtk_source_languages_manager_get_available_languages (GtkSourceLanguagesManager *lm)
{
	GSList *filenames, *l;
	GHashTable *lang_hash;

	g_return_val_if_fail (GTK_IS_SOURCE_LANGUAGES_MANAGER (lm), NULL);

	if (lm->priv->available_languages != NULL)
	{
		return lm->priv->available_languages;
	}

	/* Build list of availables languages */
	filenames = get_lang_files (lm);

	lang_hash = g_hash_table_new (g_str_hash, g_str_equal);

	for (l = filenames; l != NULL; l = l->next)
	{
		GtkSourceLanguage *lang;

		lang = _gtk_source_language_new_from_file ((const gchar*)l->data,
							   lm);

		if (lang == NULL)
		{
			g_warning ("Error reading language specification file '%s'",
				   (const gchar*)l->data);
			continue;
		}

		if (g_hash_table_lookup (lang_hash, lang->priv->id) == NULL)
		{
			g_hash_table_insert (lang_hash,
					     lang->priv->id,
					     lang);
		}
	}

	slist_deep_free (filenames);

	g_hash_table_foreach (lang_hash, (GHFunc) prepend_lang, lm);

	g_hash_table_destroy (lang_hash);

	return lm->priv->available_languages;
}

/**
 * gtk_source_languages_manager_get_language_for_mime_type:
 * @lm: a #GtkSourceLanguagesManager.
 * @mime_type: a mime type.
 *
 * Gets the #GtkSourceLanguage which is associated with the given @mime_type
 * in the language manager.
 *
 * Return value: a #GtkSourceLanguage, or %NULL if there is no language
 * associated with the given @mime_type.
 **/
/* FIXME use hash table here */
GtkSourceLanguage *
gtk_source_languages_manager_get_language_for_mime_type (GtkSourceLanguagesManager *lm,
							 const gchar               *mime_type)
{
	const GSList *languages;
	g_return_val_if_fail (mime_type != NULL, NULL);

	languages = gtk_source_languages_manager_get_available_languages (lm);

	while (languages != NULL)
	{
		GSList *mime_types, *tmp;

		GtkSourceLanguage *lang = GTK_SOURCE_LANGUAGE (languages->data);

		tmp = mime_types = gtk_source_language_get_mime_types (lang);

		while (tmp != NULL)
		{
			if (strcmp ((const gchar*)tmp->data, mime_type) == 0)
			{
				break;
			}

			tmp = g_slist_next (tmp);
		}

		slist_deep_free (mime_types);
		if (tmp != NULL)
			return lang;

		languages = g_slist_next (languages);
	}

	return NULL;
}

/**
 * gtk_source_languages_manager_get_language_for_filename:
 * @lm: a #GtkSourceLanguagesManager.
 * @filename: a filename in glib filename encoding.
 *
 * Return value: a #GtkSourceLanguage, or %NULL if there is no language
 * with glob matching @filename.
 **/
/* FIXME use xdgmimeglob */
GtkSourceLanguage *
gtk_source_languages_manager_get_language_for_filename (GtkSourceLanguagesManager *lm,
							const gchar               *filename)
{
	const GSList *languages;
	char *utf8_filename;

	g_return_val_if_fail (filename != NULL, NULL);

	utf8_filename = g_filename_to_utf8 (filename, -1, NULL, NULL, NULL);
	g_return_val_if_fail (utf8_filename != NULL, NULL);

	languages = gtk_source_languages_manager_get_available_languages (lm);

	while (languages != NULL)
	{
		GSList *globs, *tmp;
		GtkSourceLanguage *lang = languages->data;

		tmp = globs = gtk_source_language_get_globs (lang);

		while (tmp != NULL)
		{
			if (g_pattern_match_simple (tmp->data, utf8_filename))
				break;

			tmp = g_slist_next (tmp);
		}

		slist_deep_free (globs);

		if (tmp != NULL)
			return lang;

		languages = g_slist_next (languages);
	}

	return NULL;
}

/**
 * gtk_source_languages_manager_get_language_by_id:
 * @lm: a #GtkSourceLanguagesManager.
 * @id: a language id.
 *
 * Gets the #GtkSourceLanguage identified by the given @id in the language
 * manager.
 *
 * Return value: a #GtkSourceLanguage, or %NULL if there is no language
 * identified by the given @id.
 **/
GtkSourceLanguage *
gtk_source_languages_manager_get_language_by_id (GtkSourceLanguagesManager *lm,
						 const gchar               *id)
{
	const GSList *languages;
	gboolean found = FALSE;
	g_return_val_if_fail (id != NULL, NULL);

	languages = gtk_source_languages_manager_get_available_languages (lm);

	while (languages != NULL)
	{
		gchar *lang_id;

		GtkSourceLanguage *lang = GTK_SOURCE_LANGUAGE (languages->data);

		lang_id = gtk_source_language_get_id (lang);

		if (lang_id != NULL && (strcmp (lang_id, id) == 0))
		{
			found = TRUE;
		}

		g_free (lang_id);

		if (found)
			return lang;

		languages = g_slist_next (languages);
	}

	return NULL;
}

static GSList *
build_file_listing (const gchar *directory,
		    GSList      *filenames,
		    const gchar *suffix)
{
	GDir *dir;
	const gchar *name;

	dir = g_dir_open (directory, 0, NULL);

	if (dir == NULL)
		return filenames;

	while ((name = g_dir_read_name (dir)))
	{
		gchar *full_path = g_build_filename (directory, name, NULL);

		if (!g_file_test (full_path, G_FILE_TEST_IS_DIR) &&
		    g_str_has_suffix (name, suffix))
		{
			filenames = g_slist_prepend (filenames, full_path);
		}
		else
		{
			g_free (full_path);
		}
	}

	g_dir_close (dir);

	return filenames;
}

static GSList *
get_data_files (GSList      *dirs,
		const gchar *suffix)
{
	GSList *filenames = NULL;

	while (dirs != NULL)
	{
		filenames = build_file_listing (dirs->data, filenames, suffix);
		dirs = dirs->next;
	}

	return filenames;
}

static GSList *
get_lang_files (GtkSourceLanguagesManager *lm)
{
	g_return_val_if_fail (lm->priv->language_specs_directories != NULL, NULL);
	return get_data_files (lm->priv->language_specs_directories, ".lang");
}

static void
slist_deep_free (GSList *list)
{
	g_slist_foreach (list, (GFunc) g_free, NULL);
	g_slist_free (list);
}

static GSList *
get_style_scheme_files (GtkSourceLanguagesManager *lm)
{
	g_return_val_if_fail (lm->priv->style_schemes_directories != NULL, NULL);
	return get_data_files (lm->priv->style_schemes_directories, ".styles");
}

/**
 * gtk_source_languages_manager_get_available_style_schemes:
 * @lm: a #GtkSourceLanguagesManager.
 *
 * Gets a list of available style schemes for the given language manager.
 * This function returns a pointer to a internal list, so there is no need to
 * free it after usage.
 *
 * Return value: a list of #GtkSourceStyleScheme.
 **/
const GSList *
gtk_source_languages_manager_get_available_style_schemes (GtkSourceLanguagesManager *lm)
{
	GSList *files;
	GHashTable *schemes;
	GtkSourceStyleScheme *scheme;

	g_return_val_if_fail (GTK_IS_SOURCE_LANGUAGES_MANAGER (lm), NULL);

	if (lm->priv->style_schemes)
		return lm->priv->style_schemes;

	files = get_style_scheme_files (lm);
	schemes = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);

	while (files)
	{
		gchar *filename = files->data;

		files = g_slist_delete_link (files, files);

		scheme = _gtk_source_style_scheme_new_from_file (filename);

		if (scheme)
		{
			const char *id = gtk_source_style_scheme_get_id (scheme);
			GtkSourceStyleScheme *old;

			old = g_hash_table_lookup (schemes, id);

			if (old)
				lm->priv->style_schemes = g_slist_remove (lm->priv->style_schemes, old);

			lm->priv->style_schemes = g_slist_prepend (lm->priv->style_schemes, scheme);
			g_hash_table_insert (schemes, g_strdup (id), g_object_ref (scheme));
		}

		g_free (filename);
	}

	g_hash_table_destroy (schemes);

	lm->priv->style_schemes = g_slist_reverse (lm->priv->style_schemes);
	return lm->priv->style_schemes;
}
