/*
 *   mooterm-keymap.h
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

#ifndef __MOO_TERM_KEYMAP_H__
#define __MOO_TERM_KEYMAP_H__

#include <gdk/gdkkeysyms.h>
#include <string.h>
#include "mooterm/mooterm-private.h"
#include "mooterm/mootermpt.h"

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


static void get_vt_key  (MooTerm     *term,
                         guint        keyval,
                         char       **string,
                         guint       *len);



#define SET_KEY(str)            \
    *string = g_strdup (str);   \
    *len = strlen (str);


#define CASE_GDK_KP_SOMETHING   \
    case GDK_KP_F1:             \
    case GDK_KP_F2:             \
    case GDK_KP_F3:             \
    case GDK_KP_F4:             \
    case GDK_KP_Enter:          \
    case GDK_KP_Home:           \
    case GDK_KP_Left:           \
    case GDK_KP_Up:             \
    case GDK_KP_Right:          \
    case GDK_KP_Down:           \
    case GDK_KP_Page_Up:        \
    case GDK_KP_Page_Down:      \
    case GDK_KP_End:            \
    case GDK_KP_Begin:          \
    case GDK_KP_Insert:         \
    case GDK_KP_Delete:         \
    case GDK_KP_Separator:      \
    case GDK_KP_Subtract:       \
    case GDK_KP_Decimal:        \
    case GDK_KP_0:              \
    case GDK_KP_1:              \
    case GDK_KP_2:              \
    case GDK_KP_3:              \
    case GDK_KP_4:              \
    case GDK_KP_5:              \
    case GDK_KP_6:              \
    case GDK_KP_7:              \
    case GDK_KP_8:              \
    case GDK_KP_9:


/* these are from VT102 manual */
static void get_keypad_key  (MooTerm     *term,
                             guint        keyval,
                             char       **string,
                             guint       *len)
{
    if (!term_get_mode (MODE_DECNKM))
    {
        switch (keyval)
        {
            case GDK_KP_Enter:
                get_vt_key (term, GDK_Return, string, len);
                break;
            case GDK_KP_Subtract:
                SET_KEY ("-");
                break;
            case GDK_KP_0:
            case GDK_KP_Insert:
                SET_KEY ("0");
                break;
            case GDK_KP_1:
            case GDK_KP_End:
                SET_KEY ("1");
                break;
            case GDK_KP_2:
            case GDK_KP_Down:
                SET_KEY ("2");
                break;
            case GDK_KP_3:
            case GDK_KP_Page_Down:
                SET_KEY ("3");
                break;
            case GDK_KP_4:
            case GDK_KP_Left:
                SET_KEY ("4");
                break;
            case GDK_KP_5:
                SET_KEY ("5");
                break;
            case GDK_KP_6:
            case GDK_KP_Right:
                SET_KEY ("6");
                break;
            case GDK_KP_7:
            case GDK_KP_Home:
            case GDK_KP_Begin:
                SET_KEY ("7");
                break;
            case GDK_KP_8:
            case GDK_KP_Up:
                SET_KEY ("8");
                break;
            case GDK_KP_9:
            case GDK_KP_Page_Up:
                SET_KEY ("9");
                break;
            case GDK_KP_Decimal:
            case GDK_KP_Delete:
                SET_KEY (".");  /* TODO should it be period/comma depending on locale? */
                break;
            case GDK_KP_Separator:
                SET_KEY (",");  /* TODO should it be period/comma depending on locale? */
                break;
            case GDK_KP_F1:
                SET_KEY ("\033OP");
                break;
            case GDK_KP_F2:
                SET_KEY ("\033OQ");
                break;
            case GDK_KP_F3:
                SET_KEY ("\033OR");
                break;
            case GDK_KP_F4:
                SET_KEY ("\033OS");
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
                SET_KEY ("\033Op");
                break;
            case GDK_KP_1:
            case GDK_KP_End:
                SET_KEY ("\033Oq");
                break;
            case GDK_KP_2:
            case GDK_KP_Down:
                SET_KEY ("\033Or");
                break;
            case GDK_KP_3:
            case GDK_KP_Page_Down:
                SET_KEY ("\033Os");
                break;
            case GDK_KP_4:
            case GDK_KP_Left:
                SET_KEY ("\033Ot");
                break;
            case GDK_KP_5:
                SET_KEY ("\033Ou");
                break;
            case GDK_KP_6:
            case GDK_KP_Right:
                SET_KEY ("\033Ov");
                break;
            case GDK_KP_7:
            case GDK_KP_Home:
            case GDK_KP_Begin:
                SET_KEY ("\033Ow");
                break;
            case GDK_KP_8:
            case GDK_KP_Up:
                SET_KEY ("\033Ox");
                break;
            case GDK_KP_9:
            case GDK_KP_Page_Up:
                SET_KEY ("\033Oy");
                break;

            case GDK_KP_Subtract:
                SET_KEY ("\033Om");
                break;
            case GDK_KP_Decimal:
            case GDK_KP_Delete:
                SET_KEY ("\033On");  /* TODO should it be period/comma depending on locale? */
                break;
            case GDK_KP_Separator:
                SET_KEY ("\033Ol");  /* TODO should it be period/comma depending on locale? */
                break;

            case GDK_KP_Enter:
                SET_KEY ("\033OM");
                break;

            case GDK_KP_F1:
                SET_KEY ("\033OP");
                break;
            case GDK_KP_F2:
                SET_KEY ("\033OQ");
                break;
            case GDK_KP_F3:
                SET_KEY ("\033OR");
                break;
            case GDK_KP_F4:
                SET_KEY ("\033OS");
                break;

            default:
                g_assert_not_reached ();
        }
    }
}


