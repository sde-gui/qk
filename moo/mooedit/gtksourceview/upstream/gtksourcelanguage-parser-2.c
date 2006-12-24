/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*-
 *  gtksourcelanguage-parser-2.c
 *  Language specification parser for 2.0 version .lang files
 *
 *  Copyright (C) 2003 - Gustavo Giráldez <gustavo.giraldez@gmx.net>
 *  Copyright (C) 2005, 2006 - Emanuele Aina, Marco Barisione
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

#undef ENABLE_DEBUG

#ifdef ENABLE_DEBUG
#define DEBUG(x) x
#else
#define DEBUG(x)
#endif

#include <string.h>
#include <fcntl.h>
#include <libxml/xmlreader.h>
#include <glib/gstdio.h>
#include "gtksourceview-i18n.h"
#include "gtksourcebuffer.h"
#include "gtksourcelanguage.h"
#include "gtksourcelanguagesmanager.h"
#include "gtksourcelanguage-private.h"
#include "gtksourcecontextengine.h"
#include <eggregex.h>

#define PARSER_ERROR (parser_error_quark ())
#define ATTR_NO_STYLE ""

typedef enum {
	PARSER_ERROR_CANNOT_OPEN     = 0,
	PARSER_ERROR_CANNOT_VALIDATE,
	PARSER_ERROR_INVALID_DOC,
	PARSER_ERROR_WRONG_VERSION,
	PARSER_ERROR_WRONG_ID,
	PARSER_ERROR_WRONG_STYLE,
	PARSER_ERROR_MALFORMED_REGEX,
	PARSER_ERROR_MALFORMED_MAP_TO
} ParserError;

struct _ParserState
{
	/* The args passed to _file_parse_version2() */
	xmlTextReader *reader;
	GtkSourceLanguage *language;
	GtkSourceContextData *ctx_data;

	/* A stack of id that representing parent contexts */
	GQueue *curr_parents;

	/* The id of the current language (used to decorate ids) */
	gchar *current_lang_id;

	/* An hash table with the defined regex as strings used to
	 * resolve references (the key is the id) */
	GHashTable *defined_regexes;

	/* The mapping between style ids and their default styles.
	 * If lang file contains this:
	 * <style id="foo" map-to="def:blah"/>
	 * <style id="bar"/>
	 * then in styles_mapping: "foo"->"def:blah", "bar"->"bar". */
	GHashTable *styles_mapping;

	/* The list of loaded languages (the item are XmlChar pointers) */
	GSList **loaded_lang_ids;

	/* A serial number incremented to get unique generated names */
	guint id_cookie;

	/* The default flags used by the regexes */
	EggRegexCompileFlags regex_compile_flags;

	gchar *opening_delimiter;
	gchar *closing_delimiter;
};
typedef struct _ParserState ParserState;



static GQuark     parser_error_quark           (void);
static gboolean   str_to_bool                  (const xmlChar *string);
static gchar     *generate_new_id              (ParserState *parser_state);
static gboolean   id_is_decorated              (const gchar *id,
                                                gchar **lang_id);
static gchar     *decorate_id                  (ParserState *parser_state,
                                                const gchar *id);

static ParserState *parser_state_new           (GtkSourceLanguage      *language,
                                                GtkSourceContextData   *ctx_data,
                                                GHashTable             *defined_regexes,
                                                GHashTable             *styles_mapping,
                                                xmlTextReader          *reader,
                                                GSList                **loaded_lang_ids);
static void       parser_state_destroy         (ParserState *parser_state);

static gboolean   file_parse                   (gchar                  *filename,
                                                GtkSourceLanguage      *language,
                                                GtkSourceContextData   *ctx_data,
                                                GHashTable             *defined_regexes,
                                                GHashTable             *styles,
                                                GSList                **loaded_lang_ids,
                                                GError                **error);

static EggRegexCompileFlags
		  update_regex_flags	       (EggRegexCompileFlags flags,
						const gchar   *option_name,
						const xmlChar *bool_value);

static gboolean   create_definition            (ParserState *parser_state,
                                                gchar *id,
                                                gchar *parent_id,
                                                gchar *style,
						GError **error);

static void       handle_context_element       (ParserState *parser_state,
                                                GError **error);
static void       handle_language_element      (ParserState *parser_state,
                                                GError **error);
static void       handle_define_regex_element  (ParserState *parser_state,
                                                GError **error);
static void       handle_default_regex_options_element
                                               (ParserState *parser_state,
                                                GError **error);
static void       element_start                (ParserState *parser_state,
						GError **error);
static void       element_end                  (ParserState *parser_state);
static gboolean   replace_by_id                (const EggRegex *egg_regex,
                                                const gchar *regex,
                                                GString *expanded_regex,
                                                gpointer data);
static gchar     *expand_regex                 (ParserState *parser_state,
                                                gchar *regex,
						EggRegexCompileFlags flags,
						gboolean do_expand_vars,
						gboolean insert_parentheses,
						GError **error);

static GQuark
parser_error_quark (void)
{
	static GQuark err_q = 0;
	if (err_q == 0)
		err_q = g_quark_from_static_string (
			"parser-error-quark");

	return err_q;
}

static gboolean
str_to_bool (const xmlChar *string)
{
	g_return_val_if_fail (string != NULL, FALSE);
	return g_ascii_strcasecmp ("true", (const gchar *) string) == 0;
}

static gchar *
generate_new_id (ParserState *parser_state)
{
	gchar *id;

	id = g_strdup_printf ("unnamed-%u", parser_state->id_cookie);
	parser_state->id_cookie++;

	DEBUG (g_message ("generated id %s", id));

	return id;
}

static gboolean
id_is_decorated (const gchar *id,
		 gchar      **lang_id)
{
	/* This function is quite simple because the XML validator check for
	 * the correctness of the id with a regex */

	gchar **tokens;
	gboolean is_decorated;

	tokens = g_strsplit (id, ":", 2);

	g_return_val_if_fail (tokens != NULL, FALSE);

	if (tokens [1] == NULL)
	{
		/* There is no ":" in the id */
		is_decorated = FALSE;
	}
	else if (tokens [1] != NULL && strcmp ("*", tokens[1]) == 0)
	{
		/* This is an undecorated "import all", and not a decorated
		 * reference */
		is_decorated = FALSE;
	}
	else
	{
		is_decorated = TRUE;
		if (lang_id != NULL)
		{
			*lang_id = g_strdup (tokens[0]);
		}
	}

	g_strfreev (tokens);

	return is_decorated;
}

