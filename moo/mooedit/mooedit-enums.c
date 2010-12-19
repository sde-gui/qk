#include "../../../moo/mooedit/mooedit-enums.h"

GType
moo_action_check_type_get_type (void)
{
    static GType etype;
    if (G_UNLIKELY (!etype))
    {
        static const GEnumValue values[] = {
            { MOO_ACTION_CHECK_SENSITIVE, (char*) "MOO_ACTION_CHECK_SENSITIVE", (char*) "MOO_ACTION_CHECK_SENSITIVE" },
            { MOO_ACTION_CHECK_VISIBLE, (char*) "MOO_ACTION_CHECK_VISIBLE", (char*) "MOO_ACTION_CHECK_VISIBLE" },
            { MOO_ACTION_CHECK_ACTIVE, (char*) "MOO_ACTION_CHECK_ACTIVE", (char*) "MOO_ACTION_CHECK_ACTIVE" },
            { 0, NULL, NULL }
        };
        etype = g_enum_register_static ("MooActionCheckType", values);
    }
    return etype;
}

GType
moo_text_cursor_get_type (void)
{
    static GType etype;
    if (G_UNLIKELY (!etype))
    {
        static const GEnumValue values[] = {
            { MOO_TEXT_CURSOR_NONE, (char*) "MOO_TEXT_CURSOR_NONE", (char*) "MOO_TEXT_CURSOR_NONE" },
            { MOO_TEXT_CURSOR_TEXT, (char*) "MOO_TEXT_CURSOR_TEXT", (char*) "MOO_TEXT_CURSOR_TEXT" },
            { MOO_TEXT_CURSOR_ARROW, (char*) "MOO_TEXT_CURSOR_ARROW", (char*) "MOO_TEXT_CURSOR_ARROW" },
            { MOO_TEXT_CURSOR_LINK, (char*) "MOO_TEXT_CURSOR_LINK", (char*) "MOO_TEXT_CURSOR_LINK" },
            { 0, NULL, NULL }
        };
        etype = g_enum_register_static ("MooTextCursor", values);
    }
    return etype;
}

GType
moo_edit_config_source_get_type (void)
{
    static GType etype;
    if (G_UNLIKELY (!etype))
    {
        static const GEnumValue values[] = {
            { MOO_EDIT_CONFIG_SOURCE_USER, (char*) "MOO_EDIT_CONFIG_SOURCE_USER", (char*) "MOO_EDIT_CONFIG_SOURCE_USER" },
            { MOO_EDIT_CONFIG_SOURCE_FILE, (char*) "MOO_EDIT_CONFIG_SOURCE_FILE", (char*) "MOO_EDIT_CONFIG_SOURCE_FILE" },
            { MOO_EDIT_CONFIG_SOURCE_FILENAME, (char*) "MOO_EDIT_CONFIG_SOURCE_FILENAME", (char*) "MOO_EDIT_CONFIG_SOURCE_FILENAME" },
            { MOO_EDIT_CONFIG_SOURCE_LANG, (char*) "MOO_EDIT_CONFIG_SOURCE_LANG", (char*) "MOO_EDIT_CONFIG_SOURCE_LANG" },
            { MOO_EDIT_CONFIG_SOURCE_AUTO, (char*) "MOO_EDIT_CONFIG_SOURCE_AUTO", (char*) "MOO_EDIT_CONFIG_SOURCE_AUTO" },
            { 0, NULL, NULL }
        };
        etype = g_enum_register_static ("MooEditConfigSource", values);
    }
    return etype;
}

GType
moo_edit_save_response_get_type (void)
{
    static GType etype;
    if (G_UNLIKELY (!etype))
    {
        static const GEnumValue values[] = {
            { MOO_EDIT_SAVE_RESPONSE_CONTINUE, (char*) "MOO_EDIT_SAVE_RESPONSE_CONTINUE", (char*) "MOO_EDIT_SAVE_RESPONSE_CONTINUE" },
            { MOO_EDIT_SAVE_RESPONSE_CANCEL, (char*) "MOO_EDIT_SAVE_RESPONSE_CANCEL", (char*) "MOO_EDIT_SAVE_RESPONSE_CANCEL" },
            { 0, NULL, NULL }
        };
        etype = g_enum_register_static ("MooEditSaveResponse", values);
    }
    return etype;
}

