%{
#define MOOTERM_COMPILATION
#include "mooterm/mootermparser.h"
#include "mooterm/mooterm-vtctls.h"

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
        int val = n;                                    \
        g_array_append_val (parser->numbers, val);      \
    }                                                   \
}

#define check_nums_len(n)                               \
{                                                       \
    if (parser->numbers->len != n)                      \
        YYABORT;                                        \
}

#define get_num(n) (((int*)parser->numbers->data)[n])

#define TERMINAL_HEIGHT (buf_screen_height (parser->term->priv->buffer))
#define TERMINAL_WIDTH  (buf_screen_width (parser->term->priv->buffer))
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
                |   RI
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


NEL:        '\033' 'E'          {   vt_not_implemented();   };
DECRC:      '\033' '8'          {   vt_DECRC ();    };
DECSC:      '\033' '7'          {   vt_DECSC ();            };
HTS:        '\033' 'H'          {   vt_not_implemented();   };
DECBI:      '\033' '6'          {   vt_not_implemented();   };
DECFI:      '\033' '9'          {   vt_not_implemented();   };
IND:        '\033' 'D'          {   vt_IND ();      };
RI:         '\033' 'M'          {   vt_RI ();       };
SS2:        '\033' 'N'          {   vt_not_implemented();   };
SS3:        '\033' 'O'          {   vt_not_implemented();   };
DECID:      '\033' 'Z'          {   vt_not_implemented();   };
RIS:        '\033' 'c'          {   vt_not_implemented();   };
DECANM:     '\033' '<'          {   vt_not_implemented();   };
DECKPAM:    '\033' '='          {   vt_DECKPAM ();  };
DECKPNM:    '\033' '>'          {   vt_DECKPNM ();  };
LS1R:       '\033' '~'          {   vt_not_implemented();   };
LS2:        '\033' 'n'          {   vt_not_implemented();   };
LS2R:       '\033' '}'          {   vt_not_implemented();   };
LS3:        '\033' 'o'          {   vt_not_implemented();   };
LS3R:       '\033' '|'          {   vt_not_implemented();   };
DECDHLT:    '\033' '#' '3'      {   vt_not_implemented();   };
DECDHLB:    '\033' '#' '4'      {   vt_not_implemented();   };
DECSWL:     '\033' '#' '5'      {   vt_not_implemented();   };
DECDWL:     '\033' '#' '6'      {   vt_not_implemented();   };
S7C1T:      '\033' ' ' 'F'      {   vt_not_implemented();   };
S8C1T:      '\033' ' ' 'G'      {   vt_not_implemented();   };

SCS:            '\033' SCS_set_num SCS_set  {   vt_SCS (get_num(0), get_num(1));    };
SCS_set_num:    '('                         {   add_number (0);                     }
            |   ')'                         {   add_number (1);                     }
            |   '*'                         {   add_number (2);                     }
            |   '+'                         {   add_number (3);                     };
SCS_set:        '0'                         {   add_number (0);                     }
            |   '1'                         {   add_number (1);                     }
            |   '2'                         {   add_number (2);                     }
            |   'A'                         {   add_number (3);                     }
            |   'B'                         {   add_number (4);                     };


/****************************************************************************/
/* Control sequences
 */

control_sequence:   DECSR
                |   SGR
                |   DECSET
                |   DECRST
                |   DECSAVE
                |   DECRESTORE
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


DECSR:          '\233' number '+' 'p'       {   vt_not_implemented();   };

/* default parameter - 1 */
CUU:            '\233' number 'A'           {   vt_CUU ($2); }
            |   '\233' 'A'                  {   vt_CUU (1);  };
CUD:            '\233' number 'B'           {   vt_CUD ($2); }
            |   '\233' 'B'                  {   vt_CUD (1);  };
CUF:            '\233' number 'C'           {   vt_CUF ($2); }
            |   '\233' 'C'                  {   vt_CUF (1);  };
