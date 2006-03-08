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




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 360 "/home/muntyan/projects/moo/moo/mooutils/mooscript/mooscript-yacc.y"
typedef union YYSTYPE {
    int ival;
    const char *str;
    MSNode *node;
} YYSTYPE;
/* Line 1447 of yacc.c.  */
#line 100 "/home/muntyan/projects/moo/moo/mooutils/mooscript/mooscript-yacc.h"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE _ms_script_yylval;



