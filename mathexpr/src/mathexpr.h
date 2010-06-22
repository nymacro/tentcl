/*
 * mathexpr -- Mathematical Expression Evaluator
 * Copyright (C) 2008 Aaron Marks
 */
#ifndef MATHEXPR_MATHEXPR_H
#define MATHEXPR_MATHEXPR_H

typedef int MATHEXPR_T;

#define MATHEXPR_FUNCTIONS

#ifdef MATHEXPR_FUNCTIONS
#define MATHEXPR_FUNCTIONS_COUNT    32
typedef MATHEXPR_T (*MathExprFunction)(int argc, MATHEXPR_T argv[]);

struct MathFunction {
    char *name;
    MathExprFunction func;
};
typedef struct MathFunction MathFunction;

#endif

struct MathState {
#ifdef MATHEXPR_FUNCTIONS
    int functionCount;
    MathFunction functions[MATHEXPR_FUNCTIONS_COUNT];
#endif
    char *buffer;   /* expression buffer */
    char *p;        /* current string pointer */
};
typedef struct MathState MathState;

MATHEXPR_T Math_eval(char *expr);
#ifdef MATHEXPR_FUNCTIONS
void Math_addFunction(MathState *state, char *name, MathExprFunction func);
#endif

#endif

