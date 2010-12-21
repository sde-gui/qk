#ifndef MOO_EDIT_ENUMS_H
#define MOO_EDIT_ENUMS_H

#include <glib-object.h>

G_BEGIN_DECLS

typedef enum {
    MOO_ACTION_CHECK_SENSITIVE,
    MOO_ACTION_CHECK_VISIBLE,
    MOO_ACTION_CHECK_ACTIVE
} MooActionCheckType;

GType moo_action_check_type_get_type (void) G_GNUC_CONST;
#define MOO_TYPE_ACTION_CHECK_TYPE (moo_action_check_type_get_type())

typedef enum {
    MOO_TEXT_CURSOR_NONE,
    MOO_TEXT_CURSOR_TEXT,
    MOO_TEXT_CURSOR_ARROW,
    MOO_TEXT_CURSOR_LINK
} MooTextCursor;

GType moo_text_cursor_get_type (void) G_GNUC_CONST;
#define MOO_TYPE_TEXT_CURSOR (moo_text_cursor_get_type())

typedef enum {
    MOO_EDIT_CONFIG_SOURCE_USER = 0,
    MOO_EDIT_CONFIG_SOURCE_FILE = 10,
    MOO_EDIT_CONFIG_SOURCE_FILENAME = 20,
    MOO_EDIT_CONFIG_SOURCE_LANG = 30,
    MOO_EDIT_CONFIG_SOURCE_AUTO = 40
} MooEditConfigSource;

GType moo_edit_config_source_get_type (void) G_GNUC_CONST;
#define MOO_TYPE_EDIT_CONFIG_SOURCE (moo_edit_config_source_get_type())

typedef enum {
    MOO_EDIT_SAVE_RESPONSE_CONTINUE = 2,
    MOO_EDIT_SAVE_RESPONSE_CANCEL = 3
} MooEditSaveResponse;

GType moo_edit_save_response_get_type (void) G_GNUC_CONST;
#define MOO_TYPE_EDIT_SAVE_RESPONSE (moo_edit_save_response_get_type())

typedef enum {
    MOO_TEXT_SELECT_CHARS,
    MOO_TEXT_SELECT_WORDS,
    MOO_TEXT_SELECT_LINES
} MooTextSelectionType;

GType moo_text_selection_type_get_type (void) G_GNUC_CONST;
#define MOO_TYPE_TEXT_SELECTION_TYPE (moo_text_selection_type_get_type())

typedef enum {
    MOO_EDIT_STATE_NORMAL,
    MOO_EDIT_STATE_LOADING,
    MOO_EDIT_STATE_SAVING,
    MOO_EDIT_STATE_PRINTING
} MooEditState;

GType moo_edit_state_get_type (void) G_GNUC_CONST;
#define MOO_TYPE_EDIT_STATE (moo_edit_state_get_type())

typedef enum {
    MOO_LE_NONE,
    MOO_LE_UNIX,
    MOO_LE_WIN32,
    MOO_LE_MAC,
    MOO_LE_MIX
} MooLineEndType;

GType moo_line_end_type_get_type (void) G_GNUC_CONST;
#define MOO_TYPE_LINE_END_TYPE (moo_line_end_type_get_type())

typedef enum {
    MOO_TEXT_SEARCH_CASELESS = 1 << 0,
    MOO_TEXT_SEARCH_REGEX = 1 << 1,
    MOO_TEXT_SEARCH_WHOLE_WORDS = 1 << 2,
    MOO_TEXT_SEARCH_REPL_LITERAL = 1 << 3
} MooTextSearchFlags;

GType moo_text_search_flags_get_type (void) G_GNUC_CONST;
#define MOO_TYPE_TEXT_SEARCH_FLAGS (moo_text_search_flags_get_type())

typedef enum {
    MOO_FIND_REGEX = 1 << 0,
    MOO_FIND_CASELESS = 1 << 1,
    MOO_FIND_IN_SELECTED = 1 << 2,
    MOO_FIND_BACKWARDS = 1 << 3,
    MOO_FIND_WHOLE_WORDS = 1 << 4,
    MOO_FIND_FROM_CURSOR = 1 << 5,
    MOO_FIND_DONT_PROMPT = 1 << 6,
    MOO_FIND_REPL_LITERAL = 1 << 7
} MooFindFlags;

GType moo_find_flags_get_type (void) G_GNUC_CONST;
#define MOO_TYPE_FIND_FLAGS (moo_find_flags_get_type())

typedef enum {
    MOO_EDIT_STATUS_NORMAL = 0,
    MOO_EDIT_MODIFIED_ON_DISK = 1 << 0,
    MOO_EDIT_DELETED = 1 << 1,
    MOO_EDIT_CHANGED_ON_DISK = MOO_EDIT_MODIFIED_ON_DISK | MOO_EDIT_DELETED,
    MOO_EDIT_MODIFIED = 1 << 2,
    MOO_EDIT_NEW = 1 << 3,
    MOO_EDIT_CLEAN = 1 << 4 /* doesn't prompt when it's being closed, even if it's modified */
} MooEditStatus;

GType moo_edit_status_get_type (void) G_GNUC_CONST;
#define MOO_TYPE_EDIT_STATUS (moo_edit_status_get_type())

typedef enum {
    MOO_DRAW_NO_WHITESPACE = 0,
    MOO_DRAW_SPACES = 1 << 0,
    MOO_DRAW_TABS = 1 << 1,
    MOO_DRAW_TRAILING_SPACES = 1 << 2
} MooDrawWhitespaceFlags;

GType moo_draw_whitespace_flags_get_type (void) G_GNUC_CONST;
#define MOO_TYPE_DRAW_WHITESPACE_FLAGS (moo_draw_whitespace_flags_get_type())


G_END_DECLS

#endif /* MOO_EDIT_ENUMS_H */
