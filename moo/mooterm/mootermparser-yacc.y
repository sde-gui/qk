%{
#define MOOTERM_COMPILATION
#include "mooterm/mootermparser.h"

#define add_number(num)                             \
{                                                   \
    if (!parser->nums_len)                          \
    {                                               \
        parser->nums[0] = num;                      \
        parser->nums_len = 1;                       \
    }                                               \
    else                                            \
    {                                               \
        if (parser->nums_len >= MAX_NUMS_LEN)       \
        {                                           \
            g_warning ("%s: too big number of "     \
                       "arguments, discarding",     \
                       G_STRLOC);                   \
            YYABORT;                                \
        }                                           \
        parser->nums[parser->nums_len++] = num;     \
    }                                               \
}

#define add_char(c)                                 \
{                                                   \
    if (!parser->string_len)                        \
    {                                               \
        parser->string[0] = c;                      \
        parser->string_len = 1;                     \
    }                                               \
    else                                            \
    {                                               \
        if (parser->string_len >= MAX_ESC_SEQ_LEN)  \
        {                                           \
            g_warning ("%s: string too long, "      \
                       "discarding", G_STRLOC);     \
            YYABORT;                                \
        }                                           \
        parser->string[parser->string_len++] = c;   \
    }                                               \
}

#define set_cmd(cmd_code)                           \
{                                                   \
    parser->cmd = cmd_code;                         \
    YYACCEPT;                                       \
}

#define check_nums_len(cmd, n)                      \
{                                                   \
    if (parser->nums_len != n)                      \
    {                                               \
        g_warning ("%s: invalid number of "         \
                   "arguments for " #cmd            \
                   " command", G_STRLOC);           \
        YYABORT;                                    \
    }                                               \
}

#define check_one(cmd, num, num1)                   \
{                                                   \
    if (num != num1)                                \
    {                                               \
        g_warning ("%s: invalid argument %d"        \
                   " for " #cmd " command",         \
                   G_STRLOC, num);                  \
        YYABORT;                                    \
    }                                               \
}

#define check_range(cmd, num, num1, num2)           \
{                                                   \
    if (num < num1 || num > num2)                   \
    {                                               \
        g_warning ("%s: invalid argument %d"        \
                   " for " #cmd " command",         \
                   G_STRLOC, num);                  \
        YYABORT;                                    \
    }                                               \
}

#define check_nums_2(cmd, num1, num2)               \
{                                                   \
    if (parser->nums_len != 2 ||                    \
        parser->nums[0] != num1 ||                  \
        parser->nums[1] != num2)                    \
    {                                               \
        g_warning ("%s: invalid arguments"          \
                   " for " #cmd " command",         \
                   G_STRLOC);                       \
        YYABORT;                                    \
    }                                               \
}

#define push_one_back()                             \
{                                                   \
    g_assert (!iter_is_start (&parser->current));   \
    iter_backward (&parser->current);               \
    iter_backward (&parser->cmd_string.end);        \
}

%}

%name-prefix="_moo_term_yy"

%lex-param      {MooTermParser *parser}
%parse-param    {MooTermParser *parser}

%token  DIGIT

%%

command:            vt100
                |   vt102
                |   vt220
                |   Edward_Moy
                |   Skip_Montanaro
                |   terminfo
;

/* Xterm Control Sequences by Edward Moy */
Edward_Moy:         G2_charset
                |   G3_charset
                |   DCS
                |   init_hilite_mouse_tracking
                |   restore_DECSET
                |   save_DECSET
                |   set_text
                |   PM
                |   APC
                |   memory_lock
                |   memory_unlock
                |   LS2
                |   LS3
                |   LS3R
                |   LS1R
;

/* From: Skip Montanaro
   To: xpert@expo.lcs.mit.edu
   Subject: XTerm Escape Sequences (X11 Version)
 */
Skip_Montanaro:     BELL
                |   BACKSPACE
                |   TAB
                |   LINEFEED
                |   VERT_TAB
                |   FORM_FEED
                |   CARRIAGE_RETURN
                |   ALT_CHARSET
                |   NORM_CHARSET
                |   track_mouse
;

terminfo:           cbt
                |   ech
                |   hpa
                |   rs2
                |   u6
                |   u8
                |   vpa
;