/* VT102 manual */
static void get_arrow_key   (MooTerm    *term,
                             guint       keyval,
                             char      **string,
                             guint      *len)
{
    if (term_get_mode (MODE_DECNKM) && term_get_mode (MODE_DECCKM))
    {
        switch (keyval)
        {
            case GDK_Left:
                SET_KEY ("\033OD");
                break;
            case GDK_Up:
                SET_KEY ("\033OA");
                break;
            case GDK_Right:
                SET_KEY ("\033OC");
                break;
            case GDK_Down:
                SET_KEY ("\033OB");
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
                SET_KEY ("\033[D");
                break;
            case GDK_Up:
                SET_KEY ("\033[A");
                break;
            case GDK_Right:
                SET_KEY ("\033[C");
                break;
            case GDK_Down:
                SET_KEY ("\033[B");
                break;

            default:
                g_assert_not_reached ();
        }
    }
}


/*
print ''.join(['case GDK_%s:\ncase GDK_%s:\n    set_key ("\\%03o");\n    break;\n' % (chr(i), chr(i+32), i-64) for i in range(ord('A'), ord('Z') + 1)])
*/
static void get_vt_ctl_key  (G_GNUC_UNUSED MooTerm     *term,
                             guint        keyval,
                             char       **string,
                             guint       *len)
{
    switch (keyval)
    {
        case GDK_A:
        case GDK_a:
            SET_KEY ("\001");
            break;
        case GDK_B:
        case GDK_b:
            SET_KEY ("\002");
            break;
        case GDK_C:
        case GDK_c:
            SET_KEY ("\003");
            break;
        case GDK_D:
        case GDK_d:
            SET_KEY ("\004");
            break;
        case GDK_E:
        case GDK_e:
            SET_KEY ("\005");
            break;
        case GDK_F:
        case GDK_f:
            SET_KEY ("\006");
            break;
        case GDK_G:
        case GDK_g:
            SET_KEY ("\007");
            break;
        case GDK_H:
        case GDK_h:
            SET_KEY ("\010");
            break;
        case GDK_I:
        case GDK_i:
            SET_KEY ("\011");
            break;
        case GDK_J:
        case GDK_j:
            SET_KEY ("\012");
            break;
        case GDK_K:
        case GDK_k:
            SET_KEY ("\013");
            break;
        case GDK_L:
        case GDK_l:
            SET_KEY ("\014");
            break;
        case GDK_M:
        case GDK_m:
            SET_KEY ("\015");
            break;
        case GDK_N:
        case GDK_n:
            SET_KEY ("\016");
            break;
        case GDK_O:
        case GDK_o:
            SET_KEY ("\017");
            break;
        case GDK_P:
        case GDK_p:
            SET_KEY ("\020");
            break;
        case GDK_Q:
        case GDK_q:
            SET_KEY ("\021");
            break;
        case GDK_R:
        case GDK_r:
            SET_KEY ("\022");
            break;
        case GDK_S:
        case GDK_s:
            SET_KEY ("\023");
            break;
        case GDK_T:
        case GDK_t:
            SET_KEY ("\024");
            break;
        case GDK_U:
        case GDK_u:
            SET_KEY ("\025");
            break;
        case GDK_V:
        case GDK_v:
            SET_KEY ("\026");
            break;
        case GDK_W:
        case GDK_w:
            SET_KEY ("\027");
            break;
        case GDK_X:
        case GDK_x:
            SET_KEY ("\030");
            break;
        case GDK_Y:
        case GDK_y:
            SET_KEY ("\031");
            break;
        case GDK_Z:
        case GDK_z:
            SET_KEY ("\032");
            break;

        case GDK_space:
            *string = g_strdup ("\000");
            *len = 1;
            break;

        case GDK_bracketleft:
        case GDK_braceleft:
            SET_KEY ("\033");
            break;
        case GDK_backslash:
        case GDK_bar:
            SET_KEY ("\034");
            break;
        case GDK_bracketright:
        case GDK_braceright:
            SET_KEY ("\035");
            break;
        case GDK_asciitilde:
        case GDK_quoteleft:
            SET_KEY ("\036");
            break;
        case GDK_question:
        case GDK_slash:
            SET_KEY ("\037");
            break;
    }
}


