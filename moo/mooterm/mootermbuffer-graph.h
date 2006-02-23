/*
 *   mootermbuffer-graph.h
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

#ifndef MOOTERM_COMPILATION
#error "This file may not be included"
#endif

#ifndef __MOO_TERM_BUFFER_GRAPH_H__
#define __MOO_TERM_BUFFER_GRAPH_H__


#define ACS_STERLING    "\302\243"      /* U+00A3 POUND SIGN */
#define ACS_DARROW      "\342\206\223"  /* U+2193 DOWNWARDS ARROW */
#define ACS_LARROW      "\342\206\220"  /* U+2190 LEFTWARDS ARROW */
#define ACS_RARROW      "\342\206\222"  /* U+2192 RIGHTWARDS ARROW */
#define ACS_UARROW      "\342\206\221"  /* U+2191 UPWARDS ARROW */
#define ACS_BOARD       "#"             /* ??? */
#define ACS_BULLET      "\342\200\242"  /* U+2022 BULLET */
#define ACS_CKBOARD     "\342\226\223"  /* U+2593 DARK SHADE */
#define ACS_DEGREE      "\302\260"      /* U+00B0 DEGREE SIGN */
#define ACS_DIAMOND     "\342\227\206"  /* U+25C6 BLACK DIAMOND */
#define ACS_GEQUAL      "\342\211\245"  /* U+2265 GREATER-THAN OR EQUAL TO */
#define ACS_PI          "\317\200"      /* U+03C0 GREEK SMALL LETTER PI */
#define ACS_HLINE       "\342\224\200"  /* U+2500 BOX DRAWINGS LIGHT HORIZONTAL */
#define ACS_LANTERN     "\347\201\257"  /* U+706F CJK UNIFIED IDEOGRAPH-706F ??? */
#define ACS_PLUS        "\342\224\274"  /* U+253C BOX DRAWINGS LIGHT VERTICAL AND HORIZONTAL */
#define ACS_LEQUAL      "\342\211\244"  /* U+2264 LESS-THAN OR EQUAL TO */
#define ACS_LLCORNER    "\342\224\224"  /* U+2514 BOX DRAWINGS LIGHT UP AND RIGHT */
#define ACS_LRCORNER    "\342\224\230"  /* U+2518 BOX DRAWINGS LIGHT UP AND LEFT */
#define ACS_NEQUAL      "\342\211\240"  /* U+2260 NOT EQUAL TO */
#define ACS_PLMINUS     "\302\261"      /* U+00B1 PLUS-MINUS SIGN */
#define ACS_S1          "\342\216\272"  /* U+23BA HORIZONTAL SCAN LINE-1 */
#define ACS_S3          "\342\216\273"  /* U+23BB HORIZONTAL SCAN LINE-3 */
#define ACS_S7          "\342\216\274"  /* U+23BC HORIZONTAL SCAN LINE-7 */
#define ACS_S9          "\342\216\275"  /* U+23BD HORIZONTAL SCAN LINE-9 */
#define ACS_BLOCK       "\342\226\210"  /* U+2588 FULL BLOCK */
#define ACS_TTEE        "\342\224\254"  /* U+252C BOX DRAWINGS LIGHT DOWN AND HORIZONTAL */
#define ACS_RTEE        "\342\224\244"  /* U+2524 BOX DRAWINGS LIGHT VERTICAL AND LEFT */
#define ACS_LTEE        "\342\224\234"  /* U+251C BOX DRAWINGS LIGHT VERTICAL AND RIGHT */
#define ACS_BTEE        "\342\224\264"  /* U+2534 BOX DRAWINGS LIGHT UP AND HORIZONTAL */
#define ACS_ULCORNER    "\342\224\214"  /* U+250C BOX DRAWINGS LIGHT DOWN AND RIGHT */
#define ACS_URCORNER    "\342\224\220"  /* U+2510 BOX DRAWINGS LIGHT DOWN AND LEFT */
#define ACS_VLINE       "\342\224\202"  /* U+2502 BOX DRAWINGS LIGHT VERTICAL */


