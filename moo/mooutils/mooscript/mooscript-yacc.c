/* A Bison parser, made by GNU Bison 2.1.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005 Free Software Foundation, Inc.

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
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

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

/* Bison version.  */
#define YYBISON_VERSION "2.1"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0

/* Substitute the variable and function names.  */
#define yyparse _ms_script_yyparse
#define yylex   _ms_script_yylex
#define yyerror _ms_script_yyerror
#define yylval  _ms_script_yylval
#define yychar  _ms_script_yychar
#define yydebug _ms_script_yydebug
#define yynerrs _ms_script_yynerrs


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     IDENTIFIER = 258,
     LITERAL = 259,
     VARIABLE = 260,
     PYTHON = 261,
     NUMBER = 262,
     IF = 263,
     THEN = 264,
     ELSE = 265,
     FI = 266,
     WHILE = 267,
     DO = 268,
     OD = 269,
     FOR = 270,
     IN = 271,
     EQ = 272,
     NEQ = 273,
     LE = 274,
     GE = 275,
     AND = 276,
     OR = 277,
     NOT = 278,
     UMINUS = 279,
     TWODOTS = 280
   };
#endif
/* Tokens.  */
#define IDENTIFIER 258
#define LITERAL 259
#define VARIABLE 260
#define PYTHON 261
#define NUMBER 262
#define IF 263
#define THEN 264
#define ELSE 265
#define FI 266
#define WHILE 267
#define DO 268
#define OD 269
#define FOR 270
#define IN 271
#define EQ 272
#define NEQ 273
#define LE 274
#define GE 275
#define AND 276
#define OR 277
#define NOT 278
#define UMINUS 279
#define TWODOTS 280




/* Copy the first part of user declarations.  */
#line 1 "/home/muntyan/projects/moo/moo/mooutils/mooscript/mooscript-yacc.y"

#include "mooscript-parser.h"
#include "mooscript-yacc.h"

#define NODE_LIST_ADD(list, node)           _ms_parser_node_list_add (parser, MS_NODE_LIST (list), node)
#define NODE_COMMAND(id, node)              _ms_parser_node_command (parser, id, MS_NODE_LIST (node))
#define NODE_IF_ELSE(cond, then_, else_)    _ms_parser_node_if_else (parser, cond, then_, else_)
#define NODE_WHILE(cond, what)              _ms_parser_node_while (parser, cond, what)
#define NODE_DO_WHILE(cond, what)           _ms_parser_node_do_while (parser, cond, what)
#define NODE_FOR(var, list, what)           _ms_parser_node_for (parser, var, list, what)
#define NODE_ASSIGNMENT(var, val)           _ms_parser_node_assignment (parser, var, val)
#define BINARY_OP(op, lval, rval)           _ms_parser_node_binary_op (parser, op, lval, rval)
#define UNARY_OP(op, val)                   _ms_parser_node_unary_op (parser, op, val)
#define NODE_NUMBER(n)                      _ms_parser_node_int (parser, n)
#define NODE_STRING(n)                      _ms_parser_node_string (parser, n)
#define NODE_PYTHON(string)                 _ms_parser_node_python (parser, string)
#define NODE_VALUE_LIST(list)               _ms_parser_node_value_list (parser, MS_NODE_LIST (list))
#define NODE_VALUE_RANGE(first,second)      _ms_parser_node_value_range (parser, first, second)
#define NODE_VAR(string)                    _ms_parser_node_var (parser, string)

#define SET_TOP_NODE(node)                  _ms_parser_set_top_node (parser, node)


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

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif

#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 26 "/home/muntyan/projects/moo/moo/mooutils/mooscript/mooscript-yacc.y"
typedef union YYSTYPE {
    int ival;
    const char *str;
    MSNode *node;
} YYSTYPE;
/* Line 196 of yacc.c.  */
#line 172 "/home/muntyan/projects/moo/moo/mooutils/mooscript/mooscript-yacc.c"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 219 of yacc.c.  */
#line 184 "/home/muntyan/projects/moo/moo/mooutils/mooscript/mooscript-yacc.c"

#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T) && (defined (__STDC__) || defined (__cplusplus))
# include <stddef.h> /* INFRINGES ON USER NAME SPACE */
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

