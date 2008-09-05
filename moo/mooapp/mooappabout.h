/*
 *   mooapp/mooappabout.h
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

#ifndef MOO_APP_ABOUT_H
#define MOO_APP_ABOUT_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_CONFIGARGS_H
#include "configargs.h"
#endif

#ifdef HAVE_SYS_UTSNAME_H
#include <sys/utsname.h>
#endif

#ifdef __WIN32__
#include <windows.h>
#endif

#include "mooutils/moopython.h"
#include <errno.h>
#include <gtk/gtk.h>

#if defined(MOO_OS_MINGW)
#define MOO_OS_NAME "Win32"
#elif defined(MOO_OS_DARWIN)
#define MOO_OS_NAME "Darwin"
#elif defined(MOO_OS_BSD)
#define MOO_OS_NAME "BSD"
#elif defined(MOO_OS_UNIX)
#define MOO_OS_NAME "UNIX"
#else
#define MOO_OS_NAME "Unknown???"
#endif


#ifdef HAVE_SYS_UTSNAME_H
static char *
get_uname (void)
{
    struct utsname name;

    if (uname (&name))
    {
        int err = errno;
        g_critical ("%s: %s", G_STRLOC, g_strerror (err));
        return NULL;
    }

    return g_strdup_printf ("%s %s (%s), %s", name.sysname,
                            name.release, name.version, name.machine);
}
#elif defined __WIN32__
static char *
get_windows_name (void)
{
    OSVERSIONINFOEXW ver;

    memset (&ver, 0, sizeof (ver));
    ver.dwOSVersionInfoSize = sizeof (OSVERSIONINFOW);

    if (!GetVersionExW ((OSVERSIONINFOW*) &ver))
        return g_strdup ("Win32");

    switch (ver.dwMajorVersion)
    {
        case 4: /* Windows NT 4.0, Windows Me, Windows 98, or Windows 95 */
            switch (ver.dwMinorVersion)
            {
                case 0: /* Windows NT 4.0 or Windows95 */
                    if (ver.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
                        return g_strdup ("Windows 95");
                    else
                        return g_strdup ("Windows NT 4.0");

                case 10:
                    return g_strdup ("Windows 98");

                case 90:
                    return g_strdup ("Windows 98");
            }

            break;

        case 5: /* Windows Server 2003 R2, Windows Server 2003, Windows XP, or Windows 2000 */
            switch (ver.dwMinorVersion)
            {
                case 0:
                    return g_strdup ("Windows 2000");
                case 1:
                    return g_strdup ("Windows XP");
                case 2:
                    return g_strdup ("Windows Server 2003");
            }

            break;

        case 6:
            memset (&ver, 0, sizeof (ver));
            ver.dwOSVersionInfoSize = sizeof (OSVERSIONINFOEXW);

            if (!GetVersionExW ((OSVERSIONINFOW*) &ver))
                return g_strdup ("Windows Vista");

            if (ver.wProductType == VER_NT_WORKSTATION)
                return g_strdup ("Windows Vista");
            else
                return g_strdup ("Windows Server \"Longhorn\"");
    }

    return g_strdup ("Win32");
}
#else
static char *
get_uname (void)
{
    char *string;

    if (g_spawn_command_line_sync ("uname -a", &string, NULL, NULL, NULL))
        return string;
    else
        return NULL;
}
#endif


static char *
get_python_info (void)
{
    if (!moo_python_running ())
        return NULL;

    return moo_python_get_info ();
}


#endif /* MOO_APP_ABOUT_H */