static gchar *
decorate_id (ParserState *parser_state,
	     const gchar *id)
{
	gchar *decorated_id;

	decorated_id = g_strdup_printf ("%s:%s",
			parser_state->current_lang_id, id);

	DEBUG (g_message ("decorated '%s' to '%s'", id, decorated_id));

	return decorated_id;
}

static gboolean
lang_id_is_already_loaded (ParserState *parser_state, gchar *lang_id)
{
	GSList *l;
	gchar *loaded_lang;

	l = *(parser_state->loaded_lang_ids);

	g_return_val_if_fail (lang_id != NULL, FALSE);

	while (l != NULL)
	{
		loaded_lang = l->data;
		if (strcmp (loaded_lang, lang_id) == 0)
		{
			return TRUE;
		}
		l = g_slist_next (l);
	}
	return FALSE;
}


static EggRegexCompileFlags
get_regex_flags (xmlNode             *node,
		 EggRegexCompileFlags flags)
{
	xmlAttr *attribute;

	for (attribute = node->properties; attribute != NULL; attribute = attribute->next)
	{
		g_return_val_if_fail (attribute->children != NULL, flags);

		if (xmlStrcmp (BAD_CAST "extended", attribute->name) == 0)
		{
			flags = update_regex_flags (flags, "extended",
						    attribute->children->content);
		}
		else if (xmlStrcmp (BAD_CAST "case-insensitive", attribute->name) == 0)
		{
			flags = update_regex_flags (flags, "case-insensitive",
						    attribute->children->content);
		}
		else if (xmlStrcmp (BAD_CAST "dot-match-all", attribute->name) == 0)
		{
			flags = update_regex_flags (flags, "dot-match-all",
						    attribute->children->content);
		}
	}

	return flags;
}

static gboolean
create_definition (ParserState *parser_state,
		   gchar       *id,
		   gchar       *parent_id,
		   gchar       *style,
		   GError     **error)
{
	gchar *match = NULL, *start = NULL, *end = NULL;
	gchar *prefix = NULL, *suffix = NULL;
	xmlChar *tmp;
	gboolean extend_parent = TRUE;
	gboolean end_at_line_end = FALSE;
	GtkSourceContextMatchOptions options = 0;

	xmlNode *context_node, *child;

	GString *all_items = NULL;

	EggRegexCompileFlags match_flags = 0, start_flags = 0, end_flags = 0;

	GError *tmp_error = NULL;

	g_assert (parser_state->ctx_data != NULL);

	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	/* extend-parent */
	tmp = xmlTextReaderGetAttribute (parser_state->reader, BAD_CAST "extend-parent");
	if (tmp != NULL)
		extend_parent = str_to_bool (tmp);
	xmlFree (tmp);

	/* end-at-line-end */
	tmp = xmlTextReaderGetAttribute (parser_state->reader, BAD_CAST "end-at-line-end");
	if (tmp != NULL)
		end_at_line_end = str_to_bool (tmp);
	xmlFree (tmp);

	DEBUG (g_message ("creating context %s, child of %s", id, parent_id ? parent_id : "(null)"));

	/* Fetch the content of the sublements using the tree API on
	 * the current node */
	context_node = xmlTextReaderExpand (parser_state->reader);

	/* The file should be validated so this should not happen */
	g_assert (context_node != NULL);

	for (child = context_node->children; child != NULL; child = child->next)
	{
		if (child->type != XML_READER_TYPE_ELEMENT)
			continue;

		/* FIXME: add PCRE_EXTRA support in EggRegex */

		g_assert (child->name);
		if (xmlStrcmp (BAD_CAST "match", child->name) == 0
				&& child->children != NULL)
		{
			/* <match> */
			match = g_strdup ((gchar *)child->children->content);
			match_flags = get_regex_flags (child, parser_state->regex_compile_flags);
		}
		else if (xmlStrcmp (BAD_CAST "start", child->name) == 0)
		{
			/* <start> */
			if (child->children != NULL)
			{
				start = g_strdup ((gchar *)child->children->content);
			}
			else
			{
				/* If the <start> element is present but
				 * has no content use an empty string */
				start = g_strdup ("");
			}
			start_flags = get_regex_flags (child, parser_state->regex_compile_flags);
		}
		else if (xmlStrcmp (BAD_CAST "end", child->name) == 0)
		{
			/* <end> */
			if (child->children != NULL)
				end = g_strdup ((gchar *)child->children->content);
			else
			{
				/* If the <end> element is present but
				 * has no content use an empty string */
				end = g_strdup ("");
			}
			end_flags = get_regex_flags (child, parser_state->regex_compile_flags);
		}
		else if (xmlStrcmp (BAD_CAST "prefix", child->name) == 0)
		{
			/* <prefix> */
			if (child->children != NULL)
				prefix = g_strdup ((gchar*) child->children->content);
			else
				prefix = g_strdup ("");
		}
		else if (xmlStrcmp (BAD_CAST "suffix", child->name) == 0)
		{
			/* <suffix> */
			if (child->children != NULL)
				suffix = g_strdup ((gchar*) child->children->content);
			else
				suffix = g_strdup ("");
		}
		else if (xmlStrcmp (BAD_CAST "keyword", child->name) == 0 &&
			 child->children != NULL)
		{
			/* FIXME: how to specify regex options for keywords?
			 * They can be specified in prefix, so it's not really
			 * important, but would be nice (case-sensitive). */

			/* <keyword> */
			if (all_items == NULL)
			{
				all_items = g_string_new (NULL);

				if (prefix != NULL)
					g_string_append (all_items, prefix);
				else
					g_string_append (all_items,
							 parser_state->opening_delimiter);

				g_string_append (all_items, "(");
				g_string_append (all_items, (gchar*) child->children->content);
			}
			else
			{
				g_string_append (all_items, "|");
				g_string_append (all_items, (gchar*) child->children->content);
			}
		}
	}

	if (all_items != NULL)
	{
		g_string_append (all_items, ")");

		if (suffix != NULL)
			g_string_append (all_items, suffix);
		else
			g_string_append (all_items,
					 parser_state->closing_delimiter);

		match = g_string_free (all_items, FALSE);
		match_flags = parser_state->regex_compile_flags;
	}

	DEBUG (g_message ("start: '%s'", start ? start : "(null)"));
	DEBUG (g_message ("end: '%s'", end ? end : "(null)"));
	DEBUG (g_message ("match: '%s'", match ? match : "(null)"));


	if (tmp_error == NULL && start != NULL)
	{
		gchar *tmp = start;
		start = expand_regex (parser_state, start, start_flags,
				      TRUE, FALSE, &tmp_error);
		g_free (tmp);
	}
	if (tmp_error == NULL && end != NULL)
	{
		gchar *tmp = end;
		end = expand_regex (parser_state, end, end_flags,
				    TRUE, FALSE, &tmp_error);
		g_free (tmp);
	}
	if (tmp_error == NULL && match != NULL)
	{
		gchar *tmp = match;
		match = expand_regex (parser_state, match, match_flags,
				      TRUE, FALSE, &tmp_error);
		g_free (tmp);
	}

	if (extend_parent)
		options |= GTK_SOURCE_CONTEXT_EXTEND_PARENT;
	if (end_at_line_end)
		options |= GTK_SOURCE_CONTEXT_END_AT_LINE_END;

	if (tmp_error == NULL)
		_gtk_source_context_data_define_context (parser_state->ctx_data,
							 id,
							 parent_id,
							 match,
							 start,
							 end,
							 style,
							 options,
							 &tmp_error);

	g_free (match);
	g_free (start);
	g_free (end);
	g_free (prefix);
	g_free (suffix);

	if (tmp_error != NULL)
	{
		g_propagate_error (error, tmp_error);
		return FALSE;
	}

	return TRUE;
}

