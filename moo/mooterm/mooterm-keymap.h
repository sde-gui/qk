/*
 *   mooterm/mooterm-keymap.h
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

#ifndef MOOTERM_MOOTERM_KEYMAP_H
#define MOOTERM_MOOTERM_KEYMAP_H

#include <gdk/gdkkeysyms.h>
#include <string.h>

/*
When the auxiliary keypad is in keypad numeric mode
(DECKPNM), the Enter key sends the same characters as
the Return key.
*/
/*
If the DECCKM function is set, then the arrow keys send
application sequences to the host. If the DECCKM function
is reset, then the arrow keys send ANSI cursor sequences
to the host.
*/
/*
MODE_DECNKM, Numeric Keypad Mode
This control function works like the DECKPAM and DECKPNM functions. DECNKM is
provided mainly for use with the request and report mode (DECRQM/DECRPM) control
functions.
Set: application sequences.
Reset: keypad characters.
*/
/*
MODE_DECBKM, Backarrow Key Mode
This control function determines whether the Backspace key works as a
backspace key or delete key.
If DECBKM is set, Backspace works as a backspace key. When you press Backspace,
the terminal sends a BS character to the host.
If DECBKM is reset, Backspace works as a delete key. When you press Backspace,
the terminal sends a DEL character to the host.
*/
/*
MODE_DECKPM, Key Position Mode
This control function selects whether the keyboard sends character codes or key
position reports to the host.
*/


#define set_key(str)        \
    *string = str;          \
    *len = strlen (str);


static gboolean get_arrow_key       (MooTerm     *term,
                                     guint        keyval,
                                     const char **string,
                                     guint       *len);
static gboolean get_backspace_key   (MooTerm     *term,
                                     const char **string,
                                     guint       *len);
static gboolean get_delete_key      (MooTerm     *term,
                                     const char **string,
                                     guint       *len);
static gboolean get_keypad_key      (MooTerm     *term,
                                     guint        keyval,
                                     const char **string,
                                     guint       *len);
static gboolean get_vt_ctl_key      (MooTerm     *term,
                                     guint        keyval,
                                     const char **string,
                                     guint       *len);
static gboolean ignore              (guint        keyval);

