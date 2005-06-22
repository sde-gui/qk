/*
 *   mooterm/mootermparser.c
 *
 *   Copyright (C) 2004-2005 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   See COPYING file that comes with this distribution.
 */

#define MOOTERM_COMPILATION
#include "mooterm/mootermbuffer-private.h"
#include <string.h>

typedef enum {
    SET_ATTRS,
    BACK_TAB,
    CURSOR_NORMAL,
    CURSOR_INVISIBLE,
    CLEAR_SCREEN,
    CHANGE_SCROLL_REGION,

    PARM_LEFT_CURSOR,
    PARM_RIGHT_CURSOR,
    PARM_DOWN_CURSOR,
    PARM_UP_CURSOR,
    CURSOR_RIGHT,
    CURSOR_LEFT,
    CURSOR_DOWN,
    CURSOR_UP,
    CURSOR_HOME,
    CURSOR_ADDRESS,
    ROW_ADDRESS,
    COLUMN_ADDRESS,
    CARRIAGE_RETURN,

    PARM_DCH,
    PARM_DELETE_LINE,
    DELETE_CHARACTER,
    DELETE_LINE,
    ERASE_CHARS,

    CLR_EOS,
    CLR_EOL,
    CLR_BOL,

    ENA_ACS,
    FLASH_SCREEN,

    PARM_ICH,
    PARM_INSERT_LINE,
    INSERT_LINE,

    ENTER_SECURE_MODE,
    INIT_2STRING,
    RESET_2STRING,
    RESET_1STRING,
    PRINT_SCREEN,
    PRTR_OFF,
    PRTR_ON,

    ORIG_PAIR,
    ENTER_REVERSE_MODE,
    ENTER_AM_MODE,
    EXIT_AM_MODE,
    ENTER_CA_MODE,
    EXIT_CA_MODE,
    ENTER_INSERT_MODE,
    EXIT_INSERT_MODE,
    ENTER_STANDOUT_MODE,
    EXIT_STANDOUT_MODE,
    ENTER_UNDERLINE_MODE,
    EXIT_UNDERLINE_MODE,

    KEYPAD_LOCAL,
    KEYPAD_XMIT,

    SET_BACKGROUND,
    SET_FOREGROUND,

    CLEAR_ALL_TABS,

    USER6,
    USER7,
    USER8,
    USER9,

    BELL,
    ENTER_ALT_CHARSET_MODE,
    EXIT_ALT_CHARSET_MODE,

    TAB,
    SCROLL_FORWARD,
    SCROLL_REVERSE,
    SET_TAB,
    ESC_l,
    ESC_m,

    RESTORE_CURSOR,
    SAVE_CURSOR
} CommandCode;

#ifdef DEBUG
#define DEBUG_PRINT(msg, str, len)                      \
{                                                       \
    char *piece = g_strndup (str, len);                 \
    g_warning ("%s: " msg ", got %s", G_STRLOC, piece); \
    g_free (piece);                                     \
}
#else
#define DEBUG_PRINT(msg, str, len)
#endif

#define MAX_ESC_SEQ_LEN 1024
#define MAX_ARG_LEN     4
#define MAX_ARG_NUM     64

#define CTR_G   '\007'
#define CTR_H   '\010'
#define CTR_I   '\011'
#define CTR_J   '\012'
#define CTR_M   '\015'
#define CTR_N   '\016'
#define CTR_O   '\017'
#define ESCAPE  '\033'

#define ENA_ACS_STRING          "\033(B\033)0"
#define ENA_ACS_STRING_LEN      6
#define INIT_2STRING_STRING     "\033[!p\033[?3;4l\033[4l\033>"
#define INIT_2STRING_STRING_LEN (strlen (INIT_2STRING_STRING))
#define CLEAR_SCREEN_STRING     "\033[H\033[2J"
#define CLEAR_SCREEN_STRING_LEN (strlen (CLEAR_SCREEN_STRING))
#define FLASH_SCREEN_STRING     "\033[?5h\033[?5l"
#define FLASH_SCREEN_STRING_LEN (strlen (FLASH_SCREEN_STRING))
#define KEYPAD_LOCAL_STRING     "\033[?1l\033>"
#define KEYPAD_LOCAL_STRING_LEN (strlen (KEYPAD_LOCAL_STRING))
#define USER8_STRING            "\033[?1;2c"
#define USER8_STRING_LEN        (strlen (USER8_STRING))

typedef enum {
    CLEAN = 0,
    ESC,
    ESC_BR,
    ESC_BR_NUMS,
    ESC_BR_H,
    ESC_BR_QUEST,
    ESC_BR_QUEST_2,
    ESC_BR_QUEST_25,
    ESC_BR_QUEST_7,
    ESC_BR_QUEST_1,
    ESC_BR_QUEST_10,
    ESC_BR_QUEST_104,
    ESC_BR_QUEST_1049,
    EXPECT
} ParserState;

