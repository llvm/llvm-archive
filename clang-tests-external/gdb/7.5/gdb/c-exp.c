/* A Bison parser, made by GNU Bison 1.875c.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003 Free Software Foundation, Inc.

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



/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     INT = 258,
     FLOAT = 259,
     DECFLOAT = 260,
     STRING = 261,
     CHAR = 262,
     NAME = 263,
     UNKNOWN_CPP_NAME = 264,
     COMPLETE = 265,
     TYPENAME = 266,
     NAME_OR_INT = 267,
     OPERATOR = 268,
     STRUCT = 269,
     CLASS = 270,
     UNION = 271,
     ENUM = 272,
     SIZEOF = 273,
     UNSIGNED = 274,
     COLONCOLON = 275,
     TEMPLATE = 276,
     ERROR = 277,
     NEW = 278,
     DELETE = 279,
     REINTERPRET_CAST = 280,
     DYNAMIC_CAST = 281,
     STATIC_CAST = 282,
     CONST_CAST = 283,
     ENTRY = 284,
     SIGNED_KEYWORD = 285,
     LONG = 286,
     SHORT = 287,
     INT_KEYWORD = 288,
     CONST_KEYWORD = 289,
     VOLATILE_KEYWORD = 290,
     DOUBLE_KEYWORD = 291,
     VARIABLE = 292,
     ASSIGN_MODIFY = 293,
     TRUEKEYWORD = 294,
     FALSEKEYWORD = 295,
     ABOVE_COMMA = 296,
     OROR = 297,
     ANDAND = 298,
     NOTEQUAL = 299,
     EQUAL = 300,
     GEQ = 301,
     LEQ = 302,
     RSH = 303,
     LSH = 304,
     DECREMENT = 305,
     INCREMENT = 306,
     UNARY = 307,
     DOT_STAR = 308,
     ARROW_STAR = 309,
     ARROW = 310,
     BLOCKNAME = 311,
     FILENAME = 312,
     DOTDOTDOT = 313
   };
#endif
#define INT 258
#define FLOAT 259
#define DECFLOAT 260
#define STRING 261
#define CHAR 262
#define NAME 263
#define UNKNOWN_CPP_NAME 264
#define COMPLETE 265
#define TYPENAME 266
#define NAME_OR_INT 267
#define OPERATOR 268
#define STRUCT 269
#define CLASS 270
#define UNION 271
#define ENUM 272
#define SIZEOF 273
#define UNSIGNED 274
#define COLONCOLON 275
#define TEMPLATE 276
#define ERROR 277
#define NEW 278
#define DELETE 279
#define REINTERPRET_CAST 280
#define DYNAMIC_CAST 281
#define STATIC_CAST 282
#define CONST_CAST 283
#define ENTRY 284
#define SIGNED_KEYWORD 285
#define LONG 286
#define SHORT 287
#define INT_KEYWORD 288
#define CONST_KEYWORD 289
#define VOLATILE_KEYWORD 290
#define DOUBLE_KEYWORD 291
#define VARIABLE 292
#define ASSIGN_MODIFY 293
#define TRUEKEYWORD 294
#define FALSEKEYWORD 295
#define ABOVE_COMMA 296
#define OROR 297
#define ANDAND 298
#define NOTEQUAL 299
#define EQUAL 300
#define GEQ 301
#define LEQ 302
#define RSH 303
#define LSH 304
#define DECREMENT 305
#define INCREMENT 306
#define UNARY 307
#define DOT_STAR 308
#define ARROW_STAR 309
#define ARROW 310
#define BLOCKNAME 311
#define FILENAME 312
#define DOTDOTDOT 313




/* Copy the first part of user declarations.  */
#line 37 "c-exp.y"


#include "defs.h"
#include "gdb_string.h"
#include <ctype.h>
#include "expression.h"
#include "value.h"
#include "parser-defs.h"
#include "language.h"
#include "c-lang.h"
#include "bfd.h" /* Required by objfiles.h.  */
#include "symfile.h" /* Required by objfiles.h.  */
#include "objfiles.h" /* For have_full_symbols and have_partial_symbols */
#include "charset.h"
#include "block.h"
#include "cp-support.h"
#include "dfp.h"
#include "gdb_assert.h"
#include "macroscope.h"

#define parse_type builtin_type (parse_gdbarch)

/* Remap normal yacc parser interface names (yyparse, yylex, yyerror, etc),
   as well as gratuitiously global symbol names, so we can have multiple
   yacc generated parsers in gdb.  Note that these are only the variables
   produced by yacc.  If other parser generators (bison, byacc, etc) produce
   additional global names that conflict at link time, then those parser
   generators need to be fixed instead of adding those names to this list. */

#define	yymaxdepth c_maxdepth
#define	yyparse	c_parse_internal
#define	yylex	c_lex
#define	yyerror	c_error
#define	yylval	c_lval
#define	yychar	c_char
#define	yydebug	c_debug
#define	yypact	c_pact	
#define	yyr1	c_r1			
#define	yyr2	c_r2			
#define	yydef	c_def		
#define	yychk	c_chk		
#define	yypgo	c_pgo		
#define	yyact	c_act		
#define	yyexca	c_exca
#define yyerrflag c_errflag
#define yynerrs	c_nerrs
#define	yyps	c_ps
#define	yypv	c_pv
#define	yys	c_s
#define	yy_yys	c_yys
#define	yystate	c_state
#define	yytmp	c_tmp
#define	yyv	c_v
#define	yy_yyv	c_yyv
#define	yyval	c_val
#define	yylloc	c_lloc
#define yyreds	c_reds		/* With YYDEBUG defined */
#define yytoks	c_toks		/* With YYDEBUG defined */
#define yyname	c_name		/* With YYDEBUG defined */
#define yyrule	c_rule		/* With YYDEBUG defined */
#define yylhs	c_yylhs
#define yylen	c_yylen
#define yydefred c_yydefred
#define yydgoto	c_yydgoto
#define yysindex c_yysindex
#define yyrindex c_yyrindex
#define yygindex c_yygindex
#define yytable	 c_yytable
#define yycheck	 c_yycheck
#define yyss	c_yyss
#define yysslim	c_yysslim
#define yyssp	c_yyssp
#define yystacksize c_yystacksize
#define yyvs	c_yyvs
#define yyvsp	c_yyvsp

#ifndef YYDEBUG
#define	YYDEBUG 1		/* Default to yydebug support */
#endif

#define YYFPRINTF parser_fprintf

int yyparse (void);

static int yylex (void);

void yyerror (char *);



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
#line 132 "c-exp.y"
typedef union YYSTYPE {
    LONGEST lval;
    struct {
      LONGEST val;
      struct type *type;
    } typed_val_int;
    struct {
      DOUBLEST dval;
      struct type *type;
    } typed_val_float;
    struct {
      gdb_byte val[16];
      struct type *type;
    } typed_val_decfloat;
    struct symbol *sym;
    struct type *tval;
    struct stoken sval;
    struct typed_stoken tsval;
    struct ttype tsym;
    struct symtoken ssym;
    int voidval;
    struct block *bval;
    enum exp_opcode opcode;
    struct internalvar *ivar;

    struct stoken_vector svec;
    VEC (type_ptr) *tvec;
    int *ivec;

    struct type_stack *type_stack;
  } YYSTYPE;
/* Line 191 of yacc.c.  */
#line 314 "c-exp.c"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */
#line 164 "c-exp.y"

/* YYSTYPE gets defined by %union */
static int parse_number (char *, int, int, YYSTYPE *);
static struct stoken operator_stoken (const char *);
static void check_parameter_typelist (VEC (type_ptr) *);


/* Line 214 of yacc.c.  */
#line 332 "c-exp.c"

#if ! defined (yyoverflow) || YYERROR_VERBOSE

# ifndef YYFREE
#  define YYFREE xfree
# endif
# ifndef YYMALLOC
#  define YYMALLOC xmalloc
# endif

/* The parser invokes alloca or xmalloc; define the necessary symbols.  */

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
  short yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE))				\
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
   typedef short yysigned_char;
#endif

