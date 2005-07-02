%{
#define MOOTERM_COMPILATION
#include "mooterm/mootermparser.h"

#define add_number(num)                         \
{                                               \
    if (!parser->nums_len)                      \
    {                                           \
        parser->nums[0] = num;                  \
        parser->nums_len = 1;                   \
    }                                           \
    else                                        \
    {                                           \
        if (parser->nums_len >= MAX_NUMS_LEN)   \
            YYABORT;                            \
        parser->nums[parser->nums_len++] = num; \
    }                                           \
}

#define set_cmd(cmd_code)                       \
{                                               \
    parser->cmd = cmd_code;                     \
    YYACCEPT;                                   \
}

%}

%name-prefix="_moo_term_yy"

%lex-param      {MooTermParser *parser}
%parse-param    {MooTermParser *parser}

%%

command:            terminfo_sequence
                |   xterm_sequence
;

terminfo_sequence:  cbt
                |   civis
                |   clear
                |   cnorm
                |   csr
                |   cub
                |   cud
                |   cuf
                |   cuf1
                |   cup
                |   cuu
                |   cuu1
                |   dch
                |   dch1
                |   dl
                |   dl1
                |   ech
                |   ed
                |   el
                |   el1
                |   enacs
                |   flash
                |   home
                |   hpa
                |   hts
                |   ich
                |   il
                |   il1
                |   mc0
                |   mc4
                |   mc5
                |   meml
                |   memu
                |   rc
                |   ri
                |   rmam
                |   rmcup
                |   rmir
                |   rmkx
                |   rs1
                |   rs2
                |   sc
                |   smam
                |   smcup
                |   smir
                |   smkx
                |   tbc
                |   u6
                |   u7
                |   u8
                |   u9
                |   vpa
                |   set_attrs
;

xterm_sequence:     set_title
                |   set_icon
                |   set_title_icon
;


/****************************************************************************/
/* numbers
 */

digit:          '0'                 { $$ = 0; }
            |   '1'                 { $$ = 1; }
            |   '2'                 { $$ = 2; }
            |   '3'                 { $$ = 3; }
            |   '4'                 { $$ = 4; }
            |   '5'                 { $$ = 5; }
            |   '6'                 { $$ = 6; }
            |   '7'                 { $$ = 7; }
            |   '8'                 { $$ = 8; }
            |   '9'                 { $$ = 9; }
;

number:         digit
            |   number digit        { $$ = $1 * 10 + $2; }
;

/****************************************************************************/
/* terminfo escape sequences
 */

/*  making this like "'[' numbers 'm'" produces conflicts, and "[10;23m" is
    not parsed */
set_attrs:  '[' 'm'                     {   set_cmd (CMD_SET_ATTRS); }
        |   '[' number 'm'              {   add_number ($2);
                                            set_cmd (CMD_SET_ATTRS); }
        |   '[' number ';' number 'm'   {   add_number ($2);
                                            add_number ($4);
                                            set_cmd (CMD_SET_ATTRS); }
        |   '[' number ';' number ';' number 'm'
                                        {   add_number ($2);
                                            add_number ($4);
                                            add_number ($6);
                                            set_cmd (CMD_SET_ATTRS); }
        |   '[' number ';' number ';' number ';' number 'm'
                                        {   add_number ($2);
                                            add_number ($4);
                                            add_number ($6);
                                            add_number ($8);
                                            set_cmd (CMD_SET_ATTRS); }
        |   '[' number ';' number ';' number ';' number ';' number 'm'
                                        {   add_number ($2);
                                            add_number ($4);
                                            add_number ($6);
                                            add_number ($8);
                                            add_number ($10);
                                            set_cmd (CMD_SET_ATTRS); }
        |   '[' number ';' number ';' number ';' number ';' number ';' number 'm'
                                        {   add_number ($2);
                                            add_number ($4);
                                            add_number ($6);
                                            add_number ($8);
                                            add_number ($10);
                                            add_number ($12);
                                            set_cmd (CMD_SET_ATTRS); }
;
        cbt:        '[' 'Z'                     { set_cmd (CMD_BACK_TAB); }
;
civis:      '[' '?' '2' '5' 'l'         { set_cmd (CMD_CURSOR_INVISIBLE); }
;
cnorm:      '[' '?' '2' '5' 'h'         { set_cmd (CMD_CURSOR_NORMAL); }
;
clear:      '[' 'H' '\033' '[' '2' 'J'  { set_cmd (CMD_CLEAR_SCREEN); }
;
csr:        '[' number ';' number 'r'   { add_number ($2);
                                          add_number ($4);
                                          set_cmd (CMD_CHANGE_SCROLL_REGION); }
;
cub:        '[' number 'D'              { add_number ($2);
                                          set_cmd (CMD_PARM_LEFT_CURSOR); }
;
cud:        '[' number 'B'              { add_number ($2);
                                          set_cmd (CMD_PARM_DOWN_CURSOR); }
;
cuf:        '[' number 'C'              { add_number ($2);
                                          set_cmd (CMD_PARM_RIGHT_CURSOR); }
;
cuf1:       '[' 'C'                     { set_cmd (CMD_CURSOR_RIGHT); }
;
cup:        '[' number ';' number 'H'   { add_number ($2);
                                          add_number ($4);
                                          set_cmd (CMD_CURSOR_ADDRESS); }
