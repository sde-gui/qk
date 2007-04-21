/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*-
 *  gtksourcelanguagemanager.c
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
#include "gtksourceview-i18n.h"
#include "gtksourcelanguage-private.h"
#include "gtksourcelanguage.h"

#define RNG_SCHEMA_FILE		"language2.rng"
#define SOURCEVIEW_DIR		"gtksourceview-2.0"
#define LANGUAGE_DIR		"language-specs"

enum {
	PROP_0,
	PROP_LANG_SPECS_DIRS
};

struct _GtkSourceLanguageManagerPrivate
{
	GHashTable	*language_ids;
	GSList 		*available_languages;
	char	       **lang_dirs;
	char		*rng_file;
};

G_DEFINE_TYPE (GtkSourceLanguageManager, gtk_source_language_manager, G_TYPE_OBJECT)


static void	 gtk_source_language_manager_finalize	 	(GObject 		   *object);

static void	 slist_deep_free 				(GSList 		   *list);
static GSList 	*get_lang_files 				(GtkSourceLanguageManager  *lm);

static void	 gtk_source_language_manager_set_property 	(GObject 		   *object,
					   			 guint 	 		    prop_id,
			    		   			 const GValue 		   *value,
					   			 GParamSpec		   *pspec);
static void	 gtk_source_language_manager_get_property 	(GObject 		   *object,
					   			 guint 	 		    prop_id,
			    		   			 GValue 		   *value,
					   			 GParamSpec		   *pspec);


static void
gtk_source_language_manager_class_init (GtkSourceLanguageManagerClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize	= gtk_source_language_manager_finalize;

	object_class->set_property = gtk_source_language_manager_set_property;
	object_class->get_property = gtk_source_language_manager_get_property;

	g_object_class_install_property (object_class,
					 PROP_LANG_SPECS_DIRS,
					 g_param_spec_boxed ("lang_files_dirs",
						 	     _("Language specification directories"),
							     _("List of directories where the "
							       "language specification files (.lang) "
							       "are located"),
							     G_TYPE_STRV,
							     G_PARAM_READWRITE));
}

