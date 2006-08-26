/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*-
 *  gtksourcelanguage-parser-ver1.c
 *  Language specification parser for 1.0 version .lang files
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

#include <libxml/parser.h>
#include "gtksourceview-i18n.h"
#include "gtksourcebuffer.h"
#include "gtksourcetag.h"
#include "gtksourcelanguage.h"
#include "gtksourcelanguagesmanager.h"
#include "gtksourcelanguage-private.h"

static gboolean
engine_add_simple_pattern (GtkSourceContextEngine *ce,
			   GtkSourceLanguage      *language,
			   const gchar            *id,
			   const gchar            *style,
			   const gchar            *pattern)
{
	gboolean result;
	char *real_id, *root_id;
	GError *error = NULL;

	g_return_val_if_fail (id != NULL, FALSE);

	root_id = g_strdup_printf ("%s:%s", language->priv->id, language->priv->id);
	real_id = g_strdup_printf ("%s:%s", language->priv->id, id);

	result = _gtk_source_context_engine_define_context (ce, real_id,
							    root_id,
							    pattern, NULL, NULL,
							    style,
							    TRUE,
							    TRUE,
							    &error);

	if (error)
	{
		g_warning ("%s", error->message);
		g_error_free (error);
	}

	g_free (real_id);
	g_free (root_id);
	return result;
}

static gboolean
engine_add_syntax_pattern (GtkSourceContextEngine *ce,
			   GtkSourceLanguage      *language,
			   const gchar            *id,
			   const gchar            *style,
			   const gchar            *pattern_start,
			   const gchar            *pattern_end,
			   gboolean                end_at_line_end)
{
	gboolean result;
	gchar *real_id, *root_id;
	gchar *freeme = NULL;
	GError *error = NULL;

	g_return_val_if_fail (id != NULL, FALSE);

	root_id = g_strdup_printf ("%s:%s", language->priv->id, language->priv->id);
	real_id = g_strdup_printf ("%s:%s", language->priv->id, id);

	/* XXX */
	if (pattern_end && g_str_has_suffix (pattern_end, "\\n"))
	{
		freeme = g_strndup (pattern_end, strlen (pattern_end) - 2);
		pattern_end = freeme;
		end_at_line_end = TRUE;
	}

	result = _gtk_source_context_engine_define_context (ce, real_id, root_id,
							    NULL,
							    pattern_start,
							    pattern_end,
							    style,
							    TRUE,
							    end_at_line_end,
							    &error);

	if (error)
	{
		g_warning ("%s", error->message);
		g_error_free (error);
	}

	g_free (real_id);
	g_free (root_id);
	g_free (freeme);

	return result;
}

static gchar *
build_keyword_list (const gchar  *id,
		    const GSList *keywords,
		    gboolean      case_sensitive,
		    gboolean      match_empty_string_at_beginning,
		    gboolean      match_empty_string_at_end,
		    const gchar  *beginning_regex,
		    const gchar  *end_regex)
{
	GString *str;
	gint keyword_count;

	g_return_val_if_fail (keywords != NULL, NULL);

	str =  g_string_new ("");

	if (keywords != NULL)
	{
		if (match_empty_string_at_beginning)
			g_string_append (str, "\\b");

		if (beginning_regex != NULL)
			g_string_append (str, beginning_regex);

		if (case_sensitive)
			g_string_append (str, "(?:");
		else
			g_string_append (str, "(?i:");

		keyword_count = 0;
		/* Due to a bug in GNU libc regular expressions
		 * implementation we can't have keyword lists of more
		 * than 250 or so elements, so we truncate such a
		 * list.  This is a temporary solution, as the correct
		 * approach would be to generate multiple keyword
		 * lists.  (See bug #110991) */
		/* we are using pcre now, and it can handle more */
#define KEYWORD_LIMIT 1000

		while (keywords != NULL && keyword_count < KEYWORD_LIMIT)
		{
			g_string_append (str, (gchar*)keywords->data);

			keywords = g_slist_next (keywords);
			keyword_count++;

			if (keywords != NULL && keyword_count < KEYWORD_LIMIT)
				g_string_append (str, "|");
		}
		g_string_append (str, ")");

		if (keyword_count >= KEYWORD_LIMIT)
		{
			g_warning ("Keyword list '%s' too long. Only the first %d "
				   "elements will be highlighted. See bug #110991 for "
				   "further details.", id, KEYWORD_LIMIT);
		}

		if (end_regex != NULL)
			g_string_append (str, end_regex);

		if (match_empty_string_at_end)
			g_string_append (str, "\\b");
	}

	return g_string_free (str, FALSE);
}

