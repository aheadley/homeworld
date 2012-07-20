/* ==========================================================================

  --- Module: eval.c

  --- Purpose: evaluates formulas in strings: e.g. "1+1" -> 2.

  --- Developed with: Pentium PC, Linux 1.2.13, gcc 2.7.0.

  --- Author: Guido Gonzato, Via Monte Ortigara 19/a, 37127 Verona, Italy;
              Tel. +39-45-8345513; e-mail: guido@ibogfs.cineca.it

  --- Last updated: 9 December 1996.

========================================================================== */

#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
//#include <values.h>
#include <stdlib.h>
#include <time.h>
#include "Types.h"
#include "Randy.h"

#include "Eval.h"

#define  strequ(s1, s2) !strcmp(s1, s2)
#define  nelems(arr) (sizeof(arr) / sizeof(arr[0]))
#define  TOKLEN  256

/* these definitions are used by bsearch()---see forward */

typedef int (*ftpr)(const void *, const void *);

typedef struct {
  int order;
  char s[6];
} FUNC_ITEM;

/* these are the possible tokens (objects): */

typedef enum

{ PLUS, MINUS, MULT, DIV, OPEN_PAR, CLOSED_PAR, COMMA, DIGIT,
  LT, LE, GT, GE, EQ, NEQ,
  BEGIN_FUNC,
  ACOS, ASIN, ATAN, ATAN2, CEIL, COS, COSH, EXP, FABS, FACT, FLOOR, FMOD,
  LDEXP, LOG, LOG10, _PI, POW, RAND, RND, SIN, SINH, SQR, SQRT, TAN, TANH,
  _ERROR, EOL } TOKEN;

/* to add more functions: add the function name above and in func_vector[]
   below, implement the function and add a reference to it in function
   factor(), then add the grammatical check in function check_fact() */

/* ----- */

static char      *s;                /* local string         */
static TOKEN     cur_token;         /* last token read      */
static char      str_token[TOKLEN]; /* last token as string */
static double    val_token;         /* numerical value      */
static int       pc;                /* string counter       */
static ERR_TYPE  exiterr;           /* error occurred       */
static FUNC_ITEM func_vector[] =    /* must be in alphabetical order */
{
    { 1,  "ACOS"  },
    { 2,  "ASIN"  },
    { 3,  "ATAN"  },
    { 4,  "ATAN2" },
    { 5,  "CEIL"  },
    { 6,  "COS"   },
    { 7,  "COSH"  },
    { 8,  "EXP"   },
    { 9,  "FABS"  },
    { 10, "FACT"  },
    { 11, "FLOOR" },
    { 12, "FMOD"  },
    { 13, "LDEXP" },
    { 14, "LOG"   },
    { 15, "LOG10" },
    { 16, "PI"    },
    { 17, "POW"   },
    { 18, "RAND"  },
    { 19, "RND"   },
    { 20, "SIN"   },
    { 21, "SINH"  },
    { 22, "SQR"   },
    { 23, "SQRT"  },
    { 24, "TAN"   },
    { 25, "TANH"  }
};

/* ----- */

double fact(int n)
{

  int    i;
  double f = 1;

  for (i = 1; i <= n; i++)
    f *= i;

  return(f);

} /* fact() */

/* ----- */

static void init_str(char *eval_str)
/* Initialize s with eval_str. */
{

  pc = 0;
  s = eval_str;

  for ( ; (*eval_str = toupper(*eval_str)); eval_str++)
    ;
/*  _strupr(eval_str); */

} /* init_str() */

/* ----- */

static void math_err(ERR_TYPE err)
{

  cur_token = _ERROR;
  exiterr = err;

} /* math_err() */

/* ----- */

static int lesser(const FUNC_ITEM *a, const FUNC_ITEM *b)
{

  return( strcmp(a->s, b->s) );

} /* lesser() */

/* ----- */

static FUNC_ITEM *lookup(FUNC_ITEM key)
{

  FUNC_ITEM *itemptr;

  itemptr = (FUNC_ITEM *) bsearch (&key, func_vector, nelems(func_vector),
            sizeof(func_vector[0]), (ftpr) lesser);

  return itemptr;

} /* lookup() */

/* ----- */