CUB:            '\233' number 'D'           {   vt_CUB ($2); }
            |   '\233' 'D'                  {   vt_CUB (1);  };
CBT:            '\233' number 'Z'           {   vt_not_implemented();   }
            |   '\233' 'Z'                  {   vt_not_implemented();   };
CHA:            '\233' number 'G'
            |   '\233' 'G'                  {   vt_not_implemented();   };
CHT:            '\233' number 'I'           {   vt_not_implemented();   }
            |   '\233' 'I'                  {   vt_not_implemented();   };
CNL:            '\233' number 'E'           {   vt_not_implemented();   }
            |   '\233' 'E'                  {   vt_not_implemented();   };
CPL:            '\233' number 'F'           {   vt_not_implemented();   }
            |   '\233' 'F'                  {   vt_not_implemented();   };
HPA:            '\233' number '`'           {   vt_not_implemented();   }
            |   '\233' '`'                  {   vt_not_implemented();   };
HPR:            '\233' number 'a'           {   vt_not_implemented();   }
            |   '\233' 'a'                  {   vt_not_implemented();   };
VPA:            '\233' number 'd'           {   vt_not_implemented();   }
            |   '\233' 'd'                  {   vt_not_implemented();   };
VPR:            '\233' number 'e'           {   vt_not_implemented();   }
            |   '\233' 'e'                  {   vt_not_implemented();   };

CUP:            '\233' numbers 'H'          {   check_nums_len (2);
                                                vt_CUP (get_num (0) ? get_num (0) : 1,
                                                        get_num (1) ? get_num (1) : 1);   }
            |   '\233' 'H'                  {   vt_CUP (1, 1);   }
            |   '\233' numbers 'f'          {   vt_not_implemented();   }  /* HVP */
            |   '\233' 'f'                  {   vt_not_implemented();   };

DECSCUSR:       '\233' number ' ' 'q'       {   vt_not_implemented();   }
            |   '\233' ' ' 'q'              {   vt_not_implemented();   };

DECST8C:        '\233' '?' number 'W'       {   vt_not_implemented();   };
TBC:            '\233' number 'g'           {   vt_not_implemented();   }
            |   '\233' 'g'                  {   vt_not_implemented();   };

DECSLRM:        '\233' numbers 's'          {   vt_not_implemented();   }
            |   '\233' 's'                  {   vt_not_implemented();   };

DECSSCLS:       '\233' number ' ' 'p'       {   vt_not_implemented();   }
            |   '\233' ' ' 'p'              {   vt_not_implemented();   };

DECSTBM:        '\233' numbers 'r'          {   check_nums_len (2);
                                                vt_DECSTBM (get_num (0) ? get_num (0) : 1,
                                                            get_num (1) ? get_num (1) : 1); }
            |   '\233' 'r'                  {   vt_DECSTBM (1, TERMINAL_HEIGHT);    };

DECSLPP:        '\233' number 't'           {   vt_not_implemented();   }
            |   '\233' 't'                  {   vt_not_implemented();   };

NP:             '\233' number 'U'           {   vt_not_implemented();   }
            |   '\233' 'U'                  {   vt_not_implemented();   };
PP:             '\233' number 'V'           {   vt_not_implemented();   }
            |   '\233' 'V'                  {   vt_not_implemented();   };

PPA:            '\233' number ' ' 'P'       {   vt_not_implemented();   }
            |   '\233' ' ' 'P'              {   vt_not_implemented();   };
PPB:            '\233' number ' ' 'R'       {   vt_not_implemented();   }
            |   '\233' ' ' 'R'              {   vt_not_implemented();   };
PPR:            '\233' number ' ' 'Q'       {   vt_not_implemented();   }
            |   '\233' ' ' 'Q'              {   vt_not_implemented();   };

DECRQDE:        '\233' '"' 'v'              {   vt_not_implemented();   };