/* YYFINAL -- State number of the termination state. */
#define YYFINAL  149
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1237

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  83
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  40
/* YYNRULES -- Number of rules. */
#define YYNRULES  222
/* YYNRULES -- Number of states. */
#define YYNSTATES  341

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   313

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    76,     2,     2,     2,    63,    49,     2,
      69,    79,    61,    59,    41,    60,    67,    62,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    82,     2,
      52,    43,    53,    44,    58,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    68,     2,    78,    48,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    80,    47,    81,    77,     2,     2,     2,
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
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    42,    45,    46,    50,
      51,    54,    55,    56,    57,    64,    65,    66,    70,    71,
      72,    73,    74,    75
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short yyprhs[] =
{
       0,     0,     3,     5,     7,     9,    11,    15,    18,    21,
      24,    27,    30,    33,    36,    39,    42,    45,    48,    52,
      57,    61,    65,    69,    73,    78,    82,    86,    90,    95,
      96,   102,   103,   109,   111,   112,   114,   118,   124,   126,
     130,   135,   140,   144,   148,   152,   156,   160,   164,   168,
     172,   176,   180,   184,   188,   192,   196,   200,   204,   208,
     212,   216,   220,   226,   230,   234,   236,   238,   240,   242,
     244,   246,   248,   253,   261,   269,   277,   285,   287,   290,
     292,   294,   296,   298,   300,   304,   307,   311,   315,   320,
     326,   328,   331,   333,   336,   338,   339,   343,   345,   347,
     349,   350,   351,   356,   357,   361,   363,   366,   368,   371,
     373,   375,   379,   382,   384,   387,   389,   392,   396,   399,
     403,   405,   407,   409,   411,   413,   416,   420,   423,   427,
     431,   435,   438,   441,   445,   450,   454,   458,   463,   467,
     472,   476,   481,   484,   488,   491,   495,   498,   502,   504,
     507,   510,   513,   516,   519,   522,   524,   527,   529,   535,
     538,   541,   543,   545,   547,   549,   551,   555,   557,   561,
     563,   566,   569,   570,   573,   576,   579,   581,   583,   585,
     588,   591,   596,   601,   604,   607,   610,   613,   616,   619,
     622,   625,   628,   631,   634,   637,   640,   643,   646,   649,
     652,   655,   658,   661,   664,   667,   670,   673,   676,   679,
     682,   686,   690,   693,   695,   697,   699,   701,   703,   705,
     707,   709,   711
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const yysigned_char yyrhs[] =
{
      84,     0,    -1,    86,    -1,    85,    -1,   110,    -1,    87,
      -1,    86,    41,    87,    -1,    61,    87,    -1,    49,    87,
      -1,    60,    87,    -1,    59,    87,    -1,    76,    87,    -1,
      77,    87,    -1,    65,    87,    -1,    64,    87,    -1,    87,
      65,    -1,    87,    64,    -1,    18,    87,    -1,    87,    72,
     121,    -1,    87,    72,   121,    10,    -1,    87,    72,    10,
      -1,    87,    72,    96,    -1,    87,    71,    87,    -1,    87,
      67,   121,    -1,    87,    67,   121,    10,    -1,    87,    67,
      10,    -1,    87,    67,    96,    -1,    87,    70,    87,    -1,
      87,    68,    86,    78,    -1,    -1,    87,    69,    88,    91,
      79,    -1,    -1,     9,    69,    89,    91,    79,    -1,    80,
      -1,    -1,    87,    -1,    91,    41,    87,    -1,    87,    69,
     113,    79,    98,    -1,    81,    -1,    90,    91,    92,    -1,
      90,   110,    92,    87,    -1,    69,   110,    79,    87,    -1,
      69,    86,    79,    -1,    87,    58,    87,    -1,    87,    61,
      87,    -1,    87,    62,    87,    -1,    87,    63,    87,    -1,
      87,    59,    87,    -1,    87,    60,    87,    -1,    87,    57,
      87,    -1,    87,    56,    87,    -1,    87,    51,    87,    -1,
      87,    50,    87,    -1,    87,    55,    87,    -1,    87,    54,
      87,    -1,    87,    52,    87,    -1,    87,    53,    87,    -1,
      87,    49,    87,    -1,    87,    48,    87,    -1,    87,    47,
      87,    -1,    87,    46,    87,    -1,    87,    45,    87,    -1,
      87,    44,    87,    82,    87,    -1,    87,    43,    87,    -1,
      87,    38,    87,    -1,     3,    -1,     7,    -1,    12,    -1,
       4,    -1,     5,    -1,    95,    -1,    37,    -1,    18,    69,
     110,    79,    -1,    25,    52,   110,    53,    69,    87,    79,
      -1,    27,    52,   110,    53,    69,    87,    79,    -1,    26,
      52,   110,    53,    69,    87,    79,    -1,    28,    52,   110,
      53,    69,    87,    79,    -1,     6,    -1,    93,     6,    -1,
      93,    -1,    39,    -1,    40,    -1,    73,    -1,    74,    -1,
      94,    20,   121,    -1,   122,    29,    -1,    94,    20,   121,
      -1,    11,    20,   121,    -1,    11,    20,    77,   121,    -1,
      11,    20,   121,    20,   121,    -1,    96,    -1,    20,   122,
      -1,   122,    -1,    58,     8,    -1,   119,    -1,    -1,    98,
      97,    98,    -1,    99,    -1,   119,    -1,   100,    -1,    -1,
      -1,   102,    61,   103,   101,    -1,    -1,    61,   104,   101,
      -1,    49,    -1,    49,   102,    -1,   102,    -1,   105,   107,
      -1,   105,    -1,   107,    -1,    69,   106,    79,    -1,   107,
     108,    -1,   108,    -1,   107,   109,    -1,   109,    -1,    68,
      78,    -1,    68,     3,    78,    -1,    69,    79,    -1,    69,
     113,    79,    -1,   115,    -1,    11,    -1,    33,    -1,    31,
      -1,    32,    -1,    31,    33,    -1,    31,    30,    33,    -1,
      31,    30,    -1,    30,    31,    33,    -1,    19,    31,    33,
      -1,    31,    19,    33,    -1,    31,    19,    -1,    31,    31,
      -1,    31,    31,    33,    -1,    31,    31,    30,    33,    -1,
      31,    31,    30,    -1,    30,    31,    31,    -1,    30,    31,
      31,    33,    -1,    19,    31,    31,    -1,    19,    31,    31,
      33,    -1,    31,    31,    19,    -1,    31,    31,    19,    33,
      -1,    32,    33,    -1,    32,    30,    33,    -1,    32,    30,
      -1,    19,    32,    33,    -1,    32,    19,    -1,    32,    19,
      33,    -1,    36,    -1,    31,    36,    -1,    14,   121,    -1,
      15,   121,    -1,    16,   121,    -1,    17,   121,    -1,    19,
     112,    -1,    19,    -1,    30,   112,    -1,    30,    -1,    21,
     121,    52,   110,    53,    -1,   100,   111,    -1,   111,   100,
      -1,    11,    -1,    33,    -1,    31,    -1,    32,    -1,   114,
      -1,   114,    41,    75,    -1,   110,    -1,   114,    41,   110,
      -1,   111,    -1,   115,   106,    -1,   111,   117,    -1,    -1,
     102,   117,    -1,    34,    35,    -1,    35,    34,    -1,   118,
      -1,    34,    -1,    35,    -1,    13,    23,    -1,    13,    24,
      -1,    13,    23,    68,    78,    -1,    13,    24,    68,    78,
      -1,    13,    59,    -1,    13,    60,    -1,    13,    61,    -1,
      13,    62,    -1,    13,    63,    -1,    13,    48,    -1,    13,
      49,    -1,    13,    47,    -1,    13,    77,    -1,    13,    76,
      -1,    13,    43,    -1,    13,    52,    -1,    13,    53,    -1,
      13,    38,    -1,    13,    57,    -1,    13,    56,    -1,    13,
      51,    -1,    13,    50,    -1,    13,    55,    -1,    13,    54,
      -1,    13,    46,    -1,    13,    45,    -1,    13,    65,    -1,
      13,    64,    -1,    13,    41,    -1,    13,    71,    -1,    13,
      72,    -1,    13,    69,    79,    -1,    13,    68,    78,    -1,
      13,   116,    -1,     8,    -1,    73,    -1,    11,    -1,    12,
      -1,     9,    -1,   120,    -1,     8,    -1,    73,    -1,   120,
      -1,     9,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   263,   263,   264,   267,   274,   275,   280,   284,   288,
     292,   296,   300,   304,   308,   312,   316,   320,   324,   330,
     337,   347,   355,   359,   365,   372,   382,   390,   394,   401,
     398,   409,   408,   431,   435,   438,   442,   446,   464,   467,
     474,   480,   486,   492,   496,   500,   504,   508,   512,   516,
     520,   524,   528,   532,   536,   540,   544,   548,   552,   556,
     560,   564,   568,   572,   576,   582,   589,   598,   609,   616,
     623,   626,   632,   642,   648,   654,   660,   669,   686,   704,
     738,   745,   754,   762,   768,   778,   793,   815,   830,   854,
     863,   864,   892,   946,   950,   951,   954,   957,   958,   962,
     963,   968,   967,   971,   970,   973,   975,   979,   988,   990,
     991,   994,   996,  1003,  1010,  1016,  1023,  1025,  1029,  1031,
    1043,  1047,  1049,  1053,  1057,  1061,  1065,  1069,  1073,  1077,
    1081,  1085,  1089,  1093,  1097,  1101,  1105,  1109,  1113,  1117,
    1121,  1125,  1129,  1133,  1137,  1141,  1145,  1149,  1153,  1157,
    1161,  1164,  1167,  1170,  1173,  1177,  1181,  1185,  1192,  1196,
    1198,  1202,  1203,  1211,  1219,  1230,  1232,  1241,  1247,  1254,
    1255,  1262,  1266,  1267,  1270,  1271,  1274,  1278,  1280,  1284,
    1286,  1288,  1290,  1292,  1294,  1296,  1298,  1300,  1302,  1304,
    1306,  1308,  1310,  1312,  1314,  1316,  1318,  1358,  1360,  1362,
    1364,  1366,  1368,  1370,  1372,  1374,  1376,  1378,  1380,  1382,
    1384,  1386,  1388,  1403,  1404,  1405,  1406,  1407,  1408,  1411,
    1412,  1420,  1428
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "INT", "FLOAT", "DECFLOAT", "STRING",
  "CHAR", "NAME", "UNKNOWN_CPP_NAME", "COMPLETE", "TYPENAME",
  "NAME_OR_INT", "OPERATOR", "STRUCT", "CLASS", "UNION", "ENUM", "SIZEOF",
  "UNSIGNED", "COLONCOLON", "TEMPLATE", "ERROR", "NEW", "DELETE",
  "REINTERPRET_CAST", "DYNAMIC_CAST", "STATIC_CAST", "CONST_CAST", "ENTRY",
  "SIGNED_KEYWORD", "LONG", "SHORT", "INT_KEYWORD", "CONST_KEYWORD",
  "VOLATILE_KEYWORD", "DOUBLE_KEYWORD", "VARIABLE", "ASSIGN_MODIFY",
  "TRUEKEYWORD", "FALSEKEYWORD", "','", "ABOVE_COMMA", "'='", "'?'",
  "OROR", "ANDAND", "'|'", "'^'", "'&'", "NOTEQUAL", "EQUAL", "'<'", "'>'",
  "GEQ", "LEQ", "RSH", "LSH", "'@'", "'+'", "'-'", "'*'", "'/'", "'%'",
  "DECREMENT", "INCREMENT", "UNARY", "'.'", "'['", "'('", "DOT_STAR",
  "ARROW_STAR", "ARROW", "BLOCKNAME", "FILENAME", "DOTDOTDOT", "'!'",
  "'~'", "']'", "')'", "'{'", "'}'", "':'", "$accept", "start", "type_exp",
  "exp1", "exp", "@1", "@2", "lcurly", "arglist", "rcurly", "string_exp",
  "block", "variable", "qualified_name", "space_identifier",
  "const_or_volatile", "cv_with_space_id",
  "const_or_volatile_or_space_identifier_noopt",
  "const_or_volatile_or_space_identifier", "ptr_operator", "@3", "@4",
  "ptr_operator_ts", "abs_decl", "direct_abs_decl", "array_mod",
  "func_mod", "type", "typebase", "typename", "parameter_typelist",
  "nonempty_typelist", "ptype", "conversion_type_id",
  "conversion_declarator", "const_and_volatile", "const_or_volatile_noopt",
  "operator", "name", "name_not_typename", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,    44,   296,    61,    63,   297,   298,   124,    94,    38,
     299,   300,    60,    62,   301,   302,   303,   304,    64,    43,
      45,    42,    47,    37,   305,   306,   307,    46,    91,    40,
     308,   309,   310,   311,   312,   313,    33,   126,    93,    41,
     123,   125,    58
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    83,    84,    84,    85,    86,    86,    87,    87,    87,
      87,    87,    87,    87,    87,    87,    87,    87,    87,    87,
      87,    87,    87,    87,    87,    87,    87,    87,    87,    88,
      87,    89,    87,    90,    91,    91,    91,    87,    92,    87,
      87,    87,    87,    87,    87,    87,    87,    87,    87,    87,
      87,    87,    87,    87,    87,    87,    87,    87,    87,    87,
      87,    87,    87,    87,    87,    87,    87,    87,    87,    87,
      87,    87,    87,    87,    87,    87,    87,    93,    93,    87,
      87,    87,    94,    94,    94,    95,    95,    96,    96,    96,
      95,    95,    95,    97,    98,    98,    99,   100,   100,   101,
     101,   103,   102,   104,   102,   102,   102,   105,   106,   106,
     106,   107,   107,   107,   107,   107,   108,   108,   109,   109,
     110,   111,   111,   111,   111,   111,   111,   111,   111,   111,
     111,   111,   111,   111,   111,   111,   111,   111,   111,   111,
     111,   111,   111,   111,   111,   111,   111,   111,   111,   111,
     111,   111,   111,   111,   111,   111,   111,   111,   111,   111,
     111,   112,   112,   112,   112,   113,   113,   114,   114,   115,
     115,   116,   117,   117,   118,   118,   119,   119,   119,   120,
     120,   120,   120,   120,   120,   120,   120,   120,   120,   120,
     120,   120,   120,   120,   120,   120,   120,   120,   120,   120,
     120,   120,   120,   120,   120,   120,   120,   120,   120,   120,
     120,   120,   120,   121,   121,   121,   121,   121,   121,   122,
     122,   122,   122
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     1,     1,     1,     3,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     3,     4,
       3,     3,     3,     3,     4,     3,     3,     3,     4,     0,
       5,     0,     5,     1,     0,     1,     3,     5,     1,     3,
       4,     4,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     5,     3,     3,     1,     1,     1,     1,     1,
       1,     1,     4,     7,     7,     7,     7,     1,     2,     1,
       1,     1,     1,     1,     3,     2,     3,     3,     4,     5,
       1,     2,     1,     2,     1,     0,     3,     1,     1,     1,
       0,     0,     4,     0,     3,     1,     2,     1,     2,     1,
       1,     3,     2,     1,     2,     1,     2,     3,     2,     3,
       1,     1,     1,     1,     1,     2,     3,     2,     3,     3,
       3,     2,     2,     3,     4,     3,     3,     4,     3,     4,
       3,     4,     2,     3,     2,     3,     2,     3,     1,     2,
       2,     2,     2,     2,     2,     1,     2,     1,     5,     2,
       2,     1,     1,     1,     1,     1,     3,     1,     3,     1,
       2,     2,     0,     2,     2,     2,     1,     1,     1,     2,
       2,     4,     4,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       3,     3,     2,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
      95,    65,    68,    69,    77,    66,   219,   222,   121,    67,
      95,     0,     0,     0,     0,     0,   155,     0,     0,     0,
       0,     0,     0,   157,   123,   124,   122,   177,   178,   148,
      71,    80,    81,     0,     0,     0,     0,     0,     0,    95,
     220,    83,     0,     0,    33,     0,     3,     2,     5,    34,
      79,     0,    70,    90,     0,    97,    95,     4,   169,   120,
     176,    98,   221,    92,    31,     0,   121,   179,   180,   196,
     207,   193,   204,   203,   190,   188,   189,   200,   199,   194,
     195,   202,   201,   198,   197,   183,   184,   185,   186,   187,
     206,   205,     0,     0,   208,   209,   192,   191,   172,   212,
     213,   217,   215,   216,   214,   218,   150,   151,   152,   153,
       0,    95,    17,   161,   163,   164,   162,   154,   222,   220,
      91,     0,    95,    95,    95,    95,   163,   164,   156,   131,
     127,   132,   125,   149,   146,   144,   142,   174,   175,     8,
      10,     9,     7,    14,    13,     0,     0,    11,    12,     1,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    16,    15,     0,     0,    29,     0,     0,
       0,    35,     0,     0,    78,     0,     0,    95,   159,   160,
     105,   103,     0,    95,   107,   109,   170,   110,   113,   115,
      85,    34,     0,    87,     0,     0,   211,   210,   172,   171,
       0,   138,   129,   145,    95,     0,     0,     0,     0,   136,
     128,   130,   126,   140,   135,   133,   147,   143,    42,     0,
       6,    64,    63,     0,    61,    60,    59,    58,    57,    52,
      51,    55,    56,    54,    53,    50,    49,    43,    47,    48,
      44,    45,    46,    25,   215,    26,    23,     0,    34,   167,
       0,   165,    27,    22,    20,    21,    18,     0,    38,    39,
       0,    86,    93,    96,    94,   106,   100,     0,   116,   118,
       0,     0,   101,   108,    95,   112,   114,     0,    88,     0,
     181,   182,   101,   173,    72,   139,     0,     0,     0,     0,
       0,   137,   141,   134,    41,     0,    24,    28,     0,    95,
      95,    19,    36,    40,    99,   104,   117,   111,   119,   100,
      32,    89,   158,     0,     0,     0,     0,    62,    30,    37,
     166,   168,   102,     0,     0,     0,     0,    73,    75,    74,
      76
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    45,    46,   145,    48,   258,   201,    49,   182,   269,
      50,    51,    52,    53,   187,    54,    55,    56,   315,   194,
     319,   276,   195,   196,   197,   198,   199,   259,    58,   117,
     281,   261,    59,    99,   209,    60,    61,    62,   106,    63
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -187
static const short yypact[] =
{
     388,  -187,  -187,  -187,  -187,  -187,  -187,   -43,    32,  -187,
     670,    56,    56,    56,    56,   197,    52,    16,    56,   -19,
     -16,    10,    19,   100,    84,    88,  -187,    45,    47,  -187,
    -187,  -187,  -187,   466,   466,   466,   466,   466,   466,   388,
      72,  -187,   466,   466,  -187,    87,  -187,    65,   964,   310,
     103,    92,  -187,  -187,    61,  -187,  1108,  -187,    24,   113,
    -187,    64,  -187,   101,  -187,    31,  -187,    93,    97,  -187,
    -187,  -187,  -187,  -187,  -187,  -187,  -187,  -187,  -187,  -187,
    -187,  -187,  -187,  -187,  -187,  -187,  -187,  -187,  -187,  -187,
    -187,  -187,    91,    94,  -187,  -187,  -187,  -187,    44,  -187,
    -187,  -187,  -187,  -187,  -187,  -187,  -187,  -187,  -187,  -187,
      32,   388,   449,  -187,    55,   138,  -187,  -187,  -187,  -187,
    -187,   127,  1108,  1108,  1108,  1108,   135,  -187,  -187,   152,
     155,   159,  -187,  -187,   162,   163,  -187,  -187,  -187,   449,
     449,   449,   449,   449,   449,    -6,   114,   449,   449,  -187,
     466,   466,   466,   466,   466,   466,   466,   466,   466,   466,
     466,   466,   466,   466,   466,   466,   466,   466,   466,   466,
     466,   466,   466,  -187,  -187,    37,   466,   760,   466,   466,
      86,   964,    -9,   117,  -187,    56,   191,    20,    24,  -187,
     146,  -187,    13,   611,   150,   115,  -187,   122,  -187,  -187,
    -187,   466,    56,   192,   140,   141,  -187,  -187,    67,  -187,
     134,   187,  -187,  -187,  1108,   168,   173,   180,   182,   205,
    -187,  -187,  -187,   206,   207,  -187,  -187,  -187,  -187,   466,
     964,   964,   964,   776,  1020,  1046,  1097,  1121,  1144,  1165,
    1165,   606,   606,   606,   606,   234,   234,   293,   371,   371,
     449,   449,   449,  -187,    32,  -187,   231,    12,   466,  -187,
     165,   201,   160,   160,  -187,  -187,   235,   466,  -187,  -187,
     466,   227,  -187,  -187,  -187,   150,    24,   171,  -187,  -187,
     172,   174,  -187,   122,   571,  -187,  -187,    -4,  -187,    56,
    -187,  -187,  -187,  -187,   544,  -187,   210,   181,   185,   190,
     195,  -187,  -187,  -187,   449,   466,  -187,  -187,    -3,    20,
     734,  -187,   964,   449,  -187,  -187,  -187,  -187,  -187,    24,
    -187,  -187,  -187,   466,   466,   466,   466,   993,  -187,  -187,
    -187,  -187,  -187,   816,   853,   890,   927,  -187,  -187,  -187,
    -187
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -187,  -187,  -187,    11,   -15,  -187,  -187,  -187,  -184,    77,
    -187,  -187,  -187,  -119,  -187,  -186,  -187,   -28,   -54,   -32,
    -187,  -187,  -187,    74,    73,  -183,  -182,     2,    21,   246,
      95,  -187,  -187,  -187,    68,  -187,  -175,    -8,    -5,   258
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -96
static const short yytable[] =
{
     112,   273,    57,   105,   105,   105,   105,   107,   108,   109,
     105,    47,   274,   121,   285,   286,   277,   287,   139,   140,
     141,   142,   143,   144,     6,   118,    64,   147,   148,    10,
     189,    98,   267,   122,   181,   150,   123,   267,   267,   100,
     101,   146,   102,   103,    10,   100,   101,   253,   254,   103,
      10,   183,    65,   150,    27,    28,   255,   105,    27,    28,
     203,   265,   124,   113,   100,   101,   208,   102,   103,    10,
     189,   125,   268,   228,   308,   320,   328,   188,    27,    28,
     137,   138,   -95,   114,   115,   116,   211,   149,   212,   119,
     307,   278,   -82,   190,   100,   101,   264,   254,   103,    10,
     285,   286,   -95,   129,   104,   191,   150,   134,   202,   184,
     104,   113,   185,   210,   130,   131,   190,   132,   135,   186,
     133,   136,   -94,   329,   215,   216,   217,   218,   292,   104,
     200,   126,   127,   116,   274,   230,   231,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   246,   247,   248,   249,   250,   251,   252,   275,   104,
     189,   204,   190,   262,   263,   205,   219,   105,   220,   206,
     256,   213,   105,   207,   191,   266,   208,   105,   223,   214,
     271,   192,   193,   192,   193,   221,   181,   257,   222,   224,
     192,   284,   225,   229,   105,   226,   227,   288,   268,   272,
       1,     2,     3,     4,     5,     6,     7,   191,   110,     9,
      10,   282,   289,   294,   304,    15,   296,    17,   290,   291,
     295,   297,    19,    20,    21,    22,   298,   175,   176,   177,
     178,   179,   180,   299,    30,   300,    31,    32,   301,   302,
     303,   306,   310,   181,   309,   311,    33,   -84,   314,   316,
     323,   317,   312,   318,   324,   313,    34,    35,    36,   325,
     270,    37,    38,   322,   326,   332,   111,   280,   283,   128,
      40,    41,   260,    42,    43,   120,   293,    44,     0,   304,
       0,   105,     0,     0,   321,     0,     0,     0,     0,     0,
     327,   314,   167,   168,   169,   170,   171,   172,   173,   174,
       0,   175,   176,   177,   178,   179,   180,     0,   333,   334,
     335,   336,   331,     1,     2,     3,     4,     5,     6,     7,
       0,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,     0,     0,     0,    19,    20,    21,    22,     0,
      23,    24,    25,    26,    27,    28,    29,    30,     0,    31,
      32,     0,   168,   169,   170,   171,   172,   173,   174,    33,
     175,   176,   177,   178,   179,   180,     0,     0,   -95,    34,
      35,    36,     0,     0,    37,    38,     0,     0,     0,    39,
       0,     0,     0,    40,    41,     0,    42,    43,     0,     0,
      44,     1,     2,     3,     4,     5,     6,     7,     0,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
       0,     0,     0,    19,    20,    21,    22,     0,    23,    24,
      25,    26,    27,    28,    29,    30,     0,    31,    32,     0,
       0,     0,   170,   171,   172,   173,   174,    33,   175,   176,
     177,   178,   179,   180,     0,     0,     0,    34,    35,    36,
       0,     0,    37,    38,     0,     0,     0,    39,     0,     0,
       0,    40,    41,     0,    42,    43,     0,     0,    44,     1,
       2,     3,     4,     5,     6,     7,     0,   110,     9,    10,
       0,     0,     0,     0,    15,     0,    17,     0,     0,     0,
       0,    19,    20,    21,    22,     0,     0,     0,     0,     0,
       0,     0,     0,    30,     0,    31,    32,     0,     0,     0,
       0,     0,     0,   173,   174,    33,   175,   176,   177,   178,
     179,   180,     0,     0,     0,    34,    35,    36,     0,     0,
      37,    38,     0,     0,     0,    39,     0,     0,     0,    40,
      41,     0,    42,    43,     0,     0,    44,     1,     2,     3,
       4,     5,     6,     7,     0,   110,     9,    10,     0,     0,
       0,     0,    15,     0,    17,     0,     0,     0,     0,    19,
      20,    21,    22,     0,     0,     0,     0,     0,     0,     0,
       0,    30,    66,    31,    32,    11,    12,    13,    14,     0,
      16,     0,    18,     0,     0,     0,     0,     0,     0,     0,
       0,    23,    24,    25,    26,    27,    28,    29,    37,    38,
       0,     0,     0,    39,     0,     0,     0,    40,    41,     0,
      42,    43,    66,     0,    44,    11,    12,    13,    14,     0,
      16,     0,    18,     0,     0,     0,     0,     0,     0,     0,
       0,    23,    24,    25,    26,    27,    28,    29,     0,     0,
     279,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     190,     0,   165,   166,   167,   168,   169,   170,   171,   172,
     173,   174,   191,   175,   176,   177,   178,   179,   180,   192,
     193,    66,     0,     0,    11,    12,    13,    14,     0,    16,
     279,    18,     0,    67,    68,     0,     0,     0,     0,     0,
      23,    24,    25,    26,    27,    28,    29,     0,    69,     0,
       0,    70,     0,    71,     0,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,     0,    85,
      86,    87,    88,    89,    90,    91,     0,     0,    92,    93,
       0,    94,    95,     0,     0,    66,    96,    97,    11,    12,
      13,    14,     0,    16,     0,    18,     0,     0,     0,     0,
       0,     0,     0,     0,    23,    24,    25,    26,    27,    28,
      29,    66,     0,     0,    11,    12,    13,    14,     0,    16,
       0,    18,     0,     0,     0,     0,     0,     0,     0,     0,
      23,    24,    25,    26,    27,    28,    29,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   330,
       0,     0,     0,     0,   151,     0,     0,     0,   -95,   152,
     153,   154,   155,   156,   157,   158,   159,   160,   161,   162,
     163,   164,   165,   166,   167,   168,   169,   170,   171,   172,
     173,   174,     0,   175,   176,   177,   178,   179,   180,     0,
       0,     0,     0,     0,   151,     0,     0,     0,   305,   152,
     153,   154,   155,   156,   157,   158,   159,   160,   161,   162,
     163,   164,   165,   166,   167,   168,   169,   170,   171,   172,
     173,   174,     0,   175,   176,   177,   178,   179,   180,     0,
       0,   151,     0,     0,     0,   337,   152,   153,   154,   155,
     156,   157,   158,   159,   160,   161,   162,   163,   164,   165,
     166,   167,   168,   169,   170,   171,   172,   173,   174,     0,
     175,   176,   177,   178,   179,   180,     0,     0,   151,     0,
       0,     0,   338,   152,   153,   154,   155,   156,   157,   158,
     159,   160,   161,   162,   163,   164,   165,   166,   167,   168,
     169,   170,   171,   172,   173,   174,     0,   175,   176,   177,
     178,   179,   180,     0,     0,   151,     0,     0,     0,   339,
     152,   153,   154,   155,   156,   157,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   174,     0,   175,   176,   177,   178,   179,   180,
       0,     0,   151,     0,     0,     0,   340,   152,   153,   154,
     155,   156,   157,   158,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   168,   169,   170,   171,   172,   173,   174,
       0,   175,   176,   177,   178,   179,   180,   153,   154,   155,
     156,   157,   158,   159,   160,   161,   162,   163,   164,   165,
     166,   167,   168,   169,   170,   171,   172,   173,   174,     0,
     175,   176,   177,   178,   179,   180,   155,   156,   157,   158,
     159,   160,   161,   162,   163,   164,   165,   166,   167,   168,
     169,   170,   171,   172,   173,   174,     0,   175,   176,   177,
     178,   179,   180,   156,   157,   158,   159,   160,   161,   162,
     163,   164,   165,   166,   167,   168,   169,   170,   171,   172,
     173,   174,     0,   175,   176,   177,   178,   179,   180,    66,
       0,     0,    11,    12,    13,    14,     0,    16,     0,    18,
       0,     0,     0,     0,     0,     0,     0,     0,    23,    24,
      25,    26,    27,    28,    29,   157,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   174,     0,   175,   176,   177,   178,   179,   180,
     158,   159,   160,   161,   162,   163,   164,   165,   166,   167,
     168,   169,   170,   171,   172,   173,   174,     0,   175,   176,
     177,   178,   179,   180,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   168,   169,   170,   171,   172,   173,   174,
       0,   175,   176,   177,   178,   179,   180,   161,   162,   163,
     164,   165,   166,   167,   168,   169,   170,   171,   172,   173,
     174,     0,   175,   176,   177,   178,   179,   180
};

static const short yycheck[] =
{
      15,   187,     0,    11,    12,    13,    14,    12,    13,    14,
      18,     0,   187,    18,   197,   197,     3,   201,    33,    34,
      35,    36,    37,    38,     8,     9,    69,    42,    43,    13,
      58,    10,    41,    52,    49,    41,    52,    41,    41,     8,
       9,    39,    11,    12,    13,     8,     9,    10,    11,    12,
      13,    49,    20,    41,    34,    35,   175,    65,    34,    35,
      65,   180,    52,    11,     8,     9,    98,    11,    12,    13,
      98,    52,    81,    79,   258,    79,    79,    56,    34,    35,
      35,    34,    58,    31,    32,    33,    31,     0,    33,    73,
      78,    78,    20,    49,     8,     9,    10,    11,    12,    13,
     283,   283,    58,    19,    73,    61,    41,    19,    77,     6,
      73,    11,    20,   111,    30,    31,    49,    33,    30,    58,
      36,    33,    58,   309,   122,   123,   124,   125,    61,    73,
      29,    31,    32,    33,   309,   150,   151,   152,   153,   154,
     155,   156,   157,   158,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   168,   169,   170,   171,   172,   190,    73,
     188,    68,    49,   178,   179,    68,    31,   175,    33,    78,
     175,    33,   180,    79,    61,   180,   208,   185,    19,    52,
     185,    68,    69,    68,    69,    33,   201,   176,    33,    30,
      68,    69,    33,    79,   202,    33,    33,   202,    81,     8,
       3,     4,     5,     6,     7,     8,     9,    61,    11,    12,
      13,    61,    20,    79,   229,    18,   214,    20,    78,    78,
      33,    53,    25,    26,    27,    28,    53,    67,    68,    69,
      70,    71,    72,    53,    37,    53,    39,    40,    33,    33,
      33,    10,    41,   258,    79,    10,    49,    20,   276,    78,
      69,    79,   267,    79,    69,   270,    59,    60,    61,    69,
     183,    64,    65,    53,    69,   319,    69,   193,   195,    23,
      73,    74,   177,    76,    77,    17,   208,    80,    -1,   294,
      -1,   289,    -1,    -1,   289,    -1,    -1,    -1,    -1,    -1,
     305,   319,    58,    59,    60,    61,    62,    63,    64,    65,
      -1,    67,    68,    69,    70,    71,    72,    -1,   323,   324,
     325,   326,   310,     3,     4,     5,     6,     7,     8,     9,
      -1,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    -1,    -1,    -1,    25,    26,    27,    28,    -1,
      30,    31,    32,    33,    34,    35,    36,    37,    -1,    39,
      40,    -1,    59,    60,    61,    62,    63,    64,    65,    49,
      67,    68,    69,    70,    71,    72,    -1,    -1,    58,    59,
      60,    61,    -1,    -1,    64,    65,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    73,    74,    -1,    76,    77,    -1,    -1,
      80,     3,     4,     5,     6,     7,     8,     9,    -1,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      -1,    -1,    -1,    25,    26,    27,    28,    -1,    30,    31,
      32,    33,    34,    35,    36,    37,    -1,    39,    40,    -1,
      -1,    -1,    61,    62,    63,    64,    65,    49,    67,    68,
      69,    70,    71,    72,    -1,    -1,    -1,    59,    60,    61,
      -1,    -1,    64,    65,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    73,    74,    -1,    76,    77,    -1,    -1,    80,     3,
       4,     5,     6,     7,     8,     9,    -1,    11,    12,    13,
      -1,    -1,    -1,    -1,    18,    -1,    20,    -1,    -1,    -1,
      -1,    25,    26,    27,    28,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    37,    -1,    39,    40,    -1,    -1,    -1,
      -1,    -1,    -1,    64,    65,    49,    67,    68,    69,    70,
      71,    72,    -1,    -1,    -1,    59,    60,    61,    -1,    -1,
      64,    65,    -1,    -1,    -1,    69,    -1,    -1,    -1,    73,
      74,    -1,    76,    77,    -1,    -1,    80,     3,     4,     5,
       6,     7,     8,     9,    -1,    11,    12,    13,    -1,    -1,
      -1,    -1,    18,    -1,    20,    -1,    -1,    -1,    -1,    25,
      26,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    37,    11,    39,    40,    14,    15,    16,    17,    -1,
      19,    -1,    21,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    30,    31,    32,    33,    34,    35,    36,    64,    65,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    73,    74,    -1,
      76,    77,    11,    -1,    80,    14,    15,    16,    17,    -1,
      19,    -1,    21,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    30,    31,    32,    33,    34,    35,    36,    -1,    -1,
      79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      49,    -1,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    61,    67,    68,    69,    70,    71,    72,    68,
      69,    11,    -1,    -1,    14,    15,    16,    17,    -1,    19,
      79,    21,    -1,    23,    24,    -1,    -1,    -1,    -1,    -1,
      30,    31,    32,    33,    34,    35,    36,    -1,    38,    -1,
      -1,    41,    -1,    43,    -1,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    -1,    59,
      60,    61,    62,    63,    64,    65,    -1,    -1,    68,    69,
      -1,    71,    72,    -1,    -1,    11,    76,    77,    14,    15,
      16,    17,    -1,    19,    -1,    21,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    30,    31,    32,    33,    34,    35,
      36,    11,    -1,    -1,    14,    15,    16,    17,    -1,    19,
      -1,    21,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      30,    31,    32,    33,    34,    35,    36,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    75,
      -1,    -1,    -1,    -1,    38,    -1,    -1,    -1,    58,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    -1,    67,    68,    69,    70,    71,    72,    -1,
      -1,    -1,    -1,    -1,    38,    -1,    -1,    -1,    82,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    -1,    67,    68,    69,    70,    71,    72,    -1,
      -1,    38,    -1,    -1,    -1,    79,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    -1,
      67,    68,    69,    70,    71,    72,    -1,    -1,    38,    -1,
      -1,    -1,    79,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    -1,    67,    68,    69,
      70,    71,    72,    -1,    -1,    38,    -1,    -1,    -1,    79,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    -1,    67,    68,    69,    70,    71,    72,
      -1,    -1,    38,    -1,    -1,    -1,    79,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      -1,    67,    68,    69,    70,    71,    72,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    -1,
      67,    68,    69,    70,    71,    72,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    -1,    67,    68,    69,
      70,    71,    72,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    -1,    67,    68,    69,    70,    71,    72,    11,
      -1,    -1,    14,    15,    16,    17,    -1,    19,    -1,    21,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,
      32,    33,    34,    35,    36,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    -1,    67,    68,    69,    70,    71,    72,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    -1,    67,    68,
      69,    70,    71,    72,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      -1,    67,    68,    69,    70,    71,    72,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    -1,    67,    68,    69,    70,    71,    72
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,     3,     4,     5,     6,     7,     8,     9,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    25,
      26,    27,    28,    30,    31,    32,    33,    34,    35,    36,
      37,    39,    40,    49,    59,    60,    61,    64,    65,    69,
      73,    74,    76,    77,    80,    84,    85,    86,    87,    90,
      93,    94,    95,    96,    98,    99,   100,   110,   111,   115,
     118,   119,   120,   122,    69,    20,    11,    23,    24,    38,
      41,    43,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    59,    60,    61,    62,    63,
      64,    65,    68,    69,    71,    72,    76,    77,   111,   116,
       8,     9,    11,    12,    73,   120,   121,   121,   121,   121,
      11,    69,    87,    11,    31,    32,    33,   112,     9,    73,
     122,   121,    52,    52,    52,    52,    31,    32,   112,    19,
      30,    31,    33,    36,    19,    30,    33,    35,    34,    87,
      87,    87,    87,    87,    87,    86,   110,    87,    87,     0,
      41,    38,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    67,    68,    69,    70,    71,
      72,    87,    91,   110,     6,    20,    58,    97,   111,   100,
      49,    61,    68,    69,   102,   105,   106,   107,   108,   109,
      29,    89,    77,   121,    68,    68,    78,    79,   102,   117,
     110,    31,    33,    33,    52,   110,   110,   110,   110,    31,
      33,    33,    33,    19,    30,    33,    33,    33,    79,    79,
      87,    87,    87,    87,    87,    87,    87,    87,    87,    87,
      87,    87,    87,    87,    87,    87,    87,    87,    87,    87,
      87,    87,    87,    10,    11,    96,   121,    86,    88,   110,
     113,   114,    87,    87,    10,    96,   121,    41,    81,    92,
      92,   121,     8,    98,   119,   102,   104,     3,    78,    79,
     106,   113,    61,   107,    69,   108,   109,    91,   121,    20,
      78,    78,    61,   117,    79,    33,   110,    53,    53,    53,
      53,    33,    33,    33,    87,    82,    10,    78,    91,    79,
      41,    10,    87,    87,   100,   101,    78,    79,    79,   103,
      79,   121,    53,    69,    69,    69,    69,    87,    79,    98,
      75,   110,   101,    87,    87,    87,    87,    79,    79,    79,
      79
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
      yyerror ("syntax error: cannot back up");\
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
# define YYLEX yylex ()
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
yy_stack_print (short *bottom, short *top)
#else
static void
yy_stack_print (bottom, top)
    short *bottom;
    short *top;
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
int yyparse (void);
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
yyparse (void)
#else
int
yyparse ()

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
     to xreallocate them elsewhere.  */

  /* The state stack.  */
  short	yyssa[YYINITDEPTH];
  short *yyss = yyssa;
  register short *yyssp;

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
	/* Give user a chance to xreallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	short *yyss1 = yyss;


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
	short *yyss1 = yyss;
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
        case 4:
#line 268 "c-exp.y"
    { write_exp_elt_opcode(OP_TYPE);
			  write_exp_elt_type(yyvsp[0].tval);
			  write_exp_elt_opcode(OP_TYPE);}
    break;

  case 6:
#line 276 "c-exp.y"
    { write_exp_elt_opcode (BINOP_COMMA); }
    break;

  case 7:
#line 281 "c-exp.y"
    { write_exp_elt_opcode (UNOP_IND); }
    break;

  case 8:
#line 285 "c-exp.y"
    { write_exp_elt_opcode (UNOP_ADDR); }
    break;

  case 9:
#line 289 "c-exp.y"
    { write_exp_elt_opcode (UNOP_NEG); }
    break;

  case 10:
#line 293 "c-exp.y"
    { write_exp_elt_opcode (UNOP_PLUS); }
    break;

  case 11:
#line 297 "c-exp.y"
    { write_exp_elt_opcode (UNOP_LOGICAL_NOT); }
    break;

  case 12:
#line 301 "c-exp.y"
    { write_exp_elt_opcode (UNOP_COMPLEMENT); }
    break;

  case 13:
#line 305 "c-exp.y"
    { write_exp_elt_opcode (UNOP_PREINCREMENT); }
    break;

  case 14:
#line 309 "c-exp.y"
    { write_exp_elt_opcode (UNOP_PREDECREMENT); }
    break;

  case 15:
#line 313 "c-exp.y"
    { write_exp_elt_opcode (UNOP_POSTINCREMENT); }
    break;

  case 16:
#line 317 "c-exp.y"
    { write_exp_elt_opcode (UNOP_POSTDECREMENT); }
    break;

  case 17:
#line 321 "c-exp.y"
    { write_exp_elt_opcode (UNOP_SIZEOF); }
    break;

  case 18:
#line 325 "c-exp.y"
    { write_exp_elt_opcode (STRUCTOP_PTR);
			  write_exp_string (yyvsp[0].sval);
			  write_exp_elt_opcode (STRUCTOP_PTR); }
    break;

  case 19:
#line 331 "c-exp.y"
    { mark_struct_expression ();
			  write_exp_elt_opcode (STRUCTOP_PTR);
			  write_exp_string (yyvsp[-1].sval);
			  write_exp_elt_opcode (STRUCTOP_PTR); }
    break;

  case 20:
#line 338 "c-exp.y"
    { struct stoken s;
			  mark_struct_expression ();
			  write_exp_elt_opcode (STRUCTOP_PTR);
			  s.ptr = "";
			  s.length = 0;
			  write_exp_string (s);
			  write_exp_elt_opcode (STRUCTOP_PTR); }
    break;

  case 21:
#line 348 "c-exp.y"
    { /* exp->type::name becomes exp->*(&type::name) */
			  /* Note: this doesn't work if name is a
			     static member!  FIXME */
			  write_exp_elt_opcode (UNOP_ADDR);
			  write_exp_elt_opcode (STRUCTOP_MPTR); }
    break;

  case 22:
#line 356 "c-exp.y"
    { write_exp_elt_opcode (STRUCTOP_MPTR); }
    break;

  case 23:
#line 360 "c-exp.y"
    { write_exp_elt_opcode (STRUCTOP_STRUCT);
			  write_exp_string (yyvsp[0].sval);
			  write_exp_elt_opcode (STRUCTOP_STRUCT); }
    break;

  case 24:
#line 366 "c-exp.y"
    { mark_struct_expression ();
			  write_exp_elt_opcode (STRUCTOP_STRUCT);
			  write_exp_string (yyvsp[-1].sval);
			  write_exp_elt_opcode (STRUCTOP_STRUCT); }
    break;

  case 25:
#line 373 "c-exp.y"
    { struct stoken s;
			  mark_struct_expression ();
			  write_exp_elt_opcode (STRUCTOP_STRUCT);
			  s.ptr = "";
			  s.length = 0;
			  write_exp_string (s);
			  write_exp_elt_opcode (STRUCTOP_STRUCT); }
    break;

  case 26:
#line 383 "c-exp.y"
    { /* exp.type::name becomes exp.*(&type::name) */
			  /* Note: this doesn't work if name is a
			     static member!  FIXME */
			  write_exp_elt_opcode (UNOP_ADDR);
			  write_exp_elt_opcode (STRUCTOP_MEMBER); }
    break;

  case 27:
#line 391 "c-exp.y"
    { write_exp_elt_opcode (STRUCTOP_MEMBER); }
    break;

  case 28:
#line 395 "c-exp.y"
    { write_exp_elt_opcode (BINOP_SUBSCRIPT); }
    break;

  case 29:
#line 401 "c-exp.y"
    { start_arglist (); }
    break;

  case 30:
#line 403 "c-exp.y"
    { write_exp_elt_opcode (OP_FUNCALL);
			  write_exp_elt_longcst ((LONGEST) end_arglist ());
			  write_exp_elt_opcode (OP_FUNCALL); }
    break;

  case 31:
#line 409 "c-exp.y"
    {
			  /* This could potentially be a an argument defined
			     lookup function (Koenig).  */
			  write_exp_elt_opcode (OP_ADL_FUNC);
			  write_exp_elt_block (expression_context_block);
			  write_exp_elt_sym (NULL); /* Placeholder.  */
			  write_exp_string (yyvsp[-1].ssym.stoken);
			  write_exp_elt_opcode (OP_ADL_FUNC);

			/* This is to save the value of arglist_len
			   being accumulated by an outer function call.  */

			  start_arglist ();
			}
    break;

  case 32:
#line 424 "c-exp.y"
    {
			  write_exp_elt_opcode (OP_FUNCALL);
			  write_exp_elt_longcst ((LONGEST) end_arglist ());
			  write_exp_elt_opcode (OP_FUNCALL);
			}
    break;

  case 33:
#line 432 "c-exp.y"
    { start_arglist (); }
    break;

  case 35:
#line 439 "c-exp.y"
    { arglist_len = 1; }
    break;

  case 36:
#line 443 "c-exp.y"
    { arglist_len++; }
    break;

  case 37:
#line 447 "c-exp.y"
    { int i;
			  VEC (type_ptr) *type_list = yyvsp[-2].tvec;
			  struct type *type_elt;
			  LONGEST len = VEC_length (type_ptr, type_list);

			  write_exp_elt_opcode (TYPE_INSTANCE);
			  write_exp_elt_longcst (len);
			  for (i = 0;
			       VEC_iterate (type_ptr, type_list, i, type_elt);
			       ++i)
			    write_exp_elt_type (type_elt);
			  write_exp_elt_longcst(len);
			  write_exp_elt_opcode (TYPE_INSTANCE);
			  VEC_free (type_ptr, type_list);
			}
    break;

  case 38:
#line 465 "c-exp.y"
    { yyval.lval = end_arglist () - 1; }
    break;

  case 39:
#line 468 "c-exp.y"
    { write_exp_elt_opcode (OP_ARRAY);
			  write_exp_elt_longcst ((LONGEST) 0);
			  write_exp_elt_longcst ((LONGEST) yyvsp[0].lval);
			  write_exp_elt_opcode (OP_ARRAY); }
    break;

  case 40:
#line 475 "c-exp.y"
    { write_exp_elt_opcode (UNOP_MEMVAL);
			  write_exp_elt_type (yyvsp[-2].tval);
			  write_exp_elt_opcode (UNOP_MEMVAL); }
    break;

  case 41:
#line 481 "c-exp.y"
    { write_exp_elt_opcode (UNOP_CAST);
			  write_exp_elt_type (yyvsp[-2].tval);
			  write_exp_elt_opcode (UNOP_CAST); }
    break;

  case 42:
#line 487 "c-exp.y"
    { }
    break;

  case 43:
#line 493 "c-exp.y"
    { write_exp_elt_opcode (BINOP_REPEAT); }
    break;

  case 44:
#line 497 "c-exp.y"
    { write_exp_elt_opcode (BINOP_MUL); }
    break;

  case 45:
#line 501 "c-exp.y"
    { write_exp_elt_opcode (BINOP_DIV); }
    break;

  case 46:
#line 505 "c-exp.y"
    { write_exp_elt_opcode (BINOP_REM); }
    break;

  case 47:
#line 509 "c-exp.y"
    { write_exp_elt_opcode (BINOP_ADD); }
    break;

  case 48:
#line 513 "c-exp.y"
    { write_exp_elt_opcode (BINOP_SUB); }
    break;

  case 49:
#line 517 "c-exp.y"
    { write_exp_elt_opcode (BINOP_LSH); }
    break;

  case 50:
#line 521 "c-exp.y"
    { write_exp_elt_opcode (BINOP_RSH); }
    break;

  case 51:
#line 525 "c-exp.y"
    { write_exp_elt_opcode (BINOP_EQUAL); }
    break;

  case 52:
#line 529 "c-exp.y"
    { write_exp_elt_opcode (BINOP_NOTEQUAL); }
    break;

  case 53:
#line 533 "c-exp.y"
    { write_exp_elt_opcode (BINOP_LEQ); }
    break;

  case 54:
#line 537 "c-exp.y"
    { write_exp_elt_opcode (BINOP_GEQ); }
    break;

  case 55:
#line 541 "c-exp.y"
    { write_exp_elt_opcode (BINOP_LESS); }
    break;

  case 56:
#line 545 "c-exp.y"
    { write_exp_elt_opcode (BINOP_GTR); }
    break;

  case 57:
#line 549 "c-exp.y"
    { write_exp_elt_opcode (BINOP_BITWISE_AND); }
    break;

  case 58:
#line 553 "c-exp.y"
    { write_exp_elt_opcode (BINOP_BITWISE_XOR); }
    break;

  case 59:
#line 557 "c-exp.y"
    { write_exp_elt_opcode (BINOP_BITWISE_IOR); }
    break;

  case 60:
#line 561 "c-exp.y"
    { write_exp_elt_opcode (BINOP_LOGICAL_AND); }
    break;

  case 61:
#line 565 "c-exp.y"
    { write_exp_elt_opcode (BINOP_LOGICAL_OR); }
    break;

  case 62:
#line 569 "c-exp.y"
    { write_exp_elt_opcode (TERNOP_COND); }
    break;

  case 63:
#line 573 "c-exp.y"
    { write_exp_elt_opcode (BINOP_ASSIGN); }
    break;

  case 64:
#line 577 "c-exp.y"
    { write_exp_elt_opcode (BINOP_ASSIGN_MODIFY);
			  write_exp_elt_opcode (yyvsp[-1].opcode);
			  write_exp_elt_opcode (BINOP_ASSIGN_MODIFY); }
    break;

  case 65:
#line 583 "c-exp.y"
    { write_exp_elt_opcode (OP_LONG);
			  write_exp_elt_type (yyvsp[0].typed_val_int.type);
			  write_exp_elt_longcst ((LONGEST)(yyvsp[0].typed_val_int.val));
			  write_exp_elt_opcode (OP_LONG); }
    break;

  case 66:
#line 590 "c-exp.y"
    {
			  struct stoken_vector vec;
			  vec.len = 1;
			  vec.tokens = &yyvsp[0].tsval;
			  write_exp_string_vector (yyvsp[0].tsval.type, &vec);
			}
    break;

  case 67:
#line 599 "c-exp.y"
    { YYSTYPE val;
			  parse_number (yyvsp[0].ssym.stoken.ptr, yyvsp[0].ssym.stoken.length, 0, &val);
			  write_exp_elt_opcode (OP_LONG);
			  write_exp_elt_type (val.typed_val_int.type);
			  write_exp_elt_longcst ((LONGEST)val.typed_val_int.val);
			  write_exp_elt_opcode (OP_LONG);
			}
    break;

  case 68:
#line 610 "c-exp.y"
    { write_exp_elt_opcode (OP_DOUBLE);
			  write_exp_elt_type (yyvsp[0].typed_val_float.type);
			  write_exp_elt_dblcst (yyvsp[0].typed_val_float.dval);
			  write_exp_elt_opcode (OP_DOUBLE); }
    break;

  case 69:
#line 617 "c-exp.y"
    { write_exp_elt_opcode (OP_DECFLOAT);
			  write_exp_elt_type (yyvsp[0].typed_val_decfloat.type);
			  write_exp_elt_decfloatcst (yyvsp[0].typed_val_decfloat.val);
			  write_exp_elt_opcode (OP_DECFLOAT); }
    break;

  case 71:
#line 627 "c-exp.y"
    {
			  write_dollar_variable (yyvsp[0].sval);
			}
    break;

  case 72:
#line 633 "c-exp.y"
    { write_exp_elt_opcode (OP_LONG);
			  write_exp_elt_type (lookup_signed_typename
					      (parse_language, parse_gdbarch,
					       "int"));
			  CHECK_TYPEDEF (yyvsp[-1].tval);
			  write_exp_elt_longcst ((LONGEST) TYPE_LENGTH (yyvsp[-1].tval));
			  write_exp_elt_opcode (OP_LONG); }
    break;

  case 73:
#line 643 "c-exp.y"
    { write_exp_elt_opcode (UNOP_REINTERPRET_CAST);
			  write_exp_elt_type (yyvsp[-4].tval);
			  write_exp_elt_opcode (UNOP_REINTERPRET_CAST); }
    break;

  case 74:
#line 649 "c-exp.y"
    { write_exp_elt_opcode (UNOP_CAST);
			  write_exp_elt_type (yyvsp[-4].tval);
			  write_exp_elt_opcode (UNOP_CAST); }
    break;

  case 75:
#line 655 "c-exp.y"
    { write_exp_elt_opcode (UNOP_DYNAMIC_CAST);
			  write_exp_elt_type (yyvsp[-4].tval);
			  write_exp_elt_opcode (UNOP_DYNAMIC_CAST); }
    break;

  case 76:
#line 661 "c-exp.y"
    { /* We could do more error checking here, but
			     it doesn't seem worthwhile.  */
			  write_exp_elt_opcode (UNOP_CAST);
			  write_exp_elt_type (yyvsp[-4].tval);
			  write_exp_elt_opcode (UNOP_CAST); }
    break;

  case 77:
#line 670 "c-exp.y"
    {
			  /* We copy the string here, and not in the
			     lexer, to guarantee that we do not leak a
			     string.  Note that we follow the
			     NUL-termination convention of the
			     lexer.  */
			  struct typed_stoken *vec = XNEW (struct typed_stoken);
			  yyval.svec.len = 1;
			  yyval.svec.tokens = vec;

			  vec->type = yyvsp[0].tsval.type;
			  vec->length = yyvsp[0].tsval.length;
			  vec->ptr = xmalloc (yyvsp[0].tsval.length + 1);
			  memcpy (vec->ptr, yyvsp[0].tsval.ptr, yyvsp[0].tsval.length + 1);
			}
    break;

  case 78:
#line 687 "c-exp.y"
    {
			  /* Note that we NUL-terminate here, but just
			     for convenience.  */
			  char *p;
			  ++yyval.svec.len;
			  yyval.svec.tokens = xrealloc (yyval.svec.tokens,
					       yyval.svec.len * sizeof (struct typed_stoken));

			  p = xmalloc (yyvsp[0].tsval.length + 1);
			  memcpy (p, yyvsp[0].tsval.ptr, yyvsp[0].tsval.length + 1);

			  yyval.svec.tokens[yyval.svec.len - 1].type = yyvsp[0].tsval.type;
			  yyval.svec.tokens[yyval.svec.len - 1].length = yyvsp[0].tsval.length;
			  yyval.svec.tokens[yyval.svec.len - 1].ptr = p;
			}
    break;

  case 79:
#line 705 "c-exp.y"
    {
			  int i;
			  enum c_string_type type = C_STRING;

			  for (i = 0; i < yyvsp[0].svec.len; ++i)
			    {
			      switch (yyvsp[0].svec.tokens[i].type)
				{
				case C_STRING:
				  break;
				case C_WIDE_STRING:
				case C_STRING_16:
				case C_STRING_32:
				  if (type != C_STRING
				      && type != yyvsp[0].svec.tokens[i].type)
				    error (_("Undefined string concatenation."));
				  type = yyvsp[0].svec.tokens[i].type;
				  break;
				default:
				  /* internal error */
				  internal_error (__FILE__, __LINE__,
						  "unrecognized type in string concatenation");
				}
			    }

			  write_exp_string_vector (type, &yyvsp[0].svec);
			  for (i = 0; i < yyvsp[0].svec.len; ++i)
			    xfree (yyvsp[0].svec.tokens[i].ptr);
			  xfree (yyvsp[0].svec.tokens);
			}
    break;

  case 80:
#line 739 "c-exp.y"
    { write_exp_elt_opcode (OP_LONG);
                          write_exp_elt_type (parse_type->builtin_bool);
                          write_exp_elt_longcst ((LONGEST) 1);
                          write_exp_elt_opcode (OP_LONG); }
    break;

  case 81:
#line 746 "c-exp.y"
    { write_exp_elt_opcode (OP_LONG);
                          write_exp_elt_type (parse_type->builtin_bool);
                          write_exp_elt_longcst ((LONGEST) 0);
                          write_exp_elt_opcode (OP_LONG); }
    break;

  case 82:
#line 755 "c-exp.y"
    {
			  if (yyvsp[0].ssym.sym)
			    yyval.bval = SYMBOL_BLOCK_VALUE (yyvsp[0].ssym.sym);
			  else
			    error (_("No file or function \"%s\"."),
				   copy_name (yyvsp[0].ssym.stoken));
			}
    break;

  case 83:
#line 763 "c-exp.y"
    {
			  yyval.bval = yyvsp[0].bval;
			}
    break;

  case 84:
#line 769 "c-exp.y"
    { struct symbol *tem
			    = lookup_symbol (copy_name (yyvsp[0].sval), yyvsp[-2].bval,
					     VAR_DOMAIN, (int *) NULL);
			  if (!tem || SYMBOL_CLASS (tem) != LOC_BLOCK)
			    error (_("No function \"%s\" in specified context."),
				   copy_name (yyvsp[0].sval));
			  yyval.bval = SYMBOL_BLOCK_VALUE (tem); }
    break;

  case 85:
#line 779 "c-exp.y"
    { struct symbol *sym = yyvsp[-1].ssym.sym;

			  if (sym == NULL || !SYMBOL_IS_ARGUMENT (sym)
			      || !symbol_read_needs_frame (sym))
			    error (_("@entry can be used only for function "
				     "parameters, not for \"%s\""),
				   copy_name (yyvsp[-1].ssym.stoken));

			  write_exp_elt_opcode (OP_VAR_ENTRY_VALUE);
			  write_exp_elt_sym (sym);
			  write_exp_elt_opcode (OP_VAR_ENTRY_VALUE);
			}
    break;

  case 86:
#line 794 "c-exp.y"
    { struct symbol *sym;
			  sym = lookup_symbol (copy_name (yyvsp[0].sval), yyvsp[-2].bval,
					       VAR_DOMAIN, (int *) NULL);
			  if (sym == 0)
			    error (_("No symbol \"%s\" in specified context."),
				   copy_name (yyvsp[0].sval));
			  if (symbol_read_needs_frame (sym))
			    {
			      if (innermost_block == 0
				  || contained_in (block_found,
						   innermost_block))
				innermost_block = block_found;
			    }

			  write_exp_elt_opcode (OP_VAR_VALUE);
			  /* block_found is set by lookup_symbol.  */
			  write_exp_elt_block (block_found);
			  write_exp_elt_sym (sym);
			  write_exp_elt_opcode (OP_VAR_VALUE); }
    break;

  case 87:
#line 816 "c-exp.y"
    {
			  struct type *type = yyvsp[-2].tsym.type;
			  CHECK_TYPEDEF (type);
			  if (TYPE_CODE (type) != TYPE_CODE_STRUCT
			      && TYPE_CODE (type) != TYPE_CODE_UNION
			      && TYPE_CODE (type) != TYPE_CODE_NAMESPACE)
			    error (_("`%s' is not defined as an aggregate type."),
				   TYPE_NAME (type));

			  write_exp_elt_opcode (OP_SCOPE);
			  write_exp_elt_type (type);
			  write_exp_string (yyvsp[0].sval);
			  write_exp_elt_opcode (OP_SCOPE);
			}
    break;

  case 88:
#line 831 "c-exp.y"
    {
			  struct type *type = yyvsp[-3].tsym.type;
			  struct stoken tmp_token;
			  CHECK_TYPEDEF (type);
			  if (TYPE_CODE (type) != TYPE_CODE_STRUCT
			      && TYPE_CODE (type) != TYPE_CODE_UNION
			      && TYPE_CODE (type) != TYPE_CODE_NAMESPACE)
			    error (_("`%s' is not defined as an aggregate type."),
				   TYPE_NAME (type));

			  tmp_token.ptr = (char*) alloca (yyvsp[0].sval.length + 2);
			  tmp_token.length = yyvsp[0].sval.length + 1;
			  tmp_token.ptr[0] = '~';
			  memcpy (tmp_token.ptr+1, yyvsp[0].sval.ptr, yyvsp[0].sval.length);
			  tmp_token.ptr[tmp_token.length] = 0;

			  /* Check for valid destructor name.  */
			  destructor_name_p (tmp_token.ptr, yyvsp[-3].tsym.type);
			  write_exp_elt_opcode (OP_SCOPE);
			  write_exp_elt_type (type);
			  write_exp_string (tmp_token);
			  write_exp_elt_opcode (OP_SCOPE);
			}
    break;

  case 89:
#line 855 "c-exp.y"
    {
			  char *copy = copy_name (yyvsp[-2].sval);
			  error (_("No type \"%s\" within class "
				   "or namespace \"%s\"."),
				 copy, TYPE_NAME (yyvsp[-4].tsym.type));
			}
    break;

  case 91:
#line 865 "c-exp.y"
    {
			  char *name = copy_name (yyvsp[0].ssym.stoken);
			  struct symbol *sym;
			  struct minimal_symbol *msymbol;

			  sym =
			    lookup_symbol (name, (const struct block *) NULL,
					   VAR_DOMAIN, (int *) NULL);
			  if (sym)
			    {
			      write_exp_elt_opcode (OP_VAR_VALUE);
			      write_exp_elt_block (NULL);
			      write_exp_elt_sym (sym);
			      write_exp_elt_opcode (OP_VAR_VALUE);
			      break;
			    }

			  msymbol = lookup_minimal_symbol (name, NULL, NULL);
			  if (msymbol != NULL)
			    write_exp_msymbol (msymbol);
			  else if (!have_full_symbols () && !have_partial_symbols ())
			    error (_("No symbol table is loaded.  Use the \"file\" command."));
			  else
			    error (_("No symbol \"%s\" in current context."), name);
			}
    break;

  case 92:
#line 893 "c-exp.y"
    { struct symbol *sym = yyvsp[0].ssym.sym;

			  if (sym)
			    {
			      if (symbol_read_needs_frame (sym))
				{
				  if (innermost_block == 0
				      || contained_in (block_found, 
						       innermost_block))
				    innermost_block = block_found;
				}

			      write_exp_elt_opcode (OP_VAR_VALUE);
			      /* We want to use the selected frame, not
				 another more inner frame which happens to
				 be in the same block.  */
			      write_exp_elt_block (NULL);
			      write_exp_elt_sym (sym);
			      write_exp_elt_opcode (OP_VAR_VALUE);
			    }
			  else if (yyvsp[0].ssym.is_a_field_of_this)
			    {
			      /* C++: it hangs off of `this'.  Must
			         not inadvertently convert from a method call
				 to data ref.  */
			      if (innermost_block == 0
				  || contained_in (block_found,
						   innermost_block))
				innermost_block = block_found;
			      write_exp_elt_opcode (OP_THIS);
			      write_exp_elt_opcode (OP_THIS);
			      write_exp_elt_opcode (STRUCTOP_PTR);
			      write_exp_string (yyvsp[0].ssym.stoken);
			      write_exp_elt_opcode (STRUCTOP_PTR);
			    }
			  else
			    {
			      struct minimal_symbol *msymbol;
			      char *arg = copy_name (yyvsp[0].ssym.stoken);

			      msymbol =
				lookup_minimal_symbol (arg, NULL, NULL);
			      if (msymbol != NULL)
				write_exp_msymbol (msymbol);
			      else if (!have_full_symbols () && !have_partial_symbols ())
				error (_("No symbol table is loaded.  Use the \"file\" command."));
			      else
				error (_("No symbol \"%s\" in current context."),
				       copy_name (yyvsp[0].ssym.stoken));
			    }
			}
    break;

  case 93:
#line 947 "c-exp.y"
    { insert_type_address_space (copy_name (yyvsp[0].ssym.stoken)); }
    break;

  case 101:
#line 968 "c-exp.y"
    { insert_type (tp_pointer); }
    break;

  case 103:
#line 971 "c-exp.y"
    { insert_type (tp_pointer); }
    break;

  case 105:
#line 974 "c-exp.y"
    { insert_type (tp_reference); }
    break;

  case 106:
#line 976 "c-exp.y"
    { insert_type (tp_reference); }
    break;

  case 107:
#line 980 "c-exp.y"
    {
			  yyval.type_stack = get_type_stack ();
			  /* This cleanup is eventually run by
			     c_parse.  */
			  make_cleanup (type_stack_cleanup, yyval.type_stack);
			}
    break;

  case 108:
#line 989 "c-exp.y"
    { yyval.type_stack = append_type_stack (yyvsp[0].type_stack, yyvsp[-1].type_stack); }
    break;

  case 111:
#line 995 "c-exp.y"
    { yyval.type_stack = yyvsp[-1].type_stack; }
    break;

  case 112:
#line 997 "c-exp.y"
    {
			  push_type_stack (yyvsp[-1].type_stack);
			  push_type_int (yyvsp[0].lval);
			  push_type (tp_array);
			  yyval.type_stack = get_type_stack ();
			}
    break;

  case 113:
#line 1004 "c-exp.y"
    {
			  push_type_int (yyvsp[0].lval);
			  push_type (tp_array);
			  yyval.type_stack = get_type_stack ();
			}
    break;

  case 114:
#line 1011 "c-exp.y"
    {
			  push_type_stack (yyvsp[-1].type_stack);
			  push_typelist (yyvsp[0].tvec);
			  yyval.type_stack = get_type_stack ();
			}
    break;

  case 115:
#line 1017 "c-exp.y"
    {
			  push_typelist (yyvsp[0].tvec);
			  yyval.type_stack = get_type_stack ();
			}
    break;

  case 116:
#line 1024 "c-exp.y"
    { yyval.lval = -1; }
    break;

  case 117:
#line 1026 "c-exp.y"
    { yyval.lval = yyvsp[-1].typed_val_int.val; }
    break;

  case 118:
#line 1030 "c-exp.y"
    { yyval.tvec = NULL; }
    break;

  case 119:
#line 1032 "c-exp.y"
    { yyval.tvec = yyvsp[-1].tvec; }
    break;

  case 121:
#line 1048 "c-exp.y"
    { yyval.tval = yyvsp[0].tsym.type; }
    break;

  case 122:
#line 1050 "c-exp.y"
    { yyval.tval = lookup_signed_typename (parse_language,
						       parse_gdbarch,
						       "int"); }
    break;

  case 123:
#line 1054 "c-exp.y"
    { yyval.tval = lookup_signed_typename (parse_language,
						       parse_gdbarch,
						       "long"); }
    break;

  case 124:
#line 1058 "c-exp.y"
    { yyval.tval = lookup_signed_typename (parse_language,
						       parse_gdbarch,
						       "short"); }
    break;

  case 125:
#line 1062 "c-exp.y"
    { yyval.tval = lookup_signed_typename (parse_language,
						       parse_gdbarch,
						       "long"); }
    break;

  case 126:
#line 1066 "c-exp.y"
    { yyval.tval = lookup_signed_typename (parse_language,
						       parse_gdbarch,
						       "long"); }
    break;

  case 127:
#line 1070 "c-exp.y"
    { yyval.tval = lookup_signed_typename (parse_language,
						       parse_gdbarch,
						       "long"); }
    break;

  case 128:
#line 1074 "c-exp.y"
    { yyval.tval = lookup_signed_typename (parse_language,
						       parse_gdbarch,
						       "long"); }
    break;

  case 129:
#line 1078 "c-exp.y"
    { yyval.tval = lookup_unsigned_typename (parse_language,
							 parse_gdbarch,
							 "long"); }
    break;

  case 130:
#line 1082 "c-exp.y"
    { yyval.tval = lookup_unsigned_typename (parse_language,
							 parse_gdbarch,
							 "long"); }
    break;

  case 131:
#line 1086 "c-exp.y"
    { yyval.tval = lookup_unsigned_typename (parse_language,
							 parse_gdbarch,
							 "long"); }
    break;

  case 132:
#line 1090 "c-exp.y"
    { yyval.tval = lookup_signed_typename (parse_language,
						       parse_gdbarch,
						       "long long"); }
    break;

  case 133:
#line 1094 "c-exp.y"
    { yyval.tval = lookup_signed_typename (parse_language,
						       parse_gdbarch,
						       "long long"); }
    break;

  case 134:
#line 1098 "c-exp.y"
    { yyval.tval = lookup_signed_typename (parse_language,
						       parse_gdbarch,
						       "long long"); }
    break;

  case 135:
#line 1102 "c-exp.y"
    { yyval.tval = lookup_signed_typename (parse_language,
						       parse_gdbarch,
						       "long long"); }
    break;

  case 136:
#line 1106 "c-exp.y"
    { yyval.tval = lookup_signed_typename (parse_language,
						       parse_gdbarch,
						       "long long"); }
    break;

  case 137:
#line 1110 "c-exp.y"
    { yyval.tval = lookup_signed_typename (parse_language,
						       parse_gdbarch,
						       "long long"); }
    break;

  case 138:
#line 1114 "c-exp.y"
    { yyval.tval = lookup_unsigned_typename (parse_language,
							 parse_gdbarch,
							 "long long"); }
    break;

  case 139:
#line 1118 "c-exp.y"
    { yyval.tval = lookup_unsigned_typename (parse_language,
							 parse_gdbarch,
							 "long long"); }
    break;

  case 140:
#line 1122 "c-exp.y"
    { yyval.tval = lookup_unsigned_typename (parse_language,
							 parse_gdbarch,
							 "long long"); }
    break;

  case 141:
#line 1126 "c-exp.y"
    { yyval.tval = lookup_unsigned_typename (parse_language,
							 parse_gdbarch,
							 "long long"); }
    break;

  case 142:
#line 1130 "c-exp.y"
    { yyval.tval = lookup_signed_typename (parse_language,
						       parse_gdbarch,
						       "short"); }
    break;

  case 143:
#line 1134 "c-exp.y"
    { yyval.tval = lookup_signed_typename (parse_language,
						       parse_gdbarch,
						       "short"); }
    break;

  case 144:
#line 1138 "c-exp.y"
    { yyval.tval = lookup_signed_typename (parse_language,
						       parse_gdbarch,
						       "short"); }
    break;

  case 145:
#line 1142 "c-exp.y"
    { yyval.tval = lookup_unsigned_typename (parse_language,
							 parse_gdbarch,
							 "short"); }
    break;

  case 146:
#line 1146 "c-exp.y"
    { yyval.tval = lookup_unsigned_typename (parse_language,
							 parse_gdbarch,
							 "short"); }
    break;

  case 147:
#line 1150 "c-exp.y"
    { yyval.tval = lookup_unsigned_typename (parse_language,
							 parse_gdbarch,
							 "short"); }
    break;

  case 148:
#line 1154 "c-exp.y"
    { yyval.tval = lookup_typename (parse_language, parse_gdbarch,
						"double", (struct block *) NULL,
						0); }
    break;

  case 149:
#line 1158 "c-exp.y"
    { yyval.tval = lookup_typename (parse_language, parse_gdbarch,
						"long double",
						(struct block *) NULL, 0); }
    break;

  case 150:
#line 1162 "c-exp.y"
    { yyval.tval = lookup_struct (copy_name (yyvsp[0].sval),
					      expression_context_block); }
    break;

  case 151:
#line 1165 "c-exp.y"
    { yyval.tval = lookup_struct (copy_name (yyvsp[0].sval),
					      expression_context_block); }
    break;

  case 152:
#line 1168 "c-exp.y"
    { yyval.tval = lookup_union (copy_name (yyvsp[0].sval),
					     expression_context_block); }
    break;

  case 153:
#line 1171 "c-exp.y"
    { yyval.tval = lookup_enum (copy_name (yyvsp[0].sval),
					    expression_context_block); }
    break;

  case 154:
#line 1174 "c-exp.y"
    { yyval.tval = lookup_unsigned_typename (parse_language,
							 parse_gdbarch,
							 TYPE_NAME(yyvsp[0].tsym.type)); }
    break;

  case 155:
#line 1178 "c-exp.y"
    { yyval.tval = lookup_unsigned_typename (parse_language,
							 parse_gdbarch,
							 "int"); }
    break;

  case 156:
#line 1182 "c-exp.y"
    { yyval.tval = lookup_signed_typename (parse_language,
						       parse_gdbarch,
						       TYPE_NAME(yyvsp[0].tsym.type)); }
    break;

  case 157:
#line 1186 "c-exp.y"
    { yyval.tval = lookup_signed_typename (parse_language,
						       parse_gdbarch,
						       "int"); }
    break;

  case 158:
#line 1193 "c-exp.y"
    { yyval.tval = lookup_template_type(copy_name(yyvsp[-3].sval), yyvsp[-1].tval,
						    expression_context_block);
			}
    break;

  case 159:
#line 1197 "c-exp.y"
    { yyval.tval = follow_types (yyvsp[0].tval); }
    break;

  case 160:
#line 1199 "c-exp.y"
    { yyval.tval = follow_types (yyvsp[-1].tval); }
    break;

  case 162:
#line 1204 "c-exp.y"
    {
		  yyval.tsym.stoken.ptr = "int";
		  yyval.tsym.stoken.length = 3;
		  yyval.tsym.type = lookup_signed_typename (parse_language,
						    parse_gdbarch,
						    "int");
		}
    break;

  case 163:
#line 1212 "c-exp.y"
    {
		  yyval.tsym.stoken.ptr = "long";
		  yyval.tsym.stoken.length = 4;
		  yyval.tsym.type = lookup_signed_typename (parse_language,
						    parse_gdbarch,
						    "long");
		}
    break;

  case 164:
#line 1220 "c-exp.y"
    {
		  yyval.tsym.stoken.ptr = "short";
		  yyval.tsym.stoken.length = 5;
		  yyval.tsym.type = lookup_signed_typename (parse_language,
						    parse_gdbarch,
						    "short");
		}
    break;

  case 165:
#line 1231 "c-exp.y"
    { check_parameter_typelist (yyvsp[0].tvec); }
    break;

  case 166:
#line 1233 "c-exp.y"
    {
			  VEC_safe_push (type_ptr, yyvsp[-2].tvec, NULL);
			  check_parameter_typelist (yyvsp[-2].tvec);
			  yyval.tvec = yyvsp[-2].tvec;
			}
    break;

  case 167:
#line 1242 "c-exp.y"
    {
		  VEC (type_ptr) *typelist = NULL;
		  VEC_safe_push (type_ptr, typelist, yyvsp[0].tval);
		  yyval.tvec = typelist;
		}
    break;

  case 168:
#line 1248 "c-exp.y"
    {
		  VEC_safe_push (type_ptr, yyvsp[-2].tvec, yyvsp[0].tval);
		  yyval.tvec = yyvsp[-2].tvec;
		}
    break;

  case 170:
#line 1256 "c-exp.y"
    {
		  push_type_stack (yyvsp[0].type_stack);
		  yyval.tval = follow_types (yyvsp[-1].tval);
		}
    break;

  case 171:
#line 1263 "c-exp.y"
    { yyval.tval = follow_types (yyvsp[-1].tval); }
    break;

  case 176:
#line 1275 "c-exp.y"
    { insert_type (tp_const);
			  insert_type (tp_volatile); 
			}
    break;

  case 177:
#line 1279 "c-exp.y"
    { insert_type (tp_const); }
    break;

  case 178:
#line 1281 "c-exp.y"
    { insert_type (tp_volatile); }
    break;

  case 179:
#line 1285 "c-exp.y"
    { yyval.sval = operator_stoken (" new"); }
    break;

  case 180:
#line 1287 "c-exp.y"
    { yyval.sval = operator_stoken (" delete"); }
    break;

  case 181:
#line 1289 "c-exp.y"
    { yyval.sval = operator_stoken (" new[]"); }
    break;

  case 182:
#line 1291 "c-exp.y"
    { yyval.sval = operator_stoken (" delete[]"); }
    break;

  case 183:
#line 1293 "c-exp.y"
    { yyval.sval = operator_stoken ("+"); }
    break;

  case 184:
#line 1295 "c-exp.y"
    { yyval.sval = operator_stoken ("-"); }
    break;

  case 185:
#line 1297 "c-exp.y"
    { yyval.sval = operator_stoken ("*"); }
    break;

  case 186:
#line 1299 "c-exp.y"
    { yyval.sval = operator_stoken ("/"); }
    break;

  case 187:
#line 1301 "c-exp.y"
    { yyval.sval = operator_stoken ("%"); }
    break;

  case 188:
#line 1303 "c-exp.y"
    { yyval.sval = operator_stoken ("^"); }
    break;

  case 189:
#line 1305 "c-exp.y"
    { yyval.sval = operator_stoken ("&"); }
    break;

  case 190:
#line 1307 "c-exp.y"
    { yyval.sval = operator_stoken ("|"); }
    break;

  case 191:
#line 1309 "c-exp.y"
    { yyval.sval = operator_stoken ("~"); }
    break;

  case 192:
#line 1311 "c-exp.y"
    { yyval.sval = operator_stoken ("!"); }
    break;

  case 193:
#line 1313 "c-exp.y"
    { yyval.sval = operator_stoken ("="); }
    break;

  case 194:
#line 1315 "c-exp.y"
    { yyval.sval = operator_stoken ("<"); }
    break;

  case 195:
#line 1317 "c-exp.y"
    { yyval.sval = operator_stoken (">"); }
    break;

  case 196:
#line 1319 "c-exp.y"
    { const char *op = "unknown";
			  switch (yyvsp[0].opcode)
			    {
			    case BINOP_RSH:
			      op = ">>=";
			      break;
			    case BINOP_LSH:
			      op = "<<=";
			      break;
			    case BINOP_ADD:
			      op = "+=";
			      break;
			    case BINOP_SUB:
			      op = "-=";
			      break;
			    case BINOP_MUL:
			      op = "*=";
			      break;
			    case BINOP_DIV:
			      op = "/=";
			      break;
			    case BINOP_REM:
			      op = "%=";
			      break;
			    case BINOP_BITWISE_IOR:
			      op = "|=";
			      break;
			    case BINOP_BITWISE_AND:
			      op = "&=";
			      break;
			    case BINOP_BITWISE_XOR:
			      op = "^=";
			      break;
			    default:
			      break;
			    }

			  yyval.sval = operator_stoken (op);
			}
    break;

  case 197:
#line 1359 "c-exp.y"
    { yyval.sval = operator_stoken ("<<"); }
    break;

  case 198:
#line 1361 "c-exp.y"
    { yyval.sval = operator_stoken (">>"); }
    break;

  case 199:
#line 1363 "c-exp.y"
    { yyval.sval = operator_stoken ("=="); }
    break;

  case 200:
#line 1365 "c-exp.y"
    { yyval.sval = operator_stoken ("!="); }
    break;

  case 201:
#line 1367 "c-exp.y"
    { yyval.sval = operator_stoken ("<="); }
    break;

  case 202:
#line 1369 "c-exp.y"
    { yyval.sval = operator_stoken (">="); }
    break;

  case 203:
#line 1371 "c-exp.y"
    { yyval.sval = operator_stoken ("&&"); }
    break;

  case 204:
#line 1373 "c-exp.y"
    { yyval.sval = operator_stoken ("||"); }
    break;

  case 205:
#line 1375 "c-exp.y"
    { yyval.sval = operator_stoken ("++"); }
    break;

  case 206:
#line 1377 "c-exp.y"
    { yyval.sval = operator_stoken ("--"); }
    break;

  case 207:
#line 1379 "c-exp.y"
    { yyval.sval = operator_stoken (","); }
    break;

  case 208:
#line 1381 "c-exp.y"
    { yyval.sval = operator_stoken ("->*"); }
    break;

  case 209:
#line 1383 "c-exp.y"
    { yyval.sval = operator_stoken ("->"); }
    break;

  case 210:
#line 1385 "c-exp.y"
    { yyval.sval = operator_stoken ("()"); }
    break;

  case 211:
#line 1387 "c-exp.y"
    { yyval.sval = operator_stoken ("[]"); }
    break;

  case 212:
#line 1389 "c-exp.y"
    { char *name;
			  long length;
			  struct ui_file *buf = mem_fileopen ();

			  c_print_type (yyvsp[0].tval, NULL, buf, -1, 0);
			  name = ui_file_xstrdup (buf, &length);
			  ui_file_delete (buf);
			  yyval.sval = operator_stoken (name);
			  xfree (name);
			}
    break;

  case 213:
#line 1403 "c-exp.y"
    { yyval.sval = yyvsp[0].ssym.stoken; }
    break;

  case 214:
#line 1404 "c-exp.y"
    { yyval.sval = yyvsp[0].ssym.stoken; }
    break;

  case 215:
#line 1405 "c-exp.y"
    { yyval.sval = yyvsp[0].tsym.stoken; }
    break;

  case 216:
#line 1406 "c-exp.y"
    { yyval.sval = yyvsp[0].ssym.stoken; }
    break;

  case 217:
#line 1407 "c-exp.y"
    { yyval.sval = yyvsp[0].ssym.stoken; }
    break;

  case 218:
#line 1408 "c-exp.y"
    { yyval.sval = yyvsp[0].sval; }
    break;

  case 221:
#line 1421 "c-exp.y"
    {
			  yyval.ssym.stoken = yyvsp[0].sval;
			  yyval.ssym.sym = lookup_symbol (yyvsp[0].sval.ptr,
						  expression_context_block,
						  VAR_DOMAIN,
						  &yyval.ssym.is_a_field_of_this);
			}
    break;


    }

/* Line 1000 of yacc.c.  */
#line 3228 "c-exp.c"

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
	      yyerror (yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    yyerror ("syntax error; also virtual memory exhausted");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror ("syntax error");
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
  yyerror ("parser stack overflow");
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


#line 1431 "c-exp.y"


/* Returns a stoken of the operator name given by OP (which does not
   include the string "operator").  */ 
static struct stoken
operator_stoken (const char *op)
{
  static const char *operator_string = "operator";
  struct stoken st = { NULL, 0 };
  st.length = strlen (operator_string) + strlen (op);
  st.ptr = xmalloc (st.length + 1);
  strcpy (st.ptr, operator_string);
  strcat (st.ptr, op);

  /* The toplevel (c_parse) will free the memory allocated here.  */
  make_cleanup (xfree, st.ptr);
  return st;
};

/* Validate a parameter typelist.  */

static void
check_parameter_typelist (VEC (type_ptr) *params)
{
  struct type *type;
  int ix;

  for (ix = 0; VEC_iterate (type_ptr, params, ix, type); ++ix)
    {
      if (type != NULL && TYPE_CODE (check_typedef (type)) == TYPE_CODE_VOID)
	{
	  if (ix == 0)
	    {
	      if (VEC_length (type_ptr, params) == 1)
		{
		  /* Ok.  */
		  break;
		}
	      VEC_free (type_ptr, params);
	      error (_("parameter types following 'void'"));
	    }
	  else
	    {
	      VEC_free (type_ptr, params);
	      error (_("'void' invalid as parameter type"));
	    }
	}
    }
}

/* Take care of parsing a number (anything that starts with a digit).
   Set yylval and return the token type; update lexptr.
   LEN is the number of characters in it.  */

/*** Needs some error checking for the float case ***/

static int
parse_number (char *p, int len, int parsed_float, YYSTYPE *putithere)
{
  /* FIXME: Shouldn't these be unsigned?  We don't deal with negative values
     here, and we do kind of silly things like cast to unsigned.  */
  LONGEST n = 0;
  LONGEST prevn = 0;
  ULONGEST un;

  int i = 0;
  int c;
  int base = input_radix;
  int unsigned_p = 0;

  /* Number of "L" suffixes encountered.  */
  int long_p = 0;

  /* We have found a "L" or "U" suffix.  */
  int found_suffix = 0;

  ULONGEST high_bit;
  struct type *signed_type;
  struct type *unsigned_type;

  if (parsed_float)
    {
      /* If it ends at "df", "dd" or "dl", take it as type of decimal floating
         point.  Return DECFLOAT.  */

      if (len >= 2 && p[len - 2] == 'd' && p[len - 1] == 'f')
	{
	  p[len - 2] = '\0';
	  putithere->typed_val_decfloat.type
	    = parse_type->builtin_decfloat;
	  decimal_from_string (putithere->typed_val_decfloat.val, 4,
			       gdbarch_byte_order (parse_gdbarch), p);
	  p[len - 2] = 'd';
	  return DECFLOAT;
	}

      if (len >= 2 && p[len - 2] == 'd' && p[len - 1] == 'd')
	{
	  p[len - 2] = '\0';
	  putithere->typed_val_decfloat.type
	    = parse_type->builtin_decdouble;
	  decimal_from_string (putithere->typed_val_decfloat.val, 8,
			       gdbarch_byte_order (parse_gdbarch), p);
	  p[len - 2] = 'd';
	  return DECFLOAT;
	}

      if (len >= 2 && p[len - 2] == 'd' && p[len - 1] == 'l')
	{
	  p[len - 2] = '\0';
	  putithere->typed_val_decfloat.type
	    = parse_type->builtin_declong;
	  decimal_from_string (putithere->typed_val_decfloat.val, 16,
			       gdbarch_byte_order (parse_gdbarch), p);
	  p[len - 2] = 'd';
	  return DECFLOAT;
	}

      if (! parse_c_float (parse_gdbarch, p, len,
			   &putithere->typed_val_float.dval,
			   &putithere->typed_val_float.type))
	return ERROR;
      return FLOAT;
    }

  /* Handle base-switching prefixes 0x, 0t, 0d, 0 */
  if (p[0] == '0')
    switch (p[1])
      {
      case 'x':
      case 'X':
	if (len >= 3)
	  {
	    p += 2;
	    base = 16;
	    len -= 2;
	  }
	break;

      case 'b':
      case 'B':
	if (len >= 3)
	  {
	    p += 2;
	    base = 2;
	    len -= 2;
	  }
	break;

      case 't':
      case 'T':
      case 'd':
      case 'D':
	if (len >= 3)
	  {
	    p += 2;
	    base = 10;
	    len -= 2;
	  }
	break;

      default:
	base = 8;
	break;
      }

  while (len-- > 0)
    {
      c = *p++;
      if (c >= 'A' && c <= 'Z')
	c += 'a' - 'A';
      if (c != 'l' && c != 'u')
	n *= base;
      if (c >= '0' && c <= '9')
	{
	  if (found_suffix)
	    return ERROR;
	  n += i = c - '0';
	}
      else
	{
	  if (base > 10 && c >= 'a' && c <= 'f')
	    {
	      if (found_suffix)
		return ERROR;
	      n += i = c - 'a' + 10;
	    }
	  else if (c == 'l')
	    {
	      ++long_p;
	      found_suffix = 1;
	    }
	  else if (c == 'u')
	    {
	      unsigned_p = 1;
	      found_suffix = 1;
	    }
	  else
	    return ERROR;	/* Char not a digit */
	}
      if (i >= base)
	return ERROR;		/* Invalid digit in this base */

      /* Portably test for overflow (only works for nonzero values, so make
	 a second check for zero).  FIXME: Can't we just make n and prevn
	 unsigned and avoid this?  */
      if (c != 'l' && c != 'u' && (prevn >= n) && n != 0)
	unsigned_p = 1;		/* Try something unsigned */

      /* Portably test for unsigned overflow.
	 FIXME: This check is wrong; for example it doesn't find overflow
	 on 0x123456789 when LONGEST is 32 bits.  */
      if (c != 'l' && c != 'u' && n != 0)
	{	
	  if ((unsigned_p && (ULONGEST) prevn >= (ULONGEST) n))
	    error (_("Numeric constant too large."));
	}
      prevn = n;
    }

  /* An integer constant is an int, a long, or a long long.  An L
     suffix forces it to be long; an LL suffix forces it to be long
     long.  If not forced to a larger size, it gets the first type of
     the above that it fits in.  To figure out whether it fits, we
     shift it right and see whether anything remains.  Note that we
     can't shift sizeof (LONGEST) * HOST_CHAR_BIT bits or more in one
     operation, because many compilers will warn about such a shift
     (which always produces a zero result).  Sometimes gdbarch_int_bit
     or gdbarch_long_bit will be that big, sometimes not.  To deal with
     the case where it is we just always shift the value more than
     once, with fewer bits each time.  */

  un = (ULONGEST)n >> 2;
  if (long_p == 0
      && (un >> (gdbarch_int_bit (parse_gdbarch) - 2)) == 0)
    {
      high_bit = ((ULONGEST)1) << (gdbarch_int_bit (parse_gdbarch) - 1);

      /* A large decimal (not hex or octal) constant (between INT_MAX
	 and UINT_MAX) is a long or unsigned long, according to ANSI,
	 never an unsigned int, but this code treats it as unsigned
	 int.  This probably should be fixed.  GCC gives a warning on
	 such constants.  */

      unsigned_type = parse_type->builtin_unsigned_int;
      signed_type = parse_type->builtin_int;
    }
  else if (long_p <= 1
	   && (un >> (gdbarch_long_bit (parse_gdbarch) - 2)) == 0)
    {
      high_bit = ((ULONGEST)1) << (gdbarch_long_bit (parse_gdbarch) - 1);
      unsigned_type = parse_type->builtin_unsigned_long;
      signed_type = parse_type->builtin_long;
    }
  else
    {
      int shift;
      if (sizeof (ULONGEST) * HOST_CHAR_BIT 
	  < gdbarch_long_long_bit (parse_gdbarch))
	/* A long long does not fit in a LONGEST.  */
	shift = (sizeof (ULONGEST) * HOST_CHAR_BIT - 1);
      else
	shift = (gdbarch_long_long_bit (parse_gdbarch) - 1);
      high_bit = (ULONGEST) 1 << shift;
      unsigned_type = parse_type->builtin_unsigned_long_long;
      signed_type = parse_type->builtin_long_long;
    }

   putithere->typed_val_int.val = n;

   /* If the high bit of the worked out type is set then this number
      has to be unsigned. */

   if (unsigned_p || (n & high_bit)) 
     {
       putithere->typed_val_int.type = unsigned_type;
     }
   else 
     {
       putithere->typed_val_int.type = signed_type;
     }

   return INT;
}

/* Temporary obstack used for holding strings.  */
static struct obstack tempbuf;
static int tempbuf_init;

/* Parse a C escape sequence.  The initial backslash of the sequence
   is at (*PTR)[-1].  *PTR will be updated to point to just after the
   last character of the sequence.  If OUTPUT is not NULL, the
   translated form of the escape sequence will be written there.  If
   OUTPUT is NULL, no output is written and the call will only affect
   *PTR.  If an escape sequence is expressed in target bytes, then the
   entire sequence will simply be copied to OUTPUT.  Return 1 if any
   character was emitted, 0 otherwise.  */

int
c_parse_escape (char **ptr, struct obstack *output)
{
  char *tokptr = *ptr;
  int result = 1;

  /* Some escape sequences undergo character set conversion.  Those we
     translate here.  */
  switch (*tokptr)
    {
      /* Hex escapes do not undergo character set conversion, so keep
	 the escape sequence for later.  */
    case 'x':
      if (output)
	obstack_grow_str (output, "\\x");
      ++tokptr;
      if (!isxdigit (*tokptr))
	error (_("\\x escape without a following hex digit"));
      while (isxdigit (*tokptr))
	{
	  if (output)
	    obstack_1grow (output, *tokptr);
	  ++tokptr;
	}
      break;

      /* Octal escapes do not undergo character set conversion, so
	 keep the escape sequence for later.  */
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
      {
	int i;
	if (output)
	  obstack_grow_str (output, "\\");
	for (i = 0;
	     i < 3 && isdigit (*tokptr) && *tokptr != '8' && *tokptr != '9';
	     ++i)
	  {
	    if (output)
	      obstack_1grow (output, *tokptr);
	    ++tokptr;
	  }
      }
      break;

      /* We handle UCNs later.  We could handle them here, but that
	 would mean a spurious error in the case where the UCN could
	 be converted to the target charset but not the host
	 charset.  */
    case 'u':
    case 'U':
      {
	char c = *tokptr;
	int i, len = c == 'U' ? 8 : 4;
	if (output)
	  {
	    obstack_1grow (output, '\\');
	    obstack_1grow (output, *tokptr);
	  }
	++tokptr;
	if (!isxdigit (*tokptr))
	  error (_("\\%c escape without a following hex digit"), c);
	for (i = 0; i < len && isxdigit (*tokptr); ++i)
	  {
	    if (output)
	      obstack_1grow (output, *tokptr);
	    ++tokptr;
	  }
      }
      break;

      /* We must pass backslash through so that it does not
	 cause quoting during the second expansion.  */
    case '\\':
      if (output)
	obstack_grow_str (output, "\\\\");
      ++tokptr;
      break;

      /* Escapes which undergo conversion.  */
    case 'a':
      if (output)
	obstack_1grow (output, '\a');
      ++tokptr;
      break;
    case 'b':
      if (output)
	obstack_1grow (output, '\b');
      ++tokptr;
      break;
    case 'f':
      if (output)
	obstack_1grow (output, '\f');
      ++tokptr;
      break;
    case 'n':
      if (output)
	obstack_1grow (output, '\n');
      ++tokptr;
      break;
    case 'r':
      if (output)
	obstack_1grow (output, '\r');
      ++tokptr;
      break;
    case 't':
      if (output)
	obstack_1grow (output, '\t');
      ++tokptr;
      break;
    case 'v':
      if (output)
	obstack_1grow (output, '\v');
      ++tokptr;
      break;

      /* GCC extension.  */
    case 'e':
      if (output)
	obstack_1grow (output, HOST_ESCAPE_CHAR);
      ++tokptr;
      break;

      /* Backslash-newline expands to nothing at all.  */
    case '\n':
      ++tokptr;
      result = 0;
      break;

      /* A few escapes just expand to the character itself.  */
    case '\'':
    case '\"':
    case '?':
      /* GCC extensions.  */
    case '(':
    case '{':
    case '[':
    case '%':
      /* Unrecognized escapes turn into the character itself.  */
    default:
      if (output)
	obstack_1grow (output, *tokptr);
      ++tokptr;
      break;
    }
  *ptr = tokptr;
  return result;
}

/* Parse a string or character literal from TOKPTR.  The string or
   character may be wide or unicode.  *OUTPTR is set to just after the
   end of the literal in the input string.  The resulting token is
   stored in VALUE.  This returns a token value, either STRING or
   CHAR, depending on what was parsed.  *HOST_CHARS is set to the
   number of host characters in the literal.  */
static int
parse_string_or_char (char *tokptr, char **outptr, struct typed_stoken *value,
		      int *host_chars)
{
  int quote;
  enum c_string_type type;

  /* Build the gdb internal form of the input string in tempbuf.  Note
     that the buffer is null byte terminated *only* for the
     convenience of debugging gdb itself and printing the buffer
     contents when the buffer contains no embedded nulls.  Gdb does
     not depend upon the buffer being null byte terminated, it uses
     the length string instead.  This allows gdb to handle C strings
     (as well as strings in other languages) with embedded null
     bytes */

  if (!tempbuf_init)
    tempbuf_init = 1;
  else
    obstack_free (&tempbuf, NULL);
  obstack_init (&tempbuf);

  /* Record the string type.  */
  if (*tokptr == 'L')
    {
      type = C_WIDE_STRING;
      ++tokptr;
    }
  else if (*tokptr == 'u')
    {
      type = C_STRING_16;
      ++tokptr;
    }
  else if (*tokptr == 'U')
    {
      type = C_STRING_32;
      ++tokptr;
    }
  else
    type = C_STRING;

  /* Skip the quote.  */
  quote = *tokptr;
  if (quote == '\'')
    type |= C_CHAR;
  ++tokptr;

  *host_chars = 0;

  while (*tokptr)
    {
      char c = *tokptr;
      if (c == '\\')
	{
	  ++tokptr;
	  *host_chars += c_parse_escape (&tokptr, &tempbuf);
	}
      else if (c == quote)
	break;
      else
	{
	  obstack_1grow (&tempbuf, c);
	  ++tokptr;
	  /* FIXME: this does the wrong thing with multi-byte host
	     characters.  We could use mbrlen here, but that would
	     make "set host-charset" a bit less useful.  */
	  ++*host_chars;
	}
    }

  if (*tokptr != quote)
    {
      if (quote == '"')
	error (_("Unterminated string in expression."));
      else
	error (_("Unmatched single quote."));
    }
  ++tokptr;

  value->type = type;
  value->ptr = obstack_base (&tempbuf);
  value->length = obstack_object_size (&tempbuf);

  *outptr = tokptr;

  return quote == '"' ? STRING : CHAR;
}

struct token
{
  char *operator;
  int token;
  enum exp_opcode opcode;
  int cxx_only;
};

static const struct token tokentab3[] =
  {
    {">>=", ASSIGN_MODIFY, BINOP_RSH, 0},
    {"<<=", ASSIGN_MODIFY, BINOP_LSH, 0},
    {"->*", ARROW_STAR, BINOP_END, 1},
    {"...", DOTDOTDOT, BINOP_END, 0}
  };

static const struct token tokentab2[] =
  {
    {"+=", ASSIGN_MODIFY, BINOP_ADD, 0},
    {"-=", ASSIGN_MODIFY, BINOP_SUB, 0},
    {"*=", ASSIGN_MODIFY, BINOP_MUL, 0},
    {"/=", ASSIGN_MODIFY, BINOP_DIV, 0},
    {"%=", ASSIGN_MODIFY, BINOP_REM, 0},
    {"|=", ASSIGN_MODIFY, BINOP_BITWISE_IOR, 0},
    {"&=", ASSIGN_MODIFY, BINOP_BITWISE_AND, 0},
    {"^=", ASSIGN_MODIFY, BINOP_BITWISE_XOR, 0},
    {"++", INCREMENT, BINOP_END, 0},
    {"--", DECREMENT, BINOP_END, 0},
    {"->", ARROW, BINOP_END, 0},
    {"&&", ANDAND, BINOP_END, 0},
    {"||", OROR, BINOP_END, 0},
    /* "::" is *not* only C++: gdb overrides its meaning in several
       different ways, e.g., 'filename'::func, function::variable.  */
    {"::", COLONCOLON, BINOP_END, 0},
    {"<<", LSH, BINOP_END, 0},
    {">>", RSH, BINOP_END, 0},
    {"==", EQUAL, BINOP_END, 0},
    {"!=", NOTEQUAL, BINOP_END, 0},
    {"<=", LEQ, BINOP_END, 0},
    {">=", GEQ, BINOP_END, 0},
    {".*", DOT_STAR, BINOP_END, 1}
  };

/* Identifier-like tokens.  */
static const struct token ident_tokens[] =
  {
    {"unsigned", UNSIGNED, OP_NULL, 0},
    {"template", TEMPLATE, OP_NULL, 1},
    {"volatile", VOLATILE_KEYWORD, OP_NULL, 0},
    {"struct", STRUCT, OP_NULL, 0},
    {"signed", SIGNED_KEYWORD, OP_NULL, 0},
    {"sizeof", SIZEOF, OP_NULL, 0},
    {"double", DOUBLE_KEYWORD, OP_NULL, 0},
    {"false", FALSEKEYWORD, OP_NULL, 1},
    {"class", CLASS, OP_NULL, 1},
    {"union", UNION, OP_NULL, 0},
    {"short", SHORT, OP_NULL, 0},
    {"const", CONST_KEYWORD, OP_NULL, 0},
    {"enum", ENUM, OP_NULL, 0},
    {"long", LONG, OP_NULL, 0},
    {"true", TRUEKEYWORD, OP_NULL, 1},
    {"int", INT_KEYWORD, OP_NULL, 0},
    {"new", NEW, OP_NULL, 1},
    {"delete", DELETE, OP_NULL, 1},
    {"operator", OPERATOR, OP_NULL, 1},

    {"and", ANDAND, BINOP_END, 1},
    {"and_eq", ASSIGN_MODIFY, BINOP_BITWISE_AND, 1},
    {"bitand", '&', OP_NULL, 1},
    {"bitor", '|', OP_NULL, 1},
    {"compl", '~', OP_NULL, 1},
    {"not", '!', OP_NULL, 1},
    {"not_eq", NOTEQUAL, BINOP_END, 1},
    {"or", OROR, BINOP_END, 1},
    {"or_eq", ASSIGN_MODIFY, BINOP_BITWISE_IOR, 1},
    {"xor", '^', OP_NULL, 1},
    {"xor_eq", ASSIGN_MODIFY, BINOP_BITWISE_XOR, 1},

    {"const_cast", CONST_CAST, OP_NULL, 1 },
    {"dynamic_cast", DYNAMIC_CAST, OP_NULL, 1 },
    {"static_cast", STATIC_CAST, OP_NULL, 1 },
    {"reinterpret_cast", REINTERPRET_CAST, OP_NULL, 1 }
  };

/* When we find that lexptr (the global var defined in parse.c) is
   pointing at a macro invocation, we expand the invocation, and call
   scan_macro_expansion to save the old lexptr here and point lexptr
   into the expanded text.  When we reach the end of that, we call
   end_macro_expansion to pop back to the value we saved here.  The
   macro expansion code promises to return only fully-expanded text,
   so we don't need to "push" more than one level.

   This is disgusting, of course.  It would be cleaner to do all macro
   expansion beforehand, and then hand that to lexptr.  But we don't
   really know where the expression ends.  Remember, in a command like

     (gdb) break *ADDRESS if CONDITION

   we evaluate ADDRESS in the scope of the current frame, but we
   evaluate CONDITION in the scope of the breakpoint's location.  So
   it's simply wrong to try to macro-expand the whole thing at once.  */
static char *macro_original_text;

/* We save all intermediate macro expansions on this obstack for the
   duration of a single parse.  The expansion text may sometimes have
   to live past the end of the expansion, due to yacc lookahead.
   Rather than try to be clever about saving the data for a single
   token, we simply keep it all and delete it after parsing has
   completed.  */
static struct obstack expansion_obstack;

static void
scan_macro_expansion (char *expansion)
{
  char *copy;

  /* We'd better not be trying to push the stack twice.  */
  gdb_assert (! macro_original_text);

  /* Copy to the obstack, and then free the intermediate
     expansion.  */
  copy = obstack_copy0 (&expansion_obstack, expansion, strlen (expansion));
  xfree (expansion);

  /* Save the old lexptr value, so we can return to it when we're done
     parsing the expanded text.  */
  macro_original_text = lexptr;
  lexptr = copy;
}


static int
scanning_macro_expansion (void)
{
  return macro_original_text != 0;
}


static void 
finished_macro_expansion (void)
{
  /* There'd better be something to pop back to.  */
  gdb_assert (macro_original_text);

  /* Pop back to the original text.  */
  lexptr = macro_original_text;
  macro_original_text = 0;
}


static void
scan_macro_cleanup (void *dummy)
{
  if (macro_original_text)
    finished_macro_expansion ();

  obstack_free (&expansion_obstack, NULL);
}

/* Return true iff the token represents a C++ cast operator.  */

static int
is_cast_operator (const char *token, int len)
{
  return (! strncmp (token, "dynamic_cast", len)
	  || ! strncmp (token, "static_cast", len)
	  || ! strncmp (token, "reinterpret_cast", len)
	  || ! strncmp (token, "const_cast", len));
}

/* The scope used for macro expansion.  */
static struct macro_scope *expression_macro_scope;

/* This is set if a NAME token appeared at the very end of the input
   string, with no whitespace separating the name from the EOF.  This
   is used only when parsing to do field name completion.  */
static int saw_name_at_eof;

/* This is set if the previously-returned token was a structure
   operator -- either '.' or ARROW.  This is used only when parsing to
   do field name completion.  */
static int last_was_structop;

/* Read one token, getting characters through lexptr.  */

static int
lex_one_token (void)
{
  int c;
  int namelen;
  unsigned int i;
  char *tokstart;
  int saw_structop = last_was_structop;
  char *copy;

  last_was_structop = 0;

 retry:

  /* Check if this is a macro invocation that we need to expand.  */
  if (! scanning_macro_expansion ())
    {
      char *expanded = macro_expand_next (&lexptr,
                                          standard_macro_lookup,
                                          expression_macro_scope);

      if (expanded)
        scan_macro_expansion (expanded);
    }

  prev_lexptr = lexptr;

  tokstart = lexptr;
  /* See if it is a special token of length 3.  */
  for (i = 0; i < sizeof tokentab3 / sizeof tokentab3[0]; i++)
    if (strncmp (tokstart, tokentab3[i].operator, 3) == 0)
      {
	if (tokentab3[i].cxx_only
	    && parse_language->la_language != language_cplus)
	  break;

	lexptr += 3;
	yylval.opcode = tokentab3[i].opcode;
	return tokentab3[i].token;
      }

  /* See if it is a special token of length 2.  */
  for (i = 0; i < sizeof tokentab2 / sizeof tokentab2[0]; i++)
    if (strncmp (tokstart, tokentab2[i].operator, 2) == 0)
      {
	if (tokentab2[i].cxx_only
	    && parse_language->la_language != language_cplus)
	  break;

	lexptr += 2;
	yylval.opcode = tokentab2[i].opcode;
	if (in_parse_field && tokentab2[i].token == ARROW)
	  last_was_structop = 1;
	return tokentab2[i].token;
      }

  switch (c = *tokstart)
    {
    case 0:
      /* If we were just scanning the result of a macro expansion,
         then we need to resume scanning the original text.
	 If we're parsing for field name completion, and the previous
	 token allows such completion, return a COMPLETE token.
         Otherwise, we were already scanning the original text, and
         we're really done.  */
      if (scanning_macro_expansion ())
        {
          finished_macro_expansion ();
          goto retry;
        }
      else if (saw_name_at_eof)
	{
	  saw_name_at_eof = 0;
	  return COMPLETE;
	}
      else if (saw_structop)
	return COMPLETE;
      else
        return 0;

    case ' ':
    case '\t':
    case '\n':
      lexptr++;
      goto retry;

    case '[':
    case '(':
      paren_depth++;
      lexptr++;
      return c;

    case ']':
    case ')':
      if (paren_depth == 0)
	return 0;
      paren_depth--;
      lexptr++;
      return c;

    case ',':
      if (comma_terminates
          && paren_depth == 0
          && ! scanning_macro_expansion ())
	return 0;
      lexptr++;
      return c;

    case '.':
      /* Might be a floating point number.  */
      if (lexptr[1] < '0' || lexptr[1] > '9')
	{
	  if (in_parse_field)
	    last_was_structop = 1;
	  goto symbol;		/* Nope, must be a symbol. */
	}
      /* FALL THRU into number case.  */

    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      {
	/* It's a number.  */
	int got_dot = 0, got_e = 0, toktype;
	char *p = tokstart;
	int hex = input_radix > 10;

	if (c == '0' && (p[1] == 'x' || p[1] == 'X'))
	  {
	    p += 2;
	    hex = 1;
	  }
	else if (c == '0' && (p[1]=='t' || p[1]=='T' || p[1]=='d' || p[1]=='D'))
	  {
	    p += 2;
	    hex = 0;
	  }

	for (;; ++p)
	  {
	    /* This test includes !hex because 'e' is a valid hex digit
	       and thus does not indicate a floating point number when
	       the radix is hex.  */
	    if (!hex && !got_e && (*p == 'e' || *p == 'E'))
	      got_dot = got_e = 1;
	    /* This test does not include !hex, because a '.' always indicates
	       a decimal floating point number regardless of the radix.  */
	    else if (!got_dot && *p == '.')
	      got_dot = 1;
	    else if (got_e && (p[-1] == 'e' || p[-1] == 'E')
		     && (*p == '-' || *p == '+'))
	      /* This is the sign of the exponent, not the end of the
		 number.  */
	      continue;
	    /* We will take any letters or digits.  parse_number will
	       complain if past the radix, or if L or U are not final.  */
	    else if ((*p < '0' || *p > '9')
		     && ((*p < 'a' || *p > 'z')
				  && (*p < 'A' || *p > 'Z')))
	      break;
	  }
	toktype = parse_number (tokstart, p - tokstart, got_dot|got_e, &yylval);
        if (toktype == ERROR)
	  {
	    char *err_copy = (char *) alloca (p - tokstart + 1);

	    memcpy (err_copy, tokstart, p - tokstart);
	    err_copy[p - tokstart] = 0;
	    error (_("Invalid number \"%s\"."), err_copy);
	  }
	lexptr = p;
	return toktype;
      }

    case '@':
      {
	char *p = &tokstart[1];
	size_t len = strlen ("entry");

	while (isspace (*p))
	  p++;
	if (strncmp (p, "entry", len) == 0 && !isalnum (p[len])
	    && p[len] != '_')
	  {
	    lexptr = &p[len];
	    return ENTRY;
	  }
      }
      /* FALLTHRU */
    case '+':
    case '-':
    case '*':
    case '/':
    case '%':
    case '|':
    case '&':
    case '^':
    case '~':
    case '!':
    case '<':
    case '>':
    case '?':
    case ':':
    case '=':
    case '{':
    case '}':
    symbol:
      lexptr++;
      return c;

    case 'L':
    case 'u':
    case 'U':
      if (tokstart[1] != '"' && tokstart[1] != '\'')
	break;
      /* Fall through.  */
    case '\'':
    case '"':
      {
	int host_len;
	int result = parse_string_or_char (tokstart, &lexptr, &yylval.tsval,
					   &host_len);
	if (result == CHAR)
	  {
	    if (host_len == 0)
	      error (_("Empty character constant."));
	    else if (host_len > 2 && c == '\'')
	      {
		++tokstart;
		namelen = lexptr - tokstart - 1;
		goto tryname;
	      }
	    else if (host_len > 1)
	      error (_("Invalid character constant."));
	  }
	return result;
      }
    }

  if (!(c == '_' || c == '$'
	|| (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')))
    /* We must have come across a bad character (e.g. ';').  */
    error (_("Invalid character '%c' in expression."), c);

  /* It's a name.  See how long it is.  */
  namelen = 0;
  for (c = tokstart[namelen];
       (c == '_' || c == '$' || (c >= '0' && c <= '9')
	|| (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '<');)
    {
      /* Template parameter lists are part of the name.
	 FIXME: This mishandles `print $a<4&&$a>3'.  */

      if (c == '<')
	{
	  if (! is_cast_operator (tokstart, namelen))
	    {
	      /* Scan ahead to get rest of the template specification.  Note
		 that we look ahead only when the '<' adjoins non-whitespace
		 characters; for comparison expressions, e.g. "a < b > c",
		 there must be spaces before the '<', etc. */
               
	      char * p = find_template_name_end (tokstart + namelen);
	      if (p)
		namelen = p - tokstart;
	    }
	  break;
	}
      c = tokstart[++namelen];
    }

  /* The token "if" terminates the expression and is NOT removed from
     the input stream.  It doesn't count if it appears in the
     expansion of a macro.  */
  if (namelen == 2
      && tokstart[0] == 'i'
      && tokstart[1] == 'f'
      && ! scanning_macro_expansion ())
    {
      return 0;
    }

  /* For the same reason (breakpoint conditions), "thread N"
     terminates the expression.  "thread" could be an identifier, but
     an identifier is never followed by a number without intervening
     punctuation.  "task" is similar.  Handle abbreviations of these,
     similarly to breakpoint.c:find_condition_and_thread.  */
  if (namelen >= 1
      && (strncmp (tokstart, "thread", namelen) == 0
	  || strncmp (tokstart, "task", namelen) == 0)
      && (tokstart[namelen] == ' ' || tokstart[namelen] == '\t')
      && ! scanning_macro_expansion ())
    {
      char *p = tokstart + namelen + 1;
      while (*p == ' ' || *p == '\t')
	p++;
      if (*p >= '0' && *p <= '9')
	return 0;
    }

  lexptr += namelen;

  tryname:

  yylval.sval.ptr = tokstart;
  yylval.sval.length = namelen;

  /* Catch specific keywords.  */
  copy = copy_name (yylval.sval);
  for (i = 0; i < sizeof ident_tokens / sizeof ident_tokens[0]; i++)
    if (strcmp (copy, ident_tokens[i].operator) == 0)
      {
	if (ident_tokens[i].cxx_only
	    && parse_language->la_language != language_cplus)
	  break;

	/* It is ok to always set this, even though we don't always
	   strictly need to.  */
	yylval.opcode = ident_tokens[i].opcode;
	return ident_tokens[i].token;
      }

  if (*tokstart == '$')
    return VARIABLE;

  if (in_parse_field && *lexptr == '\0')
    saw_name_at_eof = 1;
  return NAME;
}

/* An object of this type is pushed on a FIFO by the "outer" lexer.  */
typedef struct
{
  int token;
  YYSTYPE value;
} token_and_value;

DEF_VEC_O (token_and_value);

/* A FIFO of tokens that have been read but not yet returned to the
   parser.  */
static VEC (token_and_value) *token_fifo;

/* Non-zero if the lexer should return tokens from the FIFO.  */
static int popping;

/* Temporary storage for c_lex; this holds symbol names as they are
   built up.  */
static struct obstack name_obstack;

/* Classify a NAME token.  The contents of the token are in `yylval'.
   Updates yylval and returns the new token type.  BLOCK is the block
   in which lookups start; this can be NULL to mean the global
   scope.  */
static int
classify_name (struct block *block)
{
  struct symbol *sym;
  char *copy;
  int is_a_field_of_this = 0;

  copy = copy_name (yylval.sval);

  sym = lookup_symbol (copy, block, VAR_DOMAIN, 
		       parse_language->la_language == language_cplus
		       ? &is_a_field_of_this : (int *) NULL);

  if (sym && SYMBOL_CLASS (sym) == LOC_BLOCK)
    {
      yylval.ssym.sym = sym;
      yylval.ssym.is_a_field_of_this = is_a_field_of_this;
      return BLOCKNAME;
    }
  else if (!sym)
    {
      /* See if it's a file name. */
      struct symtab *symtab;

      symtab = lookup_symtab (copy);
      if (symtab)
	{
	  yylval.bval = BLOCKVECTOR_BLOCK (BLOCKVECTOR (symtab), STATIC_BLOCK);
	  return FILENAME;
	}
    }

  if (sym && SYMBOL_CLASS (sym) == LOC_TYPEDEF)
    {
      yylval.tsym.type = SYMBOL_TYPE (sym);
      return TYPENAME;
    }

  yylval.tsym.type
    = language_lookup_primitive_type_by_name (parse_language,
					      parse_gdbarch, copy);
  if (yylval.tsym.type != NULL)
    return TYPENAME;

  /* Input names that aren't symbols but ARE valid hex numbers, when
     the input radix permits them, can be names or numbers depending
     on the parse.  Note we support radixes > 16 here.  */
  if (!sym
      && ((copy[0] >= 'a' && copy[0] < 'a' + input_radix - 10)
	  || (copy[0] >= 'A' && copy[0] < 'A' + input_radix - 10)))
    {
      YYSTYPE newlval;	/* Its value is ignored.  */
      int hextype = parse_number (copy, yylval.sval.length, 0, &newlval);
      if (hextype == INT)
	{
	  yylval.ssym.sym = sym;
	  yylval.ssym.is_a_field_of_this = is_a_field_of_this;
	  return NAME_OR_INT;
	}
    }

  /* Any other kind of symbol */
  yylval.ssym.sym = sym;
  yylval.ssym.is_a_field_of_this = is_a_field_of_this;

  if (sym == NULL
      && parse_language->la_language == language_cplus
      && !is_a_field_of_this
      && !lookup_minimal_symbol (copy, NULL, NULL))
    return UNKNOWN_CPP_NAME;

  return NAME;
}

/* Like classify_name, but used by the inner loop of the lexer, when a
   name might have already been seen.  FIRST_NAME is true if the token
   in `yylval' is the first component of a name, false otherwise.  */

static int
classify_inner_name (struct block *block, int first_name)
{
  struct type *type, *new_type;
  char *copy;

  if (first_name)
    return classify_name (block);

  type = check_typedef (yylval.tsym.type);
  if (TYPE_CODE (type) != TYPE_CODE_STRUCT
      && TYPE_CODE (type) != TYPE_CODE_UNION
      && TYPE_CODE (type) != TYPE_CODE_NAMESPACE)
    return ERROR;

  copy = copy_name (yylval.tsym.stoken);
  yylval.ssym.sym = cp_lookup_nested_symbol (yylval.tsym.type, copy, block);
  if (yylval.ssym.sym == NULL)
    return ERROR;

  switch (SYMBOL_CLASS (yylval.ssym.sym))
    {
    case LOC_BLOCK:
    case LOC_LABEL:
      return ERROR;

    case LOC_TYPEDEF:
      yylval.tsym.type = SYMBOL_TYPE (yylval.ssym.sym);;
      return TYPENAME;

    default:
      yylval.ssym.is_a_field_of_this = 0;
      return NAME;
    }
  internal_error (__FILE__, __LINE__, _("not reached"));
}

/* The outer level of a two-level lexer.  This calls the inner lexer
   to return tokens.  It then either returns these tokens, or
   aggregates them into a larger token.  This lets us work around a
   problem in our parsing approach, where the parser could not
   distinguish between qualified names and qualified types at the
   right point.
   
   This approach is still not ideal, because it mishandles template
   types.  See the comment in lex_one_token for an example.  However,
   this is still an improvement over the earlier approach, and will
   suffice until we move to better parsing technology.  */
static int
yylex (void)
{
  token_and_value current;
  int first_was_coloncolon, last_was_coloncolon, first_iter;

  if (popping && !VEC_empty (token_and_value, token_fifo))
    {
      token_and_value tv = *VEC_index (token_and_value, token_fifo, 0);
      VEC_ordered_remove (token_and_value, token_fifo, 0);
      yylval = tv.value;
      return tv.token;
    }
  popping = 0;

  current.token = lex_one_token ();
  if (current.token == NAME)
    current.token = classify_name (expression_context_block);
  if (parse_language->la_language != language_cplus
      || (current.token != TYPENAME && current.token != COLONCOLON))
    return current.token;

  first_was_coloncolon = current.token == COLONCOLON;
  last_was_coloncolon = first_was_coloncolon;
  obstack_free (&name_obstack, obstack_base (&name_obstack));
  if (!last_was_coloncolon)
    obstack_grow (&name_obstack, yylval.sval.ptr, yylval.sval.length);
  current.value = yylval;
  first_iter = 1;
  while (1)
    {
      token_and_value next;

      next.token = lex_one_token ();
      next.value = yylval;

      if (next.token == NAME && last_was_coloncolon)
	{
	  int classification;

	  classification = classify_inner_name (first_was_coloncolon
						? NULL
						: expression_context_block,
						first_iter);
	  /* We keep going until we either run out of names, or until
	     we have a qualified name which is not a type.  */
	  if (classification != TYPENAME && classification != NAME)
	    {
	      /* Push the final component and leave the loop.  */
	      VEC_safe_push (token_and_value, token_fifo, &next);
	      break;
	    }

	  /* Update the partial name we are constructing.  */
	  if (!first_iter)
	    {
	      /* We don't want to put a leading "::" into the name.  */
	      obstack_grow_str (&name_obstack, "::");
	    }
	  obstack_grow (&name_obstack, next.value.sval.ptr,
			next.value.sval.length);

	  yylval.sval.ptr = obstack_base (&name_obstack);
	  yylval.sval.length = obstack_object_size (&name_obstack);
	  current.value = yylval;
	  current.token = classification;

	  last_was_coloncolon = 0;
	}
      else if (next.token == COLONCOLON && !last_was_coloncolon)
	last_was_coloncolon = 1;
      else
	{
	  /* We've reached the end of the name.  */
	  VEC_safe_push (token_and_value, token_fifo, &next);
	  break;
	}

      first_iter = 0;
    }

  popping = 1;

  /* If we ended with a "::", insert it too.  */
  if (last_was_coloncolon)
    {
      token_and_value cc;
      memset (&cc, 0, sizeof (token_and_value));
      if (first_was_coloncolon && first_iter)
	{
	  yylval = cc.value;
	  return COLONCOLON;
	}
      cc.token = COLONCOLON;
      VEC_safe_insert (token_and_value, token_fifo, 0, &cc);
    }

  yylval = current.value;
  yylval.sval.ptr = obstack_copy0 (&expansion_obstack,
				   yylval.sval.ptr,
				   yylval.sval.length);
  return current.token;
}

int
c_parse (void)
{
  int result;
  struct cleanup *back_to = make_cleanup (free_current_contents,
					  &expression_macro_scope);

  /* Set up the scope for macro expansion.  */
  expression_macro_scope = NULL;

  if (expression_context_block)
    expression_macro_scope
      = sal_macro_scope (find_pc_line (expression_context_pc, 0));
  else
    expression_macro_scope = default_macro_scope ();
  if (! expression_macro_scope)
    expression_macro_scope = user_macro_scope ();

  /* Initialize macro expansion code.  */
  obstack_init (&expansion_obstack);
  gdb_assert (! macro_original_text);
  make_cleanup (scan_macro_cleanup, 0);

  make_cleanup_restore_integer (&yydebug);
  yydebug = parser_debug;

  /* Initialize some state used by the lexer.  */
  last_was_structop = 0;
  saw_name_at_eof = 0;

  VEC_free (token_and_value, token_fifo);
  popping = 0;
  obstack_init (&name_obstack);
  make_cleanup_obstack_free (&name_obstack);

  result = yyparse ();
  do_cleanups (back_to);
  return result;
}


void
yyerror (char *msg)
{
  if (prev_lexptr)
    lexptr = prev_lexptr;

  error (_("A %s in expression, near `%s'."), (msg ? msg : "error"), lexptr);
}