static gboolean
add_ref (ParserState               *parser_state,
	 const gchar               *ref,
	 GtkSourceContextRefOptions options,
	 const gchar               *style,
	 GError	                  **error)
{
	gchar *container_id;
	gboolean all = FALSE;
	gchar *ref_id;
	gchar *lang_id;

	GtkSourceLanguagesManager *lm;
	GtkSourceLanguage *imported_language;

	GError *tmp_error = NULL;

	/* Return if an error is already set */
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	if (id_is_decorated (ref, &lang_id))
	{
		if (!lang_id_is_already_loaded (parser_state, lang_id))
		{
			lm = _gtk_source_language_get_languages_manager (parser_state->language);
			imported_language = gtk_source_languages_manager_get_language_by_id (lm, lang_id);

			if (imported_language == NULL)
			{
				g_set_error (&tmp_error,
						PARSER_ERROR,
						PARSER_ERROR_WRONG_ID,
						"unable to resolve language '%s' in ref '%s'",
						lang_id, ref);
			}
			else
			{
				file_parse (imported_language->priv->lang_file_name,
					    parser_state->language,
					    parser_state->ctx_data,
					    parser_state->defined_regexes,
					    parser_state->styles_mapping,
					    parser_state->loaded_lang_ids,
					    &tmp_error);

				if (tmp_error != NULL)
				{
					GError *tmp_error2 = NULL;
					g_set_error (&tmp_error2, PARSER_ERROR, tmp_error->code,
						     "In file '%s' referenced from '%s': %s",
						     imported_language->priv->lang_file_name,
						     parser_state->language->priv->lang_file_name,
						     tmp_error->message);
					g_clear_error (&tmp_error);
					tmp_error = tmp_error2;
				}
			}
		}
		ref_id = g_strdup (ref);
		g_free (lang_id);
	}
	else
	{
		ref_id = decorate_id (parser_state, ref);
	}

	if (tmp_error == NULL && parser_state->ctx_data != NULL)
	{
		if (g_str_has_suffix (ref, ":*"))
		{
			all = TRUE;
			ref_id [strlen (ref_id) - 2] = '\0';
		}

		if (all && (options & (GTK_SOURCE_CONTEXT_IGNORE_STYLE | GTK_SOURCE_CONTEXT_OVERRIDE_STYLE)))
		{
			g_set_error (&tmp_error, PARSER_ERROR,
				     PARSER_ERROR_WRONG_STYLE,
				     "style override used with wildcard context reference"
				     " in language '%s' in ref '%s'",
				     lang_id, ref);
		}
	}

	if (tmp_error == NULL && parser_state->ctx_data != NULL)
	{
		container_id = g_queue_peek_head (parser_state->curr_parents);

		/* If the document is validated container_id is never NULL */
		g_assert (container_id);

		_gtk_source_context_data_add_ref (parser_state->ctx_data,
						  container_id,
						  ref_id,
						  options,
						  style,
						  all,
						  &tmp_error);

		DEBUG (g_message ("appended %s in %s", ref_id, container_id));
	}

	g_free (ref_id);

	if (tmp_error != NULL)
	{
		g_propagate_error (error, tmp_error);
		return FALSE;
	}

	return TRUE;
}

static gboolean
create_sub_pattern (ParserState *parser_state,
                    gchar *id,
                    gchar *sub_pattern,
                    gchar *style,
                    GError **error)
{
	gchar *container_id;
	xmlChar *where;

	GError *tmp_error = NULL;

	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	container_id = g_queue_peek_head (parser_state->curr_parents);

	/* If the document is validated container is never NULL */
	g_assert (container_id);

	where = xmlTextReaderGetAttribute (parser_state->reader, BAD_CAST "where");

	_gtk_source_context_data_add_sub_pattern (parser_state->ctx_data,
						  id,
						  container_id,
						  sub_pattern,
						  (gchar*) where,
						  style,
						  &tmp_error);

	xmlFree (where);

	if (tmp_error != NULL)
	{
		g_propagate_error (error, tmp_error);
		return FALSE;
	}

	return TRUE;
}

