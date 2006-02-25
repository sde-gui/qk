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
     NUMBER = 261,
     IF = 262,
     THEN = 263,
     ELSE = 264,
     FI = 265,
     WHILE = 266,
     DO = 267,
     OD = 268,
     FOR = 269,
     IN = 270,
     EQ = 271,
     NEQ = 272,
     LE = 273,
     GE = 274,
     AND = 275,
     OR = 276,
     NOT = 277,
     UMINUS = 278,
     TWODOTS = 279
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
#define FI 265
#define WHILE 266
#define DO 267
#define OD 268
#define FOR 269
#define IN 270
#define EQ 271
#define NEQ 272
#define LE 273
#define GE 274
#define AND 275
#define OR 276
#define NOT 277
#define UMINUS 278
#define TWODOTS 279




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 25 "/home/muntyan/projects/moo/moo/mooutils/mooscript/mooscript-yacc.y"
typedef union YYSTYPE {
    int ival;
    const char *str;
    MSNode *node;
} YYSTYPE;
/* Line 1447 of yacc.c.  */
#line 92 "/home/muntyan/projects/moo/moo/mooutils/mooscript/mooscript-yacc.h"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE _ms_script_yylval;



