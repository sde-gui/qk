
/* Generated data (by glib-mkenums) */

#include "mooutils-enums.h"

#include "moodialogs.h"

GType
moo_save_changes_dialog_response_get_type (void)
{
    static GType etype;
    if (G_UNLIKELY (!etype))
    {
        static const GEnumValue values[] = {
            { MOO_SAVE_CHANGES_RESPONSE_CANCEL, (char*) "MOO_SAVE_CHANGES_RESPONSE_CANCEL", (char*) "cancel" },
            { MOO_SAVE_CHANGES_RESPONSE_SAVE, (char*) "MOO_SAVE_CHANGES_RESPONSE_SAVE", (char*) "save" },
            { MOO_SAVE_CHANGES_RESPONSE_DONT_SAVE, (char*) "MOO_SAVE_CHANGES_RESPONSE_DONT_SAVE", (char*) "dont-save" },
            { 0, NULL, NULL }
        };
        etype = g_enum_register_static ("MooSaveChangesDialogResponse", values);
    }
    return etype;
}

#include "moofiledialog.h"

GType
moo_file_dialog_type_get_type (void)
{
    static GType etype;
    if (G_UNLIKELY (!etype))
    {
        static const GEnumValue values[] = {
            { MOO_FILE_DIALOG_OPEN, (char*) "MOO_FILE_DIALOG_OPEN", (char*) "open" },
            { MOO_FILE_DIALOG_OPEN_ANY, (char*) "MOO_FILE_DIALOG_OPEN_ANY", (char*) "open-any" },
            { MOO_FILE_DIALOG_SAVE, (char*) "MOO_FILE_DIALOG_SAVE", (char*) "save" },
            { MOO_FILE_DIALOG_OPEN_DIR, (char*) "MOO_FILE_DIALOG_OPEN_DIR", (char*) "open-dir" },
            { 0, NULL, NULL }
        };
        etype = g_enum_register_static ("MooFileDialogType", values);
    }
    return etype;
}

#include "moofilewatch.h"

GType
moo_file_event_code_get_type (void)
{
    static GType etype;
    if (G_UNLIKELY (!etype))
    {
        static const GEnumValue values[] = {
            { MOO_FILE_EVENT_CHANGED, (char*) "MOO_FILE_EVENT_CHANGED", (char*) "changed" },
            { MOO_FILE_EVENT_CREATED, (char*) "MOO_FILE_EVENT_CREATED", (char*) "created" },
            { MOO_FILE_EVENT_DELETED, (char*) "MOO_FILE_EVENT_DELETED", (char*) "deleted" },
            { MOO_FILE_EVENT_ERROR, (char*) "MOO_FILE_EVENT_ERROR", (char*) "error" },
            { 0, NULL, NULL }
        };
        etype = g_enum_register_static ("MooFileEventCode", values);
    }
    return etype;
}

#include "mooprefs.h"

GType
moo_prefs_kind_get_type (void)
{
    static GType etype;
    if (G_UNLIKELY (!etype))
    {
        static const GEnumValue values[] = {
            { MOO_PREFS_RC, (char*) "MOO_PREFS_RC", (char*) "rc" },
            { MOO_PREFS_STATE, (char*) "MOO_PREFS_STATE", (char*) "state" },
            { 0, NULL, NULL }
        };
        etype = g_enum_register_static ("MooPrefsKind", values);
    }
    return etype;
}

#include "moouixml.h"

GType
moo_ui_node_type_get_type (void)
{
    static GType etype;
    if (G_UNLIKELY (!etype))
    {
        static const GEnumValue values[] = {
            { MOO_UI_NODE_CONTAINER, (char*) "MOO_UI_NODE_CONTAINER", (char*) "container" },
            { MOO_UI_NODE_WIDGET, (char*) "MOO_UI_NODE_WIDGET", (char*) "widget" },
            { MOO_UI_NODE_ITEM, (char*) "MOO_UI_NODE_ITEM", (char*) "item" },
            { MOO_UI_NODE_SEPARATOR, (char*) "MOO_UI_NODE_SEPARATOR", (char*) "separator" },
            { MOO_UI_NODE_PLACEHOLDER, (char*) "MOO_UI_NODE_PLACEHOLDER", (char*) "placeholder" },
            { 0, NULL, NULL }
        };
        etype = g_enum_register_static ("MooUINodeType", values);
    }
    return etype;
}

GType
moo_ui_node_flags_get_type (void)
{
    static GType etype;
    if (G_UNLIKELY (!etype))
    {
        static const GFlagsValue values[] = {
            { MOO_UI_NODE_ENABLE_EMPTY, (char*) "MOO_UI_NODE_ENABLE_EMPTY", (char*) "empty" },
            { 0, NULL, NULL }
        };
        etype = g_flags_register_static ("MooUINodeFlags", values);
    }
    return etype;
}

GType
moo_ui_widget_type_get_type (void)
{
    static GType etype;
    if (G_UNLIKELY (!etype))
    {
        static const GEnumValue values[] = {
            { MOO_UI_MENUBAR, (char*) "MOO_UI_MENUBAR", (char*) "menubar" },
            { MOO_UI_MENU, (char*) "MOO_UI_MENU", (char*) "menu" },
            { MOO_UI_TOOLBAR, (char*) "MOO_UI_TOOLBAR", (char*) "toolbar" },
            { 0, NULL, NULL }
        };
        etype = g_enum_register_static ("MooUIWidgetType", values);
    }
    return etype;
}


/* Generated data ends here */