static void
handle_context_element (ParserState *parser_state,
			GError **error)
{
	gchar *id, *parent_id, *style_ref;
	xmlChar *ref, *sub_pattern, *tmp;
	int is_empty;
	gboolean success;
	gboolean ignore_style = FALSE;
	GtkSourceContextRefOptions options = 0;

	GError *tmp_error = NULL;

	g_return_if_fail (error != NULL && *error == NULL);

	ref = xmlTextReaderGetAttribute (parser_state->reader, BAD_CAST "ref");
	sub_pattern = xmlTextReaderGetAttribute (parser_state->reader,
			BAD_CAST "sub-pattern");

	tmp = xmlTextReaderGetAttribute (parser_state->reader, BAD_CAST "ignore-style");
	if (tmp != NULL && str_to_bool (tmp))
		ignore_style = TRUE;
	xmlFree (tmp);

	tmp = xmlTextReaderGetAttribute (parser_state->reader, BAD_CAST "style-ref");
	if (tmp == NULL || id_is_decorated ((gchar*) tmp, NULL))
		style_ref = g_strdup ((gchar*) tmp);
	else
		style_ref = decorate_id (parser_state, (gchar*) tmp);
	xmlFree (tmp);

	if (ignore_style && ref == NULL)
	{
		g_set_error (error, PARSER_ERROR,
			     PARSER_ERROR_WRONG_STYLE,
			     "ignore-style used not in a reference to context");

		xmlFree (ref);
		g_free (style_ref);

		return;
	}

	if (ignore_style)
	{
		options |= GTK_SOURCE_CONTEXT_IGNORE_STYLE;

		if (style_ref != NULL)
			g_warning ("style-ref and ignore-style used simultaneously");
	}

	if (!ignore_style && style_ref != NULL &&
	    g_hash_table_lookup (parser_state->styles_mapping, style_ref) == NULL)
	{
		g_warning ("style '%s' not defined", style_ref);
	}

	if (ref != NULL)
	{
		if (style_ref != NULL)
			options |= GTK_SOURCE_CONTEXT_OVERRIDE_STYLE;

		add_ref (parser_state, (gchar*) ref, options, style_ref, &tmp_error);
	}
	else
	{
		char *freeme = NULL;

		tmp = xmlTextReaderGetAttribute (parser_state->reader, BAD_CAST "id");
		if (tmp == NULL)
		{
			freeme = generate_new_id (parser_state);
			tmp = xmlStrdup (BAD_CAST freeme);
		}

		if (id_is_decorated ((gchar*) tmp, NULL))
			id = g_strdup ((gchar*) tmp);
		else
			id = decorate_id (parser_state, (gchar*) tmp);

		g_free (freeme);
		xmlFree (tmp);

		if (parser_state->ctx_data != NULL)
		{
			if (sub_pattern != NULL)
			{
				create_sub_pattern (parser_state, id,
						(gchar *)sub_pattern, style_ref,
						&tmp_error);
			}
			else
			{
				parent_id = g_queue_peek_head (
						parser_state->curr_parents);

				is_empty = xmlTextReaderIsEmptyElement (
						parser_state->reader);

				success = create_definition (parser_state, id, parent_id,
						style_ref, &tmp_error);

				if (success && !is_empty)
				{
					/* Push the new context in the curr_parents
					 * stack only if other contexts can be
					 * defined inside it */
					g_queue_push_head (parser_state->curr_parents,
							g_strdup (id));
				}
			}
		}

		g_free (id);
	}


	g_free (style_ref);
	xmlFree (sub_pattern);
	xmlFree (ref);

	if (tmp_error != NULL)
	{
		g_propagate_error (error, tmp_error);
		return;
	}
}

static void
handle_language_element (ParserState *parser_state,
			 GError **error)
{
	/* FIXME: check that the language name, version, etc. are the
	 * right ones - Paolo */
	xmlChar *lang_id, *lang_version;
	xmlChar *expected_version = BAD_CAST "2.0";

	g_return_if_fail (error != NULL && *error == NULL);

	lang_version = xmlTextReaderGetAttribute (parser_state->reader, BAD_CAST "version");

	if (lang_version == NULL ||
			(xmlStrcmp (expected_version, lang_version) != 0))
	{
		g_set_error (error,
			     PARSER_ERROR,
			     PARSER_ERROR_WRONG_VERSION,
			     "wrong language version '%s', expected '%s'",
			     lang_version ? (gchar*) lang_version : "(none)",
			     (gchar*) expected_version);
	}

	lang_id = xmlTextReaderGetAttribute (parser_state->reader, BAD_CAST "id");

	parser_state->current_lang_id = g_strdup ((gchar *)lang_id);

	*(parser_state->loaded_lang_ids) = g_slist_prepend (
			*(parser_state->loaded_lang_ids), lang_id);

	xmlFree (lang_version);
}


struct ReplaceByIdData {
	ParserState *parser_state;
	GError *error;
};

static gboolean
replace_by_id (const EggRegex *egg_regex,
	       const gchar    *regex,
	       GString        *expanded_regex,
	       gpointer        user_data)
{
	gchar *id, *subst, *escapes;
	gchar *tmp;
	GError *tmp_error = NULL;

	struct ReplaceByIdData *data = user_data;

	escapes = egg_regex_fetch (egg_regex, 1, regex);
	tmp = egg_regex_fetch (egg_regex, 2, regex);

	g_strstrip (tmp);

	if (id_is_decorated (tmp, NULL))
		id = g_strdup (tmp);
	else
		id = decorate_id (data->parser_state, tmp);
	g_free (tmp);

	subst = g_hash_table_lookup (data->parser_state->defined_regexes, id);
	if (subst == NULL)
		g_set_error (&tmp_error,
			     PARSER_ERROR, PARSER_ERROR_WRONG_ID,
			     _("Unknown id '%s' in regex '%s'"), id, regex);

	if (tmp_error == NULL)
	{
		g_string_append (expanded_regex, escapes);
		g_string_append (expanded_regex, subst);
	}

	g_free (escapes);
	g_free (id);

	if (tmp_error != NULL)
	{
		g_propagate_error (&data->error, tmp_error);
		return TRUE;
	}

	return FALSE;
}

static EggRegexCompileFlags
update_regex_flags (EggRegexCompileFlags flags,
		    const gchar         *option_name,
		    const xmlChar       *value)
{
	EggRegexCompileFlags single_flag;

	DEBUG (g_message ("setting the '%s' regex flag to %d", option_name, value));

	if (strcmp ("case-insensitive", option_name) == 0)
	{
		single_flag = EGG_REGEX_CASELESS;
	}
	else if (strcmp ("extended", option_name) == 0)
	{
		single_flag = EGG_REGEX_EXTENDED;
	}
	else if (strcmp ("dot-match-all", option_name) == 0)
	{
		single_flag = EGG_REGEX_DOTALL;
	}
	else
		return flags;

	if (str_to_bool (value))
		flags |= single_flag;
	else
		flags &= ~single_flag;

	return flags;
}

