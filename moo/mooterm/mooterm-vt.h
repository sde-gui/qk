/*
 *   mooterm-vt.h
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

#ifndef __MOO_TERM_VT_H__
#define __MOO_TERM_VT_H__

enum {
    MODE_IRM,       /*  Insert/Replace Mode
                        This control function selects how the terminal adds characters to page
                        memory. The terminal always adds new characters at the cursor position.
                        If IRM mode is set, then new characters move characters in page memory to
                        the right. Characters moved past the page's right border are lost.
                        If IRM mode is reset, then new characters replace the character at the cursor
                        position.   */
    MODE_SRM,       /*  Local Echo: Send/Receive Mode
                        This control function turns local echo on or off. When local echo is on,
                        the terminal sends keyboard characters to the screen. The host does not
                        have to send (echo) the characters back to the terminal display. When
                        local echo is off, the terminal only sends characters to the host. It
                        is up to the host to echo characters back to the screen.
                        When the SRM function is set, the terminal sends keyboard characters to
                        the host only. The host can echo the characters back to the screen.
                        When the SRM function is reset, the terminal sends keyboard characters
                        to the host and to the screen. The host does have to echo characters back
                        to the terminal.    */
    MODE_LNM,       /*  Line Feed/New Line Mode
                        This control function selects the characters sent to the host when you
                        press the Return key. LNM also controls how the terminal interprets line
                        feed (LF), form feed (FF), and vertical tab (VT) characters.
                        If LNM is set, then the cursor moves to the first column on the next line
                        when the terminal receives an LF, FF, or VT character. When you press Return,
                        the terminal sends both a carriage return (CR) and line feed (LF).
                        If LNM is reset, then the cursor moves to the current column on the next line
                        when the terminal receives an LF, FF, or VT character. When you press Return,
                        the terminal sends only a carriage return (CR) character.
                        When the auxiliary keypad is in keypad numeric mode (DECKPNM), the Enter key
                        sends the same characters as the Return key.    */
    MODE_DECCKM,    /*  Cursor Keys Mode
                        This control function selects the sequences the arrow keys send. You can use
                        the four arrow keys to move the cursor through the current page or to send
                        special application commands.
                        If the DECCKM function is set, then the arrow keys send application sequences
                        to the host.
                        If the DECCKM function is reset, then the arrow keys send ANSI cursor sequences
                        to the host.    */
    MODE_DECANM,    /*  ANSI Mode
                        DECANM changes the terminal to the VT52 mode of operation. In VT52 mode,
                        the terminal acts like a VT52 terminal. This mode lets you use applications
                        designed for the VT52 terminal. */
    MODE_DECSCNM,   /*  Screen Mode: Light or Dark Screen
                        This control function selects a dark or light background on the screen.
                        When DECSCNM is set, the screen displays dark characters on a light background.
                        When DECSCNM is reset, the screen displays light characters on a dark background.   */
    MODE_DECOM,     /*  Origin Mode
                        This control function sets the origin for the cursor. DECOM determines if the
                        cursor position is restricted to inside the page margins. When you power up or
                        reset the terminal, you reset origin mode.
                        When DECOM is set, the home cursor position is at the upper-left corner of the
                        screen, within the margins. The starting point for line numbers depends on the
                        current top margin setting. The cursor cannot move outside of the margins.
                        When DECOM is reset, the home cursor position is at the upper-left corner of the
                        screen. The starting point for line numbers is independent of the margins. The
                        cursor can move outside of the margins. */
    MODE_DECAWM,    /*  Autowrap Mode
                        This control function determines whether or not received characters automatically
                        wrap to the next line when the cursor reaches the right border of a page in page
                        memory.
                        If the DECAWM function is set, then graphic characters received when the cursor
                        is at the right border of the page appear at the beginning of the next line. Any
                        text on the page scrolls up if the cursor is at the end of the scrolling region.
                        If the DECAWM function is reset, then graphic characters received when the cursor
                        is at the right border of the page replace characters already on the page.
                        NOTE: Regardless of this selection, the tab character never moves the cursor
                        to the next line. */
    MODE_DECTCEM,   /*  Text Cursor Enable Mode
                        This control function makes the cursor visible or invisible.
                        Set: makes the cursor visible.
                        Reset: makes the cursor invisible.  */
    MODE_DECNKM,    /*  Numeric Keypad Mode
                        This control function works like the DECKPAM and DECKPNM functions. DECNKM is
                        provided mainly for use with the request and report mode (DECRQM/DECRPM) control
                        functions.
                        Set: application sequences.
                        Reset: keypad characters.   */
    MODE_DECBKM,    /*  Backarrow Key Mode
                        This control function determines whether the Backspace key works as a
                        backspace key or delete key.
                        If DECBKM is set, Backspace works as a backspace key. When you press Backspace,
                        the terminal sends a BS character to the host.
                        If DECBKM is reset, Backspace works as a delete key. When you press Backspace,
                        the terminal sends a DEL character to the host.    */
    MODE_DECKPM,    /*  Key Position Mode
                        This control function selects whether the keyboard sends character codes or key
                        position reports to the host. DECKPM lets new applications take full control of
                        the keyboard including single shifts, locking shifts, and compose character processing.
                        If the DECKPM function is set, then all keyboard keys send extended reports that
                        include the key position and the state of modifier keys when pressed. A modifier
                        key is pressed in combination with another key to modify the code sent by that key.
                        The Ctrl key is a modifier key.
                        If the DECKPM function is reset, then the keyboard keys send character codes.
                        DECKPM only affects keyboard input; it does not affect how the terminal interprets data
                        from the host.  */

    MODE_CA,
    MODE_REVERSE_WRAPAROUND,

    MODE_PRESS_TRACKING,
    MODE_PRESS_AND_RELEASE_TRACKING,
    MODE_HILITE_MOUSE_TRACKING,

    MODE_MAX
};


