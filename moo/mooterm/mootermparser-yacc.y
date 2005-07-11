%{
#define MOOTERM_COMPILATION
#include "mooterm/mootermparser.h"
#include "mooterm/mooterm-ctlfuncs.h"

#define add_number(n)                                   \
{                                                       \
    if (parser->numbers->len >= MAX_PARAMS_NUM)         \
    {                                                   \
        g_warning ("%s: too many parameters passed",    \
                   G_STRLOC);                           \
        YYABORT;                                        \
    }                                                   \
    else                                                \
    {                                                   \
        g_array_append_val (parser->numbers, n);        \
    }                                                   \
}
%}

%name-prefix="_moo_term_yy"

%lex-param      {MooTermParser *parser}
%parse-param    {MooTermParser *parser}

%%

control_function:   escape_sequence
                |   control_sequence
;


/****************************************************************************/
/* Escape sequences
 */

escape_sequence:    NEL
                |   DECRC
                |   DECSC
                |   HTS
                |   DECBI
                |   DECFI
                |   IND
                |   SS2
                |   SS3
                |   DECID
                |   RIS
                |   DECANM  /* exit ANSI mode */
                |   DECKPAM
                |   DECKPNM
                |   LS1R
                |   LS2
                |   LS2R
                |   LS3
                |   LS3R
                |   DECDHLT
                |   DECDHLB
                |   DECSWL
                |   DECDWL
                |   SCS
                |   S7C1T
                |   S8C1T
;


NEL:        '\033' 'E';
DECRC:      '\033' '8';
DECSC:      '\033' '7';
HTS:        '\033' 'H';
DECBI:      '\033' '6';
DECFI:      '\033' '9';
IND:        '\033' 'D';
SS2:        '\033' 'N';
SS3:        '\033' 'O';
DECID:      '\033' 'Z';
RIS:        '\033' 'c';
DECANM:     '\033' '<';
DECKPAM:    '\033' '=';
DECKPNM:    '\033' '>';
LS1R:       '\033' '~';
LS2:        '\033' 'n';
LS2R:       '\033' '}';
LS3:        '\033' 'o';
LS3R:       '\033' '|';
DECDHLT:    '\033' '#' '3';
DECDHLB:    '\033' '#' '4';
DECSWL:     '\033' '#' '5';
DECDWL:     '\033' '#' '6';
S7C1T:      '\033' ' ' 'F';
S8C1T:      '\033' ' ' 'G';

SCS:            '\033' SCS_set_num SCS_set;
SCS_set_num:    '('
            |   ')'
            |   '*'
            |   '+';
SCS_set:        '0'
            |   '1'
            |   '2'
            |   'A'
            |   'B';


/****************************************************************************/
/* Control sequences
 */

control_sequence:   DECSR
                |   SGR
                |   DECSET
                |   DECRST
                |   DEC_save
                |   DEC_restore
                |   SM
                |   RM
                |   CUB
                |   CBT
                |   CUD
                |   CUF
                |   CHA
                |   CHT
                |   CNL
                |   CPL
                |   CUP
                |   CUU
                |   HPA
                |   HPR
                |   VPA
                |   VPR
                |   DSR
                |   DECSCUSR
                |   DECST8C
                |   TBC
                |   DECSLRM
                |   DECSSCLS
                |   DECSTBM
                |   DECSCPP
                |   DECSLPP
                |   NP
                |   PP
                |   PPA
                |   PPB
                |   PPR
                |   DECRQDE
                |   DECSNLS
                |   SD
                |   SU
                |   DECRQUPSS
                |   DCH
                |   DECDC
                |   DL
                |   ECH
                |   ED
                |   EL
                |   ICH
                |   DECIC
                |   IL
                |   DECSCA
                |   DECSED
                |   DECSEL
                |   DECCARA
                |   DECCRA
                |   DECERA
                |   DECFRA
                |   DECRARA
                |   DECSACE
                |   DECSERA
                |   DECRQCRA
                |   DA1
                |   DA2
                |   DA3
                |   DECTST
                |   DECSTR
                |   DECSASD
                |   DECSSDT
;


DECSR:          '\233' number '+' 'p';

/* default parameter - 1 */
CUU:            '\233' number 'A'           {   vt_CUU ($2); }
            |   '\233' 'A'                  {   vt_CUU (1);  };
CUD:            '\233' number 'B'           {   vt_CUD ($2); }
            |   '\233' 'B'                  {   vt_CUD (1);  };
CUF:            '\233' number 'C'           {   vt_CUF ($2); }
            |   '\233' 'C'                  {   vt_CUF (1);  };
CUB:            '\233' number 'D'           {   vt_CUB ($2); }
            |   '\233' 'D'                  {   vt_CUB (1);  };
CBT:            '\233' number 'Z'
            |   '\233' 'Z';
CHA:            '\233' number 'G'
            |   '\233' 'G';
CHT:            '\233' number 'I'
            |   '\233' 'I';
CNL:            '\233' number 'E'
            |   '\233' 'E';
CPL:            '\233' number 'F'
            |   '\233' 'F';
HPA:            '\233' number '`'
            |   '\233' '`';
HPR:            '\233' number 'a'
            |   '\233' 'a';
VPA:            '\233' number 'd'
            |   '\233' 'd';
VPR:            '\233' number 'e'
            |   '\233' 'e';