typedef enum {
    WAITING = 0,
    ONE_CHAR    = 1 << 1,
    CHARS       = 1 << 2,
    COMMAND     = 1 << 3,
    REPEAT      = 1 << 4
} ParseCharResult;

struct _MooTermParser {
    MooTermBuffer  *parent;

    ParserState     state;
    CommandCode     command;
    guint           repeat;

    const char     *buffer_start;
    char            buffer[MAX_ESC_SEQ_LEN];
    guint           buffer_len;
    const char     *expect;
    guint           expect_len;
    CommandCode     expect_cmd;
    guint           nums[MAX_ARG_NUM];
    guint           nums_len;
    char           *this_num;
    guint           this_num_len;
};


static ParseCharResult parse_nums_add_digit (MooTermParser *p, char c);
static ParseCharResult parse_nums_semicolon (MooTermParser *p, char c);
static ParseCharResult parse_nums_finish_m  (MooTermParser *p, char c);
static ParseCharResult parse_nums_finish_2  (MooTermParser *p, char c);
static ParseCharResult parse_nums_finish_1  (MooTermParser *p, char c);
static ParseCharResult parse_nums_invalid   (MooTermParser *p, char c);

static ParseCharResult parse_char (MooTermParser *p, const char *c)
{
    if (!p->state)
    {
        switch (*c)
        {
            case CTR_G:
                p->command = BELL;
                return COMMAND;
            case CTR_N:
                p->command = ENTER_ALT_CHARSET_MODE;
                return COMMAND;
            case CTR_O:
                p->command = EXIT_ALT_CHARSET_MODE;
                return COMMAND;
            case CTR_M:
                p->command = CARRIAGE_RETURN;
                return COMMAND;
            case CTR_H:
                p->command = CURSOR_LEFT;
                return COMMAND;
            case CTR_J:
                p->command = CURSOR_DOWN;
                return COMMAND;
            case CTR_I:
                p->command = TAB;
                return COMMAND;

            case ESCAPE:
                p->state = ESC;
                p->buffer_start = c;
                p->buffer[0] = ESCAPE;
                p->buffer_len = 1;
                return WAITING;

            default:
                return ONE_CHAR;
        }
    }

    else if (p->state == ESC)
    {
        g_assert (p->buffer_len == 1 && p->buffer[0] == ESCAPE);

        switch (*c)
        {
            case 'H':
                p->command = SET_TAB;
                p->state = CLEAN;
                p->buffer_len = 0;
                p->buffer_start = NULL;
                return COMMAND;
            case 'l':
                p->command = ESC_l;
                p->state = CLEAN;
                p->buffer_len = 0;
                p->buffer_start = NULL;
                return COMMAND;
            case 'm':
                p->command = ESC_m;
                p->state = CLEAN;
                p->buffer_len = 0;
                p->buffer_start = NULL;
                return COMMAND;
            case '8':
                p->command = RESTORE_CURSOR;
                p->state = CLEAN;
                p->buffer_len = 0;
                p->buffer_start = NULL;
                return COMMAND;
            case '7':
                p->command = SAVE_CURSOR;
                p->state = CLEAN;
                p->buffer_len = 0;
                p->buffer_start = NULL;
                return COMMAND;
            case 'M':
                p->command = SCROLL_REVERSE;
                p->state = CLEAN;
                p->buffer_len = 0;
                p->buffer_start = NULL;
                return COMMAND;
            case 'c':
                p->command = RESET_1STRING;
                p->state = CLEAN;
                p->buffer_len = 0;
                p->buffer_start = NULL;
                return COMMAND;

            case '(':
                p->state = EXPECT;
                p->buffer[p->buffer_len++] = *c;
                p->expect_cmd = ENA_ACS;
                p->expect = ENA_ACS_STRING;
                p->expect_len = ENA_ACS_STRING_LEN;
                return WAITING;

            case '[':
                p->state = ESC_BR;
                p->buffer[p->buffer_len++] = *c;
                return WAITING;

            default:
                p->state = CLEAN;
                p->buffer[p->buffer_len++] = *c;
                DEBUG_PRINT ("invalid escape sequence",
                             p->buffer, p->buffer_len);
                return CHARS;
        }
    }

    else if (p->state == ESC_BR)
    {
        g_assert (p->buffer_len == 2 && !strncmp (p->buffer, "\033[", 2));

        switch (*c)
        {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                p->state = ESC_BR_NUMS;
                p->buffer[p->buffer_len++] = *c;
                p->nums_len = 1;
                p->nums[0] = *c - '0';
                p->this_num = p->buffer + 2;
                p->this_num_len = 1;
                return WAITING;

            case 'Z':
                p->command = BACK_TAB;
                p->state = CLEAN;
                p->buffer_len = 0;
                p->buffer_start = NULL;
                return COMMAND;
            case 'C':
                p->command = CURSOR_RIGHT;
                p->state = CLEAN;
                p->buffer_len = 0;
                p->buffer_start = NULL;
                return COMMAND;
            case 'A':
                p->command = CURSOR_UP;
                p->state = CLEAN;
                p->buffer_len = 0;
                p->buffer_start = NULL;
                return COMMAND;
            case 'P':
                p->command = DELETE_CHARACTER;
                p->state = CLEAN;
                p->buffer_len = 0;
                p->buffer_start = NULL;
                return COMMAND;
            case 'M':
                p->command = DELETE_LINE;
                p->state = CLEAN;
                p->buffer_len = 0;
                p->buffer_start = NULL;
                return COMMAND;
            case 'J':
                p->command = CLR_EOS;
                p->state = CLEAN;
                p->buffer_len = 0;
                p->buffer_start = NULL;
                return COMMAND;
            case 'K':
                p->command = CLR_EOL;
                p->state = CLEAN;
                p->buffer_len = 0;
                p->buffer_start = NULL;
                return COMMAND;
            case 'L':
                p->command = INSERT_LINE;
                p->state = CLEAN;
                p->buffer_len = 0;
                p->buffer_start = NULL;
                return COMMAND;
            case 'i':
                p->command = PRINT_SCREEN;
                p->state = CLEAN;
                p->buffer_len = 0;
                p->buffer_start = NULL;
                return COMMAND;
            case 'c':
                p->command = USER9;
                p->state = CLEAN;
                p->buffer_len = 0;
                p->buffer_start = NULL;
                return COMMAND;

            case '?':
                p->state = ESC_BR_QUEST;
                p->buffer[p->buffer_len++] = *c;
                return WAITING;

            case 'H':
                p->state = ESC_BR_H;
                p->buffer[p->buffer_len++] = *c;
                return WAITING;

            case '!':
                p->state = EXPECT;
                p->buffer[p->buffer_len++] = *c;
                p->expect = INIT_2STRING_STRING;
                p->expect_len = INIT_2STRING_STRING_LEN;
                p->expect_cmd = INIT_2STRING;
                return WAITING;

            default:
                p->state = CLEAN;
                p->buffer[p->buffer_len++] = *c;
                DEBUG_PRINT ("invalid escape sequence",
                             p->buffer, p->buffer_len);
                return CHARS;
        }
    }

    else if (p->state == ESC_BR_H)
    {
        g_assert (p->buffer_len == 3 && !strncmp (p->buffer, "\033[H", 3));

        if (*c == ESCAPE)
        {
            p->state = EXPECT;
            p->buffer[p->buffer_len++] = *c;
            p->expect = CLEAR_SCREEN_STRING;
            p->expect_len = CLEAR_SCREEN_STRING_LEN;
            p->expect_cmd = CLEAR_SCREEN;
            return WAITING;
        }
        else
        {
            p->state = CLEAN;
            p->buffer_len = 0;
            p->buffer_start = NULL;
            p->command = CURSOR_HOME;
            p->repeat = 1;
            return REPEAT | COMMAND;
        }
    }

    else if (p->state == ESC_BR_QUEST)
    {
        g_assert (p->buffer_len == 3 && !strncmp (p->buffer, "\033[?", 3));

        switch (*c)
        {
            case '2':
                p->state = ESC_BR_QUEST_2;
                p->buffer[p->buffer_len++] = *c;
                return WAITING;

            case '5':
                p->state = EXPECT;
                p->buffer[p->buffer_len++] = *c;
                /* TODO: there is delay specified: \E[?5h$<100/>\E[?5l */
                p->expect = FLASH_SCREEN_STRING;
                p->expect_len = FLASH_SCREEN_STRING_LEN;
                p->expect_cmd = FLASH_SCREEN;
                return WAITING;

            case '7':
                p->state = ESC_BR_QUEST_7;
                p->buffer[p->buffer_len++] = *c;
                return WAITING;

            case '1':
                p->state = ESC_BR_QUEST_1;
                p->buffer[p->buffer_len++] = *c;
                return WAITING;

            default:
                p->state = CLEAN;
                p->buffer[p->buffer_len++] = *c;
                DEBUG_PRINT ("invalid escape sequence",
                             p->buffer, p->buffer_len);
                return CHARS;
        }
    }

    else if (p->state == ESC_BR_QUEST_2)
    {
        g_assert (p->buffer_len == 4 && !strncmp (p->buffer, "\033[?2", 4));

        if (*c == '5')
        {
            p->state = ESC_BR_QUEST_25;
            p->buffer[p->buffer_len++] = *c;
            return WAITING;
        }
        else
        {
            p->state = CLEAN;
            p->buffer[p->buffer_len++] = *c;
            DEBUG_PRINT ("invalid escape sequence",
                         p->buffer, p->buffer_len);
            return CHARS;
        }
    }

    else if (p->state == ESC_BR_QUEST_25)
    {
        g_assert (p->buffer_len == 5 && !strncmp (p->buffer, "\033[?25", 5));

        switch (*c)
        {
            case 'l':
                p->state = CLEAN;
                p->buffer_len = 0;
                p->buffer_start = NULL;
                p->command = CURSOR_INVISIBLE;
                return COMMAND;
            case 'h':
                p->state = CLEAN;
                p->buffer_len = 0;
                p->buffer_start = NULL;
                p->command = CURSOR_NORMAL;
                return COMMAND;
            default:
                p->state = CLEAN;
                p->buffer[p->buffer_len++] = *c;
                DEBUG_PRINT ("invalid escape sequence",
                             p->buffer, p->buffer_len);
                return CHARS;
        }
    }

    else if (p->state == ESC_BR_QUEST_7)
    {
        g_assert (p->buffer_len == 4 && !strncmp (p->buffer, "\033[?7", 4));

        switch (*c)
        {
            case 'l':
                p->state = CLEAN;
                p->buffer_len = 0;
                p->buffer_start = NULL;
                p->command = EXIT_AM_MODE;
                return COMMAND;
            case 'h':
                p->state = CLEAN;
                p->buffer_len = 0;
                p->buffer_start = NULL;
                p->command = ENTER_AM_MODE;
                return COMMAND;
            default:
                p->state = CLEAN;
                p->buffer[p->buffer_len++] = *c;
                DEBUG_PRINT ("invalid escape sequence",
                             p->buffer, p->buffer_len);
                return CHARS;
        }
    }

    else if (p->state == ESC_BR_QUEST_1)
    {
        g_assert (p->buffer_len == 4 && !strncmp (p->buffer, "\033[?1", 4));

        switch (*c)
        {
            case '0':
                p->state = ESC_BR_QUEST_10;
                p->buffer[p->buffer_len++] = *c;
                return WAITING;
            case 'l':
                p->state = EXPECT;
                p->buffer[p->buffer_len++] = *c;
                p->expect = KEYPAD_LOCAL_STRING;
                p->expect_len = KEYPAD_LOCAL_STRING_LEN;
                p->expect_cmd = KEYPAD_LOCAL;
                return WAITING;
            case ';':
                p->state = EXPECT;
                p->buffer[p->buffer_len++] = *c;
                p->expect = USER8_STRING;
                p->expect_len = USER8_STRING_LEN;
                p->expect_cmd = USER8;
                return WAITING;
            default:
                p->state = CLEAN;
                p->buffer[p->buffer_len++] = *c;
                DEBUG_PRINT ("invalid escape sequence",
                             p->buffer, p->buffer_len);
                return CHARS;
        }
    }

    else if (p->state == ESC_BR_QUEST_10)
    {
        g_assert (p->buffer_len == 5 && !strncmp (p->buffer, "\033[?10", 5));

        if (*c == '4')
        {
            p->state = ESC_BR_QUEST_104;
            p->buffer[p->buffer_len++] = *c;
            return WAITING;
        }
        else
        {
            p->state = CLEAN;
            p->buffer[p->buffer_len++] = *c;
            DEBUG_PRINT ("invalid escape sequence",
                         p->buffer, p->buffer_len);
            return CHARS;
        }
    }

    else if (p->state == ESC_BR_QUEST_104)
    {
        g_assert (p->buffer_len == 6 && !strncmp (p->buffer, "\033[?104", 6));

        if (*c == '9')
        {
            p->state = ESC_BR_QUEST_1049;
            p->buffer[p->buffer_len++] = *c;
            return WAITING;
        }
        else
        {
            p->state = CLEAN;
            p->buffer[p->buffer_len++] = *c;
            DEBUG_PRINT ("invalid escape sequence",
                         p->buffer, p->buffer_len);
            return CHARS;
        }
    }

    else if (p->state == ESC_BR_QUEST_1049)
    {
        g_assert (p->buffer_len == 7 && !strncmp (p->buffer, "\033[?1049", 7));

        switch (*c)
        {
            case 'l':
                p->state = CLEAN;
                p->buffer_len = 0;
                p->buffer_start = NULL;
                p->command = EXIT_CA_MODE;
                return COMMAND;
            case 'h':
                p->state = CLEAN;
                p->buffer_len = 0;
                p->buffer_start = NULL;
                p->command = ENTER_CA_MODE;
                return COMMAND;
            default:
                p->state = CLEAN;
                p->buffer[p->buffer_len++] = *c;
                DEBUG_PRINT ("invalid escape sequence",
                             p->buffer, p->buffer_len);
                return CHARS;
        }
    }

    else if (p->state == EXPECT)
    {
        p->buffer[p->buffer_len++] = *c;

        if (*c != p->expect[p->buffer_len - 1])
        {
            p->state = CLEAN;
            DEBUG_PRINT ("invalid expect",
                         p->buffer, p->buffer_len);
            return CHARS;
        }

        if (p->buffer_len == p->expect_len)
        {
            p->state = CLEAN;
            p->buffer_len = 0;
            p->buffer_start = NULL;
            p->command = p->expect_cmd;
            return COMMAND;
        }

        return WAITING;
    }

    else if (p->state == ESC_BR_NUMS)
    {
        g_assert (p->buffer_len > 2 && p->buffer[0] == ESCAPE && p->buffer[1] == '[' &&
                ('0' <= p->buffer[2] && p->buffer[2] <= '9'));

        switch (*c)
        {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                return parse_nums_add_digit (p, *c);

            case ';':
                return parse_nums_semicolon (p, *c);

            case 'm':
                return parse_nums_finish_m (p, *c);

            case 'r':
            case 'H':
            case 'R':
                return parse_nums_finish_2 (p, *c);

            case 'K':
            case 'i':
            case 'h':
            case 'l':
            case 'g':
            case 'n':
            case 'A':
            case 'B':
            case 'C':
            case 'D':
            case 'd':
            case 'G':
            case 'P':
            case 'M':
            case 'X':
            case '@':
            case 'L':
                return parse_nums_finish_1 (p, *c);

            default:
                return parse_nums_invalid (p, *c);
        }
    }

    else
    {
#ifdef G_DISABLE_ASSERT
        p->buffer[p->buffer_len++] = *c;
        p->state = CLEAN;
        DEBUG_PRINT ("impossible",
                     p->buffer, p->buffer_len);
        return CHARS;
#else
        g_assert_not_reached ();
#endif
    }
}