#define DEFAULT_MODE_IRM                        FALSE
#define DEFAULT_MODE_SRM                        FALSE
#define DEFAULT_MODE_LNM                        FALSE
#define DEFAULT_MODE_DECCKM                     FALSE
#define DEFAULT_MODE_DECANM                     FALSE
#define DEFAULT_MODE_DECSCNM                    FALSE
#define DEFAULT_MODE_DECOM                      FALSE
#define DEFAULT_MODE_DECAWM                     TRUE
#define DEFAULT_MODE_DECTCEM                    TRUE
#define DEFAULT_MODE_DECNKM                     FALSE
#define DEFAULT_MODE_DECBKM                     TRUE    /* Backspace key send BS */
#define DEFAULT_MODE_DECKPM                     FALSE
#define DEFAULT_MODE_CA                         FALSE
#define DEFAULT_MODE_REVERSE_WRAPAROUND         FALSE
#define DEFAULT_MODE_PRESS_TRACKING             FALSE
#define DEFAULT_MODE_PRESS_AND_RELEASE_TRACKING FALSE
#define DEFAULT_MODE_HILITE_MOUSE_TRACKING      FALSE


#define set_default_modes(ar)                                                       \
{                                                                                   \
    ar[MODE_IRM] = DEFAULT_MODE_IRM;                                                \
    ar[MODE_SRM] = DEFAULT_MODE_SRM;                                                \
    ar[MODE_LNM] = DEFAULT_MODE_LNM;                                                \
    ar[MODE_DECCKM] = DEFAULT_MODE_DECCKM;                                          \
    ar[MODE_DECANM] = DEFAULT_MODE_DECANM;                                          \
    ar[MODE_DECSCNM] = DEFAULT_MODE_DECSCNM;                                        \
    ar[MODE_DECOM] = DEFAULT_MODE_DECOM;                                            \
    ar[MODE_DECAWM] = DEFAULT_MODE_DECAWM;                                          \
    ar[MODE_DECTCEM] = DEFAULT_MODE_DECTCEM;                                        \
    ar[MODE_DECNKM] = DEFAULT_MODE_DECNKM;                                          \
    ar[MODE_DECBKM] = DEFAULT_MODE_DECBKM;                                          \
    ar[MODE_DECKPM] = DEFAULT_MODE_DECKPM;                                          \
    ar[MODE_CA] = DEFAULT_MODE_CA;                                                  \
    ar[MODE_REVERSE_WRAPAROUND] = DEFAULT_MODE_REVERSE_WRAPAROUND;                  \
    ar[MODE_PRESS_TRACKING] = DEFAULT_MODE_PRESS_TRACKING;                          \
    ar[MODE_PRESS_AND_RELEASE_TRACKING] = DEFAULT_MODE_PRESS_AND_RELEASE_TRACKING;  \
    ar[MODE_HILITE_MOUSE_TRACKING] = MODE_HILITE_MOUSE_TRACKING;                    \
}