static gboolean
calc_regex_flags (const gchar *flags_string, EggRegexCompileFlags *flags)
{
	gboolean add = TRUE;
	while (*flags_string != '\0')
	{
		EggRegexCompileFlags single_flag = 0;
		switch (g_utf8_get_char (flags_string))
		{
			case '-':
				add = FALSE;
				break;
			case '+':
				add = TRUE;
				break;
			case 'i':
				single_flag = EGG_REGEX_CASELESS;
				break;
			case 'x':
				single_flag = EGG_REGEX_EXTENDED;
				break;
			case 's':
				/* FIXME: do we need this? */
				single_flag = EGG_REGEX_DOTALL;
				break;
			default:
				return FALSE;
		}
		if (single_flag != 0)
		{
			if (add)
				*flags |= single_flag;
			else
				*flags &= ~single_flag;
		}
		flags_string = g_utf8_next_char (flags_string);
	}

	return TRUE;
}

static gchar *
expand_regex_vars (ParserState *parser_state, gchar *regex, gint len, GError **error)
{
	/* This is the commented regex without the doubled escape needed
	 * in a C string:
	 *
	 * (?<!\\)(\\\\)*\\%\{([^@]*?)\}
	 * |------------||---||------||-|
	 *      |          |      |    |
	 *      |    the string   |   the ending
	 *      |       "\%{"     |   bracket "}"
	 *      |                 |
	 * a even sequence        |
	 * of escapes or      the id of the
	 * a char that        included regex
	 * is not an          (not greedy)
	 * escape (this
	 * is not matched)
	 *
	 * The first block is needed to ensure that the sequence is
	 * not escaped.
	 * Matches with an id containing a "@" sign are ignored and
	 * passed to the engine because they are references to subpatterns
	 * in a different regex (i.e. in the start regex while we are in the
	 * end regex.)
	 * The sub pattern containing the id is the second.
	 */
	const gchar *re = "(?<!\\\\)(\\\\\\\\)*\\\\%\\{([^@]*?)\\}";
	gchar *expanded_regex;
	EggRegex *egg_re;
	struct ReplaceByIdData data;

	if (regex == NULL)
		return NULL;

	egg_re = egg_regex_new (re, 0, 0, NULL);

	data.parser_state = parser_state;
	data.error = NULL;
	expanded_regex = egg_regex_replace_eval (egg_re, regex, len, 0, 0,
						 replace_by_id, &data, NULL);

	if (data.error == NULL)
        {
		DEBUG (g_message ("expanded regex vars '%s' to '%s'",
				  regex, expanded_regex));
        }

	egg_regex_free (egg_re);

	if (data.error != NULL)
	{
		g_free (expanded_regex);
		g_propagate_error (error, data.error);
		return NULL;
	}

	return expanded_regex;
}

static gboolean
replace_delimiter (const EggRegex *egg_regex,
		   const gchar    *regex,
		   GString        *expanded_regex,
		   gpointer        data)
{
	gchar *delim, *escapes;
	ParserState *parser_state = data;

	escapes = egg_regex_fetch (egg_regex, 1, regex);
	g_string_append (expanded_regex, escapes);

	delim = egg_regex_fetch (egg_regex, 2, regex);
	DEBUG (g_message ("replacing '\\%%%s'", delim));

	switch (delim[0])
	{
		case '[':
			g_string_append (expanded_regex,
					parser_state->opening_delimiter);
			break;
		case ']':
			g_string_append (expanded_regex,
					parser_state->closing_delimiter);
			break;
	}

	g_free (delim);
	g_free (escapes);

	return FALSE;
}

static gchar *
expand_regex_delimiters (ParserState *parser_state,
		gchar *regex,
		gint len)
{
	/* This is the commented regex without the doubled escape needed
	 * in a C string:
	 *
	 * (?<!\\)(\\\\)*\\%([\[|\])
	 * |------------||---------|
	 *      |             |
	 *      |        the strings
	 *      |        "\%[" or "\%]"
	 *      |
	 * a even sequence of escapes or
	 * a char that is not an escape
	 * (this is not matched)
	 *
	 * The first block is needed to ensure that the sequence is
	 * not escaped.
	 */
	const gchar *re = "(?<!\\\\)(\\\\\\\\)*\\\\%(\\[|\\])";
	gchar *expanded_regex;
	EggRegex *egg_re;

	if (regex == NULL)
		return NULL;

	egg_re = egg_regex_new (re, 0, 0, NULL);

	expanded_regex = egg_regex_replace_eval (egg_re, regex, len, 0, 0,
						 replace_delimiter, parser_state, NULL);

	DEBUG (g_message ("expanded regex delims '%s' to '%s'",
				regex, expanded_regex));

	egg_regex_free (egg_re);

	return expanded_regex;
}

static gchar *
expand_regex (ParserState *parser_state,
	      gchar *regex,
	      EggRegexCompileFlags flags,
	      gboolean do_expand_vars,
	      gboolean insert_parentheses,
	      GError **error)
{
	gchar *tmp_regex;
	GString *expanded_regex;
	/* Length in *bytes* of the regex passed to expand_regex_vars. */
	gint len;

	g_assert (parser_state != NULL);
	g_return_val_if_fail (error == NULL || *error == NULL, NULL);

	if (regex == NULL)
		return NULL;

	if (g_utf8_get_char (regex) == '/')
	{
		gchar *regex_end = g_utf8_strrchr (regex, -1, '/');
		if (regex_end == regex)
		{
			g_set_error (error,
					PARSER_ERROR, PARSER_ERROR_MALFORMED_REGEX,
					_ ("malformed regex '%s'"), regex);
			return NULL;
		}
		regex = g_utf8_next_char (regex);
		len = regex_end - regex;
	}
	else
	{
		len = -1;
	}

	if (do_expand_vars)
	{
		tmp_regex = expand_regex_vars (parser_state, regex, len, error);

		if (tmp_regex == NULL)
			return NULL;
	}
	else
	{
		if (len == -1)
			len = strlen (regex);
		tmp_regex = g_strndup (regex, len);
	}

	regex = tmp_regex;
	tmp_regex = expand_regex_delimiters (parser_state, regex, len);
	g_free (regex);

	/* Set the options and add not capturing parentheses if
	 * insert_parentheses is TRUE (this is needed for included
	 * regular expressions.) */
	expanded_regex = g_string_new ("");
	if (insert_parentheses)
		g_string_append (expanded_regex, "(?:");
	g_string_append (expanded_regex, "(?");
	if (flags != 0)
	{
		if (flags & EGG_REGEX_CASELESS)
			g_string_append (expanded_regex, "i");
		if (flags & EGG_REGEX_EXTENDED)
			g_string_append (expanded_regex, "x");
		if (flags & EGG_REGEX_DOTALL)
			g_string_append (expanded_regex, "s");
	}
	if (flags != (EGG_REGEX_CASELESS | EGG_REGEX_EXTENDED |
		EGG_REGEX_DOTALL))
	{
		g_string_append (expanded_regex, "-");
		if (!(flags & EGG_REGEX_CASELESS))
			g_string_append (expanded_regex, "i");
		if (!(flags & EGG_REGEX_EXTENDED))
			g_string_append (expanded_regex, "x");
		if (!(flags & EGG_REGEX_DOTALL))
			g_string_append (expanded_regex, "s");
	}
	g_string_append (expanded_regex, ")");
	g_string_append (expanded_regex, tmp_regex);
	if (insert_parentheses)
	{
		/* The '\n' is needed otherwise, if the regex is "extended"
		 * and it ends with a comment, the ')' is appended inside the
		 * comment itself */
		if (flags & EGG_REGEX_EXTENDED)
		{
			g_string_append (expanded_regex, "\n");
		}
		/* FIXME: if the regex is not extended this doesn't works */
		g_string_append (expanded_regex, ")");
	}
	g_free (tmp_regex);

	return g_string_free (expanded_regex, FALSE);
}

