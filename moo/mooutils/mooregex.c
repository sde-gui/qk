/*
 *   mooregex.c
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

#include "mooutils/mooregex.h"
#include "mooutils/eggregex.h"


MooRegex *
moo_regex_new (const gchar           *pattern,
               MooRegexCompileFlags   compile_options,
               MooRegexMatchFlags     match_options,
               GError               **error)
{
    EggRegex *regex;

    regex = egg_regex_new (pattern, compile_options, match_options, error);

    if (regex)
        egg_regex_optimize (regex, NULL);

    return (MooRegex*) regex;
}


void
moo_regex_free (MooRegex *regex)
{
    egg_regex_free ((EggRegex*) regex);
}


void
moo_regex_clear (MooRegex *regex)
{
    egg_regex_clear ((EggRegex*) regex);
}


gboolean
moo_regex_match (MooRegex           *regex,
                 const char         *string,
                 MooRegexMatchFlags  match_options)
{
    return egg_regex_match ((EggRegex*) regex, string, match_options);
}


gchar *
moo_regex_fetch (MooRegex   *regex,
                 int         match_num,
                 const char *string)
{
    return egg_regex_fetch ((EggRegex*) regex, match_num, string);
}
