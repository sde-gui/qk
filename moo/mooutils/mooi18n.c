/*
 *   mooi18n.c
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

#include "mooutils/mooi18n.h"
#include "mooutils/mooutils-misc.h"
#include <glib.h>


const char *
moo_gettext (const char *string)
{
#ifdef ENABLE_NLS
    static gboolean been_here = FALSE;

    g_return_val_if_fail (string != NULL, NULL);

    if (!been_here)
    {
        been_here = TRUE;
        bindtextdomain (GETTEXT_PACKAGE, moo_get_locale_dir ());
#ifdef HAVE_BIND_TEXTDOMAIN_CODESET
        bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
#endif
    }

    return dgettext (GETTEXT_PACKAGE, string);
#else /* !ENABLE_NLS */
    return string;
#endif /* !ENABLE_NLS */
}

char *
_moo_dgettext (const char *domain, const char *string)
{
#ifdef ENABLE_NLS
    gchar *tmp;
    const gchar *translated;

    g_return_val_if_fail (string != NULL, NULL);

    if (domain == NULL)
        return g_strdup (moo_gettext (string));

    translated = dgettext (domain, string);

    if (strcmp (translated, string) == 0)
        return g_strdup (moo_gettext (string));

    if (g_utf8_validate (translated, -1, NULL))
        return g_strdup (translated);

    tmp = g_locale_to_utf8 (translated, -1, NULL, NULL, NULL);

    if (tmp == NULL)
        return g_strdup (string);
    else
        return tmp;
#else
    return g_strdup (string);
#endif /* !ENABLE_NLS */
}