static gboolean get_vt_key          (MooTerm     *term,
                                     guint        keyval,
                                     const char **string,
                                     guint       *len)
{
    switch (keyval)
    {
        case GDK_Left:
        case GDK_Up:
        case GDK_Right:
        case GDK_Down:
            return get_arrow_key (term, keyval, string, len);

        case GDK_BackSpace:
            return get_backspace_key (term, string, len);
        case GDK_Delete:
            return get_delete_key (term, string, len);

        case GDK_KP_Space:
            return get_vt_key (term, GDK_space, string, len);
        case GDK_KP_Tab:
            return get_vt_key (term, GDK_Tab, string, len);

        case GDK_KP_F1:
        case GDK_KP_F2:
        case GDK_KP_F3:
        case GDK_KP_F4:
        case GDK_KP_Enter:
        case GDK_KP_Home:
        case GDK_KP_Left:
        case GDK_KP_Up:
        case GDK_KP_Right:
        case GDK_KP_Down:
        case GDK_KP_Page_Up:
        case GDK_KP_Page_Down:
        case GDK_KP_End:
        case GDK_KP_Begin:
        case GDK_KP_Insert:
        case GDK_KP_Delete:
        case GDK_KP_Equal:
        case GDK_KP_Multiply:
        case GDK_KP_Add:
        case GDK_KP_Separator:
        case GDK_KP_Subtract:
        case GDK_KP_Decimal:
        case GDK_KP_Divide:
        case GDK_KP_0:
        case GDK_KP_1:
        case GDK_KP_2:
        case GDK_KP_3:
        case GDK_KP_4:
        case GDK_KP_5:
        case GDK_KP_6:
        case GDK_KP_7:
        case GDK_KP_8:
        case GDK_KP_9:
            return get_keypad_key (term, keyval, string, len);

        case GDK_Tab:
            set_key ("\011");
            break;
        case GDK_Linefeed:
            set_key ("\012");
            break;
        case GDK_Clear:
            set_key ("\013");
            break;
        case GDK_Return:
            set_key ("\015");
            break;
        case GDK_Pause:
            set_key ("\023");
            break;
        case GDK_Escape:
            set_key ("\033");
            break;

        /* keys below are from `infocmp xterm` */

        case GDK_Home:
            set_key ("\033OH");
            break;
        case GDK_Page_Up:
            set_key ("\033[5~");
            break;
        case GDK_Page_Down:
            set_key ("\033[6~");
            break;
        case GDK_End:
            set_key ("\033OF");
            break;
        case GDK_Insert:
            set_key ("\033[2~");
            break;
        case GDK_F1:
            set_key ("\033OP");
            break;
        case GDK_F2:
            set_key ("\033OQ");
            break;
        case GDK_F3:
            set_key ("\033OR");
            break;
        case GDK_F4:
            set_key ("\033OS");
            break;
        case GDK_F5:
            set_key ("\033[15~");
            break;
        case GDK_F6:
            set_key ("\033[17~");
            break;
        case GDK_F7:
            set_key ("\033[18~");
            break;
        case GDK_F8:
            set_key ("\033[19~");
            break;
        case GDK_F9:
            set_key ("\033[20~");
            break;
        case GDK_F10:
            set_key ("\033[21~");
            break;
        case GDK_F11:
            set_key ("\033[23~");
            break;
        case GDK_F12:
            set_key ("\033[24~");
            break;
        case GDK_F13:
            set_key ("\033O2P");
            break;
        case GDK_F14:
            set_key ("\033O2Q");
            break;
        case GDK_F15:
            set_key ("\033O2R");
            break;
        case GDK_F16:
            set_key ("\033O2S");
            break;
        case GDK_F17:
            set_key ("\033[15;2~");
            break;
        case GDK_F18:
            set_key ("\033[17;2~");
            break;
        case GDK_F19:
            set_key ("\033[18;2~");
            break;
        case GDK_F20:
            set_key ("\033[19;2~");
            break;
        case GDK_F21:
            set_key ("\033[20;2~");
            break;
        case GDK_F22:
            set_key ("\033[21;2~");
            break;
        case GDK_F23:
            set_key ("\033[23;2~");
            break;
        case GDK_F24:
            set_key ("\033[24;2~");
            break;
        case GDK_F25:
            set_key ("\033O5P");
            break;
        case GDK_F26:
            set_key ("\033O5Q");
            break;
        case GDK_F27:
            set_key ("\033O5R");
            break;
        case GDK_F28:
            set_key ("\033O5S");
            break;
        case GDK_F29:
            set_key ("\033[15;5~");
            break;
        case GDK_F30:
            set_key ("\033[17;5~");
            break;
        case GDK_F31:
            set_key ("\033[18;5~");
            break;
        case GDK_F32:
            set_key ("\033[19;5~");
            break;
        case GDK_F33:
            set_key ("\033[20;5~");
            break;
        case GDK_F34:
            set_key ("\033[21;5~");
            break;
        case GDK_F35:
            set_key ("\033[23;5~");
            break;

        default:
            return FALSE;
    }

    return TRUE;
}


/* VT102 manual */
static gboolean get_arrow_key       (MooTerm     *term,
                                     guint        keyval,
                                     const char **string,
                                     guint       *len)
{
    if (term_get_mode (MODE_DECNKM) && term_get_mode (MODE_DECCKM))
    {
        switch (keyval)
        {
            case GDK_Left:
                set_key ("\033OD");
                break;
            case GDK_Up:
                set_key ("\033OA");
                break;
            case GDK_Right:
                set_key ("\033OC");
                break;
            case GDK_Down:
                set_key ("\033OB");
                break;

            default:
                g_assert_not_reached ();
        }
    }
    else
    {
        switch (keyval)
        {
            case GDK_Left:
                set_key ("\033[D");
                break;
            case GDK_Up:
                set_key ("\033[A");
                break;
            case GDK_Right:
                set_key ("\033[C");
                break;
            case GDK_Down:
                set_key ("\033[B");
                break;

            default:
                g_assert_not_reached ();
        }
    }

    return TRUE;
}


static gboolean get_backspace_key   (MooTerm     *term,
                                     const char **string,
                                     guint       *len)
{
    if (term_get_mode (MODE_DECBKM))
    {
        set_key ("\010");
        return TRUE;
    }
    else
    {
        return get_delete_key (term, string, len);
    }
}


/* TODO */
static gboolean get_delete_key      (G_GNUC_UNUSED MooTerm     *term,
                                     const char **string,
                                     guint       *len)
{
    set_key ("\033[3~");
    return TRUE;
}


