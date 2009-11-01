enums = {}
flags = {}

enums['MooEditConfigSource'] = [
    [ 'MOO_EDIT_CONFIG_SOURCE_USER'    , '0' ],
    [ 'MOO_EDIT_CONFIG_SOURCE_FILE'    , '10' ],
    [ 'MOO_EDIT_CONFIG_SOURCE_FILENAME', '20' ],
    [ 'MOO_EDIT_CONFIG_SOURCE_LANG'    , '30' ],
    [ 'MOO_EDIT_CONFIG_SOURCE_AUTO'    , '40' ],
]

enums['MooActionCheckType'] = [
    [ 'MOO_ACTION_CHECK_SENSITIVE' ],
    [ 'MOO_ACTION_CHECK_VISIBLE' ],
    [ 'MOO_ACTION_CHECK_ACTIVE' ],
]

flags['MooEditStatus'] = [
    [ 'MOO_EDIT_STATUS_NORMAL',    '0' ],
    [ 'MOO_EDIT_MODIFIED_ON_DISK', '1 << 0' ],
    [ 'MOO_EDIT_DELETED',          '1 << 1' ],
    [ 'MOO_EDIT_CHANGED_ON_DISK',  'MOO_EDIT_MODIFIED_ON_DISK | MOO_EDIT_DELETED' ],
    [ 'MOO_EDIT_MODIFIED',         '1 << 2' ],
    [ 'MOO_EDIT_NEW',              '1 << 3' ],
    [ 'MOO_EDIT_CLEAN',            '1 << 4', "doesn't prompt when it's being closed, even if it's modified" ],
]

enums['MooEditState'] = [
    [ 'MOO_EDIT_STATE_NORMAL' ],
    [ 'MOO_EDIT_STATE_LOADING' ],
    [ 'MOO_EDIT_STATE_SAVING' ],
    [ 'MOO_EDIT_STATE_PRINTING' ],
]

flags['MooFindFlags'] = [
    [ 'MOO_FIND_REGEX',        '1 << 0' ],
    [ 'MOO_FIND_CASELESS',     '1 << 1' ],
    [ 'MOO_FIND_IN_SELECTED',  '1 << 2' ],
    [ 'MOO_FIND_BACKWARDS',    '1 << 3' ],
    [ 'MOO_FIND_WHOLE_WORDS',  '1 << 4' ],
    [ 'MOO_FIND_FROM_CURSOR',  '1 << 5' ],
    [ 'MOO_FIND_DONT_PROMPT',  '1 << 6' ],
    [ 'MOO_FIND_REPL_LITERAL', '1 << 7' ],
]

flags['MooTextSearchFlags'] = [
    [ 'MOO_TEXT_SEARCH_CASELESS',     '1 << 0' ],
    [ 'MOO_TEXT_SEARCH_REGEX',        '1 << 1' ],
    [ 'MOO_TEXT_SEARCH_WHOLE_WORDS',  '1 << 2' ],
    [ 'MOO_TEXT_SEARCH_REPL_LITERAL', '1 << 3' ],
]

enums['MooTextSelectionType'] = [
    [ 'MOO_TEXT_SELECT_CHARS' ],
    [ 'MOO_TEXT_SELECT_WORDS' ],
    [ 'MOO_TEXT_SELECT_LINES' ],
]
