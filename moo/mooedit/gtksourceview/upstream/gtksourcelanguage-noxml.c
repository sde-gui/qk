/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*-
 *  gtksourcelanguage-noxml.c
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

#include <config.h>

#ifdef MOO_USE_XML
#error "This file may not be compiled in when libxml is used"
#endif

#include "gtksourcelanguage-private.h"
#include "gtksourcelanguage.h"

G_DEFINE_TYPE (GtkSourceLanguage, gtk_source_language, G_TYPE_OBJECT)

GtkSourceLanguage *
_gtk_source_language_new_from_file (G_GNUC_UNUSED const gchar *filename,
				    G_GNUC_UNUSED GtkSourceLanguagesManager *lm)
{
	return NULL;
}

static void
gtk_source_language_class_init (G_GNUC_UNUSED GtkSourceLanguageClass *klass)
{
}

static void
gtk_source_language_init (G_GNUC_UNUSED GtkSourceLanguage *lang)
{
}

gchar *
gtk_source_language_get_id (G_GNUC_UNUSED GtkSourceLanguage *language)
{
	g_return_val_if_reached (NULL);
}

gchar *
gtk_source_language_get_name (G_GNUC_UNUSED GtkSourceLanguage *language)
{
	g_return_val_if_reached (NULL);
}

gchar *
gtk_source_language_get_section	(G_GNUC_UNUSED GtkSourceLanguage *language)
{
	g_return_val_if_reached (NULL);
}

const gchar *
gtk_source_language_get_property (G_GNUC_UNUSED GtkSourceLanguage *language,
				  G_GNUC_UNUSED const gchar       *name)
{
	g_return_val_if_reached (NULL);
}

GtkSourceLanguagesManager *
_gtk_source_language_get_languages_manager (G_GNUC_UNUSED GtkSourceLanguage *language)
{
	g_return_val_if_reached (NULL);
}

void
_gtk_source_language_define_language_styles (G_GNUC_UNUSED GtkSourceLanguage *lang)
{
	g_return_if_reached ();
}

GtkSourceEngine *
_gtk_source_language_create_engine (G_GNUC_UNUSED GtkSourceLanguage *language)
{
	g_return_val_if_reached (NULL);
}
