#ifndef MOO_EDIT_ENUMS_H
#define MOO_EDIT_ENUMS_H

typedef enum {
    MOO_EDIT_CONFIG_SOURCE_USER = 0,
    MOO_EDIT_CONFIG_SOURCE_FILE = 10,
    MOO_EDIT_CONFIG_SOURCE_FILENAME = 20,
    MOO_EDIT_CONFIG_SOURCE_LANG = 30,
    MOO_EDIT_CONFIG_SOURCE_AUTO = 40
} MooEditConfigSource;

typedef enum {
    MOO_SAVE_RESPONSE_CONTINUE = 2,
    MOO_SAVE_RESPONSE_CANCEL = 3
} MooSaveResponse;

typedef enum {
    MOO_EDIT_STATE_NORMAL,
    MOO_EDIT_STATE_LOADING,
    MOO_EDIT_STATE_SAVING,
    MOO_EDIT_STATE_PRINTING
} MooEditState;

typedef enum {
    MOO_EDIT_STATUS_NORMAL           = 0,
    MOO_EDIT_STATUS_MODIFIED_ON_DISK = 1 << 0,
    MOO_EDIT_STATUS_DELETED          = 1 << 1,
    MOO_EDIT_STATUS_CHANGED_ON_DISK  = MOO_EDIT_STATUS_MODIFIED_ON_DISK | MOO_EDIT_STATUS_DELETED,
    MOO_EDIT_STATUS_MODIFIED         = 1 << 2,
    MOO_EDIT_STATUS_NEW              = 1 << 3,
    MOO_EDIT_STATUS_CLEAN            = 1 << 4 /* doesn't prompt when it's being closed, even if it's modified */
} MooEditStatus;

typedef enum {
    MOO_LE_NONE,
    MOO_LE_UNIX,
    MOO_LE_WIN32,
    MOO_LE_MAC,
    MOO_LE_MIX
} MooLineEndType;

typedef enum {
    MOO_TEXT_SELECT_CHARS,
    MOO_TEXT_SELECT_WORDS,
    MOO_TEXT_SELECT_LINES
} MooTextSelectionType;

typedef enum {
    MOO_TEXT_SEARCH_CASELESS = 1 << 0,
    MOO_TEXT_SEARCH_REGEX = 1 << 1,
    MOO_TEXT_SEARCH_WHOLE_WORDS = 1 << 2,
    MOO_TEXT_SEARCH_REPL_LITERAL = 1 << 3
} MooTextSearchFlags;

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

typedef enum {
    MOO_DRAW_WS_NONE     = 0,
    MOO_DRAW_WS_SPACES   = 1 << 0,
    MOO_DRAW_WS_TABS     = 1 << 1,
    MOO_DRAW_WS_TRAILING = 1 << 2
} MooDrawWsFlags;

typedef enum {
    MOO_ACTION_CHECK_SENSITIVE,
    MOO_ACTION_CHECK_VISIBLE,
    MOO_ACTION_CHECK_ACTIVE
} MooActionCheckType;

typedef enum {
    MOO_TEXT_CURSOR_NONE,
    MOO_TEXT_CURSOR_TEXT,
    MOO_TEXT_CURSOR_ARROW,
    MOO_TEXT_CURSOR_LINK
} MooTextCursor;

#endif /* MOO_EDIT_ENUMS_H */
