#include <gmodule.h>
#include "mooedit/mooplugin.h"
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef MOO_VERSION
#define MOO_VERSION NULL
#endif

gboolean dummy_init (void);


static gboolean dummy_plugin_init (void)
{
    return TRUE;
}


G_MODULE_EXPORT gboolean
dummy_init (void)
{
    MooPluginParams params = {
        TRUE
    };

    MooPluginPrefsParams prefs_params;

    MooPluginInfo info = {
        MOO_PLUGIN_CURRENT_VERSION,
        "Dummy",
        "Dummy",
        "Dummy plugin",
        "Yevgen Muntyan <muntyan@tamu.edu>",
        MOO_VERSION,
        (MooPluginInitFunc) dummy_plugin_init,
        NULL, /* MooEditPluginDeinitFunc */
        NULL, /* MooEditPluginWindowAttachFunc */
        NULL, /* MooEditPluginWindowDetachFunc */
        &params,
        &prefs_params
    };

    return moo_plugin_register (&info, NULL);
}