/* these are from VT102 manual */
static gboolean get_keypad_key      (MooTerm     *term,
                                     guint        keyval,
                                     const char **string,
                                     guint       *len)
{
    if (!term_get_mode (MODE_DECNKM))
    {
        switch (keyval)
        {
            case GDK_KP_Subtract:
                set_key ("-");
                break;
            case GDK_KP_Equal:
                set_key ("=");
                break;
            case GDK_KP_Multiply:
                set_key ("*");
                break;
            case GDK_KP_Add:
                set_key ("+");
                break;

            case GDK_KP_Divide:
                set_key ("/");
                break;
            case GDK_KP_0:
            case GDK_KP_Insert:
                set_key ("0");
                break;
            case GDK_KP_1:
            case GDK_KP_End:
                set_key ("1");
                break;
            case GDK_KP_2:
            case GDK_KP_Down:
                set_key ("2");
                break;
            case GDK_KP_3:
            case GDK_KP_Page_Down:
                set_key ("3");
                break;
            case GDK_KP_4:
            case GDK_KP_Left:
                set_key ("4");
                break;
            case GDK_KP_5:
                set_key ("5");
                break;
            case GDK_KP_6:
            case GDK_KP_Right:
                set_key ("6");
                break;
            case GDK_KP_7:
            case GDK_KP_Home:
            case GDK_KP_Begin:
                set_key ("7");
                break;
            case GDK_KP_8:
            case GDK_KP_Up:
                set_key ("8");
                break;
            case GDK_KP_9:
            case GDK_KP_Page_Up:
                set_key ("9");
                break;
            case GDK_KP_Decimal:
            case GDK_KP_Delete:
                set_key (".");  /* TODO should it be period/comma depending on locale? */
                break;
            case GDK_KP_Separator:
                set_key (",");  /* TODO should it be period/comma depending on locale? */
                break;
            case GDK_KP_Enter:
                return get_vt_key (term, GDK_Return, string, len);
            case GDK_KP_F1:
                set_key ("\033OP");
                break;
            case GDK_KP_F2:
                set_key ("\033OQ");
                break;
            case GDK_KP_F3:
                set_key ("\033OR");
                break;
            case GDK_KP_F4:
                set_key ("\033OS");
                break;

            default:
                g_assert_not_reached ();
        }
    }
    else
    {
        switch (keyval)
        {
            case GDK_KP_0:
            case GDK_KP_Insert:
                set_key ("\033Op");
                break;
            case GDK_KP_1:
            case GDK_KP_End:
                set_key ("\033Oq");
                break;
            case GDK_KP_2:
            case GDK_KP_Down:
                set_key ("\033Or");
                break;
            case GDK_KP_3:
            case GDK_KP_Page_Down:
                set_key ("\033Os");
                break;
            case GDK_KP_4:
            case GDK_KP_Left:
                set_key ("\033Ot");
                break;
            case GDK_KP_5:
                set_key ("\033Ou");
                break;
            case GDK_KP_6:
            case GDK_KP_Right:
                set_key ("\033Ov");
                break;
            case GDK_KP_7:
            case GDK_KP_Home:
            case GDK_KP_Begin:
                set_key ("\033Ow");
                break;
            case GDK_KP_8:
            case GDK_KP_Up:
                set_key ("\033Ox");
                break;
            case GDK_KP_9:
            case GDK_KP_Page_Up:
                set_key ("\033Oy");
                break;

            case GDK_KP_Subtract:
                set_key ("\033Om");
                break;
            case GDK_KP_Decimal:
            case GDK_KP_Delete:
                set_key ("\033On");  /* TODO should it be period/comma depending on locale? */
                break;
            case GDK_KP_Separator:
                set_key ("\033Ol");  /* TODO should it be period/comma depending on locale? */
                break;

            case GDK_KP_Enter:
                set_key ("\033OM");
                break;

            case GDK_KP_F1:
                set_key ("\033OP");
                break;
            case GDK_KP_F2:
                set_key ("\033OQ");
                break;
            case GDK_KP_F3:
                set_key ("\033OR");
                break;
            case GDK_KP_F4:
                set_key ("\033OS");
                break;

            case GDK_KP_Equal:
                set_key ("=");
                break;
            case GDK_KP_Multiply:
                set_key ("*");
                break;
            case GDK_KP_Add:
                set_key ("+");
                break;
            case GDK_KP_Divide:
                set_key ("/");
                break;

            default:
                g_assert_not_reached ();
        }
    }

    return TRUE;
}


