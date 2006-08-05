/*
 *   completion-plugin.c
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

#include "config.h"
#include "completion.h"
#include "mooedit/mooplugin-macro.h"
#include "mooedit/plugins/mooeditplugins.h"
#include "mooutils/mooi18n.h"


static gboolean cmpl_plugin_init        (CmplPlugin     *plugin);
static void     cmpl_plugin_deinit      (CmplPlugin     *plugin);


static gboolean
cmpl_plugin_init (CmplPlugin *plugin)
{
    MooWindowClass *klass = g_type_class_ref (MOO_TYPE_EDIT_WINDOW);
    MooEditor *editor = moo_editor_instance ();
    MooUIXML *xml = moo_editor_get_ui_xml (editor);

    g_return_val_if_fail (klass != NULL, FALSE);
    g_return_val_if_fail (editor != NULL, FALSE);

    moo_window_class_new_action (klass, "CompleteWord",
                                 "display-name", _("Complete Word"),
                                 "label", _("Complete Word"),
                                 "tooltip", _("Complete Word"),
                                 "accel", "<Ctrl>space",
                                 "closure-callback", _completion_callback,
                                 NULL);

    if (xml)
    {
        plugin->ui_merge_id = moo_ui_xml_new_merge_id (xml);
        moo_ui_xml_add_item (xml, plugin->ui_merge_id,
                             "Editor/Menubar/Edit",
                             "CompleteWord", "CompleteWord", -1);
    }

    _cmpl_plugin_load (plugin);

    g_type_class_unref (klass);
    return TRUE;
}


static void
cmpl_plugin_deinit (CmplPlugin *plugin)
{
    MooWindowClass *klass = g_type_class_ref (MOO_TYPE_EDIT_WINDOW);
    MooEditor *editor = moo_editor_instance ();
    MooUIXML *xml = moo_editor_get_ui_xml (editor);

    _cmpl_plugin_clear (plugin);

    moo_window_class_remove_action (klass, "CompleteWord");

    if (plugin->ui_merge_id)
        moo_ui_xml_remove_ui (xml, plugin->ui_merge_id);
    plugin->ui_merge_id = 0;

    g_type_class_unref (klass);
}


MOO_PLUGIN_DEFINE_INFO (cmpl,
                        N_("Completion"), N_("Makes it complete"),
                        "Yevgen Muntyan <muntyan@tamu.edu>",
                        MOO_VERSION, NULL)
MOO_PLUGIN_DEFINE_FULL (Cmpl, cmpl,
                        cmpl_plugin_init, cmpl_plugin_deinit,
                        NULL, NULL, NULL, NULL,
                        NULL, 0, 0)


gboolean
_moo_completion_plugin_init (void)
{
    return moo_plugin_register (CMPL_PLUGIN_ID,
                                cmpl_plugin_get_type (),
                                &cmpl_plugin_info,
                                NULL);
}
