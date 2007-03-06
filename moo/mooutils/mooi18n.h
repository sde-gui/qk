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

#define _(String) moo_gettext (String)
#define Q_(String) g_strip_context ((String), moo_gettext (String))
#define N_(String) (String)
#define D_(String,Domain) dgettext (Domain, String)
#define QD_(String,Domain) g_strip_context ((String), D_ (String, Domain))

#else /* !ENABLE_NLS */

#undef textdomain
#undef gettext
#undef dgettext
#undef dcgettext
#undef ngettext
#undef bindtextdomain
#undef bind_textdomain_codeset
#define _(String) (String)
#define N_(String) (String)
#define Q_(String) g_strip_context ((String), (String))
#define D_(String,Domain) (String)
#define QD_(String,Domain) g_strip_context ((String), (String))
#define textdomain(String) (String)
#define gettext(String) (String)
#define dgettext(Domain,String) (String)
#define dcgettext(Domain,String,Type) (String)
#define ngettext(str,str_pl,n) ((n) > 1 ? (str_pl) ? (str))
#define dngettext(dom,str,str_pl,n) ((n) > 1 ? (str_pl) ? (str))
#define bindtextdomain(Domain,Directory) (Domain)
#define bind_textdomain_codeset(Domain,Codeset) (Codeset)

#endif /* !ENABLE_NLS */


const char *moo_gettext (const char *string);


G_END_DECLS

#endif /* __MOO_I18N_H__ */
