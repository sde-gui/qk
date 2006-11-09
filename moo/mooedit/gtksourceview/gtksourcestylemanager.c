/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*-
 *  gtksourcestylemanager.c
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

#include "gtksourcestylemanager.h"
#include "gtksourceview-marshal.h"
#include <string.h>

#define SCHEME_FILE_SUFFIX	".styles"
#define SOURCEVIEW_DIR		"gtksourceview-2.0"
#define STYLES_DIR		"styles"


struct _GtkSourceStyleManagerPrivate
{
	GSList		*schemes;
	GSList		*dirs;
	gboolean	 need_reload;
};

enum {
	CHANGED,
	N_SIGNALS
};

static guint signals[N_SIGNALS];

G_DEFINE_TYPE (GtkSourceStyleManager, gtk_source_style_manager, G_TYPE_OBJECT)


static void	 gtk_source_style_manager_finalize	 	(GObject	*object);


static void
gtk_source_style_manager_class_init (GtkSourceStyleManagerClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize	= gtk_source_style_manager_finalize;

	signals[CHANGED] = g_signal_new ("changed",
					 G_OBJECT_CLASS_TYPE (object_class),
					 G_SIGNAL_RUN_LAST,
					 G_STRUCT_OFFSET (GtkSourceStyleManagerClass, changed),
					 NULL, NULL,
					 _gtksourceview_marshal_VOID__VOID,
					 G_TYPE_NONE, 0);
}

static void
gtk_source_style_manager_init (GtkSourceStyleManager *mgr)
{
	mgr->priv = g_new0 (GtkSourceStyleManagerPrivate, 1);
	mgr->priv->schemes = NULL;
	mgr->priv->dirs = NULL;
	mgr->priv->need_reload = TRUE;
}

static void
gtk_source_style_manager_finalize (GObject *object)
{
	GtkSourceStyleManager *mgr;
	GSList *schemes;

	mgr = GTK_SOURCE_STYLE_MANAGER (object);

	schemes = mgr->priv->schemes;
	mgr->priv->schemes = NULL;
	g_slist_foreach (schemes, (GFunc) g_object_unref, NULL);
	g_slist_free (schemes);
	g_slist_foreach (mgr->priv->dirs, (GFunc) g_free, NULL);
	g_slist_free (mgr->priv->dirs);
	g_free (mgr->priv);

	G_OBJECT_CLASS (gtk_source_style_manager_parent_class)->finalize (object);
}

GtkSourceStyleManager *
gtk_source_style_manager_new (void)
{
	return g_object_new (GTK_TYPE_SOURCE_STYLE_MANAGER, NULL);
}

static GSList *
build_file_listing (const gchar *directory,
		    GSList      *filenames)
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
		    g_str_has_suffix (name, SCHEME_FILE_SUFFIX))
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
get_scheme_files (GtkSourceStyleManager *mgr)
{
	gchar **dirs;
	guint n_dirs;
	guint i;
	GSList *files = NULL;

	gtk_source_style_manager_get_search_path (mgr, &dirs, &n_dirs);

	for (i = 0; i < n_dirs; ++i)
		files = build_file_listing (dirs[i], files);

	g_strfreev (dirs);
	return files;
}

static gboolean
build_reference_chain (GtkSourceStyleScheme *scheme,
		       GHashTable           *hash,
		       GSList              **ret)
{
	GSList *chain;
	gboolean retval = TRUE;

	chain = g_slist_prepend (NULL, scheme);

	while (TRUE)
	{
		GtkSourceStyleScheme *parent_scheme;
		const gchar *parent_id;

		parent_id = _gtk_source_style_scheme_get_parent_id (scheme);

		if (parent_id == NULL)
			break;

		parent_scheme = g_hash_table_lookup (hash, parent_id);

		if (parent_scheme == NULL)
		{
			g_warning ("unknown parent scheme %s in scheme %s",
				   parent_id, gtk_source_style_scheme_get_id (scheme));
			retval = FALSE;
			break;
		}
		else if (g_slist_find (chain, parent_scheme) != NULL)
		{
			g_warning ("reference cycle in scheme %s", parent_id);
			retval = FALSE;
			break;
		}
		else
		{
			_gtk_source_style_scheme_set_parent (scheme, parent_scheme);
		}

		chain = g_slist_prepend (chain, parent_scheme);
		scheme = parent_scheme;
	}

	*ret = chain;
	return retval;
}

