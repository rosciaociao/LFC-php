/* A Bison parser, made by GNU Bison 2.5.  */

/* Bison interface for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2011 Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

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


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     T_LOGICAL_OR = 258,
     T_LOGICAL_AND = 259,
     T_PRINT = 260,
     T_MOD_EQUAL = 261,
     T_CONCAT_EQUAL = 262,
     T_DIV_EQUAL = 263,
     T_MUL_EQUAL = 264,
     T_MINUS_EQUAL = 265,
     T_PLUS_EQUAL = 266,
     T_BOOLEAN_OR = 267,
     T_BOOLEAN_AND = 268,
     T_IS_NOT_EQUAL = 269,
     T_IS_EQUAL = 270,
     T_IS_GREATER_OR_EQUAL = 271,
     T_IS_SMALLER_OR_EQUAL = 272,
     T_DEC = 273,
     T_INC = 274,
     T_IF = 275,
     T_ELSEIF = 276,
     T_ELSE = 277,
     T_ENDIF = 278,
     T_LNUMBER = 279,
     T_DNUMBER = 280,
     T_STRING = 281,
     T_VARIABLE = 282,
     T_CONSTANT = 283,
     T_NUM_STRING = 284,
     T_ENCAPSED_AND_WHITESPACE = 285,
     T_CONSTANT_ENCAPSED_STRING = 286,
     T_CHARACTER = 287,
     T_ECHO = 288,
     T_DO = 289,
     T_WHILE = 290,
     T_FOR = 291,
     T_SWITCH = 292,
     T_CASE = 293,
     T_DEFAULT = 294,
     T_BREAK = 295,
     T_CONTINUE = 296,
     T_ARRAY = 297,
     T_DEFINE = 298,
     T_WHITESPACE = 299,
     T_INIT = 300,
     T_FINAL = 301
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 2068 of yacc.c  */
#line 105 "php_parser.y"

 char *id;
 


/* Line 2068 of yacc.c  */
#line 102 "php_parser.tab.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;

#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} YYLTYPE;
# define yyltype YYLTYPE /* obsolescent; will be withdrawn */
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif

extern YYLTYPE yylloc;