static void read_token()
/* Read the next token from the input string. The last token read is
   cur_token, while str_token is the token as a string. For example, reading
   from the string the number 12.3371, cur_token is DIGIT, str_token is
   "12.3371", val_token is 12.3371. */
{

//  TOKEN       tok;   /* for searching */
  int         i;     /* for loop      */
  char        *err;  /* for strtod()  */
  FUNC_ITEM   f,
              *ptr;

  /* skip blanks */

  while (isspace(s[pc]))
    pc++;

  if (isalpha(s[pc])) {   /* it may be a function */

    strcpy(str_token, "");
    i = 0;
    while ( (isalnum(s[pc])) && (i < TOKLEN) )  /* !!! NOT isalpha() */
      str_token[i++] = s[pc++];
    str_token[i] = '\0';

    /* check whether it is a function */

    f.order = 0; /* dummy */
    strcpy(f.s, str_token);

    if ( (ptr = lookup(f)) != NULL )
      cur_token = BEGIN_FUNC + ptr->order;
    else
      math_err(SYNTAX);

    return;

  } /* if isalpha() */

  if (isdigit(s[pc]) || s[pc] == '.') {  /* it's a number */

    strcpy(str_token, "");
    i = 0;
    while ( (isdigit(s[pc]) || s[pc] == '.') && i < TOKLEN )
      str_token[i++] = s[pc++];
    if (s[pc] == 'E') {
      str_token[i++] = s[pc++];
      if (s[pc] == '+' || s[pc] == '-')
        str_token[i++] = s[pc++];
      while (isdigit(s[pc]) && i < TOKLEN )
        str_token[i++] = s[pc++];
    }

/*****
    while ( (isdigit(s[pc]) || s[pc] == '.' || (s[pc] == 'E') ||
            (s[pc] == '+') || (s[pc] == '-')) && (i < TOKLEN) )
      str_token[i++] = s[pc++];
*****/

    str_token[i] = '\0';

    val_token = strtod(str_token, &err);

    if (*err == '\0')
      cur_token = DIGIT;
    else
      math_err(SYNTAX);

    return;

  } /* if isdigit() */

  /* other chars */

  switch (s[pc]) {

    case '+':   cur_token = PLUS;
                break;

    case '-':   cur_token = MINUS;
                break;

    case '*':   cur_token = MULT;
                break;

    case '/':   cur_token = DIV;
                break;

    case '(':   cur_token = OPEN_PAR;
                break;

    case ')':   cur_token = CLOSED_PAR;
                break;

    case ',':   cur_token = COMMA;
                break;

    case '<':   if (s[pc + 1] == '=') {
                  cur_token = LE;
                  pc++;
                }
                else
                  cur_token = LT;
                break;

    case '>':   if (s[pc + 1] == '=') {
                  cur_token = GE;
                  pc++;
                }
                else
                  cur_token = GT;
                break;

    case '=':   if (s[pc + 1] == '=') {
                  cur_token = EQ;
                  pc++;
                }
                else {
                  math_err(SYNTAX);
                  return;
                }
                break;

    case '!':   if (s[pc + 1] == '=') {
                  cur_token = NEQ;
                  pc++;
                }
                else {
                  math_err(SYNTAX);
                  return;
                }
                break;

    case '\0':  cur_token = EOL;
                break;

    default:    math_err(SYNTAX);
                break;

  } /* switch *pc */

  pc++;

} /* read_token() */

/* ----- */

static double expression();
static double multiply();
static double factor();

/* ----- */

static double logical_expression()
{

  double e1, e2;
  TOKEN  opr;

  e1 = expression();

  if (cur_token == _ERROR)
    return(0.0);

  while (cur_token == LT || cur_token == LE ||
         cur_token == GT || cur_token == GE ||
         cur_token == EQ || cur_token == NEQ) {

    opr = cur_token;
    read_token();
    e2 = expression();
    if (cur_token == _ERROR)
      return(0.0);

    switch (opr) {

      case  LT:   e1 = (e1 < e2);
                  break;

      case  LE:   e1 = (e1 <= e2);
                  break;

      case  GT:   e1 = (e1 > e2);
                  break;

      case  GE:   e1 = (e1 >= e2);
                  break;

      case  EQ:   e1 = (e1 == e2);
                  break;

      case  NEQ:  e1 = (e1 != e2);
                  break;
      default:
            break;

    } /* switch */

  } /* while */

  return(e1);

} /* logical_expression() */

/* ----- */

