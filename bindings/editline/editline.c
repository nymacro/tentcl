#include <stdio.h>
#include <stdlib.h>
#include <editline.h>
#include "../../src/tcl.h"

TclReturn Tcl_readline(Tcl *vm, int argc, TclValue argv[], TclValue *ret) {
    char *str = readline((argc >= 2) ? argv[1] : "");
    if (str) {
        TclValue_set(ret, str);
        free(str);
    }
    return TCL_OK;
}

TclReturn Tcl_addhistory(Tcl *vm, int argc, TclValue argv[], TclValue *ret) {
    if (argc != 2) {
        return TCL_EXCEPTION;
    }
    add_history(argv[1]);
    return TCL_OK;
}

void Tcl_registerLibrary(Tcl *vm) {
    Tcl_register(vm, "readline", Tcl_readline);
    Tcl_register(vm, "addhistory", Tcl_addhistory);
}
