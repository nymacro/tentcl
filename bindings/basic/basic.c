#include <stdio.h>
#include "../../src/tcl.h"

TclReturn Tcl_test(Tcl *vm, int argc, TclValue argv[], TclValue *ret) {
    int i;
    printf("Hello there!");
    printf("You provided %i arguments, they were:\n", argc - 1);
    for (i = 1; i < argc; i++) {
        printf("%s\n", argv[i]);
    }
    TclValue_new(ret, "wow. it really works!!");
    return TCL_OK;
}

void Tcl_registerLibrary(Tcl *vm) {
    printf("registering dynamic library 'basic'\n");
    Tcl_register(vm, "test", Tcl_test);
}