static gchar *
language_strconvescape (gchar *source)
{
	gunichar cur_char;
	gunichar last_char = 0;
	gchar *dest;
	gchar *src;

	if (source == NULL)
		return NULL;

	src = dest = source;

	while (*src)
	{
		cur_char = g_utf8_get_char (src);
		src = g_utf8_next_char (src);
		if (last_char == '\\' && cur_char == 'n') {
			cur_char = '\n';
		} else if (last_char == '\\' && cur_char == 't') {
			cur_char = '\t';
		}
		dest += g_unichar_to_utf8 (cur_char, dest);
		last_char = cur_char;
	}
	*dest = '\0';

	return source;
}

static void
parseLineComment (xmlNodePtr              cur,
		  gchar                  *id,
		  xmlChar                *style,
		  GtkSourceContextEngine *ce,
		  GtkSourceLanguage      *language)
{
	xmlNodePtr child;

	child = cur->xmlChildrenNode;

	if ((child != NULL) && !xmlStrcmp (child->name, (const xmlChar *)"start-regex"))
	{
		xmlChar *start_regex;

		start_regex = xmlNodeListGetString (child->doc, child->xmlChildrenNode, 1);

		engine_add_syntax_pattern (ce, language, id, (gchar*) style,
// 					   language_strconvescape ((gchar*) start_regex),
					   (gchar*) start_regex,
					   NULL, TRUE);

		xmlFree (start_regex);
	}
	else
	{
		g_warning ("Missing start-regex in tag 'line-comment' (%s, line %ld)",
			   child->doc->name, xmlGetLineNo (child));
	}
}

static void
parseBlockComment (xmlNodePtr              cur,
		   gchar                  *id,
		   xmlChar                *style,
		   GtkSourceContextEngine *ce,
		   GtkSourceLanguage      *language)
{
	xmlChar *start_regex = NULL;
	xmlChar *end_regex = NULL;

	xmlNodePtr child;

	child = cur->xmlChildrenNode;

	while (child != NULL)
	{
		if (!xmlStrcmp (child->name, (const xmlChar *)"start-regex"))
		{
			start_regex = xmlNodeListGetString (child->doc, child->xmlChildrenNode, 1);
		}
		else
		if (!xmlStrcmp (child->name, (const xmlChar *)"end-regex"))
		{
			end_regex = xmlNodeListGetString (child->doc, child->xmlChildrenNode, 1);
		}

		child = child->next;
	}

	if (start_regex == NULL)
	{
		g_warning ("Missing start-regex in tag 'block-comment' (%s, line %ld)",
			   child->doc->name, xmlGetLineNo (cur));

		return;
	}

	if (end_regex == NULL)
	{
		xmlFree (start_regex);

		g_warning ("Missing end-regex in tag 'block-comment' (%s, line %ld)",
			   child->doc->name, xmlGetLineNo (cur));

		return;
	}

	engine_add_syntax_pattern (ce, language, id, (gchar*) style,
// 				   language_strconvescape ((gchar*) start_regex),
// 				   language_strconvescape ((gchar*) end_regex),
				   (gchar*) start_regex,
				   (gchar*) end_regex,
				   FALSE);

	xmlFree (start_regex);
	xmlFree (end_regex);
}

