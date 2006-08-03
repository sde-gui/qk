/*
 *   mooi18n.h
 *
 *   Copyright (C) 2004-2006 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef __MOO_I18N_H__
#define __MOO_I18N_H__

#include <config.h>
#include <glib/gstrfuncs.h>
#include <libintl.h>

G_BEGIN_DECLS


#ifdef ENABLE_NLS

#define _(String) _moo_gettext (String)
#define Q_(String) g_strip_context ((String), _moo_gettext (String))
#define N_(String) (String)
#define D_(String,Domain) dgettext (Domain, String)

const char *_moo_gettext (const char *string);

#else /* !ENABLE_NLS */

#define _(String) (String)
#define N_(String) (String)
#define Q_(String) (String)
#define D_(String,Domain) (String)
#define textdomain(String) (String)
#define gettext(String) (String)
#define dgettext(Domain,String) (String)
#define dcgettext(Domain,String,Type) (String)
#define bindtextdomain(Domain,Directory) (Domain)
#define bind_textdomain_codeset(Domain,Codeset) (Codeset)

#endif /* !ENABLE_NLS */


G_END_DECLS

#endif /* __MOO_I18N_H__ */