static void
handle_define_regex_element (ParserState *parser_state,
			     GError **error)
{
	gchar *id;
	xmlChar *regex;
	xmlChar *tmp;
	gchar *expanded_regex;
	int i;
	const gchar *regex_options[] = {"extended", "case-insensitive", "dot-match-all", NULL};
	EggRegexCompileFlags flags;
	GError *tmp_error = NULL;

	int type;

	g_return_if_fail (error != NULL && *error == NULL);

	if (parser_state->ctx_data == NULL)
		return;

	tmp = xmlTextReaderGetAttribute (parser_state->reader, BAD_CAST "id");

	/* If the file is validated <define-regex> must have an id
	 * attribute */
	g_assert (tmp != NULL);

	if (id_is_decorated ((gchar *)tmp, NULL))
		id = g_strdup ((gchar *)tmp);
	else
		id = decorate_id (parser_state, (gchar *)tmp);
	xmlFree (tmp);

	flags = parser_state->regex_compile_flags;

	for (i=0; regex_options[i] != NULL; i++)
	{
		tmp = xmlTextReaderGetAttribute (parser_state->reader,
						 BAD_CAST regex_options[i]);
		if (tmp != NULL)
			flags = update_regex_flags (flags, regex_options[i], tmp);
		xmlFree (tmp);
	}


	xmlTextReaderRead (parser_state->reader);

	type = xmlTextReaderNodeType (parser_state->reader);

	if (type == XML_READER_TYPE_TEXT || type == XML_READER_TYPE_CDATA)
		regex = xmlTextReaderValue (parser_state->reader);
	else
		regex = xmlStrdup (BAD_CAST "");

	expanded_regex = expand_regex (parser_state, (gchar*) regex, flags,
				       FALSE, TRUE, &tmp_error);

	if (tmp_error == NULL)
	{
		DEBUG (g_message ("defined regex %s: \"%s\"", id, (gchar *)regex));
		g_hash_table_insert (parser_state->defined_regexes, id, expanded_regex);
	}

	xmlFree (regex);

	if (tmp_error != NULL)
	{
		g_propagate_error (error, tmp_error);
		return;
	}
}

static void
handle_default_regex_options_element (ParserState *parser_state,
				      GError     **error)
{
	xmlChar *options;
	int ret, type;

	g_return_if_fail (error != NULL && *error == NULL);

	if (parser_state->ctx_data == NULL)
		return;

	do {
		ret = xmlTextReaderRead (parser_state->reader);
		g_assert (ret == 1);

		type = xmlTextReaderNodeType (parser_state->reader);

	} while (type != XML_READER_TYPE_TEXT &&
		 type != XML_READER_TYPE_CDATA);

	options = xmlTextReaderValue (parser_state->reader);

	g_strstrip ((gchar*) options);

	if (!calc_regex_flags ((gchar*) options, &(parser_state->regex_compile_flags)))
	{
		g_set_error (error,
			     PARSER_ERROR,
			     PARSER_ERROR_MALFORMED_REGEX,
			     _ ("malformed regex options '%s'"),
			     (gchar*) options);
	}

	xmlFree (options);
}

static void
map_style (ParserState *parser_state,
	   gchar       *style_id,
	   gchar       *map_to,
	   GError     **error)
{
	const gchar *real_map_to;

	g_return_if_fail (error != NULL && *error == NULL);
	g_return_if_fail (style_id != NULL);

	if (map_to != NULL)
	{
		if (g_hash_table_lookup (parser_state->styles_mapping, map_to) != NULL)
			real_map_to = map_to;
		else
			real_map_to = NULL;
	}
	else
	{
		real_map_to = style_id;
	}

	DEBUG (g_message ("mapping the style of '%s' to '%s' -> '%s'",
			  style_id,
			  map_to ? map_to : "(null)",
			  mapped_style));

	if (real_map_to != NULL)
		g_hash_table_insert (parser_state->styles_mapping,
				     g_strdup (style_id),
				     g_strdup (real_map_to));
	else
		g_set_error (error,
			     PARSER_ERROR,
			     PARSER_ERROR_WRONG_ID,
			     "unable to map style '%s' to '%s'",
			     style_id, map_to);
}

static void
parse_language_with_id (ParserState *parser_state,
		gchar *lang_id,
		GError **error)
{
	GtkSourceLanguagesManager *lm;
	GtkSourceLanguage *imported_language;
	GError *tmp_error = NULL;

	g_return_if_fail (error != NULL && *error == NULL);

	lm = _gtk_source_language_get_languages_manager (parser_state->language);
	imported_language = gtk_source_languages_manager_get_language_by_id (lm, lang_id);

	if (imported_language == NULL)
	{
		g_set_error (&tmp_error,
				PARSER_ERROR,
				PARSER_ERROR_WRONG_ID,
				"unable to resolve language '%s'",
				lang_id);
	}
	else
	{
		file_parse (imported_language->priv->lang_file_name,
			    parser_state->language,
			    parser_state->ctx_data,
			    parser_state->defined_regexes,
			    parser_state->styles_mapping,
			    parser_state->loaded_lang_ids,
			    &tmp_error);
	}

	if (tmp_error != NULL)
	{
		g_propagate_error (error, tmp_error);
		return;
	}
}

