/* EggRegex -- regular expression API wrapper around PCRE.
 *
 * Copyright (C) 1999, 2000 Scott Wimer
 * Copyright (C) 2004, Matthias Clasen <mclasen@redhat.com>
 * Copyright (C) 2005 - 2006, Marco Barisione <marco@barisione.org>
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

/* Modified by muntyan to adapt for libmoo */

#ifndef __EGG_REGEX_H__
#define __EGG_REGEX_H__

#include <glib.h>

G_BEGIN_DECLS


#define egg_regex_error_quark _moo_egg_regex_error_quark
#define egg_regex_new _moo_egg_regex_new
#define egg_regex_free _moo_egg_regex_free
#define egg_regex_optimize _moo_egg_regex_optimize
#define egg_regex_copy _moo_egg_regex_copy
#define egg_regex_get_pattern _moo_egg_regex_get_pattern
#define egg_regex_clear _moo_egg_regex_clear
#define egg_regex_match_simple _moo_egg_regex_match_simple
#define egg_regex_match _moo_egg_regex_match
#define egg_regex_match_full _moo_egg_regex_match_full
#define egg_regex_match_next _moo_egg_regex_match_next
#define egg_regex_match_next_full _moo_egg_regex_match_next_full
#define egg_regex_match_all _moo_egg_regex_match_all
#define egg_regex_match_all_full _moo_egg_regex_match_all_full
#define egg_regex_get_match_count _moo_egg_regex_get_match_count
#define egg_regex_is_partial_match _moo_egg_regex_is_partial_match
#define egg_regex_fetch _moo_egg_regex_fetch
#define egg_regex_fetch_pos _moo_egg_regex_fetch_pos
#define egg_regex_fetch_named _moo_egg_regex_fetch_named
#define egg_regex_fetch_named_pos _moo_egg_regex_fetch_named_pos
#define egg_regex_fetch_all _moo_egg_regex_fetch_all
#define egg_regex_get_string_number _moo_egg_regex_get_string_number
#define egg_regex_split_simple _moo_egg_regex_split_simple
#define egg_regex_split _moo_egg_regex_split
#define egg_regex_split_full _moo_egg_regex_split_full
#define egg_regex_split_next _moo_egg_regex_split_next
#define egg_regex_split_next_full _moo_egg_regex_split_next_full
#define egg_regex_expand_references _moo_egg_regex_expand_references
#define egg_regex_replace _moo_egg_regex_replace
#define egg_regex_replace_literal _moo_egg_regex_replace_literal
#define egg_regex_replace_eval _moo_egg_regex_replace_eval
#define egg_regex_escape_string _moo_egg_regex_escape_string
#define egg_regex_escape _moo_egg_regex_escape
#define egg_regex_check_replacement _moo_egg_regex_check_replacement
#define egg_regex_eval_replacement _moo_egg_regex_eval_replacement
#define egg_regex_try_eval_replacement _moo_egg_regex_try_eval_replacement
#define _egg_regex_get_memory _moo_egg_regex_get_memory


typedef enum
{
  EGG_REGEX_ERROR_COMPILE,
  EGG_REGEX_ERROR_OPTIMIZE,
  EGG_REGEX_ERROR_REPLACE,
  EGG_REGEX_ERROR_MATCH
} EggRegexError;

#define EGG_REGEX_ERROR egg_regex_error_quark ()

GQuark egg_regex_error_quark (void);

/* Remember to update EGG_REGEX_COMPILE_ALL in gregex.c after
 * adding a new flag. */
typedef enum
{
  EGG_REGEX_CASELESS          = 1 << 0,
  EGG_REGEX_MULTILINE         = 1 << 1,
  EGG_REGEX_DOTALL            = 1 << 2,
  EGG_REGEX_EXTENDED          = 1 << 3,
  EGG_REGEX_ANCHORED          = 1 << 4,
  EGG_REGEX_DOLLAR_ENDONLY    = 1 << 5,
  EGG_REGEX_UNGREEDY          = 1 << 9,
  EGG_REGEX_RAW               = 1 << 11,
  EGG_REGEX_NO_AUTO_CAPTURE   = 1 << 12,
  EGG_REGEX_DUPNAMES          = 1 << 19,
  EGG_REGEX_NEWLINE_CR        = 1 << 20,
  EGG_REGEX_NEWLINE_CRLF      = 1 << 21 | EGG_REGEX_NEWLINE_CR
} EggRegexCompileFlags;

/* Remember to update EGG_REGEX_MATCH_ALL in gregex.c after
 * adding a new flag. */
typedef enum
{
  EGG_REGEX_MATCH_ANCHORED      = 1 << 4,
  EGG_REGEX_MATCH_NOTBOL        = 1 << 7,
  EGG_REGEX_MATCH_NOTEOL        = 1 << 8,
  EGG_REGEX_MATCH_NOTEMPTY      = 1 << 10,
  EGG_REGEX_MATCH_PARTIAL       = 1 << 15,
  EGG_REGEX_MATCH_NEWLINE_CR    = 1 << 20,
  EGG_REGEX_MATCH_NEWLINE_LF    = 1 << 21,
  EGG_REGEX_MATCH_NEWLINE_CRLF  = EGG_REGEX_MATCH_NEWLINE_CR | EGG_REGEX_MATCH_NEWLINE_LF
} EggRegexMatchFlags;

typedef struct _EggRegex  EggRegex;

typedef gboolean (*EggRegexEvalCallback) (const EggRegex*, const gchar*, GString*, gpointer);


