/* EggRegex -- regular expression API wrapper around PCRE.
 *
 * Copyright (C) 1999, 2000 Scott Wimer
 * Copyright (C) 2004, Matthias Clasen <mclasen@redhat.com>
 * Copyright (C) 2005 - 2006, Marco Barisione <barisione@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "config.h"
#include <glib.h>
#include <string.h>
#include "eggregex.h"
#include <pcre.h>

/* FIXME when this is in glib */
#define _(s) s

/* if the string is in UTF-8 use g_utf8_next_char(), else just
 * use +1. */
#define REGEX_UTF8(re) (((re)->compile_opts & PCRE_UTF8) != 0)
#define NEXT_CHAR(s,utf8) ((utf8) ? g_utf8_next_char (s) : ((s) + 1))
#define NEXT_CHAR_RE(re, s) (NEXT_CHAR (s, REGEX_UTF8 (re)))

/* TRUE if ret is an error code, FALSE otherwise. */
#define IS_PCRE_ERROR(ret) ((ret) < PCRE_ERROR_NOMATCH && (ret) != PCRE_ERROR_PARTIAL)

#define WORKSPACE_INITIAL 1000
#define OFFSETS_DFA_MIN_SIZE 21

struct _EggRegex
{
  gint ref_count;	/* reference count */
  gchar *pattern;       /* the pattern */
  pcre *regex;		/* compiled form of the pattern */
  pcre_extra *extra;	/* data stored when egg_regex_optimize() is used */
  gint matches;		/* number of matching sub patterns */
  gint pos;		/* position in the string where last match left off */
  gint *offsets;	/* array of offsets paired 0,1 ; 2,3 ; 3,4 etc */
  gint n_offsets;	/* number of offsets */
  gint *workspace;	/* workspace for pcre_dfa_exec() */
  gint n_workspace;	/* number of workspace elements */
  EggRegexCompileFlags compile_opts;	/* options used at compile time on the pattern */
  EggRegexMatchFlags match_opts;	/* options used at match time on the regex */
  gssize string_len;	/* length of the string last used against */
  GSList *delims;	/* delimiter sub strings from split next */
};

GQuark
egg_regex_error_quark (void)
{
  static GQuark error_quark = 0;

  if (error_quark == 0)
    error_quark = g_quark_from_static_string ("g-regex-error-quark");

  return error_quark;
}

static EggRegex *
regex_new (pcre                *re,
	   const gchar         *pattern,
	   EggRegexCompileFlags compile_options,
	   EggRegexMatchFlags   match_options)
{
  /* function used internally by egg_regex_new() and egg_regex_copy()
   * to create a new EggRegex from a pcre structure */
  EggRegex *regex = g_new0 (EggRegex, 1);
  gint capture_count;

  regex->regex = re;
  regex->pattern = g_strdup (pattern);
  regex->string_len = -1;	/* not set yet */

  /* set the options */
  regex->compile_opts = compile_options;
  regex->match_opts = match_options;

  /* find out how many sub patterns exist in this pattern, and
   * setup the offsets array and n_offsets accordingly */
  pcre_fullinfo (regex->regex, regex->extra,
		 PCRE_INFO_CAPTURECOUNT, &capture_count);
  regex->n_offsets = (capture_count + 1) * 3;
  regex->offsets = g_new0 (gint, regex->n_offsets);

  return regex;
}

/**
 * egg_regex_new:
 * @pattern: the regular expression.
 * @compile_options: compile options for the regular expression.
 * @match_options: match options for the regular expression.
 * @error: return location for a #GError.
 *
 * Compiles the regular expression to an internal form, and does the initial
 * setup of the #EggRegex structure.
 *
 * Returns: a #EggRegex structure.
 */
