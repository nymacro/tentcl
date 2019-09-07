/*
 * Tentcl -- Standard Function Library
 * Copyright (C) 2006-2018 Aaron Marks. All Rights Reserved.
 */
#ifndef TENTCL_STD_H
#define TENTCL_STD_H

#include "tcl.h"

struct TclUserFunction {
    List *args;
    TclValue *code;
};
typedef struct TclUserFunction TclUserFunction;

void TclStd_register(Tcl*);

TclReturn TclStd_userFuncall(Tcl*, int, TclValue*[], TclValue*);
TclReturn TclStd_eval(Tcl *vm, int argc, TclValue *argv[], TclValue *ret);

#endif
