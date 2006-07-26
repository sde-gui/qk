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

#ifndef __EGGREGEX_H__
#define __EGGREGEX_H__

#include <glib/gtypes.h>
#include <glib/gquark.h>
#include <glib/gerror.h>
#include <glib/gstring.h>

G_BEGIN_DECLS

typedef enum
{
  EGG_REGEX_ERROR_COMPILE,
  EGG_REGEX_ERROR_OPTIMIZE,
  EGG_REGEX_ERROR_REPLACE,
  EGG_REGEX_ERROR_MATCH
} EggRegexError;

#define EGG_REGEX_ERROR egg_regex_error_quark ()

GQuark egg_regex_error_quark (void);

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
  EGG_REGEX_NO_AUTO_CAPTURE   = 1 << 12
} EggRegexCompileFlags;

typedef enum
{
  EGG_REGEX_MATCH_ANCHORED    = 1 << 4,
  EGG_REGEX_MATCH_NOTBOL      = 1 << 7,
  EGG_REGEX_MATCH_NOTEOL      = 1 << 8,
  EGG_REGEX_MATCH_NOTEMPTY    = 1 << 10,
  EGG_REGEX_MATCH_PARTIAL     = 1 << 15
} EggRegexMatchFlags;

typedef struct _EggRegex  EggRegex;

typedef gboolean (*EggRegexEvalCallback) (const EggRegex*, const gchar*, GString*, gpointer);


EggRegex  *egg_regex_new          (const gchar           *pattern,
				   EggRegexCompileFlags   compile_options,
				   EggRegexMatchFlags     match_options,
				   GError               **error);
gboolean   egg_regex_optimize     (EggRegex              *regex,
				   GError               **error);
EggRegex  *egg_regex_ref	  (EggRegex              *regex);
void       egg_regex_unref	  (EggRegex              *regex);
EggRegex  *egg_regex_copy	  (const EggRegex        *regex);
const gchar * egg_regex_get_pattern
				  (const EggRegex        *regex);
void       egg_regex_clear        (EggRegex              *regex);
gboolean   egg_regex_match_simple (const gchar           *pattern,
				   const gchar           *string,
				   EggRegexCompileFlags   compile_options,
				   EggRegexMatchFlags     match_options);
gboolean   egg_regex_match        (EggRegex              *regex,
				   const gchar           *string,
				   EggRegexMatchFlags     match_options);
gboolean   egg_regex_match_full   (EggRegex              *regex,
				   const gchar           *string,
				   gssize                 string_len,
				   gint                   start_position,
				   EggRegexMatchFlags     match_options,
				   GError               **error);
gboolean   egg_regex_match_next   (EggRegex              *regex,
				   const gchar           *string,
				   EggRegexMatchFlags     match_options);
gboolean   egg_regex_match_next_full
				  (EggRegex              *regex,
				   const gchar           *string,
				   gssize                 string_len,
				   gint                   start_position,
				   EggRegexMatchFlags     match_options,
				   GError               **error);
gboolean   egg_regex_match_all    (EggRegex              *regex,
				   const gchar           *string,
				   EggRegexMatchFlags     match_options);
gboolean   egg_regex_match_all_full
				  (EggRegex              *regex,
				   const gchar           *string,
				   gssize                 string_len,
				   gint                   start_position,
				   EggRegexMatchFlags     match_options,
				   GError               **error);
gint       egg_regex_get_match_count
				  (const EggRegex        *regex);
gboolean   egg_regex_is_partial_match
				  (const EggRegex        *regex);
gchar     *egg_regex_fetch        (const EggRegex        *regex,
				   gint                   match_num,
				   const gchar           *string);
gboolean   egg_regex_fetch_pos    (const EggRegex        *regex,
				   gint                   match_num,
				   gint                  *start_pos,
				   gint                  *end_pos);
gchar     *egg_regex_fetch_named  (const EggRegex        *regex,
				   const gchar           *name,
				   const gchar           *string);
gboolean   egg_regex_fetch_named_pos
				  (const EggRegex        *regex,
				   const gchar           *name,
				   gint                  *start_pos,
				   gint                  *end_pos);
gchar    **egg_regex_fetch_all    (const EggRegex        *regex,
				   const gchar           *string);
gint       egg_regex_get_string_number
				  (const EggRegex        *regex,
				   const gchar           *name);
gchar    **egg_regex_split_simple (const gchar           *pattern,
				   const gchar           *string,
				   EggRegexCompileFlags   compile_options,
				   EggRegexMatchFlags     match_options);
gchar    **egg_regex_split	  (EggRegex              *regex,
				   const gchar           *string,
				   EggRegexMatchFlags     match_options);
gchar    **egg_regex_split_full   (EggRegex              *regex,
				   const gchar           *string,
				   gssize                 string_len,
				   gint                   start_position,
				   EggRegexMatchFlags     match_options,
				   gint                   max_tokens,
				   GError               **error);
gchar     *egg_regex_split_next	  (EggRegex              *regex,
				   const gchar           *string,
				   EggRegexMatchFlags     match_options);
gchar     *egg_regex_split_next_full
				  (EggRegex              *regex,
				   const gchar           *string,
				   gssize                 string_len,
				   gint                   start_position,
				   EggRegexMatchFlags     match_options,
				   GError               **error);
gchar     *egg_regex_replace      (EggRegex              *regex,
				   const gchar           *string,
				   gssize                 string_len,
				   gint                   start_position,
				   const gchar           *replacement,
				   EggRegexMatchFlags     match_options,
				   GError               **error);
gchar     *egg_regex_replace_literal
				  (EggRegex              *regex,
				   const gchar           *string,
				   gssize                 string_len,
				   gint                   start_position,
				   const gchar           *replacement,
				   EggRegexMatchFlags     match_options);
gchar     *egg_regex_replace_eval (EggRegex              *regex,
				   const gchar           *string,
				   gssize                 string_len,
				   gint                   start_position,
				   EggRegexMatchFlags     match_options,
				   EggRegexEvalCallback   eval,
				   gpointer               user_data);
gchar     *egg_regex_escape_string
				  (const gchar           *string,
				   gint                   length);


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


G_END_DECLS


#endif  /*  __EGGREGEX_H__ */