static void get_backspace_key   (MooTerm  *term,
                                 char    **normal,
                                 guint    *normal_length,
                                 gboolean *suppress_meta_esc)
{
    char c;

    switch (term->priv->settings.backspace_binding)
    {
        case MOO_TERM_ERASE_ASCII_BACKSPACE:
            *normal = g_strdup ("\010");
            *normal_length = 1;
            *suppress_meta_esc = FALSE;
            break;

        case MOO_TERM_ERASE_ASCII_DELETE:
            *normal = g_strdup ("\177");
            *normal_length = 1;
            *suppress_meta_esc = FALSE;
            break;

        case MOO_TERM_ERASE_DELETE_SEQUENCE:
            *normal = g_strdup ("\033[3~");
            *normal_length = 4;
            *suppress_meta_esc = TRUE;
            break;

            /* Use the tty's erase character. */
        case MOO_TERM_ERASE_AUTO:
        default:
            c = _moo_term_pt_get_erase_char (term->priv->pt);
            if (c)
            {
                *normal = g_strdup_printf("%c", c);
                *normal_length = 1;
                *suppress_meta_esc = FALSE;
            }
            else
            {
                *normal = g_strdup ("\010");
                *normal_length = 1;
                *suppress_meta_esc = FALSE;
            }
            break;
    }
}


static void get_delete_key      (MooTerm  *term,
                                 char    **normal,
                                 guint    *normal_length,
                                 gboolean *suppress_meta_esc)
{
    switch (term->priv->settings.delete_binding)
    {
        case MOO_TERM_ERASE_ASCII_BACKSPACE:
            *normal = g_strdup("\010");
            *normal_length = 1;
            break;

        case MOO_TERM_ERASE_ASCII_DELETE:
            *normal = g_strdup("\177");
            *normal_length = 1;
            break;

        case MOO_TERM_ERASE_DELETE_SEQUENCE:
        case MOO_TERM_ERASE_AUTO:
        default:
            *normal = g_strdup ("\033[3~");
            *normal_length = 4;
            *suppress_meta_esc = TRUE;
            break;
    }
}