static ParseCharResult parse_nums_add_digit (MooTermParser *p, char c)
{
    g_assert ('0' <= c && c <= '9');

    p->buffer[p->buffer_len++] = c;

    if (!p->this_num_len)
    {
        p->this_num = p->buffer + (p->buffer_len - 1);
        p->this_num_len = 1;
        p->nums_len++;
        p->nums[p->nums_len - 1] = c - '0';
        return WAITING;
    }
    else
    {
        p->this_num_len++;
        p->nums[p->nums_len - 1] = 10 * p->nums[p->nums_len - 1] + (c - '0');
        return WAITING;
    }
}


static ParseCharResult parse_nums_semicolon (MooTermParser *p, char c)
{
    g_assert (c == ';');

    p->buffer[p->buffer_len++] = c;

    if (!p->this_num_len)
    {
        p->state = CLEAN;
        DEBUG_PRINT ("semicolon not after number",
                     p->buffer, p->buffer_len);
        return CHARS;
    }

    p->this_num = NULL;
    p->this_num_len = 0;

    return WAITING;
}


static ParseCharResult parse_nums_finish_m  (MooTermParser *p, char c)
{
    g_assert (c == 'm');

    p->buffer[p->buffer_len++] = c;
    p->state = CLEAN;
    p->command = SET_ATTRS;
    p->this_num = NULL;
    p->this_num_len = 0;

    return COMMAND;
}


