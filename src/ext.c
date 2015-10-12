/*
 * Tentcl -- Extension Function Library
 * Copyright (C) 2006-2015 Aaron Marks. All Rights Reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include "ext.h"
#include "std.h"
#ifdef WITH_LIBRARIES
#include <dlfcn.h>
#endif

#include "common.h"

static List lambdas;
static List dlls;

// THIS FUNCTION CONFLICTS WITH STANDARD TCL COMMAND
// Output various information to stdout
/*tcl: info
 * Prints out an information summary of the program state.
 */
TclReturn TclStd_info(Tcl *vm, int argc, TclValue argv[], TclValue *ret) {
    printf(TENTCL_VERSION "\n");
    /*printf("Variables in existence:  %i\n", List_size(vm->variables->list));*/
    /*printf("Commands in existence:   %i\n", List_size(vm->functions.list));*/
    printf("Current namespace depth: %i\n", List_size(vm->namespace));
    /*printf("Open files:              %i\n", List_size(files.list));*/
    /*printf("Heap size:               %i\n", GC_get_heap_size());*/
    return TCL_OK;
}

/*tcl: lambda varlist body
 * Returns an anonymous function. Used like proc except with name argument
 * left out.
 */
TclReturn TclStd_lambda(Tcl *vm, int argc, TclValue argv[], TclValue *ret) {
    static int count = 0; // unique id returned for lambda
    if (argc != 3) {
        return TCL_EXCEPTION;
    }

    char lambdaName[256];
    snprintf(lambdaName, 255, "__lambda_%i", count);
    lambdaName[255] = 0;
    count++;

    TclUserFunction *f = (TclUserFunction*)malloc(sizeof(TclUserFunction));
    f->args = List_malloc();
    Tcl_split(vm, argv[1], " \t\n", f->args);

    TclValue_new(&f->code, argv[2]);
    Hash_get(functions, lambdaName)->data = f;

    Tcl_register(vm, lambdaName, TclStd_userFuncall);

    TclValue_new(ret, lambdaName);

    List_push(&lambdas, f);

    return TCL_OK;
}

/*tcl: add num ?num ...?
 * Returns the sum of all arguments
 */
TclReturn TclStd_add(Tcl *vm, int argc, TclValue argv[], TclValue *ret) {
    if (argc < 2) {
        return TCL_EXCEPTION;
    }
    char result[32];
    int sum = 0;
    int i;

    for (i = 1; i < argc; i++) {
        sum += atoi(argv[i]);
    }
    sprintf(result, "%i", sum);
    TclValue_new(ret, result);
    return TCL_OK;
}

/*tcl: sub num num ?num ...?
 * Returns the inverse sum of all arguments
 */
TclReturn TclStd_sub(Tcl *vm, int argc, TclValue argv[], TclValue *ret) {
    if (argc < 3) {
        return TCL_EXCEPTION;
    }
    char result[32];
    int sum = atoi(argv[1]);
    int i;

    for (i = 2; i < argc; i++) {
        sum -= atoi(argv[i]);
    }
    sprintf(result, "%i", sum);
    TclValue_new(ret, result);
    return TCL_OK;
}

/*tcl: mul num num ?num ...?
 * Returns the product of all arguments
 */
TclReturn TclStd_mul(Tcl *vm, int argc, TclValue argv[], TclValue *ret) {
    if (argc < 2) {
        return TCL_EXCEPTION;
    }
    char result[32];
    int sum = atoi(argv[1]);
    int i;

    for (i = 2; i < argc; i++) {
        sum *= atoi(argv[i]);
    }
    sprintf(result, "%i", sum);
    TclValue_new(ret, result);
    return TCL_OK;
}

/*tcl: div num num ?num ...?
 * Returns the quotient of all arguments
 */
TclReturn TclStd_div(Tcl *vm, int argc, TclValue argv[], TclValue *ret) {
    if (argc < 2) {
        return TCL_EXCEPTION;
    }
    char result[32];
    int sum = atoi(argv[1]);
    int i;

    for (i = 2; i < argc; i++) {
        sum /= atoi(argv[i]);
    }
    sprintf(result, "%i", sum);
    TclValue_new(ret, result);
    return TCL_OK;
}

/*tcl: eql num num
 * Returns 1 (true) if both arguments are equal, false otherwise.
 */
TclReturn TclStd_eql(Tcl *vm, int argc, TclValue argv[], TclValue *ret) {
    if (argc != 3) {
        return TCL_EXCEPTION;
    }
    int first = atoi(argv[1]);
    int second = atoi(argv[2]);
    char result[32];
    sprintf(result, "%i", first == second);
    TclValue_new(ret, result);
    return TCL_OK;
}

/*tcl: gt num num
 * Returns 1 (true) if first argument is greater than second argument, false
 * otherwise.
 */
