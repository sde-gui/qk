/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*-
 *  gtksourcelanguage-private.h
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

#ifndef __GTK_SOURCE_LANGUAGE_PRIVATE_H__
#define __GTK_SOURCE_LANGUAGE_PRIVATE_H__

#include <glib.h>
#include "gtksourcecontextengine.h"
#include "gtksourcelanguagesmanager.h"

G_BEGIN_DECLS

struct _GtkSourceLanguagePrivate
{
	gchar			*lang_file_name;
	gchar                   *translation_domain;

	gchar			*id;
	gchar			*name;
	gchar			*section;

	/* maps style names to default styles (e.g. "comment" to "def:comment") */
	GHashTable		*styles;

	gint                     version;
	gboolean		 hidden;

	GSList			*mime_types;
	GSList			*globs;

	gchar			*brackets;
	gchar			*line_comment;
	gchar			*block_comment_start;
	gchar			*block_comment_end;

	GtkSourceLanguagesManager *languages_manager;
};

GtkSourceLanguage *_gtk_source_language_new_from_file (const gchar			*filename,
						       GtkSourceLanguagesManager	*lm);

GtkSourceLanguagesManager *_gtk_source_language_get_languages_manager (GtkSourceLanguage *language);
const char *_gtk_source_languages_manager_get_rng_file (GtkSourceLanguagesManager *lm);

void _gtk_source_language_define_language_styles  (GtkSourceLanguage      *language);
gboolean _gtk_source_language_file_parse_version1 (GtkSourceLanguage      *language,
						   GtkSourceContextEngine *engine);
gboolean _gtk_source_language_file_parse_version2 (GtkSourceLanguage      *language,
						   GtkSourceContextEngine *engine);
GtkSourceEngine *_gtk_source_language_create_engine (GtkSourceLanguage *language);

G_END_DECLS

#endif  /* __GTK_SOURCE_LANGUAGE_PRIVATE_H__ */