/*
blank                                        " "                              " "                             _
diamond                                      ACS_DIAMOND                      "+"                            `
checker board (stipple)                      ACS_CKBOARD                      ":"                            a
horizontal tab                               "\t"                             "\t"                           b
form feed                                    "\014"                           "\014"                         c
carriage return                              "\r"                             "\r"                           d
line feed                                    "\n"                             "\n"                           e
degree symbol                                ACS_DEGREE                       "\"                            f
plus/minus                                   ACS_PLMINUS                      "#"                            g
board of squares                             ACS_BOARD                        "#"                            h
lantern symbol                               ACS_LANTERN                      "#"                            i
lower right corner                           ACS_LRCORNER                     "+"                            j
upper right corner                           ACS_URCORNER                     "+"                            k
upper left corner                            ACS_ULCORNER                     "+"                            l
lower left corner                            ACS_LLCORNER                     "+"                            m
large plus or crossover                      ACS_PLUS                         "+"                            n
scan line 1                                  ACS_S1                           "~"                            o
scan line 3                                  ACS_S3                           "-"                            p
horizontal line                              ACS_HLINE                        "-"                            q
scan line 7                                  ACS_S7                           "-"                            r
scan line 9                                  ACS_S9                           "_"                            s
tee pointing right                           ACS_LTEE                         "+"                            t
tee pointing left                            ACS_RTEE                         "+"                            u
tee pointing up                              ACS_BTEE                         "+"                            v
tee pointing down                            ACS_TTEE                         "+"                            w
vertical line                                ACS_VLINE                        "|"                            x
less-than-or-equal-to                        ACS_LEQUAL                       "<"                            y
greater-than-or-equal-to                     ACS_GEQUAL                       ">"                            z
greek pi                                     ACS_PI                           "*"                            {
not-equal                                    ACS_NEQUAL                       "!"                            |
UK pound sign                                ACS_STERLING                     "f"                            }
bullet                                       ACS_BULLET                       "o"                            ~
*/


/* drawing chars are _`a-z{|}~ -- 0x5F - 0x7E */
#define MAX_GRAPH 126

static const char *DRAWING_SET_STRINGS[MAX_GRAPH + 1] = {
    /* 95 nulls */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,

    /* do we need control characters here? */
    " ", ACS_DIAMOND, ACS_CKBOARD, "?", "?", "?", "?", ACS_DEGREE, ACS_PLMINUS, ACS_BOARD,
    ACS_LANTERN, ACS_LRCORNER, ACS_URCORNER, ACS_ULCORNER, ACS_LLCORNER, ACS_PLUS, ACS_S1, ACS_S3,
    ACS_HLINE, ACS_S7, ACS_S9, ACS_LTEE, ACS_RTEE, ACS_BTEE, ACS_TTEE, ACS_VLINE, ACS_LEQUAL,
    ACS_GEQUAL, ACS_PI, ACS_NEQUAL, ACS_STERLING, ACS_BULLET
};

static gunichar ASCII_DRAWING_SET[MAX_GRAPH + 1] = {
    /* 95 0s */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

    ' ', '+', ':', '?', '?', '?', '?', '\\', '#', '#', '#',
    '+', '+', '+', '+', '+', '~', '-', '-', '-', '_',
    '+', '+', '+', '+', '|', '<', '>', '*', '!', 'f', 'o'
};

static gunichar DRAWING_SET[MAX_GRAPH + 1];

static gunichar *graph_sets[5];

static void init_drawing_sets (void)
{
    guint i;

    for (i = 0; i <= MAX_GRAPH; ++i)
    {
        if (DRAWING_SET_STRINGS[i])
            DRAWING_SET[i] = g_utf8_get_char (DRAWING_SET_STRINGS[i]);
        else if ('\040' <= i && i <= '\176')
            DRAWING_SET[i] = i;

        if (!ASCII_DRAWING_SET[i] && '\040' <= i && i <= '\176')
            ASCII_DRAWING_SET[i] = i;
    }

    graph_sets[0] = graph_sets[2] = DRAWING_SET;
    graph_sets[1] = graph_sets[3] = graph_sets[4] = NULL;
}


#endif /* __MOO_TERM_BUFFER_GRAPH_H__ */
