%{
#define MOOTERM_COMPILATION
#include "mooterm/mootermparser.h"
#include "mooterm/mooterm-vtctls.h"

#define ADD_NUMBER(n)                                   \
G_STMT_START {                                          \
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
} G_STMT_END

#define NUMS_LEN (parser->numbers->len)

#define CHECK_NUMS_LEN(n)                               \
G_STMT_START {                                          \
    if (parser->numbers->len != n)                      \
        YYABORT;                                        \
} G_STMT_END

#define GET_NUM(n) (((int*)parser->numbers->data)[n])

#define TERMINAL_HEIGHT (buf_screen_height (parser->term->priv->buffer))
#define TERMINAL_WIDTH  (buf_screen_width (parser->term->priv->buffer))
%}

%name-prefix="_moo_term_yy"

%lex-param      {MooTermParser *parser}
%parse-param    {MooTermParser *parser}

%%

control_function:   escape_sequence
                |   control_sequence
                |   device_control_sequence
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
                |   DECALN
;


NEL:        '\033' 'E'          {   VT_NEL;             };
DECRC:      '\033' '8'          {   VT_DECRC;           };
DECSC:      '\033' '7'          {   VT_DECSC;           };
HTS:        '\033' 'H'          {   VT_HTS;             };
DECBI:      '\033' '6'          {   VT_NOT_IMPLEMENTED; };
DECFI:      '\033' '9'          {   VT_NOT_IMPLEMENTED; };
IND:        '\033' 'D'          {   VT_IND;             };
RI:         '\033' 'M'          {   VT_RI;              };
SS2:        '\033' 'N'          {   VT_NOT_IMPLEMENTED; };
SS3:        '\033' 'O'          {   VT_NOT_IMPLEMENTED; };
DECID:      '\033' 'Z'          {   VT_NOT_IMPLEMENTED; };
RIS:        '\033' 'c'          {   VT_RIS;             };
DECANM:     '\033' '<'          {   VT_NOT_IMPLEMENTED; };
DECKPAM:    '\033' '='          {   VT_DECKPAM;         };
DECKPNM:    '\033' '>'          {   VT_DECKPNM;         };
LS1R:       '\033' '~'          {   VT_NOT_IMPLEMENTED; };
LS2:        '\033' 'n'          {   VT_NOT_IMPLEMENTED; };
LS2R:       '\033' '}'          {   VT_NOT_IMPLEMENTED; };
LS3:        '\033' 'o'          {   VT_NOT_IMPLEMENTED; };
LS3R:       '\033' '|'          {   VT_NOT_IMPLEMENTED; };
DECDHLT:    '\033' '#' '3'      {   VT_NOT_IMPLEMENTED; };
DECDHLB:    '\033' '#' '4'      {   VT_NOT_IMPLEMENTED; };
DECSWL:     '\033' '#' '5'      {   VT_NOT_IMPLEMENTED; };
DECDWL:     '\033' '#' '6'      {   VT_NOT_IMPLEMENTED; };
DECALN:     '\033' '#' '8'      {   VT_DECALN;          };
S7C1T:      '\033' ' ' 'F'      {   VT_IGNORED;         };
S8C1T:      '\033' ' ' 'G'      {   VT_IGNORED;         };

SCS:            '\033' SCS_set_num SCS_set  {   VT_SCS (GET_NUM(0), GET_NUM(1));    };
SCS_set_num:    '('                         {   ADD_NUMBER (0);                     }
            |   ')'                         {   ADD_NUMBER (1);                     }
            |   '*'                         {   ADD_NUMBER (2);                     }
            |   '+'                         {   ADD_NUMBER (3);                     };
SCS_set:        '0'                         {   ADD_NUMBER (0);                     }
            |   '1'                         {   ADD_NUMBER (1);                     }
            |   '2'                         {   ADD_NUMBER (2);                     }
            |   'A'                         {   ADD_NUMBER (3);                     }
            |   'B'                         {   ADD_NUMBER (4);                     };


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


DECSR:          '\233' number '+' 'p'       {   VT_NOT_IMPLEMENTED;     };