EggRegex *
egg_regex_new (const gchar         *pattern,
 	     EggRegexCompileFlags   compile_options,
	     EggRegexMatchFlags     match_options,
	     GError             **error)
{
  pcre *re;
  const gchar *errmsg;
  gint erroffset;
  static gboolean initialized = FALSE;

  g_return_val_if_fail (pattern != NULL, NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  if (!initialized)
    {
      gint support;
      const gchar *msg;

      pcre_config (PCRE_CONFIG_UTF8, &support);
      if (!support)
	{
	  msg = _("PCRE library is compiled without UTF8 support");
	  g_critical (msg);
	  g_set_error (error, EGG_REGEX_ERROR, EGG_REGEX_ERROR_COMPILE, msg);
	  return NULL;
	}

      pcre_config (PCRE_CONFIG_UNICODE_PROPERTIES, &support);
      if (!support)
	{
	  msg = _("PCRE library is compiled without UTF8 properties support");
	  g_critical (msg);
	  g_set_error (error, EGG_REGEX_ERROR, EGG_REGEX_ERROR_COMPILE, msg);
	  return NULL;
	}

#ifndef USE_SYSTEM_PCRE
      /* If we are using the system-supplied PCRE library it's not
       * safe to do this. */
      pcre_malloc = (void * (*) (size_t)) g_try_malloc;
      pcre_free = g_free;
      pcre_stack_malloc = pcre_malloc;
      pcre_stack_free = pcre_free;
#endif

      initialized = TRUE;
    }

  /* In EggRegex the string are, by default, UTF-8 encoded. PCRE
   * instead uses UTF-8 only if required with PCRE_UTF8. */
  if (compile_options & EGG_REGEX_RAW)
    {
      /* disable utf-8 */
      compile_options &= ~EGG_REGEX_RAW;
    }
  else
    {
      /* enable utf-8 */
      compile_options |= PCRE_UTF8 | PCRE_NO_UTF8_CHECK;
      match_options |= PCRE_NO_UTF8_CHECK;
    }

  /* compile the pattern */
  re = pcre_compile (pattern, compile_options, &errmsg, &erroffset, NULL);

  /* if the compilation failed, set the error member and return
   * immediately */
  if (re == NULL)
    {
      GError *tmp_error = g_error_new (EGG_REGEX_ERROR,
				       EGG_REGEX_ERROR_COMPILE,
				       _("Error while compiling regular "
					 "expression %s at char %d: %s"),
				       pattern, erroffset, errmsg);
      g_propagate_error (error, tmp_error);

      return NULL;
    }
  else
    return regex_new (re, pattern, compile_options, match_options);
}

/**
 * egg_regex_ref:
 * @regex: a #EggRegex.
 *
 * Increases the reference count of the @regex by 1.
 *
 * Returns: the @regex that was passed in.
 **/
EggRegex *
egg_regex_ref (EggRegex *regex)
{
  if (regex != NULL)
    ++regex->ref_count;
  return regex;
}

/**
 * egg_regex_unref:
 * @regex: a #EggRegex.
 *
 * Decreases the reference count of the @regex by 1. If the reference
 * count went to 0, the @regex will be destroyed and the memory
 * allocated will be freed.
 **/
void
egg_regex_unref (EggRegex *regex)
{
  if (regex == NULL || --regex->ref_count)
    return;

  g_free (regex->pattern);
  g_slist_free (regex->delims);
  g_free (regex->offsets);
  g_free (regex->workspace);
  if (regex->regex != NULL)
    g_free (regex->regex);
  if (regex->extra != NULL)
    g_free (regex->extra);
  g_free (regex);
}

/**
 * egg_regex_copy:
 * @regex: a #EggRegex structure from egg_regex_new().
 *
 * Copies a #EggRegex.
 *
 * Returns: a newly allocated copy of @regex, or %NULL if an error
 *          occurred.
 */
EggRegex *
egg_regex_copy (const EggRegex *regex)
{
  EggRegex *copy;
  gint res;
  gint size;
  pcre *re;

  g_return_val_if_fail (regex != NULL, NULL);

  res = pcre_fullinfo (regex->regex, NULL, PCRE_INFO_SIZE, &size);
  g_return_val_if_fail (res >= 0, NULL);
  re = g_malloc (size);
  memcpy (re, regex->regex, size);
  copy = regex_new (re, regex->pattern, regex->compile_opts, regex->match_opts);

  if (regex->extra != NULL)
    {
      res = pcre_fullinfo (regex->regex, regex->extra,
		      	   PCRE_INFO_STUDYSIZE, &size);
      g_return_val_if_fail (res >= 0, copy);
      copy->extra = g_new0 (pcre_extra, 1);
      copy->extra->flags = PCRE_EXTRA_STUDY_DATA;
      copy->extra->study_data = g_malloc (size);
      memcpy (copy->extra->study_data, regex->extra->study_data, size);
    }

  return copy;
}

/**
 * egg_regex_get_pattern:
 * @regex: a #EggRegex structure.
 *
 * Gets the pattern string associated with @regex, i.e. a copy of the string passed
 * to egg_regex_new().
 *
 * Returns: the pattern of @regex.
 */
const gchar *
egg_regex_get_pattern (const EggRegex *regex)
{
  g_return_val_if_fail (regex != NULL, NULL);

  return regex->pattern;
}

/**
 * egg_regex_clear:
 * @regex: a #EggRegex structure.
 *
 * Clears out the members of @regex that are holding information about the
 * last set of matches for this pattern.  egg_regex_clear() needs to be
 * called between uses of egg_regex_match_next() or egg_regex_match_next_full()
 * against new target strings.
 */
void
egg_regex_clear (EggRegex *regex)
{
  g_return_if_fail (regex != NULL);

  regex->matches = -1;
  regex->string_len = -1;
  regex->pos = 0;

  /* if the pattern was used with egg_regex_split_next(), it may have
   * delimiter offsets stored.  Free up those guys as well. */
  if (regex->delims != NULL)
    g_slist_free (regex->delims);
}

/**
 * egg_regex_optimize:
 * @regex: a #EggRegex structure.
 * @error: return location for a #GError.
 *
 * If the pattern will be used many times, then it may be worth the
 * effort to optimize it to improve the speed of matches.
 *
 * Returns: %TRUE if @regex has been optimized or was already optimized,
 *          %FALSE otherwise.
 */
gboolean
egg_regex_optimize (EggRegex  *regex,
		  GError **error)
{
  const gchar *errmsg;

  g_return_val_if_fail (regex != NULL, FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  if (regex->extra != NULL)
    /* already optimized. */
    return TRUE;

  regex->extra = pcre_study (regex->regex, 0, &errmsg);

  if (errmsg)
    {
      GError *tmp_error = g_error_new (EGG_REGEX_ERROR,
				       EGG_REGEX_ERROR_OPTIMIZE,
				       _("Error while optimizing "
					 "regular expression %s: %s"),
				       regex->pattern,
				       errmsg);
      g_propagate_error (error, tmp_error);
      return FALSE;
    }

  return TRUE;
}

/**
 * egg_regex_match_simple:
 * @pattern: the regular expression.
 * @string: the string to scan for matches.
 * @compile_options: compile options for the regular expression.
 * @match_options: match options.
 *
 * Scans for a match in @string for @pattern.
 *
 * This function is equivalent to egg_regex_match() but it does not
 * require to compile the pattern with egg_regex_new(), avoiding some
 * lines of code when you need just to do a match without extracting
 * substrings, capture counts, and so on.
 *
 * If this function is to be called on the same @pattern more than
 * once, it's more efficient to compile the pattern once with
 * egg_regex_new() and then use egg_regex_match().
 *
 * Returns: %TRUE is the string matched, %FALSE otherwise.
 */
gboolean
egg_regex_match_simple (const gchar        *pattern,
		      const gchar        *string,
		      EggRegexCompileFlags  compile_options,
		      EggRegexMatchFlags    match_options)
{
  EggRegex *regex;
  gboolean result;

  regex = egg_regex_new (pattern, compile_options, 0, NULL);
  if (!regex)
    return FALSE;
  result = egg_regex_match_full (regex, string, -1, 0, match_options, NULL);
  egg_regex_unref (regex);
  return result;
}

/**
 * egg_regex_match:
 * @regex: a #EggRegex structure from egg_regex_new().
 * @string: the string to scan for matches.
 * @match_options:  match options.
 *
 * Scans for a match in string for the pattern in @regex. The @match_options
 * are combined with the match options specified when the @regex structure
 * was created, letting you have more flexibility in reusing #EggRegex
 * structures.
 *
 * Returns: %TRUE is the string matched, %FALSE otherwise.
 */
gboolean
egg_regex_match (EggRegex          *regex,
	       const gchar     *string,
	       EggRegexMatchFlags match_options)
{
  return egg_regex_match_full (regex, string, -1, 0,
			       match_options, NULL);
}

/**
 * egg_regex_match_full:
 * @regex: a #EggRegex structure from egg_regex_new().
 * @string: the string to scan for matches.
 * @string_len: the length of @string, or -1 if @string is nul-terminated.
 * @start_position: starting index of the string to match.
 * @match_options:  match options.
 * @error: location to store the error occuring, or NULL to ignore errors.
 *
 * Scans for a match in string for the pattern in @regex. The @match_options
 * are combined with the match options specified when the @regex structure
 * was created, letting you have more flexibility in reusing #EggRegex
 * structures.
 *
 * Setting @start_position differs from just passing over a shortened string
 * and  setting #EGG_REGEX_MATCH_NOTBOL in the case of a pattern that begins
 * with any kind of lookbehind assertion, such as "\b".
 *
 * Returns: %TRUE is the string matched, %FALSE otherwise.
 */
gboolean
egg_regex_match_full (EggRegex          *regex,
		      const gchar       *string,
		      gssize             string_len,
		      gint               start_position,
		      EggRegexMatchFlags match_options,
		      GError           **error)
{
  g_return_val_if_fail (regex != NULL, FALSE);
  g_return_val_if_fail (string != NULL, FALSE);
  g_return_val_if_fail (start_position >= 0, FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  if (string_len < 0)
    string_len = strlen(string);

  regex->string_len = string_len;

  /* perform the match */
  regex->matches = pcre_exec (regex->regex, regex->extra,
			      string, regex->string_len,
			      start_position,
			      regex->match_opts | match_options,
			      regex->offsets, regex->n_offsets);
  if (IS_PCRE_ERROR (regex->matches))
  {
    g_set_error (error, EGG_REGEX_ERROR, EGG_REGEX_ERROR_MATCH,
		 _("Error while matching regular expression %s"),
		 regex->pattern);
    return FALSE;
  }

  /* set regex->pos to -1 so that a call to egg_regex_match_next()
   * fails without a previous call to egg_regex_clear(). */
  regex->pos = -1;

  return regex->matches >= 0;
}

/**
 * egg_regex_match_next:
 * @regex: a #EggRegex structure.
 * @string: the string to scan for matches.
 * @match_options: the match options.
 *
 * Scans for the next match in @string of the pattern in @regex.
 * array.  The match options are combined with the match options set when
 * the @regex was created.
 *
 * You have to call egg_regex_clear() to reuse the same pattern on a new
 * string.
 *
 * Returns: %TRUE is the string matched, %FALSE otherwise.
 */
gboolean
egg_regex_match_next (EggRegex          *regex,
		    const gchar     *string,
		    EggRegexMatchFlags match_options)
{
  return egg_regex_match_next_full (regex, string, -1, 0,
				    match_options, NULL);
}

/**
 * egg_regex_match_next_full:
 * @regex: a #EggRegex structure.
 * @string: the string to scan for matches.
 * @string_len: the length of @string, or -1 if @string is nul-terminated.
 * @start_position: starting index of the string to match.
 * @match_options: the match options.
 * @error: location to store the error occuring, or NULL to ignore errors.
 *
 * Scans for the next match in @string of the pattern in @regex.
 * array.  The match options are combined with the match options set when
 * the @regex was created.
 *
 * You have to call egg_regex_clear() to reuse the same pattern on a new
 * string.
 *
 * Setting @start_position differs from just passing over a shortened string
 * and  setting #EGG_REGEX_MATCH_NOTBOL in the case of a pattern that begins
 * with any kind of lookbehind assertion, such as "\b".
 *
 * Returns: %TRUE is the string matched, %FALSE otherwise.
 */
gboolean
egg_regex_match_next_full (EggRegex          *regex,
			   const gchar       *string,
			   gssize             string_len,
			   gint               start_position,
			   EggRegexMatchFlags match_options,
			   GError           **error)
{
  g_return_val_if_fail (regex != NULL, FALSE);
  g_return_val_if_fail (string != NULL, FALSE);
  g_return_val_if_fail (start_position >= 0, FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  if G_UNLIKELY(regex->pos < 0)
    {
      const gchar *msg = "egg_regex_match_next_full: called without a "
                         "previous call to egg_regex_clear()";
      g_log (G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, msg);
      g_set_error (error, EGG_REGEX_ERROR, EGG_REGEX_ERROR_MATCH, msg);
      return FALSE;
    }

  /* if this regex hasn't been used on this string before, then we
   * need to calculate the length of the string, and set pos to the
   * start of it.
   * Knowing if this regex has been used on this string is a bit of
   * a challenge.  For now, we require the user to call egg_regex_clear()
   * in between usages on a new string.  Not perfect, but not such a
   * bad solution either.
   */
  if (regex->string_len == -1)
    {
      if (string_len < 0)
        string_len = strlen(string);
      regex->string_len = string_len;

      regex->pos = start_position;
    }


  /* perform the match */
  regex->matches = pcre_exec (regex->regex, regex->extra,
			      string, regex->string_len,
			      regex->pos,
			      regex->match_opts | match_options,
			      regex->offsets, regex->n_offsets);
  if (IS_PCRE_ERROR (regex->matches))
  {
    g_set_error (error, EGG_REGEX_ERROR, EGG_REGEX_ERROR_MATCH,
		 _("Error while matching regular expression %s"),
		 regex->pattern);
    return FALSE;
  }

  /* avoid infinite loops if regex is an empty string or something
   * equivalent */
  if (regex->pos == regex->offsets[1])
    {
      regex->pos = NEXT_CHAR_RE (regex, string + regex->pos) - string;
      if (regex->pos > regex->string_len)
	{
	  /* we have reached the end of the string */
	  regex->pos = -1;
	  return FALSE;
        }
    }
  else
    {
      regex->pos = regex->offsets[1];
    }

  return regex->matches >= 0;
}

/**
 * egg_regex_match_all:
 * @regex: a #EggRegex structure from egg_regex_new().
 * @string: the string to scan for matches.
 * @match_options: match options.
 *
 * Using the standard algorithm for regular expression matching only the
 * longest match in the string is retrieved. This function uses a
 * different algorithm so it can retrieve all the possible matches.
 * For more documentation see egg_regex_match_all_full().
 *
 * Returns: %TRUE is the string matched, %FALSE otherwise.
 */
gboolean
egg_regex_match_all (EggRegex          *regex,
		     const gchar       *string,
		     EggRegexMatchFlags match_options)
{
  return egg_regex_match_all_full (regex, string, -1, 0,
				   match_options, NULL);
}

/**
 * egg_regex_match_all_full:
 * @regex: a #EggRegex structure from egg_regex_new().
 * @string: the string to scan for matches.
 * @string_len: the length of @string, or -1 if @string is nul-terminated.
 * @start_position: starting index of the string to match.
 * @match_options: match options.
 * @error: location to store the error occuring, or NULL to ignore errors.
 *
 * Using the standard algorithm for regular expression matching only the
 * longest match in the string is retrieved, it is not possibile to obtain
 * all the available matches. For instance matching
 * "&lt;a&gt; &lt;b&gt; &lt;c&gt;" against the pattern "&lt;.*&gt;" you get
 * "&lt;a&gt; &lt;b&gt; &lt;c&gt;".
 *
 * This function uses a different algorithm (called DFA, i.e. deterministic
 * finite automaton), so it can retrieve all the possible matches, all
 * starting at the same point in the string. For instance matching
 * "&lt;a&gt; &lt;b&gt; &lt;c&gt;" against the pattern "&lt;.*&gt;" you
 * would obtain three matches: "&lt;a&gt; &lt;b&gt; &lt;c&gt;",
 * "&lt;a&gt; &lt;b&gt;" and "&lt;a&gt;".
 *
 * The number of matched strings is retrieved using
 * egg_regex_get_match_count().
 * To obtain the matched strings and their position you can use,
 * respectively, egg_regex_fetch() and egg_regex_fetch_pos(). Note that the
 * strings are returned in reverse order of length; that is, the longest
 * matching string is given first.
 *
 * Note that the DFA algorithm is slower than the standard one and it is not
 * able to capture substrings, so backreferences do not work.
 *
 * Setting @start_position differs from just passing over a shortened string
 * and  setting #EGG_REGEX_MATCH_NOTBOL in the case of a pattern that begins
 * with any kind of lookbehind assertion, such as "\b".
 *
 * Returns: %TRUE is the string matched, %FALSE otherwise.
 */
gboolean
egg_regex_match_all_full (EggRegex          *regex,
			  const gchar       *string,
			  gssize             string_len,
			  gint               start_position,
			  EggRegexMatchFlags match_options,
			  GError           **error)
{
  g_return_val_if_fail (regex != NULL, FALSE);
  g_return_val_if_fail (string != NULL, FALSE);
  g_return_val_if_fail (start_position >= 0, FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  if (string_len < 0)
    string_len = strlen(string);

  regex->string_len = string_len;

  if (regex->workspace == NULL)
    {
      regex->n_workspace = WORKSPACE_INITIAL;
      regex->workspace = g_new (gint, regex->n_workspace);
    }

  if (regex->n_offsets < OFFSETS_DFA_MIN_SIZE)
    {
      regex->n_offsets = OFFSETS_DFA_MIN_SIZE;
      regex->offsets = g_realloc (regex->offsets,
				  regex->n_offsets * sizeof(gint));
    }

  /* perform the match */
  regex->matches = pcre_dfa_exec (regex->regex, regex->extra,
				  string, regex->string_len,
				  start_position,
				  regex->match_opts | match_options,
				  regex->offsets, regex->n_offsets,
				  regex->workspace, regex->n_workspace);
  if (regex->matches == PCRE_ERROR_DFA_WSSIZE)
  {
    /* regex->workspace is too small. */
    regex->n_workspace *= 2;
    regex->workspace = g_realloc (regex->workspace,
				  regex->n_workspace * sizeof(gint));
    return egg_regex_match_all_full (regex, string, string_len,
				     start_position, match_options, error);
  }
  else if (regex->matches == 0)
  {
    /* regex->offsets is too small. */
    regex->n_offsets *= 2;
    regex->offsets = g_realloc (regex->offsets,
				regex->n_offsets * sizeof(gint));
    return egg_regex_match_all_full (regex, string, string_len,
				     start_position, match_options, error);
  }
  else if (IS_PCRE_ERROR (regex->matches))
  {
    g_set_error (error, EGG_REGEX_ERROR, EGG_REGEX_ERROR_MATCH,
		 _("Error while matching regular expression %s"),
		 regex->pattern);
    return FALSE;
  }

  /* set regex->pos to -1 so that a call to egg_regex_match_next()
   * fails without a previous call to egg_regex_clear(). */
  regex->pos = -1;

  return regex->matches >= 0;
}

/**
 * egg_regex_get_match_count:
 * @regex: a #EggRegex structure.
 *
 * Retrieves the number of matched substrings (including substring 0, that
 * is the whole matched text) in the last call to egg_regex_match*(), so 1
 * is returned if the pattern has no substrings in it and 0 is returned if
 * the match failed.
 *
 * If the last match was obtained using the DFA algorithm, that is using
 * egg_regex_match_all() or egg_regex_match_all_full(), the retrieved
 * count is not that of the number of capturing parentheses but that of
 * the number of matched substrings.
 *
 * Returns:  Number of matched substrings, or -1 if an error occurred.
 */
gint
egg_regex_get_match_count (const EggRegex *regex)
{
  g_return_val_if_fail (regex != NULL, -1);

  if (regex->matches == PCRE_ERROR_NOMATCH)
    /* no match */
    return 0;
  else if (regex->matches < PCRE_ERROR_NOMATCH)
    /* error */
    return -1;
  else
    /* match */
    return regex->matches;
}

/**
 * egg_regex_is_partial_match:
 * @regex: a #EggRegex structure.
 *
 * Usually if the string passed to egg_regex_match*() matches as far as
 * it goes, but is too short to match the entire pattern, %FALSE is
 * returned. There are circumstances where it might be helpful to
 * distinguish this case from other cases in which there is no match.
 *
 * Consider, for example, an application where a human is required to
 * type in data for a field with specific formatting requirements. An
 * example might be a date in the form ddmmmyy, defined by the pattern
 * "^\d?\d(jan|feb|mar|apr|may|jun|jul|aug|sep|oct|nov|dec)\d\d$".
 * If the application sees the user’s keystrokes one by one, and can
 * check that what has been typed so far is potentially valid, it is
 * able to raise an error as soon as a mistake is made.
 *
 * EggRegex supports the concept of partial matching by means of the
 * #EGG_REGEX_MATCH_PARTIAL flag. When this is set the return code for
 * egg_regex_match() or egg_regex_match_full() is, as usual, %TRUE
 * for a complete match, %FALSE otherwise. But, when this functions
 * returns %FALSE, you can check if the match was partial calling
 * egg_regex_is_partial_match().
 *
 * When using partial matching you cannot use egg_regex_fetch*().
 *
 * Because of the way certain internal optimizations are implemented the
 * partial matching algorithm cannot be used with all patterns. So repeated
 * single characters such as "a{2,4}" and repeated single metasequences such
 * as "\d+" are not permitted if the maximum number of occurrences is
 * greater than one. Optional items such as "\d?" (where the maximum is one)
 * are permitted. Quantifiers with any values are permitted after
 * parentheses, so the invalid examples above can be coded thus "(a){2,4}"
 * and "(\d)+". If #EGG_REGEX_MATCH_PARTIAL is set for a pattern that does
 * not conform to the restrictions, matching functions return an error.
 *
 * Returns: %TRUE if the match was partial, %FALSE otherwise.
 */
gboolean
egg_regex_is_partial_match (const EggRegex *regex)
{
  g_return_val_if_fail (regex != NULL, -1);

  return regex->matches == PCRE_ERROR_PARTIAL;
}

/**
 * egg_regex_fetch:
 * @regex: #EggRegex structure used in last match.
 * @match_num: number of the sub expression.
 * @string: the string on which the last match was made.
 *
 * Retrieves the text matching the @match_num<!-- -->'th capturing parentheses.
 * 0 is the full text of the match, 1 is the first paren set, 2 the second,
 * and so on.
 *
 * If @match_num is a valid sub pattern but it didn't match anything (e.g.
 * sub pattern 1, matching "b" against "(a)?b") then an empty string is
 * returned.
 *
 * If the last match was obtained using the DFA algorithm, that is using
 * egg_regex_match_all() or egg_regex_match_all_full(), the retrieved
 * string is not that of a set of parentheses but that of a matched
 * substring. Substrings are matched in reverse order of length, so 0 is
 * the longest match.
 *
 * Returns: The matched substring, or %NULL if an error occurred.
 *          You have to free the string yourself.
 */
gchar *
egg_regex_fetch (const EggRegex *regex,
	       gint         match_num,
	       const gchar *string)
{
  /* we cannot use pcre_get_substring() because it allocates the
   * string using pcre_malloc(). */
  gchar *match = NULL;
  gint start, end;

  g_return_val_if_fail (regex != NULL, NULL);
  g_return_val_if_fail (match_num >= 0, NULL);
  g_return_val_if_fail (regex->string_len >= 0, NULL);

  /* match_num does not exist or it didn't matched, i.e. matching "b"
   * against "(a)?b" then group 0 is empty. */
  if (!egg_regex_fetch_pos (regex, match_num, &start, &end))
    match = NULL;
  else if (start == -1)
    match = g_strdup ("");
  else
    match = g_strndup (&string[start], end - start);

  return match;
}

/**
 * egg_regex_fetch_pos:
 * @regex: #EggRegex structure used in last match.
 * @match_num: number of the sub expression.
 * @start_pos: pointer to location where to store the start position.
 * @end_pos: pointer to location where to store the end position.
 *
 * Retrieves the position of the @match_num<!-- -->'th capturing parentheses.
 * 0 is the full text of the match, 1 is the first paren set, 2 the second,
 * and so on.
 *
 * If @match_num is a valid sub pattern but it didn't match anything (e.g.
 * sub pattern 1, matching "b" against "(a)?b") then @start_pos and @end_pos
 * are set to -1 and %TRUE is returned.
 *
 * If the last match was obtained using the DFA algorithm, that is using
 * egg_regex_match_all() or egg_regex_match_all_full(), the retrieved
 * position is not that of a set of parentheses but that of a matched
 * substring. Substrings are matched in reverse order of length, so 0 is
 * the longest match.
 *
 * Returns: %TRUE if the position was fetched, %FALSE otherwise. If the
 *          position cannot be fetched, @start_pos and @end_pos are left
 *          unchanged.
 */
gboolean
egg_regex_fetch_pos (const EggRegex    *regex,
		   gint         match_num,
		   gint        *start_pos,
		   gint        *end_pos)
{
  g_return_val_if_fail (regex != NULL, FALSE);
  g_return_val_if_fail (match_num >= 0, FALSE);

  /* make sure the sub expression number they're requesting is less than
   * the total number of sub expressions that were matched. */
  if (match_num >= regex->matches)
    return FALSE;

  if (start_pos != NULL)
    {
      *start_pos = regex->offsets[2 * match_num];
    }

  if (end_pos != NULL)
    {
      *end_pos = regex->offsets[2 * match_num + 1];
    }

  return TRUE;
}

/**
 * egg_regex_fetch_named:
 * @regex: #EggRegex structure used in last match.
 * @name: name of the subexpression.
 * @string: the string on which the last match was made.
 *
 * Retrieves the text matching the capturing parentheses named @name.
 *
 * If @name is a valid sub pattern name but it didn't match anything (e.g.
 * sub pattern "X", matching "b" against "(?P&lt;X&gt;a)?b") then an empty
 * string is returned.
 *
 * Returns: The matched substring, or %NULL if an error occurred.
 *          You have to free the string yourself.
 */
gchar *
egg_regex_fetch_named (const EggRegex *regex,
		     const gchar  *name,
		     const gchar  *string)
{
  /* we cannot use pcre_get_named_substring() because it allocates the
   * string using pcre_malloc(). */
  gint num;

  g_return_val_if_fail (regex != NULL, NULL);
  g_return_val_if_fail (string != NULL, NULL);
  g_return_val_if_fail (name != NULL, NULL);

  num = egg_regex_get_string_number (regex, name);
  if (num == -1)
    return NULL;
  else
    return egg_regex_fetch (regex, num, string);
}

/**
 * egg_regex_fetch_named_pos:
 * @regex: #EggRegex structure used in last match.
 * @name: name of the subexpression.
 * @start_pos: pointer to location where to store the start position.
 * @end_pos: pointer to location where to store the end position.
 *
 * Retrieves the position of the capturing parentheses named @name.
 *
 * If @name is a valid sub pattern name but it didn't match anything (e.g.
 * sub pattern "X", matching "b" against "(?P&lt;X&gt;a)?b") then @start_pos and
 * @end_pos are set to -1 and %TRUE is returned.
 *
 * Returns: %TRUE if the position was fetched, %FALSE otherwise. If the
 *          position cannot be fetched, @start_pos and @end_pos are left
 *          unchanged.
 */
gboolean
egg_regex_fetch_named_pos (const EggRegex *regex,
			 const gchar  *name,
			 gint         *start_pos,
			 gint         *end_pos)
{
  gint num;

  num = egg_regex_get_string_number (regex, name);
  if (num == -1)
    return FALSE;

  return egg_regex_fetch_pos (regex, num, start_pos, end_pos);
}

/**
 * egg_regex_fetch_all:
 * @regex: a #EggRegex structure.
 * @string: the string on which the last match was made.
 *
 * Bundles up pointers to each of the matching substrings from a match
 * and stores them in an array of gchar pointers. The first element in
 * the returned array is the match number 0, i.e. the entire matched
 * text.
 *
 * If a sub pattern didn't match anything (e.g. sub pattern 1, matching
 * "b" against "(a)?b") then an empty string is inserted.
 *
 * If the last match was obtained using the DFA algorithm, that is using
 * egg_regex_match_all() or egg_regex_match_all_full(), the retrieved
 * strings are not that matched by sets of parentheses but that of the
 * matched substring. Substrings are matched in reverse order of length,
 * so the first one is the longest match.
 *
 * Returns: a %NULL-terminated array of gchar * pointers. It must be freed
 *          using g_strfreev(). If the memory can't be allocated, returns
 *          %NULL.
 */
gchar **
egg_regex_fetch_all (const EggRegex *regex,
		   const gchar  *string)
{
  gchar **listptr = NULL; /* the list pcre_get_substring_list() will fill */
  gchar **result;

  g_return_val_if_fail (regex != NULL, FALSE);
  g_return_val_if_fail (string != NULL, FALSE);

  if (regex->matches < 0)
    return NULL;

  pcre_get_substring_list (string, regex->offsets,
			   regex->matches, (const char ***)&listptr);

  if (listptr)
    {
      /* PCRE returns a single block of memory allocated
       * with pcre_malloc() which isn't suitable for
       * g_strfreev().
       */
      result = g_strdupv (listptr);
      pcre_free_substring_list ((const gchar **)listptr);
    }
  else
    result = NULL;

  return result;
}

/**
 * egg_regex_get_string_number:
 * @regex: #EggRegex structure.
 * @name: name of the subexpression.
 *
 * Retrieves the number of the subexpression named @name.
 *
 * Returns: The number of the subexpression or -1 if @name does not exists.
 */
gint
egg_regex_get_string_number (const EggRegex *regex,
			   const gchar  *name)
{
  gint num;

  g_return_val_if_fail (regex != NULL, -1);
  g_return_val_if_fail (name != NULL, -1);

  num = pcre_get_stringnumber (regex->regex, name);
  if (num == PCRE_ERROR_NOSUBSTRING)
	  num = -1;

  return num;
}

/**
 * egg_regex_split_simple:
 * @pattern: the regular expression.
 * @string: the string to scan for matches.
 * @compile_options: compile options for the regular expression.
 * @match_options: match options.
 *
 * Breaks the string on the pattern, and returns an array of the tokens.
 * If the pattern contains capturing parentheses, then the text for each
 * of the substrings will also be returned. If the pattern does not match
 * anywhere in the string, then the whole string is returned as the first
 * token.
 *
 * This function is equivalent to egg_regex_split() but it does not
 * require to compile the pattern with egg_regex_new(), avoiding some
 * lines of code when you need just to do a split without extracting
 * substrings, capture counts, and so on.
 *
 * If this function is to be called on the same @pattern more than
 * once, it's more efficient to compile the pattern once with
 * egg_regex_new() and then use egg_regex_split().
 *
 * Returns: a %NULL-terminated gchar ** array. Free it using g_strfreev().
 **/
gchar **
egg_regex_split_simple (const gchar        *pattern,
		      const gchar        *string,
		      EggRegexCompileFlags  compile_options,
		      EggRegexMatchFlags    match_options)
{
  EggRegex *regex;
  gchar **result;

  regex = egg_regex_new (pattern, compile_options, 0, NULL);
  if (!regex)
    return NULL;
  result = egg_regex_split_full (regex, string, -1, 0, match_options, 0, NULL);
  egg_regex_unref (regex);
  return result;
}

/**
 * egg_regex_split:
 * @regex:  a #EggRegex structure.
 * @string:  the string to split with the pattern.
 * @match_options:  match time option flags.
 *
 * Breaks the string on the pattern, and returns an array of the tokens.
 * If the pattern contains capturing parentheses, then the text for each
 * of the substrings will also be returned. If the pattern does not match
 * anywhere in the string, then the whole string is returned as the first
 * token.
 *
 * Returns: a %NULL-terminated gchar ** array. Free it using g_strfreev().
 **/
gchar **
egg_regex_split (EggRegex           *regex,
	       const gchar      *string,
	       EggRegexMatchFlags  match_options)
{
  return egg_regex_split_full (regex, string, -1, 0,
                               match_options, 0, NULL);
}

/**
 * egg_regex_split_full:
 * @regex:  a #EggRegex structure.
 * @string:  the string to split with the pattern.
 * @string_len: the length of @string, or -1 if @string is nul-terminated.
 * @start_position: starting index of the string to match.
 * @match_options:  match time option flags.
 * @max_tokens: the maximum number of tokens to split @string into. If this
 *    is less than 1, the string is split completely.
 * @error: return location for a #GError.
 *
 * Breaks the string on the pattern, and returns an array of the tokens.
 * If the pattern contains capturing parentheses, then the text for each
 * of the substrings will also be returned. If the pattern does not match
 * anywhere in the string, then the whole string is returned as the first
 * token.
 *
 * Setting @start_position differs from just passing over a shortened string
 * and  setting #EGG_REGEX_MATCH_NOTBOL in the case of a pattern that begins
 * with any kind of lookbehind assertion, such as "\b".
 *
 * Returns: a %NULL-terminated gchar ** array. Free it using g_strfreev().
 **/
gchar **
egg_regex_split_full (EggRegex           *regex,
		      const gchar      *string,
		      gssize            string_len,
		      gint              start_position,
		      EggRegexMatchFlags  match_options,
		      gint              max_tokens,
		      GError          **error)
{
  gchar **string_list;		/* The array of char **s worked on */
  gint pos;
  gboolean match_ok;
  gint match_count;
  gint tokens;
  gint new_pos;
  gchar *token;
  GList *list, *last;
  GError *tmp_error = NULL;

  g_return_val_if_fail (regex != NULL, NULL);
  g_return_val_if_fail (string != NULL, NULL);
  g_return_val_if_fail (start_position >= 0, NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  /* we need to reset the state of the regex as we are going to use
   * egg_regex_match_next_full(). */
  egg_regex_clear (regex);

  new_pos = start_position;
  tokens = 0;
  list = NULL;
  while (TRUE)
    {
      if ((max_tokens < 1) || (tokens <  max_tokens - 1))
        {
	  match_ok = egg_regex_match_next_full (regex, string, string_len,
                                                new_pos, match_options,
                                                &tmp_error);
	  if (tmp_error != NULL)
	    {
	      g_propagate_error (error, tmp_error);
	      g_list_foreach (list, (GFunc)g_free, NULL);
	      g_list_free (list);
	      return NULL;
	    }
	}
      else
	{
	  match_ok = FALSE;
	  if (regex->string_len < 0)
	    {
	      /* egg_regex_match_next_full() wasn't called, so
	       * regex->string_len can be -1, but we need a valid
	       * length to copy the last part of the string. */
	      if (string_len < 0)
		string_len = strlen (string);
	      regex->string_len = string_len;
	    }
	}

      if (match_ok)
	{
	  token = g_strndup (string + new_pos, regex->offsets[0] - new_pos);
	  list = g_list_prepend (list, token);

	  /* if there were substrings, these need to get added to the
	   * list as well */
	  match_count = egg_regex_get_match_count (regex);
	  if (match_count > 1)
	    {
	      gint i;
	      for (i = 1; i < match_count; i++)
		list = g_list_prepend (list, egg_regex_fetch (regex, i, string));
	    }

	  new_pos = regex->pos;	/* move new_pos to end of match */
	  tokens++;
	}
      else	 /* if there was no match, copy to end of string, and break */
	{
	  token = g_strndup (string + new_pos, regex->string_len - new_pos);
	  list = g_list_prepend (list, token);
	  break;
	}
    }

  string_list = g_new (gchar *, g_list_length (list) + 1);
  pos = 0;
  for (last = g_list_last (list); last; last = g_list_previous (last))
    string_list[pos++] = last->data;
  string_list[pos] = 0;

  g_list_free (list);
  return string_list;
}

/**
 * egg_regex_split_next:
 * @regex: a #EggRegex structure from egg_regex_new().
 * @string:  the string to split on pattern.
 * @match_options:  match time options for the regex.
 *
 * egg_regex_split_next() breaks the string on pattern, and returns the
 * tokens, one per call.  If the pattern contains capturing parentheses,
 * then the text for each of the substrings will also be returned.
 * If the pattern does not match anywhere in the string, then the whole
 * string is returned as the first token.
 *
 * You have to call egg_regex_clear() to reuse the same pattern on a new
 * string.
 *
 * Returns:  a gchar * to the next token of the string.
 */
gchar *
egg_regex_split_next (EggRegex          *regex,
		    const gchar     *string,
		    EggRegexMatchFlags match_options)
{
  return egg_regex_split_next_full (regex, string, -1, 0, match_options,
                                    NULL);
}

/**
 * egg_regex_split_next_full:
 * @regex: a #EggRegex structure from egg_regex_new().
 * @string:  the string to split on pattern.
 * @string_len: the length of @string, or -1 if @string is nul-terminated.
 * @start_position: starting index of the string to match.
 * @match_options:  match time options for the regex.
 * @error: return location for a #GError.
 *
 * egg_regex_split_next_full() breaks the string on pattern, and returns
 * the tokens, one per call.  If the pattern contains capturing parentheses,
 * then the text for each of the substrings will also be returned.
 * If the pattern does not match anywhere in the string, then the whole
 * string is returned as the first token.
 *
 * You have to call egg_regex_clear() to reuse the same pattern on a new
 * string.
 *
 * Setting @start_position differs from just passing over a shortened string
 * and  setting #EGG_REGEX_MATCH_NOTBOL in the case of a pattern that begins
 * with any kind of lookbehind assertion, such as "\b".
 *
 * Returns:  a gchar * to the next token of the string.
 */
gchar *
egg_regex_split_next_full (EggRegex          *regex,
			   const gchar     *string,
			   gssize           string_len,
			   gint             start_position,
			   EggRegexMatchFlags match_options,
			   GError         **error)
{
  gint new_pos;
  gchar *token = NULL;
  gboolean match_ok;
  gint match_count;
  GError *tmp_error = NULL;

  g_return_val_if_fail (regex != NULL, NULL);
  g_return_val_if_fail (string != NULL, NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  new_pos = MAX (regex->pos, start_position);

  /* if there are delimiter substrings stored, return those one at a
   * time.
   */
  if (regex->delims != NULL)
    {
      token = regex->delims->data;
      regex->delims = g_slist_remove (regex->delims, token);
      return token;
    }

  if (regex->pos == -1)
    /* the last call to egg_regex_match_next_full() returned NULL. */
    return NULL;

  /* use egg_regex_match_next() to find the next occurance of the pattern
   * in the string.  We use new_pos to keep track of where the stuff
   * up to the current match starts.  Copy that token of the string off
   * and append it to the buffer using g_strndup.  We have to NUL term the
   * token we copied off before returning it.
   */
  match_ok = egg_regex_match_next_full (regex, string, string_len,
                                        start_position, match_options,
                                        &tmp_error);
  if (tmp_error != NULL)
    {
      g_propagate_error (error, tmp_error);
      return NULL;
    }

  if (match_ok)
    {
      token = g_strndup (string + new_pos, regex->offsets[0] - new_pos);

      /* if there were substrings, these need to get added to the
       * list of delims */
      match_count = egg_regex_get_match_count (regex);
      if (match_count > 1)
	{
	  gint i;
	  for (i = 1; i < match_count; i++)
	    regex->delims = g_slist_append (regex->delims,
					    egg_regex_fetch (regex, i, string));
	}
    }
  else		/* if there was no match, copy to end of string */
    token = g_strndup (string + new_pos, regex->string_len - new_pos);

  return token;
}

enum
{
  REPL_TYPE_STRING,
  REPL_TYPE_CHARACTER,
  REPL_TYPE_SYMBOLIC_REFERENCE,
  REPL_TYPE_NUMERIC_REFERENCE
};

typedef struct
{
  gchar *text;
  gint   type;
  gint   num;
  gchar  c;
} InterpolationData;

static void
free_interpolation_data (InterpolationData *data)
{
  g_free (data->text);
  g_free (data);
}

static const gchar *
expand_escape (const gchar        *replacement,
	       const gchar        *p,
	       InterpolationData  *data,
               gboolean            utf8,
	       GError            **error)
{
  const gchar *q, *r;
  gint x, d, h, i;
  const gchar *error_detail;
  gint base = 0;
  GError *tmp_error = NULL;

  p++;
  switch (*p)
    {
    case 't':
      p++;
      data->c = '\t';
      data->type = REPL_TYPE_CHARACTER;
      break;
    case 'n':
      p++;
      data->c = '\n';
      data->type = REPL_TYPE_CHARACTER;
      break;
    case 'v':
      p++;
      data->c = '\v';
      data->type = REPL_TYPE_CHARACTER;
      break;
    case 'r':
      p++;
      data->c = '\r';
      data->type = REPL_TYPE_CHARACTER;
      break;
    case 'f':
      p++;
      data->c = '\f';
      data->type = REPL_TYPE_CHARACTER;
      break;
    case 'a':
      p++;
      data->c = '\a';
      data->type = REPL_TYPE_CHARACTER;
      break;
    case 'b':
      p++;
      data->c = '\b';
      data->type = REPL_TYPE_CHARACTER;
      break;
    case '\\':
      p++;
      data->c = '\\';
      data->type = REPL_TYPE_CHARACTER;
      break;
    case 'x':
      p++;
      x = 0;
      if (*p == '{')
	{
	  p++;
	  do
	    {
	      h = g_ascii_xdigit_value (*p);
	      if (h < 0)
		{
		  error_detail = _("hexadecimal digit or '}' expected");
		  goto error;
		}
	      x = x * 16 + h;
	      p++;
	    }
	  while (*p != '}');
	  p++;
	}
      else
	{
	  for (i = 0; i < 2; i++)
	    {
	      h = g_ascii_xdigit_value (*p);
	      if (h < 0)
		{
		  error_detail = _("hexadecimal digit expected");
		  goto error;
		}
	      x = x * 16 + h;
	      p++;
	    }
	}
      data->type = REPL_TYPE_STRING;
      data->text = g_new0 (gchar, 8);
      g_unichar_to_utf8 (x, data->text);
      break;
    case 'l':
    case 'u':
    case 'L':
    case 'U':
    case 'E':
    case 'Q':
    case 'G':
      error_detail = _("escape sequence not allowed");
      goto error;
    case 'g':
      p++;
      if (*p != '<')
	{
	  error_detail = _("missing '<' in symbolic reference");
	  goto error;
	}
      q = p + 1;
      do
	{
	  p++;
	  if (!*p)
	    {
	      error_detail = _("unfinished symbolic reference");
	      goto error;
	    }
	}
      while (*p != '>');
      if (p - q == 0)
	{
	  error_detail = _("zero-length symbolic reference");
	  goto error;
	}
      if (g_ascii_isdigit (*q))
	{
	  x = 0;
	  do
	    {
	      h = g_ascii_digit_value (*q);
	      if (h < 0)
		{
		  error_detail = _("digit expected");
		  p = q;
		  goto error;
		}
	      x = x * 10 + h;
	      q++;
	    }
	  while (q != p);
	  data->num = x;
	  data->type = REPL_TYPE_NUMERIC_REFERENCE;
	}
      else
	{
	  r = q;
	  do
	    {
	      if (!g_ascii_isalnum (*r))
		{
		  error_detail = _("illegal symbolic reference");
		  p = r;
		  goto error;
		}
	      r++;
	    }
	  while (r != p);
	  data->text = g_strndup (q, p - q);
	  data->type = REPL_TYPE_SYMBOLIC_REFERENCE;
	}
      p++;
      break;
    case '0':
      /* if \0 is followed by a number is an octal number representing a
       * character, else it is a numeric reference. */
      if (g_ascii_digit_value (*NEXT_CHAR (p, utf8)) >= 0)
        {
          base = 8;
          p = NEXT_CHAR (p, utf8);
        }
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      x = 0;
      d = 0;
      for (i = 0; i < 3; i++)
	{
	  h = g_ascii_digit_value (*p);
	  if (h < 0)
	    break;
	  if (h > 7)
	    {
	      if (base == 8)
		break;
	      else
		base = 10;
	    }
	  if (i == 2 && base == 10)
	    break;
	  x = x * 8 + h;
	  d = d * 10 + h;
	  p++;
	}
      if (base == 8 || i == 3)
	{
	  data->type = REPL_TYPE_STRING;
	  data->text = g_new0 (gchar, 8);
	  g_unichar_to_utf8 (x, data->text);
	}
      else
	{
	  data->type = REPL_TYPE_NUMERIC_REFERENCE;
	  data->num = d;
	}
      break;
    case 0:
      error_detail = _("stray final '\\'");
      goto error;
      break;
    default:
      data->type = REPL_TYPE_STRING;
      data->text = g_new0 (gchar, 8);
      g_unichar_to_utf8 (g_utf8_get_char (p), data->text);
      p = NEXT_CHAR (p, utf8);
    }

  return p;

 error:
  tmp_error = g_error_new (EGG_REGEX_ERROR,
			   EGG_REGEX_ERROR_REPLACE,
			   _("Error while parsing replacement "
			     "text \"%s\" at char %d: %s"),
			   replacement,
			   p - replacement,
			   error_detail);
  g_propagate_error (error, tmp_error);

  return NULL;
}

static GList *
split_replacement (const gchar  *replacement,
                   gboolean      utf8,
		   GError      **error)
{
  GList *list = NULL;
  InterpolationData *data;
  const gchar *p, *start;

  start = p = replacement;
  while (*p)
    {
      if (*p == '\\')
	{
	  data = g_new0 (InterpolationData, 1);
	  start = p = expand_escape (replacement, p, data, utf8, error);
	  if (p == NULL)
	    {
	      g_list_foreach (list, (GFunc)free_interpolation_data, NULL);
	      g_list_free (list);

	      return NULL;
	    }
	  list = g_list_prepend (list, data);
	}
      else
	{
	  p++;
	  if (*p == '\\' || *p == '\0')
	    {
	      if (p - start > 0)
		{
		  data = g_new0 (InterpolationData, 1);
		  data->text = g_strndup (start, p - start);
		  data->type = REPL_TYPE_STRING;
		  list = g_list_prepend (list, data);
		}
	    }
	}
    }

  return g_list_reverse (list);
}

static gboolean
interpolate_replacement (const EggRegex *regex,
			 const gchar  *string,
			 GString      *result,
			 gpointer      data)
{
  GList *list;
  InterpolationData *idata;
  gchar *match;

  for (list = data; list; list = list->next)
    {
      idata = list->data;
      switch (idata->type)
	{
	case REPL_TYPE_STRING:
	  g_string_append (result, idata->text);
	  break;
	case REPL_TYPE_CHARACTER:
	  g_string_append_c (result, idata->c);
	  break;
	case REPL_TYPE_NUMERIC_REFERENCE:
	  match = egg_regex_fetch (regex, idata->num, string);
	  if (match)
	    {
	      g_string_append (result, match);
	      g_free (match);
	    }
	  break;
	case REPL_TYPE_SYMBOLIC_REFERENCE:
	  match = egg_regex_fetch_named (regex, idata->text, string);
	  if (match)
	    {
	      g_string_append (result, match);
	      g_free (match);
	    }
	  break;
	}
    }

  return FALSE;
}

/**
 * egg_regex_replace:
 * @regex:  a #EggRegex structure.
 * @string:  the string to perform matches against.
 * @string_len: the length of @string, or -1 if @string is nul-terminated.
 * @start_position: starting index of the string to match.
 * @replacement:  text to replace each match with.
 * @match_options:  options for the match.
 * @error: location to store the error occuring, or NULL to ignore errors.
 *
 * Replaces all occurances of the pattern in @regex with the
 * replacement text. Backreferences of the form '\number' or '\g&lt;number&gt;'
 * in the replacement text are interpolated by the number-th captured
 * subexpression of the match, '\g&lt;name&gt;' refers to the captured subexpression
 * with the given name. '\0' refers to the complete match, but '\0' followed
 * by a number is the octal representation of a character. To include a
 * literal '\' in the replacement, write '\\'. If you do not need to use
 * backreferences use egg_regex_replace_literal().
 *
 * Setting @start_position differs from just passing over a shortened string
 * and  setting #EGG_REGEX_MATCH_NOTBOL in the case of a pattern that begins
 * with any kind of lookbehind assertion, such as "\b".
 *
 * Returns: a newly allocated string containing the replacements.
 */
gchar *
egg_regex_replace (EggRegex            *regex,
		 const gchar       *string,
		 gssize             string_len,
		 gint               start_position,
		 const gchar       *replacement,
		 EggRegexMatchFlags   match_options,
		 GError           **error)
{
  gchar *result;
  GList *list;

  g_return_val_if_fail (regex != NULL, NULL);
  g_return_val_if_fail (string != NULL, NULL);
  g_return_val_if_fail (start_position >= 0, NULL);
  g_return_val_if_fail (replacement != NULL, NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  list = split_replacement (replacement, REGEX_UTF8 (regex), error);
  result = egg_regex_replace_eval (regex,
				 string, string_len, start_position,
				 match_options,
				 interpolate_replacement,
				 (gpointer)list);
  g_list_foreach (list, (GFunc)free_interpolation_data, NULL);
  g_list_free (list);

  return result;
}

static gboolean
literal_replacement (G_GNUC_UNUSED const EggRegex *regex,
                     G_GNUC_UNUSED const gchar *string,
		     GString      *result,
		     gpointer      data)
{
  g_string_append (result, data);
  return FALSE;
}

/**
 * egg_regex_replace_literal:
 * @regex:  a #EggRegex structure.
 * @string:  the string to perform matches against.
 * @string_len: the length of @string, or -1 if @string is nul-terminated.
 * @start_position: starting index of the string to match.
 * @replacement:  text to replace each match with.
 * @match_options:  options for the match.
 *
 * Replaces all occurances of the pattern in @regex with the
 * replacement text. @replacement is replaced literally, to
 * include backreferences use egg_regex_replace().
 *
 * Setting @start_position differs from just passing over a shortened string
 * and  setting #EGG_REGEX_MATCH_NOTBOL in the case of a pattern that begins
 * with any kind of lookbehind assertion, such as "\b".
 *
 * Returns: a newly allocated string containing the replacements.
 */
gchar *
egg_regex_replace_literal (EggRegex          *regex,
			 const gchar     *string,
			 gssize           string_len,
			 gint             start_position,
			 const gchar     *replacement,
			 EggRegexMatchFlags match_options)
{
  g_return_val_if_fail (replacement != NULL, NULL);

  return egg_regex_replace_eval (regex,
			       string, string_len, start_position,
			       match_options,
			       literal_replacement,
			       (gpointer)replacement);
}

/**
 * egg_regex_replace_eval:
 * @regex: a #EggRegex structure from egg_regex_new().
 * @string:  string to perform matches against.
 * @string_len: the length of @string, or -1 if @string is nul-terminated.
 * @start_position: starting index of the string to match.
 * @match_options:  Options for the match.
 * @eval: a function to call for each match.
 * @user_data: user data to pass to the function.
 *
 * Replaces occurances of the pattern in regex with the output of @eval
 * for that occurance.
 *
 * Setting @start_position differs from just passing over a shortened string
 * and  setting #EGG_REGEX_MATCH_NOTBOL in the case of a pattern that begins
 * with any kind of lookbehind assertion, such as "\b".
 *
 * Returns: a newly allocated string containing the replacements.
 */
gchar *
egg_regex_replace_eval (EggRegex            *regex,
		      const gchar       *string,
		      gssize             string_len,
		      gint               start_position,
		      EggRegexMatchFlags   match_options,
		      EggRegexEvalCallback eval,
		      gpointer           user_data)
{
  GString *result;
  gint str_pos = 0;
  gboolean done = FALSE;

  g_return_val_if_fail (regex != NULL, NULL);
  g_return_val_if_fail (string != NULL, NULL);
  g_return_val_if_fail (start_position >= 0, NULL);
  g_return_val_if_fail (eval != NULL, NULL);

  if (string_len < 0)
    string_len = strlen(string);

  /* clear out the regex for reuse, just in case */
  egg_regex_clear (regex);

  result = g_string_sized_new (string_len);

  /* run down the string making matches. */
  while (!done &&
	 egg_regex_match_next_full (regex, string, string_len,
				    start_position, match_options, NULL))
    {
      g_string_append_len (result,
			   string + str_pos,
			   regex->offsets[0] - str_pos);
      done = (*eval) (regex, string, result, user_data);
      str_pos = regex->offsets[1];
    }

  g_string_append_len (result, string + str_pos, string_len - str_pos);

  return g_string_free (result, FALSE);
}

/**
 * egg_regex_escape_string:
 * @string: the string to escape.
 * @length: the length of @string, or -1 if @string is nul-terminated.
 *
 * Escapes the special characters used for regular expressions in @string,
 * for instance "a.b*c" becomes "a\.b\*c". This function is useful to
 * dynamically generate regular expressions.
 *
 * @string can contain NULL characters that are replaced with "\0", in this
 * case remember to specify the correct length of @string in @length.
 *
 * Returns: a newly allocated escaped string.
 */
gchar *
egg_regex_escape_string (const gchar *string,
		       gint         length)
{
  GString *escaped;
  const char *p, *piece_start, *end;

  g_return_val_if_fail (string != NULL, NULL);

  if (length < 0)
    length = strlen (string);

  end = string + length;
  p = piece_start = string;
  escaped = g_string_sized_new (length + 1);

  while (p < end)
    {
      switch (*p)
	{
          case '\0':
	  case '\\':
	  case '|':
	  case '(':
	  case ')':
	  case '[':
	  case ']':
	  case '{':
	  case '}':
	  case '^':
	  case '$':
	  case '*':
	  case '+':
	  case '?':
	  case '.':
	    if (p != piece_start)
	      /* copy the previous piece. */
	      g_string_append_len (escaped, piece_start, p - piece_start);
	    g_string_append_c (escaped, '\\');
            if (*p == '\0')
              g_string_append_c (escaped, '0');
            else
	      g_string_append_c (escaped, *p);
	    piece_start = ++p;
	    break;
	  default:
	    p = g_utf8_next_char (p);
      }
  }

  if (piece_start < end)
      g_string_append_len (escaped, piece_start, end - piece_start);

  return g_string_free (escaped, FALSE);
}



/*****************************************************************************/
/* muntyan's additions
 */
gboolean
egg_regex_escape (const char *string,
                  int         bytes,
                  GString    *dest)
{
    const char *p, *piece, *end;
    gboolean escaped = FALSE;

    g_return_val_if_fail (string != NULL, TRUE);
    g_return_val_if_fail (dest != NULL, TRUE);

    if (bytes < 0)
        bytes = strlen (string);

    end = string + bytes;
    p = piece = string;

    while (p < end)
    {
        switch (*p)
        {
            case '\\':
            case '|':
            case '(':
            case ')':
            case '[':
            case ']':
            case '{':
            case '}':
            case '^':
            case '$':
            case '*':
            case '+':
            case '?':
            case '.':
                escaped = TRUE;
                if (p != piece)
                    g_string_append_len (dest, piece, p - piece);
                g_string_append_c (dest, '\\');
                g_string_append_c (dest, *p);
                piece = ++p;
                break;

            default:
                p = g_utf8_next_char (p);
                break;
        }
    }

    if (escaped && piece < end)
        g_string_append_len (dest, piece, end - piece);

    return escaped;
}


/**
 * egg_regex_check_replacement:
 * @replacement: replacement string
 * @has_references: location for information about references
 * @error: location to store error
 */
gboolean
egg_regex_check_replacement (const gchar *replacement,
                             gboolean    *has_references,
                             GError     **error)
{
    GList *list, *l;
    GError *tmp = NULL;

    list = split_replacement (replacement, TRUE, &tmp);

    if (tmp)
    {
        g_propagate_error (error, tmp);
        return FALSE;
    }

    if (has_references)
    {
        *has_references = FALSE;

        for (l = list; l != NULL; l = l->next)
        {
            InterpolationData *data = l->data;

            if (data->type == REPL_TYPE_SYMBOLIC_REFERENCE ||
                data->type == REPL_TYPE_NUMERIC_REFERENCE)
            {
                *has_references = TRUE;
                break;
            }
        }
    }

    g_list_foreach (list, (GFunc) free_interpolation_data, NULL);
    g_list_free (list);

    return TRUE;
}


char *
egg_regex_try_eval_replacement (EggRegex   *regex,
                                const char *replacement,
                                GError    **error)
{
    GList *list, *l;
    GError *tmp = NULL;
    GString *string;
    gboolean result = TRUE;
    InterpolationData *idata;

    g_return_val_if_fail (regex != NULL, NULL);
    g_return_val_if_fail (replacement != NULL, NULL);

    if (!*replacement)
        return g_strdup ("");

    list = split_replacement (replacement, TRUE, &tmp);

    if (tmp)
    {
        g_propagate_error (error, tmp);
        return NULL;
    }

    if (!list)
        return g_strdup ("");

    string = g_string_new (NULL);

    for (l = list; l && result; l = l->next)
    {
        idata = l->data;

        switch (idata->type)
        {
            case REPL_TYPE_STRING:
                g_string_append (string, idata->text);
                break;

            case REPL_TYPE_CHARACTER:
                g_string_append_c (string, idata->c);
                break;

            case REPL_TYPE_NUMERIC_REFERENCE:
            case REPL_TYPE_SYMBOLIC_REFERENCE:
                g_set_error (error, EGG_REGEX_ERROR,
                             EGG_REGEX_ERROR_REPLACE,
                             "Pattern '%s' contains internal references, can't "
                                     "evaluate replacement without matching",
                             egg_regex_get_pattern (regex));
                result = FALSE;
                break;
        }
    }

    g_list_foreach (list, (GFunc) free_interpolation_data, NULL);
    g_list_free (list);

    if (result)
    {
        return g_string_free (string, FALSE);
    }
    else
    {
        g_string_free (string, TRUE);
        return NULL;
    }
}


/**
 * egg_regex_eval_replacement:
 * @gregex:  a #EggRegex structure
 * @string: the string on which the last match was made
 * @replacement: replacement string
 * @error: location to store error
 *
 * Evaluates replacement after successful match.
 *
 * Returns: a newly allocated string containing the replacement.
 */
char *
egg_regex_eval_replacement (EggRegex   *regex,
                            const char *string,
                            const char *replacement,
                            GError    **error)
{
    GString *result;
    GList *list;

    g_return_val_if_fail (replacement != NULL, NULL);

    if (!*replacement)
        return g_strdup ("");

    list = split_replacement (replacement, TRUE, error);

    if (!list)
        return NULL;

    result = g_string_new (NULL);
    interpolate_replacement (regex, string, result, list);
    g_list_foreach (list, (GFunc) free_interpolation_data, NULL);
    g_list_free (list);

    return g_string_free (result, FALSE);
}
