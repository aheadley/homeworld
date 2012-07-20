/* ==========================================================================

  --- Header: eval.h

  --- Purpose: interface to eval.c

  --- Developed with: Pentium PC, Linux 1.2.13, gcc 2.7.0.

  --- Author: Guido Gonzato, Via Monte Ortigara 19/a, 37127 Verona, Italy;
              Tel. +39-45-8345513; Email: guido@ibogfs.cineca.it

  --- Last updated: 15 November 1996

========================================================================== */

#ifndef ___EVAL_H
#define ___EVAL_H
/* types */

typedef enum { ALL_OK, DIV_ZERO, RANGE, SYNTAX } ERR_TYPE;

/* functions */

ERR_TYPE evalSyntaxOK(char *);
double evalEvaluate(char *str, ERR_TYPE *);
char *evalErrorString(ERR_TYPE type);
char *evalNum2Str(double n, char *s);

/* --- End of file eval.h --- */

#endif //___EVAL_H