static void
parse_style (ParserState *parser_state,
	     GError     **error)
{
	gchar *id;
	xmlChar *name, *map_to;
	xmlChar *tmp;
	gchar *lang_id = NULL;
	GError *tmp_error = NULL;

	g_return_if_fail (error != NULL && *error == NULL);

	tmp = xmlTextReaderGetAttribute (parser_state->reader,
					 BAD_CAST "id");

	if (id_is_decorated ((gchar*) tmp, NULL))
		id = g_strdup ((gchar*) tmp);
	else
		id = decorate_id (parser_state, (gchar*) tmp);

	xmlFree (tmp);

	name = xmlTextReaderGetAttribute (parser_state->reader,
					  BAD_CAST "_name");

	if (name != NULL)
	{
		tmp = xmlStrdup (BAD_CAST dgettext (parser_state->language->priv->translation_domain,
				 (gchar*) name));
		xmlFree (name);
		name = tmp;
	}
	else
	{
		name = xmlTextReaderGetAttribute (parser_state->reader,
						  BAD_CAST "name");
	}

	map_to = xmlTextReaderGetAttribute (parser_state->reader,
					    BAD_CAST "map-to");

	if (map_to != NULL && !id_is_decorated ((gchar*) map_to, &lang_id))
	{
		g_set_error (&tmp_error,
			     PARSER_ERROR,
			     PARSER_ERROR_MALFORMED_MAP_TO,
			     "the map-to attribute '%s' for the style '%s' lacks the prefix",
			     map_to, id);
	}

	if (tmp_error == NULL && lang_id != NULL && lang_id[0] == 0)
	{
		g_free (lang_id);
		lang_id = NULL;
	}

	if (tmp_error == NULL && lang_id != NULL && !lang_id_is_already_loaded (parser_state, lang_id))
		parse_language_with_id (parser_state, lang_id, &tmp_error);

	DEBUG (g_message ("style %s (%s) to be mapped to '%s'",
			  name, id, map_to ? (char*) map_to : "(null)"));

	if (tmp_error == NULL)
		map_style (parser_state, id, (gchar*) map_to, &tmp_error);

	g_free (lang_id);
	g_free (id);
	xmlFree (name);
	xmlFree (map_to);

	if (tmp_error != NULL)
	{
		g_propagate_error (error, tmp_error);
		return;
	}
}

static void
handle_keyword_char_class_element (ParserState *parser_state,
				   GError     **error)
{
	xmlChar *char_class;
	int ret, type;

	g_return_if_fail (error != NULL && *error == NULL);

	if (parser_state->ctx_data == NULL)
		return;

	do {
		ret = xmlTextReaderRead (parser_state->reader);
		g_assert (ret == 1);
		type = xmlTextReaderNodeType (parser_state->reader);
	}
	while (type != XML_READER_TYPE_TEXT && type != XML_READER_TYPE_CDATA);

	char_class = xmlTextReaderValue (parser_state->reader);

	g_free (parser_state->opening_delimiter);
	g_free (parser_state->closing_delimiter);

	parser_state->opening_delimiter = g_strdup_printf ("(?!<%s)(?=%s)",
							   char_class, char_class);
	parser_state->closing_delimiter = g_strdup_printf ("(?<=%s)(?!%s)",
							   char_class, char_class);

	xmlFree (char_class);
}

static void
handle_styles_element (ParserState *parser_state,
		       GError     **error)
{
	int ret, type;
	const xmlChar *tag_name;
	GError *tmp_error = NULL;

	g_return_if_fail (error != NULL && *error == NULL);

	while (TRUE)
	{
		ret = xmlTextReaderRead (parser_state->reader);

		xmlTextReaderIsValid (parser_state->reader);

		/* FIXME: is xmlTextReaderIsValid call needed here or
		 * error func will be called? */
		if (*error != NULL)
			break;

		tag_name = xmlTextReaderConstName (parser_state->reader);
		type = xmlTextReaderNodeType  (parser_state->reader);

		/* End at the closing </styles> tag */
		if (tag_name && type == XML_READER_TYPE_END_ELEMENT &&
		    !xmlStrcmp (BAD_CAST "styles", tag_name))
			break;

		/* Skip nodes that aren't <style> elements */
		if (tag_name == NULL || xmlStrcmp (BAD_CAST "style", tag_name))
			continue;

		/* Handle <style> elements */
		parse_style (parser_state, &tmp_error);

		if (tmp_error != NULL)
			break;
	}

	if (tmp_error != NULL)
	{
		g_propagate_error (error, tmp_error);
		return;
	}
}


static void
element_start (ParserState *parser_state,
	       GError     **error)
{
	const xmlChar *name;
	GError *tmp_error = NULL;

	/* TODO: check the namespace and ignore everithing is not in our namespace */
	name = xmlTextReaderConstName (parser_state->reader);

	if (xmlStrcmp (BAD_CAST "context", name) == 0)
	{
		handle_context_element (parser_state, &tmp_error);
	}
	else if (xmlStrcmp (BAD_CAST "define-regex", name) == 0)
	{
		handle_define_regex_element (parser_state, &tmp_error);
	}
	else if (xmlStrcmp (BAD_CAST "language", name) == 0)
	{
		handle_language_element (parser_state, &tmp_error);
	}
	else if (xmlStrcmp (BAD_CAST "styles", name) == 0)
	{
		handle_styles_element (parser_state, &tmp_error);
	}
	else if (xmlStrcmp (BAD_CAST "keyword-char-class", name) == 0)
	{
		handle_keyword_char_class_element (parser_state, &tmp_error);
	}
	else if (xmlStrcmp (BAD_CAST "default-regex-options", name) == 0)
	{
		handle_default_regex_options_element (parser_state, &tmp_error);
	}

	if (tmp_error != NULL)
	{
		g_propagate_error (error, tmp_error);
		return;
	}
}

static void
element_end (ParserState *parser_state)
{
	const xmlChar *name;
	gchar *popped_id;

	name = xmlTextReaderConstName (parser_state->reader);

	if (!xmlStrcmp (name, BAD_CAST "context"))
	{
		/* pop the first element in the curr_parents list */
		popped_id = g_queue_pop_head (parser_state->curr_parents);
		g_free (popped_id);
	}
}

static void
text_reader_structured_error_func (GError     **gerror,
				   xmlErrorPtr  error)
{
	/* FIXME: does someone know how to use libxml api? */

	if (*gerror == NULL)
		g_set_error (gerror, PARSER_ERROR, PARSER_ERROR_INVALID_DOC, "parser error");

	/* XXX: g_print now because of --g-fatal-warnings */
	g_print ("in file %s on line %d: %s\n", error->file, error->line, error->message);
}