/* default parameter - 1 */
CUU:            '\233' number 'A'           {   VT_CUU ($2);            }
            |   '\233' 'A'                  {   VT_CUU (1);             };
CUD:            '\233' number 'B'           {   VT_CUD ($2);            }
            |   '\233' 'B'                  {   VT_CUD (1);             };
CUF:            '\233' number 'C'           {   VT_CUF ($2);            }
            |   '\233' 'C'                  {   VT_CUF (1);             };
CUB:            '\233' number 'D'           {   VT_CUB ($2);            }
            |   '\233' 'D'                  {   VT_CUB (1);             };
CBT:            '\233' number 'Z'           {   VT_CBT ($2 ? $2 : 1);   }
            |   '\233' 'Z'                  {   VT_CBT (1);             };
CHA:            '\233' number 'G'           {   VT_CHA ($2 ? $2 : 1);   }
            |   '\233' 'G'                  {   VT_CHA (1);             };
CHT:            '\233' number 'I'           {   VT_CHT ($2 ? $2 : 1);   }
            |   '\233' 'I'                  {   VT_CHT (1);             };
CNL:            '\233' number 'E'           {   VT_CNL ($2 ? $2 : 1);   }
            |   '\233' 'E'                  {   VT_CNL (1);             };
CPL:            '\233' number 'F'           {   VT_CPL ($2 ? $2 : 1);   }
            |   '\233' 'F'                  {   VT_CPL (1);             };
HPA:            '\233' number '`'           {   VT_HPA ($2 ? $2 : 1);   }
            |   '\233' '`'                  {   VT_HPA (1);             };
HPR:            '\233' number 'a'           {   VT_HPR ($2 ? $2 : 1);   }
            |   '\233' 'a'                  {   VT_HPR (1);             };
VPA:            '\233' number 'd'           {   VT_VPA ($2 ? $2 : 1);   }
            |   '\233' 'd'                  {   VT_VPA (1);             };
VPR:            '\233' number 'e'           {   VT_VPR ($2 ? $2 : 1);   }
            |   '\233' 'e'                  {   VT_VPR (1);             };

CUP:            '\233' numbers 'H'          {   CHECK_NUMS_LEN (2);
                                                VT_CUP (GET_NUM (0) ? GET_NUM (0) : 1,
                                                        GET_NUM (1) ? GET_NUM (1) : 1);   }
            |   '\233' 'H'                  {   VT_CUP (1, 1);   }
            |   '\233' numbers 'f'          {   CHECK_NUMS_LEN (2);
                                                VT_CUP (GET_NUM (0) ? GET_NUM (0) : 1,
                                                        GET_NUM (1) ? GET_NUM (1) : 1);   }  /* HVP */
            |   '\233' 'f'                  {   VT_CUP (1, 1);   };

DECSCUSR:       '\233' number ' ' 'q'       {   VT_NOT_IMPLEMENTED; }
            |   '\233' ' ' 'q'              {   VT_NOT_IMPLEMENTED; };

DECST8C:        '\233' '?' number 'W'       {   VT_NOT_IMPLEMENTED; };
TBC:            '\233' number 'g'           {   VT_TBC ($2);        }
            |   '\233' 'g'                  {   VT_TBC (0);         };

DECSLRM:        '\233' numbers 's'          {   VT_NOT_IMPLEMENTED; }
            |   '\233' 's'                  {   VT_NOT_IMPLEMENTED; };

DECSSCLS:       '\233' number ' ' 'p'       {   VT_NOT_IMPLEMENTED; }
            |   '\233' ' ' 'p'              {   VT_NOT_IMPLEMENTED; };

DECSTBM:        '\233' numbers 'r'          {   CHECK_NUMS_LEN (2);
                                                VT_DECSTBM (GET_NUM (0) ? GET_NUM (0) : 1,
                                                            GET_NUM (1) ? GET_NUM (1) : 1); }
            |   '\233' 'r'                  {   VT_DECSTBM (1, TERMINAL_HEIGHT);    };

DECSLPP:        '\233' number 't'           {   VT_NOT_IMPLEMENTED; }
            |   '\233' 't'                  {   VT_NOT_IMPLEMENTED; };