static GSList *
check_parents (GSList     *schemes,
	       GHashTable *hash)
{
	GSList *to_check;

	to_check = g_slist_copy (schemes);

	while (to_check != NULL)
	{
		GSList *chain;
		gboolean valid;

		valid = build_reference_chain (to_check->data, hash, &chain);

		while (chain != NULL)
		{
			GtkSourceStyleScheme *scheme = chain->data;
			to_check = g_slist_remove (to_check, scheme);

			if (!valid)
			{
				schemes = g_slist_remove (schemes, scheme);
				g_hash_table_remove (hash, gtk_source_style_scheme_get_id (scheme));
				g_object_unref (scheme);
			}

			chain = g_slist_delete_link (chain, chain);
		}
	}

	return schemes;
}

static void
gtk_source_style_manager_changed (GtkSourceStyleManager *mgr)
{
	if (!mgr->priv->need_reload)
	{
		mgr->priv->need_reload = TRUE;
		g_signal_emit (mgr, signals[CHANGED], 0);
	}
}

static void
gtk_source_style_manager_reload (GtkSourceStyleManager *mgr)
{
	GHashTable *schemes_hash;
	GSList *schemes;
	GSList *files;
	GtkSourceStyleScheme *def_scheme;

	g_return_if_fail (GTK_IS_SOURCE_STYLE_MANAGER (mgr));

	schemes = mgr->priv->schemes;
	mgr->priv->schemes = NULL;
	g_slist_foreach (schemes, (GFunc) g_object_unref, NULL);
	g_slist_free (schemes);

	files = get_scheme_files (mgr);
	schemes_hash = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);
	def_scheme = _gtk_source_style_scheme_default_new ();
	schemes = g_slist_prepend (NULL, def_scheme);
	g_hash_table_insert (schemes_hash, g_strdup (gtk_source_style_scheme_get_id (def_scheme)),
			     g_object_ref (def_scheme));

	while (files != NULL)
	{
		GtkSourceStyleScheme *scheme;
		gchar *filename = files->data;

		files = g_slist_delete_link (files, files);

		scheme = _gtk_source_style_scheme_new_from_file (filename);

		if (scheme != NULL)
		{
			const gchar *id = gtk_source_style_scheme_get_id (scheme);
			GtkSourceStyleScheme *old;

			old = g_hash_table_lookup (schemes_hash, id);

			if (old != NULL)
				schemes = g_slist_remove (schemes, old);

			schemes = g_slist_prepend (schemes, scheme);
			g_hash_table_insert (schemes_hash, g_strdup (id), g_object_ref (scheme));
		}

		g_free (filename);
	}

	schemes = check_parents (schemes, schemes_hash);
	g_hash_table_destroy (schemes_hash);

	mgr->priv->schemes = schemes;
	mgr->priv->need_reload = FALSE;
}

void
gtk_source_style_manager_set_search_path (GtkSourceStyleManager	*mgr,
					  gchar		       **path,
					  gint			 n_elements)
{
	int i;

	g_return_if_fail (GTK_IS_SOURCE_STYLE_MANAGER (mgr));
	g_return_if_fail (path != NULL || n_elements == 0);

	if (n_elements < 0)
		n_elements = g_strv_length (path);

	g_slist_foreach (mgr->priv->dirs, (GFunc) g_free, NULL);
	mgr->priv->dirs = NULL;

	for (i = 0; i < n_elements; ++i)
		mgr->priv->dirs = g_slist_prepend (mgr->priv->dirs, g_strdup (path[i]));

	mgr->priv->dirs = g_slist_reverse (mgr->priv->dirs);

	gtk_source_style_manager_changed (mgr);
}