static double expression()
/* This function evaluates expressions. In case of an error, it aborts
   without looking for possible other errors. In order to check ALL the
   possible errors (e.g. syntax), use evalSyntaxOK(). */
{

  double   p, e;    /* for partial results */
  TOKEN    opr;     /* either + or - */

  /* cur_token is already initialised */

  if (cur_token == MINUS) {
    read_token();
    e = -multiply();
  }
  else
    e = multiply();

  if (cur_token == _ERROR)
    return(0.0);

  while (cur_token == PLUS || cur_token == MINUS) {

    opr = cur_token;
    read_token();
    p = multiply();
    if (cur_token == _ERROR)
      return(0.0);

    switch (opr) {
      case PLUS:  e += p;
                  break;
      case MINUS: e -= p;
      default:	break;
    } /* switch */

  } /* while */

  return(e);

} /* expression() */

/* ----- */

static double multiply()
/* Evaluate * and / */
{

  double  p1, p2, temp;
  TOKEN   opr;    /* either * or / */

  p1 = factor();
  if (cur_token == _ERROR)
    return(0.0);

  while (cur_token == MULT || cur_token == DIV) {
    opr = cur_token;
    read_token();
    p2 = factor();
    if (cur_token == _ERROR)
      return(0.0);

    switch (opr) {
      case  MULT: p1 *= p2;
                  break;
      case  DIV:  /* prevent division by zero */
                  temp = p2;
                  if (temp != 0.0)
                    p1 /= temp;
                  else {
                    math_err(DIV_ZERO);
                    return(0.0);
                  }
        default: break;
    } /* switch */

  } /* while */

  return (p1);

} /* multiply() */

/* ----- */

