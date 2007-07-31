/*
 *   mooindenter-settings.h
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef MOOEDIT_COMPILATION
#error "This file may not be included"
#endif

#ifndef MOO_INDENTER_SETTINGS_H
#define MOO_INDENTER_SETTINGS_H

#include "mooindenter.h"

G_BEGIN_DECLS


#ifdef __OBJC__
@class MooIndenterRegex;
#else
typedef struct MooIndenterRegex MooIndenterRegex;
#endif

MooIndenterRegex    *_moo_indenter_get_regex        (const char         *id_);
MooIndenterRegex    *_moo_indenter_regex_ref        (MooIndenterRegex   *regex);
void                 _moo_indenter_regex_unref      (MooIndenterRegex   *regex);
gboolean             _moo_indenter_regex_newline    (MooIndenterRegex   *regex,
                                                     MooIndenter        *indenter,
                                                     GtkTextIter        *where);


G_END_DECLS

#endif /* MOO_INDENTER_SETTINGS_H */
