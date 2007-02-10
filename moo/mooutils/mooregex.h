/*
 *   mooregex.h
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef __MOO_REGEX_H__
#define __MOO_REGEX_H__

#include <glib.h>

G_BEGIN_DECLS


typedef enum
{
  MOO_REGEX_CASELESS          = 1 << 0,
  MOO_REGEX_MULTILINE         = 1 << 1,
  MOO_REGEX_DOTALL            = 1 << 2,
  MOO_REGEX_EXTENDED          = 1 << 3,
  MOO_REGEX_ANCHORED          = 1 << 4,
  MOO_REGEX_DOLLAR_ENDONLY    = 1 << 5,
  MOO_REGEX_UNGREEDY          = 1 << 9,
  MOO_REGEX_RAW               = 1 << 11,
  MOO_REGEX_NO_AUTO_CAPTURE   = 1 << 12,
  MOO_REGEX_DUPNAMES          = 1 << 19,
  MOO_REGEX_NEWLINE_CR        = 1 << 20,
  MOO_REGEX_NEWLINE_CRLF      = 1 << 21 | MOO_REGEX_NEWLINE_CR
} MooRegexCompileFlags;

typedef enum
{
  MOO_REGEX_MATCH_ANCHORED      = 1 << 4,
  MOO_REGEX_MATCH_NOTBOL        = 1 << 7,
  MOO_REGEX_MATCH_NOTEOL        = 1 << 8,
  MOO_REGEX_MATCH_NOTEMPTY      = 1 << 10,
  MOO_REGEX_MATCH_PARTIAL       = 1 << 15,
  MOO_REGEX_MATCH_NEWLINE_CR    = 1 << 20,
  MOO_REGEX_MATCH_NEWLINE_LF    = 1 << 21,
  MOO_REGEX_MATCH_NEWLINE_CRLF  = MOO_REGEX_MATCH_NEWLINE_CR | MOO_REGEX_MATCH_NEWLINE_LF
} MooRegexMatchFlags;

typedef struct _MooRegex MooRegex;


MooRegex    *moo_regex_new      (const char             *pattern,
                                 MooRegexCompileFlags    compile_options,
                                 MooRegexMatchFlags      match_options,
                                 GError                **error);
void         moo_regex_free     (MooRegex               *regex);
void         moo_regex_clear    (MooRegex               *regex);
gboolean     moo_regex_match    (MooRegex               *regex,
                                 const char             *string,
                                 MooRegexMatchFlags      match_options);
gchar       *moo_regex_fetch    (MooRegex               *regex,
                                 int                     match_num,
                                 const char             *string);


G_END_DECLS

#endif /* __MOO_REGEX_H__ */