static double factor()
/* Return a factor */
{

  double  f, f2, tmp;
  TOKEN   func;
//  char    *err;

  /* I think I've discovered a bug in Turbo C++ 3.0. It strikes here in
     factor() when calling functions that can cause a RANGE error, like
     ln() or sqrt(). This program compiled with Turbo C++ will crash when
     the user enters something like sqrt(-1) or ln(0), but if compiled by
     gcc the error is trapped correctly. */

  if (cur_token == EOL || cur_token == _ERROR) {
  /* factor() must find something valid */
    cur_token = _ERROR;
    if (cur_token == EOL)
      exiterr = SYNTAX;
    return(0.0);
  }

  switch (cur_token) {

    case DIGIT:     f = val_token;
                    break;

    case OPEN_PAR:  read_token();
                    f = expression();
                    if (cur_token == _ERROR)
                      return(0.0);
                    if (cur_token != CLOSED_PAR) {
                      math_err(SYNTAX);
                      return(0.0);
                    }
                    break;

    case _PI:       f = 3.14159265359789;
                    break;

    case ATAN2:
    case FMOD:
    case LDEXP:
    case POW:       func = cur_token;
                    read_token();  /* look for "(" */
                    if (cur_token != OPEN_PAR) {
                      math_err(SYNTAX);
                      return(0.0);
                    }
                    read_token();
                    f = expression();
                    if (cur_token == _ERROR)
                      return(0.0);
                    if (cur_token != COMMA) {
                      math_err(SYNTAX);
                      return(0.0);
                    }
                    read_token();
                    f2 = expression();
                    if (cur_token == _ERROR)
                      return(0.0);

                    switch(func) {

                      case ATAN2: f = atan2(f, f2);
                                  break;

                      case FMOD:  f = fmod(f, f2);
                                  break;

                      case LDEXP: f = ldexp(f, (int) f2);
                                  break; /* !!! add HUGE_VAL condition */

                      case POW:   if (f == 0.0 && f2 <= 0.0) {
                                    math_err(RANGE);
                                    return(0.0);
                                  }
                                  f2 = modf(f2, &tmp);
                                  if (f < 0.0 && tmp != 0.0) {
                                    math_err(RANGE);
                                    return(0.0);
                                  }
                                  f = pow(f, tmp);
                                  break;
                        default: break;

                    } /* switch func */

                    if (cur_token != CLOSED_PAR) {
                      math_err(SYNTAX);
                      return(0.0);
                    }
                    break;

                    if (cur_token != CLOSED_PAR) {
                      math_err(SYNTAX);
                      return(0.0);
                    }
                    break;

    case ACOS:
    case ASIN:
    case ATAN:
    case CEIL:
    case COS:
    case COSH:
    case EXP:
    case FABS:
    case FACT:
    case FLOOR:
    case LOG:
    case LOG10:
    case RAND:
    case RND:
    case SIN:
    case SINH:
    case SQR:
    case SQRT:
    case TAN:
    case TANH:      func = cur_token;
                    read_token();  /* look for "(" */
                    if (cur_token != OPEN_PAR) {
                      math_err(SYNTAX);
                      return(0.0);
                    }
                    read_token();
                    f = expression();
                    if (cur_token == _ERROR)
                      return(0.0);

                    switch (func) {

                      case ACOS:   if ( fabs(f) <= 1)
                                     f = acos(f);
                                   else {
                                     math_err(RANGE);
                                     return(0.0);
                                   }
                                   break;

                      case ASIN:   if ( fabs(f) <= 1)
                                     f = asin(f);
                                   else {
                                     math_err(RANGE);
                                     return(0.0);
                                   }
                                   break;

                      case ATAN:   f = atan(f);
                                   break;

                      case CEIL:   f = ceil(f);
                                   break;

                      case COS:    f = cos(f);
                                   break;

                      case COSH:   f = cosh(f);
                                   break;

                      case EXP:    f = exp(f);
                                   break;

                      case FABS:   f = fabs(f);
                                   break;

                      case FACT:   if (f > 0.0)
                                     f = fact((int) f);
                                   else {
                                     math_err(RANGE);
                                     return(0.0);
                                   }
                                   break;

                      case FLOOR:  f = floor(f);
                                   break;

                      case LOG:    if (f > 0.0)
                                     f = log(f);
                                   else {
                                     math_err(RANGE);
                                     return(0.0);
                                   }
                                   break;

                      case LOG10:  if (f > 0.0)
                                     f = log10(f);
                                   else {
                                     math_err(RANGE);
                                     return(0.0);
                                   }
                                   break;

                      case RAND:
                                   f = 1 + (int) ( f * ranRandom(RAN_ETG)/(double)(UDWORD_Max));
                                   break;

                      case RND:
                                   f *= (double) ranRandom(RAN_ETG) / (double)UDWORD_Max;
                                   break;

                      case SIN:    f = sin(f);
                                   break;

                      case SINH:   f = sinh(f);
                                   break;

                      case SQR:    f *= f;
                                   break;

                      case SQRT:   if (f >= 0)
                                     f = sqrt(f);
                                   else {
                                     math_err(RANGE);
                                     return(0.0);
                                   }
                                   break;

                      case TAN:    if (cos(f) != 0.0)
                                     f = tan(f);
                                   else {
                                     math_err(RANGE);
                                     return(0.0);
                                   }
                                   break;

                      case TANH:   f = tanh(f);
                                   break;

                           /* ...add more functions here... */
                        default:
                            break;
                    } /* switch func */

                    if (cur_token != CLOSED_PAR) {
                      math_err(SYNTAX);
                      return(0.0);
                    }
                    break;

    default:        math_err(SYNTAX);
                    return(0.0);

  } /* switch cur_token */

  read_token();
  return(f);

  /* I tried to debug. That strange error strikes HERE - exiting factor() */

} /* factor() */

/* ----- */

double evalEvaluate(char *str, ERR_TYPE *err)
/* This function calculates the expression in str, stores the result in v,
   and returns an exit code. */
{

  double e;

  exiterr = ALL_OK;  /* in the end we should find it unchanged */
  init_str(str);

  /* in case of an error, logical_expression() aborts */

  read_token();
  e = logical_expression();

  if ( (exiterr == ALL_OK) && (cur_token != EOL) )
    *err = SYNTAX;
  else
    *err = exiterr;

  return e;

} /* evalEvaluate() */

/* ----- */

/* Functions for checking the syntax of input string */

static void check_expr();
static void check_mult();
static void check_fact();

/* ----- */

void check_log_expr()
{

  check_expr();
  if (exiterr == SYNTAX)
    return;

  while (cur_token == LT || cur_token == LE ||
         cur_token == GT || cur_token == GE ||
         cur_token == EQ || cur_token == NEQ) {
    read_token();
    check_expr();
    if (exiterr == SYNTAX)
      return;
  }

} /* --- check_log_expr() --- */

/* ----- */

