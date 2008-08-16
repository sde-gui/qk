/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

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

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.3"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0

/* Substitute the variable and function names.  */
#define yyparse _moo_term_yyparse
#define yylex   _moo_term_yylex
#define yyerror _moo_term_yyerror
#define yylval  _moo_term_yylval
#define yychar  _moo_term_yychar
#define yydebug _moo_term_yydebug
#define yynerrs _moo_term_yynerrs






/* Copy the first part of user declarations.  */
#line 1 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"

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

#define DEFLT_1(n)  (n ? n : 1)

#define TERMINAL_HEIGHT ((int)parser->term->priv->height)
#define TERMINAL_WIDTH  ((int)parser->term->priv->width)


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

#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 145 "/home/muntyan/projects/moo/moo/mooterm/mootermparser-yacc.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

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

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int i)
#else
static int
YYID (i)
    int i;
#endif
{
  return i;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
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
      while (YYID (0))
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
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  189
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   428

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  85
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  109
/* YYNRULES -- Number of rules.  */
#define YYNRULES  280
/* YYNRULES -- Number of states.  */
#define YYNSTATES  316

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   257

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     3,     2,     2,
       2,     2,    28,    78,    61,    24,    73,     2,    65,    67,
      31,    32,    33,    34,     2,     2,     2,     2,    35,    36,
      37,    25,    26,    27,     8,     6,     5,     9,     2,    84,
      16,    17,    18,    50,    71,    38,    39,    42,    10,     4,
      29,    30,     7,    43,    69,    70,    72,    11,    12,    13,
      58,    60,    59,    63,    64,    56,    57,    51,    68,     2,
      14,     2,     2,     2,     2,     2,    44,    45,     2,    15,
      46,    47,    48,    52,    80,     2,     2,     2,    81,    79,
      20,    22,    41,    49,    54,    53,    55,    66,    62,     2,
      75,    77,    74,    76,    23,    21,    19,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    82,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,    40,    83,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     5,     7,     9,    11,    13,    15,    17,
      19,    21,    23,    25,    27,    29,    31,    33,    35,    37,
      39,    41,    43,    45,    47,    49,    51,    53,    55,    57,
      59,    61,    63,    65,    68,    71,    74,    77,    80,    83,
      86,    89,    92,    95,    98,   101,   104,   107,   110,   113,
     116,   119,   122,   125,   129,   133,   137,   141,   145,   149,
     153,   157,   159,   161,   163,   165,   167,   169,   171,   173,
     175,   177,   179,   181,   183,   185,   187,   189,   191,   193,
     195,   197,   199,   201,   203,   205,   207,   209,   211,   213,
     215,   217,   219,   221,   223,   225,   227,   229,   231,   233,
     235,   237,   239,   241,   243,   245,   247,   249,   251,   253,
     255,   257,   259,   261,   263,   265,   267,   269,   271,   273,
     275,   277,   279,   281,   283,   285,   287,   289,   291,   293,
     295,   297,   299,   301,   303,   305,   307,   309,   311,   316,
     320,   323,   327,   330,   334,   337,   341,   344,   348,   351,
     355,   358,   362,   365,   369,   372,   376,   379,   383,   386,
     390,   393,   397,   400,   404,   407,   411,   414,   418,   421,
     426,   430,   435,   439,   442,   446,   449,   454,   458,   462,
     465,   469,   472,   476,   479,   483,   486,   491,   495,   500,
     504,   509,   513,   517,   521,   525,   529,   534,   538,   543,
     547,   551,   554,   558,   561,   565,   568,   572,   575,   579,
     582,   586,   589,   593,   596,   601,   606,   610,   615,   619,
     624,   628,   633,   638,   643,   648,   653,   658,   663,   668,
     673,   678,   683,   688,   692,   695,   699,   703,   708,   712,
     717,   721,   725,   729,   732,   737,   741,   746,   750,   754,
     757,   761,   764,   769,   773,   778,   782,   784,   790,   793,
     796,   799,   801,   804,   806,   808,   811,   813,   815,   817,
     819,   821,   823,   825,   827,   829,   831,   833,   836,   838,
     841
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
      86,     0,    -1,    87,    -1,   118,    -1,   187,    -1,    88,
      -1,    89,    -1,    90,    -1,    91,    -1,    92,    -1,    93,
      -1,    94,    -1,    95,    -1,    96,    -1,    97,    -1,    98,
      -1,    99,    -1,   100,    -1,   101,    -1,   102,    -1,   103,
      -1,   104,    -1,   105,    -1,   106,    -1,   107,    -1,   108,
      -1,   109,    -1,   110,    -1,   111,    -1,   115,    -1,   113,
      -1,   114,    -1,   112,    -1,     3,     4,    -1,     3,     5,
      -1,     3,     6,    -1,     3,     7,    -1,     3,     8,    -1,
       3,     9,    -1,     3,    10,    -1,     3,    11,    -1,     3,
      12,    -1,     3,    13,    -1,     3,    14,    -1,     3,    15,
      -1,     3,    16,    -1,     3,    17,    -1,     3,    18,    -1,
       3,    19,    -1,     3,    20,    -1,     3,    21,    -1,     3,
      22,    -1,     3,    23,    -1,     3,    24,    25,    -1,     3,
      24,    26,    -1,     3,    24,    27,    -1,     3,    24,     8,
      -1,     3,    24,     5,    -1,     3,    28,    29,    -1,     3,
      28,    30,    -1,     3,   116,   117,    -1,    31,    -1,    32,
      -1,    33,    -1,    34,    -1,    35,    -1,    36,    -1,    37,
      -1,    38,    -1,    39,    -1,   119,    -1,   180,    -1,   181,
      -1,   182,    -1,   185,    -1,   186,    -1,   183,    -1,   184,
      -1,   123,    -1,   124,    -1,   121,    -1,   122,    -1,   125,
      -1,   126,    -1,   127,    -1,   128,    -1,   133,    -1,   120,
      -1,   129,    -1,   130,    -1,   131,    -1,   132,    -1,   174,
      -1,   134,    -1,   135,    -1,   136,    -1,   137,    -1,   138,
      -1,   139,    -1,   162,    -1,   140,    -1,   141,    -1,   142,
      -1,   143,    -1,   144,    -1,   145,    -1,   146,    -1,   172,
      -1,   147,    -1,   148,    -1,   149,    -1,   152,    -1,   150,
      -1,   153,    -1,   154,    -1,   155,    -1,   156,    -1,   157,
      -1,   151,    -1,   158,    -1,   159,    -1,   160,    -1,   161,
      -1,   163,    -1,   164,    -1,   165,    -1,   166,    -1,   167,
      -1,   171,    -1,   168,    -1,   173,    -1,   175,    -1,   176,
      -1,   177,    -1,   178,    -1,   179,    -1,   169,    -1,   170,
      -1,    40,   190,    34,    41,    -1,    40,   190,    38,    -1,
      40,    38,    -1,    40,   190,    39,    -1,    40,    39,    -1,
      40,   190,    42,    -1,    40,    42,    -1,    40,   190,    10,
      -1,    40,    10,    -1,    40,   190,    14,    -1,    40,    14,
      -1,    40,   190,    30,    -1,    40,    30,    -1,    40,   190,
      43,    -1,    40,    43,    -1,    40,   190,     4,    -1,    40,
       4,    -1,    40,   190,    29,    -1,    40,    29,    -1,    40,
     190,    44,    -1,    40,    44,    -1,    40,   190,    45,    -1,
      40,    45,    -1,    40,   190,    46,    -1,    40,    46,    -1,
      40,   190,    47,    -1,    40,    47,    -1,    40,   193,     7,
      -1,    40,     7,    -1,    40,   193,    48,    -1,    40,    48,
      -1,    40,   190,    28,    49,    -1,    40,    28,    49,    -1,
      40,    50,   190,    51,    -1,    40,   190,    52,    -1,    40,
      52,    -1,    40,   193,    53,    -1,    40,    53,    -1,    40,
     190,    28,    41,    -1,    40,    28,    41,    -1,    40,   193,
      54,    -1,    40,    54,    -1,    40,   190,    55,    -1,    40,
      55,    -1,    40,   190,    56,    -1,    40,    56,    -1,    40,
     190,    57,    -1,    40,    57,    -1,    40,   190,    28,    58,
      -1,    40,    28,    58,    -1,    40,   190,    28,    59,    -1,
      40,    28,    59,    -1,    40,   190,    28,    60,    -1,    40,
      28,    60,    -1,    40,    61,    62,    -1,    40,   190,    63,
      -1,    40,   190,    64,    -1,    40,    65,    66,    -1,    40,
     190,    67,    19,    -1,    40,    67,    19,    -1,    40,   190,
      67,    21,    -1,    40,    67,    21,    -1,    40,   190,    58,
      -1,    40,    58,    -1,    40,   190,    11,    -1,    40,    11,
      -1,    40,   190,    68,    -1,    40,    68,    -1,    40,   190,
      69,    -1,    40,    69,    -1,    40,   190,    70,    -1,    40,
      70,    -1,    40,   190,    71,    -1,    40,    71,    -1,    40,
     190,    72,    -1,    40,    72,    -1,    40,   190,    61,    49,
      -1,    40,    50,   190,    69,    -1,    40,    50,    69,    -1,
      40,    50,   190,    70,    -1,    40,    50,    70,    -1,    40,
     193,    73,    23,    -1,    40,    73,    23,    -1,    40,   193,
      73,    54,    -1,    40,   193,    73,    62,    -1,    40,   193,
      73,    74,    -1,    40,   193,    73,    75,    -1,    40,   193,
      73,    55,    -1,    40,   193,    73,    76,    -1,    40,   193,
      73,    21,    -1,    40,   193,    73,    19,    -1,    40,   193,
      33,    75,    -1,    40,   193,    33,    23,    -1,    40,   193,
      33,    77,    -1,    40,    50,   193,    20,    -1,    40,   193,
      20,    -1,    40,    15,    -1,    40,   190,    15,    -1,    40,
      18,    15,    -1,    40,    18,   190,    15,    -1,    40,    17,
      15,    -1,    40,    17,   190,    15,    -1,    40,   193,    77,
      -1,    40,    78,    41,    -1,    40,   193,    79,    -1,    40,
      79,    -1,    40,    50,   193,    80,    -1,    40,    50,    80,
      -1,    40,    50,   193,    81,    -1,    40,    50,    81,    -1,
      40,   193,    80,    -1,    40,    80,    -1,    40,   193,    81,
      -1,    40,    81,    -1,    40,    50,   193,    53,    -1,    40,
      50,    53,    -1,    40,    50,   193,    54,    -1,    40,    50,
      54,    -1,   188,    -1,    82,    73,    49,   189,    83,    -1,
      73,    52,    -1,    61,    41,    -1,    73,    23,    -1,    55,
      -1,    33,    23,    -1,    54,    -1,   191,    -1,   190,   191,
      -1,    35,    -1,    36,    -1,    37,    -1,    25,    -1,    26,
      -1,    27,    -1,     8,    -1,     6,    -1,     5,    -1,     9,
      -1,    84,    -1,   192,    84,    -1,   190,    -1,   192,   190,
      -1,   193,   192,   190,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,    44,    44,    45,    46,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     129,   130,   131,   132,   133,   134,   135,   136,   137,   138,
     139,   140,   141,   142,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   152,   153,   154,   155,   156,   157,   158,
     159,   160,   161,   162,   163,   164,   165,   166,   167,   168,
     169,   170,   171,   172,   173,   174,   175,   176,   177,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   200,   203,
     204,   205,   206,   207,   208,   209,   210,   211,   212,   213,
     214,   215,   216,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   230,   233,   234,   237,   239,
     240,   242,   243,   244,   246,   247,   249,   250,   252,   256,
     258,   259,   261,   262,   263,   264,   266,   267,   268,   269,
     270,   271,   273,   275,   276,   278,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   300,   301,   302,   303,   304,   306,
     307,   308,   309,   310,   311,   312,   313,   314,   315,   317,
     318,   319,   321,   330,   340,   341,   342,   343,   344,   345,
     347,   349,   352,   353,   354,   355,   356,   357,   358,   359,
     360,   361,   362,   363,   364,   365,   372,   375,   377,   378,
     379,   380,   381,   382,   390,   391,   394,   395,   396,   397,
     398,   399,   400,   401,   402,   403,   406,   407,   410,   411,
     412
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "'\\033'", "'E'", "'8'", "'7'", "'H'",
  "'6'", "'9'", "'D'", "'M'", "'N'", "'O'", "'Z'", "'c'", "'<'", "'='",
  "'>'", "'~'", "'n'", "'}'", "'o'", "'|'", "'#'", "'3'", "'4'", "'5'",
  "' '", "'F'", "'G'", "'('", "')'", "'*'", "'+'", "'0'", "'1'", "'2'",
  "'A'", "'B'", "'\\233'", "'p'", "'C'", "'I'", "'`'", "'a'", "'d'", "'e'",
  "'f'", "'q'", "'?'", "'W'", "'g'", "'s'", "'r'", "'t'", "'U'", "'V'",
  "'P'", "'R'", "'Q'", "'\"'", "'v'", "'S'", "'T'", "'&'", "'u'", "'''",
  "'X'", "'J'", "'K'", "'@'", "'L'", "'$'", "'z'", "'x'", "'{'", "'y'",
  "'!'", "'m'", "'h'", "'l'", "'\\220'", "'\\234'", "';'", "$accept",
  "control_function", "escape_sequence", "NEL", "DECRC", "DECSC", "HTS",
  "DECBI", "DECFI", "IND", "RI", "SS2", "SS3", "DECID", "RIS", "DECANM",
  "DECKPAM", "DECKPNM", "LS1R", "LS2", "LS2R", "LS3", "LS3R", "DECDHLT",
  "DECDHLB", "DECSWL", "DECDWL", "DECALN", "S7C1T", "S8C1T", "SCS",
  "SCS_set_num", "SCS_set", "control_sequence", "DECSR", "CUU", "CUD",
  "CUF", "CUB", "CBT", "CHA", "CHT", "CNL", "CPL", "HPA", "HPR", "VPA",
  "VPR", "CUP", "DECSCUSR", "DECST8C", "TBC", "DECSLRM", "DECSSCLS",
  "DECSTBM", "DECSLPP", "NP", "PP", "PPA", "PPB", "PPR", "DECRQDE", "SD",
  "SU", "DECRQUPSS", "DECDC", "DECIC", "DCH", "DL", "ECH", "ED", "EL",
  "ICH", "IL", "DECSCA", "DECSED", "DECSEL", "DECSCPP", "DECCARA",
  "DECCRA", "DECERA", "DECFRA", "DECRARA", "DECSERA", "DECSASD", "DECSSDT",
  "DECSACE", "DECSNLS", "DECRQCRA", "DSR", "DA1", "DA2", "DA3", "DECTST",
  "DECSTR", "SGR", "DECSET", "DECRST", "SM", "RM", "DECSAVE", "DECRESTORE",
  "device_control_sequence", "DECRQSS", "DECRQSS_param", "number", "digit",
  "semicolons", "numbers", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,    27,    69,    56,    55,    72,    54,    57,
      68,    77,    78,    79,    90,    99,    60,    61,    62,   126,
     110,   125,   111,   124,    35,    51,    52,    53,    32,    70,
      71,    40,    41,    42,    43,    48,    49,    50,    65,    66,
     155,   112,    67,    73,    96,    97,   100,   101,   102,   113,
      63,    87,   103,   115,   114,   116,    85,    86,    80,    82,
      81,    34,   118,    83,    84,    38,   117,    39,    88,    74,
      75,    64,    76,    36,   122,   120,   123,   121,    33,   109,
     104,   108,   144,   156,    59
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    85,    86,    86,    86,    87,    87,    87,    87,    87,
      87,    87,    87,    87,    87,    87,    87,    87,    87,    87,
      87,    87,    87,    87,    87,    87,    87,    87,    87,    87,
      87,    87,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   116,   116,   116,   117,   117,   117,   117,   117,
     118,   118,   118,   118,   118,   118,   118,   118,   118,   118,
     118,   118,   118,   118,   118,   118,   118,   118,   118,   118,
     118,   118,   118,   118,   118,   118,   118,   118,   118,   118,
     118,   118,   118,   118,   118,   118,   118,   118,   118,   118,
     118,   118,   118,   118,   118,   118,   118,   118,   118,   118,
     118,   118,   118,   118,   118,   118,   118,   118,   118,   118,
     118,   118,   118,   118,   118,   118,   118,   118,   119,   120,
     120,   121,   121,   122,   122,   123,   123,   124,   124,   125,
     125,   126,   126,   127,   127,   128,   128,   129,   129,   130,
     130,   131,   131,   132,   132,   133,   133,   133,   133,   134,
     134,   135,   136,   136,   137,   137,   138,   138,   139,   139,
     140,   140,   141,   141,   142,   142,   143,   143,   144,   144,
     145,   145,   146,   147,   148,   149,   150,   150,   151,   151,
     152,   152,   153,   153,   154,   154,   155,   155,   156,   156,
     157,   157,   158,   158,   159,   160,   160,   161,   161,   162,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   174,   174,   175,   175,   176,   176,   177,   177,
     178,   179,   180,   180,   181,   181,   182,   182,   183,   183,
     184,   184,   185,   185,   186,   186,   187,   188,   189,   189,
     189,   189,   189,   189,   190,   190,   191,   191,   191,   191,
     191,   191,   191,   191,   191,   191,   192,   192,   193,   193,
     193
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     3,     3,     3,     3,     3,     3,     3,
       3,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     4,     3,
       2,     3,     2,     3,     2,     3,     2,     3,     2,     3,
       2,     3,     2,     3,     2,     3,     2,     3,     2,     3,
       2,     3,     2,     3,     2,     3,     2,     3,     2,     4,
       3,     4,     3,     2,     3,     2,     4,     3,     3,     2,
       3,     2,     3,     2,     3,     2,     4,     3,     4,     3,
       4,     3,     3,     3,     3,     3,     4,     3,     4,     3,
       3,     2,     3,     2,     3,     2,     3,     2,     3,     2,
       3,     2,     3,     2,     4,     4,     3,     4,     3,     4,
       3,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     3,     2,     3,     3,     4,     3,     4,
       3,     3,     3,     2,     4,     3,     4,     3,     3,     2,
       3,     2,     4,     3,     4,     3,     1,     5,     2,     2,
       2,     1,     2,     1,     1,     2,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     2,     1,     2,
       3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       0,     0,     0,     0,     0,     2,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      32,    30,    31,    29,     3,    70,    87,    80,    81,    78,
      79,    82,    83,    84,    85,    88,    89,    90,    91,    86,
      93,    94,    95,    96,    97,    98,   100,   101,   102,   103,
     104,   105,   106,   108,   109,   110,   112,   118,   111,   113,
     114,   115,   116,   117,   119,   120,   121,   122,    99,   123,
     124,   125,   126,   127,   129,   136,   137,   128,   107,   130,
      92,   131,   132,   133,   134,   135,    71,    72,    73,    76,
      77,    74,    75,     4,   256,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,     0,     0,    61,    62,    63,
      64,     0,   154,   274,   273,   166,   272,   275,   146,   203,
     148,   234,     0,     0,   269,   270,   271,     0,   156,   150,
     266,   267,   268,   140,   142,   144,   152,   158,   160,   162,
     164,   168,     0,   173,   175,   179,   181,   183,   185,   201,
       0,     0,     0,   205,   207,   209,   211,   213,     0,     0,
     243,   249,   251,   276,   278,   264,     0,     0,     0,     1,
      57,    56,    53,    54,    55,    58,    59,    65,    66,    67,
      68,    69,    60,   238,     0,   236,     0,   177,   170,   187,
     189,   191,   253,   255,   216,   218,   245,   247,   278,     0,
     192,   195,   197,   199,   220,   241,   153,   145,   202,   147,
     235,     0,   155,   149,     0,   139,   141,   143,   151,   157,
     159,   161,   163,   172,   180,   182,   184,   200,     0,   193,
     194,     0,   204,   206,   208,   210,   212,   265,   277,   279,
     165,   233,     0,   167,   174,   178,     0,   240,   242,   248,
     250,     0,     0,   239,   237,   171,   215,   217,   232,   252,
     254,   244,   246,   176,   169,   186,   188,   190,   138,   214,
     196,   198,   230,   229,   231,   228,   227,   219,   221,   225,
     222,   223,   224,   226,   280,     0,   263,   261,     0,     0,
       0,   262,   259,   260,   258,   257
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,   131,   202,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   310,   184,   185,   186,   187
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -185
static const yytype_int16 yypact[] =
{
      27,   394,    46,   -64,    12,  -185,  -185,  -185,  -185,  -185,
    -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,
    -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,
    -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,
    -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,
    -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,
    -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,
    -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,
    -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,
    -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,
    -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,
    -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,
    -185,  -185,  -185,  -185,  -185,   311,   -24,  -185,  -185,  -185,
    -185,   107,  -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,
    -185,  -185,   288,   324,  -185,  -185,  -185,   123,  -185,  -185,
    -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,
    -185,  -185,   126,  -185,  -185,  -185,  -185,  -185,  -185,  -185,
     -45,   -43,    -6,  -185,  -185,  -185,  -185,  -185,    -2,   -16,
    -185,  -185,  -185,  -185,   207,  -185,     2,   121,   -23,  -185,
    -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,
    -185,  -185,  -185,  -185,   347,  -185,   360,  -185,  -185,  -185,
    -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,   275,    86,
    -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,
    -185,   198,  -185,  -185,   -10,  -185,  -185,  -185,  -185,  -185,
    -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,   -17,  -185,
    -185,    -3,  -185,  -185,  -185,  -185,  -185,  -185,  -185,   151,
    -185,  -185,    -9,  -185,  -185,  -185,   266,  -185,  -185,  -185,
    -185,     2,   -14,  -185,  -185,  -185,  -185,  -185,  -185,  -185,
    -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,
    -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,
    -185,  -185,  -185,  -185,   151,    11,  -185,  -185,     1,   -19,
     -40,  -185,  -185,  -185,  -185,  -185
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,
    -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,
    -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,
    -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,
    -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,
    -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,
    -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,
    -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,
    -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,
    -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,  -185,
    -185,  -185,  -185,  -185,  -185,  -142,  -182,  -184,  -117
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint16 yytable[] =
{
     204,   206,   257,   271,   313,   195,   196,   133,   134,   188,
     136,   137,   189,   222,   292,   223,   290,   220,   291,   305,
     218,   224,   257,   221,   257,   225,   272,   144,   145,   146,
       1,   288,   289,   314,   311,   271,   257,   150,   151,   152,
     306,   307,   312,   315,   259,   219,     0,   308,     0,     0,
     132,   133,   134,   135,   136,   137,   138,   139,     0,   309,
     140,   141,     0,   142,   143,     0,   293,     2,   294,     0,
       0,   144,   145,   146,   147,   148,   149,   257,     0,     0,
       0,   150,   151,   152,   153,   154,   258,     0,   155,   156,
     157,   158,   159,   160,   161,     0,   162,     0,   163,   164,
     165,   166,   167,   168,   169,     0,   278,   170,     0,     3,
       0,   171,     0,   172,   173,   174,   175,   176,   177,   178,
       0,     0,   257,     0,   179,   180,   181,   182,   260,   304,
     183,   133,   134,     0,   136,   137,     0,     0,     0,   279,
     280,   261,   197,   198,   199,   200,   201,     0,     0,     0,
       0,   144,   145,   146,   262,     0,   133,   134,     0,   136,
     137,   150,   151,   152,   207,     0,   281,   282,     0,   263,
     183,     0,   208,     0,   264,   265,   144,   145,   146,   212,
     213,   209,   210,   211,     0,     0,   150,   151,   152,     0,
       0,     0,     0,     0,   266,   214,   215,     0,   267,     0,
     268,   269,   270,     0,     0,   183,   216,   217,     0,     0,
     183,   226,   133,   134,     0,   136,   137,   227,   228,     0,
       0,   229,   230,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   144,   145,   146,   231,   232,   233,     0,   283,
       0,   234,   150,   151,   152,   235,   236,   284,     0,   237,
     238,   239,   240,   241,   242,     0,   285,   286,   287,   243,
       0,     0,   244,   245,   246,   247,     0,     0,   248,     0,
     249,   250,     0,     0,   251,   252,   253,   254,   255,   256,
     133,   134,     0,   136,   137,   295,     0,   296,     0,   297,
       0,     0,     0,   133,   134,     0,   136,   137,     0,     0,
     144,   145,   146,   203,     0,     0,     0,     0,     0,     0,
     150,   151,   152,   144,   145,   146,   190,     0,     0,   191,
     298,   299,     0,   150,   151,   152,   275,     0,   300,   133,
     134,     0,   136,   137,     0,     0,   192,   193,   194,   205,
     301,   302,   303,     0,   276,   277,     0,     0,     0,   144,
     145,   146,   133,   134,     0,   136,   137,     0,     0,   150,
     151,   152,   273,     0,     0,   133,   134,     0,   136,   137,
       0,     0,   144,   145,   146,   274,     0,     0,     0,     0,
       0,     0,   150,   151,   152,   144,   145,   146,     0,     0,
       0,     0,     0,     0,     0,   150,   151,   152,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,     0,
       0,     0,   126,     0,     0,   127,   128,   129,   130
};

static const yytype_int16 yycheck[] =
{
     142,   143,   184,   187,    23,    29,    30,     5,     6,    73,
       8,     9,     0,    19,    23,    21,    19,    62,    21,    33,
     162,    23,   204,    66,   206,    41,    49,    25,    26,    27,
       3,    41,    49,    52,    23,   219,   218,    35,    36,    37,
      54,    55,    41,    83,   186,   162,    -1,    61,    -1,    -1,
       4,     5,     6,     7,     8,     9,    10,    11,    -1,    73,
      14,    15,    -1,    17,    18,    -1,    75,    40,    77,    -1,
      -1,    25,    26,    27,    28,    29,    30,   259,    -1,    -1,
      -1,    35,    36,    37,    38,    39,    84,    -1,    42,    43,
      44,    45,    46,    47,    48,    -1,    50,    -1,    52,    53,
      54,    55,    56,    57,    58,    -1,    20,    61,    -1,    82,
      -1,    65,    -1,    67,    68,    69,    70,    71,    72,    73,
      -1,    -1,   304,    -1,    78,    79,    80,    81,     7,   271,
      84,     5,     6,    -1,     8,     9,    -1,    -1,    -1,    53,
      54,    20,    35,    36,    37,    38,    39,    -1,    -1,    -1,
      -1,    25,    26,    27,    33,    -1,     5,     6,    -1,     8,
       9,    35,    36,    37,    41,    -1,    80,    81,    -1,    48,
      84,    -1,    49,    -1,    53,    54,    25,    26,    27,    53,
      54,    58,    59,    60,    -1,    -1,    35,    36,    37,    -1,
      -1,    -1,    -1,    -1,    73,    69,    70,    -1,    77,    -1,
      79,    80,    81,    -1,    -1,    84,    80,    81,    -1,    -1,
      84,     4,     5,     6,    -1,     8,     9,    10,    11,    -1,
      -1,    14,    15,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    25,    26,    27,    28,    29,    30,    -1,    41,
      -1,    34,    35,    36,    37,    38,    39,    49,    -1,    42,
      43,    44,    45,    46,    47,    -1,    58,    59,    60,    52,
      -1,    -1,    55,    56,    57,    58,    -1,    -1,    61,    -1,
      63,    64,    -1,    -1,    67,    68,    69,    70,    71,    72,
       5,     6,    -1,     8,     9,    19,    -1,    21,    -1,    23,
      -1,    -1,    -1,     5,     6,    -1,     8,     9,    -1,    -1,
      25,    26,    27,    15,    -1,    -1,    -1,    -1,    -1,    -1,
      35,    36,    37,    25,    26,    27,     5,    -1,    -1,     8,
      54,    55,    -1,    35,    36,    37,    51,    -1,    62,     5,
       6,    -1,     8,     9,    -1,    -1,    25,    26,    27,    15,
      74,    75,    76,    -1,    69,    70,    -1,    -1,    -1,    25,
      26,    27,     5,     6,    -1,     8,     9,    -1,    -1,    35,
      36,    37,    15,    -1,    -1,     5,     6,    -1,     8,     9,
      -1,    -1,    25,    26,    27,    15,    -1,    -1,    -1,    -1,
      -1,    -1,    35,    36,    37,    25,    26,    27,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    35,    36,    37,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    -1,
      -1,    -1,    28,    -1,    -1,    31,    32,    33,    34
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,    40,    82,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,   155,   156,   157,   158,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   168,   169,   170,   171,   172,   173,
     174,   175,   176,   177,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    28,    31,    32,    33,
      34,   116,     4,     5,     6,     7,     8,     9,    10,    11,
      14,    15,    17,    18,    25,    26,    27,    28,    29,    30,
      35,    36,    37,    38,    39,    42,    43,    44,    45,    46,
      47,    48,    50,    52,    53,    54,    55,    56,    57,    58,
      61,    65,    67,    68,    69,    70,    71,    72,    73,    78,
      79,    80,    81,    84,   190,   191,   192,   193,    73,     0,
       5,     8,    25,    26,    27,    29,    30,    35,    36,    37,
      38,    39,   117,    15,   190,    15,   190,    41,    49,    58,
      59,    60,    53,    54,    69,    70,    80,    81,   190,   193,
      62,    66,    19,    21,    23,    41,     4,    10,    11,    14,
      15,    28,    29,    30,    34,    38,    39,    42,    43,    44,
      45,    46,    47,    52,    55,    56,    57,    58,    61,    63,
      64,    67,    68,    69,    70,    71,    72,   191,    84,   190,
       7,    20,    33,    48,    53,    54,    73,    77,    79,    80,
      81,   192,    49,    15,    15,    51,    69,    70,    20,    53,
      54,    80,    81,    41,    49,    58,    59,    60,    41,    49,
      19,    21,    23,    75,    77,    19,    21,    23,    54,    55,
      62,    74,    75,    76,   190,    33,    54,    55,    61,    73,
     189,    23,    41,    23,    52,    83
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
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (parser, YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
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
    while (YYID (0))
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
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value, parser); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, MooTermParser *parser)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep, parser)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    MooTermParser *parser;
#endif
{
  if (!yyvaluep)
    return;
  YYUSE (parser);
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, MooTermParser *parser)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep, parser)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    MooTermParser *parser;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep, parser);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *bottom, yytype_int16 *top)
#else
static void
yy_stack_print (bottom, top)
    yytype_int16 *bottom;
    yytype_int16 *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule, MooTermParser *parser)
