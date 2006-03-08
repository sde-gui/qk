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
     CONTINUE = 272,
     BREAK = 273,
     RETURN = 274,
     EQ = 275,
     NEQ = 276,
     LE = 277,
     GE = 278,
     AND = 279,
     OR = 280,
     NOT = 281,
     UMINUS = 282,
     TWODOTS = 283
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
#define CONTINUE 272
#define BREAK 273
#define RETURN 274
#define EQ 275
#define NEQ 276
#define LE 277
#define GE 278
#define AND 279
#define OR 280
#define NOT 281
#define UMINUS 282
#define TWODOTS 283




/* Copy the first part of user declarations.  */
#line 1 "/home/muntyan/projects/ggap/ggap/moo/mooutils/mooscript/mooscript-yacc.y"

#include "mooscript-parser-priv.h"
#include "mooscript-yacc.h"


static MSNode *
node_list_add (MSParser   *parser,
               MSNodeList *list,
               MSNode     *node)
{
    if (!node)
        return NULL;

    if (!list)
    {
        list = ms_node_list_new ();
        _ms_parser_add_node (parser, list);
    }

    ms_node_list_add (list, node);
    return MS_NODE (list);
}


static MSNode *
node_command (MSParser   *parser,
              const char *name,
              MSNodeList *list)
{
    MSNodeCommand *cmd;

    g_return_val_if_fail (name != NULL, NULL);
    g_return_val_if_fail (!list || MS_IS_NODE_LIST (list), NULL);

    cmd = ms_node_command_new (name, list);
    _ms_parser_add_node (parser, cmd);

    return MS_NODE (cmd);
}


static MSNode *
node_if_else (MSParser   *parser,
              MSNode     *condition,
              MSNode     *then_,
              MSNode     *else_)
{
    MSNodeIfElse *node;

    g_return_val_if_fail (condition && then_, NULL);

    node = ms_node_if_else_new (condition, then_, else_);
    _ms_parser_add_node (parser, node);

    return MS_NODE (node);
}


static MSNode *
node_while (MSParser   *parser,
            MSCondType  type,
            MSNode     *cond,
            MSNode     *what)
{
    MSNodeWhile *loop;

    g_return_val_if_fail (cond != NULL, NULL);

    loop = ms_node_while_new (type, cond, what);
    _ms_parser_add_node (parser, loop);

    return MS_NODE (loop);
}


static MSNode *
node_for (MSParser   *parser,
          MSNode     *var,
          MSNode     *list,
          MSNode     *what)
{
    MSNodeFor *loop;

    g_return_val_if_fail (var && list, NULL);

    loop = ms_node_for_new (var, list, what);
    _ms_parser_add_node (parser, loop);

    return MS_NODE (loop);
}


static MSNode *
node_var (MSParser   *parser,
          const char *name)
{
    MSNodeVar *node;

    node = ms_node_var_new (name);
    _ms_parser_add_node (parser, node);

    return MS_NODE (node);
}


static MSNode *
node_assignment (MSParser   *parser,
                 const char *name,
                 MSNode     *val)
{
    MSNodeAssign *node;
    MSNode *var;

    g_return_val_if_fail (name && name[0], NULL);
    g_return_val_if_fail (val != NULL, NULL);

    var = node_var (parser, name);
    node = ms_node_assign_new (MS_NODE_VAR (var), val);
    _ms_parser_add_node (parser, node);

    return MS_NODE (node);
}


static MSNode *
node_binary_op (MSParser   *parser,
                MSBinaryOp  op,
                MSNode     *lval,
                MSNode     *rval)
{
    MSNodeCommand *cmd;

    g_return_val_if_fail (lval && rval, NULL);

    cmd = ms_node_binary_op_new (op, lval, rval);
    _ms_parser_add_node (parser, cmd);

    return MS_NODE (cmd);
}


static MSNode *
node_unary_op (MSParser   *parser,
               MSUnaryOp   op,
               MSNode     *val)
{
    MSNodeCommand *cmd;

    g_return_val_if_fail (val != NULL, NULL);

    cmd = ms_node_unary_op_new (op, val);
    _ms_parser_add_node (parser, cmd);

    return MS_NODE (cmd);
}


static MSNode *
node_int (MSParser   *parser,
          int         n)
{
    MSNodeValue *node;
    MSValue *value;

    value = ms_value_int (n);
    node = ms_node_value_new (value);
    ms_value_unref (value);
    _ms_parser_add_node (parser, node);

    return MS_NODE (node);
}


static MSNode *
node_string (MSParser   *parser,
             const char *string)
{
    MSNodeValue *node;
    MSValue *value;

    value = ms_value_string (string);
    node = ms_node_value_new (value);
    ms_value_unref (value);
    _ms_parser_add_node (parser, node);

    return MS_NODE (node);
}