static void
parseString (xmlNodePtr              cur,
	     gchar                  *id,
	     xmlChar                *style,
	     GtkSourceContextEngine *ce,
	     GtkSourceLanguage      *language)
{
	xmlChar *start_regex = NULL;
	xmlChar *end_regex = NULL;

	xmlChar *prop = NULL;
	gboolean end_at_line_end = TRUE;

	xmlNodePtr child;

	prop = xmlGetProp (cur, BAD_CAST "end-at-line-end");
	if (prop != NULL)
	{
		if (!xmlStrcasecmp (prop, (const xmlChar *)"TRUE") ||
		    !xmlStrcmp (prop, (const xmlChar *)"1"))

				end_at_line_end = TRUE;
			else
				end_at_line_end = FALSE;

		xmlFree (prop);
	}

	child = cur->xmlChildrenNode;

	while (child != NULL)
	{
		if (!xmlStrcmp (child->name, (const xmlChar *)"start-regex"))
		{
			start_regex = xmlNodeListGetString (child->doc, child->xmlChildrenNode, 1);
		}
		else
		if (!xmlStrcmp (child->name, (const xmlChar *)"end-regex"))
		{
			end_regex = xmlNodeListGetString (child->doc, child->xmlChildrenNode, 1);
		}

		child = child->next;
	}

	if (start_regex == NULL)
	{
		g_warning ("Missing start-regex in tag 'string' (%s, line %ld)",
			   child->doc->name, xmlGetLineNo (cur));

		return;
	}

	if (end_regex == NULL)
	{
		xmlFree (start_regex);

		g_warning ("Missing end-regex in tag 'string' (%s, line %ld)",
			   child->doc->name, xmlGetLineNo (cur));

		return;
	}

// 	language_strconvescape ((gchar*) start_regex);
// 	language_strconvescape ((gchar*) end_regex);

	engine_add_syntax_pattern (ce, language, id,
				   (gchar*) style,
				   (gchar*) start_regex,
				   (gchar*) end_regex,
				   end_at_line_end);

	xmlFree (start_regex);
	xmlFree (end_regex);
}

static void
parseKeywordList (xmlNodePtr              cur,
		  gchar                  *id,
		  xmlChar                *style,
		  GtkSourceContextEngine *ce,
		  GtkSourceLanguage      *language)
{
	gboolean case_sensitive = TRUE;
	gboolean match_empty_string_at_beginning = TRUE;
	gboolean match_empty_string_at_end = TRUE;
	gchar  *beginning_regex = NULL;
	gchar  *end_regex = NULL;

	GSList *list = NULL;
	gchar *regex;

	xmlChar *prop;

	xmlNodePtr child;

	prop = xmlGetProp (cur, BAD_CAST "case-sensitive");
	if (prop != NULL)
	{
		if (!xmlStrcasecmp (prop, (const xmlChar *)"TRUE") ||
		    !xmlStrcmp (prop, (const xmlChar *)"1"))

				case_sensitive = TRUE;
			else
				case_sensitive = FALSE;

		xmlFree (prop);
	}

	prop = xmlGetProp (cur, BAD_CAST "match-empty-string-at-beginning");
	if (prop != NULL)
	{
		if (!xmlStrcasecmp (prop, (const xmlChar *)"TRUE") ||
		    !xmlStrcmp (prop, (const xmlChar *)"1"))

				match_empty_string_at_beginning = TRUE;
			else
				match_empty_string_at_beginning = FALSE;

		xmlFree (prop);
	}

	prop = xmlGetProp (cur, BAD_CAST "match-empty-string-at-end");
	if (prop != NULL)
	{
		if (!xmlStrcasecmp (prop, (const xmlChar *)"TRUE") ||
		    !xmlStrcmp (prop, (const xmlChar *)"1"))

				match_empty_string_at_end = TRUE;
			else
				match_empty_string_at_end = FALSE;

		xmlFree (prop);
	}

	prop = xmlGetProp (cur, BAD_CAST "beginning-regex");
	if (prop != NULL)
	{
		beginning_regex = g_strdup ((gchar *)prop);

		xmlFree (prop);
	}

	prop = xmlGetProp (cur, BAD_CAST "end-regex");
	if (prop != NULL)
	{
		end_regex = g_strdup ((gchar *)prop);

		xmlFree (prop);
	}

	child = cur->xmlChildrenNode;

	while (child != NULL)
	{
		if (!xmlStrcmp (child->name, BAD_CAST "keyword"))
		{
			xmlChar *keyword;
			keyword = xmlNodeListGetString (child->doc, child->xmlChildrenNode, 1);

			list = g_slist_prepend (list,
						language_strconvescape ((gchar*) keyword));
		}

		child = child->next;
	}

	list = g_slist_reverse (list);

	if (list == NULL)
	{
		g_warning ("No keywords in tag 'keyword-list' (%s, line %ld)",
			   child->doc->name, xmlGetLineNo (cur));

		g_free (beginning_regex),
		g_free (end_regex);

		return;
	}

	regex = build_keyword_list (id,
				    list,
				    case_sensitive,
				    match_empty_string_at_beginning,
				    match_empty_string_at_end,
// 				    language_strconvescape (beginning_regex),
// 				    language_strconvescape (end_regex));
				    beginning_regex,
				    end_regex);

	g_free (beginning_regex),
	g_free (end_regex);

	g_slist_foreach (list, (GFunc) xmlFree, NULL);
	g_slist_free (list);

	engine_add_simple_pattern (ce, language, id, (gchar*) style, regex);

	g_free (regex);
}