static ParseCharResult parse_nums_finish_2  (MooTermParser *p, char c)
{
    p->buffer[p->buffer_len++] = c;
    p->state = CLEAN;
    p->this_num = NULL;
    p->this_num_len = 0;

    if (p->nums_len != 2)
    {
        DEBUG_PRINT ("number of arguments != 2",
                     p->buffer, p->buffer_len);
        return CHARS;
    }

    switch (c)
    {
        case 'r':
            p->command = CHANGE_SCROLL_REGION;
            p->buffer_len = 0;
            return COMMAND;
        case 'R':
            p->command = USER6;
            p->buffer_len = 0;
            return COMMAND;
        case 'H':
            p->command = CURSOR_ADDRESS;
            p->buffer_len = 0;
            return COMMAND;

        default:
#ifdef G_DISABLE_ASSERT
            DEBUG_PRINT ("impossible",
                        p->buffer, p->buffer_len);
            return CHARS;
#else
            g_assert_not_reached ();
#endif
    }
}


static ParseCharResult parse_nums_finish_1  (MooTermParser *p, char c)
{
    guint num = p->nums[0];

    p->buffer[p->buffer_len++] = c;
    p->state = CLEAN;
    p->this_num = NULL;
    p->this_num_len = 0;

    if (p->nums_len != 1)
    {
        DEBUG_PRINT ("number of arguments != 1",
                     p->buffer, p->buffer_len);
        return CHARS;
    }

    switch (c)
    {
        case 'K':
            if (num != 1)
            {
                DEBUG_PRINT ("invalid escape sequence",
                             p->buffer, p->buffer_len);
                return CHARS;
            }
            else
            {
                p->command = CLR_BOL;
                p->buffer_len = 0;
                return COMMAND;
            }

        case 'i':
            if (num == 4)
            {
                p->command = PRTR_OFF;
                p->buffer_len = 0;
                return COMMAND;
            }
            else if (num == 5)
            {
                p->command = PRTR_ON;
                p->buffer_len = 0;
                return COMMAND;
            }
            else
            {
                DEBUG_PRINT ("invalid escape sequence",
                             p->buffer, p->buffer_len);
                return CHARS;
            }

        case 'h':
            if (num == 4)
            {
                p->command = ENTER_INSERT_MODE;
                p->buffer_len = 0;
                return COMMAND;
            }
            else
            {
                DEBUG_PRINT ("invalid escape sequence",
                             p->buffer, p->buffer_len);
                return CHARS;
            }

        case 'l':
            if (num == 4)
            {
                p->command = EXIT_INSERT_MODE;
                p->buffer_len = 0;
                return COMMAND;
            }
            else
            {
                DEBUG_PRINT ("invalid escape sequence",
                             p->buffer, p->buffer_len);
                return CHARS;
            }

        case 'g':
            if (num == 3)
            {
                p->command = CLEAR_ALL_TABS;
                p->buffer_len = 0;
                return COMMAND;
            }
            else
            {
                DEBUG_PRINT ("invalid escape sequence",
                             p->buffer, p->buffer_len);
                return CHARS;
            }

        case 'n':
            if (num == 6)
            {
                p->command = USER7;
                p->buffer_len = 0;
                return COMMAND;
            }
            else
            {
                DEBUG_PRINT ("invalid escape sequence",
                             p->buffer, p->buffer_len);
                return CHARS;
            }

        case 'A':
            p->command = PARM_UP_CURSOR;
            p->buffer_len = 0;
            return COMMAND;

        case 'B':
            p->command = PARM_DOWN_CURSOR;
            p->buffer_len = 0;
            return COMMAND;

        case 'C':
            p->command = PARM_RIGHT_CURSOR;
            p->buffer_len = 0;
            return COMMAND;

        case 'D':
            p->command = PARM_LEFT_CURSOR;
            p->buffer_len = 0;
            return COMMAND;

        case 'd':
            p->command = ROW_ADDRESS;
            p->buffer_len = 0;
            return COMMAND;

        case 'G':
            p->command = COLUMN_ADDRESS;
            p->buffer_len = 0;
            return COMMAND;

        case 'P':
            p->command = PARM_DCH;
            p->buffer_len = 0;
            return COMMAND;

        case 'M':
            p->command = PARM_DELETE_LINE;
            p->buffer_len = 0;
            return COMMAND;

        case 'X':
            p->command = ERASE_CHARS;
            p->buffer_len = 0;
            return COMMAND;

        case '@':
            p->command = PARM_ICH;
            p->buffer_len = 0;
            return COMMAND;

        case 'L':
            p->command = PARM_INSERT_LINE;
            p->buffer_len = 0;
            return COMMAND;

        default:
#ifdef G_DISABLE_ASSERT
            DEBUG_PRINT ("impossible",
                         p->buffer, p->buffer_len);
            return CHARS;
#else
            g_assert_not_reached ();
#endif
    }
}

