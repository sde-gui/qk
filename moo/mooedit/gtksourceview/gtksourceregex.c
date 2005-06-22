/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 *  gtksourceregex.c
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

/*****************************************************************************
 * Changed by Muntyan
 *
 * 03/28/2005: added refcounting
 * 03/28/2005: changed #include "gnu-regex/regex.h"
 * 04/15/2005: replaced NATIVE_GNU_REGEX with USE_NATIVE_GNU_REGEX,
 *             added gtk_source_regex_destroy() which calls
 *             gtk_source_regex_unref()
 * 04/22/2005: added pcre support
 *
 *****************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <stdlib.h>
#include <glib.h>

#define USE_PCRE 1

#if USE_PCRE

#include "mooutils/eggregex.h"
#include "gtksourceregex.h"

struct _GtkSourceRegex
{
	guint			 ref_count;
	EggRegex		*regex;
};


GtkSourceRegex *gtk_source_regex_compile 	(const gchar          *pattern)
{
	GtkSourceRegex *regex;
	GError *error = NULL;

	g_return_val_if_fail (pattern != NULL, NULL);

	regex = g_new0 (GtkSourceRegex, 1);
	regex->ref_count = 1;

	regex->regex = egg_regex_new (pattern,
				      EGG_REGEX_MULTILINE,
				      0, &error);

	if (!regex->regex) {
		if (error) {
			g_warning ("Regex failed to compile: %s",
				   error->message);
			g_error_free (error);
		}
		else
			g_warning ("Regex failed to compile");
		g_free (regex);
		return NULL;
	}

	egg_regex_optimize (regex->regex, &error);
	if (error) {
		g_warning ("Regex failed to optimize: %s",
			   error->message);
		g_error_free (error);
	}

	return regex;
}


GtkSourceRegex *
gtk_source_regex_ref (GtkSourceRegex *regex)
{
	g_return_val_if_fail (regex != NULL, NULL);
	++(regex->ref_count);
	return regex;
}

void
gtk_source_regex_unref (GtkSourceRegex *regex)
{
	if (regex == NULL)
		return;

	if (--(regex->ref_count))
		return;

	egg_regex_free (regex->regex);
	g_free (regex);
}

void
gtk_source_regex_destroy (GtkSourceRegex *regex)
{
	gtk_source_regex_unref (regex);
}


/**
 * gtk_source_regex_search:
 * @regex: Regular expression object (#GtkSourceRegex).
 * @text: Buffer to search.
 * @pos: Offset position (i.e. character offset, not byte offset).
 * @length: Length in bytes; -1 for the full text length.
 * @match: (optional) Where to return match information.
 *
 * Return value: the offset where a match occurred, or less than 0 for
 * errors or no match.
 **/
gint
gtk_source_regex_search (GtkSourceRegex       *regex,
			 const gchar          *text,
			 gint                  pos,
			 gint                  length,
			 GtkSourceBufferMatch *match,
			 guint                 options)
{
	gint res, startindex = 0, endindex = 0;
	EggRegexMatchFlags opt = 0;

	g_return_val_if_fail (regex != NULL, -2);
	g_return_val_if_fail (text != NULL, -2);
	g_return_val_if_fail (pos >= 0, -2);

	if (length < 0)
		length = strlen (text);

	/* convert pos from offset to byte index */
	if (pos > 0)
		pos = g_utf8_offset_to_pointer (text, pos) - text;

#if 0
	if (options & GTK_SOURCE_REGEX_NOT_BOL)
		opt |= EGG_REGEX_MATCH_NOTBOL;
	if (options & GTK_SOURCE_REGEX_NOT_EOL)
		opt |= EGG_REGEX_MATCH_NOTEOL;
#endif

	egg_regex_clear (regex->regex);
	res = egg_regex_match (regex->regex, text + pos, length - pos, opt);

	if (res <= 0)
		return -1;

	egg_regex_fetch_pos (regex->regex, text + pos, 0,
			     &startindex, &endindex);

	if (match != NULL) {
		match->startindex = pos + startindex;
		match->endindex = pos + endindex;

		match->startpos =
			g_utf8_pointer_to_offset (text, text + pos + startindex);
		match->endpos = match->startpos +
			g_utf8_pointer_to_offset (text + pos + startindex,
						  text + pos + endindex);

		return match->startpos;
	} else
		return g_utf8_pointer_to_offset (text, text + pos + startindex);
}


/*
 * pos: offset
 *
 * Returns: if @regex matched @text
 */
gboolean
gtk_source_regex_match (GtkSourceRegex *regex,
			const gchar    *text,
			gint            pos,
			gint            length,
			guint           options)
{
	EggRegexMatchFlags opt = 0;

	g_return_val_if_fail (regex != NULL, -1);
	g_return_val_if_fail (pos >= 0, -1);

	if (length < 0)
		length = strlen (text);

	pos = g_utf8_offset_to_pointer (text, pos) - text;

#if 0
	if (options & GTK_SOURCE_REGEX_NOT_BOL)
		opt |= EGG_REGEX_MATCH_NOTBOL;
	if (options & GTK_SOURCE_REGEX_NOT_EOL)
		opt |= EGG_REGEX_MATCH_NOTEOL;
#endif

	egg_regex_clear (regex->regex);
	return egg_regex_match (regex->regex, text + pos, length - pos, opt) > 0;
}