TclReturn TclStd_gt(Tcl *vm, int argc, TclValue argv[], TclValue *ret) {
    if (argc != 3) {
        return TCL_EXCEPTION;
    }
    int first = atoi(argv[1]);
    int second = atoi(argv[2]);
    char result[32];
    sprintf(result, "%i", first > second);
    TclValue_new(ret, result);
    return TCL_OK;
}

/*tcl: lt num num
 * Returns 1 (true) if first argument is lesser than second argument, false
 * otherwise.
 */
TclReturn TclStd_lt(Tcl *vm, int argc, TclValue argv[], TclValue *ret) {
    if (argc != 3) {
        return TCL_EXCEPTION;
    }
    int first = atoi(argv[1]);
    int second = atoi(argv[2]);
    char result[32];
    sprintf(result, "%i", first < second);
    TclValue_new(ret, result);
    return TCL_OK;
}

/*tcl: or expr ?expr ...?
 * Logical or with short-circuit evaluation, returns the result of the last
 * true expression.
 */
TclReturn TclStd_or(Tcl *vm, int argc, TclValue argv[], TclValue *ret) {
    if (argc < 2) {
        return TCL_EXCEPTION;
    }
    TclValue tmp;
    int elmret;
    int i;
    TclValue_new(&tmp, NULL);
    for (i = 1; i < argc; i++) {
        elmret = Tcl_eval(vm, argv[i], &tmp);
        if (atoi(tmp)) {
            break;
        }
    }
    TclValue_new(ret, tmp);
    TclValue_delete(&tmp);
    return TCL_OK;
}

/*tcl: and expr ?expr ...?
 * Logical and, returning the result of the last expression evaluated.
 */
TclReturn TclStd_and(Tcl *vm, int argc, TclValue argv[], TclValue *ret) {
    if (argc < 2) {
        return TCL_EXCEPTION;
    }
    TclValue tmp;
    int elmret;
    int i;
    TclValue_new(&tmp, NULL);
    for (i = 1; i < argc; i++) {
        elmret = Tcl_eval(vm, argv[i], &tmp);
        if (!atoi(tmp)) {
            break;
        }
    }
    TclValue_new(ret, tmp);
    TclValue_delete(&tmp);
    return TCL_OK;
}

/*tcl: range num num
 * Return a range of number in a list.
 */
TclReturn TclStd_range(Tcl *vm, int argc, TclValue argv[], TclValue *ret) {
    if (argc != 3) {
        return TCL_EXCEPTION;
    }
    int i;
    int incr = 1;
    char result[32];
    if (atoi(argv[2]) < atoi(argv[1]))
        incr = -1;
    for (i = atoi(argv[1]); i != atoi(argv[2]) + incr; i += incr) {
        sprintf(result, "%i", i);
        TclValue_append(ret, result);
        TclValue_append(ret, " ");
    }
    return TCL_OK;
}

typedef void (*TclLibraryRegister)(Tcl *vm);

/*tcl: use library
 * Load a Tentcl shared library
 */
TclReturn TclStd_use(Tcl *vm, int argc, TclValue argv[], TclValue *ret) {
    if (argc != 2) {
        printf("wrong number of arguments\n");
        return TCL_EXCEPTION;
    }
#ifdef WITH_LIBRARIES
    void *dll = dlopen(argv[1], RTLD_NOW);
    if (!dll) {
        printf("%s\n", dlerror());
        return TCL_EXCEPTION;
    }
    ((TclLibraryRegister)dlsym(dll, "Tcl_registerLibrary"))(vm);
    List_push(&dlls, dll);
#else
    puts("use not supported\n");
#endif
    TclValue_new(ret, "loaded");
    return TCL_OK;
}

static void dllsDealloc(ListNode *node) {
#ifdef WITH_LIBRARIES
    dlclose(node->data);
#endif
}

static void lambdasDealloc(ListNode *node) {
    TclUserFunction *f = (TclUserFunction*)node->data;
    List_delete(f->args);
}

void TclExt_cleanup(void) {
    List_delete(&dlls);
    List_delete(&lambdas);
}

void TclExt_register(Tcl *vm) {
    Tcl_register(vm, "info", TclStd_info);
    Tcl_register(vm, "lambda", TclStd_lambda);
    Tcl_register(vm, "add", TclStd_add);
    Tcl_register(vm, "sub", TclStd_sub);
    Tcl_register(vm, "div", TclStd_div);
    Tcl_register(vm, "mul", TclStd_mul);
    Tcl_register(vm, "eql", TclStd_eql);
    Tcl_register(vm, "gt", TclStd_gt);
    Tcl_register(vm, "lt", TclStd_lt);
    Tcl_register(vm, "or", TclStd_or);
    Tcl_register(vm, "and", TclStd_and);
    Tcl_register(vm, "range", TclStd_range);
    Tcl_register(vm, "use", TclStd_use);

    List_new(&dlls);
    dlls.dealloc = dllsDealloc;

    List_new(&lambdas);
    lambdas.dealloc = lambdasDealloc;

    atexit(TclExt_cleanup);
}
