/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*-
 *  gtksourcelanguagemanager.h
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

#ifndef __GTK_SOURCE_LANGUAGE_MANAGER_H__
#define __GTK_SOURCE_LANGUAGE_MANAGER_H__

#include <gtksourceview/gtksourcelanguage.h>

G_BEGIN_DECLS

#define GTK_TYPE_SOURCE_LANGUAGE_MANAGER		(gtk_source_language_manager_get_type ())
#define GTK_SOURCE_LANGUAGE_MANAGER(obj)		(GTK_CHECK_CAST ((obj), GTK_TYPE_SOURCE_LANGUAGE_MANAGER, GtkSourceLanguageManager))
#define GTK_SOURCE_LANGUAGE_MANAGER_CLASS(klass)	(GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_SOURCE_LANGUAGE_MANAGER, GtkSourceLanguageManagerClass))
#define GTK_IS_SOURCE_LANGUAGE_MANAGER(obj)		(GTK_CHECK_TYPE ((obj), GTK_TYPE_SOURCE_LANGUAGE_MANAGER))
#define GTK_IS_SOURCE_LANGUAGE_MANAGER_CLASS(klass)	(GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_SOURCE_LANGUAGE_MANAGER))


typedef struct _GtkSourceLanguageManager		GtkSourceLanguageManager;
typedef struct _GtkSourceLanguageManagerClass		GtkSourceLanguageManagerClass;

typedef struct _GtkSourceLanguageManagerPrivate	GtkSourceLanguageManagerPrivate;

struct _GtkSourceLanguageManager
{
	GObject                   parent;

	GtkSourceLanguageManagerPrivate *priv;
};

struct _GtkSourceLanguageManagerClass
{
	GObjectClass              parent_class;

	/* Padding for future expansion */
	void (*_gtk_source_reserved1) (void);
	void (*_gtk_source_reserved2) (void);
};


GType			  gtk_source_language_manager_get_type			(void) G_GNUC_CONST;

GtkSourceLanguageManager *gtk_source_language_manager_new			(void);


const GSList		 *gtk_source_language_manager_get_available_languages	(GtkSourceLanguageManager *lm);

GtkSourceLanguage	 *gtk_source_language_manager_get_language_by_id	(GtkSourceLanguageManager *lm,
										 const gchar              *id);

gchar			**gtk_source_language_manager_get_lang_files_dirs	(GtkSourceLanguageManager *lm);
void			  gtk_source_language_manager_set_lang_files_dirs	(GtkSourceLanguageManager *lm,
										 gchar                   **dirs);

G_END_DECLS

#endif /* __GTK_SOURCE_LANGUAGE_MANAGER_H__ */