vt100:              CUB
                |   CUD
                |   CUF
                |   CUP
                |   CUU
                |   DA
                |   DECALN_or_line_size
                |   DECID
                |   DECKPAM
                |   DECKPNM
                |   DECLL
                |   DECSC_DECRC
                |   DECREQTPARM
                |   DECSTBM
                |   DECTST
                |   DSR
                |   ED
                |   EL
                |   HTS
                |   HVP
                |   IND
                |   NEL
                |   RI
                |   RIS
                |   RM
                |   SCS
                |   SGR
                |   SM
                |   TBC
;

vt102:              MC              /* Media Copy - printer stuff - ignored */
                |   SS2
                |   SS3
;

vt220:              ICH
                |   DCH
                |   IL
                |   DL
;

/****************************************************************************/
/* vt100
 */

DECALN_or_line_size:    '#' number          {   push_one_back ();
                                                add_number ($2);
                                                switch ($2)
                                                {
                                                    case 3:
                                                    case 4:
                                                        set_cmd (CMD_DECDHL);
                                                    case 6:
                                                        set_cmd (CMD_DECDWL);
                                                    case 5:
                                                        set_cmd (CMD_DECSWL);
                                                    case 8:
                                                        set_cmd (CMD_DECALN);
                                                    default:
                                                        g_warning ("%s: invalid argument %d",
                                                                   G_STRLOC, $2);
                                                        YYABORT;
                                                }
                                            }
;
DECLL:          '[' number 'q'              {   add_number ($2); set_cmd (CMD_DECLL); }
;
SCS:            G0_charset
            |   G1_charset
;
DECTST:         '[' numbers 'y'             {   check_nums_len (DECTST, 2); set_cmd (CMD_DECTST); }
;

SM:             '[' numbers 'h'             {   set_cmd (CMD_SM); }
            |   '[' '?' numbers 'h'         {   set_cmd (CMD_DECSET); }
;
RM:             '[' numbers 'l'             {   set_cmd (CMD_RM); }
            |   '[' '?' numbers 'l'         {   set_cmd (CMD_DECRST); }
;
G0_charset:     '(' number                  {   push_one_back ();
                                                check_range (G0_charset, $2, 0, 2);
                                                add_number (0);
                                                set_cmd (CMD_G0_CHARSET); }
            |   '(' 'A'                     {   add_number (3);
                                                set_cmd (CMD_G0_CHARSET); }
            |   '(' 'B'                     {   add_number (4);
                                                set_cmd (CMD_G0_CHARSET); }
;
G1_charset:     ')' number                  {   push_one_back ();
                                                check_range (G1_charset, $2, 0, 2);
                                                add_number (0);
                                                set_cmd (CMD_G1_CHARSET); }
            |   ')' 'A'                     {   add_number (3);
                                                set_cmd (CMD_G1_CHARSET); }
            |   ')' 'B'                     {   add_number (4);
                                                set_cmd (CMD_G1_CHARSET); }
;
DECSC_DECRC:    number                      {   push_one_back ();
                                                check_range (DECSC_DECRC, $1, 7, 8);
                                                if ($1 == 7)
                                                {
                                                    set_cmd (CMD_DECSC);
                                                }
                                                else
                                                {
                                                    set_cmd (CMD_DECRC);
                                                }
                                            }
;
DECKPAM:        '='                         {   set_cmd (CMD_DECKPAM); }
;
DECKPNM:        '>'                         {   set_cmd (CMD_DECKPNM); }
;
IND:            'D'                         {   set_cmd (CMD_IND); }
;
NEL:            'E'                         {   set_cmd (CMD_NEL); }
;
HTS:            'H'                         {   set_cmd (CMD_HTS); }
;
RI:             'M'                         {   set_cmd (CMD_RI); }
;
DECID:          'Z'                         {   set_cmd (CMD_DA); }
;
CUU:            '[' number 'A'              {   add_number ($2); set_cmd (CMD_CUU); }
            |   '[' 'A'                     {   add_number (1); set_cmd (CMD_CUU); }
;
CUD:            '[' number 'B'              {   add_number ($2); set_cmd (CMD_CUD); }
            |   '[' 'B'                     {   add_number (1); set_cmd (CMD_CUD); }
;
CUF:            '[' number 'C'              {   add_number ($2); set_cmd (CMD_CUF); }
            |   '[' 'C'                     {   add_number (1); set_cmd (CMD_CUF); }
;
CUB:            '[' number 'D'              {   add_number ($2); set_cmd (CMD_CUB); }
            |   '[' 'D'                     {   add_number (1); set_cmd (CMD_CUB); }
