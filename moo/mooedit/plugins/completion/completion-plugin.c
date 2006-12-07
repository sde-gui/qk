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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "completion.h"
#include "mooedit/mooplugin-macro.h"
#include "mooedit/plugins/mooeditplugins.h"
#include "mooutils/mooi18n.h"
#include "mooutils/moomarshals.h"


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

    moo_window_class_new_action (klass, "CompleteWord", NULL,
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

    plugin->cmpl_quark = g_quark_from_static_string ("moo-completion");
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


static void
set_lang_completion_meth (CmplPlugin        *plugin,
                          const char        *lang,
                          MooTextCompletion *cmpl)
{
    _completion_plugin_set_lang_completion (plugin, lang, cmpl);
}

static void
set_doc_completion_meth (CmplPlugin        *plugin,
                         MooEdit           *doc,
                         MooTextCompletion *cmpl)
{
    _completion_plugin_set_doc_completion (plugin, doc, cmpl);
}


MOO_PLUGIN_DEFINE_INFO (cmpl,
                        N_("Completion"), N_("Provides text completion"),
                        "Yevgen Muntyan <muntyan@tamu.edu>",
                        MOO_VERSION, NULL)
MOO_PLUGIN_DEFINE_FULL (Cmpl, cmpl,
                        cmpl_plugin_init, cmpl_plugin_deinit,
                        NULL, NULL, NULL, NULL,
                        NULL, 0, 0)


gboolean
_moo_completion_plugin_init (void)
{
    GType ptype = cmpl_plugin_get_type ();

    if (!moo_plugin_register (CMPL_PLUGIN_ID,
                              cmpl_plugin_get_type (),
                              &cmpl_plugin_info,
                              NULL))
        return FALSE;

    moo_plugin_method_new ("set-lang-completion", ptype,
                           G_CALLBACK (set_lang_completion_meth),
                           _moo_marshal_VOID__STRING_OBJECT,
                           G_TYPE_NONE, 2,
                           G_TYPE_STRING,
                           MOO_TYPE_TEXT_COMPLETION);

    moo_plugin_method_new ("set-doc-completion", ptype,
                           G_CALLBACK (set_doc_completion_meth),
                           _moo_marshal_VOID__OBJECT_OBJECT,
                           G_TYPE_NONE, 2,
                           MOO_TYPE_EDIT,
                           MOO_TYPE_TEXT_COMPLETION);

    return TRUE;
}
