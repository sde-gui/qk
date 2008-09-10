/*
 *   mooi18n.c
 *
 *   Copyright (C) 2004-2008 by Yevgen Muntyan <muntyan@tamu.edu>
 *
 *   This file is part of medit.  medit is free software; you can
 *   redistribute it and/or modify it under the terms of the
 *   GNU Lesser General Public License as published by the
 *   Free Software Foundation; either version 2.1 of the License,
 *   or (at your option) any later version.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with medit.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "mooutils/mooi18n.h"
#include "mooutils/mooutils-misc.h"
#include "mooutils/moocompat.h"
#include <glib.h>


#ifdef ENABLE_NLS
static void
init_gettext (void)
{
    static gboolean been_here = FALSE;

    if (!been_here)
    {
        been_here = TRUE;
        bindtextdomain (GETTEXT_PACKAGE, moo_get_locale_dir ());
        bindtextdomain (GETTEXT_PACKAGE "-gsv", moo_get_locale_dir ());
#ifdef HAVE_BIND_TEXTDOMAIN_CODESET
        bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
        bind_textdomain_codeset (GETTEXT_PACKAGE "-gsv", "UTF-8");
#endif
    }
}
#endif /* ENABLE_NLS */

const char *
moo_gettext (const char *string)
{
#ifdef ENABLE_NLS
    g_return_val_if_fail (string != NULL, NULL);
    init_gettext ();
    return dgettext (GETTEXT_PACKAGE, string);
#else /* !ENABLE_NLS */
    return string;
#endif /* !ENABLE_NLS */
}

const char *
moo_pgettext (const char *msgctxtid, gsize msgidoffset)
{
#ifdef ENABLE_NLS
    g_return_val_if_fail (msgctxtid != NULL, NULL);
    init_gettext ();
    return moo_dpgettext (GETTEXT_PACKAGE, msgctxtid, msgidoffset);
#else /* !ENABLE_NLS */
    return msgctxtid;
#endif /* !ENABLE_NLS */
}

const char *
moo_pgettext2 (const char *context, const char *msgctxtid)
{
#ifdef ENABLE_NLS
    char *tmp;
    const char *translation;

    g_return_val_if_fail (msgctxtid != NULL, NULL);
    init_gettext ();

    tmp = g_strjoin (context, "\004", msgctxtid, NULL);
    translation = dgettext (GETTEXT_PACKAGE, tmp);

    if (translation == tmp)
        translation = msgctxtid;

    g_free (tmp);
    return translation;
#else /* !ENABLE_NLS */
    return msgctxtid;
#endif /* !ENABLE_NLS */
}

const char *
moo_dpgettext (const char *domain, const char *msgctxtid, gsize msgidoffset)
{
    g_return_val_if_fail (domain != NULL, NULL);
    g_return_val_if_fail (msgctxtid != NULL, NULL);
#ifdef ENABLE_NLS
    return g_dpgettext (GETTEXT_PACKAGE, msgctxtid, msgidoffset);
#else
    return msgctxtid + msgidoffset;
#endif
}

const char *
_moo_gsv_gettext (const char *string)
{
#ifdef ENABLE_NLS
    g_return_val_if_fail (string != NULL, NULL);
    init_gettext ();
    return dgettext (GETTEXT_PACKAGE "-gsv", string);
#else /* !ENABLE_NLS */
    return string;
#endif /* !ENABLE_NLS */
}

char *
_moo_gsv_dgettext (const char *domain, const char *string)
{
#ifdef ENABLE_NLS
    gchar *tmp;
    const gchar *translated;

    g_return_val_if_fail (string != NULL, NULL);

    init_gettext ();

    if (domain == NULL)
        return g_strdup (_moo_gsv_gettext (string));

    translated = dgettext (domain, string);

    if (strcmp (translated, string) == 0)
        return g_strdup (_moo_gsv_gettext (string));

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
