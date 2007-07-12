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
     NUMBER = 261,
     IF = 262,
     THEN = 263,
     ELSE = 264,
     ELIF = 265,
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
#define NUMBER 261
#define IF 262
#define THEN 263
#define ELSE 264
#define ELIF 265
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
#line 1 "/home/muntyan/projects/moo/moo/mooscript/mooscript.y"

#include "mooscript-parser-private.h"
#include "mooscript-yacc.h"


static MSNode *
node_list_add (MSParser   *parser,
               MSNodeList *list,
               MSNode     *node)
{
    if (!list)
    {
        list = _ms_node_list_new ();
        _ms_parser_add_node (parser, list);
    }

    if (!node)
    {
        MSValue *none = ms_value_none ();
        node = MS_NODE (_ms_node_value_new (none));
        ms_value_unref (none);
        _ms_parser_add_node (parser, node);
    }

    _ms_node_list_add (list, node);
    return MS_NODE (list);
}


static MSNode *
node_function (MSParser   *parser,
               MSNode     *func,
               MSNodeList *args)
{
    MSNodeFunction *node;

    g_return_val_if_fail (func != NULL, NULL);
    g_return_val_if_fail (!args || MS_IS_NODE_LIST (args), NULL);

    node = _ms_node_function_new (func, args);
    _ms_parser_add_node (parser, node);

    return MS_NODE (node);
}


static MSNode *
node_if_else (MSParser   *parser,
              MSNode     *condition,
              MSNode     *then_,
              MSNodeList *elif_,
              MSNode     *else_)
{
    MSNodeIfElse *node;

    g_return_val_if_fail (condition && then_, NULL);

    node = _ms_node_if_else_new (condition, then_, elif_, else_);
    _ms_parser_add_node (parser, node);

    return MS_NODE (node);
}

static MSNode *
node_condition (MSParser   *parser,
                MSNode     *condition,
                MSNode     *then_)
{
    MSNode *tuple;
    tuple = node_list_add (parser, NULL, condition);
    tuple = node_list_add (parser, MS_NODE_LIST (tuple), then_);
    return tuple;
}


static MSNode *
node_while (MSParser   *parser,
            MSCondType  type,
            MSNode     *cond,
            MSNode     *what)
{
    MSNodeWhile *loop;

    g_return_val_if_fail (cond != NULL, NULL);

    loop = _ms_node_while_new (type, cond, what);
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

    loop = _ms_node_for_new (var, list, what);
    _ms_parser_add_node (parser, loop);

    return MS_NODE (loop);
}


static MSNode *
node_var (MSParser   *parser,
          const char *name)
{
    MSNodeVar *node;

    node = _ms_node_var_new (name);
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
    node = _ms_node_assign_new (MS_NODE_VAR (var), val);
    _ms_parser_add_node (parser, node);

    return MS_NODE (node);
}


static MSNode *
node_binary_op (MSParser   *parser,
                MSBinaryOp  op,
                MSNode     *lval,
                MSNode     *rval)
{
    MSNodeFunction *node;

    g_return_val_if_fail (lval && rval, NULL);

    node = _ms_node_binary_op_new (op, lval, rval);
    _ms_parser_add_node (parser, node);

    return MS_NODE (node);
}


static MSNode *
node_unary_op (MSParser   *parser,
               MSUnaryOp   op,
               MSNode     *val)
{
    MSNodeFunction *node;

    g_return_val_if_fail (val != NULL, NULL);

    node = _ms_node_unary_op_new (op, val);
    _ms_parser_add_node (parser, node);

    return MS_NODE (node);
}


static MSNode *
node_int (MSParser   *parser,
          int         n)
{
    MSNodeValue *node;
    MSValue *value;

    value = ms_value_int (n);
    node = _ms_node_value_new (value);
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
    node = _ms_node_value_new (value);
    ms_value_unref (value);
    _ms_parser_add_node (parser, node);

    return MS_NODE (node);
}


static MSNode *
node_value_list (MSParser   *parser,
                 MSNodeList *list)
{
    MSNodeValList *node;

    node = _ms_node_val_list_new (list);
    _ms_parser_add_node (parser, node);

    return MS_NODE (node);
}


static MSNode *
node_value_range (MSParser   *parser,
                  MSNode     *first,
                  MSNode     *last)
{
    MSNodeValList *node;

    node = _ms_node_val_range_new (first, last);
    _ms_parser_add_node (parser, node);

    return MS_NODE (node);
}


static MSNode *
node_get_item (MSParser   *parser,
               MSNode     *obj,
               MSNode     *key)
{
    MSNodeGetItem *node;

    node = _ms_node_get_item_new (obj, key);
    _ms_parser_add_node (parser, node);

    return MS_NODE (node);
}


static MSNode *
node_set_item (MSParser   *parser,
               MSNode     *obj,
               MSNode     *key,
               MSNode     *val)
{
    MSNodeSetItem *node;

    node = _ms_node_set_item_new (obj, key, val);
    _ms_parser_add_node (parser, node);

    return MS_NODE (node);
}


static MSNode *
node_dict (MSParser   *parser,
           MSNodeList *list)
{
    MSNodeDict *node;

    node = _ms_node_dict_new (list);
    _ms_parser_add_node (parser, node);

    return MS_NODE (node);
}


static MSNode *
node_dict_entry (MSParser   *parser,
                 const char *key,
                 MSNode     *val)
{
    MSNodeDictEntry *node;

    node = _ms_node_dict_entry_new (key, val);
    _ms_parser_add_node (parser, node);

    return MS_NODE (node);
}


static MSNode *
node_return (MSParser   *parser,
             MSNode     *val)
{
    MSNodeReturn *node;

    node = _ms_node_return_new (val);
    _ms_parser_add_node (parser, node);

    return MS_NODE (node);
}


static MSNode *
node_break (MSParser *parser)
{
    MSNodeBreak *node;

    node = _ms_node_break_new (MS_BREAK_BREAK);
    _ms_parser_add_node (parser, node);

    return MS_NODE (node);
}


