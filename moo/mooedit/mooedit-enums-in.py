enums = {}
flags = {}

enums['MooEditConfigSource'] = [
    [ 'MOO_EDIT_CONFIG_SOURCE_USER'    , '0' , None, 'user' ],
    [ 'MOO_EDIT_CONFIG_SOURCE_FILE'    , '10', None, 'file'],
    [ 'MOO_EDIT_CONFIG_SOURCE_FILENAME', '20', None, 'filename' ],
    [ 'MOO_EDIT_CONFIG_SOURCE_LANG'    , '30', None, 'lang' ],
    [ 'MOO_EDIT_CONFIG_SOURCE_AUTO'    , '40', None, 'auto' ],
]

enums['MooActionCheckType'] = [
    [ 'MOO_ACTION_CHECK_SENSITIVE', None, None, 'check-sensitive' ],
    [ 'MOO_ACTION_CHECK_VISIBLE'  , None, None, 'check-visible' ],
    [ 'MOO_ACTION_CHECK_ACTIVE'   , None, None, 'check-active' ],
]

flags['MooEditStatus'] = [
    [ 'MOO_EDIT_STATUS_NORMAL',    '0', None, 'normal' ],
    [ 'MOO_EDIT_MODIFIED_ON_DISK', '1 << 0', None, 'modified-on-disk' ],
    [ 'MOO_EDIT_DELETED',          '1 << 1', None, 'delteted' ],
    [ 'MOO_EDIT_CHANGED_ON_DISK',  'MOO_EDIT_MODIFIED_ON_DISK | MOO_EDIT_DELETED', None, 'changed-on-disk' ],
    [ 'MOO_EDIT_MODIFIED',         '1 << 2', None, 'modified' ],
    [ 'MOO_EDIT_NEW',              '1 << 3', None, 'new' ],
    [ 'MOO_EDIT_CLEAN',            '1 << 4', "doesn't prompt when it's being closed, even if it's modified", 'clean' ],
]

enums['MooEditState'] = [
    [ 'MOO_EDIT_STATE_NORMAL', None, None, 'normal' ],
    [ 'MOO_EDIT_STATE_LOADING', None, None, 'loading' ],
    [ 'MOO_EDIT_STATE_SAVING', None, None, 'saving' ],
    [ 'MOO_EDIT_STATE_PRINTING', None, None, 'printing' ],
]

enums['MooEditSaveResponse'] = [
    [ 'MOO_EDIT_SAVE_RESPONSE_CONTINUE', '2', None, 'continue' ],
    [ 'MOO_EDIT_SAVE_RESPONSE_CANCEL',   '3', None, 'cancel' ],
]

# Keep in sync with line_end_menu_items in mooeditwindow.c
enums['MooLineEndType'] = [
    [ 'MOO_LE_NONE', None, None, 'none' ],
    [ 'MOO_LE_UNIX', None, None, 'unix' ],
    [ 'MOO_LE_WIN32', None, None, 'win32' ],
    [ 'MOO_LE_MAC', None, None, 'mac' ],
    [ 'MOO_LE_MIX', None, None, 'mix' ],
]

flags['MooFindFlags'] = [
    [ 'MOO_FIND_REGEX',        '1 << 0', None, 'regex' ],
    [ 'MOO_FIND_CASELESS',     '1 << 1', None, 'caseless' ],
    [ 'MOO_FIND_IN_SELECTED',  '1 << 2', None, 'in-selected' ],
    [ 'MOO_FIND_BACKWARDS',    '1 << 3', None, 'backwards' ],
    [ 'MOO_FIND_WHOLE_WORDS',  '1 << 4', None, 'whole-words' ],
    [ 'MOO_FIND_FROM_CURSOR',  '1 << 5', None, 'from-cursor' ],
    [ 'MOO_FIND_DONT_PROMPT',  '1 << 6', None, 'dont-prompt' ],
    [ 'MOO_FIND_REPL_LITERAL', '1 << 7', None, 'repl-literal' ],
]

flags['MooTextSearchFlags'] = [
    [ 'MOO_TEXT_SEARCH_CASELESS',     '1 << 0', None, 'caseless' ],
    [ 'MOO_TEXT_SEARCH_REGEX',        '1 << 1', None, 'regex' ],
    [ 'MOO_TEXT_SEARCH_WHOLE_WORDS',  '1 << 2', None, 'whole-words' ],
    [ 'MOO_TEXT_SEARCH_REPL_LITERAL', '1 << 3', None, 'repl-literal' ],
]

enums['MooTextSelectionType'] = [
    [ 'MOO_TEXT_SELECT_CHARS', None, None, 'chars' ],
    [ 'MOO_TEXT_SELECT_WORDS', None, None, 'words' ],
    [ 'MOO_TEXT_SELECT_LINES', None, None, 'lines' ],
]

enums['MooTextCursor'] = [
    [ 'MOO_TEXT_CURSOR_NONE', None, None, 'none' ],
    [ 'MOO_TEXT_CURSOR_TEXT', None, None, 'text' ],
    [ 'MOO_TEXT_CURSOR_ARROW', None, None, 'arrow' ],
    [ 'MOO_TEXT_CURSOR_LINK', None, None, 'link' ],
]

flags['MooDrawWhitespaceFlags'] = [
    [ 'MOO_DRAW_NO_WHITESPACE',   '0',      None, None, 'none' ],
    [ 'MOO_DRAW_SPACES',          '1 << 0', None, None, 'spaces' ],
    [ 'MOO_DRAW_TABS',            '1 << 1', None, None, 'tabs' ],
    [ 'MOO_DRAW_TRAILING_SPACES', '1 << 2', None, None, 'trailing-spaces' ],
]