static gboolean
file_parse (gchar                     *filename,
	    GtkSourceLanguage         *language,
	    GtkSourceContextData      *ctx_data,
	    GHashTable                *defined_regexes,
	    GHashTable                *styles,
	    GSList                   **loaded_lang_ids,
	    GError                   **error)
{
	ParserState *parser_state;
	xmlTextReader *reader = NULL;
	int ret;
	int fd;
	GError *tmp_error = NULL;
	GtkSourceLanguagesManager *lm;
	const gchar *rng_lang_schema;

	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	DEBUG (g_message ("loading file '%s'", filename));

	/*
	 * Use fd instead of filename so that it's utf8 safe on w32.
	 */
	fd = g_open (filename, O_RDONLY, 0);

	if (fd != -1)
		reader = xmlReaderForFd (fd, filename, NULL, 0);

	if (reader == NULL)
	{
		g_set_error (&tmp_error,
			     PARSER_ERROR,
			     PARSER_ERROR_CANNOT_OPEN,
			     "unable to open the file");
		goto error;
	}

	lm = _gtk_source_language_get_languages_manager (language);
	rng_lang_schema = _gtk_source_languages_manager_get_rng_file (lm);

	if (rng_lang_schema == NULL)
	{
		g_set_error (&tmp_error,
			     PARSER_ERROR,
			     PARSER_ERROR_CANNOT_VALIDATE,
			     "could not find the RelaxNG schema file");
		goto error;
	}

	if (xmlTextReaderRelaxNGValidate (reader, rng_lang_schema))
	{
		g_set_error (&tmp_error,
			     PARSER_ERROR,
			     PARSER_ERROR_CANNOT_VALIDATE,
			     "unable to load the RelaxNG schema '%s'",
			     rng_lang_schema);
		goto error;
	}

	parser_state = parser_state_new (language, ctx_data,
					 defined_regexes, styles,
					 reader, loaded_lang_ids);
	xmlTextReaderSetStructuredErrorHandler (reader,
						(xmlStructuredErrorFunc) text_reader_structured_error_func,
						&tmp_error);

	while (!tmp_error && (ret = xmlTextReaderRead (parser_state->reader)) == 1)
	{
		int type;

		xmlTextReaderIsValid (reader);

		/* FIXME: is xmlTextReaderIsValid call needed here or
		 * error func will be called? */
		if (tmp_error != NULL)
			break;

		type = xmlTextReaderNodeType (parser_state->reader);

		switch (type)
		{
			case XML_READER_TYPE_ELEMENT:
				element_start (parser_state, &tmp_error);
				break;
			case XML_READER_TYPE_END_ELEMENT:
				element_end (parser_state);
				break;
		}
	}

	parser_state_destroy (parser_state);

	if (tmp_error != NULL)
		goto error;

	return TRUE;

error:
	g_propagate_error (error, tmp_error);
	return FALSE;
}

static ParserState *
parser_state_new (GtkSourceLanguage       *language,
		  GtkSourceContextData    *ctx_data,
		  GHashTable              *defined_regexes,
		  GHashTable              *styles_mapping,
		  xmlTextReader	          *reader,
		  GSList                 **loaded_lang_ids)
{
	ParserState *parser_state;
	parser_state = g_new (ParserState, 1);

	parser_state->language = language;
	parser_state->ctx_data = ctx_data;

	parser_state->current_lang_id = NULL;

	parser_state->id_cookie = 0;
	parser_state->regex_compile_flags = 0;

	parser_state->reader = reader;

	parser_state->defined_regexes = defined_regexes;
	parser_state->styles_mapping = styles_mapping;

	parser_state->loaded_lang_ids = loaded_lang_ids;

	parser_state->curr_parents = g_queue_new ();

	parser_state->opening_delimiter = g_strdup ("\\b");
	parser_state->closing_delimiter = g_strdup ("\\b");

	return parser_state;
}

static void
parser_state_destroy (ParserState *parser_state)
{
	if (parser_state->reader != NULL)
		xmlFreeTextReader (parser_state->reader);

	g_queue_free (parser_state->curr_parents);
	g_free (parser_state->current_lang_id);

	g_free (parser_state->opening_delimiter);
	g_free (parser_state->closing_delimiter);

	g_free (parser_state);
}

static gboolean
steal_styles_mapping (char       *style,
		      char       *map_to,
		      GHashTable *styles)
{
	if (strcmp (style, map_to) != 0)
	{
		g_hash_table_insert (styles, style, map_to);
		return TRUE;
	}

	return FALSE;
}

gboolean
_gtk_source_language_file_parse_version2 (GtkSourceLanguage       *language,
					  GtkSourceContextData    *ctx_data)
{
	GHashTable *defined_regexes, *styles;
	gboolean success;
	GError *error = NULL;
	gchar *filename;
	GSList *loaded_lang_ids = NULL;
	GSList *l;

	g_return_val_if_fail (ctx_data != NULL, FALSE);

	filename = language->priv->lang_file_name;

	/* TODO: as an optimization tell the parser to merge CDATA
	 * as text nodes (XML_PARSE_NOCDATA), and to ignore blank
	 * nodes (XML_PARSE_NOBLANKS), if it is possible with
	 * xmlTextReader. */
	xmlKeepBlanksDefault (0);
	xmlLineNumbersDefault (1);
	xmlSubstituteEntitiesDefault (1);
	DEBUG (xmlPedanticParserDefault (1));

	defined_regexes = g_hash_table_new_full (g_str_hash, g_str_equal,
						 g_free, g_free);
	styles = g_hash_table_new_full (g_str_hash, g_str_equal,
					g_free, g_free);

	success = file_parse (filename, language, ctx_data,
			      defined_regexes, styles,
			      &loaded_lang_ids, &error);

	if (success)
		success = _gtk_source_context_data_resolve_refs (ctx_data, &error);

	if (success)
		g_hash_table_foreach_steal (styles,
					    (GHRFunc) steal_styles_mapping,
					    language->priv->styles);

	for (l = loaded_lang_ids; l != NULL; l = l->next)
	{
		xmlFree (l->data);
	}
	g_slist_free (loaded_lang_ids);

	g_hash_table_destroy (defined_regexes);
	g_hash_table_destroy (styles);

	if (!success)
	{
		g_warning ("Failed to load '%s': %s",
			   filename, error->message);
		g_error_free (error);
		return FALSE;
	}

	return TRUE;
}