#else /* !USE_PCRE */


#ifdef USE_NATIVE_GNU_REGEX
#include <sys/types.h>
#include <regex.h>
#else
#include "mooutils/gnu-regex/regex.h"
#endif

#include "gtksourceview-i18n.h"
#include "gtksourceregex.h"

/* Implementation using GNU Regular Expressions */

struct _GtkSourceRegex
{
	guint			 ref_count;
	struct re_pattern_buffer buf;
	struct re_registers 	 reg;
};

GtkSourceRegex *
gtk_source_regex_compile (const gchar *pattern)
{
	GtkSourceRegex *regex;
	const char *result = NULL;

	g_return_val_if_fail (pattern != NULL, NULL);

	regex = g_new0 (GtkSourceRegex, 1);
	regex->ref_count = 1;

	re_syntax_options = RE_SYNTAX_POSIX_MINIMAL_EXTENDED;
	regex->buf.translate = NULL;
	regex->buf.fastmap = g_malloc (256);
	regex->buf.allocated = 0;
	regex->buf.buffer = NULL;

	if ((result = re_compile_pattern (pattern,
					  strlen (pattern),
					  &regex->buf)) == NULL) {
		/* success...now try to compile a fastmap */
		if (re_compile_fastmap (&regex->buf) != 0) {
			g_warning ("Regex failed to create a fastmap.");
			/* error...no fastmap */
			g_free (regex->buf.fastmap);
			regex->buf.fastmap = NULL;
		}

		return regex;
	} else {
		g_free (regex->buf.fastmap);
		g_free (regex);
		g_warning ("Regex failed to compile: %s", result);
		return NULL;
	}
}


GtkSourceRegex *
gtk_source_regex_ref (GtkSourceRegex *regex)
{
	g_return_val_if_fail (regex != NULL, NULL);
	++(regex->ref_count);
	return regex;
}

void
gtk_source_regex_unref (GtkSourceRegex *regex)
{
	if (regex == NULL)
		return;

	if (--(regex->ref_count))
		return;

	g_free (regex->buf.fastmap);
	regex->buf.fastmap = NULL;
	regfree (&regex->buf);
	if (regex->reg.num_regs > 0) {
		/* FIXME: maybe we should pre-allocate the registers
		 * at compile time? */
		free (regex->reg.start);
		free (regex->reg.end);
		regex->reg.num_regs = 0;
	}
	g_free (regex);
}

void
gtk_source_regex_destroy (GtkSourceRegex *regex)
{
	gtk_source_regex_unref (regex);
}


/**
 * gtk_source_regex_search:
 * @regex: Regular expression object (#GtkSourceRegex).
 * @text: Buffer to search.
 * @pos: Offset position (i.e. character offset, not byte offset).
 * @length: Length in bytes; -1 for the full text length.
 * @match: (optional) Where to return match information.
 *
 * Return value: the offset where a match occurred, or less than 0 for
 * errors or no match.
 **/
gint
gtk_source_regex_search (GtkSourceRegex       *regex,
			 const gchar          *text,
			 gint                  pos,
			 gint                  length,
			 GtkSourceBufferMatch *match,
			 guint                 options)
{
	gint res;

	g_return_val_if_fail (regex != NULL, -2);
	g_return_val_if_fail (text != NULL, -2);
	g_return_val_if_fail (pos >= 0, -2);

	if (length < 0)
		length = strlen (text);

	/* convert pos from offset to byte index */
	if (pos > 0)
		pos = g_utf8_offset_to_pointer (text, pos) - text;

	regex->buf.not_bol = (options & GTK_SOURCE_REGEX_NOT_BOL);
	regex->buf.not_eol = (options & GTK_SOURCE_REGEX_NOT_EOL);

	res = re_search (&regex->buf, text, length,
			 pos, length - pos, &regex->reg);

	if (res < 0)
		return res;

	if (match != NULL) {
		match->startindex = res;
		match->endindex = regex->reg.end [0];

		match->startpos =
			g_utf8_pointer_to_offset (text, text + res);
		match->endpos = match->startpos +
			g_utf8_pointer_to_offset (text + res,
						  text + regex->reg.end [0]);

		return match->startpos;
	} else
		return g_utf8_pointer_to_offset (text, text + res);
}

/* regex_match -- tries to match regex at the 'pos' position in the text. It
 * returns the number of chars matched, or -1 if no match.
 * Warning: The number matched can be 0, if the regex matches the empty string.
 */

/*
 * pos: offset
 *
 * Returns: if @regex matched @text
 */
gboolean
gtk_source_regex_match (GtkSourceRegex *regex,
			const gchar    *text,
			gint            pos,
			gint            length,
			guint           options)
{
	g_return_val_if_fail (regex != NULL, -1);
	g_return_val_if_fail (pos >= 0, -1);

	if (length < 0)
		length = strlen (text);

	pos = g_utf8_offset_to_pointer (text, pos) - text;

	regex->buf.not_bol = (options & GTK_SOURCE_REGEX_NOT_BOL);
	regex->buf.not_eol = (options & GTK_SOURCE_REGEX_NOT_EOL);

	return (re_match (&regex->buf, text, length, pos,
			  &regex->reg) > 0);
}

#endif /* !USE_PCRE */
