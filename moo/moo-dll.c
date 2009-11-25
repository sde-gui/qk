#include <glib.h>
#include <windows.h>
#include <mooutils/mooutils-macros.h>

HINSTANCE _moo_hinst_dll = NULL;

BOOL WINAPI DllMain (HINSTANCE hinstDLL, G_GNUC_UNUSED DWORD fdwReason, G_GNUC_UNUSED LPVOID lpvReserved)
{
    _moo_hinst_dll = hinstDLL;
    return TRUE;
}