static void
gtk_source_language_manager_set_property (GObject 	*object,
					  guint 	 prop_id,
					  const GValue *value,
					  GParamSpec	*pspec)
{
	GtkSourceLanguageManager *lm;

	lm = GTK_SOURCE_LANGUAGE_MANAGER (object);

	switch (prop_id)
	{
	    case PROP_LANG_SPECS_DIRS:
		gtk_source_language_manager_set_lang_files_dirs (lm, g_value_get_boxed (value));
		break;

	    default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gtk_source_language_manager_get_property (GObject 	*object,
					  guint 	 prop_id,
					  GValue 	*value,
					  GParamSpec	*pspec)
{
	GtkSourceLanguageManager *lm;

	lm = GTK_SOURCE_LANGUAGE_MANAGER (object);

	switch (prop_id)
	{
	    case PROP_LANG_SPECS_DIRS:
		    g_value_set_boxed (value, gtk_source_language_manager_get_lang_files_dirs (lm));
		    break;

	    default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gtk_source_language_manager_init (GtkSourceLanguageManager *lm)
{
	lm->priv = g_new0 (GtkSourceLanguageManagerPrivate, 1);
	lm->priv->language_ids = NULL;
	lm->priv->available_languages = NULL;
	lm->priv->lang_dirs = NULL;
	lm->priv->rng_file = NULL;
}

/**
 * gtk_source_language_manager_new:
 *
 * Creates a new language manager.
 *
 * Returns: a #GtkSourceLanguageManager.
 **/
GtkSourceLanguageManager *
gtk_source_language_manager_new (void)
{
	return g_object_new (GTK_TYPE_SOURCE_LANGUAGE_MANAGER, NULL);
}

static void
gtk_source_language_manager_finalize (GObject *object)
{
	GtkSourceLanguageManager *lm;

	lm = GTK_SOURCE_LANGUAGE_MANAGER (object);

	if (lm->priv->language_ids)
		g_hash_table_destroy (lm->priv->language_ids);

	g_slist_foreach (lm->priv->available_languages, (GFunc) g_object_unref, NULL);
	g_slist_free (lm->priv->available_languages);
	g_strfreev (lm->priv->lang_dirs);
	g_free (lm->priv->rng_file);
	g_free (lm->priv);

	G_OBJECT_CLASS (gtk_source_language_manager_parent_class)->finalize (object);
}

/**
 * gtk_source_language_manager_set_lang_files_dirs:
 * @lm: a #GtkSourceLanguageManager.
 * @dirs: a %NULL-terminated array of strings or %NULL.
 *
 * Sets a list of language files directories for the given language manager.
 * @dirs == %NULL resets directories list to default.
 **/
void
gtk_source_language_manager_set_lang_files_dirs	(GtkSourceLanguageManager *lm,
						 gchar                   **dirs)
{
	char **tmp;

	g_return_if_fail (GTK_IS_SOURCE_LANGUAGE_MANAGER (lm));
	/* FIXME: so what do we do? rescan, refresh? */
	g_return_if_fail (lm->priv->available_languages == NULL);

	tmp = lm->priv->lang_dirs;
	lm->priv->lang_dirs = g_strdupv (dirs);
	g_strfreev (tmp);

	g_object_notify (G_OBJECT (lm), "lang-files-dirs");
}

/**
 * gtk_source_language_manager_get_lang_files_dirs:
 * @lm: a #GtkSourceLanguageManager.
 *
 * Gets a list of language files directories for the given language manager.
 *
 * Returns: %NULL-terminated array containg a list of language files directories.
 * It is owned by @lm and must not be modified or freed.
 **/
gchar **
gtk_source_language_manager_get_lang_files_dirs	(GtkSourceLanguageManager *lm)
{
	g_return_val_if_fail (GTK_IS_SOURCE_LANGUAGE_MANAGER (lm), NULL);

	if (lm->priv->lang_dirs == NULL)
	{
		const gchar * const *xdg_dirs;
		GPtrArray *new_dirs;

		new_dirs = g_ptr_array_new ();

		for (xdg_dirs = g_get_system_data_dirs (); xdg_dirs && *xdg_dirs; ++xdg_dirs)
			g_ptr_array_add (new_dirs, g_build_filename (*xdg_dirs,
								     SOURCEVIEW_DIR,
								     LANGUAGE_DIR,
								     NULL));
#ifdef G_OS_UNIX
		g_ptr_array_add (new_dirs, g_strdup_printf ("%s/%s",
							    g_get_user_data_dir (),
							    ".gnome2/gtksourceview-1.0/language-specs"));
#endif
		g_ptr_array_add (new_dirs, g_build_filename (g_get_user_data_dir (),
							     SOURCEVIEW_DIR,
							     LANGUAGE_DIR,
							     NULL));
		g_ptr_array_add (new_dirs, NULL);

		lm->priv->lang_dirs = (gchar**) g_ptr_array_free (new_dirs, FALSE);
	}

	return lm->priv->lang_dirs;
}

/**
 * _gtk_source_language_manager_get_rng_file:
 * @lm: a #GtkSourceLanguageManager.
 *
 * Returns location of the RNG schema file for lang files version 2.
 *
 * Returns: path to RNG file. It belongs to %lm and must not be freed or modified.
 **/
const char *
_gtk_source_language_manager_get_rng_file (GtkSourceLanguageManager *lm)
{
	g_return_val_if_fail (GTK_IS_SOURCE_LANGUAGE_MANAGER (lm), NULL);

	if (lm->priv->rng_file == NULL)
	{
		gchar **dirs;

		for (dirs = gtk_source_language_manager_get_lang_files_dirs (lm);
		     dirs != NULL && *dirs != NULL;
		     ++dirs)
		{
			char *file = g_build_filename (*dirs, RNG_SCHEMA_FILE, NULL);

			if (g_file_test (file, G_FILE_TEST_EXISTS))
			{
				lm->priv->rng_file = file;
				break;
			}

			g_free (file);
		}
	}

	return lm->priv->rng_file;
}

static void
prepend_lang (G_GNUC_UNUSED gchar      *id,
	      GtkSourceLanguage        *lang,
	      GtkSourceLanguageManager *lm)
{
	lm->priv->available_languages =
		g_slist_prepend (lm->priv->available_languages, lang);
}

static void
ensure_languages (GtkSourceLanguageManager *lm)
{
	GSList *filenames, *l;

	if (lm->priv->language_ids != NULL)
		return;

	/* Build list of availables languages */
	filenames = get_lang_files (lm);
	lm->priv->language_ids = g_hash_table_new (g_str_hash, g_str_equal);

	for (l = filenames; l != NULL; l = l->next)
	{
		GtkSourceLanguage *lang;
		const gchar *filename;

		filename = l->data;
		lang = _gtk_source_language_new_from_file (filename, lm);

		if (lang == NULL)
		{
			g_warning ("Error reading language specification file '%s'", filename);
			continue;
		}

		if (g_hash_table_lookup (lm->priv->language_ids, lang->priv->id) == NULL)
			g_hash_table_insert (lm->priv->language_ids,
					     lang->priv->id,
					     lang);
		else
			g_object_unref (lang);
	}

	g_hash_table_foreach (lm->priv->language_ids, (GHFunc) prepend_lang, lm);
	slist_deep_free (filenames);
}

/**
 * gtk_source_language_manager_get_available_languages:
 * @lm: a #GtkSourceLanguageManager.
 *
 * Gets a list of available languages for the given language manager.
 *
 * Returns: a list of #GtkSourceLanguage. Return value is owned by @lm and should
 * not be modified or freed.
 **/
const GSList *
gtk_source_language_manager_get_available_languages (GtkSourceLanguageManager *lm)
{
	g_return_val_if_fail (GTK_IS_SOURCE_LANGUAGE_MANAGER (lm), NULL);
	ensure_languages (lm);
	return lm->priv->available_languages;
}

/**
 * gtk_source_language_manager_get_language_by_id:
 * @lm: a #GtkSourceLanguageManager.
 * @id: a language id.
 *
 * Gets the #GtkSourceLanguage identified by the given @id in the language
 * manager.
 *
 * Returns: a #GtkSourceLanguage, or %NULL if there is no language
 * identified by the given @id. Return value is owned by @lm and should not
 * be freed.
 **/
GtkSourceLanguage *
gtk_source_language_manager_get_language_by_id (GtkSourceLanguageManager *lm,
						const gchar              *id)
{
	g_return_val_if_fail (id != NULL, NULL);
	ensure_languages (lm);
	return g_hash_table_lookup (lm->priv->language_ids, id);
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

	while ((name = g_dir_read_name (dir)) != NULL)
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
get_data_files (gchar      **dirs,
		const gchar *suffix)
{
	GSList *filenames = NULL;

	while (dirs && *dirs)
	{
		filenames = build_file_listing (*dirs, filenames, suffix);
		++dirs;
	}

	return filenames;
}

static GSList *
get_lang_files (GtkSourceLanguageManager *lm)
{
	return get_data_files (gtk_source_language_manager_get_lang_files_dirs (lm), ".lang");
}

static void
slist_deep_free (GSList *list)
{
	g_slist_foreach (list, (GFunc) g_free, NULL);
	g_slist_free (list);
}