;
hpa:        '[' number 'G'              { add_number ($2);
                                          set_cmd (CMD_COLUMN_ADDRESS); }
                                          ;
vpa:        '[' number 'd'              { add_number ($2);
                                          set_cmd (CMD_ROW_ADDRESS); }
                                          ;
cuu:        '[' number 'A'              { add_number ($2);
                                          set_cmd (CMD_PARM_UP_CURSOR); }
;
cuu1:       '[' 'A'                     { set_cmd (CMD_CURSOR_UP); }
;
dch:        '[' number 'P'              { add_number ($2);
                                          set_cmd (CMD_PARM_DCH); }
;
dch1:       '[' 'P'                     { set_cmd (CMD_DELETE_CHARACTER); }
;
dl:         '[' number 'M'              { add_number ($2);
                                          set_cmd (CMD_PARM_DELETE_LINE); }
;
dl1:        '[' 'M'                     { set_cmd (CMD_DELETE_LINE); }
;

ech:        '[' number 'X'              { add_number ($2);
                                          set_cmd (CMD_BACK_TAB); }
;
ed:         '[' 'J'                     { set_cmd (CMD_CLR_EOS); }
;
el:         '[' 'K'                     { set_cmd (CMD_CLR_EOL); }
;
el1:        '[' '1' 'K'                 { set_cmd (CMD_CLR_BOL); }
;
enacs:      '(' 'B' '\033' ')' '0'      { set_cmd (CMD_ENA_ACS); }
;
flash:      '[' '?' '5' 'h' '\033' '[' '?' '5' 'l'  { set_cmd (CMD_FLASH_SCREEN); }
;
home:       '[' 'H'                     { set_cmd (CMD_CURSOR_HOME); }
;
hts:        'H'                         { set_cmd (CMD_SET_TAB); }
;
ich:        '[' number '@'              { add_number ($2);
                                          set_cmd (CMD_PARM_ICH); }
;
il:         '[' number 'L'              { add_number ($2);
                                          set_cmd (CMD_PARM_INSERT_LINE); }
;
il1:        '[' 'L'                     { set_cmd (CMD_INSERT_LINE); }
;
mc0:        '[' 'i'                     { set_cmd (CMD_PRINT_SCREEN); }
;
mc4:        '[' '4' 'i'                 { set_cmd (CMD_PRTR_OFF); }
;
mc5:        '[' '5' 'i'                 { set_cmd (CMD_PRTR_ON); }
;
meml:       'l'                         { set_cmd (CMD_ESC_l); }
;
memu:       'm'                         { set_cmd (CMD_ESC_m); }
;
sc:         '7'                         { set_cmd (CMD_SAVE_CURSOR); }
;
rc:         '8'                         { set_cmd (CMD_RESTORE_CURSOR); }
;
ri:         'M'                         { set_cmd (CMD_SCROLL_REVERSE); }
;
smam:       '[' '?' '7' 'h'             { set_cmd (CMD_ENTER_AM_MODE); }
;
rmam:       '[' '?' '7' 'l'             { set_cmd (CMD_EXIT_AM_MODE); }
;
smcup:      '[' '?' '1' '0' '4' '9' 'h' { set_cmd (CMD_ENTER_CA_MODE); }
;
rmcup:      '[' '?' '1' '0' '4' '9' 'l' { set_cmd (CMD_EXIT_CA_MODE); }
;
smir:       '[' '4' 'h'                 { set_cmd (CMD_ENTER_INSERT_MODE); }
;
rmir:       '[' '4' 'l'                 { set_cmd (CMD_EXIT_INSERT_MODE); }
;
rmkx:       '[' '?' '1' 'l' '\033' '>'  { set_cmd (CMD_KEYPAD_LOCAL); }
;
rs1:        'c'                         { set_cmd (CMD_RESET_1STRING); }
;
rs2:        '[' '!' 'p' '\033' '[' '?' '3' ';' '4' 'l' '\033' '[' '4' 'l' '\033' '>' { set_cmd (CMD_RESET_2STRING); }
;
smkx:       '[' '?' '1' 'h' '\033' '='  { set_cmd (CMD_KEYPAD_XMIT); }
;
tbc:        '[' '3' 'g'                 { set_cmd (CMD_CLEAR_ALL_TABS); }
;
u6:         '[' number ';' number 'R'   { add_number ($2);
                                          add_number ($4);
                                          set_cmd (CMD_USER6); }
;
u7:         '[' '6' 'n'                 { set_cmd (CMD_USER7); }
;
u8:         '[' '?' '1' ';' '2' 'c'     { set_cmd (CMD_USER8); }
;
u9:         '[' 'c'                     { set_cmd (CMD_USER9); }
;


/****************************************************************************/
/* xterm commands
 */

set_title:      ']' '0' ';'   { set_cmd (CMD_SET_WINDOW_TITLE); }
;
set_icon:       ']' '1' ';'   { set_cmd (CMD_SET_ICON_NAME); }
;
set_title_icon: ']' '2' ';'   { set_cmd (CMD_SET_WINDOW_ICON_NAME); }
;