static void
parsePatternItem (xmlNodePtr              cur,
		  gchar                  *id,
		  xmlChar                *style,
		  GtkSourceContextEngine *ce,
		  GtkSourceLanguage      *language)
{
	xmlNodePtr child;

	child = cur->xmlChildrenNode;

	if ((child != NULL) && !xmlStrcmp (child->name, (const xmlChar *)"regex"))
	{
		xmlChar *regex;

		regex = xmlNodeListGetString (child->doc, child->xmlChildrenNode, 1);

		engine_add_simple_pattern (ce, language, id, (gchar*) style,
// 					   language_strconvescape ((gchar*) regex));
					   (gchar*) regex);

		xmlFree (regex);
	}
	else
	{
		g_warning ("Missing regex in tag 'pattern-item' (%s, line %ld)",
			   child->doc->name, xmlGetLineNo (child));
	}
}

static void
parseSyntaxItem (xmlNodePtr              cur,
		 const gchar            *id,
		 xmlChar                *style,
		 GtkSourceContextEngine *ce,
		 GtkSourceLanguage      *language)
{
	xmlChar *start_regex = NULL;
	xmlChar *end_regex = NULL;

	xmlNodePtr child;

	child = cur->xmlChildrenNode;

	while (child != NULL)
	{
		if (!xmlStrcmp (child->name, (const xmlChar *)"start-regex"))
		{
			start_regex = xmlNodeListGetString (child->doc, child->xmlChildrenNode, 1);
		}
		else
		if (!xmlStrcmp (child->name, (const xmlChar *)"end-regex"))
		{
			end_regex = xmlNodeListGetString (child->doc, child->xmlChildrenNode, 1);
		}

		child = child->next;
	}

	if (start_regex == NULL)
	{
		g_warning ("Missing start-regex in tag 'syntax-item' (%s, line %ld)",
			   child->doc->name, xmlGetLineNo (cur));

		return;
	}

	if (end_regex == NULL)
	{
		xmlFree (start_regex);

		g_warning ("Missing end-regex in tag 'syntax-item' (%s, line %ld)",
			   child->doc->name, xmlGetLineNo (cur));

		return;
	}

	engine_add_syntax_pattern (ce, language, id, (gchar*) style,
// 				   language_strconvescape ((gchar*) start_regex),
// 				   language_strconvescape ((gchar*) end_regex),
				   (gchar*) start_regex,
				   (gchar*) end_regex,
				   FALSE);

	xmlFree (start_regex);
	xmlFree (end_regex);
}

static void
parseTag (GtkSourceLanguage      *language,
	  xmlNodePtr              cur,
	  GtkSourceContextEngine *ce)
{
	xmlChar *name;
	xmlChar *style;
	xmlChar *id;

	name = xmlGetProp (cur, BAD_CAST "_name");
	if (name == NULL)
	{
		name = xmlGetProp (cur, BAD_CAST "name");
		id = xmlStrdup (name);
	}
	else
	{
		xmlChar *tmp = xmlStrdup (BAD_CAST dgettext (
							language->priv->translation_domain,
							(gchar *)name));
		id = name;
		name = tmp;
	}

	if (name == NULL)
	{
		return;
	}

	style = xmlGetProp (cur, BAD_CAST "style");

	if (style == NULL)
	{
		/* FIXME */
		style = xmlStrdup (BAD_CAST "Normal");
	}

	if (!xmlStrcmp (cur->name, (const xmlChar*) "line-comment"))
	{
		parseLineComment (cur, (gchar*) id, style, ce, language);
	}
	else if (!xmlStrcmp (cur->name, (const xmlChar*) "block-comment"))
	{
		parseBlockComment (cur, (gchar*) id, style, ce, language);
	}
	else if (!xmlStrcmp (cur->name, (const xmlChar*) "string"))
	{
		parseString (cur, (gchar*) id, style, ce, language);
	}
	else if (!xmlStrcmp (cur->name, (const xmlChar*) "keyword-list"))
	{
		parseKeywordList (cur, (gchar*) id, style, ce, language);
	}
	else if (!xmlStrcmp (cur->name, (const xmlChar*) "pattern-item"))
	{
		parsePatternItem (cur, (gchar*) id, style, ce, language);
	}
	else if (!xmlStrcmp (cur->name, (const xmlChar*) "syntax-item"))
	{
		parseSyntaxItem (cur, (gchar*) id, style, ce, language);
	}
	else
	{
		g_print ("Unknown tag: %s\n", cur->name);
	}

// 	if (populate_styles_table)
// 		g_hash_table_insert (language->priv->tag_id_to_style_name,
// 				     g_strdup ((gchar *)id),
// 				     g_strdup ((gchar *)style));

// 	if (tags)
// 	{
// 		GtkTextTag *tag;
// 		GtkSourceTagStyle *ts;
//
// 		tag = gtk_source_tag_new ((gchar *)id, (gchar *)name);
// 		*tags = g_slist_prepend (*tags, tag);
//
// 		ts = gtk_source_language_get_tag_style (language, (gchar *)id);
//
// 		if (ts != NULL)
// 		{
// 			gtk_source_tag_set_style (GTK_SOURCE_TAG (tag), ts);
// 			gtk_source_tag_style_free (ts);
// 		}
//
// 	}

	xmlFree (name);
	xmlFree (style);
	xmlFree (id);
}

