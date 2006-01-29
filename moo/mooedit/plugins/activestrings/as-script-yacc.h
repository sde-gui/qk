/* A Bison parser, made by GNU Bison 2.0.  */

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




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 24 "/home/muntyan/Projects/moo-svn/moo/mooedit/plugins/activestrings/as-script-yacc.y"
typedef union YYSTYPE {
    int ival;
    const char *str;
    ASNode *node;
} YYSTYPE;
/* Line 1318 of yacc.c.  */
#line 83 "/home/muntyan/Projects/moo-svn/moo/mooedit/plugins/activestrings/as-script-yacc.h"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE _as_script_yylval;