NP:             '\233' number 'U'           {   VT_NOT_IMPLEMENTED; }
            |   '\233' 'U'                  {   VT_NOT_IMPLEMENTED; };
PP:             '\233' number 'V'           {   VT_NOT_IMPLEMENTED; }
            |   '\233' 'V'                  {   VT_NOT_IMPLEMENTED; };

PPA:            '\233' number ' ' 'P'       {   VT_NOT_IMPLEMENTED; }
            |   '\233' ' ' 'P'              {   VT_NOT_IMPLEMENTED; };
PPB:            '\233' number ' ' 'R'       {   VT_NOT_IMPLEMENTED; }
            |   '\233' ' ' 'R'              {   VT_NOT_IMPLEMENTED; };
PPR:            '\233' number ' ' 'Q'       {   VT_NOT_IMPLEMENTED; }
            |   '\233' ' ' 'Q'              {   VT_NOT_IMPLEMENTED; };

DECRQDE:        '\233' '"' 'v'              {   VT_NOT_IMPLEMENTED; };

SD:             '\233' number 'S'           {   VT_NOT_IMPLEMENTED; };
SU:             '\233' number 'T'           {   VT_NOT_IMPLEMENTED; };

DECRQUPSS:      '\233' '&' 'u'              {   VT_NOT_IMPLEMENTED; };


DECDC:          '\233' number '\'' '~'      {   VT_NOT_IMPLEMENTED;     }
            |   '\233'  '\'' '~'            {   VT_NOT_IMPLEMENTED;     };
DECIC:          '\233' number '\'' '}'      {   VT_NOT_IMPLEMENTED;     }
            |   '\233'  '\'' '}'            {   VT_NOT_IMPLEMENTED;     };
DCH:            '\233' number 'P'           {   VT_DCH ($2 ? $2 : 1);   }
            |   '\233' 'P'                  {   VT_DCH (1);             };
DL:             '\233' number 'M'           {   VT_DL ($2 ? $2 : 1);    }
            |   '\233' 'M'                  {   VT_DL (1);              };
ECH:            '\233' number 'X'           {   VT_ECH ($2 ? $2 : 1);   }
            |   '\233' 'X'                  {   VT_ECH (1);             };
ED:             '\233' number 'J'           {   VT_ED ($2);             }
            |   '\233' 'J'                  {   VT_ED (0);              };
EL:             '\233' number 'K'           {   VT_EL ($2);             }
            |   '\233' 'K'                  {   VT_EL (0);              };
ICH:            '\233' number '@'           {   VT_ICH ($2 ? $2 : 1);   }
            |   '\233' '@'                  {   VT_ICH (1);             };
IL:             '\233' number 'L'           {   VT_IL ($2 ? $2 : 1);    }
            |   '\233' 'L'                  {   VT_IL (1);              };

DECSCA:         '\233' number '"' 'q'       {   VT_NOT_IMPLEMENTED;   };
DECSED:         '\233' '?' number 'J'       {   VT_NOT_IMPLEMENTED;   }
            |   '\233' '?' 'J'              {   VT_NOT_IMPLEMENTED;   };
DECSEL:         '\233' '?' number 'K'       {   VT_NOT_IMPLEMENTED;   }
            |   '\233' '?' 'K'              {   VT_NOT_IMPLEMENTED;   };

DECSCPP:        '\233' numbers '$' '|'      {   VT_NOT_IMPLEMENTED;   }
            |   '\233' '$' '|'              {   VT_NOT_IMPLEMENTED;   };
DECCARA:        '\233' numbers '$' 'r'      {   VT_NOT_IMPLEMENTED;   };
DECCRA:         '\233' numbers '$' 'v'      {   VT_NOT_IMPLEMENTED;   };
DECERA:         '\233' numbers '$' 'z'      {   VT_NOT_IMPLEMENTED;   };
DECFRA:         '\233' numbers '$' 'x'      {   VT_NOT_IMPLEMENTED;   };
DECRARA:        '\233' numbers '$' 't'      {   VT_NOT_IMPLEMENTED;   };
DECSERA:        '\233' numbers '$' '{'      {   VT_NOT_IMPLEMENTED;   };
DECSASD:        '\233' numbers '$' '}'      {   VT_NOT_IMPLEMENTED;   };
DECSSDT:        '\233' numbers '$' '~'      {   VT_NOT_IMPLEMENTED;   };