void
gtk_source_style_manager_get_search_path (GtkSourceStyleManager	*mgr,
					  gchar		      ***path,
					  guint			*n_elements)
{
	guint n, i;
	GSList *l;

	g_return_if_fail (GTK_IS_SOURCE_STYLE_MANAGER (mgr));
	g_return_if_fail (path != NULL);

	if (mgr->priv->dirs == NULL)
	{
		const gchar * const *xdg_dirs;

		mgr->priv->dirs = g_slist_prepend (NULL,
						   g_build_filename (g_get_user_config_dir (),
								     SOURCEVIEW_DIR,
								     STYLES_DIR,
								     NULL));

		for (xdg_dirs = g_get_system_data_dirs (); xdg_dirs && *xdg_dirs; ++xdg_dirs)
			mgr->priv->dirs = g_slist_prepend (mgr->priv->dirs,
							   g_build_filename (*xdg_dirs,
									     SOURCEVIEW_DIR,
									     STYLES_DIR,
									     NULL));

		mgr->priv->dirs = g_slist_reverse (mgr->priv->dirs);
	}

	n = g_slist_length (mgr->priv->dirs);
	*path = g_new0 (char*, n + 1);

	for (i = 0, l = mgr->priv->dirs; l != NULL; ++i, l = l->next)
		(*path)[i] = g_strdup (l->data);

	if (n_elements != NULL)
		*n_elements = n;
}

void
gtk_source_style_manager_append_search_path (GtkSourceStyleManager *manager,
					     const gchar           *directory)
{
	char **dirs, **new_dirs;
	guint n_dirs;

	g_return_if_fail (GTK_IS_SOURCE_STYLE_MANAGER (manager));
	g_return_if_fail (directory != NULL);

	gtk_source_style_manager_get_search_path (manager, &dirs, &n_dirs);
	new_dirs = g_new (char*, n_dirs + 2);

	if (n_dirs)
		memcpy (new_dirs, dirs, n_dirs * sizeof(char*));

	new_dirs[n_dirs] = (char*) directory;
	new_dirs[n_dirs + 1] = NULL;

	gtk_source_style_manager_set_search_path (manager, new_dirs, n_dirs + 1);

	g_strfreev (dirs);
	g_free (new_dirs);
}

void
gtk_source_style_manager_prepend_search_path (GtkSourceStyleManager *manager,
					      const gchar           *directory)
{
	char **dirs, **new_dirs;
	guint n_dirs;

	g_return_if_fail (GTK_IS_SOURCE_STYLE_MANAGER (manager));
	g_return_if_fail (directory != NULL);

	gtk_source_style_manager_get_search_path (manager, &dirs, &n_dirs);
	new_dirs = g_new (char*, n_dirs + 2);

	if (n_dirs)
		memcpy (new_dirs + 1, dirs, n_dirs * sizeof(char*));

	new_dirs[0] = (char*) directory;
	new_dirs[n_dirs + 1] = NULL;

	gtk_source_style_manager_set_search_path (manager, new_dirs, n_dirs + 1);

	g_strfreev (dirs);
	g_free (new_dirs);
}

// gboolean
// gtk_source_style_manager_add_scheme (GtkSourceStyleManager *manager,
// 				     const gchar           *filename)
// {
// 	GtkSourceStyleScheme *scheme;
//
// 	g_return_val_if_fail (GTK_IS_SOURCE_STYLE_MANAGER (manager), FALSE);
// 	g_return_val_if_fail (filename != NULL, FALSE);
//
// 	scheme = _gtk_source_style_scheme_new_from_file (filename);
//
// 	if (scheme == NULL)
// 		return FALSE;
//
// 	/* ??? */
// }

static void
reload_if_needed (GtkSourceStyleManager *mgr)
{
	if (mgr->priv->need_reload)
		gtk_source_style_manager_reload (mgr);
}

GSList *
gtk_source_style_manager_list_schemes (GtkSourceStyleManager *mgr)
{
	GSList *list;

	g_return_val_if_fail (GTK_IS_SOURCE_STYLE_MANAGER (mgr), NULL);

	reload_if_needed (mgr);

	list = g_slist_copy (mgr->priv->schemes);
	g_slist_foreach (list, (GFunc) g_object_ref, NULL);

	return list;
}

GtkSourceStyleScheme *
gtk_source_style_manager_get_scheme (GtkSourceStyleManager *mgr,
				     const gchar           *scheme_id)
{
	GSList *l;

	g_return_val_if_fail (GTK_IS_SOURCE_STYLE_MANAGER (mgr), NULL);
	g_return_val_if_fail (scheme_id != NULL, NULL);

	reload_if_needed (mgr);

	for (l = mgr->priv->schemes; l != NULL; l = l->next)
		if (!strcmp (scheme_id, gtk_source_style_scheme_get_id (l->data)))
			return l->data;

	return NULL;
}
