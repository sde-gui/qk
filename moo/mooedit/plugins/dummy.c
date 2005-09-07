#include <gmodule.h>
#include "mooedit/mooeditplugin.h"
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef MOO_VERSION
#define MOO_VERSION NULL
#endif

gboolean dummy_init (void);


static GtkWidget *create_prefs_page (void)
{
    return moo_prefs_dialog_page_new ("Dummy", GTK_STOCK_YES);
}

static gboolean dummy_plugin_init (void)
{
    return TRUE;
}


G_MODULE_EXPORT gboolean
dummy_init (void)
{
    MooEditPluginParams params = {
        TRUE, FALSE, 0
    };

    MooEditPluginPrefsParams prefs_params = {
        (MooEditPluginPrefsPageCreateFunc) create_prefs_page
    };

    MooEditPluginInfo info = {
        MOO_EDIT_PLUGIN_CURRENT_VERSION,
        "Dummy",
        "Dummy",
        "Dummy plugin",
        "Yevgen Muntyan <muntyan@tamu.edu>",
        MOO_VERSION,
        (MooEditPluginInitFunc) dummy_plugin_init,
        NULL, /* MooEditPluginDeinitFunc */
        NULL, /* MooEditPluginWindowAttachFunc */
        NULL, /* MooEditPluginWindowDetachFunc */
        NULL, /* MooEditPluginPaneCreateFunc */
        NULL, /* MooEditPluginPaneDestroyFunc */
        &params,
        &prefs_params
    };

    return moo_edit_plugin_register (&info, NULL);
}