#define buf_get_mode(mode)  (buf->priv->modes[mode])
#define term_get_mode(mode) (term->priv->modes[mode])


#define GET_DEC_MODE(code, mode)                    \
    switch (code)                                   \
    {                                               \
        case 1:                                     \
            mode = MODE_DECCKM;                     \
            break;                                  \
        case 2:                                     \
            mode = MODE_DECANM;                     \
            break;                                  \
        case 5:                                     \
            mode = MODE_DECSCNM;                    \
            break;                                  \
        case 6:                                     \
            mode = MODE_DECOM;                      \
            break;                                  \
        case 7:                                     \
            mode = MODE_DECAWM;                     \
            break;                                  \
        case 25:                                    \
            mode = MODE_DECTCEM;                    \
            break;                                  \
        case 66:                                    \
            mode = MODE_DECNKM;                     \
            break;                                  \
        case 67:                                    \
            mode = MODE_DECBKM;                     \
            break;                                  \
        case 81:                                    \
            mode = MODE_DECKPM;                     \
            break;                                  \
                                                    \
        case 9:                                     \
            mode = MODE_PRESS_TRACKING;             \
            break;                                  \
        case 45:                                    \
            mode = MODE_REVERSE_WRAPAROUND;         \
            break;                                  \
        case 1000:                                  \
            mode = MODE_PRESS_AND_RELEASE_TRACKING; \
            break;                                  \
        case 1049:                                  \
            mode = MODE_CA;                         \
            break;                                  \
                                                    \
        case 3:                                     \
        case 4:                                     \
        case 8:                                     \
        case 18:                                    \
        case 19:                                    \
        case 34:                                    \
        case 35:                                    \
        case 36:                                    \
        case 40:                                    \
        case 42:                                    \
        case 57:                                    \
        case 60:                                    \
        case 61:                                    \
        case 64:                                    \
        case 68:                                    \
        case 69:                                    \
        case 73:                                    \
        case 95:                                    \
        case 96:                                    \
        case 97:                                    \
        case 98:                                    \
        case 99:                                    \
        case 100:                                   \
        case 101:                                   \
        case 102:                                   \
        case 103:                                   \
        case 104:                                   \
        case 106:                                   \
        case 1001:                                  \
            g_message ("%s: IGNORING mode %d",      \
                       G_STRLOC, code);             \
            break;                                  \
                                                    \
        default:                                    \
            g_warning ("%s: unknown mode %d",       \
                       G_STRLOC, code);             \
    }

