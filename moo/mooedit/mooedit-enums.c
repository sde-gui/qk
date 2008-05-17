
/* Generated data (by glib-mkenums) */

#include "mooedit-enums.h"

#include "moocommand.h"

GType
moo_command_options_get_type (void)
{
    static GType etype;
    if (G_UNLIKELY (!etype))
    {
        static const GFlagsValue values[] = {
            { MOO_COMMAND_NEED_DOC, (char*) "MOO_COMMAND_NEED_DOC", (char*) "doc" },
            { MOO_COMMAND_NEED_FILE, (char*) "MOO_COMMAND_NEED_FILE", (char*) "file" },
            { MOO_COMMAND_NEED_SAVE, (char*) "MOO_COMMAND_NEED_SAVE", (char*) "save" },
            { MOO_COMMAND_NEED_SAVE_ALL, (char*) "MOO_COMMAND_NEED_SAVE_ALL", (char*) "save-all" },
            { MOO_COMMAND_NEED_WINDOW, (char*) "MOO_COMMAND_NEED_WINDOW", (char*) "window" },
            { 0, NULL, NULL }
        };
        etype = g_flags_register_static ("MooCommandOptions", values);
    }
    return etype;
}

#include "mooeditconfig.h"

GType
moo_edit_config_source_get_type (void)
{
    static GType etype;
    if (G_UNLIKELY (!etype))
    {
        static const GEnumValue values[] = {
            { MOO_EDIT_CONFIG_SOURCE_USER, (char*) "MOO_EDIT_CONFIG_SOURCE_USER", (char*) "user" },
            { MOO_EDIT_CONFIG_SOURCE_FILE, (char*) "MOO_EDIT_CONFIG_SOURCE_FILE", (char*) "file" },
            { MOO_EDIT_CONFIG_SOURCE_FILENAME, (char*) "MOO_EDIT_CONFIG_SOURCE_FILENAME", (char*) "filename" },
            { MOO_EDIT_CONFIG_SOURCE_LANG, (char*) "MOO_EDIT_CONFIG_SOURCE_LANG", (char*) "lang" },
            { MOO_EDIT_CONFIG_SOURCE_AUTO, (char*) "MOO_EDIT_CONFIG_SOURCE_AUTO", (char*) "auto" },
            { 0, NULL, NULL }
        };
        etype = g_enum_register_static ("MooEditConfigSource", values);
    }
    return etype;
}

#include "mooeditwindow.h"

GType
moo_action_check_type_get_type (void)
{
    static GType etype;
    if (G_UNLIKELY (!etype))
    {
        static const GEnumValue values[] = {
            { MOO_ACTION_CHECK_SENSITIVE, (char*) "MOO_ACTION_CHECK_SENSITIVE", (char*) "sensitive" },
            { MOO_ACTION_CHECK_VISIBLE, (char*) "MOO_ACTION_CHECK_VISIBLE", (char*) "visible" },
            { MOO_ACTION_CHECK_ACTIVE, (char*) "MOO_ACTION_CHECK_ACTIVE", (char*) "active" },
            { 0, NULL, NULL }
        };
        etype = g_enum_register_static ("MooActionCheckType", values);
    }
    return etype;
}

#include "mooedit.h"

GType
moo_edit_status_get_type (void)
{
    static GType etype;
    if (G_UNLIKELY (!etype))
    {
        static const GFlagsValue values[] = {
            { MOO_EDIT_MODIFIED_ON_DISK, (char*) "MOO_EDIT_MODIFIED_ON_DISK", (char*) "modified-on-disk" },
            { MOO_EDIT_DELETED, (char*) "MOO_EDIT_DELETED", (char*) "deleted" },
            { MOO_EDIT_CHANGED_ON_DISK, (char*) "MOO_EDIT_CHANGED_ON_DISK", (char*) "changed-on-disk" },
            { MOO_EDIT_MODIFIED, (char*) "MOO_EDIT_MODIFIED", (char*) "modified" },
            { MOO_EDIT_NEW, (char*) "MOO_EDIT_NEW", (char*) "new" },
            { MOO_EDIT_CLEAN, (char*) "MOO_EDIT_CLEAN", (char*) "clean" },
            { 0, NULL, NULL }
        };
        etype = g_flags_register_static ("MooEditStatus", values);
    }
    return etype;
}