static MSNode *
node_python (MSParser   *parser,
             const char *string)
{
    MSNodePython *node;

    node = ms_node_python_new (string);
    _ms_parser_add_node (parser, node);

    return MS_NODE (node);
}


static MSNode *
node_value_list (MSParser   *parser,
                 MSNodeList *list)
{
    MSNodeValList *node;

    node = ms_node_val_list_new (list);
    _ms_parser_add_node (parser, node);

    return MS_NODE (node);
}


static MSNode *
node_value_range (MSParser   *parser,
                  MSNode     *first,
                  MSNode     *last)
{
    MSNodeValList *node;

    node = ms_node_val_range_new (first, last);
    _ms_parser_add_node (parser, node);

    return MS_NODE (node);
}


static MSNode *
node_list_elm (MSParser   *parser,
               MSNode     *list,
               MSNode     *ind)
{
    MSNodeListElm *node;

    node = ms_node_list_elm_new (list, ind);
    _ms_parser_add_node (parser, node);

    return MS_NODE (node);
}


static MSNode *
node_list_assign (MSParser   *parser,
                  MSNode     *list,
                  MSNode     *ind,
                  MSNode     *val)
{
    MSNodeListAssign *node;

    node = ms_node_list_assign_new (list, ind, val);
    _ms_parser_add_node (parser, node);

    return MS_NODE (node);
}


static MSNode *
node_dict (MSParser   *parser,
           MSNodeList *list)
{
    MSNodeDict *node;

    node = ms_node_dict_new (list);
    _ms_parser_add_node (parser, node);

    return MS_NODE (node);
}


static MSNode *
node_dict_entry (MSParser   *parser,
                 const char *key,
                 MSNode     *val)
{
    MSNodeDictEntry *node;

    node = ms_node_dict_entry_new (key, val);
    _ms_parser_add_node (parser, node);

    return MS_NODE (node);
}


static MSNode *
node_return (MSParser   *parser,
             MSNode     *val)
{
    MSNodeReturn *node;

    node = ms_node_return_new (val);
    _ms_parser_add_node (parser, node);

    return MS_NODE (node);
}


static MSNode *
node_break (MSParser *parser)
{
    MSNodeBreak *node;

    node = ms_node_break_new (MS_BREAK_BREAK);
    _ms_parser_add_node (parser, node);

    return MS_NODE (node);
}


static MSNode *
node_continue (MSParser *parser)
{
    MSNodeBreak *node;

    node = ms_node_break_new (MS_BREAK_CONTINUE);
    _ms_parser_add_node (parser, node);

    return MS_NODE (node);
}


static MSNode *
node_dict_elm (MSParser   *parser,
               MSNode     *dict,
               const char *key)
{
    MSNodeDictElm *node;

    node = ms_node_dict_elm_new (dict, key);
    _ms_parser_add_node (parser, node);

    return MS_NODE (node);
}