CUP:            '\233' numbers 'H'
            |   '\233' 'H'
            |   '\233' numbers 'f'  /* HVP */
            |   '\233' 'f';

DECSCUSR:       '\233' number ' ' 'q'
            |   '\233' ' ' 'q';

DECST8C:        '\233' '?' number 'W';
TBC:            '\233' number 'g'
            |   '\233' 'g';

DECSLRM:        '\233' numbers 's'
            |   '\233' 's';

DECSSCLS:       '\233' number ' ' 'p'
            |   '\233' ' ' 'p';

DECSTBM:        '\233' numbers 'r'
            |   '\233' 'r';

DECSLPP:        '\233' number 't'
            |   '\233' 't';

NP:             '\233' number 'U'
            |   '\233' 'U';
PP:             '\233' number 'V'
            |   '\233' 'V';

PPA:            '\233' number ' ' 'P'
            |   '\233' ' ' 'P';
PPB:            '\233' number ' ' 'R'
            |   '\233' ' ' 'R';
PPR:            '\233' number ' ' 'Q'
            |   '\233' ' ' 'Q';

DECRQDE:        '\233' '"' 'v';

SD:             '\233' number 'S';
SU:             '\233' number 'T';

DECRQUPSS:      '\233' '&' 'u';


DECDC:          '\233' number '\'' '~'
            |   '\233'  '\'' '~';
DECIC:          '\233' number '\'' '}'
            |   '\233'  '\'' '}';
DCH:            '\233' number 'P'           {   vt_DCH ($2 ? $2 : 1);   }
            |   '\233' 'P'                  {   vt_DCH (1);             };
DL:             '\233' number 'M'           {   vt_DL ($2 ? $2 : 1);    }
            |   '\233' 'M'                  {   vt_DL (1);              };
ECH:            '\233' number 'X'           {   vt_ECH ($2 ? $2 : 1);   }
            |   '\233' 'X'                  {   vt_ECH (1);             };
ED:             '\233' number 'J'           {   vt_ED ($2);             }
            |   '\233' 'J'                  {   vt_ED (0);              };
EL:             '\233' number 'K'           {   vt_EL ($2);             }
            |   '\233' 'K'                  {   vt_EL (0);              };
ICH:            '\233' number '@'           {   vt_ICH ($2 ? $2 : 1);   }
            |   '\233' '@'                  {   vt_ICH (1);             };
IL:             '\233' number 'L'           {   vt_IL ($2 ? $2 : 1);    }
            |   '\233' 'L'                  {   vt_IL (1);              };

DECSCA:         '\233' number '"' 'q';
DECSED:         '\233' '?' number 'J'
            |   '\233' '?' 'J';
DECSEL:         '\233' '?' number 'K'
            |   '\233' '?' 'K';

DECSCPP:        '\233' numbers '$' '|'
            |   '\233' '$' '|';
DECCARA:        '\233' numbers '$' 'r';
DECCRA:         '\233' numbers '$' 'v';
DECERA:         '\233' numbers '$' 'z';
DECFRA:         '\233' numbers '$' 'x';
DECRARA:        '\233' numbers '$' 't';
DECSERA:        '\233' numbers '$' '{';
DECSASD:        '\233' numbers '$' '}';
DECSSDT:        '\233' numbers '$' '~';

DECSACE:        '\233' numbers '*' 'x';
DECSNLS:        '\233' numbers '*' '|';
DECRQCRA:       '\233' numbers '*' 'y';

DSR:            '\233' '?' numbers 'n'
            |   '\233' numbers 'n';

DA1:            '\233' 'c'
            |   '\233' number 'c';
DA2:            '\233' '>' 'c'
            |   '\233' '>' number 'c';
DA3:            '\233' '=' 'c'
            |   '\233' '=' number 'c';

DECTST:         '\233' numbers 'y';

DECSTR:         '\233' '!' 'p';


SGR:            '\233' numbers 'm'          {   vt_SGR ();  }
            |   '\233' 'm'                  {   vt_SGR ();  }
;
DECSET:         '\233' '?' numbers 'h'
            |   '\233' '?' 'h';
DECRST:         '\233' '?' numbers 'l'
            |   '\233' '?' 'l';
SM:             '\233' numbers 'h'
            |   '\233' 'h';
RM:             '\233' numbers 'l'
            |   '\233' 'l';

DEC_save:       '\233' '?' numbers 's'
            |   '\233' '?' 's';
DEC_restore:    '\233' '?' numbers 'r'
            |   '\233' '?' 'r';


/****************************************************************************/
/* numbers
 */

number:         digit           {   $$ = $1;   }
            |   number digit    {   $$ = $1 * 10 + $2;  }
            ;

digit:          '0'             {   $$ = 0;    }
            |   '1'             {   $$ = 1;    }
            |   '2'             {   $$ = 2;    }
            |   '3'             {   $$ = 3;    }
            |   '4'             {   $$ = 4;    }
            |   '5'             {   $$ = 5;    }
            |   '6'             {   $$ = 6;    }
            |   '7'             {   $$ = 7;    }
            |   '8'             {   $$ = 8;    }
            |   '9'             {   $$ = 9;    }
            ;

numbers:        number              {   add_number ($1);    }
            |   numbers ';' number  {   add_number ($3);    }
;