static gboolean
define_root_context (GtkSourceContextEngine *ce,
		     GtkSourceLanguage      *language)
{
	gboolean result;
	gchar *id;
	GError *error = NULL;

	g_return_val_if_fail (language->priv->id != NULL, FALSE);

	id = g_strdup_printf ("%s:%s", language->priv->id, language->priv->id);
	result = _gtk_source_context_engine_define_context (ce, id,
							    NULL, NULL, NULL, NULL,
							    NULL, TRUE, FALSE,
							    &error);

	if (error)
	{
		g_warning ("%s", error->message);
		g_error_free (error);
	}

	g_free (id);
	return result;
}

gboolean
_gtk_source_language_file_parse_version1 (GtkSourceLanguage      *language,
					  GtkSourceContextEngine *engine)
{
	xmlDocPtr doc;
	xmlNodePtr cur;
	GMappedFile *mf;
	gunichar esc_char = 0;

	xmlKeepBlanksDefault (0);

	mf = g_mapped_file_new (language->priv->lang_file_name, FALSE, NULL);

	if (mf == NULL)
	{
		doc = NULL;
	}
	else
	{
		doc = xmlParseMemory (g_mapped_file_get_contents (mf),
				      g_mapped_file_get_length (mf));

		g_mapped_file_free (mf);
	}

	if (doc == NULL)
	{
		g_warning ("Impossible to parse file '%s'",
			   language->priv->lang_file_name);
		return FALSE;
	}

	cur = xmlDocGetRootElement (doc);

	if (cur == NULL)
	{
		g_warning ("The lang file '%s' is empty",
			   language->priv->lang_file_name);

		xmlFreeDoc (doc);
		return FALSE;
	}

	if (xmlStrcmp (cur->name, (const xmlChar *) "language"))
	{
		g_warning ("File '%s' is of the wrong type",
			   language->priv->lang_file_name);

		xmlFreeDoc (doc);
		return FALSE;
	}

	if (!define_root_context (engine, language))
	{
		g_warning ("Could not create root context for file '%s'",
			   language->priv->lang_file_name);

		xmlFreeDoc (doc);
		return FALSE;
	}

	/* FIXME: check that the language name, version, etc. are the
	 * right ones - Paolo */

	cur = xmlDocGetRootElement (doc);
	cur = cur->xmlChildrenNode;
	g_return_val_if_fail (cur != NULL, FALSE);

	while (cur != NULL)
	{
		if (!xmlStrcmp (cur->name, (const xmlChar *)"escape-char"))
		{
			xmlChar *escape;

			escape = xmlNodeListGetString (doc, cur->xmlChildrenNode, 1);
			esc_char = g_utf8_get_char_validated ((gchar*) escape, -1);

			if (esc_char == (gunichar) -1 || esc_char == (gunichar) -2)
			{
				g_warning ("Invalid (non UTF8) escape character in file '%s'",
					   language->priv->lang_file_name);
				esc_char = 0;
			}

			xmlFree (escape);
		}
		else
		{
			parseTag (language,
				  cur,
				  engine);
		}

		cur = cur->next;
	}

	if (esc_char != 0)
		_gtk_source_context_engine_set_escape_char (engine, esc_char);

	_gtk_source_language_define_language_styles (language);

	xmlFreeDoc (doc);
	return TRUE;
}

