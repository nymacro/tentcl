/*
 * Tentcl -- Shell
 * Copyright (C) 2006-2015 Aaron Marks. All Rights Reserved.
 */

#ifndef TENTCL_REPL_H

#include "LineRead.h"
#include "tcl.h"

TclReturn TclRepl_repl(Tcl *vm, FILE *input);

#endif