void check_expr()
/* Used to verify the syntax in str */
{

  /* When check_expr starts, cur_token has already been set */

  if (cur_token == MINUS)
    read_token();
  check_mult();
  if (exiterr == SYNTAX)
    return;

  while (cur_token == PLUS || cur_token == MINUS) {
    read_token();
    check_mult();
    if (exiterr == SYNTAX)
      return;
  }

} /* check_expr() */

/* ----- */

static void check_mult()
/* Check * and / */
{

  check_fact();
  if (exiterr == SYNTAX)
    return;

  while (cur_token == MULT || cur_token == DIV) {
    read_token();
    check_fact();
    if (exiterr == SYNTAX)
      return;
  }

} /* check_mult() */

/* ----- */

static void check_fact()
{

  if (cur_token == EOL) {
    exiterr = SYNTAX;
    return;
  }

  switch (cur_token) {

    case DIGIT:
    case _PI:       break; /* do nothing */

    case OPEN_PAR:  read_token();
                    check_expr();
                    if (exiterr == SYNTAX)
                      return;
                    if (cur_token != CLOSED_PAR) {
                      exiterr = SYNTAX;
                      return;
                    }
                    break;  /* OPEN_PAR */

    case ATAN2:
    case FMOD:
    case LDEXP:
    case POW:       read_token();
                    if (cur_token != OPEN_PAR) {
                      exiterr = SYNTAX;
                      return;
                    }
                    read_token();
                    check_expr();
                    if (exiterr == SYNTAX)
                      return;
                    if (cur_token != COMMA) {
                      exiterr = SYNTAX;
                      return;
                    }
                    read_token();
                    check_expr();
                    if (exiterr == SYNTAX)
                      return;
                    if (cur_token != CLOSED_PAR) {
                      exiterr = SYNTAX;
                      return;
                    }
                    break;  /* ATAN2 to POW */

    case ACOS:
    case ASIN:
    case ATAN:
    case CEIL:
    case COS:
    case COSH:
    case EXP:
    case FABS:
    case FACT:
    case FLOOR:
    case LOG:
    case LOG10:
    case RAND:
    case RND:
    case SIN:
    case SINH:
    case SQR:
    case SQRT:
    case TAN:
    case TANH:      read_token();
                    if (cur_token != OPEN_PAR) {
                      exiterr = SYNTAX;
                      return;
                    }
                    read_token();
                    check_expr();
                    if (exiterr == SYNTAX)
                      return;
                    if (cur_token != CLOSED_PAR) {
                      exiterr = SYNTAX;
                      return;
                    }
                    break;  /* ACOS to TANH */

    default:        exiterr = SYNTAX;
                    return;
                    break;

  } /* switch (cur_token) */

  read_token();

} /* check_fact() */

/* ----- */

ERR_TYPE evalSyntaxOK(char *str)
/* Return OK if the expression in str is syntaxically correct, NOT_OK
   otherwise. */
{

  exiterr = ALL_OK;  /* if everything goes OK, exiterr will stay ALL_OK */
  init_str(str);
  read_token();
  check_log_expr();

  return(exiterr);

} /* evalSyntaxOK() */

/* ----- */

char *evalErrorStringTable[] =
{
    "OK",
    "Divide by zero",
    "Range",
    "Syntax"
};
//return a string for an error code
char *evalErrorString(ERR_TYPE type)
{
    if (type > SYNTAX || type < 0)
    {
        return("Unknown");
    }
    return(evalErrorStringTable[type]);
}
char *strdel(char *s, int pos, int num)
/* Delete num characters from string s, starting from pos. Equivalent to
   Turbo Pascal's Delete(). */
{

  int i;

  for (i = pos; s[i + num] != '\0'; i++)
    s[i] = s[i + num];

  s[i] = '\0';
  return(s);

} /* strdel() */

/* ----- */
//return a beautified string for an expression
char *evalNum2Str(double n, char *s)
/* From number to string using the shortest possible format */
{

  int i;

  if (n > 1E12)   /* keep exponential format */
    sprintf(s, "%f", n);
  else {
    sprintf(s, "%12.12f", n);
    for (i = strlen(s) - 1; s[i] == '0'; i--)
      strdel(s, i, 1);
    if (s[i] == '.')
      strdel(s, i, 1);
  }

  return(s);

} /* num2str() */

/* --- End of file eval.c --- */