static MSNode *
node_continue (MSParser *parser)
{
    MSNodeBreak *node;

    node = _ms_node_break_new (MS_BREAK_CONTINUE);
    _ms_parser_add_node (parser, node);

    return MS_NODE (node);
}


static MSNode *
node_env_var (MSParser   *parser,
              MSNode     *var,
              MSNode     *deflt)
{
    MSNodeEnvVar *node;

    node = _ms_node_env_var_new (var, deflt);
    _ms_parser_add_node (parser, node);

    return MS_NODE (node);
}


static MSNode *
node_dict_elm (MSParser   *parser,
               MSNode     *dict,
               const char *key)
{
    MSNodeDictElm *node;

    node = _ms_node_dict_elm_new (dict, key);
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

    node = _ms_node_dict_assign_new (dict, key, val);
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
# define YYERROR_VERBOSE 1
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif

#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 377 "/home/muntyan/projects/moo/moo/mooscript/mooscript.y"
{
    int ival;
    const char *str;
    MSNode *node;
}
/* Line 193 of yacc.c.  */
#line 536 "/home/muntyan/projects/moo/moo/mooscript/mooscript-yacc.c"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 549 "/home/muntyan/projects/moo/moo/mooscript/mooscript-yacc.c"

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
#define YYFINAL  54
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   991

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  50
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  17
/* YYNRULES -- Number of rules.  */
#define YYNRULES  73
/* YYNRULES -- Number of states.  */
#define YYNSTATES  161

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   283

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,    36,    48,    33,     2,     2,
      44,    45,    31,    30,    49,    29,    41,    32,     2,     2,
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
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     5,     7,    10,    13,    16,    17,    19,
      21,    23,    25,    27,    29,    31,    34,    40,    45,    53,
      59,    67,    74,    83,    88,    94,    96,    98,   100,   104,
     111,   118,   124,   130,   134,   138,   142,   146,   150,   154,
     158,   162,   166,   170,   174,   178,   181,   184,   187,   191,
     195,   197,   199,   201,   205,   209,   213,   217,   221,   225,
     231,   236,   241,   245,   250,   257,   260,   261,   263,   267,
     268,   270,   274,   278
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      51,     0,    -1,    52,    -1,    53,    -1,    52,    53,    -1,
       1,    37,    -1,    54,    37,    -1,    -1,    58,    -1,    56,
      -1,    55,    -1,    59,    -1,    17,    -1,    18,    -1,    19,
      -1,    19,    58,    -1,    12,    58,    13,    52,    14,    -1,
      13,    52,    12,    58,    -1,    15,    66,    16,    58,    13,
      52,    14,    -1,     7,    58,     8,    52,    11,    -1,     7,
      58,     8,    52,     9,    52,    11,    -1,     7,    58,     8,
      52,    57,    11,    -1,     7,    58,     8,    52,    57,     9,
      52,    11,    -1,    10,    58,     8,    52,    -1,    57,    10,
      58,     8,    52,    -1,    62,    -1,    61,    -1,    60,    -1,
       3,    38,    58,    -1,    62,    39,    58,    40,    38,    58,
      -1,    62,    39,     1,    40,    38,    58,    -1,    62,    41,
       3,    38,    58,    -1,    62,    42,    62,    43,    62,    -1,
      58,    30,    58,    -1,    58,    29,    58,    -1,    58,    32,
      58,    -1,    58,    31,    58,    -1,    58,    24,    58,    -1,
      58,    25,    58,    -1,    58,    20,    58,    -1,    58,    21,
      58,    -1,    58,    34,    58,    -1,    58,    35,    58,    -1,
      58,    22,    58,    -1,    58,    23,    58,    -1,    29,    62,
      -1,    26,    62,    -1,    36,    62,    -1,    62,    33,    62,
      -1,    62,    16,    62,    -1,     6,    -1,     4,    -1,    66,
      -1,    44,    54,    45,    -1,    44,     1,    45,    -1,    39,
      63,    40,    -1,    39,     1,    40,    -1,    46,    64,    47,
      -1,    46,     1,    47,    -1,    39,    58,    28,    58,    40,
      -1,    62,    44,    63,    45,    -1,    62,    39,    58,    40,
      -1,    62,    41,     3,    -1,    48,    44,    54,    45,    -1,
      48,    44,    54,    49,    54,    45,    -1,    48,     3,    -1,
      -1,    58,    -1,    63,    49,    58,    -1,    -1,    65,    -1,
      64,    49,    65,    -1,     3,    38,    58,    -1,     3,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   413,   413,   416,   417,   421,   422,   425,   426,   427,
     428,   429,   430,   431,   432,   433,   436,   437,   438,   442,
     443,   444,   445,   450,   451,   454,   455,   456,   460,   461,
     462,   463,   466,   470,   471,   472,   473,   475,   476,   478,
     479,   480,   481,   482,   483,   484,   485,   486,   487,   489,
     493,   494,   495,   496,   497,   498,   499,   500,   501,   502,
     503,   504,   505,   506,   507,   508,   511,   512,   513,   516,
     517,   518,   521,   524
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "IDENTIFIER", "LITERAL", "VARIABLE",
  "NUMBER", "IF", "THEN", "ELSE", "ELIF", "FI", "WHILE", "DO", "OD", "FOR",
  "IN", "CONTINUE", "BREAK", "RETURN", "EQ", "NEQ", "LE", "GE", "AND",
  "OR", "NOT", "UMINUS", "TWODOTS", "'-'", "'+'", "'*'", "'/'", "'%'",
  "'<'", "'>'", "'#'", "';'", "'='", "'['", "']'", "'.'", "'?'", "':'",
  "'('", "')'", "'{'", "'}'", "'$'", "','", "$accept", "script", "program",
  "stmt_or_error", "stmt", "loop", "if_stmt", "elif_block", "expr",
  "assignment", "ternary", "compound_expr", "simple_expr", "list_elms",
  "dict_elms", "dict_entry", "variable", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,    45,
      43,    42,    47,    37,    60,    62,    35,    59,    61,    91,
      93,    46,    63,    58,    40,    41,   123,   125,    36,    44
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    50,    51,    52,    52,    53,    53,    54,    54,    54,
      54,    54,    54,    54,    54,    54,    55,    55,    55,    56,
      56,    56,    56,    57,    57,    58,    58,    58,    59,    59,
      59,    59,    60,    61,    61,    61,    61,    61,    61,    61,
      61,    61,    61,    61,    61,    61,    61,    61,    61,    61,
      62,    62,    62,    62,    62,    62,    62,    62,    62,    62,
      62,    62,    62,    62,    62,    62,    63,    63,    63,    64,
      64,    64,    65,    66
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     2,     2,     2,     0,     1,     1,
       1,     1,     1,     1,     1,     2,     5,     4,     7,     5,
       7,     6,     8,     4,     5,     1,     1,     1,     3,     6,
       6,     5,     5,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     2,     2,     2,     3,     3,
       1,     1,     1,     3,     3,     3,     3,     3,     3,     5,
       4,     4,     3,     4,     6,     2,     0,     1,     3,     0,
       1,     3,     3,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,    73,    51,    50,     0,     0,     0,     0,    12,
      13,    14,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     3,     0,    10,     9,     8,    11,    27,    26,    25,
      52,     5,     0,    73,     0,    25,     0,     0,     0,    15,
      46,    45,    47,     0,    67,     0,     0,     0,     0,     0,
       0,    70,    65,     7,     1,     4,     6,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    66,    28,     0,     0,     0,     0,
       0,     0,    56,     0,    55,     0,    54,    53,    58,     0,
      57,     0,     0,    39,    40,    43,    44,    37,    38,    34,
      33,    36,    35,    41,    42,    49,    48,     0,     0,    62,
       0,    67,     0,     0,     0,    62,     0,    17,     0,     0,
      68,    72,    71,    63,     7,     0,    61,     0,     0,    60,
       0,     0,    19,     0,    61,    16,     0,    59,     0,     0,
       0,    31,    32,     0,     0,     0,     0,    21,     0,    64,
      30,    29,    20,     0,     0,     0,    18,     0,    22,     0,
       0
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    19,    20,    21,    22,    23,    24,   133,    25,    26,
      27,    28,    35,    45,    50,    51,    30
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -41
static const yytype_int16 yypact[] =
{
     565,   -27,   -10,   -41,   -41,   724,   724,   565,    26,   -41,
     -41,   724,    94,    94,    94,   194,   611,     5,    -2,    35,
     173,   -41,     7,   -41,   -41,   924,   -41,   -41,   -41,     6,
     -41,   -41,   724,   -41,   753,    78,   809,   657,    27,   924,
     143,   143,   143,    11,   908,   -38,    16,    22,    24,    36,
     -22,   -41,   -41,   703,   -41,   -41,   -41,   724,   724,   724,
     724,   724,   724,   724,   724,   724,   724,   724,   724,    94,
      94,    20,    62,    94,   724,   924,   565,   724,    74,   565,
     724,   724,   -41,   724,   -41,   724,   -41,   -41,   -41,   724,
     -41,    75,   -40,    -7,    -7,    -7,    -7,   -41,    56,   940,
     940,   956,   956,    -7,    -7,   143,   143,    41,   845,    57,
      19,   924,   -30,   243,   866,   -41,   381,   809,   829,   887,
     924,   924,   -41,   -41,   703,    63,    65,   724,    94,   -41,
     565,   724,   -41,    23,   -41,   -41,   565,   -41,    59,   724,
     724,   924,   143,   427,   771,   565,   724,   -41,   473,   -41,
     924,   924,   -41,   565,   519,   789,   -41,   289,   -41,   565,
     335
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -41,   -41,    -4,    18,   -12,   -41,   -41,   -41,    25,   -41,
     -41,   -41,     0,    33,   -41,    30,   101
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -70
static const yytype_int16 yytable[] =
{
      29,    52,    84,    37,    47,   123,    48,    29,    49,   124,
      31,    85,    40,    41,    42,   129,    29,    61,    62,    85,
      29,   107,    69,    33,     3,    90,     4,    91,    32,    33,
      34,    36,   145,   146,   147,    54,    39,    29,    55,    70,
      44,    92,    53,    81,    56,    71,    12,    72,    73,    13,
      74,    82,   -69,    29,   -69,    55,    14,    75,    77,    15,
      78,    86,   128,    74,    16,   109,    17,    87,    18,   105,
     106,    88,   113,   110,    89,   116,    29,   115,    49,    29,
      61,   125,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,    69,   127,   108,    33,     3,   111,
       4,   139,   114,   140,   149,   117,   118,   112,   119,    38,
     120,    70,   138,    29,   121,     0,    29,    77,     0,    78,
      73,   122,    74,     0,    29,     0,   143,     0,   142,     0,
      29,    55,   148,    15,    55,     0,    29,     0,    16,     0,
      17,   154,    18,    29,     0,    29,     0,     0,    29,   157,
       0,     0,   141,    29,    29,   160,   144,    29,     0,    29,
      29,    55,     0,     0,   150,   151,    55,     0,     0,     0,
       0,   155,    55,    -2,     1,    55,     2,     3,    55,     4,
       5,     0,    77,     0,    78,     6,     7,    74,     8,     0,
       9,    10,    11,     0,     0,    43,     0,    33,     3,    12,
       4,     0,    13,     0,     0,     0,     0,     0,     0,    14,
      -7,     0,    15,     0,     0,     0,     0,    16,     0,    17,
      12,    18,     0,    13,     0,     0,     0,     0,     0,     0,
      14,     0,     0,    15,   -66,     0,     0,     0,    16,     0,
      17,     0,    18,   -66,     1,     0,     2,     3,     0,     4,
       5,     0,   130,   131,   132,     6,     7,     0,     8,     0,
       9,    10,    11,     0,     0,     0,     0,     0,     0,    12,
       0,     0,    13,     0,     0,     0,     0,     0,     0,    14,
      -7,     0,    15,     0,     0,     0,     0,    16,     0,    17,
       1,    18,     2,     3,     0,     4,     5,     0,   -23,   -23,
     -23,     6,     7,     0,     8,     0,     9,    10,    11,     0,
       0,     0,     0,     0,     0,    12,     0,     0,    13,     0,
       0,     0,     0,     0,     0,    14,    -7,     0,    15,     0,
       0,     0,     0,    16,     0,    17,     1,    18,     2,     3,
       0,     4,     5,     0,   -24,   -24,   -24,     6,     7,     0,
       8,     0,     9,    10,    11,     0,     0,     0,     0,     0,
       0,    12,     0,     0,    13,     0,     0,     0,     0,     0,
       0,    14,    -7,     0,    15,     0,     0,     0,     0,    16,
       0,    17,     1,    18,     2,     3,     0,     4,     5,     0,
       0,     0,     0,     6,     7,   135,     8,     0,     9,    10,
      11,     0,     0,     0,     0,     0,     0,    12,     0,     0,
      13,     0,     0,     0,     0,     0,     0,    14,    -7,     0,
      15,     0,     0,     0,     0,    16,     0,    17,     1,    18,
       2,     3,     0,     4,     5,     0,     0,     0,   152,     6,
       7,     0,     8,     0,     9,    10,    11,     0,     0,     0,
       0,     0,     0,    12,     0,     0,    13,     0,     0,     0,
       0,     0,     0,    14,    -7,     0,    15,     0,     0,     0,
       0,    16,     0,    17,     1,    18,     2,     3,     0,     4,
       5,     0,     0,     0,     0,     6,     7,   156,     8,     0,
       9,    10,    11,     0,     0,     0,     0,     0,     0,    12,
       0,     0,    13,     0,     0,     0,     0,     0,     0,    14,
      -7,     0,    15,     0,     0,     0,     0,    16,     0,    17,
       1,    18,     2,     3,     0,     4,     5,     0,     0,     0,
     158,     6,     7,     0,     8,     0,     9,    10,    11,     0,
       0,     0,     0,     0,     0,    12,     0,     0,    13,     0,
       0,     0,     0,     0,     0,    14,    -7,     0,    15,     0,
       0,     0,     0,    16,     0,    17,     1,    18,     2,     3,
       0,     4,     5,     0,     0,     0,     0,     6,     7,     0,
       8,     0,     9,    10,    11,     0,     0,     0,     0,     0,
       0,    12,     0,     0,    13,     0,     0,     0,     0,     0,
       0,    14,    -7,     0,    15,     0,     0,     0,     0,    16,
       0,    17,    46,    18,     2,     3,     0,     4,     5,     0,
       0,     0,     0,     6,     7,     0,     8,     0,     9,    10,
      11,     0,     0,     0,     0,     0,     0,    12,     0,     0,
      13,     0,     0,     0,     0,     0,     0,    14,     0,     0,
      15,     0,     0,     0,     0,    16,    -7,    17,     1,    18,
       2,     3,     0,     4,     5,     0,     0,     0,     0,    80,
       7,     0,     8,     0,     9,    10,    11,     0,     0,     0,
       0,     0,     0,    12,     0,     0,    13,     0,     0,     0,
       0,     0,     0,    14,    -7,     0,    15,     0,     0,     0,
       0,    16,     0,    17,     0,    18,     2,     3,     0,     4,
       5,     0,     0,     0,     0,     6,     7,     0,     8,     0,
       9,    10,    11,     0,     0,     0,     0,    33,     3,    12,
       4,     0,    13,     0,     0,     0,     0,     0,     0,    14,
       0,     0,    15,     0,     0,     0,     0,    16,     0,    17,
      12,    18,     0,    13,     0,     0,     0,     0,     0,     0,
      14,    76,     0,    15,     0,     0,     0,     0,    16,     0,
      17,     0,    18,    57,    58,    59,    60,    61,    62,   153,
       0,     0,    63,    64,    65,    66,     0,    67,    68,     0,
       0,    57,    58,    59,    60,    61,    62,   159,     0,     0,
      63,    64,    65,    66,     0,    67,    68,     0,     0,    57,
      58,    59,    60,    61,    62,     0,     0,     0,    63,    64,
      65,    66,    79,    67,    68,     0,     0,     0,     0,    57,
      58,    59,    60,    61,    62,     0,     0,     0,    63,    64,
      65,    66,   136,    67,    68,     0,     0,     0,     0,    57,
      58,    59,    60,    61,    62,     0,     0,     0,    63,    64,
      65,    66,     0,    67,    68,    57,    58,    59,    60,    61,
      62,     0,     0,     0,    63,    64,    65,    66,     0,    67,
      68,     0,     0,     0,     0,   126,    57,    58,    59,    60,
      61,    62,     0,     0,     0,    63,    64,    65,    66,     0,
      67,    68,     0,     0,     0,     0,   134,    57,    58,    59,
      60,    61,    62,     0,     0,     0,    63,    64,    65,    66,
       0,    67,    68,     0,     0,     0,     0,   137,    57,    58,
      59,    60,    61,    62,     0,     0,    83,    63,    64,    65,
      66,     0,    67,    68,    57,    58,    59,    60,    61,    62,
       0,     0,     0,    63,    64,    65,    66,     0,    67,    68,
      57,    58,    59,    60,    61,    62,     0,     0,     0,     0,
       0,    65,    66,     0,    67,    68,    57,    58,    59,    60,
      61,    62,     0,     0,     0,     0,     0,     0,     0,     0,
      67,    68
};

static const yytype_int16 yycheck[] =
{
       0,     3,    40,     7,    16,    45,     1,     7,     3,    49,
      37,    49,    12,    13,    14,    45,    16,    24,    25,    49,
      20,     1,    16,     3,     4,    47,     6,    49,    38,     3,
       5,     6,     9,    10,    11,     0,    11,    37,    20,    33,
      15,    53,    44,    16,    37,    39,    26,    41,    42,    29,
      44,    40,    47,    53,    49,    37,    36,    32,    39,    39,
      41,    45,    43,    44,    44,     3,    46,    45,    48,    69,
      70,    47,    76,    73,    38,    79,    76,     3,     3,    79,
      24,    40,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    16,    38,    71,     3,     4,    74,
       6,    38,    77,    38,    45,    80,    81,    74,    83,     8,
      85,    33,   124,   113,    89,    -1,   116,    39,    -1,    41,
      42,    91,    44,    -1,   124,    -1,   130,    -1,   128,    -1,
     130,   113,   136,    39,   116,    -1,   136,    -1,    44,    -1,
      46,   145,    48,   143,    -1,   145,    -1,    -1,   148,   153,
      -1,    -1,   127,   153,   154,   159,   131,   157,    -1,   159,
     160,   143,    -1,    -1,   139,   140,   148,    -1,    -1,    -1,
      -1,   146,   154,     0,     1,   157,     3,     4,   160,     6,
       7,    -1,    39,    -1,    41,    12,    13,    44,    15,    -1,
      17,    18,    19,    -1,    -1,     1,    -1,     3,     4,    26,
       6,    -1,    29,    -1,    -1,    -1,    -1,    -1,    -1,    36,
      37,    -1,    39,    -1,    -1,    -1,    -1,    44,    -1,    46,
      26,    48,    -1,    29,    -1,    -1,    -1,    -1,    -1,    -1,
      36,    -1,    -1,    39,    40,    -1,    -1,    -1,    44,    -1,
      46,    -1,    48,    49,     1,    -1,     3,     4,    -1,     6,
       7,    -1,     9,    10,    11,    12,    13,    -1,    15,    -1,
      17,    18,    19,    -1,    -1,    -1,    -1,    -1,    -1,    26,
      -1,    -1,    29,    -1,    -1,    -1,    -1,    -1,    -1,    36,
      37,    -1,    39,    -1,    -1,    -1,    -1,    44,    -1,    46,
       1,    48,     3,     4,    -1,     6,     7,    -1,     9,    10,
      11,    12,    13,    -1,    15,    -1,    17,    18,    19,    -1,
      -1,    -1,    -1,    -1,    -1,    26,    -1,    -1,    29,    -1,
      -1,    -1,    -1,    -1,    -1,    36,    37,    -1,    39,    -1,
      -1,    -1,    -1,    44,    -1,    46,     1,    48,     3,     4,
      -1,     6,     7,    -1,     9,    10,    11,    12,    13,    -1,
      15,    -1,    17,    18,    19,    -1,    -1,    -1,    -1,    -1,
      -1,    26,    -1,    -1,    29,    -1,    -1,    -1,    -1,    -1,
      -1,    36,    37,    -1,    39,    -1,    -1,    -1,    -1,    44,
      -1,    46,     1,    48,     3,     4,    -1,     6,     7,    -1,
      -1,    -1,    -1,    12,    13,    14,    15,    -1,    17,    18,
      19,    -1,    -1,    -1,    -1,    -1,    -1,    26,    -1,    -1,
      29,    -1,    -1,    -1,    -1,    -1,    -1,    36,    37,    -1,
      39,    -1,    -1,    -1,    -1,    44,    -1,    46,     1,    48,
       3,     4,    -1,     6,     7,    -1,    -1,    -1,    11,    12,
      13,    -1,    15,    -1,    17,    18,    19,    -1,    -1,    -1,
      -1,    -1,    -1,    26,    -1,    -1,    29,    -1,    -1,    -1,
      -1,    -1,    -1,    36,    37,    -1,    39,    -1,    -1,    -1,
      -1,    44,    -1,    46,     1,    48,     3,     4,    -1,     6,
       7,    -1,    -1,    -1,    -1,    12,    13,    14,    15,    -1,
      17,    18,    19,    -1,    -1,    -1,    -1,    -1,    -1,    26,
      -1,    -1,    29,    -1,    -1,    -1,    -1,    -1,    -1,    36,
      37,    -1,    39,    -1,    -1,    -1,    -1,    44,    -1,    46,
       1,    48,     3,     4,    -1,     6,     7,    -1,    -1,    -1,
      11,    12,    13,    -1,    15,    -1,    17,    18,    19,    -1,
      -1,    -1,    -1,    -1,    -1,    26,    -1,    -1,    29,    -1,
      -1,    -1,    -1,    -1,    -1,    36,    37,    -1,    39,    -1,
      -1,    -1,    -1,    44,    -1,    46,     1,    48,     3,     4,
      -1,     6,     7,    -1,    -1,    -1,    -1,    12,    13,    -1,
      15,    -1,    17,    18,    19,    -1,    -1,    -1,    -1,    -1,
      -1,    26,    -1,    -1,    29,    -1,    -1,    -1,    -1,    -1,
      -1,    36,    37,    -1,    39,    -1,    -1,    -1,    -1,    44,
      -1,    46,     1,    48,     3,     4,    -1,     6,     7,    -1,
      -1,    -1,    -1,    12,    13,    -1,    15,    -1,    17,    18,
      19,    -1,    -1,    -1,    -1,    -1,    -1,    26,    -1,    -1,
      29,    -1,    -1,    -1,    -1,    -1,    -1,    36,    -1,    -1,
      39,    -1,    -1,    -1,    -1,    44,    45,    46,     1,    48,
       3,     4,    -1,     6,     7,    -1,    -1,    -1,    -1,    12,
      13,    -1,    15,    -1,    17,    18,    19,    -1,    -1,    -1,
      -1,    -1,    -1,    26,    -1,    -1,    29,    -1,    -1,    -1,
      -1,    -1,    -1,    36,    37,    -1,    39,    -1,    -1,    -1,
      -1,    44,    -1,    46,    -1,    48,     3,     4,    -1,     6,
       7,    -1,    -1,    -1,    -1,    12,    13,    -1,    15,    -1,
      17,    18,    19,    -1,    -1,    -1,    -1,     3,     4,    26,
       6,    -1,    29,    -1,    -1,    -1,    -1,    -1,    -1,    36,
      -1,    -1,    39,    -1,    -1,    -1,    -1,    44,    -1,    46,
      26,    48,    -1,    29,    -1,    -1,    -1,    -1,    -1,    -1,
      36,     8,    -1,    39,    -1,    -1,    -1,    -1,    44,    -1,
      46,    -1,    48,    20,    21,    22,    23,    24,    25,     8,
      -1,    -1,    29,    30,    31,    32,    -1,    34,    35,    -1,
      -1,    20,    21,    22,    23,    24,    25,     8,    -1,    -1,
      29,    30,    31,    32,    -1,    34,    35,    -1,    -1,    20,
      21,    22,    23,    24,    25,    -1,    -1,    -1,    29,    30,
      31,    32,    13,    34,    35,    -1,    -1,    -1,    -1,    20,
      21,    22,    23,    24,    25,    -1,    -1,    -1,    29,    30,
      31,    32,    13,    34,    35,    -1,    -1,    -1,    -1,    20,
      21,    22,    23,    24,    25,    -1,    -1,    -1,    29,    30,
      31,    32,    -1,    34,    35,    20,    21,    22,    23,    24,
      25,    -1,    -1,    -1,    29,    30,    31,    32,    -1,    34,
      35,    -1,    -1,    -1,    -1,    40,    20,    21,    22,    23,
      24,    25,    -1,    -1,    -1,    29,    30,    31,    32,    -1,
      34,    35,    -1,    -1,    -1,    -1,    40,    20,    21,    22,
      23,    24,    25,    -1,    -1,    -1,    29,    30,    31,    32,
      -1,    34,    35,    -1,    -1,    -1,    -1,    40,    20,    21,
      22,    23,    24,    25,    -1,    -1,    28,    29,    30,    31,
      32,    -1,    34,    35,    20,    21,    22,    23,    24,    25,
      -1,    -1,    -1,    29,    30,    31,    32,    -1,    34,    35,
      20,    21,    22,    23,    24,    25,    -1,    -1,    -1,    -1,
      -1,    31,    32,    -1,    34,    35,    20,    21,    22,    23,
      24,    25,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      34,    35
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,     3,     4,     6,     7,    12,    13,    15,    17,
      18,    19,    26,    29,    36,    39,    44,    46,    48,    51,
      52,    53,    54,    55,    56,    58,    59,    60,    61,    62,
      66,    37,    38,     3,    58,    62,    58,    52,    66,    58,
      62,    62,    62,     1,    58,    63,     1,    54,     1,     3,
      64,    65,     3,    44,     0,    53,    37,    20,    21,    22,
      23,    24,    25,    29,    30,    31,    32,    34,    35,    16,
      33,    39,    41,    42,    44,    58,     8,    39,    41,    13,
      12,    16,    40,    28,    40,    49,    45,    45,    47,    38,
      47,    49,    54,    58,    58,    58,    58,    58,    58,    58,
      58,    58,    58,    58,    58,    62,    62,     1,    58,     3,
      62,    58,    63,    52,    58,     3,    52,    58,    58,    58,
      58,    58,    65,    45,    49,    40,    40,    38,    43,    45,
       9,    10,    11,    57,    40,    14,    13,    40,    54,    38,
      38,    58,    62,    52,    58,     9,    10,    11,    52,    45,
      58,    58,    11,     8,    52,    58,    14,    52,    11,     8,
      52
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
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, MSParser *parser)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep, parser)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    MSParser *parser;
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
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, MSParser *parser)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep, parser)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    MSParser *parser;
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
yy_reduce_print (YYSTYPE *yyvsp, int yyrule, MSParser *parser)
#else
static void
yy_reduce_print (yyvsp, yyrule, parser)
    YYSTYPE *yyvsp;
    int yyrule;
    MSParser *parser;
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
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, MSParser *parser)
#else
static void
yydestruct (yymsg, yytype, yyvaluep, parser)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
    MSParser *parser;
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
        case 2:
#line 413 "/home/muntyan/projects/moo/moo/mooscript/mooscript.y"
    { _ms_parser_set_top_node (parser, (yyvsp[(1) - (1)].node)); ;}
    break;

  case 3:
#line 416 "/home/muntyan/projects/moo/moo/mooscript/mooscript.y"
    { (yyval.node) = node_list_add (parser, NULL, (yyvsp[(1) - (1)].node)); ;}
    break;

  case 4:
#line 417 "/home/muntyan/projects/moo/moo/mooscript/mooscript.y"
    { (yyval.node) = node_list_add (parser, MS_NODE_LIST ((yyvsp[(1) - (2)].node)), (yyvsp[(2) - (2)].node)); ;}
    break;

  case 5:
#line 421 "/home/muntyan/projects/moo/moo/mooscript/mooscript.y"
    { (yyval.node) = NULL; ;}
    break;

  case 6:
#line 422 "/home/muntyan/projects/moo/moo/mooscript/mooscript.y"
    { (yyval.node) = (yyvsp[(1) - (2)].node); ;}
    break;

  case 7:
#line 425 "/home/muntyan/projects/moo/moo/mooscript/mooscript.y"
    { (yyval.node) = NULL; ;}
    break;

  case 12:
#line 430 "/home/muntyan/projects/moo/moo/mooscript/mooscript.y"
    { (yyval.node) = node_continue (parser); ;}
    break;

  case 13:
#line 431 "/home/muntyan/projects/moo/moo/mooscript/mooscript.y"
    { (yyval.node) = node_break (parser); ;}
    break;

  case 14:
#line 432 "/home/muntyan/projects/moo/moo/mooscript/mooscript.y"
    { (yyval.node) = node_return (parser, NULL); ;}
    break;

  case 15:
#line 433 "/home/muntyan/projects/moo/moo/mooscript/mooscript.y"
    { (yyval.node) = node_return (parser, (yyvsp[(2) - (2)].node)); ;}
    break;

  case 16:
#line 436 "/home/muntyan/projects/moo/moo/mooscript/mooscript.y"
    { (yyval.node) = node_while (parser, MS_COND_BEFORE, (yyvsp[(2) - (5)].node), (yyvsp[(4) - (5)].node)); ;}
    break;

  case 17:
#line 437 "/home/muntyan/projects/moo/moo/mooscript/mooscript.y"
    { (yyval.node) = node_while (parser, MS_COND_AFTER, (yyvsp[(4) - (4)].node), (yyvsp[(2) - (4)].node)); ;}
    break;

  case 18:
#line 438 "/home/muntyan/projects/moo/moo/mooscript/mooscript.y"
    { (yyval.node) = node_for (parser, (yyvsp[(2) - (7)].node), (yyvsp[(4) - (7)].node), (yyvsp[(6) - (7)].node)); ;}
    break;

  case 19:
#line 442 "/home/muntyan/projects/moo/moo/mooscript/mooscript.y"
    { (yyval.node) = node_if_else (parser, (yyvsp[(2) - (5)].node), (yyvsp[(4) - (5)].node), NULL, NULL); ;}
    break;

  case 20:
#line 443 "/home/muntyan/projects/moo/moo/mooscript/mooscript.y"
    { (yyval.node) = node_if_else (parser, (yyvsp[(2) - (7)].node), (yyvsp[(4) - (7)].node), NULL, (yyvsp[(6) - (7)].node)); ;}
    break;

  case 21:
#line 444 "/home/muntyan/projects/moo/moo/mooscript/mooscript.y"
    { (yyval.node) = node_if_else (parser, (yyvsp[(2) - (6)].node), (yyvsp[(4) - (6)].node), MS_NODE_LIST ((yyvsp[(5) - (6)].node)), NULL); ;}
    break;

  case 22:
#line 446 "/home/muntyan/projects/moo/moo/mooscript/mooscript.y"
    { (yyval.node) = node_if_else (parser, (yyvsp[(2) - (8)].node), (yyvsp[(4) - (8)].node), MS_NODE_LIST ((yyvsp[(5) - (8)].node)), (yyvsp[(7) - (8)].node)); ;}
    break;

  case 23:
#line 450 "/home/muntyan/projects/moo/moo/mooscript/mooscript.y"
    { (yyval.node) = node_list_add (parser, NULL, node_condition (parser, (yyvsp[(2) - (4)].node), (yyvsp[(4) - (4)].node))); ;}
    break;

  case 24:
#line 451 "/home/muntyan/projects/moo/moo/mooscript/mooscript.y"
    { (yyval.node) = node_list_add (parser, MS_NODE_LIST ((yyvsp[(1) - (5)].node)), node_condition (parser, (yyvsp[(3) - (5)].node), (yyvsp[(5) - (5)].node))); ;}
    break;

  case 28:
#line 460 "/home/muntyan/projects/moo/moo/mooscript/mooscript.y"
    { (yyval.node) = node_assignment (parser, (yyvsp[(1) - (3)].str), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 29:
#line 461 "/home/muntyan/projects/moo/moo/mooscript/mooscript.y"
    { (yyval.node) = node_set_item (parser, (yyvsp[(1) - (6)].node), (yyvsp[(3) - (6)].node), (yyvsp[(6) - (6)].node)); ;}
    break;

  case 30:
#line 462 "/home/muntyan/projects/moo/moo/mooscript/mooscript.y"
    { (yyval.node) = NULL; ;}
    break;

  case 31:
#line 463 "/home/muntyan/projects/moo/moo/mooscript/mooscript.y"
    { (yyval.node) = node_dict_assign (parser, (yyvsp[(1) - (5)].node), (yyvsp[(3) - (5)].str), (yyvsp[(5) - (5)].node)); ;}
    break;

  case 32:
#line 466 "/home/muntyan/projects/moo/moo/mooscript/mooscript.y"
    { (yyval.node) = node_if_else (parser, (yyvsp[(1) - (5)].node), (yyvsp[(3) - (5)].node), NULL, (yyvsp[(5) - (5)].node)); ;}
    break;

  case 33:
#line 470 "/home/muntyan/projects/moo/moo/mooscript/mooscript.y"
    { (yyval.node) = node_binary_op (parser, MS_OP_PLUS, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 34:
#line 471 "/home/muntyan/projects/moo/moo/mooscript/mooscript.y"
    { (yyval.node) = node_binary_op (parser, MS_OP_MINUS, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 35:
#line 472 "/home/muntyan/projects/moo/moo/mooscript/mooscript.y"
    { (yyval.node) = node_binary_op (parser, MS_OP_DIV, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 36:
#line 473 "/home/muntyan/projects/moo/moo/mooscript/mooscript.y"
    { (yyval.node) = node_binary_op (parser, MS_OP_MULT, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 37:
#line 475 "/home/muntyan/projects/moo/moo/mooscript/mooscript.y"
    { (yyval.node) = node_binary_op (parser, MS_OP_AND, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 38:
#line 476 "/home/muntyan/projects/moo/moo/mooscript/mooscript.y"
    { (yyval.node) = node_binary_op (parser, MS_OP_OR, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 39:
#line 478 "/home/muntyan/projects/moo/moo/mooscript/mooscript.y"
    { (yyval.node) = node_binary_op (parser, MS_OP_EQ, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 40:
#line 479 "/home/muntyan/projects/moo/moo/mooscript/mooscript.y"
    { (yyval.node) = node_binary_op (parser, MS_OP_NEQ, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 41:
#line 480 "/home/muntyan/projects/moo/moo/mooscript/mooscript.y"
    { (yyval.node) = node_binary_op (parser, MS_OP_LT, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 42:
#line 481 "/home/muntyan/projects/moo/moo/mooscript/mooscript.y"
    { (yyval.node) = node_binary_op (parser, MS_OP_GT, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 43:
#line 482 "/home/muntyan/projects/moo/moo/mooscript/mooscript.y"
    { (yyval.node) = node_binary_op (parser, MS_OP_LE, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 44:
#line 483 "/home/muntyan/projects/moo/moo/mooscript/mooscript.y"
    { (yyval.node) = node_binary_op (parser, MS_OP_GE, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 45:
#line 484 "/home/muntyan/projects/moo/moo/mooscript/mooscript.y"
    { (yyval.node) = node_unary_op (parser, MS_OP_UMINUS, (yyvsp[(2) - (2)].node)); ;}
    break;

  case 46:
#line 485 "/home/muntyan/projects/moo/moo/mooscript/mooscript.y"
    { (yyval.node) = node_unary_op (parser, MS_OP_NOT, (yyvsp[(2) - (2)].node)); ;}
    break;

  case 47:
#line 486 "/home/muntyan/projects/moo/moo/mooscript/mooscript.y"
    { (yyval.node) = node_unary_op (parser, MS_OP_LEN, (yyvsp[(2) - (2)].node)); ;}
    break;

  case 48:
#line 487 "/home/muntyan/projects/moo/moo/mooscript/mooscript.y"
    { (yyval.node) = node_binary_op (parser, MS_OP_FORMAT, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 49:
#line 489 "/home/muntyan/projects/moo/moo/mooscript/mooscript.y"
    { (yyval.node) = node_binary_op (parser, MS_OP_IN, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 50:
#line 493 "/home/muntyan/projects/moo/moo/mooscript/mooscript.y"
    { (yyval.node) = node_int (parser, (yyvsp[(1) - (1)].ival)); ;}
    break;

  case 51:
#line 494 "/home/muntyan/projects/moo/moo/mooscript/mooscript.y"
    { (yyval.node) = node_string (parser, (yyvsp[(1) - (1)].str)); ;}
    break;

  case 53:
#line 496 "/home/muntyan/projects/moo/moo/mooscript/mooscript.y"
    { (yyval.node) = (yyvsp[(2) - (3)].node); ;}
    break;

  case 54:
#line 497 "/home/muntyan/projects/moo/moo/mooscript/mooscript.y"
    { (yyval.node) = NULL; ;}
    break;

  case 55:
#line 498 "/home/muntyan/projects/moo/moo/mooscript/mooscript.y"
    { (yyval.node) = node_value_list (parser, MS_NODE_LIST ((yyvsp[(2) - (3)].node))); ;}
    break;

  case 56:
#line 499 "/home/muntyan/projects/moo/moo/mooscript/mooscript.y"
    { (yyval.node) = NULL; ;}
    break;

  case 57:
#line 500 "/home/muntyan/projects/moo/moo/mooscript/mooscript.y"
    { (yyval.node) = node_dict (parser, (yyvsp[(2) - (3)].node) ? MS_NODE_LIST ((yyvsp[(2) - (3)].node)) : NULL); ;}
    break;

  case 58:
#line 501 "/home/muntyan/projects/moo/moo/mooscript/mooscript.y"
    { (yyval.node) = NULL; ;}
    break;

  case 59:
#line 502 "/home/muntyan/projects/moo/moo/mooscript/mooscript.y"
    { (yyval.node) = node_value_range (parser, (yyvsp[(2) - (5)].node), (yyvsp[(4) - (5)].node)); ;}
    break;

  case 60:
#line 503 "/home/muntyan/projects/moo/moo/mooscript/mooscript.y"
    { (yyval.node) = node_function (parser, (yyvsp[(1) - (4)].node), (yyvsp[(3) - (4)].node) ? MS_NODE_LIST ((yyvsp[(3) - (4)].node)) : NULL); ;}
    break;

  case 61:
#line 504 "/home/muntyan/projects/moo/moo/mooscript/mooscript.y"
    { (yyval.node) = node_get_item (parser, (yyvsp[(1) - (4)].node), (yyvsp[(3) - (4)].node)); ;}
    break;

  case 62:
#line 505 "/home/muntyan/projects/moo/moo/mooscript/mooscript.y"
    { (yyval.node) = node_dict_elm (parser, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].str)); ;}
    break;

  case 63:
#line 506 "/home/muntyan/projects/moo/moo/mooscript/mooscript.y"
    { (yyval.node) = node_env_var (parser, (yyvsp[(3) - (4)].node), NULL); ;}
    break;

  case 64:
#line 507 "/home/muntyan/projects/moo/moo/mooscript/mooscript.y"
    { (yyval.node) = node_env_var (parser, (yyvsp[(3) - (6)].node), (yyvsp[(5) - (6)].node)); ;}
    break;

  case 65:
#line 508 "/home/muntyan/projects/moo/moo/mooscript/mooscript.y"
    { (yyval.node) = node_env_var (parser, node_string (parser, (yyvsp[(2) - (2)].str)), NULL); ;}
    break;

  case 66:
#line 511 "/home/muntyan/projects/moo/moo/mooscript/mooscript.y"
    { (yyval.node) = NULL; ;}
    break;

  case 67:
#line 512 "/home/muntyan/projects/moo/moo/mooscript/mooscript.y"
    { (yyval.node) = node_list_add (parser, NULL, (yyvsp[(1) - (1)].node)); ;}
    break;

  case 68:
#line 513 "/home/muntyan/projects/moo/moo/mooscript/mooscript.y"
    { (yyval.node) = node_list_add (parser, MS_NODE_LIST ((yyvsp[(1) - (3)].node)), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 69:
#line 516 "/home/muntyan/projects/moo/moo/mooscript/mooscript.y"
    { (yyval.node) = NULL; ;}
    break;

  case 70:
#line 517 "/home/muntyan/projects/moo/moo/mooscript/mooscript.y"
    { (yyval.node) = node_list_add (parser, NULL, (yyvsp[(1) - (1)].node)); ;}
    break;

  case 71:
#line 518 "/home/muntyan/projects/moo/moo/mooscript/mooscript.y"
    { (yyval.node) = node_list_add (parser, MS_NODE_LIST ((yyvsp[(1) - (3)].node)), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 72:
#line 521 "/home/muntyan/projects/moo/moo/mooscript/mooscript.y"
    { (yyval.node) = node_dict_entry (parser, (yyvsp[(1) - (3)].str), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 73:
#line 524 "/home/muntyan/projects/moo/moo/mooscript/mooscript.y"
    { (yyval.node) = node_var (parser, (yyvsp[(1) - (1)].str)); ;}
    break;


/* Line 1267 of yacc.c.  */
#line 2371 "/home/muntyan/projects/moo/moo/mooscript/mooscript-yacc.c"
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


#line 527 "/home/muntyan/projects/moo/moo/mooscript/mooscript.y"