GType
moo_edit_state_get_type (void)
{
    static GType etype;
    if (G_UNLIKELY (!etype))
    {
        static const GEnumValue values[] = {
            { MOO_EDIT_STATE_NORMAL, (char*) "MOO_EDIT_STATE_NORMAL", (char*) "normal" },
            { MOO_EDIT_STATE_LOADING, (char*) "MOO_EDIT_STATE_LOADING", (char*) "loading" },
            { MOO_EDIT_STATE_SAVING, (char*) "MOO_EDIT_STATE_SAVING", (char*) "saving" },
            { MOO_EDIT_STATE_PRINTING, (char*) "MOO_EDIT_STATE_PRINTING", (char*) "printing" },
            { 0, NULL, NULL }
        };
        etype = g_enum_register_static ("MooEditState", values);
    }
    return etype;
}

#include "mootextfind.h"

GType
moo_find_flags_get_type (void)
{
    static GType etype;
    if (G_UNLIKELY (!etype))
    {
        static const GFlagsValue values[] = {
            { MOO_FIND_REGEX, (char*) "MOO_FIND_REGEX", (char*) "regex" },
            { MOO_FIND_CASELESS, (char*) "MOO_FIND_CASELESS", (char*) "caseless" },
            { MOO_FIND_IN_SELECTED, (char*) "MOO_FIND_IN_SELECTED", (char*) "in-selected" },
            { MOO_FIND_BACKWARDS, (char*) "MOO_FIND_BACKWARDS", (char*) "backwards" },
            { MOO_FIND_WHOLE_WORDS, (char*) "MOO_FIND_WHOLE_WORDS", (char*) "whole-words" },
            { MOO_FIND_FROM_CURSOR, (char*) "MOO_FIND_FROM_CURSOR", (char*) "from-cursor" },
            { MOO_FIND_DONT_PROMPT, (char*) "MOO_FIND_DONT_PROMPT", (char*) "dont-prompt" },
            { MOO_FIND_REPL_LITERAL, (char*) "MOO_FIND_REPL_LITERAL", (char*) "repl-literal" },
            { 0, NULL, NULL }
        };
        etype = g_flags_register_static ("MooFindFlags", values);
    }
    return etype;
}

#include "mootextsearch.h"

GType
moo_text_search_flags_get_type (void)
{
    static GType etype;
    if (G_UNLIKELY (!etype))
    {
        static const GFlagsValue values[] = {
            { MOO_TEXT_SEARCH_CASELESS, (char*) "MOO_TEXT_SEARCH_CASELESS", (char*) "caseless" },
            { MOO_TEXT_SEARCH_REGEX, (char*) "MOO_TEXT_SEARCH_REGEX", (char*) "regex" },
            { MOO_TEXT_SEARCH_WHOLE_WORDS, (char*) "MOO_TEXT_SEARCH_WHOLE_WORDS", (char*) "whole-words" },
            { MOO_TEXT_SEARCH_REPL_LITERAL, (char*) "MOO_TEXT_SEARCH_REPL_LITERAL", (char*) "repl-literal" },
            { 0, NULL, NULL }
        };
        etype = g_flags_register_static ("MooTextSearchFlags", values);
    }
    return etype;
}

#include "mootextview.h"

GType
moo_text_selection_type_get_type (void)
{
    static GType etype;
    if (G_UNLIKELY (!etype))
    {
        static const GEnumValue values[] = {
            { MOO_TEXT_SELECT_CHARS, (char*) "MOO_TEXT_SELECT_CHARS", (char*) "chars" },
            { MOO_TEXT_SELECT_WORDS, (char*) "MOO_TEXT_SELECT_WORDS", (char*) "words" },
            { MOO_TEXT_SELECT_LINES, (char*) "MOO_TEXT_SELECT_LINES", (char*) "lines" },
            { 0, NULL, NULL }
        };
        etype = g_enum_register_static ("MooTextSelectionType", values);
    }
    return etype;
}


/* Generated data ends here */