DECSACE:        '\233' numbers '*' 'x'      {   VT_NOT_IMPLEMENTED;   };
DECSNLS:        '\233' numbers '*' '|'      {   VT_NOT_IMPLEMENTED;   };
DECRQCRA:       '\233' numbers '*' 'y'      {   VT_NOT_IMPLEMENTED;   };

DSR:            '\233' '?' numbers 'n'      {   if (NUMS_LEN == 2)
                                                {
                                                    VT_DSR (GET_NUM (0), GET_NUM (1), TRUE);
                                                }
                                                else
                                                {
                                                    VT_DSR (GET_NUM (0), -1, TRUE);
                                                }
                                            }
            |   '\233' numbers 'n'          {   if (NUMS_LEN == 2)
                                                {
                                                    VT_DSR (GET_NUM (0), GET_NUM (1), FALSE);
                                                }
                                                else
                                                {
                                                    VT_DSR (GET_NUM (0), -1, FALSE);
                                                }
                                            };

DA1:            '\233' 'c'                  {   VT_DA1;             }
            |   '\233' number 'c'           {   VT_DA1;             };
DA2:            '\233' '>' 'c'              {   VT_DA2;             }
            |   '\233' '>' number 'c'       {   VT_DA2;             };
DA3:            '\233' '=' 'c'              {   VT_DA3;             }
            |   '\233' '=' number 'c'       {   VT_DA3;             };

DECTST:         '\233' numbers 'y'          {   VT_IGNORED;         };

DECSTR:         '\233' '!' 'p'              {   VT_DECSTR;          };


SGR:            '\233' numbers 'm'          {   VT_SGR;             }
            |   '\233' 'm'                  {   VT_SGR;             };
DECSET:         '\233' '?' numbers 'h'      {   VT_DECSET;          }
            |   '\233' '?' 'h'              {   VT_DECSET;          };
DECRST:         '\233' '?' numbers 'l'      {   VT_DECRST;          }
            |   '\233' '?' 'l'              {   VT_DECRST;          };
SM:             '\233' numbers 'h'          {   VT_SM;              }
            |   '\233' 'h'                  {   VT_SM;              };
RM:             '\233' numbers 'l'          {   VT_RM;              }
            |   '\233' 'l'                  {   VT_RM;              };
DECSAVE:        '\233' '?' numbers 's'      {   VT_DECSAVE;         }
            |   '\233' '?' 's'              {   VT_DECSAVE;         };
DECRESTORE:     '\233' '?' numbers 'r'      {   VT_DECRESTORE;      }
            |   '\233' '?' 'r'              {   VT_DECRESTORE;      };


/****************************************************************************/
/* Device control sequences
 */

device_control_sequence:    DECRQSS;


DECRQSS:    '\220' '$' 'q' DECRQSS_param '\234'     {   VT_DECRQSS (GET_NUM (0));  }

DECRQSS_param:  '$' 'g'         {   ADD_NUMBER (CODE_DECSASD);  }
            |   '"' 'p'         {   ADD_NUMBER (CODE_DECSCL);   }
            |   '$' '|'         {   ADD_NUMBER (CODE_DECSCPP);  }
            |   't'             {   ADD_NUMBER (CODE_DECSLPP);  }
            |   '*' '|'         {   ADD_NUMBER (CODE_DECSNLS);  }
            |   'r'             {   ADD_NUMBER (CODE_DECSTBM);  }
;


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

semicolons:     ';'
            |   semicolons ';'              {   ADD_NUMBER (-1);                    }
;

numbers:        number                      {   ADD_NUMBER ($1);                    }
            |   semicolons number           {   ADD_NUMBER (-1); ADD_NUMBER ($2);   }
            |   numbers semicolons number   {   ADD_NUMBER ($3);                    }
;