#else
static void
yy_reduce_print (yyvsp, yyrule, parser)
    YYSTYPE *yyvsp;
    int yyrule;
    MooTermParser *parser;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      fprintf (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       , parser);
      fprintf (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule, parser); \
} while (YYID (0))

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
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
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
      YYSIZE_T yyn = 0;
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

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
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
      int yychecklim = YYLAST - yyn + 1;
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
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
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
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, MooTermParser *parser)
#else
static void
yydestruct (yymsg, yytype, yyvaluep, parser)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
    MooTermParser *parser;
#endif
{
  YYUSE (yyvaluep);
  YYUSE (parser);

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
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (MooTermParser *parser);
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
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (MooTermParser *parser)
#else
int
yyparse (parser)
    MooTermParser *parser;
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
#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  yytype_int16 yyssa[YYINITDEPTH];
  yytype_int16 *yyss = yyssa;
  yytype_int16 *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

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
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;


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
	yytype_int16 *yyss1 = yyss;
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

  /* Do appropriate processing given the current state.  Read a
     look-ahead token if we need one and don't already have one.  */

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

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

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
        case 33:
#line 85 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_NEL;             ;}
    break;

  case 34:
#line 86 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_DECRC;           ;}
    break;

  case 35:
#line 87 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_DECSC;           ;}
    break;

  case 36:
#line 88 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_HTS;             ;}
    break;

  case 37:
#line 89 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_NOT_IMPLEMENTED; ;}
    break;

  case 38:
#line 90 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_NOT_IMPLEMENTED; ;}
    break;

  case 39:
#line 91 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_IND;             ;}
    break;

  case 40:
#line 92 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_RI;              ;}
    break;

  case 41:
#line 93 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_NOT_IMPLEMENTED; ;}
    break;

  case 42:
#line 94 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_NOT_IMPLEMENTED; ;}
    break;

  case 43:
#line 95 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_NOT_IMPLEMENTED; ;}
    break;

  case 44:
#line 96 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_RIS;             ;}
    break;

  case 45:
#line 97 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_NOT_IMPLEMENTED; ;}
    break;

  case 46:
#line 98 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_DECKPAM;         ;}
    break;

  case 47:
#line 99 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_DECKPNM;         ;}
    break;

  case 48:
#line 100 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_NOT_IMPLEMENTED; ;}
    break;

  case 49:
#line 101 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_NOT_IMPLEMENTED; ;}
    break;

  case 50:
#line 102 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_NOT_IMPLEMENTED; ;}
    break;

  case 51:
#line 103 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_NOT_IMPLEMENTED; ;}
    break;

  case 52:
#line 104 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_NOT_IMPLEMENTED; ;}
    break;

  case 53:
#line 105 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_NOT_IMPLEMENTED; ;}
    break;

  case 54:
#line 106 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_NOT_IMPLEMENTED; ;}
    break;

  case 55:
#line 107 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_NOT_IMPLEMENTED; ;}
    break;

  case 56:
#line 108 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_NOT_IMPLEMENTED; ;}
    break;

  case 57:
#line 109 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_DECALN;          ;}
    break;

  case 58:
#line 110 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_IGNORED;         ;}
    break;

  case 59:
#line 111 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_IGNORED;         ;}
    break;

  case 60:
#line 113 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_SCS (GET_NUM(0), GET_NUM(1));    ;}
    break;

  case 61:
#line 114 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   ADD_NUMBER (0);                     ;}
    break;

  case 62:
#line 115 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   ADD_NUMBER (1);                     ;}
    break;

  case 63:
#line 116 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   ADD_NUMBER (2);                     ;}
    break;

  case 64:
#line 117 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   ADD_NUMBER (3);                     ;}
    break;

  case 65:
#line 118 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   ADD_NUMBER (0);                     ;}
    break;

  case 66:
#line 119 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   ADD_NUMBER (1);                     ;}
    break;

  case 67:
#line 120 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   ADD_NUMBER (2);                     ;}
    break;

  case 68:
#line 121 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   ADD_NUMBER (3);                     ;}
    break;

  case 69:
#line 122 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   ADD_NUMBER (4);                     ;}
    break;

  case 138:
#line 200 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_NOT_IMPLEMENTED;     ;}
    break;

  case 139:
#line 203 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_CUU (DEFLT_1((yyvsp[(2) - (3)])));   ;}
    break;

  case 140:
#line 204 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_CUU (1);             ;}
    break;

  case 141:
#line 205 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_CUD (DEFLT_1((yyvsp[(2) - (3)])));   ;}
    break;

  case 142:
#line 206 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_CUD (1);             ;}
    break;

  case 143:
#line 207 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_CUF (DEFLT_1((yyvsp[(2) - (3)])));   ;}
    break;

  case 144:
#line 208 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_CUF (1);             ;}
    break;

  case 145:
#line 209 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_CUB (DEFLT_1((yyvsp[(2) - (3)])));   ;}
    break;

  case 146:
#line 210 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_CUB (1);             ;}
    break;

  case 147:
#line 211 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_CBT (DEFLT_1((yyvsp[(2) - (3)])));   ;}
    break;

  case 148:
#line 212 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_CBT (1);             ;}
    break;

  case 149:
#line 213 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_CHA (DEFLT_1((yyvsp[(2) - (3)])));   ;}
    break;

  case 150:
#line 214 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_CHA (1);             ;}
    break;

  case 151:
#line 215 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_CHT (DEFLT_1((yyvsp[(2) - (3)])));   ;}
    break;

  case 152:
#line 216 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_CHT (1);             ;}
    break;

  case 153:
#line 217 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_CNL (DEFLT_1((yyvsp[(2) - (3)])));   ;}
    break;

  case 154:
#line 218 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_CNL (1);             ;}
    break;

  case 155:
#line 219 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_CPL (DEFLT_1((yyvsp[(2) - (3)])));   ;}
    break;

  case 156:
#line 220 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_CPL (1);             ;}
    break;

  case 157:
#line 221 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_HPA (DEFLT_1((yyvsp[(2) - (3)])));   ;}
    break;

  case 158:
#line 222 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_HPA (1);             ;}
    break;

  case 159:
#line 223 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_HPR (DEFLT_1((yyvsp[(2) - (3)])));   ;}
    break;

  case 160:
#line 224 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_HPR (1);             ;}
    break;

  case 161:
#line 225 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_VPA (DEFLT_1((yyvsp[(2) - (3)])));   ;}
    break;

  case 162:
#line 226 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_VPA (1);             ;}
    break;

  case 163:
#line 227 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_VPR (DEFLT_1((yyvsp[(2) - (3)])));   ;}
    break;

  case 164:
#line 228 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_VPR (1);             ;}
    break;

  case 165:
#line 230 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   CHECK_NUMS_LEN (2);
                                                VT_CUP (DEFLT_1(GET_NUM(0)),
                                                        DEFLT_1(GET_NUM(1)));   ;}
    break;

  case 166:
#line 233 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_CUP (1, 1);   ;}
    break;

  case 167:
#line 234 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   CHECK_NUMS_LEN (2);
                                                VT_CUP (DEFLT_1(GET_NUM(0)),
                                                        DEFLT_1(GET_NUM(1)));   ;}
    break;

  case 168:
#line 237 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_CUP (1, 1);   ;}
    break;

  case 169:
#line 239 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_NOT_IMPLEMENTED; ;}
    break;

  case 170:
#line 240 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_NOT_IMPLEMENTED; ;}
    break;

  case 171:
#line 242 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_NOT_IMPLEMENTED; ;}
    break;

  case 172:
#line 243 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_TBC ((yyvsp[(2) - (3)]));        ;}
    break;

  case 173:
#line 244 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_TBC (0);         ;}
    break;

  case 174:
#line 246 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_NOT_IMPLEMENTED; ;}
    break;

  case 175:
#line 247 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_NOT_IMPLEMENTED; ;}
    break;

  case 176:
#line 249 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_NOT_IMPLEMENTED; ;}
    break;

  case 177:
#line 250 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_NOT_IMPLEMENTED; ;}
    break;

  case 178:
#line 252 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   CHECK_NUMS_LEN (2);
                                                VT_DECSTBM (DEFLT_1(GET_NUM(0)),
                                                        GET_NUM(1) ? GET_NUM(1) :
                                                                TERMINAL_HEIGHT); ;}
    break;

  case 179:
#line 256 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_DECSTBM (1, TERMINAL_HEIGHT);    ;}
    break;

  case 180:
#line 258 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_NOT_IMPLEMENTED; ;}
    break;

  case 181:
#line 259 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_NOT_IMPLEMENTED; ;}
    break;

  case 182:
#line 261 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_NOT_IMPLEMENTED; ;}
    break;

  case 183:
#line 262 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_NOT_IMPLEMENTED; ;}
    break;

  case 184:
#line 263 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_NOT_IMPLEMENTED; ;}
    break;

  case 185:
#line 264 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_NOT_IMPLEMENTED; ;}
    break;

  case 186:
#line 266 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_NOT_IMPLEMENTED; ;}
    break;

  case 187:
#line 267 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_NOT_IMPLEMENTED; ;}
    break;

  case 188:
#line 268 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_NOT_IMPLEMENTED; ;}
    break;

  case 189:
#line 269 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_NOT_IMPLEMENTED; ;}
    break;

  case 190:
#line 270 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_NOT_IMPLEMENTED; ;}
    break;

  case 191:
#line 271 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_NOT_IMPLEMENTED; ;}
    break;

  case 192:
#line 273 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_NOT_IMPLEMENTED; ;}
    break;

  case 193:
#line 275 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_NOT_IMPLEMENTED; ;}
    break;

  case 194:
#line 276 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_NOT_IMPLEMENTED; ;}
    break;

  case 195:
#line 278 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_NOT_IMPLEMENTED; ;}
    break;

  case 196:
#line 281 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_NOT_IMPLEMENTED;     ;}
    break;

  case 197:
#line 282 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_NOT_IMPLEMENTED;     ;}
    break;

  case 198:
#line 283 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_NOT_IMPLEMENTED;     ;}
    break;

  case 199:
#line 284 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_NOT_IMPLEMENTED;     ;}
    break;

  case 200:
#line 285 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_DCH (DEFLT_1((yyvsp[(2) - (3)])));   ;}
    break;

  case 201:
#line 286 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_DCH (1);             ;}
    break;

  case 202:
#line 287 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_DL (DEFLT_1((yyvsp[(2) - (3)])));    ;}
    break;

  case 203:
#line 288 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_DL (1);              ;}
    break;

  case 204:
#line 289 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_ECH (DEFLT_1((yyvsp[(2) - (3)])));   ;}
    break;

  case 205:
#line 290 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_ECH (1);             ;}
    break;

  case 206:
#line 291 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_ED ((yyvsp[(2) - (3)]));             ;}
    break;

  case 207:
#line 292 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_ED (0);              ;}
    break;

  case 208:
#line 293 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_EL ((yyvsp[(2) - (3)]));             ;}
    break;

  case 209:
#line 294 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_EL (0);              ;}
    break;

  case 210:
#line 295 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_ICH (DEFLT_1((yyvsp[(2) - (3)])));   ;}
    break;

  case 211:
#line 296 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_ICH (1);             ;}
    break;

  case 212:
#line 297 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_IL (DEFLT_1((yyvsp[(2) - (3)])));    ;}
    break;

  case 213:
#line 298 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_IL (1);              ;}
    break;

  case 214:
#line 300 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_NOT_IMPLEMENTED;   ;}
    break;

  case 215:
#line 301 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_NOT_IMPLEMENTED;   ;}
    break;

  case 216:
#line 302 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_NOT_IMPLEMENTED;   ;}
    break;

  case 217:
#line 303 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_NOT_IMPLEMENTED;   ;}
    break;

  case 218:
#line 304 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_NOT_IMPLEMENTED;   ;}
    break;

  case 219:
#line 306 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_NOT_IMPLEMENTED;   ;}
    break;

  case 220:
#line 307 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_NOT_IMPLEMENTED;   ;}
    break;

  case 221:
#line 308 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_NOT_IMPLEMENTED;   ;}
    break;

  case 222:
#line 309 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_NOT_IMPLEMENTED;   ;}
    break;

  case 223:
#line 310 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_NOT_IMPLEMENTED;   ;}
    break;

  case 224:
#line 311 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_NOT_IMPLEMENTED;   ;}
    break;

  case 225:
#line 312 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_NOT_IMPLEMENTED;   ;}
    break;

  case 226:
#line 313 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_NOT_IMPLEMENTED;   ;}
    break;

  case 227:
#line 314 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_NOT_IMPLEMENTED;   ;}
    break;

  case 228:
#line 315 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_NOT_IMPLEMENTED;   ;}
    break;

  case 229:
#line 317 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_NOT_IMPLEMENTED;   ;}
    break;

  case 230:
#line 318 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_NOT_IMPLEMENTED;   ;}
    break;

  case 231:
#line 319 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_NOT_IMPLEMENTED;   ;}
    break;

  case 232:
#line 321 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   if (NUMS_LEN == 2)
                                                {
                                                    VT_DSR (GET_NUM (0), GET_NUM (1), TRUE);
                                                }
                                                else
                                                {
                                                    VT_DSR (GET_NUM (0), -1, TRUE);
                                                }
                                            ;}
    break;

  case 233:
#line 330 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   if (NUMS_LEN == 2)
                                                {
                                                    VT_DSR (GET_NUM (0), GET_NUM (1), FALSE);
                                                }
                                                else
                                                {
                                                    VT_DSR (GET_NUM (0), -1, FALSE);
                                                }
                                            ;}
    break;

  case 234:
#line 340 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_DA1;             ;}
    break;

  case 235:
#line 341 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_DA1;             ;}
    break;

  case 236:
#line 342 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_DA2;             ;}
    break;

  case 237:
#line 343 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_DA2;             ;}
    break;

  case 238:
#line 344 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_DA3;             ;}
    break;

  case 239:
#line 345 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_DA3;             ;}
    break;

  case 240:
#line 347 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_IGNORED;         ;}
    break;

  case 241:
#line 349 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_DECSTR;          ;}
    break;

  case 242:
#line 352 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_SGR;             ;}
    break;

  case 243:
#line 353 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_SGR;             ;}
    break;

  case 244:
#line 354 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_DECSET;          ;}
    break;

  case 245:
#line 355 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_DECSET;          ;}
    break;

  case 246:
#line 356 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_DECRST;          ;}
    break;

  case 247:
#line 357 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_DECRST;          ;}
    break;

  case 248:
#line 358 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_SM;              ;}
    break;

  case 249:
#line 359 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_SM;              ;}
    break;

  case 250:
#line 360 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_RM;              ;}
    break;

  case 251:
#line 361 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_RM;              ;}
    break;

  case 252:
#line 362 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_DECSAVE;         ;}
    break;

  case 253:
#line 363 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_DECSAVE;         ;}
    break;

  case 254:
#line 364 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_DECRESTORE;      ;}
    break;

  case 255:
#line 365 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_DECRESTORE;      ;}
    break;

  case 257:
#line 375 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   VT_DECRQSS (GET_NUM (0));  ;}
    break;

  case 258:
#line 377 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   ADD_NUMBER (CODE_DECSASD);  ;}
    break;

  case 259:
#line 378 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   ADD_NUMBER (CODE_DECSCL);   ;}
    break;

  case 260:
#line 379 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   ADD_NUMBER (CODE_DECSCPP);  ;}
    break;

  case 261:
#line 380 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   ADD_NUMBER (CODE_DECSLPP);  ;}
    break;

  case 262:
#line 381 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   ADD_NUMBER (CODE_DECSNLS);  ;}
    break;

  case 263:
#line 382 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   ADD_NUMBER (CODE_DECSTBM);  ;}
    break;

  case 264:
#line 390 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   (yyval) = (yyvsp[(1) - (1)]);   ;}
    break;

  case 265:
#line 391 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   (yyval) = (yyvsp[(1) - (2)]) * 10 + (yyvsp[(2) - (2)]);  ;}
    break;

  case 266:
#line 394 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   (yyval) = 0;    ;}
    break;

  case 267:
#line 395 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   (yyval) = 1;    ;}
    break;

  case 268:
#line 396 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   (yyval) = 2;    ;}
    break;

  case 269:
#line 397 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   (yyval) = 3;    ;}
    break;

  case 270:
#line 398 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   (yyval) = 4;    ;}
    break;

  case 271:
#line 399 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   (yyval) = 5;    ;}
    break;

  case 272:
#line 400 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   (yyval) = 6;    ;}
    break;

  case 273:
#line 401 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   (yyval) = 7;    ;}
    break;

  case 274:
#line 402 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   (yyval) = 8;    ;}
    break;

  case 275:
#line 403 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   (yyval) = 9;    ;}
    break;

  case 277:
#line 407 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   ADD_NUMBER (-1);                    ;}
    break;

  case 278:
#line 410 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   ADD_NUMBER ((yyvsp[(1) - (1)]));                    ;}
    break;

  case 279:
#line 411 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   ADD_NUMBER (-1); ADD_NUMBER ((yyvsp[(2) - (2)]));   ;}
    break;

  case 280:
#line 412 "/home/muntyan/projects/moo/moo/mooterm/mootermparser.y"
    {   ADD_NUMBER ((yyvsp[(3) - (3)]));                    ;}
    break;


/* Line 1267 of yacc.c.  */
#line 2666 "/home/muntyan/projects/moo/moo/mooterm/mootermparser-yacc.c"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
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
#if ! YYERROR_VERBOSE
      yyerror (parser, YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (parser, yymsg);
	  }
	else
	  {
	    yyerror (parser, YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
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
	  yydestruct ("Error: discarding",
		      yytoken, &yylval, parser);
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
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
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


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp, parser);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;


  /* Shift the error token.  */
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
		 yytoken, &yylval, parser);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp, parser);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}