/*
print ''.join(['case GDK_%s:\ncase GDK_%s:\n    set_key ("\\%03o");\n    break;\n' % (chr(i), chr(i+32), i-64) for i in range(ord('A'), ord('Z') + 1)])
*/
static gboolean get_vt_ctl_key      (G_GNUC_UNUSED MooTerm     *term,
                                     guint        keyval,
                                     const char **string,
                                     guint       *len)
{
    switch (keyval)
    {
        case GDK_A:
        case GDK_a:
            set_key ("\001");
            break;
        case GDK_B:
        case GDK_b:
            set_key ("\002");
            break;
        case GDK_C:
        case GDK_c:
            set_key ("\003");
            break;
        case GDK_D:
        case GDK_d:
            set_key ("\004");
            break;
        case GDK_E:
        case GDK_e:
            set_key ("\005");
            break;
        case GDK_F:
        case GDK_f:
            set_key ("\006");
            break;
        case GDK_G:
        case GDK_g:
            set_key ("\007");
            break;
        case GDK_H:
        case GDK_h:
            set_key ("\010");
            break;
        case GDK_I:
        case GDK_i:
            set_key ("\011");
            break;
        case GDK_J:
        case GDK_j:
            set_key ("\012");
            break;
        case GDK_K:
        case GDK_k:
            set_key ("\013");
            break;
        case GDK_L:
        case GDK_l:
            set_key ("\014");
            break;
        case GDK_M:
        case GDK_m:
            set_key ("\015");
            break;
        case GDK_N:
        case GDK_n:
            set_key ("\016");
            break;
        case GDK_O:
        case GDK_o:
            set_key ("\017");
            break;
        case GDK_P:
        case GDK_p:
            set_key ("\020");
            break;
        case GDK_Q:
        case GDK_q:
            set_key ("\021");
            break;
        case GDK_R:
        case GDK_r:
            set_key ("\022");
            break;
        case GDK_S:
        case GDK_s:
            set_key ("\023");
            break;
        case GDK_T:
        case GDK_t:
            set_key ("\024");
            break;
        case GDK_U:
        case GDK_u:
            set_key ("\025");
            break;
        case GDK_V:
        case GDK_v:
            set_key ("\026");
            break;
        case GDK_W:
        case GDK_w:
            set_key ("\027");
            break;
        case GDK_X:
        case GDK_x:
            set_key ("\030");
            break;
        case GDK_Y:
        case GDK_y:
            set_key ("\031");
            break;
        case GDK_Z:
        case GDK_z:
            set_key ("\032");
            break;

        case GDK_space:
            *string = "\000";
            *len = 1;
            break;

        case GDK_bracketleft:
        case GDK_braceleft:
            set_key ("\033");
            break;
        case GDK_backslash:
        case GDK_bar:
            set_key ("\034");
            break;
        case GDK_bracketright:
        case GDK_braceright:
            set_key ("\035");
            break;
        case GDK_asciitilde:
        case GDK_quoteleft:
            set_key ("\036");
            break;
        case GDK_question:
        case GDK_slash:
            set_key ("\037");
            break;

        default:
            return FALSE;
    }

    return TRUE;
}


static gboolean ignore              (guint        keyval)
{
    switch (keyval)
    {
        case GDK_VoidSymbol:
        case GDK_Menu:

        case GDK_Alt_L:
        case GDK_Alt_R:
        case GDK_Caps_Lock:
        case GDK_Control_L:
        case GDK_Control_R:
        case GDK_Eisu_Shift:
        case GDK_Hyper_L:
        case GDK_Hyper_R:
        case GDK_ISO_First_Group_Lock:
        case GDK_ISO_Group_Lock:
        case GDK_ISO_Group_Shift:
        case GDK_ISO_Last_Group_Lock:
        case GDK_ISO_Level3_Lock:
        case GDK_ISO_Level3_Shift:
        case GDK_ISO_Lock:
        case GDK_ISO_Next_Group_Lock:
        case GDK_ISO_Prev_Group_Lock:
        case GDK_Kana_Lock:
        case GDK_Kana_Shift:
        case GDK_Meta_L:
        case GDK_Meta_R:
        case GDK_Num_Lock:
        case GDK_Scroll_Lock:
        case GDK_Shift_L:
        case GDK_Shift_Lock:
        case GDK_Shift_R:
        case GDK_Super_L:
        case GDK_Super_R:
            return TRUE;

        default:
            return FALSE;
    }
}


#endif /* MOOTERM_MOOTERM_KEYMAP_H */
