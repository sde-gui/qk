#include "mooutils/gspawn/gspawn.h"
#include <string.h>
#include <windows.h>
#include <io.h>
#include <glib.h>

inline static gchar *
_glib_get_dll_directory (void)
{
  gchar *retval;
  gchar *p;
  wchar_t wc_fn[MAX_PATH];

  /* This code is different from that in
   * g_win32_get_package_installation_directory_of_module() in that
   * here we return the actual folder where the GLib DLL is. We don't
   * do the check for it being in a "bin" or "lib" subfolder and then
   * returning the parent of that.
   *
   * In a statically built GLib, glib_dll will be NULL and we will
   * thus look up the application's .exe file's location.
   */
  if (!GetModuleFileNameW (NULL, wc_fn, MAX_PATH))
    return NULL;

  retval = g_utf16_to_utf8 (wc_fn, -1, NULL, NULL, NULL);

  p = strrchr (retval, G_DIR_SEPARATOR);
  if (p == NULL)
    {
      /* Wtf? */
      return NULL;
    }
  *p = '\0';

  return retval;
}
