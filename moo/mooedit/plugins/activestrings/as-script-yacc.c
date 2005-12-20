/* A Bison parser, made by GNU Bison 1.875d.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Written by Richard Stallman by simplifying the original so called
   ``semantic'' parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0

/* If NAME_PREFIX is specified substitute the variables and functions
   names.  */
#define yyparse _as_script_yyparse
#define yylex   _as_script_yylex
#define yyerror _as_script_yyerror
#define yylval  _as_script_yylval
#define yychar  _as_script_yychar
#define yydebug _as_script_yydebug
#define yynerrs _as_script_yynerrs


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     IDENTIFIER = 258,
     LITERAL = 259,
     VARIABLE = 260,
     NUMBER = 261,
     IF = 262,
     THEN = 263,
     ELSE = 264,
     ELIF = 265,
     WHILE = 266,
     REPEAT = 267,
     EQ = 268,
     NEQ = 269,
     LE = 270,
     GE = 271,
     AND = 272,
     OR = 273,
     NOT = 274,
     UMINUS = 275
   };
#endif
#define IDENTIFIER 258
#define LITERAL 259
#define VARIABLE 260
#define NUMBER 261
#define IF 262
#define THEN 263
#define ELSE 264
#define ELIF 265
#define WHILE 266
#define REPEAT 267
#define EQ 268
#define NEQ 269
#define LE 270
#define GE 271
#define AND 272
#define OR 273
#define NOT 274
#define UMINUS 275




/* Copy the first part of user declarations.  */
#line 1 "/home/muntyan/projects/moo/moo/mooedit/plugins/activestrings/as-script-yacc.y"

#include "as-script-parser.h"
#include "as-script-yacc.h"

#define NODE_LIST_ADD(list, node)           _as_parser_node_list_add (parser, AS_NODE_LIST (list), node)
#define NODE_COMMAND(id, node)              _as_parser_node_command (parser, id, AS_NODE_LIST (node))
#define NODE_IF_ELSE(cond, then_, else_)    _as_parser_node_if_else (parser, cond, then_, else_)
#define NODE_REPEAT(times, what)            _as_parser_node_repeat (parser, times, what)
#define NODE_WHILE(cond, what)              _as_parser_node_while (parser, cond, what)
#define NODE_ASSIGNMENT(var, val)           _as_parser_node_assignment (parser, AS_NODE_VAR (var), val)
#define BINARY_OP(op, lval, rval)           _as_parser_node_binary_op (parser, op, lval, rval)
#define UNARY_OP(op, val)                   _as_parser_node_unary_op (parser, op, val)
#define NODE_NUMBER(n)                      _as_parser_node_int (parser, n)
#define NODE_STRING(n)                      _as_parser_node_string (parser, n)
#define NODE_VALUE_LIST(list)               _as_parser_node_value_list (parser, AS_NODE_LIST (list))
#define VAR_POSITIONAL(n)                   _as_parser_node_var_pos (parser, n)
#define VAR_NAMED(string)                   _as_parser_node_var_named (parser, string)

#define SET_TOP_NODE(node)                  _as_parser_set_top_node (parser, node)


/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 24 "/home/muntyan/projects/moo/moo/mooedit/plugins/activestrings/as-script-yacc.y"
typedef union YYSTYPE {
    int ival;
    const char *str;
    ASNode *node;
} YYSTYPE;
/* Line 191 of yacc.c.  */
#line 152 "/home/muntyan/projects/moo/moo/mooedit/plugins/activestrings/as-script-yacc.c"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 214 of yacc.c.  */
#line 164 "/home/muntyan/projects/moo/moo/mooedit/plugins/activestrings/as-script-yacc.c"

#if ! defined (yyoverflow) || YYERROR_VERBOSE

