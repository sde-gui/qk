/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*-
 *  gtksourcecontextengine.h
 *
 *  Copyright (C) 2003 - Gustavo Giráldez
 *  Copyright (C) 2005 - Marco Barisione, Emanuele Aina
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __GTK_SOURCE_CONTEXT_ENGINE_H__
#define __GTK_SOURCE_CONTEXT_ENGINE_H__

#include <gtksourceview/gtksourceengine.h>
#include <gtksourceview/gtksourcelanguage.h>

G_BEGIN_DECLS

#define GTK_TYPE_SOURCE_CONTEXT_ENGINE            (_gtk_source_context_engine_get_type ())
#define GTK_SOURCE_CONTEXT_ENGINE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_SOURCE_CONTEXT_ENGINE, GtkSourceContextEngine))
#define GTK_SOURCE_CONTEXT_ENGINE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_SOURCE_CONTEXT_ENGINE, GtkSourceContextEngineClass))
#define GTK_IS_SOURCE_CONTEXT_ENGINE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_SOURCE_CONTEXT_ENGINE))
#define GTK_IS_SOURCE_CONTEXT_ENGINE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_SOURCE_CONTEXT_ENGINE))
#define GTK_SOURCE_CONTEXT_ENGINE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_SOURCE_CONTEXT_ENGINE, GtkSourceContextEngineClass))

typedef struct _GtkSourceContextEngine        GtkSourceContextEngine;
typedef struct _GtkSourceContextEngineClass   GtkSourceContextEngineClass;
typedef struct _GtkSourceContextEnginePrivate GtkSourceContextEnginePrivate;

struct _GtkSourceContextEngine
{
	GtkSourceEngine engine;

	/*< private >*/
	GtkSourceContextEnginePrivate *priv;
};

struct _GtkSourceContextEngineClass
{
	GtkSourceEngineClass parent_class;
};

typedef enum {
	GTK_SOURCE_CONTEXT_EXTEND_PARENT	= 1 << 0,
	GTK_SOURCE_CONTEXT_END_AT_LINE_END	= 1 << 1,
	GTK_SOURCE_CONTEXT_FIRST_LINE_ONLY	= 1 << 2,
} GtkSourceContextMatchOptions;

GType		 _gtk_source_context_engine_get_type	(void) G_GNUC_CONST;

GtkSourceContextEngine *_gtk_source_context_engine_new  (GtkSourceLanguage	*lang);

gboolean	 _gtk_source_context_engine_define_context
							(GtkSourceContextEngine	 *ce,
							 const gchar		 *id,
							 const gchar		 *parent_id,
							 const gchar		 *match_regex,
							 const gchar		 *start_regex,
							 const gchar		 *end_regex,
							 const gchar		 *style,
							 GtkSourceContextMatchOptions options,
							 GError			**error);

gboolean	 _gtk_source_context_engine_add_sub_pattern
							(GtkSourceContextEngine	 *ce,
							 const gchar		 *id,
							 const gchar		 *parent_id,
							 const gchar		 *name,
							 const gchar		 *where,
							 const gchar		 *style,
							 GError			**error);

gboolean	 _gtk_source_context_engine_add_ref 	(GtkSourceContextEngine	 *ce,
							 const gchar		 *parent_id,
							 const gchar		 *ref_id,
							 gboolean		  all,
							 GError			**error);

/* Only for lang files version 1, do not use it */
void		 _gtk_source_context_engine_set_escape_char (GtkSourceContextEngine	*ce,
							     gunichar			 esc_char);

G_END_DECLS

#endif /* __GTK_SOURCE_CONTEXT_ENGINE_H__ */