;
CUP:            '[' numbers 'H'             {   check_nums_len (CUP, 2); set_cmd (CMD_CUP); }
            |   '[' 'H'                     {   add_number (1); add_number (1); set_cmd (CMD_CUP); }
;
ED:             '[' number 'J'              {   add_number ($2); set_cmd (CMD_ED); }
            |   '[' 'J'                     {   add_number (0); set_cmd (CMD_ED); }
;
EL:             '[' number 'K'              {   add_number ($2); set_cmd (CMD_EL); }
            |   '[' 'K'                     {   add_number (0); set_cmd (CMD_EL); }
;
DA:             '[' number 'c'              {   check_one (DA, $2, 0); set_cmd (CMD_DA); }
            |   '[' 'c'                     {   set_cmd (CMD_DA); }
;
HVP:            '[' numbers 'f'             {   check_nums_len (HVP, 2); set_cmd (CMD_HVP); }
            |   '[' 'f'                     {   add_number (1); add_number (1); set_cmd (CMD_HVP); }
;
TBC:            '[' number 'g'              {   add_number ($2); set_cmd (CMD_TBC); }
            |   '[' 'g'                     {   add_number (0); set_cmd (CMD_TBC); }
;
SGR:            '[' numbers 'm'             {   set_cmd (CMD_SGR); }
            |   '[' 'm'                     {   add_number (0); set_cmd (CMD_SGR); }
;
DSR:            '[' numbers 'n'             {   set_cmd (CMD_DSR); }
;
DECSTBM:        '[' numbers 'r'             {   check_nums_len (DECSTBM, 2); set_cmd (CMD_DECSTBM); }
            |   '[' 'r'                     {   add_number (0); add_number (1000000); set_cmd (CMD_DECSTBM); }
;
DECREQTPARM:    '[' number 'x'              {   add_number ($2); set_cmd (CMD_DECREQTPARM); }
;
RIS:            'c'                         {   set_cmd (CMD_RIS); }
;

/****************************************************************************/
/* vt102
 */

MC:             '[' '?' number 'i'          {   set_cmd (CMD_MC); }
            |   '[' number 'i'              {   set_cmd (CMD_MC); }
            |   '[' 'i'                     {   set_cmd (CMD_MC); }
;
SS2:            'N'                         {   set_cmd (CMD_SS2); }
;
SS3:            'O'                         {   set_cmd (CMD_SS3); }
;

/****************************************************************************/
/* vt220
 */

ICH:            '[' number '@'              {   add_number ($2); set_cmd (CMD_ICH); }
            |   '[' '@'                     {   add_number (1); set_cmd (CMD_ICH); }
;
DCH:            '[' number 'P'              {   add_number ($2); set_cmd (CMD_DCH); }
            |   '[' 'P'                     {   add_number (1); set_cmd (CMD_DCH); }
;
IL:             '[' number 'L'              {   add_number ($2); set_cmd (CMD_IL); }
            |   '[' 'L'                     {   add_number (1); set_cmd (CMD_IL); }
;
DL:             '[' number 'M'              {   add_number ($2); set_cmd (CMD_DL); }
            |   '[' 'M'                     {   add_number (1); set_cmd (CMD_DL); }
;

/****************************************************************************/
/* numbers
 */

number:         DIGIT               { $$ = $1; }
            |   number DIGIT        { $$ = $1 * 10 + $2; }
;

numbers:        number              { add_number ($1); }
            |   numbers ';' number  { add_number ($3); }
;


/****************************************************************************/
/* Skip_Montanaro
 */

BELL:           '\007'                      {   set_cmd (CMD_BELL); }
            |   '#' '\007'                  {   set_cmd (CMD_BELL); }
            |   '(' '\007'                  {   set_cmd (CMD_BELL); }
            |   '[' '\007'                  {   set_cmd (CMD_BELL); }
            |   '[' '?' '\007'              {   set_cmd (CMD_BELL); }
;
BACKSPACE:      '\010'                      {   set_cmd (CMD_BACKSPACE); }
            |   '#' '\010'                  {   set_cmd (CMD_BACKSPACE); }
            |   '(' '\010'                  {   set_cmd (CMD_BACKSPACE); }
            |   '[' '\010'                  {   set_cmd (CMD_BACKSPACE); }
            |   '[' '?' '\010'              {   set_cmd (CMD_BACKSPACE); }
;
TAB:            '\011'                      {   set_cmd (CMD_TAB); }
            |   '#' '\011'                  {   set_cmd (CMD_TAB); }
            |   '(' '\011'                  {   set_cmd (CMD_TAB); }
            |   '[' '\011'                  {   set_cmd (CMD_TAB); }
            |   '[' '?' '\011'              {   set_cmd (CMD_TAB); }