static ParseCharResult parse_nums_invalid   (MooTermParser *p, char c)
{
    p->buffer[p->buffer_len++] = c;
    p->state = CLEAN;
    p->this_num = NULL;
    p->this_num_len = 0;

    DEBUG_PRINT ("number of arguments != 1",
                 p->buffer, p->buffer_len);

    return CHARS;
}


MooTermParser  *moo_term_parser_new     (MooTermBuffer  *buf)
{
    MooTermParser *p = g_new0 (MooTermParser, 1);
    p->parent = buf;
    return p;
}


void            moo_term_parser_free    (MooTermParser  *parser)
{
    g_return_if_fail (parser != NULL);
    g_free (parser);
}


static void exec_command    (MooTermParser  *parser);

void            moo_term_parser_parse   (MooTermParser  *parser,
                                         const char     *string,
                                         gssize          len)
{
    const char *p;
    const char *chars = NULL;
    guint chars_len = 0;

    g_assert (parser != NULL);
    g_return_if_fail (string != NULL && len != 0);

    p = string;
    while ((len > 0 && p < string + len) || (len < 0 && *p != 0))
    {
        ParseCharResult res = parse_char (parser, p);

        if (res == ONE_CHAR)
        {
            if (chars)
            {
                chars_len++;
            }
            else
            {
                chars = p;
                chars_len = 1;
            }
        }
        else if (res == CHARS)
        {
            if (chars)
            {
                chars_len += parser->buffer_len;
            }
            else
            {
                chars = parser->buffer_start;
                chars_len = parser->buffer_len;
            }
        }
        else if (res & COMMAND)
        {
            if (chars)
            {
                buf_plain_chars (parser->parent, chars, chars_len);
                chars = NULL;
            }

            exec_command (parser);

            if (res & REPEAT)
            {
                p -= (parser->repeat - 1);
                parser->repeat = 0;
                continue;
            }
        }

        ++p;
    }

    if (chars)
    {
        buf_plain_chars (parser->parent, chars, chars_len);
        parser->buffer_start = NULL;
    }
}


