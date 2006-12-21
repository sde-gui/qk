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
#include "completion-glade.h"
#include "mooedit/mooplugin-macro.h"
#include "mooedit/plugins/mooeditplugins.h"
#include "mooutils/mooi18n.h"
#include "mooutils/moomarshals.h"
#include "mooutils/mooprefsdialogpage.h"
#include <string.h>


#define AUTO_COMPLETE_PREFS_KEY CMPL_PREFS_ROOT "/auto_complete"
#define POPUP_INTERVAL_PREFS_KEY CMPL_PREFS_ROOT "/auto_complete_timeout"


static gboolean cmpl_plugin_init        (CmplPlugin     *plugin);
static void     cmpl_plugin_deinit      (CmplPlugin     *plugin);


static void
focused_doc_changed (MooEditor *editor,
                     G_GNUC_UNUSED gpointer whatever,
                     CmplPlugin *plugin)
{
    MooEdit *doc = NULL;

    g_object_get (editor, "focused-doc", &doc, NULL);
    _cmpl_plugin_set_focused_doc (plugin, doc);

    if (doc)
        g_object_unref (doc);
}

static void
set_auto_complete (CmplPlugin *plugin,
                   gboolean    auto_complete)
{
    MooEditor *editor = moo_editor_instance ();

    auto_complete = auto_complete != 0;

    if (auto_complete == plugin->auto_complete)
        return;

    plugin->auto_complete = auto_complete;

    if (auto_complete)
    {
        g_signal_connect (editor, "notify::focused-doc",
                          G_CALLBACK (focused_doc_changed),
                          plugin);
        focused_doc_changed (editor, NULL, plugin);
    }
    else
    {
        g_signal_handlers_disconnect_by_func (editor, (gpointer) focused_doc_changed, plugin);
        _cmpl_plugin_set_focused_doc (plugin, NULL);
    }
}

static void
set_popup_interval (CmplPlugin *plugin,
                    int         interval)
{
    if (interval <= 0)
    {
        g_warning ("%s: oops", G_STRLOC);
        interval = DEFAULT_POPUP_TIMEOUT;
    }

    plugin->popup_interval = interval;
}

static void
prefs_notify (const char   *key,
              const GValue *newval,
              CmplPlugin   *plugin)
{
    if (!strcmp (key, AUTO_COMPLETE_PREFS_KEY))
        set_auto_complete (plugin, g_value_get_boolean (newval));
    else if (!strcmp (key, POPUP_INTERVAL_PREFS_KEY))
        set_popup_interval (plugin, g_value_get_int (newval));
}


static void
completion_callback (MooEditWindow *window)
{
    CmplPlugin *plugin;
    MooEdit *doc;

    plugin = moo_plugin_lookup (CMPL_PLUGIN_ID);
    g_return_if_fail (plugin != NULL);

    doc = moo_edit_window_get_active_doc (window);
    g_return_if_fail (doc != NULL);

    _completion_complete (plugin, doc, FALSE);
}


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
                                 "closure-callback", completion_callback,
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

    moo_prefs_new_key_bool (AUTO_COMPLETE_PREFS_KEY, FALSE);
    moo_prefs_new_key_int (POPUP_INTERVAL_PREFS_KEY, DEFAULT_POPUP_TIMEOUT);
    plugin->prefs_notify =
        moo_prefs_notify_connect (AUTO_COMPLETE_PREFS_KEY,
                                  MOO_PREFS_MATCH_PREFIX,
                                  (MooPrefsNotify) prefs_notify,
                                  plugin, NULL);
    set_auto_complete (plugin, moo_prefs_get_bool (AUTO_COMPLETE_PREFS_KEY));
    set_popup_interval (plugin, moo_prefs_get_int (POPUP_INTERVAL_PREFS_KEY));

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

    moo_prefs_notify_disconnect (plugin->prefs_notify);
    plugin->prefs_notify = 0;
    set_auto_complete (plugin, FALSE);

    g_type_class_unref (klass);
}


static GtkWidget *
cmpl_plugin_prefs_page (G_GNUC_UNUSED CmplPlugin *plugin)
{
    MooPrefsDialogPage *page;
    MooGladeXML *xml;

    xml = moo_glade_xml_new_empty (GETTEXT_PACKAGE);
    page = moo_prefs_dialog_page_new_from_xml (_("Completion"), NULL,
                                               xml, COMPLETION_PLUGIN_GLADE_XML,
                                               "page", CMPL_PREFS_ROOT);

    g_object_unref (xml);
    return GTK_WIDGET (page);
}


static void
set_lang_completion_meth (CmplPlugin        *plugin,
                          const char        *lang,
                          MooTextCompletion *cmpl)
{
    _cmpl_plugin_set_lang_completion (plugin, lang, cmpl);
}

static void
set_doc_completion_meth (CmplPlugin        *plugin,
                         MooEdit           *doc,
                         MooTextCompletion *cmpl)
{
    _cmpl_plugin_set_doc_completion (plugin, doc, cmpl);
}


MOO_PLUGIN_DEFINE_INFO (cmpl,
                        /* Completion plugin name */
                        N_("Completion"),
                        /* Completion plugin description */
                        N_("Provides text completion"),
                        "Yevgen Muntyan <muntyan@tamu.edu>",
                        MOO_VERSION, NULL)
MOO_PLUGIN_DEFINE_FULL (Cmpl, cmpl,
                        cmpl_plugin_init, cmpl_plugin_deinit,
                        NULL, NULL, NULL, NULL,
                        cmpl_plugin_prefs_page, 0, 0)


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