GType
moo_text_selection_type_get_type (void)
{
    static GType etype;
    if (G_UNLIKELY (!etype))
    {
        static const GEnumValue values[] = {
            { MOO_TEXT_SELECT_CHARS, (char*) "MOO_TEXT_SELECT_CHARS", (char*) "MOO_TEXT_SELECT_CHARS" },
            { MOO_TEXT_SELECT_WORDS, (char*) "MOO_TEXT_SELECT_WORDS", (char*) "MOO_TEXT_SELECT_WORDS" },
            { MOO_TEXT_SELECT_LINES, (char*) "MOO_TEXT_SELECT_LINES", (char*) "MOO_TEXT_SELECT_LINES" },
            { 0, NULL, NULL }
        };
        etype = g_enum_register_static ("MooTextSelectionType", values);
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
            { MOO_EDIT_STATE_NORMAL, (char*) "MOO_EDIT_STATE_NORMAL", (char*) "MOO_EDIT_STATE_NORMAL" },
            { MOO_EDIT_STATE_LOADING, (char*) "MOO_EDIT_STATE_LOADING", (char*) "MOO_EDIT_STATE_LOADING" },
            { MOO_EDIT_STATE_SAVING, (char*) "MOO_EDIT_STATE_SAVING", (char*) "MOO_EDIT_STATE_SAVING" },
            { MOO_EDIT_STATE_PRINTING, (char*) "MOO_EDIT_STATE_PRINTING", (char*) "MOO_EDIT_STATE_PRINTING" },
            { 0, NULL, NULL }
        };
        etype = g_enum_register_static ("MooEditState", values);
    }
    return etype;
}

GType
moo_line_end_type_get_type (void)
{
    static GType etype;
    if (G_UNLIKELY (!etype))
    {
        static const GEnumValue values[] = {
            { MOO_LE_NONE, (char*) "MOO_LE_NONE", (char*) "MOO_LE_NONE" },
            { MOO_LE_UNIX, (char*) "MOO_LE_UNIX", (char*) "MOO_LE_UNIX" },
            { MOO_LE_WIN32, (char*) "MOO_LE_WIN32", (char*) "MOO_LE_WIN32" },
            { MOO_LE_MAC, (char*) "MOO_LE_MAC", (char*) "MOO_LE_MAC" },
            { MOO_LE_MIX, (char*) "MOO_LE_MIX", (char*) "MOO_LE_MIX" },
            { 0, NULL, NULL }
        };
        etype = g_enum_register_static ("MooLineEndType", values);
    }
    return etype;
}

GType
moo_text_search_flags_get_type (void)
{
    static GType etype;
    if (G_UNLIKELY (!etype))
    {
        static const GFlagsValue values[] = {
            { MOO_TEXT_SEARCH_CASELESS, (char*) "MOO_TEXT_SEARCH_CASELESS", (char*) "MOO_TEXT_SEARCH_CASELESS" },
            { MOO_TEXT_SEARCH_REGEX, (char*) "MOO_TEXT_SEARCH_REGEX", (char*) "MOO_TEXT_SEARCH_REGEX" },
            { MOO_TEXT_SEARCH_WHOLE_WORDS, (char*) "MOO_TEXT_SEARCH_WHOLE_WORDS", (char*) "MOO_TEXT_SEARCH_WHOLE_WORDS" },
            { MOO_TEXT_SEARCH_REPL_LITERAL, (char*) "MOO_TEXT_SEARCH_REPL_LITERAL", (char*) "MOO_TEXT_SEARCH_REPL_LITERAL" },
            { 0, NULL, NULL }
        };
        etype = g_flags_register_static ("MooTextSearchFlags", values);
    }
    return etype;
}