EggRegex	 *egg_regex_new			(const gchar         *pattern,
						 EggRegexCompileFlags   compile_options,
						 EggRegexMatchFlags     match_options,
						 GError             **error);
void		  egg_regex_free		(EggRegex              *regex);
gboolean	  egg_regex_optimize		(EggRegex              *regex,
						 GError             **error);
EggRegex	 *egg_regex_copy		(const EggRegex        *regex);
const gchar	 *egg_regex_get_pattern		(const EggRegex        *regex);
void		  egg_regex_clear		(EggRegex              *regex);
gboolean	  egg_regex_match_simple	(const gchar         *pattern,
						 const gchar         *string,
						 EggRegexCompileFlags   compile_options,
						 EggRegexMatchFlags     match_options);
gboolean	  egg_regex_match		(EggRegex              *regex,
						 const gchar         *string,
						 EggRegexMatchFlags     match_options);
gboolean	  egg_regex_match_full		(EggRegex              *regex,
						 const gchar         *string,
						 gssize               string_len,
						 gint                 start_position,
						 EggRegexMatchFlags     match_options,
						 GError             **error);
gboolean	  egg_regex_match_next		(EggRegex              *regex,
						 const gchar         *string,
						 EggRegexMatchFlags     match_options);
gboolean	  egg_regex_match_next_full	(EggRegex              *regex,
						 const gchar         *string,
						 gssize               string_len,
						 gint                 start_position,
						 EggRegexMatchFlags     match_options,
						 GError             **error);
gboolean	  egg_regex_match_all		(EggRegex              *regex,
						 const gchar         *string,
						 EggRegexMatchFlags     match_options);
gboolean	  egg_regex_match_all_full	(EggRegex              *regex,
						 const gchar         *string,
						 gssize               string_len,
						 gint                 start_position,
						 EggRegexMatchFlags     match_options,
						 GError             **error);
gint		  egg_regex_get_match_count	(const EggRegex        *regex);
gboolean	  egg_regex_is_partial_match	(const EggRegex        *regex);
gchar		 *egg_regex_fetch		(const EggRegex        *regex,
						 gint                 match_num,
						 const gchar         *string);
gboolean	  egg_regex_fetch_pos		(const EggRegex        *regex,
						 gint                 match_num,
						 gint                *start_pos,
						 gint                *end_pos);
gchar		 *egg_regex_fetch_named		(const EggRegex        *regex,
						 const gchar         *name,
						 const gchar         *string);
gboolean	  egg_regex_fetch_named_pos	(const EggRegex        *regex,
						 const gchar         *name,
						 gint                *start_pos,
						 gint                *end_pos);
gchar		**egg_regex_fetch_all		(const EggRegex        *regex,
						 const gchar         *string);
gint		  egg_regex_get_string_number	(const EggRegex        *regex,
						 const gchar         *name);
gchar		**egg_regex_split_simple	(const gchar         *pattern,
						 const gchar         *string,
						 EggRegexCompileFlags   compile_options,
						 EggRegexMatchFlags     match_options);
gchar		**egg_regex_split		(EggRegex              *regex,
						 const gchar         *string,
						 EggRegexMatchFlags     match_options);
gchar		**egg_regex_split_full		(EggRegex              *regex,
						 const gchar         *string,
						 gssize               string_len,
						 gint                 start_position,
						 EggRegexMatchFlags     match_options,
						 gint                 max_tokens,
						 GError             **error);
gchar		 *egg_regex_split_next		(EggRegex              *regex,
						 const gchar         *string,
						 EggRegexMatchFlags     match_options);
gchar		 *egg_regex_split_next_full	(EggRegex              *regex,
						 const gchar         *string,
						 gssize               string_len,
						 gint                 start_position,
						 EggRegexMatchFlags     match_options,
						 GError             **error);
gchar		 *egg_regex_expand_references	(EggRegex              *regex,
						 const gchar         *string,
						 const gchar         *string_to_expand,
						 GError             **error);
gchar		 *egg_regex_replace		(EggRegex              *regex,
						 const gchar         *string,
						 gssize               string_len,
						 gint                 start_position,
						 const gchar         *replacement,
						 EggRegexMatchFlags     match_options,
						 GError             **error);
gchar		 *egg_regex_replace_literal	(EggRegex              *regex,
						 const gchar         *string,
						 gssize               string_len,
						 gint                 start_position,
						 const gchar         *replacement,
						 EggRegexMatchFlags     match_options,
						 GError             **error);
gchar		 *egg_regex_replace_eval	(EggRegex              *regex,
						 const gchar         *string,
						 gssize               string_len,
						 gint                 start_position,
						 EggRegexMatchFlags     match_options,
						 EggRegexEvalCallback   eval,
						 gpointer             user_data,
						 GError             **error);
gchar		 *egg_regex_escape_string	(const gchar         *string,
						 gint                 length);

/*****************************************************************************/
/* muntyan's additions
 */
gboolean    egg_regex_escape                (const char *string,
                                             int         bytes,
                                             GString    *dest);
gboolean    egg_regex_check_replacement     (const char *replacement,
                                             gboolean   *has_references,
                                             GError    **error);
char       *egg_regex_eval_replacement      (EggRegex   *regex,
                                             const char *string,
                                             const char *replacement,
                                             GError    **error);
char       *egg_regex_try_eval_replacement  (EggRegex   *regex,
                                             const char *replacement,
                                             GError    **error);

gsize       _egg_regex_get_memory           (EggRegex   *regex);


G_END_DECLS


#endif  /*  __EGG_REGEX_H__ */