static void buf_set_attributes  (MooTermBuffer  *buf,
                                 guint          *attrs,
                                 guint           attrs_len)
{
    guint i;

    if (!attrs_len)
    {
        buf_set_attrs_mask (buf, 0);
        buf_set_secure_mode (buf, FALSE);
        return;
    }

    for (i = 0; i < attrs_len; ++i)
    {
        switch (attrs[i])
        {
            case ANSI_ALL_ATTRS_OFF:
                buf_set_attrs_mask (buf, 0);
                buf_set_secure_mode (buf, FALSE);
                break;
            case ANSI_BOLD:
                buf_add_attrs_mask (buf, MOO_TERM_TEXT_BOLD);
                break;
            case ANSI_UNDERSCORE:
                buf_add_attrs_mask (buf, MOO_TERM_TEXT_UNDERLINE);
                break;
            case ANSI_BLINK:
                g_warning ("%s: ignoring blink", G_STRLOC);
                break;
            case ANSI_REVERSE:
                buf_add_attrs_mask (buf, MOO_TERM_TEXT_REVERSE);
                break;
            case ANSI_CONCEALED:
                buf_set_secure_mode (buf, TRUE);
                break;
            case 20 + ANSI_BOLD:
                buf_remove_attrs_mask (buf, MOO_TERM_TEXT_BOLD);
                break;
            case 20 + ANSI_UNDERSCORE:
                buf_remove_attrs_mask (buf, MOO_TERM_TEXT_UNDERLINE);
                break;
            case 20 + ANSI_BLINK:
                g_warning ("%s: ignoring blink", G_STRLOC);
                break;
            case 20 + ANSI_REVERSE:
                buf_remove_attrs_mask (buf, MOO_TERM_TEXT_REVERSE);
                break;
            case 20 + ANSI_CONCEALED:
                buf_set_secure_mode (buf, FALSE);
                break;

            default:
                if (30 <= attrs[i] && attrs[i] <= 37)
                    buf_set_ansi_foreground (buf, attrs[i] - 30);
                else if (40 <= attrs[i] && attrs[i] <= 47)
                    buf_set_ansi_background (buf, attrs[i] - 40);
                else if (39 == attrs[i])
                    buf_set_ansi_foreground (buf, 8);
                else if (49 == attrs[i])
                    buf_set_ansi_background (buf, 8);
                else
                    g_warning ("%s: unknown text attribute %d",
                               G_STRLOC, attrs[i]);
        }
    }
}


