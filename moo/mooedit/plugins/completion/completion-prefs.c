/*
 *   completion-prefs.c
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

#include "completion.h"
#include "mooedit/plugins/completion/completion-glade.h"
#include "mooutils/mooprefsdialogpage.h"


GtkWidget *
_cmpl_plugin_prefs_page (MooPlugin *plugin)
{
    GtkWidget *page;
    MooGladeXML *xml;

    xml = moo_glade_xml_new_empty ();
    moo_glade_xml_map_id (xml, "page", MOO_TYPE_PREFS_DIALOG_PAGE);
    page = moo_prefs_dialog_page_new_from_xml ("Completion", GTK_STOCK_CONVERT,
                                               xml, COMPLETION_GLADE_UI, -1,
                                               "page", NULL);

    g_object_set_data_full (G_OBJECT (xml), "cmpl-plugin",
                            g_object_ref (plugin), (GDestroyNotify) g_object_unref);

//     g_signal_connect_swapped (page, "apply", G_CALLBACK (prefs_page_apply), xml);
//     g_signal_connect_swapped (page, "init", G_CALLBACK (prefs_page_init), xml);
//     g_signal_connect_swapped (page, "destroy", G_CALLBACK (prefs_page_destroy), xml);

    g_object_unref (xml);
    return page;
}