;
LINEFEED:       '\012'                      {   set_cmd (CMD_LINEFEED); }
            |   '#' '\012'                  {   set_cmd (CMD_LINEFEED); }
            |   '(' '\012'                  {   set_cmd (CMD_LINEFEED); }
            |   '[' '\012'                  {   set_cmd (CMD_LINEFEED); }
            |   '[' '?' '\012'              {   set_cmd (CMD_LINEFEED); }
;
VERT_TAB:       '\013'                      {   set_cmd (CMD_VERT_TAB); }
            |   '#' '\013'                  {   set_cmd (CMD_VERT_TAB); }
            |   '(' '\013'                  {   set_cmd (CMD_VERT_TAB); }
            |   '[' '\013'                  {   set_cmd (CMD_VERT_TAB); }
            |   '[' '?' '\013'              {   set_cmd (CMD_VERT_TAB); }
;
FORM_FEED:      '\014'                      {   set_cmd (CMD_FORM_FEED); }
            |   '#' '\014'                  {   set_cmd (CMD_FORM_FEED); }
            |   '(' '\014'                  {   set_cmd (CMD_FORM_FEED); }
            |   '[' '\014'                  {   set_cmd (CMD_FORM_FEED); }
            |   '[' '?' '\014'              {   set_cmd (CMD_FORM_FEED); }
;
CARRIAGE_RETURN:'\015'                      {   set_cmd (CMD_CARRIAGE_RETURN); }
            |   '#' '\015'                  {   set_cmd (CMD_CARRIAGE_RETURN); }
            |   '(' '\015'                  {   set_cmd (CMD_CARRIAGE_RETURN); }
            |   '[' '\015'                  {   set_cmd (CMD_CARRIAGE_RETURN); }
            |   '[' '?' '\015'              {   set_cmd (CMD_CARRIAGE_RETURN); }
;
ALT_CHARSET:    '\016'                      {   set_cmd (CMD_ALT_CHARSET); }
            |   '#' '\016'                  {   set_cmd (CMD_ALT_CHARSET); }
            |   '(' '\016'                  {   set_cmd (CMD_ALT_CHARSET); }
            |   '[' '\016'                  {   set_cmd (CMD_ALT_CHARSET); }
            |   '[' '?' '\016'              {   set_cmd (CMD_ALT_CHARSET); }
;
NORM_CHARSET:   '\017'                      {   set_cmd (CMD_NORM_CHARSET); }
            |   '#' '\017'                  {   set_cmd (CMD_NORM_CHARSET); }
            |   '(' '\017'                  {   set_cmd (CMD_NORM_CHARSET); }
            |   '[' '\017'                  {   set_cmd (CMD_NORM_CHARSET); }
            |   '[' '?' '\017'              {   set_cmd (CMD_NORM_CHARSET); }
;
track_mouse:    '[' 'T'                     {   set_cmd (CMD_TRACK_MOUSE); }
;


/****************************************************************************/
/* Edward_Moy
 */

G2_charset:     '*' DIGIT                   {   check_range (G2_charset, $2, 0, 2);
                                                add_number (0);
                                                set_cmd (CMD_G2_CHARSET); }
            |   '*' 'A'                     {   add_number (3);
                                                set_cmd (CMD_G2_CHARSET); }
            |   '*' 'B'                     {   add_number (4);
                                                set_cmd (CMD_G2_CHARSET); }
;
G3_charset:     '+' DIGIT                   {   check_range (G3_charset, $2, 0, 2);
                                                add_number (0);
                                                set_cmd (CMD_G3_CHARSET); }
            |   '+' 'A'                     {   add_number (3);
                                                set_cmd (CMD_G3_CHARSET); }
            |   '+' 'B'                     {   add_number (4);
                                                set_cmd (CMD_G3_CHARSET); }
;
DCS:            'P' no_escape_string '\033' '\\'
                                            {   set_cmd (CMD_DCS); }
            |   'P' '\033' '\\'             {   set_cmd (CMD_DCS); }
;
init_hilite_mouse_tracking: '[' numbers 'T' {   check_nums_len (init_hilite_mouse_tracking, 5);
                                                set_cmd (CMD_INIT_HILITE_MOUSE_TRACKING); }