SD:             '\233' number 'S'           {   vt_not_implemented();   };
SU:             '\233' number 'T'           {   vt_not_implemented();   };

DECRQUPSS:      '\233' '&' 'u'              {   vt_not_implemented();   };


DECDC:          '\233' number '\'' '~'      {   vt_not_implemented();   }
            |   '\233'  '\'' '~'            {   vt_not_implemented();   };
DECIC:          '\233' number '\'' '}'      {   vt_not_implemented();   }
            |   '\233'  '\'' '}'            {   vt_not_implemented();   };
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

DECSCA:         '\233' number '"' 'q'       {   vt_not_implemented();   };
DECSED:         '\233' '?' number 'J'       {   vt_not_implemented();   }
            |   '\233' '?' 'J'              {   vt_not_implemented();   };
DECSEL:         '\233' '?' number 'K'       {   vt_not_implemented();   }
            |   '\233' '?' 'K'              {   vt_not_implemented();   };

DECSCPP:        '\233' numbers '$' '|'      {   vt_not_implemented();   }
            |   '\233' '$' '|'              {   vt_not_implemented();   };
DECCARA:        '\233' numbers '$' 'r'      {   vt_not_implemented();   };
DECCRA:         '\233' numbers '$' 'v'      {   vt_not_implemented();   };
DECERA:         '\233' numbers '$' 'z'      {   vt_not_implemented();   };
DECFRA:         '\233' numbers '$' 'x'      {   vt_not_implemented();   };
DECRARA:        '\233' numbers '$' 't'      {   vt_not_implemented();   };
DECSERA:        '\233' numbers '$' '{'      {   vt_not_implemented();   };
DECSASD:        '\233' numbers '$' '}'      {   vt_not_implemented();   };
DECSSDT:        '\233' numbers '$' '~'      {   vt_not_implemented();   };

DECSACE:        '\233' numbers '*' 'x'      {   vt_not_implemented();   };
DECSNLS:        '\233' numbers '*' '|'      {   vt_not_implemented();   };
DECRQCRA:       '\233' numbers '*' 'y'      {   vt_not_implemented();   };

DSR:            '\233' '?' numbers 'n'      {   vt_not_implemented();   }
            |   '\233' numbers 'n'          {   vt_not_implemented();   };

DA1:            '\233' 'c'                  {   vt_DA1 ();   }
            |   '\233' number 'c'           {   vt_DA1 ();   };
DA2:            '\233' '>' 'c'              {   vt_DA2 ();   }
            |   '\233' '>' number 'c'       {   vt_DA2 ();   };
DA3:            '\233' '=' 'c'              {   vt_DA3 ();   }
            |   '\233' '=' number 'c'       {   vt_DA3 ();   };

DECTST:         '\233' numbers 'y'          {   vt_not_implemented();   };

DECSTR:         '\233' '!' 'p'              {   vt_not_implemented();   };


SGR:            '\233' numbers 'm'          {   vt_SGR ();          }
            |   '\233' 'm'                  {   vt_SGR ();          };
DECSET:         '\233' '?' numbers 'h'      {   vt_DECSET ();       }
            |   '\233' '?' 'h'              {   vt_DECSET ();       };
DECRST:         '\233' '?' numbers 'l'      {   vt_DECRST ();       }
            |   '\233' '?' 'l'              {   vt_DECRST ();       };
SM:             '\233' numbers 'h'          {   vt_SM ();           }
            |   '\233' 'h'                  {   vt_SM ();           };
RM:             '\233' numbers 'l'          {   vt_RM ();           }
            |   '\233' 'l'                  {   vt_RM ();           };
DECSAVE:        '\233' '?' numbers 's'      {   vt_DECSAVE ();      }
            |   '\233' '?' 's'              {   vt_DECSAVE ();      };
DECRESTORE:     '\233' '?' numbers 'r'      {   vt_DECRESTORE ();   }
            |   '\233' '?' 'r'              {   vt_DECRESTORE ();   };


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