static void exec_command    (MooTermParser  *parser)
{
    switch (parser->command)
    {
        case SET_ATTRS:
            if (parser->nums_len)
                buf_set_attributes (parser->parent,
                                    parser->nums,
                                    parser->nums_len);
            else
                buf_set_attributes (parser->parent,
                                    NULL, 0);
            break;

        case PARM_LEFT_CURSOR:
            buf_cursor_move (parser->parent, 0, -parser->nums[0]);
            break;
        case PARM_RIGHT_CURSOR:
            buf_cursor_move (parser->parent, 0, parser->nums[0]);
            break;
        case PARM_DOWN_CURSOR:
            buf_parm_down_cursor (parser->parent, parser->nums[0]);
            break;
        case PARM_UP_CURSOR:
            buf_cursor_move (parser->parent, -parser->nums[0], 0);
            break;
        case ROW_ADDRESS:
            buf_cursor_move_to (parser->parent, parser->nums[0] - 1, -1);
            break;
        case COLUMN_ADDRESS:
            buf_cursor_move_to (parser->parent, -1, parser->nums[0] - 1);
            break;
        case PARM_DCH:
            buf_parm_dch (parser->parent, parser->nums[0]);
            break;
        case PARM_DELETE_LINE:
            buf_parm_delete_line (parser->parent, parser->nums[0]);
            break;
        case ERASE_CHARS:
            buf_erase_chars (parser->parent, parser->nums[0]);
            break;
        case PARM_ICH:
            buf_parm_ich (parser->parent, parser->nums[0]);
            break;
        case PARM_INSERT_LINE:
            buf_parm_insert_line (parser->parent, parser->nums[0]);
            break;
        case SET_BACKGROUND:
            buf_set_background (parser->parent, parser->nums[0]);
            break;
        case SET_FOREGROUND:
            buf_set_foreground (parser->parent, parser->nums[0]);
            break;

        case CHANGE_SCROLL_REGION:
            buf_change_scroll_region (parser->parent,
                                      parser->nums[0] - 1,
                                      parser->nums[1] - 1);
            break;
        case CURSOR_ADDRESS:
            buf_cursor_move_to (parser->parent,
                                parser->nums[0] - 1,
                                parser->nums[1] - 1);
            break;
        case USER6:
            buf_user6 (parser->parent,
                       parser->nums[0] - 1,
                       parser->nums[1] - 1);
            break;

        case BACK_TAB:
            buf_back_tab (parser->parent);
            break;
        case CURSOR_NORMAL:
            buf_cursor_normal (parser->parent);
            break;
        case CURSOR_INVISIBLE:
            buf_cursor_invisible (parser->parent);
            break;
        case CLEAR_SCREEN:
            buf_clear_screen (parser->parent);
            break;
        case CURSOR_RIGHT:
            buf_cursor_move (parser->parent, 0, 1);
            break;
        case CURSOR_LEFT:
            buf_cursor_move (parser->parent, 0, -1);
            break;
        case CURSOR_DOWN:
            buf_cursor_down (parser->parent);
            break;
        case CURSOR_UP:
            buf_cursor_move (parser->parent, -1, 0);
            break;
        case CURSOR_HOME:
            buf_cursor_home (parser->parent);
            break;
        case CARRIAGE_RETURN:
            buf_carriage_return (parser->parent);
            break;
        case DELETE_CHARACTER:
            buf_delete_character (parser->parent);
            break;
        case DELETE_LINE:
            buf_delete_line (parser->parent);
            break;
        case CLR_EOS:
            buf_clr_eos (parser->parent);
            break;
        case CLR_EOL:
            buf_clr_eol (parser->parent);
            break;
        case CLR_BOL:
            buf_clr_bol (parser->parent);
            break;
        case ENA_ACS:
            buf_ena_acs (parser->parent);
            break;
        case FLASH_SCREEN:
            buf_flash_screen (parser->parent);
            break;
        case INSERT_LINE:
            buf_insert_line (parser->parent);
            break;
        case ENTER_SECURE_MODE:
            buf_enter_secure_mode (parser->parent);
            break;
        case INIT_2STRING:
            buf_init_2string (parser->parent);
            break;
        case RESET_2STRING:
            buf_reset_2string (parser->parent);
            break;
        case RESET_1STRING:
            buf_reset_1string (parser->parent);
            break;
        case PRINT_SCREEN:
            buf_print_screen (parser->parent);
            break;
        case PRTR_OFF:
            buf_prtr_off (parser->parent);
            break;
        case PRTR_ON:
            buf_prtr_on (parser->parent);
            break;
        case ORIG_PAIR:
            buf_orig_pair (parser->parent);
            break;
        case ENTER_REVERSE_MODE:
            buf_enter_reverse_mode (parser->parent);
            break;
        case ENTER_AM_MODE:
            buf_enter_am_mode (parser->parent);
            break;
        case EXIT_AM_MODE:
            buf_exit_am_mode (parser->parent);
            break;
        case ENTER_CA_MODE:
            buf_enter_ca_mode (parser->parent);
            break;
        case EXIT_CA_MODE:
            buf_exit_ca_mode (parser->parent);
            break;
        case ENTER_INSERT_MODE:
            buf_enter_insert_mode (parser->parent);
            break;
        case EXIT_INSERT_MODE:
            buf_exit_insert_mode (parser->parent);
            break;
        case ENTER_STANDOUT_MODE:
            buf_enter_reverse_mode (parser->parent);
            break;
        case EXIT_STANDOUT_MODE:
            buf_exit_reverse_mode (parser->parent);
            break;
        case ENTER_UNDERLINE_MODE:
            buf_enter_underline_mode (parser->parent);
            break;
        case EXIT_UNDERLINE_MODE:
            buf_exit_underline_mode (parser->parent);
            break;
        case KEYPAD_LOCAL:
            buf_keypad_local (parser->parent);
            break;
        case KEYPAD_XMIT:
            buf_keypad_xmit (parser->parent);
            break;
        case CLEAR_ALL_TABS:
            buf_clear_all_tabs (parser->parent);
            break;
        case USER7:
            buf_user7 (parser->parent);
            break;
        case USER8:
            buf_user8 (parser->parent);
            break;
        case USER9:
            buf_user9 (parser->parent);
            break;
        case BELL:
            buf_bell (parser->parent);
            break;
        case ENTER_ALT_CHARSET_MODE:
            buf_enter_alt_charset_mode (parser->parent);
            break;
        case EXIT_ALT_CHARSET_MODE:
            buf_exit_alt_charset_mode (parser->parent);
            break;
        case TAB:
            buf_tab (parser->parent);
            break;
        case SCROLL_FORWARD:
            buf_scroll_forward (parser->parent);
            break;
        case SCROLL_REVERSE:
            buf_scroll_reverse (parser->parent);
            break;
        case SET_TAB:
            buf_set_tab (parser->parent);
            break;
        case ESC_l:
            buf_esc_l (parser->parent);
            break;
        case ESC_m:
            buf_esc_m (parser->parent);
            break;
        case RESTORE_CURSOR:
            buf_restore_cursor (parser->parent);
            break;
        case SAVE_CURSOR:
            buf_save_cursor (parser->parent);
            break;
    }
}