#define GET_ANSI_MODE(code, mode)                   \
    switch (code)                                   \
    {                                               \
        case 4:                                     \
            mode = MODE_IRM;                        \
            break;                                  \
        case 12:                                    \
            mode = MODE_SRM;                        \
            break;                                  \
        case 20:                                    \
            mode = MODE_LNM;                        \
            break;                                  \
                                                    \
        case 1:                                     \
        case 2:                                     \
        case 3:                                     \
        case 5:                                     \
        case 7:                                     \
        case 10:                                    \
        case 11:                                    \
        case 13:                                    \
        case 14:                                    \
        case 15:                                    \
        case 16:                                    \
        case 17:                                    \
        case 18:                                    \
        case 19:                                    \
            g_warning ("%s: ignoring mode %d",      \
                       G_STRLOC, code);             \
            break;                                  \
                                                    \
        case -1:                                    \
            break;                                  \
                                                    \
        default:                                    \
            g_warning ("%s: unknown mode %d",       \
                       G_STRLOC, code);             \
    }


#define VT_ESC_             "\033"
#define VT_CSI_             "\033["
#define VT_DCS_             "\033P"
#define VT_ST_              "\033\\"
#define VT_DECID_           VT_CSI_ "?1;2c"

/*
DA1:
1   132 columns
2   Printer port
4   Sixel
6   Selective erase
7   Soft character set (DRCS)       TODO
8   User-defined keys (UDKs)        TODO
9   National replacement character sets (NRCS)
    (International terminal only)   TODO
12  Yugoslavian (SCS)
15  Technical character set         TODO
18  Windowing capability            TODO
21  Horizontal scrolling            TODO
23  Greek
24  Turkish
42  ISO Latin-2 character set
44  PCTerm                          TODO
45  Soft key map                    TODO
46  ASCII emulation                 TODO
*/
#define VT_DA1_             VT_CSI_ "?64;1;c"

/* TODO */
#define VT_DA2_             VT_CSI_ ">61;20;1;c"
/* TODO */
#define VT_DA3_             VT_DCS_ "!|FFFFFFFF" VT_ST_


typedef enum {
    ANSI_ALL_ATTRIBUTES_OFF = 0,
    ANSI_BOLD               = 1,
    ANSI_UNDERLINE          = 4,
    ANSI_BLINKING           = 5,
    ANSI_NEGATIVE           = 7,
    ANSI_INVISIBLE          = 8,

    /* TODO: why 22, why not 21? */
    ANSI_BOLD_OFF           = 22,
    ANSI_UNDERLINE_OFF      = 24,
    ANSI_BLINKING_OFF       = 25,
    ANSI_NEGATIVE_OFF       = 27,
    ANSI_INVISIBLE_OFF      = 28,

    ANSI_FORE_BLACK         = 30,
    ANSI_FORE_RED           = 31,
    ANSI_FORE_GREEN         = 32,
    ANSI_FORE_YELLOW        = 33,
    ANSI_FORE_BLUE          = 34,
    ANSI_FORE_MAGENTA       = 35,
    ANSI_FORE_CYAN          = 36,
    ANSI_FORE_WHITE         = 37,

    ANSI_BACK_BLACK         = 40,
    ANSI_BACK_RED           = 41,
    ANSI_BACK_GREEN         = 42,
    ANSI_BACK_YELLOW        = 43,
    ANSI_BACK_BLUE          = 44,
    ANSI_BACK_MAGENTA       = 45,
    ANSI_BACK_CYAN          = 46,
    ANSI_BACK_WHITE         = 47
} AnsiTextAttr;


/* DECRQSS parameters */
typedef enum {
    CODE_DECSASD,       /* Select Active Status Display*/
    CODE_DECSCL,        /* Set Conformance Level */
    CODE_DECSCPP,       /* Set Columns Per Page */
    CODE_DECSLPP,       /* Set Lines Per Page */
    CODE_DECSNLS,       /* Set Number of Lines per Screen */
    CODE_DECSTBM        /* Set Top and Bottom Margins */
} DECRQSSCode;

#define FINAL_DECSASD   "$g"
#define FINAL_DECSCL    "\"p"
#define FINAL_DECSCPP   "$|"
#define FINAL_DECSLPP   "t"
#define FINAL_DECSNLS   "*|"
#define FINAL_DECSTBM   "r"


#endif /* __MOO_TERM_VT_H__ */