static void get_vt_key  (MooTerm     *term,
                         guint        keyval,
                         char       **string,
                         guint       *len)
{
    switch (keyval)
    {
        case GDK_Left:
        case GDK_Up:
        case GDK_Right:
        case GDK_Down:
            get_arrow_key (term, keyval, string, len);
            break;

        CASE_GDK_KP_SOMETHING
            get_keypad_key (term, keyval, string, len);
            break;

        case GDK_Tab:
        case GDK_KP_Tab:
            SET_KEY ("\011");
            break;
        case GDK_Linefeed:
            SET_KEY ("\012");
            break;
        case GDK_Clear:
            SET_KEY ("\013");
            break;
        case GDK_Return:
            SET_KEY ("\015");
            break;
        case GDK_Pause:
            SET_KEY ("\023");
            break;
        case GDK_Escape:
            SET_KEY ("\033");
            break;

        /* keys below are from `infocmp xterm` */

        case GDK_Home:
            SET_KEY ("\033OH");
            break;
        case GDK_Page_Up:
            SET_KEY ("\033[5~");
            break;
        case GDK_Page_Down:
            SET_KEY ("\033[6~");
            break;
        case GDK_End:
            SET_KEY ("\033OF");
            break;
        case GDK_Insert:
            SET_KEY ("\033[2~");
            break;
        case GDK_F1:
            SET_KEY ("\033OP");
            break;
        case GDK_F2:
            SET_KEY ("\033OQ");
            break;
        case GDK_F3:
            SET_KEY ("\033OR");
            break;
        case GDK_F4:
            SET_KEY ("\033OS");
            break;
        case GDK_F5:
            SET_KEY ("\033[15~");
            break;
        case GDK_F6:
            SET_KEY ("\033[17~");
            break;
        case GDK_F7:
            SET_KEY ("\033[18~");
            break;
        case GDK_F8:
            SET_KEY ("\033[19~");
            break;
        case GDK_F9:
            SET_KEY ("\033[20~");
            break;
        case GDK_F10:
            SET_KEY ("\033[21~");
            break;
        case GDK_F11:
            SET_KEY ("\033[23~");
            break;
        case GDK_F12:
            SET_KEY ("\033[24~");
            break;
        case GDK_F13:
            SET_KEY ("\033O2P");
            break;
        case GDK_F14:
            SET_KEY ("\033O2Q");
            break;
        case GDK_F15:
            SET_KEY ("\033O2R");
            break;
        case GDK_F16:
            SET_KEY ("\033O2S");
            break;
        case GDK_F17:
            SET_KEY ("\033[15;2~");
            break;
        case GDK_F18:
            SET_KEY ("\033[17;2~");
            break;
        case GDK_F19:
            SET_KEY ("\033[18;2~");
            break;
        case GDK_F20:
            SET_KEY ("\033[19;2~");
            break;
        case GDK_F21:
            SET_KEY ("\033[20;2~");
            break;
        case GDK_F22:
            SET_KEY ("\033[21;2~");
            break;
        case GDK_F23:
            SET_KEY ("\033[23;2~");
            break;
        case GDK_F24:
            SET_KEY ("\033[24;2~");
            break;
        case GDK_F25:
            SET_KEY ("\033O5P");
            break;
        case GDK_F26:
            SET_KEY ("\033O5Q");
            break;
        case GDK_F27:
            SET_KEY ("\033O5R");
            break;
        case GDK_F28:
            SET_KEY ("\033O5S");
            break;
        case GDK_F29:
            SET_KEY ("\033[15;5~");
            break;
        case GDK_F30:
            SET_KEY ("\033[17;5~");
            break;
        case GDK_F31:
            SET_KEY ("\033[18;5~");
            break;
        case GDK_F32:
            SET_KEY ("\033[19;5~");
            break;
        case GDK_F33:
            SET_KEY ("\033[20;5~");
            break;
        case GDK_F34:
            SET_KEY ("\033[21;5~");
            break;
        case GDK_F35:
            SET_KEY ("\033[23;5~");
            break;
    }
}


static gboolean key_is_modifier (guint  keyval)
{
    switch (keyval)
    {
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


#endif /* __MOO_TERM_KEYMAP_H__ */