# ifndef YYFREE
#  define YYFREE free
# endif
# ifndef YYMALLOC
#  define YYMALLOC malloc
# endif

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   define YYSTACK_ALLOC alloca
#  endif
# else
#  if defined (alloca) || defined (_ALLOCA_H)
#   define YYSTACK_ALLOC alloca
#  else
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
# else
#  if defined (__STDC__) || defined (__cplusplus)
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   define YYSIZE_T size_t
#  endif
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
# endif
#endif /* ! defined (yyoverflow) || YYERROR_VERBOSE */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (defined (YYSTYPE_IS_TRIVIAL) && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short int yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short int) + sizeof (YYSTYPE))			\
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined (__GNUC__) && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  register YYSIZE_T yyi;		\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (0)
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (0)

#endif

#if defined (__STDC__) || defined (__cplusplus)
   typedef signed char yysigned_char;
#else
   typedef short int yysigned_char;
#endif

/* YYFINAL -- State number of the termination state. */
#define YYFINAL  40
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   312

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  40
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  16
/* YYNRULES -- Number of rules. */
#define YYNRULES  52
/* YYNRULES -- Number of states. */
#define YYNSTATES  89

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   275

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,    39,    25,     2,     2,
      34,    35,    23,    22,    38,    21,     2,    24,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    32,    28,
      26,    33,    27,    31,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    36,     2,    37,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    29,     2,    30,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned char yyprhs[] =
{
       0,     0,     3,     5,     8,    12,    16,    18,    20,    22,
      24,    26,    28,    29,    31,    32,    34,    36,    39,    42,
      43,    46,    51,    58,    64,    68,    72,    76,    78,    82,
      86,    90,    94,    97,   101,   105,   108,   112,   116,   120,
     124,   128,   132,   134,   136,   138,   142,   146,   150,   151,
     153,   157,   160
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const yysigned_char yyrhs[] =
{
      41,     0,    -1,    42,    -1,    44,    28,    -1,    42,    44,
      28,    -1,    29,    45,    30,    -1,    46,    -1,    52,    -1,
      48,    -1,    49,    -1,    50,    -1,    51,    -1,    -1,    43,
      -1,    -1,    42,    -1,    43,    -1,    42,    43,    -1,     3,
      47,    -1,    -1,    47,    53,    -1,     7,    52,     8,    43,
      -1,     7,    52,     8,    43,     9,    43,    -1,    52,    31,
      52,    32,    52,    -1,    12,    53,    43,    -1,    11,    53,
      43,    -1,    55,    33,    52,    -1,    53,    -1,    52,    22,
      52,    -1,    52,    21,    52,    -1,    52,    24,    52,    -1,
      52,    23,    52,    -1,    21,    52,    -1,    52,    17,    52,
      -1,    52,    18,    52,    -1,    19,    52,    -1,    52,    13,
      52,    -1,    52,    14,    52,    -1,    52,    26,    52,    -1,
      52,    27,    52,    -1,    52,    15,    52,    -1,    52,    16,
      52,    -1,     6,    -1,     4,    -1,    55,    -1,    34,    45,
      35,    -1,    36,    54,    37,    -1,    53,    25,    53,    -1,
      -1,    52,    -1,    54,    38,    52,    -1,    39,     6,    -1,
      39,     3,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned char yyrline[] =
{
       0,    59,    59,    61,    62,    66,    67,    68,    69,    70,
      71,    72,    75,    76,    79,    80,    81,    82,    85,    88,
      89,    92,    93,    97,   100,   101,   105,   108,   109,   110,
     111,   112,   113,   115,   116,   117,   119,   120,   121,   122,
     123,   124,   128,   129,   130,   131,   132,   133,   136,   137,
     138,   141,   142
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "IDENTIFIER", "LITERAL", "VARIABLE",
  "NUMBER", "IF", "THEN", "ELSE", "ELIF", "WHILE", "REPEAT", "EQ", "NEQ",
  "LE", "GE", "AND", "OR", "NOT", "UMINUS", "'-'", "'+'", "'*'", "'/'",
  "'%'", "'<'", "'>'", "';'", "'{'", "'}'", "'?'", "':'", "'='", "'('",
  "')'", "'['", "']'", "','", "'$'", "$accept", "script", "program",
  "non_empty_stmt", "stmt", "stmt_or_program", "command", "expr_list",
  "if_stmt", "ternary", "loop", "assignment", "expr", "simple_expr",
  "list_elms", "variable", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short int yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,    45,    43,    42,    47,    37,    60,    62,    59,   123,
     125,    63,    58,    61,    40,    41,    91,    93,    44,    36
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    40,    41,    42,    42,    43,    43,    43,    43,    43,
      43,    43,    44,    44,    45,    45,    45,    45,    46,    47,
      47,    48,    48,    49,    50,    50,    51,    52,    52,    52,
      52,    52,    52,    52,    52,    52,    52,    52,    52,    52,
      52,    52,    53,    53,    53,    53,    53,    53,    54,    54,
      54,    55,    55
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     2,     3,     3,     1,     1,     1,     1,
       1,     1,     0,     1,     0,     1,     1,     2,     2,     0,
       2,     4,     6,     5,     3,     3,     3,     1,     3,     3,
       3,     3,     2,     3,     3,     2,     3,     3,     3,     3,
       3,     3,     1,     1,     1,     3,     3,     3,     0,     1,
       3,     2,     2
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
      12,    19,    43,    42,     0,     0,     0,     0,     0,    12,
      12,    48,     0,     0,     2,    13,     0,     6,     8,     9,
      10,    11,     7,    27,    44,    18,     0,    44,     0,     0,
      35,    32,    15,    16,     0,     0,    49,     0,    52,    51,
       1,     0,     3,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    20,     0,
      25,    24,    17,     5,    45,    46,     0,     4,    36,    37,
      40,    41,    33,    34,    29,    28,    31,    30,    38,    39,
       0,    47,    26,    21,    50,     0,     0,    23,    22
};

/* YYDEFGOTO[NTERM-NUM]. */
static const yysigned_char yydefgoto[] =
{
      -1,    13,    32,    15,    16,    34,    17,    25,    18,    19,
      20,    21,    22,    23,    37,    27
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -21
static const short int yypact[] =
{
     168,   -21,   -21,   -21,   172,    19,    19,   172,   172,    62,
      96,   172,    10,     3,   115,   -21,   -13,   -21,   -21,   -21,
     -21,   -21,   236,    -6,    -9,    19,   201,   -21,   134,   134,
     -21,   -21,   115,    -1,     5,    -2,   255,   -20,   -21,   -21,
     -21,     8,   -21,   172,   172,   172,   172,   172,   172,   172,
     172,   172,   172,   172,   172,   172,    19,   172,    -6,   168,
     -21,   -21,    -1,   -21,   -21,   -21,   172,   -21,     4,     4,
       4,     4,   -21,    20,   270,   270,   285,   285,     4,     4,
     216,   -21,   255,    29,   255,   172,   168,   255,   -21
};

/* YYPGOTO[NTERM-NUM].  */
static const yysigned_char yypgoto[] =
{
     -21,   -21,    39,     2,   -12,    46,   -21,   -21,   -21,   -21,
     -21,   -21,    -3,     1,   -21,     0
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -15
static const yysigned_char yytable[] =
{
      24,    26,    41,    40,    30,    31,    28,    29,    36,    24,
      24,    33,    33,    38,    24,    42,    39,    65,    66,    56,
      41,    47,    48,     2,    57,     3,    58,   -13,    24,    24,
      60,    61,    24,    64,    62,    63,    67,    47,    86,    14,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    10,    82,    11,    35,    81,    12,    24,
       0,    83,     0,    84,     0,     1,     2,     0,     3,     4,
       0,     0,     0,     5,     6,     0,     0,     0,     0,     0,
       0,     7,    87,     8,     0,     0,    24,     0,    88,     0,
       0,     9,   -14,     0,     0,     0,    10,     0,    11,     1,
       2,    12,     3,     4,     0,     0,     0,     5,     6,     0,
       0,     0,     0,     0,     0,     7,     0,     8,     1,     2,
       0,     3,     4,     0,     0,     9,     5,     6,     0,     0,
      10,   -14,    11,     0,     7,    12,     8,     1,     2,     0,
       3,     4,     0,   -12,     9,     5,     6,     0,     0,    10,
       0,    11,     0,     7,    12,     8,     0,     0,     0,    56,
       0,     0,     0,     9,     0,     0,     0,     0,    10,     0,
      11,     1,     2,    12,     3,     4,     2,     0,     3,     5,
       6,     0,     0,     0,     0,     0,     0,     7,     0,     8,
       0,     7,     0,     8,     0,     0,     0,     9,     0,     0,
       0,     0,    10,     0,    11,     0,    10,    12,    11,    59,
       0,    12,     0,     0,    43,    44,    45,    46,    47,    48,
       0,     0,    49,    50,    51,    52,     0,    53,    54,    43,
      44,    45,    46,    47,    48,     0,     0,    49,    50,    51,
      52,     0,    53,    54,     0,     0,     0,     0,    85,    43,
      44,    45,    46,    47,    48,     0,     0,    49,    50,    51,
      52,     0,    53,    54,     0,     0,     0,    55,    43,    44,
      45,    46,    47,    48,     0,     0,    49,    50,    51,    52,
       0,    53,    54,    43,    44,    45,    46,    47,    48,     0,
       0,     0,     0,    51,    52,     0,    53,    54,    43,    44,
      45,    46,    47,    48,     0,     0,     0,     0,     0,     0,
       0,    53,    54
};

static const yysigned_char yycheck[] =
{
       0,     4,    14,     0,     7,     8,     5,     6,    11,     9,
      10,     9,    10,     3,    14,    28,     6,    37,    38,    25,
      32,    17,    18,     4,    33,     6,    25,    28,    28,    29,
      28,    29,    32,    35,    32,    30,    28,    17,     9,     0,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    34,    57,    36,    10,    56,    39,    59,
      -1,    59,    -1,    66,    -1,     3,     4,    -1,     6,     7,
      -1,    -1,    -1,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    19,    85,    21,    -1,    -1,    86,    -1,    86,    -1,
      -1,    29,    30,    -1,    -1,    -1,    34,    -1,    36,     3,
       4,    39,     6,     7,    -1,    -1,    -1,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    19,    -1,    21,     3,     4,
      -1,     6,     7,    -1,    -1,    29,    11,    12,    -1,    -1,
      34,    35,    36,    -1,    19,    39,    21,     3,     4,    -1,
       6,     7,    -1,    28,    29,    11,    12,    -1,    -1,    34,
      -1,    36,    -1,    19,    39,    21,    -1,    -1,    -1,    25,
      -1,    -1,    -1,    29,    -1,    -1,    -1,    -1,    34,    -1,
      36,     3,     4,    39,     6,     7,     4,    -1,     6,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    19,    -1,    21,
      -1,    19,    -1,    21,    -1,    -1,    -1,    29,    -1,    -1,
      -1,    -1,    34,    -1,    36,    -1,    34,    39,    36,     8,
      -1,    39,    -1,    -1,    13,    14,    15,    16,    17,    18,
      -1,    -1,    21,    22,    23,    24,    -1,    26,    27,    13,
      14,    15,    16,    17,    18,    -1,    -1,    21,    22,    23,
      24,    -1,    26,    27,    -1,    -1,    -1,    -1,    32,    13,
      14,    15,    16,    17,    18,    -1,    -1,    21,    22,    23,
      24,    -1,    26,    27,    -1,    -1,    -1,    31,    13,    14,
      15,    16,    17,    18,    -1,    -1,    21,    22,    23,    24,
      -1,    26,    27,    13,    14,    15,    16,    17,    18,    -1,
      -1,    -1,    -1,    23,    24,    -1,    26,    27,    13,    14,
      15,    16,    17,    18,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    26,    27
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,     3,     4,     6,     7,    11,    12,    19,    21,    29,
      34,    36,    39,    41,    42,    43,    44,    46,    48,    49,
      50,    51,    52,    53,    55,    47,    52,    55,    53,    53,
      52,    52,    42,    43,    45,    45,    52,    54,     3,     6,
       0,    44,    28,    13,    14,    15,    16,    17,    18,    21,
      22,    23,    24,    26,    27,    31,    25,    33,    53,     8,
      43,    43,    43,    30,    35,    37,    38,    28,    52,    52,
      52,    52,    52,    52,    52,    52,    52,    52,    52,    52,
      52,    53,    52,    43,    52,    32,     9,    52,    43
};

#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# if defined (__STDC__) || defined (__cplusplus)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# endif
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { 								\
      yyerror (parser, "syntax error: cannot back up");\
      YYERROR;							\
    }								\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

/* YYLLOC_DEFAULT -- Compute the default location (before the actions
   are run).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)		\
   ((Current).first_line   = (Rhs)[1].first_line,	\
    (Current).first_column = (Rhs)[1].first_column,	\
    (Current).last_line    = (Rhs)[N].last_line,	\
    (Current).last_column  = (Rhs)[N].last_column)
#endif

/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex (parser)
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (0)

# define YYDSYMPRINT(Args)			\
do {						\
  if (yydebug)					\
    yysymprint Args;				\
} while (0)

# define YYDSYMPRINTF(Title, Token, Value, Location)		\
do {								\
  if (yydebug)							\
    {								\
      YYFPRINTF (stderr, "%s ", Title);				\
      yysymprint (stderr, 					\
                  Token, Value);	\
      YYFPRINTF (stderr, "\n");					\
    }								\
} while (0)

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_stack_print (short int *bottom, short int *top)
#else
static void
yy_stack_print (bottom, top)
    short int *bottom;
    short int *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (/* Nothing. */; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_reduce_print (int yyrule)
#else
static void
yy_reduce_print (yyrule)
    int yyrule;
#endif
{
  int yyi;
  unsigned int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %u), ",
             yyrule - 1, yylno);
  /* Print the symbols being reduced, and their result.  */
  for (yyi = yyprhs[yyrule]; 0 <= yyrhs[yyi]; yyi++)
    YYFPRINTF (stderr, "%s ", yytname [yyrhs[yyi]]);
  YYFPRINTF (stderr, "-> %s\n", yytname [yyr1[yyrule]]);
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (Rule);		\
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YYDSYMPRINT(Args)
# define YYDSYMPRINTF(Title, Token, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   SIZE_MAX < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#if defined (YYMAXDEPTH) && YYMAXDEPTH == 0
# undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  register const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
{
  register char *yyd = yydest;
  register const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

#endif /* !YYERROR_VERBOSE */



#if YYDEBUG
/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yysymprint (FILE *yyoutput, int yytype, YYSTYPE *yyvaluep)
#else
static void
yysymprint (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  if (yytype < YYNTOKENS)
    {
      YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
# ifdef YYPRINT
      YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
    }
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  switch (yytype)
    {
      default:
        break;
    }
  YYFPRINTF (yyoutput, ")");
}

#endif /* ! YYDEBUG */
/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yydestruct (int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yytype, yyvaluep)
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  switch (yytype)
    {

      default:
        break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM);
# else
int yyparse ();
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int yyparse (ASParser *parser);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM)
# else
int yyparse (YYPARSE_PARAM)
  void *YYPARSE_PARAM;
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int
yyparse (ASParser *parser)
#else
int
yyparse (parser)
    ASParser *parser;
#endif
#endif
{
  
  register int yystate;
  register int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  short int yyssa[YYINITDEPTH];
  short int *yyss = yyssa;
  register short int *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;



#define YYPOPSTACK   (yyvsp--, yyssp--)

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* When reducing, the number of symbols on the RHS of the reduced
     rule.  */
  int yylen;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;


  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	short int *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyoverflowlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyoverflowlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	short int *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyoverflowlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YYDSYMPRINTF ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */
  YYDPRINTF ((stderr, "Shifting token %s, ", yytname[yytoken]));

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;


  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 59 "/home/muntyan/projects/moo/moo/mooedit/plugins/activestrings/as-script-yacc.y"
    { SET_TOP_NODE (yyvsp[0].node); ;}
    break;

  case 3:
#line 61 "/home/muntyan/projects/moo/moo/mooedit/plugins/activestrings/as-script-yacc.y"
    { yyval.node = NODE_LIST_ADD (NULL, yyvsp[-1].node); ;}
    break;

  case 4:
#line 62 "/home/muntyan/projects/moo/moo/mooedit/plugins/activestrings/as-script-yacc.y"
    { yyval.node = NODE_LIST_ADD (yyvsp[-2].node, yyvsp[-1].node); ;}
    break;

  case 5:
#line 66 "/home/muntyan/projects/moo/moo/mooedit/plugins/activestrings/as-script-yacc.y"
    { yyval.node = yyvsp[-1].node; ;}
    break;

  case 12:
#line 75 "/home/muntyan/projects/moo/moo/mooedit/plugins/activestrings/as-script-yacc.y"
    { yyval.node = NULL; ;}
    break;

  case 14:
#line 79 "/home/muntyan/projects/moo/moo/mooedit/plugins/activestrings/as-script-yacc.y"
    { yyval.node = NULL; ;}
    break;

  case 17:
#line 82 "/home/muntyan/projects/moo/moo/mooedit/plugins/activestrings/as-script-yacc.y"
    { yyval.node = NODE_LIST_ADD (yyvsp[-1].node, yyvsp[0].node); ;}
    break;

  case 18:
#line 85 "/home/muntyan/projects/moo/moo/mooedit/plugins/activestrings/as-script-yacc.y"
    { yyval.node = NODE_COMMAND (yyvsp[-1].str, yyvsp[0].node); ;}
    break;

  case 19:
#line 88 "/home/muntyan/projects/moo/moo/mooedit/plugins/activestrings/as-script-yacc.y"
    { yyval.node = NULL; ;}
    break;

  case 20:
#line 89 "/home/muntyan/projects/moo/moo/mooedit/plugins/activestrings/as-script-yacc.y"
    { yyval.node = NODE_LIST_ADD (yyvsp[-1].node, yyvsp[0].node); ;}
    break;

  case 21:
#line 92 "/home/muntyan/projects/moo/moo/mooedit/plugins/activestrings/as-script-yacc.y"
    { yyval.node = NODE_IF_ELSE (yyvsp[-2].node, yyvsp[0].node, NULL); ;}
    break;

  case 22:
#line 94 "/home/muntyan/projects/moo/moo/mooedit/plugins/activestrings/as-script-yacc.y"
    { yyval.node = NODE_IF_ELSE (yyvsp[-4].node, yyvsp[-2].node, yyvsp[0].node); ;}
    break;

  case 23:
#line 97 "/home/muntyan/projects/moo/moo/mooedit/plugins/activestrings/as-script-yacc.y"
    { yyval.node = NODE_IF_ELSE (yyvsp[-4].node, yyvsp[-2].node, yyvsp[0].node); ;}
    break;

  case 24:
#line 100 "/home/muntyan/projects/moo/moo/mooedit/plugins/activestrings/as-script-yacc.y"
    { yyval.node = NODE_REPEAT (yyvsp[-1].node, yyvsp[0].node); ;}
    break;

  case 25:
#line 101 "/home/muntyan/projects/moo/moo/mooedit/plugins/activestrings/as-script-yacc.y"
    { yyval.node = NODE_WHILE (yyvsp[-1].node, yyvsp[0].node); ;}
    break;

  case 26:
#line 105 "/home/muntyan/projects/moo/moo/mooedit/plugins/activestrings/as-script-yacc.y"
    { yyval.node = NODE_ASSIGNMENT (yyvsp[-2].node, yyvsp[0].node); ;}
    break;

  case 28:
#line 109 "/home/muntyan/projects/moo/moo/mooedit/plugins/activestrings/as-script-yacc.y"
    { yyval.node = BINARY_OP (AS_OP_PLUS, yyvsp[-2].node, yyvsp[0].node); ;}
    break;

  case 29:
#line 110 "/home/muntyan/projects/moo/moo/mooedit/plugins/activestrings/as-script-yacc.y"
    { yyval.node = BINARY_OP (AS_OP_MINUS, yyvsp[-2].node, yyvsp[0].node); ;}
    break;

  case 30:
#line 111 "/home/muntyan/projects/moo/moo/mooedit/plugins/activestrings/as-script-yacc.y"
    { yyval.node = BINARY_OP (AS_OP_DIV, yyvsp[-2].node, yyvsp[0].node); ;}
    break;

  case 31:
#line 112 "/home/muntyan/projects/moo/moo/mooedit/plugins/activestrings/as-script-yacc.y"
    { yyval.node = BINARY_OP (AS_OP_MULT, yyvsp[-2].node, yyvsp[0].node); ;}
    break;

  case 32:
#line 113 "/home/muntyan/projects/moo/moo/mooedit/plugins/activestrings/as-script-yacc.y"
    { yyval.node = UNARY_OP (AS_OP_UMINUS, yyvsp[0].node); ;}
    break;

  case 33:
#line 115 "/home/muntyan/projects/moo/moo/mooedit/plugins/activestrings/as-script-yacc.y"
    { yyval.node = BINARY_OP (AS_OP_AND, yyvsp[-2].node, yyvsp[0].node); ;}
    break;

  case 34:
#line 116 "/home/muntyan/projects/moo/moo/mooedit/plugins/activestrings/as-script-yacc.y"
    { yyval.node = BINARY_OP (AS_OP_OR, yyvsp[-2].node, yyvsp[0].node); ;}
    break;

  case 35:
#line 117 "/home/muntyan/projects/moo/moo/mooedit/plugins/activestrings/as-script-yacc.y"
    { yyval.node = UNARY_OP (AS_OP_NOT, yyvsp[0].node); ;}
    break;

  case 36:
#line 119 "/home/muntyan/projects/moo/moo/mooedit/plugins/activestrings/as-script-yacc.y"
    { yyval.node = BINARY_OP (AS_OP_EQ, yyvsp[-2].node, yyvsp[0].node); ;}
    break;

  case 37:
#line 120 "/home/muntyan/projects/moo/moo/mooedit/plugins/activestrings/as-script-yacc.y"
    { yyval.node = BINARY_OP (AS_OP_NEQ, yyvsp[-2].node, yyvsp[0].node); ;}
    break;

  case 38:
#line 121 "/home/muntyan/projects/moo/moo/mooedit/plugins/activestrings/as-script-yacc.y"
    { yyval.node = BINARY_OP (AS_OP_LT, yyvsp[-2].node, yyvsp[0].node); ;}
    break;

  case 39:
#line 122 "/home/muntyan/projects/moo/moo/mooedit/plugins/activestrings/as-script-yacc.y"
    { yyval.node = BINARY_OP (AS_OP_GT, yyvsp[-2].node, yyvsp[0].node); ;}
    break;

  case 40:
#line 123 "/home/muntyan/projects/moo/moo/mooedit/plugins/activestrings/as-script-yacc.y"
    { yyval.node = BINARY_OP (AS_OP_LE, yyvsp[-2].node, yyvsp[0].node); ;}
    break;

  case 41:
#line 124 "/home/muntyan/projects/moo/moo/mooedit/plugins/activestrings/as-script-yacc.y"
    { yyval.node = BINARY_OP (AS_OP_GE, yyvsp[-2].node, yyvsp[0].node); ;}
    break;

  case 42:
#line 128 "/home/muntyan/projects/moo/moo/mooedit/plugins/activestrings/as-script-yacc.y"
    { yyval.node = NODE_NUMBER (yyvsp[0].ival); ;}
    break;

  case 43:
#line 129 "/home/muntyan/projects/moo/moo/mooedit/plugins/activestrings/as-script-yacc.y"
    { yyval.node = NODE_STRING (yyvsp[0].str); ;}
    break;

  case 45:
#line 131 "/home/muntyan/projects/moo/moo/mooedit/plugins/activestrings/as-script-yacc.y"
    { yyval.node = yyvsp[-1].node; ;}
    break;

  case 46:
#line 132 "/home/muntyan/projects/moo/moo/mooedit/plugins/activestrings/as-script-yacc.y"
    { yyval.node = NODE_VALUE_LIST (yyvsp[-1].node); ;}
    break;

  case 47:
#line 133 "/home/muntyan/projects/moo/moo/mooedit/plugins/activestrings/as-script-yacc.y"
    { yyval.node = BINARY_OP (AS_OP_PRINT, yyvsp[-2].node, yyvsp[0].node); ;}
    break;

  case 48:
#line 136 "/home/muntyan/projects/moo/moo/mooedit/plugins/activestrings/as-script-yacc.y"
    { yyval.node = NULL; ;}
    break;

  case 49:
#line 137 "/home/muntyan/projects/moo/moo/mooedit/plugins/activestrings/as-script-yacc.y"
    { yyval.node = NODE_LIST_ADD (NULL, yyvsp[0].node); ;}
    break;

  case 50:
#line 138 "/home/muntyan/projects/moo/moo/mooedit/plugins/activestrings/as-script-yacc.y"
    { yyval.node = NODE_LIST_ADD (yyvsp[-2].node, yyvsp[0].node); ;}
    break;

  case 51:
#line 141 "/home/muntyan/projects/moo/moo/mooedit/plugins/activestrings/as-script-yacc.y"
    { yyval.node = VAR_POSITIONAL (yyvsp[0].ival); ;}
    break;

  case 52:
#line 142 "/home/muntyan/projects/moo/moo/mooedit/plugins/activestrings/as-script-yacc.y"
    { yyval.node = VAR_NAMED (yyvsp[0].str); ;}
    break;


    }

/* Line 1010 of yacc.c.  */
#line 1368 "/home/muntyan/projects/moo/moo/mooedit/plugins/activestrings/as-script-yacc.c"

  yyvsp -= yylen;
  yyssp -= yylen;


  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (YYPACT_NINF < yyn && yyn < YYLAST)
	{
	  YYSIZE_T yysize = 0;
	  int yytype = YYTRANSLATE (yychar);
	  const char* yyprefix;
	  char *yymsg;
	  int yyx;

	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  int yyxbegin = yyn < 0 ? -yyn : 0;

	  /* Stay within bounds of both yycheck and yytname.  */
	  int yychecklim = YYLAST - yyn;
	  int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
	  int yycount = 0;

	  yyprefix = ", expecting ";
	  for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	      {
		yysize += yystrlen (yyprefix) + yystrlen (yytname [yyx]);
		yycount += 1;
		if (yycount == 5)
		  {
		    yysize = 0;
		    break;
		  }
	      }
	  yysize += (sizeof ("syntax error, unexpected ")
		     + yystrlen (yytname[yytype]));
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "syntax error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[yytype]);

	      if (yycount < 5)
		{
		  yyprefix = ", expecting ";
		  for (yyx = yyxbegin; yyx < yyxend; ++yyx)
		    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
		      {
			yyp = yystpcpy (yyp, yyprefix);
			yyp = yystpcpy (yyp, yytname[yyx]);
			yyprefix = " or ";
		      }
		}
	      yyerror (parser, yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    yyerror (parser, "syntax error; also virtual memory exhausted");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror (parser, "syntax error");
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* If at end of input, pop the error token,
	     then the rest of the stack, then return failure.  */
	  if (yychar == YYEOF)
	     for (;;)
	       {
		 YYPOPSTACK;
		 if (yyssp == yyss)
		   YYABORT;
		 YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
		 yydestruct (yystos[*yyssp], yyvsp);
	       }
        }
      else
	{
	  YYDSYMPRINTF ("Error: discarding", yytoken, &yylval, &yylloc);
	  yydestruct (yytoken, &yylval);
	  yychar = YYEMPTY;

	}
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

#ifdef __GNUC__
  /* Pacify GCC when the user code never invokes YYERROR and the label
     yyerrorlab therefore never appears in user code.  */
  if (0)
     goto yyerrorlab;
#endif

  yyvsp -= yylen;
  yyssp -= yylen;
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;

      YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
      yydestruct (yystos[yystate], yyvsp);
      YYPOPSTACK;
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  YYDPRINTF ((stderr, "Shifting error token, "));

  *++yyvsp = yylval;


  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*----------------------------------------------.
| yyoverflowlab -- parser overflow comes here.  |
`----------------------------------------------*/
yyoverflowlab:
  yyerror (parser, "parser stack overflow");
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}


#line 145 "/home/muntyan/projects/moo/moo/mooedit/plugins/activestrings/as-script-yacc.y"