static MSNode *
node_dict_assign (MSParser   *parser,
                  MSNode     *dict,
                  const char *key,
                  MSNode     *val)
{
    MSNodeDictAssign *node;

    node = ms_node_dict_assign_new (dict, key, val);
    _ms_parser_add_node (parser, node);

    return MS_NODE (node);
}



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
#line 355 "/home/muntyan/projects/ggap/ggap/moo/mooutils/mooscript/mooscript-yacc.y"
typedef union YYSTYPE {
    int ival;
    const char *str;
    MSNode *node;
} YYSTYPE;
/* Line 196 of yacc.c.  */
#line 507 "/home/muntyan/projects/ggap/ggap/moo/mooutils/mooscript/mooscript-yacc.c"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 219 of yacc.c.  */
#line 519 "/home/muntyan/projects/ggap/ggap/moo/mooutils/mooscript/mooscript-yacc.c"

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
#define YYFINAL  49
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   634

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  49
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  16
/* YYNRULES -- Number of rules. */
#define YYNRULES  61
/* YYNRULES -- Number of states. */
#define YYNSTATES  128

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   283

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,    36,     2,    33,     2,     2,
      44,    45,    31,    30,    48,    29,    41,    32,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    43,    37,
      34,    38,    35,    42,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    39,     2,    40,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    46,     2,    47,     2,     2,     2,     2,
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
      25,    26,    27,    28
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned char yyprhs[] =
{
       0,     0,     3,     5,     7,    10,    13,    15,    16,    18,
      20,    22,    24,    26,    28,    30,    33,    39,    44,    52,
      58,    66,    68,    70,    72,    76,    83,    89,    95,    99,
     103,   107,   111,   115,   119,   123,   127,   131,   135,   139,
     143,   146,   149,   152,   156,   158,   160,   162,   166,   170,
     174,   180,   185,   190,   194,   195,   197,   201,   202,   204,
     208,   212
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const yysigned_char yyrhs[] =
{
      50,     0,    -1,    51,    -1,    52,    -1,    51,    52,    -1,
      53,    37,    -1,     6,    -1,    -1,    56,    -1,    55,    -1,
      54,    -1,    57,    -1,    17,    -1,    18,    -1,    19,    -1,
      19,    56,    -1,    12,    56,    13,    51,    14,    -1,    13,
      51,    12,    56,    -1,    15,    64,    16,    56,    13,    51,
      14,    -1,     8,    56,     9,    51,    11,    -1,     8,    56,
       9,    51,    10,    51,    11,    -1,    60,    -1,    59,    -1,
      58,    -1,     3,    38,    56,    -1,    60,    39,    56,    40,
      38,    56,    -1,    60,    41,     3,    38,    56,    -1,    60,
      42,    60,    43,    60,    -1,    56,    30,    56,    -1,    56,
      29,    56,    -1,    56,    32,    56,    -1,    56,    31,    56,
      -1,    56,    24,    56,    -1,    56,    25,    56,    -1,    56,
      20,    56,    -1,    56,    21,    56,    -1,    56,    34,    56,
      -1,    56,    35,    56,    -1,    56,    22,    56,    -1,    56,
      23,    56,    -1,    29,    60,    -1,    26,    60,    -1,    36,
      60,    -1,    60,    33,    60,    -1,     7,    -1,     4,    -1,
      64,    -1,    44,    53,    45,    -1,    39,    61,    40,    -1,
      46,    62,    47,    -1,    39,    56,    28,    56,    40,    -1,
       3,    44,    61,    45,    -1,    60,    39,    56,    40,    -1,
      60,    41,     3,    -1,    -1,    56,    -1,    61,    48,    56,
      -1,    -1,    63,    -1,    62,    48,    63,    -1,     3,    38,
      56,    -1,     3,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short int yyrline[] =
{
       0,   396,   396,   399,   400,   404,   405,   408,   409,   410,
     411,   412,   413,   414,   415,   416,   419,   420,   421,   425,
     426,   429,   430,   431,   435,   436,   437,   440,   444,   445,
     446,   447,   449,   450,   452,   453,   454,   455,   456,   457,
     458,   459,   460,   461,   465,   466,   467,   468,   469,   470,
     471,   472,   473,   474,   477,   478,   479,   482,   483,   484,
     487,   490
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "IDENTIFIER", "LITERAL", "VARIABLE",
  "PYTHON", "NUMBER", "IF", "THEN", "ELSE", "FI", "WHILE", "DO", "OD",
  "FOR", "IN", "CONTINUE", "BREAK", "RETURN", "EQ", "NEQ", "LE", "GE",
  "AND", "OR", "NOT", "UMINUS", "TWODOTS", "'-'", "'+'", "'*'", "'/'",
  "'%'", "'<'", "'>'", "'#'", "';'", "'='", "'['", "']'", "'.'", "'?'",
  "':'", "'('", "')'", "'{'", "'}'", "','", "$accept", "script", "program",
  "stmt_or_python", "stmt", "loop", "if_stmt", "expr", "assignment",
  "ternary", "compound_expr", "simple_expr", "list_elms", "dict_elms",
  "dict_entry", "variable", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short int yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,    45,
      43,    42,    47,    37,    60,    62,    35,    59,    61,    91,
      93,    46,    63,    58,    40,    41,   123,   125,    44
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    49,    50,    51,    51,    52,    52,    53,    53,    53,
      53,    53,    53,    53,    53,    53,    54,    54,    54,    55,
      55,    56,    56,    56,    57,    57,    57,    58,    59,    59,
      59,    59,    59,    59,    59,    59,    59,    59,    59,    59,
      59,    59,    59,    59,    60,    60,    60,    60,    60,    60,
      60,    60,    60,    60,    61,    61,    61,    62,    62,    62,
      63,    64
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     1,     2,     2,     1,     0,     1,     1,
       1,     1,     1,     1,     1,     2,     5,     4,     7,     5,
       7,     1,     1,     1,     3,     6,     5,     5,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       2,     2,     2,     3,     1,     1,     1,     3,     3,     3,
       5,     4,     4,     3,     0,     1,     3,     0,     1,     3,
       3,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       7,    61,    45,     6,    44,     0,     0,     7,     0,    12,
      13,    14,     0,     0,     0,    54,     7,    57,     0,     2,
       3,     0,    10,     9,     8,    11,    23,    22,    21,    46,
       0,    54,    61,     0,    21,     0,     7,    61,     0,    15,
      41,    40,    42,    55,     0,     0,     0,     0,    58,     1,
       4,     5,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    24,    55,
       0,     7,     0,     0,     7,     0,     0,     0,    48,     0,
      47,     0,    49,     0,    34,    35,    38,    39,    32,    33,
      29,    28,    31,    30,    36,    37,    43,     0,    53,     0,
      51,     7,     0,    53,     7,    17,     0,     0,    56,    60,
      59,    52,     0,     0,     7,    19,    52,    16,     7,    50,
       0,    26,    27,     7,     7,    25,    20,    18
};

/* YYDEFGOTO[NTERM-NUM]. */
static const yysigned_char yydefgoto[] =
{
      -1,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    34,    44,    47,    48,    29
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -44
static const short int yypact[] =
{
     330,   -35,   -44,   -44,   -44,   236,   236,   330,    12,   -44,
     -44,   236,    31,    31,    31,   236,   404,    34,    51,   154,
     -44,    17,   -44,   -44,   567,   -44,   -44,   -44,   -31,   -44,
     236,   236,    15,   432,     6,   452,   374,   -44,    45,   567,
     -11,   -11,   -11,   551,     1,    18,    24,   -43,   -44,   -44,
     -44,   -44,   236,   236,   236,   236,   236,   236,   236,   236,
     236,   236,   236,   236,    31,   236,    63,    31,   567,   567,
       7,   330,   236,    66,   330,   236,   236,   236,   -44,   236,
     -44,   236,   -44,    34,    32,    32,    32,    32,   -44,    48,
     583,   583,   599,   599,    32,    32,   -11,   488,    35,     3,
     -44,    14,   509,   -44,   198,   452,   472,   530,   567,   567,
     -44,    38,   236,    31,   330,   -44,   -44,   -44,   330,   -44,
     236,   567,   -11,   242,   286,   567,   -44,   -44
};

/* YYPGOTO[NTERM-NUM].  */
static const yysigned_char yypgoto[] =
{
     -44,   -44,    -6,   -13,    62,   -44,   -44,    75,   -44,   -44,
     -44,     0,    52,   -44,    -4,    74
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -8
static const yysigned_char yytable[] =
{
      28,    36,    64,    30,    82,    83,    50,    28,    65,    31,
      66,    67,    40,    41,    42,    37,    28,     1,     2,    28,
       3,     4,     5,    50,   114,   115,     6,     7,    72,     8,
      73,     9,    10,    11,    32,     2,    28,    46,     4,    64,
      12,    78,    72,    13,    73,    72,   113,    73,    67,    79,
      14,    49,   100,    15,    51,    79,    56,    57,    16,    31,
      17,    76,    81,    80,    96,   101,    98,    99,   104,   103,
      15,    28,    56,   112,    28,    16,   120,    17,    45,   110,
      33,    35,    38,    70,     0,     0,    39,     0,    50,     0,
      43,    50,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    28,     0,     0,    28,    68,    69,     0,   123,     0,
      50,    50,   124,   122,    28,     0,     0,     0,    28,     0,
       0,     0,     0,    28,    28,     0,     0,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,     0,
      97,     0,     0,     0,     0,     0,     0,   102,     0,     0,
     105,   106,   107,     0,   108,     0,   109,     1,     2,     0,
       3,     4,     5,     0,     0,     0,     6,     7,     0,     8,
       0,     9,    10,    11,     0,     0,     0,     0,     0,     0,
      12,     0,     0,    13,     0,     0,     0,   121,     0,     0,
      14,    -7,     0,    15,     0,   125,     0,     0,    16,     0,
      17,     1,     2,     0,     3,     4,     5,     0,     0,     0,
       6,     7,   117,     8,     0,     9,    10,    11,     0,     0,
       0,     0,     0,     0,    12,     0,     0,    13,     0,     0,
       0,     0,     0,     0,    14,     0,     0,    15,     0,    32,
       2,     0,    16,     4,    17,     1,     2,     0,     3,     4,
       5,     0,     0,   126,     6,     7,     0,     8,     0,     9,
      10,    11,    12,     0,     0,    13,     0,     0,    12,     0,
       0,    13,    14,     0,     0,    15,     0,     0,    14,     0,
      16,    15,    17,     0,     0,     0,    16,     0,    17,     1,
       2,     0,     3,     4,     5,     0,     0,     0,     6,     7,
     127,     8,     0,     9,    10,    11,     0,     0,     0,     0,
       0,     0,    12,     0,     0,    13,     0,     0,     0,     0,
       0,     0,    14,     0,     0,    15,     0,     0,     0,     0,
      16,     0,    17,     1,     2,     0,     3,     4,     5,     0,
       0,     0,     6,     7,     0,     8,     0,     9,    10,    11,
       0,     0,     0,     0,     0,     0,    12,     0,     0,    13,
       0,     0,     0,     0,     0,     0,    14,     0,     0,    15,
       0,     0,     0,     0,    16,     0,    17,     1,     2,     0,
       3,     4,     5,     0,     0,     0,    75,     7,     0,     8,
       0,     9,    10,    11,     0,     0,     0,     0,     0,     0,
      12,     0,     0,    13,     0,     0,     0,     1,     2,     0,
      14,     4,     5,    15,     0,     0,     6,     7,    16,     8,
      17,     9,    10,    11,     0,     0,     0,     0,     0,     0,
      12,     0,     0,    13,     0,     0,     0,     0,     0,     0,
      14,    71,     0,    15,     0,     0,     0,     0,    16,     0,
      17,     0,    52,    53,    54,    55,    56,    57,     0,     0,
       0,    58,    59,    60,    61,    74,    62,    63,     0,     0,
       0,     0,    52,    53,    54,    55,    56,    57,     0,     0,
       0,    58,    59,    60,    61,   118,    62,    63,     0,     0,
       0,     0,    52,    53,    54,    55,    56,    57,     0,     0,
       0,    58,    59,    60,    61,     0,    62,    63,    52,    53,
      54,    55,    56,    57,     0,     0,     0,    58,    59,    60,
      61,     0,    62,    63,     0,     0,     0,     0,   111,    52,
      53,    54,    55,    56,    57,     0,     0,     0,    58,    59,
      60,    61,     0,    62,    63,     0,     0,     0,     0,   116,
      52,    53,    54,    55,    56,    57,     0,     0,     0,    58,
      59,    60,    61,     0,    62,    63,     0,     0,     0,     0,
     119,    52,    53,    54,    55,    56,    57,     0,     0,    77,
      58,    59,    60,    61,     0,    62,    63,    52,    53,    54,
      55,    56,    57,     0,     0,     0,    58,    59,    60,    61,
       0,    62,    63,    52,    53,    54,    55,    56,    57,     0,
       0,     0,     0,     0,    60,    61,     0,    62,    63,    52,
      53,    54,    55,    56,    57,     0,     0,     0,     0,     0,
       0,     0,     0,    62,    63
};

static const yysigned_char yycheck[] =
{
       0,     7,    33,    38,    47,    48,    19,     7,    39,    44,
      41,    42,    12,    13,    14,     3,    16,     3,     4,    19,
       6,     7,     8,    36,    10,    11,    12,    13,    39,    15,
      41,    17,    18,    19,     3,     4,    36,     3,     7,    33,
      26,    40,    39,    29,    41,    39,    43,    41,    42,    48,
      36,     0,    45,    39,    37,    48,    24,    25,    44,    44,
      46,    16,    38,    45,    64,    71,     3,    67,    74,     3,
      39,    71,    24,    38,    74,    44,    38,    46,    16,    83,
       5,     6,     8,    31,    -1,    -1,    11,    -1,   101,    -1,
      15,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   101,    -1,    -1,   104,    30,    31,    -1,   114,    -1,
     123,   124,   118,   113,   114,    -1,    -1,    -1,   118,    -1,
      -1,    -1,    -1,   123,   124,    -1,    -1,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    -1,
      65,    -1,    -1,    -1,    -1,    -1,    -1,    72,    -1,    -1,
      75,    76,    77,    -1,    79,    -1,    81,     3,     4,    -1,
       6,     7,     8,    -1,    -1,    -1,    12,    13,    -1,    15,
      -1,    17,    18,    19,    -1,    -1,    -1,    -1,    -1,    -1,
      26,    -1,    -1,    29,    -1,    -1,    -1,   112,    -1,    -1,
      36,    37,    -1,    39,    -1,   120,    -1,    -1,    44,    -1,
      46,     3,     4,    -1,     6,     7,     8,    -1,    -1,    -1,
      12,    13,    14,    15,    -1,    17,    18,    19,    -1,    -1,
      -1,    -1,    -1,    -1,    26,    -1,    -1,    29,    -1,    -1,
      -1,    -1,    -1,    -1,    36,    -1,    -1,    39,    -1,     3,
       4,    -1,    44,     7,    46,     3,     4,    -1,     6,     7,
       8,    -1,    -1,    11,    12,    13,    -1,    15,    -1,    17,
      18,    19,    26,    -1,    -1,    29,    -1,    -1,    26,    -1,
      -1,    29,    36,    -1,    -1,    39,    -1,    -1,    36,    -1,
      44,    39,    46,    -1,    -1,    -1,    44,    -1,    46,     3,
       4,    -1,     6,     7,     8,    -1,    -1,    -1,    12,    13,
      14,    15,    -1,    17,    18,    19,    -1,    -1,    -1,    -1,
      -1,    -1,    26,    -1,    -1,    29,    -1,    -1,    -1,    -1,
      -1,    -1,    36,    -1,    -1,    39,    -1,    -1,    -1,    -1,
      44,    -1,    46,     3,     4,    -1,     6,     7,     8,    -1,
      -1,    -1,    12,    13,    -1,    15,    -1,    17,    18,    19,
      -1,    -1,    -1,    -1,    -1,    -1,    26,    -1,    -1,    29,
      -1,    -1,    -1,    -1,    -1,    -1,    36,    -1,    -1,    39,
      -1,    -1,    -1,    -1,    44,    -1,    46,     3,     4,    -1,
       6,     7,     8,    -1,    -1,    -1,    12,    13,    -1,    15,
      -1,    17,    18,    19,    -1,    -1,    -1,    -1,    -1,    -1,
      26,    -1,    -1,    29,    -1,    -1,    -1,     3,     4,    -1,
      36,     7,     8,    39,    -1,    -1,    12,    13,    44,    15,
      46,    17,    18,    19,    -1,    -1,    -1,    -1,    -1,    -1,
      26,    -1,    -1,    29,    -1,    -1,    -1,    -1,    -1,    -1,
      36,     9,    -1,    39,    -1,    -1,    -1,    -1,    44,    -1,
      46,    -1,    20,    21,    22,    23,    24,    25,    -1,    -1,
      -1,    29,    30,    31,    32,    13,    34,    35,    -1,    -1,
      -1,    -1,    20,    21,    22,    23,    24,    25,    -1,    -1,
      -1,    29,    30,    31,    32,    13,    34,    35,    -1,    -1,
      -1,    -1,    20,    21,    22,    23,    24,    25,    -1,    -1,
      -1,    29,    30,    31,    32,    -1,    34,    35,    20,    21,
      22,    23,    24,    25,    -1,    -1,    -1,    29,    30,    31,
      32,    -1,    34,    35,    -1,    -1,    -1,    -1,    40,    20,
      21,    22,    23,    24,    25,    -1,    -1,    -1,    29,    30,
      31,    32,    -1,    34,    35,    -1,    -1,    -1,    -1,    40,
      20,    21,    22,    23,    24,    25,    -1,    -1,    -1,    29,
      30,    31,    32,    -1,    34,    35,    -1,    -1,    -1,    -1,
      40,    20,    21,    22,    23,    24,    25,    -1,    -1,    28,
      29,    30,    31,    32,    -1,    34,    35,    20,    21,    22,
      23,    24,    25,    -1,    -1,    -1,    29,    30,    31,    32,
      -1,    34,    35,    20,    21,    22,    23,    24,    25,    -1,
      -1,    -1,    -1,    -1,    31,    32,    -1,    34,    35,    20,
      21,    22,    23,    24,    25,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    34,    35
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,     3,     4,     6,     7,     8,    12,    13,    15,    17,
      18,    19,    26,    29,    36,    39,    44,    46,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    64,
      38,    44,     3,    56,    60,    56,    51,     3,    64,    56,
      60,    60,    60,    56,    61,    53,     3,    62,    63,     0,
      52,    37,    20,    21,    22,    23,    24,    25,    29,    30,
      31,    32,    34,    35,    33,    39,    41,    42,    56,    56,
      61,     9,    39,    41,    13,    12,    16,    28,    40,    48,
      45,    38,    47,    48,    56,    56,    56,    56,    56,    56,
      56,    56,    56,    56,    56,    56,    60,    56,     3,    60,
      45,    51,    56,     3,    51,    56,    56,    56,    56,    56,
      63,    40,    38,    43,    10,    11,    40,    14,    13,    40,
      38,    56,    60,    51,    51,    56,    11,    14
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
#line 396 "/home/muntyan/projects/ggap/ggap/moo/mooutils/mooscript/mooscript-yacc.y"
    { _ms_parser_set_top_node (parser, (yyvsp[0].node)); ;}
    break;

  case 3:
#line 399 "/home/muntyan/projects/ggap/ggap/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = node_list_add (parser, NULL, (yyvsp[0].node)); ;}
    break;

  case 4:
#line 400 "/home/muntyan/projects/ggap/ggap/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = node_list_add (parser, MS_NODE_LIST ((yyvsp[-1].node)), (yyvsp[0].node)); ;}
    break;

  case 5:
#line 404 "/home/muntyan/projects/ggap/ggap/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = (yyvsp[-1].node); ;}
    break;

  case 6:
#line 405 "/home/muntyan/projects/ggap/ggap/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = node_python (parser, (yyvsp[0].str)); ;}
    break;

  case 7:
#line 408 "/home/muntyan/projects/ggap/ggap/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = NULL; ;}
    break;

  case 12:
#line 413 "/home/muntyan/projects/ggap/ggap/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = node_continue (parser); ;}
    break;

  case 13:
#line 414 "/home/muntyan/projects/ggap/ggap/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = node_break (parser); ;}
    break;

  case 14:
#line 415 "/home/muntyan/projects/ggap/ggap/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = node_return (parser, NULL); ;}
    break;

  case 15:
#line 416 "/home/muntyan/projects/ggap/ggap/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = node_return (parser, (yyvsp[0].node)); ;}
    break;

  case 16:
#line 419 "/home/muntyan/projects/ggap/ggap/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = node_while (parser, MS_COND_BEFORE, (yyvsp[-3].node), (yyvsp[-1].node)); ;}
    break;

  case 17:
#line 420 "/home/muntyan/projects/ggap/ggap/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = node_while (parser, MS_COND_AFTER, (yyvsp[0].node), (yyvsp[-2].node)); ;}
    break;

  case 18:
#line 421 "/home/muntyan/projects/ggap/ggap/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = node_for (parser, (yyvsp[-5].node), (yyvsp[-3].node), (yyvsp[-1].node)); ;}
    break;

  case 19:
#line 425 "/home/muntyan/projects/ggap/ggap/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = node_if_else (parser, (yyvsp[-3].node), (yyvsp[-1].node), NULL); ;}
    break;

  case 20:
#line 426 "/home/muntyan/projects/ggap/ggap/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = node_if_else (parser, (yyvsp[-5].node), (yyvsp[-3].node), (yyvsp[-1].node)); ;}
    break;

  case 24:
#line 435 "/home/muntyan/projects/ggap/ggap/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = node_assignment (parser, (yyvsp[-2].str), (yyvsp[0].node)); ;}
    break;

  case 25:
#line 436 "/home/muntyan/projects/ggap/ggap/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = node_list_assign (parser, (yyvsp[-5].node), (yyvsp[-3].node), (yyvsp[0].node)); ;}
    break;

  case 26:
#line 437 "/home/muntyan/projects/ggap/ggap/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = node_dict_assign (parser, (yyvsp[-4].node), (yyvsp[-2].str), (yyvsp[0].node)); ;}
    break;

  case 27:
#line 440 "/home/muntyan/projects/ggap/ggap/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = node_if_else (parser, (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); ;}
    break;

  case 28:
#line 444 "/home/muntyan/projects/ggap/ggap/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = node_binary_op (parser, MS_OP_PLUS, (yyvsp[-2].node), (yyvsp[0].node)); ;}
    break;

  case 29:
#line 445 "/home/muntyan/projects/ggap/ggap/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = node_binary_op (parser, MS_OP_MINUS, (yyvsp[-2].node), (yyvsp[0].node)); ;}
    break;

  case 30:
#line 446 "/home/muntyan/projects/ggap/ggap/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = node_binary_op (parser, MS_OP_DIV, (yyvsp[-2].node), (yyvsp[0].node)); ;}
    break;

  case 31:
#line 447 "/home/muntyan/projects/ggap/ggap/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = node_binary_op (parser, MS_OP_MULT, (yyvsp[-2].node), (yyvsp[0].node)); ;}
    break;

  case 32:
#line 449 "/home/muntyan/projects/ggap/ggap/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = node_binary_op (parser, MS_OP_AND, (yyvsp[-2].node), (yyvsp[0].node)); ;}
    break;

  case 33:
#line 450 "/home/muntyan/projects/ggap/ggap/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = node_binary_op (parser, MS_OP_OR, (yyvsp[-2].node), (yyvsp[0].node)); ;}
    break;

  case 34:
#line 452 "/home/muntyan/projects/ggap/ggap/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = node_binary_op (parser, MS_OP_EQ, (yyvsp[-2].node), (yyvsp[0].node)); ;}
    break;

  case 35:
#line 453 "/home/muntyan/projects/ggap/ggap/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = node_binary_op (parser, MS_OP_NEQ, (yyvsp[-2].node), (yyvsp[0].node)); ;}
    break;

  case 36:
#line 454 "/home/muntyan/projects/ggap/ggap/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = node_binary_op (parser, MS_OP_LT, (yyvsp[-2].node), (yyvsp[0].node)); ;}
    break;

  case 37:
#line 455 "/home/muntyan/projects/ggap/ggap/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = node_binary_op (parser, MS_OP_GT, (yyvsp[-2].node), (yyvsp[0].node)); ;}
    break;

  case 38:
#line 456 "/home/muntyan/projects/ggap/ggap/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = node_binary_op (parser, MS_OP_LE, (yyvsp[-2].node), (yyvsp[0].node)); ;}
    break;

  case 39:
#line 457 "/home/muntyan/projects/ggap/ggap/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = node_binary_op (parser, MS_OP_GE, (yyvsp[-2].node), (yyvsp[0].node)); ;}
    break;

  case 40:
#line 458 "/home/muntyan/projects/ggap/ggap/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = node_unary_op (parser, MS_OP_UMINUS, (yyvsp[0].node)); ;}
    break;

  case 41:
#line 459 "/home/muntyan/projects/ggap/ggap/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = node_unary_op (parser, MS_OP_NOT, (yyvsp[0].node)); ;}
    break;

  case 42:
#line 460 "/home/muntyan/projects/ggap/ggap/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = node_unary_op (parser, MS_OP_LEN, (yyvsp[0].node)); ;}
    break;

  case 43:
#line 461 "/home/muntyan/projects/ggap/ggap/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = node_binary_op (parser, MS_OP_FORMAT, (yyvsp[-2].node), (yyvsp[0].node)); ;}
    break;

  case 44:
#line 465 "/home/muntyan/projects/ggap/ggap/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = node_int (parser, (yyvsp[0].ival)); ;}
    break;

  case 45:
#line 466 "/home/muntyan/projects/ggap/ggap/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = node_string (parser, (yyvsp[0].str)); ;}
    break;

  case 47:
#line 468 "/home/muntyan/projects/ggap/ggap/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = (yyvsp[-1].node); ;}
    break;

  case 48:
#line 469 "/home/muntyan/projects/ggap/ggap/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = node_value_list (parser, MS_NODE_LIST ((yyvsp[-1].node))); ;}
    break;

  case 49:
#line 470 "/home/muntyan/projects/ggap/ggap/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = node_dict (parser, (yyvsp[-1].node) ? MS_NODE_LIST ((yyvsp[-1].node)) : NULL); ;}
    break;

  case 50:
#line 471 "/home/muntyan/projects/ggap/ggap/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = node_value_range (parser, (yyvsp[-3].node), (yyvsp[-1].node)); ;}
    break;

  case 51:
#line 472 "/home/muntyan/projects/ggap/ggap/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = node_command (parser, (yyvsp[-3].str), (yyvsp[-1].node) ? MS_NODE_LIST ((yyvsp[-1].node)) : NULL); ;}
    break;

  case 52:
#line 473 "/home/muntyan/projects/ggap/ggap/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = node_list_elm (parser, (yyvsp[-3].node), (yyvsp[-1].node)); ;}
    break;

  case 53:
#line 474 "/home/muntyan/projects/ggap/ggap/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = node_dict_elm (parser, (yyvsp[-2].node), (yyvsp[0].str)); ;}
    break;

  case 54:
#line 477 "/home/muntyan/projects/ggap/ggap/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = NULL; ;}
    break;

  case 55:
#line 478 "/home/muntyan/projects/ggap/ggap/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = node_list_add (parser, NULL, (yyvsp[0].node)); ;}
    break;

  case 56:
#line 479 "/home/muntyan/projects/ggap/ggap/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = node_list_add (parser, MS_NODE_LIST ((yyvsp[-2].node)), (yyvsp[0].node)); ;}
    break;

  case 57:
#line 482 "/home/muntyan/projects/ggap/ggap/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = NULL; ;}
    break;

  case 58:
#line 483 "/home/muntyan/projects/ggap/ggap/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = node_list_add (parser, NULL, (yyvsp[0].node)); ;}
    break;

  case 59:
#line 484 "/home/muntyan/projects/ggap/ggap/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = node_list_add (parser, MS_NODE_LIST ((yyvsp[-2].node)), (yyvsp[0].node)); ;}
    break;

  case 60:
#line 487 "/home/muntyan/projects/ggap/ggap/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = node_dict_entry (parser, (yyvsp[-2].str), (yyvsp[0].node)); ;}
    break;

  case 61:
#line 490 "/home/muntyan/projects/ggap/ggap/moo/mooutils/mooscript/mooscript-yacc.y"
    { (yyval.node) = node_var (parser, (yyvsp[0].str)); ;}
    break;


      default: break;
    }

/* Line 1126 of yacc.c.  */
#line 1975 "/home/muntyan/projects/ggap/ggap/moo/mooutils/mooscript/mooscript-yacc.c"

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


#line 493 "/home/muntyan/projects/ggap/ggap/moo/mooutils/mooscript/mooscript-yacc.y"