;
restore_DECSET: '[' '?' numbers 'r'         {   set_cmd (CMD_RESTORE_DECSET); }
;
save_DECSET:    '[' '?' numbers 's'         {   set_cmd (CMD_SAVE_DECSET); }
;
set_text:       ']' number ';' printable_string non_printable_char
                                            {   add_number ($2); set_cmd (CMD_SET_TEXT); }
            |   ']' number ';' non_printable_char
                                            {   add_number ($2); set_cmd (CMD_SET_TEXT); }
;
PM:             '^' no_escape_string '\033' '\\'
                                            {   set_cmd (CMD_PM); }
            |   '^' '\033' '\\'             {   set_cmd (CMD_PM); }
;
APC:            '_' no_escape_string '\033' '\\'
                                            {   set_cmd (CMD_APC); }
            |   '_' '\033' '\\'             {   set_cmd (CMD_APC); }
;
memory_lock:    'l'                         {   set_cmd (CMD_MEMORY_LOCK); }
;
memory_unlock:  'm'                         {   set_cmd (CMD_MEMORY_UNLOCK); }
;
LS2:            'n'                         {   set_cmd (CMD_LS2); }
;
LS3:            'o'                         {   set_cmd (CMD_LS3); }
;
LS3R:           '|'                         {   set_cmd (CMD_LS3R); }
;
LS1R:           '~'                         {   set_cmd (CMD_LS1R); }
;

/****************************************************************************/
/* terminfo escape sequences
 */

cbt:        '[' 'Z'                     { set_cmd (CMD_BACK_TAB); }
;
hpa:        '[' number 'G'              { add_number ($2);
                                          set_cmd (CMD_COLUMN_ADDRESS); }
                                          ;
vpa:        '[' number 'd'              { add_number ($2);
                                          set_cmd (CMD_ROW_ADDRESS); }
                                          ;
ech:        '[' number 'X'              { add_number ($2);
                                          set_cmd (CMD_BACK_TAB); }
;
rs2:        '[' '!' 'p' '\033' '[' '?' DIGIT ';' DIGIT 'l' '\033' '[' DIGIT 'l' '\033' '>'
                                        { check_one (rs2, $7, 3);
                                          check_one (rs2, $9, 4);
                                          check_one (rs2, $13, 4);
                                          set_cmd (CMD_RESET_2STRING); }
;
u6:         '[' numbers 'R'             { set_cmd (CMD_USER6); }
;
u8:         '[' '?' numbers 'c'         { check_nums_2 (u8, 1, 2);
                                          set_cmd (CMD_USER8); }
;


/****************************************************************************/
/* strings and characters
 */

printable_char:     ' ' | '!' | '"' | '#' | '$' | '%' | '&' | '\'' | '(' | ')' | '*' | '+'
                |   ',' | '-' | '.' | '/' | ':'| ';' | '<' | '=' | '>' | '?' | '@'
                |   'A' | 'B' | 'C' | 'D' | 'E' | 'F' | 'G' |'H' | 'I' | 'J' | 'K' | 'L'
                |   'M' | 'N' | 'O' | 'P' | 'Q' | 'R' | 'S' | 'T' | 'U' | 'V' | 'W' | 'X'
                |   'Y' | 'Z' | '[' | '\\' | ']' | '^' | '_' | '`' | 'a' | 'b'| 'c' | 'd'
                |   'e' | 'f' | 'g' | 'h' | 'i' | 'j' | 'k' | 'l' | 'm' | 'n' | 'o' |'p'
                |   'q' | 'r' | 's' | 't' | 'u' | 'v' | 'w' | 'x' | 'y' | 'z' | '{' | '|'
                |   '}' | '~'
                |   DIGIT   { $$ = $1 + 48; }
;

non_printable_non_escape_char: '\001' | '\002' | '\003' | '\004' | '\005' | '\006' | '\007'
                |   '\010' | '\011' | '\012' | '\013' | '\014' | '\015' | '\016' | '\017'
                |   '\020' | '\021' | '\022' | '\023' | '\024' | '\025' | '\026' | '\027'
                |   '\030' | '\031' | '\032' | '\034' | '\035' | '\036' | '\037' | '\177'
;

non_printable_char: non_printable_non_escape_char
                |   '\033'
;

non_escape_char:    printable_char
                |   non_printable_non_escape_char
;

no_escape_string:   non_escape_char                     { add_char ($1); }
                |   no_escape_string non_escape_char    { add_char ($1); }
;

printable_string:   printable_char                      { add_char ($1); }
                |   printable_string printable_char     { add_char ($1); }
;