GType
moo_find_flags_get_type (void)
{
    static GType etype;
    if (G_UNLIKELY (!etype))
    {
        static const GFlagsValue values[] = {
            { MOO_FIND_REGEX, (char*) "MOO_FIND_REGEX", (char*) "MOO_FIND_REGEX" },
            { MOO_FIND_CASELESS, (char*) "MOO_FIND_CASELESS", (char*) "MOO_FIND_CASELESS" },
            { MOO_FIND_IN_SELECTED, (char*) "MOO_FIND_IN_SELECTED", (char*) "MOO_FIND_IN_SELECTED" },
            { MOO_FIND_BACKWARDS, (char*) "MOO_FIND_BACKWARDS", (char*) "MOO_FIND_BACKWARDS" },
            { MOO_FIND_WHOLE_WORDS, (char*) "MOO_FIND_WHOLE_WORDS", (char*) "MOO_FIND_WHOLE_WORDS" },
            { MOO_FIND_FROM_CURSOR, (char*) "MOO_FIND_FROM_CURSOR", (char*) "MOO_FIND_FROM_CURSOR" },
            { MOO_FIND_DONT_PROMPT, (char*) "MOO_FIND_DONT_PROMPT", (char*) "MOO_FIND_DONT_PROMPT" },
            { MOO_FIND_REPL_LITERAL, (char*) "MOO_FIND_REPL_LITERAL", (char*) "MOO_FIND_REPL_LITERAL" },
            { 0, NULL, NULL }
        };
        etype = g_flags_register_static ("MooFindFlags", values);
    }
    return etype;
}

GType
moo_edit_status_get_type (void)
{
    static GType etype;
    if (G_UNLIKELY (!etype))
    {
        static const GFlagsValue values[] = {
            { MOO_EDIT_STATUS_NORMAL, (char*) "MOO_EDIT_STATUS_NORMAL", (char*) "MOO_EDIT_STATUS_NORMAL" },
            { MOO_EDIT_MODIFIED_ON_DISK, (char*) "MOO_EDIT_MODIFIED_ON_DISK", (char*) "MOO_EDIT_MODIFIED_ON_DISK" },
            { MOO_EDIT_DELETED, (char*) "MOO_EDIT_DELETED", (char*) "MOO_EDIT_DELETED" },
            { MOO_EDIT_CHANGED_ON_DISK, (char*) "MOO_EDIT_CHANGED_ON_DISK", (char*) "MOO_EDIT_CHANGED_ON_DISK" },
            { MOO_EDIT_MODIFIED, (char*) "MOO_EDIT_MODIFIED", (char*) "MOO_EDIT_MODIFIED" },
            { MOO_EDIT_NEW, (char*) "MOO_EDIT_NEW", (char*) "MOO_EDIT_NEW" },
            { MOO_EDIT_CLEAN, (char*) "MOO_EDIT_CLEAN", (char*) "MOO_EDIT_CLEAN" },
            { 0, NULL, NULL }
        };
        etype = g_flags_register_static ("MooEditStatus", values);
    }
    return etype;
}

GType
moo_draw_whitespace_flags_get_type (void)
{
    static GType etype;
    if (G_UNLIKELY (!etype))
    {
        static const GFlagsValue values[] = {
            { MOO_DRAW_NO_WHITESPACE, (char*) "MOO_DRAW_NO_WHITESPACE", (char*) "MOO_DRAW_NO_WHITESPACE" },
            { MOO_DRAW_SPACES, (char*) "MOO_DRAW_SPACES", (char*) "MOO_DRAW_SPACES" },
            { MOO_DRAW_TABS, (char*) "MOO_DRAW_TABS", (char*) "MOO_DRAW_TABS" },
            { MOO_DRAW_TRAILING_SPACES, (char*) "MOO_DRAW_TRAILING_SPACES", (char*) "MOO_DRAW_TRAILING_SPACES" },
            { 0, NULL, NULL }
        };
        etype = g_flags_register_static ("MooDrawWhitespaceFlags", values);
    }
    return etype;
}