#if ! defined (yyoverflow) || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if defined (__STDC__) || defined (__cplusplus)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     define YYINCLUDED_STDLIB_H
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2005 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM ((YYSIZE_T) -1)
#  endif
#  ifdef __cplusplus
extern "C" {
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if (! defined (malloc) && ! defined (YYINCLUDED_STDLIB_H) \
	&& (defined (__STDC__) || defined (__cplusplus)))
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if (! defined (free) && ! defined (YYINCLUDED_STDLIB_H) \
	&& (defined (__STDC__) || defined (__cplusplus)))
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifdef __cplusplus
}
#  endif
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
	  YYSIZE_T yyi;				\
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
#define YYLAST   450

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  43
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  15
/* YYNRULES -- Number of rules. */
#define YYNRULES  49
/* YYNRULES -- Number of states. */
#define YYNSTATES  100

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   280

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,    33,     2,    30,     2,     2,
      38,    39,    28,    27,    42,    26,     2,    29,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    37,    35,
      31,    34,    32,    36,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    40,     2,    41,     2,     2,     2,     2,     2,     2,
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
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned char yyprhs[] =
{
       0,     0,     3,     5,     7,    10,    13,    15,    16,    18,
      20,    22,    28,    33,    41,    47,    55,    57,    59,    61,
      63,    65,    69,    75,    79,    83,    87,    91,    95,    99,
     103,   107,   111,   115,   119,   123,   125,   127,   129,   133,
     137,   143,   147,   150,   153,   156,   157,   159,   163,   165
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const yysigned_char yyrhs[] =
{
      44,     0,    -1,    45,    -1,    46,    -1,    45,    46,    -1,
      47,    35,    -1,     6,    -1,    -1,    50,    -1,    49,    -1,
      48,    -1,    12,    50,    13,    45,    14,    -1,    13,    45,
      12,    50,    -1,    15,    56,    16,    50,    13,    45,    14,
      -1,     8,    50,     9,    45,    11,    -1,     8,    50,     9,
      45,    10,    45,    11,    -1,    54,    -1,    53,    -1,    51,
      -1,    57,    -1,    52,    -1,     3,    34,    50,    -1,    54,
      36,    54,    37,    54,    -1,    50,    27,    50,    -1,    50,
      26,    50,    -1,    50,    29,    50,    -1,    50,    28,    50,
      -1,    50,    21,    50,    -1,    50,    22,    50,    -1,    50,
      17,    50,    -1,    50,    18,    50,    -1,    50,    31,    50,
      -1,    50,    32,    50,    -1,    50,    19,    50,    -1,    50,
      20,    50,    -1,     7,    -1,     4,    -1,    56,    -1,    38,
      47,    39,    -1,    40,    55,    41,    -1,    40,    50,    25,
      50,    41,    -1,    54,    30,    54,    -1,    33,    54,    -1,
      23,    54,    -1,    26,    54,    -1,    -1,    50,    -1,    55,
      42,    50,    -1,     3,    -1,     3,    38,    55,    39,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned char yyrline[] =
{
       0,    66,    66,    69,    70,    74,    75,    78,    79,    80,
      81,    84,    85,    86,    90,    91,    94,    95,    96,    97,
      98,   102,   105,   109,   110,   111,   112,   114,   115,   117,
     118,   119,   120,   121,   122,   126,   127,   128,   129,   130,
     131,   132,   133,   134,   135,   138,   139,   140,   143,   146
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "IDENTIFIER", "LITERAL", "VARIABLE",
  "PYTHON", "NUMBER", "IF", "THEN", "ELSE", "FI", "WHILE", "DO", "OD",
  "FOR", "IN", "EQ", "NEQ", "LE", "GE", "AND", "OR", "NOT", "UMINUS",
  "TWODOTS", "'-'", "'+'", "'*'", "'/'", "'%'", "'<'", "'>'", "'#'", "'='",
  "';'", "'?'", "':'", "'('", "')'", "'['", "']'", "','", "$accept",
  "script", "program", "stmt_or_python", "stmt", "loop", "if_stmt", "expr",
  "assignment", "ternary", "compound_expr", "simple_expr", "list_elms",
  "variable", "function", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short int yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,    45,    43,    42,    47,
      37,    60,    62,    35,    61,    59,    63,    58,    40,    41,
      91,    93,    44
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    43,    44,    45,    45,    46,    46,    47,    47,    47,
      47,    48,    48,    48,    49,    49,    50,    50,    50,    50,
      50,    51,    52,    53,    53,    53,    53,    53,    53,    53,
      53,    53,    53,    53,    53,    54,    54,    54,    54,    54,
      54,    54,    54,    54,    54,    55,    55,    55,    56,    57
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     1,     2,     2,     1,     0,     1,     1,
       1,     5,     4,     7,     5,     7,     1,     1,     1,     1,
       1,     3,     5,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     1,     1,     1,     3,     3,
       5,     3,     2,     2,     2,     0,     1,     3,     1,     4
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       7,    48,    36,     6,    35,     0,     0,     7,     0,     0,
       0,     0,     7,    45,     0,     2,     3,     0,    10,     9,
       8,    18,    20,    17,    16,    37,    19,     0,    45,     0,
       0,     7,    48,     0,    43,    44,    42,     0,    46,     0,
       1,     4,     5,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    21,    46,     0,
       7,     7,     0,     0,    38,     0,    39,     0,    29,    30,
      33,    34,    27,    28,    24,    23,    26,    25,    31,    32,
      41,     0,    49,     7,     7,    12,     0,     0,    47,     0,
       7,    14,    11,     7,    40,    22,     7,     7,    15,    13
};

/* YYDEFGOTO[NTERM-NUM]. */
static const yysigned_char yydefgoto[] =
{
      -1,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    39,    25,    26
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -30
static const short int yypact[] =
{
     232,   -22,   -30,   -30,   -30,     1,     1,   232,    -1,   102,
     102,   102,   280,     1,    18,   108,   -30,   -20,   -30,   -30,
     386,   -30,   -30,   -30,   -29,   -30,   -30,     1,     1,   320,
     337,   256,   -30,    13,   -30,   -30,   -30,    -9,   370,   -19,
     -30,   -30,   -30,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,   102,   102,   -30,   386,   -25,
     232,   232,     1,     1,   -30,     1,   -30,     1,     4,     4,
       4,     4,   -30,    10,   402,   402,   418,   418,     4,     4,
     -30,   -24,   -30,    84,   146,   337,   354,   304,   386,   102,
     232,   -30,   -30,   232,   -30,     2,   170,   208,   -30,   -30
};

/* YYPGOTO[NTERM-NUM].  */
static const yysigned_char yypgoto[] =
{
     -30,   -30,    -7,   -12,    21,   -30,   -30,    15,   -30,   -30,
     -30,     0,     7,    28,   -30
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -8
static const yysigned_char yytable[] =
{
      31,    55,    32,    41,     1,     2,    55,    56,     4,    34,
      35,    36,    27,    89,    82,    42,    28,    67,    40,    41,
      29,    30,    66,    67,     9,    47,    48,    10,    38,    63,
      64,    47,    55,    37,    11,    59,    33,     0,     0,    12,
       0,    13,    57,    58,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    83,    84,    80,    81,     0,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
       0,    41,    41,     0,     0,     0,     0,    85,    86,     0,
      87,     0,    88,    96,    41,    41,    97,     1,     2,    95,
       3,     4,     5,     0,    90,    91,     6,     7,     0,     8,
       0,     0,     0,     0,     0,    32,     2,     9,     0,     4,
      10,     1,     2,     0,     3,     4,     5,    11,     0,     0,
       6,     7,    12,     8,    13,     9,     0,     0,    10,     0,
       0,     9,     0,     0,    10,    11,     0,     0,     0,     0,
      12,    11,    13,    -7,     0,     0,    12,     0,    13,     1,
       2,     0,     3,     4,     5,     0,     0,     0,     6,     7,
      92,     8,     0,     0,     0,     0,     0,     0,     0,     9,
       0,     0,    10,     1,     2,     0,     3,     4,     5,    11,
       0,    98,     6,     7,    12,     8,    13,     0,     0,     0,
       0,     0,     0,     9,     0,     0,    10,     0,     0,     0,
       0,     0,     0,    11,     0,     0,     0,     0,    12,     0,
      13,     1,     2,     0,     3,     4,     5,     0,     0,     0,
       6,     7,    99,     8,     0,     0,     0,     0,     0,     0,
       0,     9,     0,     0,    10,     1,     2,     0,     3,     4,
       5,    11,     0,     0,     6,     7,    12,     8,    13,     0,
       0,     0,     0,     0,     0,     9,     0,     0,    10,     1,
       2,     0,     3,     4,     5,    11,     0,     0,    62,     7,
      12,     8,    13,     0,     0,     0,     0,     0,     0,     9,
       0,     0,    10,     1,     2,     0,     0,     4,     5,    11,
       0,     0,     6,     7,    12,     8,    13,     0,     0,     0,
       0,     0,     0,     9,     0,     0,    10,     0,     0,     0,
       0,     0,     0,    11,     0,     0,     0,     0,    12,     0,
      13,    43,    44,    45,    46,    47,    48,     0,     0,    60,
      49,    50,    51,    52,     0,    53,    54,    43,    44,    45,
      46,    47,    48,     0,     0,    94,    49,    50,    51,    52,
      61,    53,    54,     0,    43,    44,    45,    46,    47,    48,
       0,     0,     0,    49,    50,    51,    52,    93,    53,    54,
       0,    43,    44,    45,    46,    47,    48,     0,     0,     0,
      49,    50,    51,    52,     0,    53,    54,    43,    44,    45,
      46,    47,    48,     0,     0,    65,    49,    50,    51,    52,
       0,    53,    54,    43,    44,    45,    46,    47,    48,     0,
       0,     0,    49,    50,    51,    52,     0,    53,    54,    43,
      44,    45,    46,    47,    48,     0,     0,     0,     0,     0,
      51,    52,     0,    53,    54,    43,    44,    45,    46,    47,
      48,     0,     0,     0,     0,     0,     0,     0,     0,    53,
      54
};

static const yysigned_char yycheck[] =
{
       7,    30,     3,    15,     3,     4,    30,    36,     7,     9,
      10,    11,    34,    37,    39,    35,    38,    42,     0,    31,
       5,     6,    41,    42,    23,    21,    22,    26,    13,    16,
      39,    21,    30,    12,    33,    28,     8,    -1,    -1,    38,
      -1,    40,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    60,    61,    55,    56,    -1,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      -1,    83,    84,    -1,    -1,    -1,    -1,    62,    63,    -1,
      65,    -1,    67,    90,    96,    97,    93,     3,     4,    89,
       6,     7,     8,    -1,    10,    11,    12,    13,    -1,    15,
      -1,    -1,    -1,    -1,    -1,     3,     4,    23,    -1,     7,
      26,     3,     4,    -1,     6,     7,     8,    33,    -1,    -1,
      12,    13,    38,    15,    40,    23,    -1,    -1,    26,    -1,
      -1,    23,    -1,    -1,    26,    33,    -1,    -1,    -1,    -1,
      38,    33,    40,    35,    -1,    -1,    38,    -1,    40,     3,
       4,    -1,     6,     7,     8,    -1,    -1,    -1,    12,    13,
      14,    15,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    23,
      -1,    -1,    26,     3,     4,    -1,     6,     7,     8,    33,
      -1,    11,    12,    13,    38,    15,    40,    -1,    -1,    -1,
      -1,    -1,    -1,    23,    -1,    -1,    26,    -1,    -1,    -1,
      -1,    -1,    -1,    33,    -1,    -1,    -1,    -1,    38,    -1,
      40,     3,     4,    -1,     6,     7,     8,    -1,    -1,    -1,
      12,    13,    14,    15,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    23,    -1,    -1,    26,     3,     4,    -1,     6,     7,
       8,    33,    -1,    -1,    12,    13,    38,    15,    40,    -1,
      -1,    -1,    -1,    -1,    -1,    23,    -1,    -1,    26,     3,
       4,    -1,     6,     7,     8,    33,    -1,    -1,    12,    13,
      38,    15,    40,    -1,    -1,    -1,    -1,    -1,    -1,    23,
      -1,    -1,    26,     3,     4,    -1,    -1,     7,     8,    33,
      -1,    -1,    12,    13,    38,    15,    40,    -1,    -1,    -1,
      -1,    -1,    -1,    23,    -1,    -1,    26,    -1,    -1,    -1,
      -1,    -1,    -1,    33,    -1,    -1,    -1,    -1,    38,    -1,
      40,    17,    18,    19,    20,    21,    22,    -1,    -1,     9,
      26,    27,    28,    29,    -1,    31,    32,    17,    18,    19,
      20,    21,    22,    -1,    -1,    41,    26,    27,    28,    29,
      13,    31,    32,    -1,    17,    18,    19,    20,    21,    22,
      -1,    -1,    -1,    26,    27,    28,    29,    13,    31,    32,
      -1,    17,    18,    19,    20,    21,    22,    -1,    -1,    -1,
      26,    27,    28,    29,    -1,    31,    32,    17,    18,    19,
      20,    21,    22,    -1,    -1,    25,    26,    27,    28,    29,
      -1,    31,    32,    17,    18,    19,    20,    21,    22,    -1,
      -1,    -1,    26,    27,    28,    29,    -1,    31,    32,    17,
      18,    19,    20,    21,    22,    -1,    -1,    -1,    -1,    -1,
      28,    29,    -1,    31,    32,    17,    18,    19,    20,    21,
      22,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,
      32
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,     3,     4,     6,     7,     8,    12,    13,    15,    23,
      26,    33,    38,    40,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    56,    57,    34,    38,    50,
      50,    45,     3,    56,    54,    54,    54,    47,    50,    55,
       0,    46,    35,    17,    18,    19,    20,    21,    22,    26,
      27,    28,    29,    31,    32,    30,    36,    50,    50,    55,
       9,    13,    12,    16,    39,    25,    41,    42,    50,    50,
      50,    50,    50,    50,    50,    50,    50,    50,    50,    50,
      54,    54,    39,    45,    45,    50,    50,    50,    50,    37,
      10,    11,    14,    13,    41,    54,    45,    45,    11,    14
};

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
    {								\
      yyerror (parser, YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (0)


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (N)								\
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (0)
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
              (Loc).first_line, (Loc).first_column,	\
              (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
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

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)		\
do {								\
  if (yydebug)							\
    {								\
      YYFPRINTF (stderr, "%s ", Title);				\
      yysymprint (stderr,					\
                  Type, Value);	\
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
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu), ",
             yyrule - 1, yylno);
  /* Print the symbols being reduced, and their result.  */
  for (yyi = yyprhs[yyrule]; 0 <= yyrhs[yyi]; yyi++)
    YYFPRINTF (stderr, "%s ", yytname[yyrhs[yyi]]);
  YYFPRINTF (stderr, "-> %s\n", yytname[yyr1[yyrule]]);
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
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
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
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

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
  const char *yys = yystr;

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
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      size_t yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

#endif /* YYERROR_VERBOSE */



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
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);


# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
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
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

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
int yyparse (MSParser *parser);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
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
yyparse (MSParser *parser)
#else
int
yyparse (parser)
    MSParser *parser;
#endif
#endif
{
  
  int yystate;
  int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
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
  short int *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;



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
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	short int *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
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
/* Read a look-ahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to look-ahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a look-ahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid look-ahead symbol.  */
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
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
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

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

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
#line 66 "/home/muntyan/projects/moo/moo/mooutils/mooscript/mooscript-yacc.y"
    { SET_TOP_NODE ((yyvsp[0].node)); ;}
    break;

  case 3:
#line 69 "/home/muntyan/projects/moo/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = NODE_LIST_ADD (NULL, (yyvsp[0].node)); ;}
    break;

  case 4:
#line 70 "/home/muntyan/projects/moo/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = NODE_LIST_ADD ((yyvsp[-1].node), (yyvsp[0].node)); ;}
    break;

  case 5:
#line 74 "/home/muntyan/projects/moo/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = (yyvsp[-1].node); ;}
    break;

  case 6:
#line 75 "/home/muntyan/projects/moo/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = NODE_PYTHON ((yyvsp[0].str)); ;}
    break;

  case 7:
#line 78 "/home/muntyan/projects/moo/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = NULL; ;}
    break;

  case 11:
#line 84 "/home/muntyan/projects/moo/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = NODE_WHILE ((yyvsp[-3].node), (yyvsp[-1].node)); ;}
    break;

  case 12:
#line 85 "/home/muntyan/projects/moo/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = NODE_DO_WHILE ((yyvsp[0].node), (yyvsp[-2].node)); ;}
    break;

  case 13:
#line 86 "/home/muntyan/projects/moo/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = NODE_FOR ((yyvsp[-5].node), (yyvsp[-3].node), (yyvsp[-1].node)); ;}
    break;

  case 14:
#line 90 "/home/muntyan/projects/moo/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = NODE_IF_ELSE ((yyvsp[-3].node), (yyvsp[-1].node), NULL); ;}
    break;

  case 15:
#line 91 "/home/muntyan/projects/moo/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = NODE_IF_ELSE ((yyvsp[-5].node), (yyvsp[-3].node), (yyvsp[-1].node)); ;}
    break;

  case 21:
#line 102 "/home/muntyan/projects/moo/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = NODE_ASSIGNMENT ((yyvsp[-2].str), (yyvsp[0].node)); ;}
    break;

  case 22:
#line 105 "/home/muntyan/projects/moo/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = NODE_IF_ELSE ((yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); ;}
    break;

  case 23:
#line 109 "/home/muntyan/projects/moo/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = BINARY_OP (MS_OP_PLUS, (yyvsp[-2].node), (yyvsp[0].node)); ;}
    break;

  case 24:
#line 110 "/home/muntyan/projects/moo/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = BINARY_OP (MS_OP_MINUS, (yyvsp[-2].node), (yyvsp[0].node)); ;}
    break;

  case 25:
#line 111 "/home/muntyan/projects/moo/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = BINARY_OP (MS_OP_DIV, (yyvsp[-2].node), (yyvsp[0].node)); ;}
    break;

  case 26:
#line 112 "/home/muntyan/projects/moo/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = BINARY_OP (MS_OP_MULT, (yyvsp[-2].node), (yyvsp[0].node)); ;}
    break;

  case 27:
#line 114 "/home/muntyan/projects/moo/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = BINARY_OP (MS_OP_AND, (yyvsp[-2].node), (yyvsp[0].node)); ;}
    break;

  case 28:
#line 115 "/home/muntyan/projects/moo/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = BINARY_OP (MS_OP_OR, (yyvsp[-2].node), (yyvsp[0].node)); ;}
    break;

  case 29:
#line 117 "/home/muntyan/projects/moo/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = BINARY_OP (MS_OP_EQ, (yyvsp[-2].node), (yyvsp[0].node)); ;}
    break;

  case 30:
#line 118 "/home/muntyan/projects/moo/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = BINARY_OP (MS_OP_NEQ, (yyvsp[-2].node), (yyvsp[0].node)); ;}
    break;

  case 31:
#line 119 "/home/muntyan/projects/moo/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = BINARY_OP (MS_OP_LT, (yyvsp[-2].node), (yyvsp[0].node)); ;}
    break;

  case 32:
#line 120 "/home/muntyan/projects/moo/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = BINARY_OP (MS_OP_GT, (yyvsp[-2].node), (yyvsp[0].node)); ;}
    break;

  case 33:
#line 121 "/home/muntyan/projects/moo/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = BINARY_OP (MS_OP_LE, (yyvsp[-2].node), (yyvsp[0].node)); ;}
    break;

  case 34:
#line 122 "/home/muntyan/projects/moo/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = BINARY_OP (MS_OP_GE, (yyvsp[-2].node), (yyvsp[0].node)); ;}
    break;

  case 35:
#line 126 "/home/muntyan/projects/moo/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = NODE_NUMBER ((yyvsp[0].ival)); ;}
    break;

  case 36:
#line 127 "/home/muntyan/projects/moo/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = NODE_STRING ((yyvsp[0].str)); ;}
    break;

  case 38:
#line 129 "/home/muntyan/projects/moo/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = (yyvsp[-1].node); ;}
    break;

  case 39:
#line 130 "/home/muntyan/projects/moo/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = NODE_VALUE_LIST ((yyvsp[-1].node)); ;}
    break;

  case 40:
#line 131 "/home/muntyan/projects/moo/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = NODE_VALUE_RANGE ((yyvsp[-3].node), (yyvsp[-1].node)); ;}
    break;

  case 41:
#line 132 "/home/muntyan/projects/moo/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = BINARY_OP (MS_OP_FORMAT, (yyvsp[-2].node), (yyvsp[0].node)); ;}
    break;

  case 42:
#line 133 "/home/muntyan/projects/moo/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = UNARY_OP (MS_OP_LEN, (yyvsp[0].node)); ;}
    break;

  case 43:
#line 134 "/home/muntyan/projects/moo/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = UNARY_OP (MS_OP_NOT, (yyvsp[0].node)); ;}
    break;

  case 44:
#line 135 "/home/muntyan/projects/moo/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = UNARY_OP (MS_OP_UMINUS, (yyvsp[0].node)); ;}
    break;

  case 45:
#line 138 "/home/muntyan/projects/moo/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = NULL; ;}
    break;

  case 46:
#line 139 "/home/muntyan/projects/moo/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = NODE_LIST_ADD (NULL, (yyvsp[0].node)); ;}
    break;

  case 47:
#line 140 "/home/muntyan/projects/moo/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = NODE_LIST_ADD ((yyvsp[-2].node), (yyvsp[0].node)); ;}
    break;

  case 48:
#line 143 "/home/muntyan/projects/moo/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = NODE_VAR ((yyvsp[0].str)); ;}
    break;

  case 49:
#line 146 "/home/muntyan/projects/moo/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = NODE_COMMAND ((yyvsp[-3].str), (yyvsp[-1].node)); ;}
    break;


      default: break;
    }

/* Line 1126 of yacc.c.  */
#line 1516 "/home/muntyan/projects/moo/moo/mooutils/mooscript/mooscript-yacc.c"

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
	  int yytype = YYTRANSLATE (yychar);
	  YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
	  YYSIZE_T yysize = yysize0;
	  YYSIZE_T yysize1;
	  int yysize_overflow = 0;
	  char *yymsg = 0;
#	  define YYERROR_VERBOSE_ARGS_MAXIMUM 5
	  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
	  int yyx;

#if 0
	  /* This is so xgettext sees the translatable formats that are
	     constructed on the fly.  */
	  YY_("syntax error, unexpected %s");
	  YY_("syntax error, unexpected %s, expecting %s");
	  YY_("syntax error, unexpected %s, expecting %s or %s");
	  YY_("syntax error, unexpected %s, expecting %s or %s or %s");
	  YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
#endif
	  char *yyfmt;
	  char const *yyf;
	  static char const yyunexpected[] = "syntax error, unexpected %s";
	  static char const yyexpecting[] = ", expecting %s";
	  static char const yyor[] = " or %s";
	  char yyformat[sizeof yyunexpected
			+ sizeof yyexpecting - 1
			+ ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
			   * (sizeof yyor - 1))];
	  char const *yyprefix = yyexpecting;

	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  int yyxbegin = yyn < 0 ? -yyn : 0;

	  /* Stay within bounds of both yycheck and yytname.  */
	  int yychecklim = YYLAST - yyn;
	  int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
	  int yycount = 1;

	  yyarg[0] = yytname[yytype];
	  yyfmt = yystpcpy (yyformat, yyunexpected);

	  for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	      {
		if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
		  {
		    yycount = 1;
		    yysize = yysize0;
		    yyformat[sizeof yyunexpected - 1] = '\0';
		    break;
		  }
		yyarg[yycount++] = yytname[yyx];
		yysize1 = yysize + yytnamerr (0, yytname[yyx]);
		yysize_overflow |= yysize1 < yysize;
		yysize = yysize1;
		yyfmt = yystpcpy (yyfmt, yyprefix);
		yyprefix = yyor;
	      }

	  yyf = YY_(yyformat);
	  yysize1 = yysize + yystrlen (yyf);
	  yysize_overflow |= yysize1 < yysize;
	  yysize = yysize1;

	  if (!yysize_overflow && yysize <= YYSTACK_ALLOC_MAXIMUM)
	    yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg)
	    {
	      /* Avoid sprintf, as that infringes on the user's name space.
		 Don't have undefined behavior even if the translation
		 produced a string with the wrong number of "%s"s.  */
	      char *yyp = yymsg;
	      int yyi = 0;
	      while ((*yyp = *yyf))
		{
		  if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		    {
		      yyp += yytnamerr (yyp, yyarg[yyi++]);
		      yyf += 2;
		    }
		  else
		    {
		      yyp++;
		      yyf++;
		    }
		}
	      yyerror (parser, yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    {
	      yyerror (parser, YY_("syntax error"));
	      goto yyexhaustedlab;
	    }
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror (parser, YY_("syntax error"));
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
        {
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
        }
      else
	{
	  yydestruct ("Error: discarding", yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse look-ahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (0)
     goto yyerrorlab;

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


      yydestruct ("Error: popping", yystos[yystate], yyvsp);
      YYPOPSTACK;
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;


  /* Shift the error token. */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

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
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (parser, YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEOF && yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK;
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}


#line 149 "/home/muntyan/projects/moo/moo/mooutils/mooscript/mooscript-yacc.y"


