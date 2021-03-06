/*
 * Tentcl -- Extension Function Library
 * Copyright (C) 2006-2018 Aaron Marks. All Rights Reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include "ext.h"
#include "std.h"
#include "repl.h"
#ifdef WITH_LIBRARIES
#include <dlfcn.h>
#endif

#include "common.h"

/* static List lambdas; */
static List dlls;

// THIS FUNCTION CONFLICTS WITH STANDARD TCL COMMAND
// Output various information to stdout
/*tcl: info
 * Prints out an information summary of the program state.
 */
TclReturn TclStd_info(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
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
/* TclReturn TclStd_lambda(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) { */
/*     static int count = 0; // unique id returned for lambda */
/*     if (argc != 3) { */
/*         return TCL_BADCMD; */
/*     } */

/*     char lambdaName[256]; */
/*     snprintf(lambdaName, sizeof(lambdaName), "__lambda_%i", count); */
/*     count++; */

/*     TclUserFunction *f = (TclUserFunction*)malloc(sizeof(TclUserFunction)); */
/*     f->args = List_malloc(); */
/*     Tcl_split(vm, TclValue_str(argv[1]), " \t\n", f->args); */

/*     TclValue_new(&f->code, TclValue_str(argv[2])); */
/*     Hash_get(functions, lambdaName)->data = f; */

/*     Tcl_register(vm, lambdaName, TclStd_userFuncall); */

/*     TclValue_set(ret, lambdaName); */

/*     List_push(&lambdas, f); */

/*     return TCL_OK; */
/* } */

/*tcl: add num ?num ...?
 * Returns the sum of all arguments
 */
TclReturn TclStd_add(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
    if (argc < 2) {
        return TCL_BADCMD;
    }
    char result[32];
    int sum = 0;
    int i;

    for (i = 1; i < argc; i++) {
        sum += TclValue_int(argv[i]);
    }
    snprintf(result, sizeof(result), "%i", sum);
    TclValue_set(ret, result);
    return TCL_OK;
}

/*tcl: sub num num ?num ...?
 * Returns the inverse sum of all arguments
 */
TclReturn TclStd_sub(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
    if (argc < 3) {
        return TCL_BADCMD;
    }
    char result[32];
    int sum = TclValue_int(argv[1]);
    int i;

    for (i = 2; i < argc; i++) {
        sum -= TclValue_int(argv[i]);
    }
    snprintf(result, sizeof(result), "%i", sum);
    TclValue_set(ret, result);
    return TCL_OK;
}

/*tcl: mul num num ?num ...?
 * Returns the product of all arguments
 */
TclReturn TclStd_mul(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
    if (argc < 2) {
        return TCL_BADCMD;
    }
    char result[32];
    int sum = TclValue_int(argv[1]);
    int i;

    for (i = 2; i < argc; i++) {
        sum *= TclValue_int(argv[i]);
    }
    sprintf(result, "%i", sum);
    TclValue_set(ret, result);
    return TCL_OK;
}

/*tcl: div num num ?num ...?
 * Returns the quotient of all arguments
 */
TclReturn TclStd_div(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
    if (argc < 2) {
        return TCL_BADCMD;
    }
    char result[32];
    int sum = TclValue_int(argv[1]);
    int i;

    for (i = 2; i < argc; i++) {
        sum /= TclValue_int(argv[i]);
    }
    sprintf(result, "%i", sum);
    TclValue_set(ret, result);
    return TCL_OK;
}

/*tcl: eql str str
 * Returns 1 (true) if both arguments are equal, false otherwise.
 */
TclReturn TclStd_eql(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
    if (argc != 3) {
        return TCL_BADCMD;
    }
    char *first = TclValue_str(argv[1]);
    char *second = TclValue_str(argv[2]);
    TclValue_set_int(ret, strcmp(first, second) == 0);
    return TCL_OK;
}

/*tcl: gt num num
 * Returns 1 (true) if first argument is greater than second argument, false
 * otherwise.
 */
TclReturn TclStd_gt(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
    if (argc != 3) {
        return TCL_BADCMD;
    }
    int first = TclValue_int(argv[1]);
    int second = TclValue_int(argv[2]);
    char result[32];
    sprintf(result, "%i", first > second);
    TclValue_set(ret, result);
    return TCL_OK;
}

/*tcl: lt num num
 * Returns 1 (true) if first argument is lesser than second argument, false
 * otherwise.
 */
TclReturn TclStd_lt(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
    if (argc != 3) {
        return TCL_BADCMD;
    }
    int first = TclValue_int(argv[1]);
    int second = TclValue_int(argv[2]);
    char result[32];
    sprintf(result, "%i", first < second);
    TclValue_set(ret, result);
    return TCL_OK;
}

/*tcl: or expr ?expr ...?
 * Logical or with short-circuit evaluation, returns the result of the last
 * true expression.
 */
TclReturn TclStd_or(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
    if (argc < 2) {
        return TCL_BADCMD;
    }
    TclValue *tmp = NULL;
    int elmret;
    int i;
    TclValue_new(&tmp, NULL);
    for (i = 1; i < argc; i++) {
        elmret = Tcl_eval(vm, TclValue_str(argv[i]), tmp);
        if (elmret == TCL_OK && tmp != NULL && TclValue_int(tmp)) {
            break;
        }
    }
    if (elmret == TCL_OK && tmp != NULL) {
        TclValue_replace(ret, tmp);
    }
    TclValue_delete(tmp);
    return TCL_OK;
}

/*tcl: and expr ?expr ...?
 * Logical and, returning the result of the last expression evaluated.
 */
TclReturn TclStd_and(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
    if (argc < 2) {
        return TCL_BADCMD;
    }
    TclValue *tmp = NULL;
    int elmret;
    int i;
    TclValue_new(&tmp, NULL);
    for (i = 1; i < argc; i++) {
        if (tmp != NULL) {
            TclValue_set(tmp, NULL);
        }
        elmret = Tcl_eval(vm, TclValue_str(argv[i]), tmp);
        if (elmret == TCL_OK && tmp != NULL && !TclValue_int(tmp)) {
            break;
        }
    }
    if (elmret == TCL_OK && tmp != NULL) {
        TclValue_replace(ret, tmp);
    }
    TclValue_delete(tmp);
    return TCL_OK;
}

/*tcl: not expr
 * Logical not
 */
TclReturn TclStd_not(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
    if (argc != 2) {
        return TCL_BADCMD;
    }
    TclValue_set_int(ret, !TclValue_int(argv[1]));
    return TCL_OK;
}

/*tcl: range num num
 * Return a range of number in a list.
 */
TclReturn TclStd_range(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
    if (argc != 3) {
        return TCL_BADCMD;
    }
    int i;
    int incr = 1;
    char result[32];
    if (TclValue_int(argv[2]) < TclValue_int(argv[1]))
        incr = -1;
    for (i = TclValue_int(argv[1]); i != TclValue_int(argv[2]) + incr; i += incr) {
        sprintf(result, "%i", i);
        TclValue_append(ret, result);
        TclValue_append(ret, " ");
    }
    return TCL_OK;
}

/*tcl: debug
 * Create a REPL with the current state
 */
TclReturn TclStd_repl(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
    printf(">> Entering REPL\n");
    TclReturn status = TclRepl_repl(vm, stdin);
    printf("\n>> Exiting REPL (%i)\n", status);
    return TCL_OK;
}

static void valuePrint(BTreeNode *node, void *_unused) {
    HashPair *pair = (HashPair*)node->data;
    printf("%s (%s)\n", pair->name, TclValue_type_str((TclValue*)pair->data));
}

/*tcl: bindings
 * Return an array of current bound variables
 */
TclReturn TclStd_bindings(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
    Hash_map(vm->variables, valuePrint, NULL);
    return TCL_OK;
}

#define GOTO_IN 1
#define GOTO_OUT 2

static void label_free(TclValueObject *obj) {
    free(obj->ptr);
}

TclReturn TclStd_label(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
    TclReturn status = TCL_OK;
    TclValue *obj;

    if (argc != 3) {
        return TCL_BADCMD;
    }

    jmp_buf *env = (jmp_buf*)malloc(sizeof(jmp_buf));
    TclValue_new_object(&obj, "label", env, (void (*)(void*))label_free);
    Tcl_addVariable_(vm, TclValue_str(argv[1]), obj);

    int j = setjmp(*env);
    if (j != GOTO_OUT) {
      status = TclStd_eval(vm, argc-1, argv+1, ret);
    }

    /* invalidate the label */
    TclValue_set_null(obj);
    /* TODO cleanup things propertly */

    return status;
}

TclReturn TclStd_goto(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
    if (argc != 2) {
        return TCL_BADCMD;
    }

    /* lookup name and see if it is a label */
    TclValue *v = Tcl_getVariableUp(vm, TclValue_str(argv[1]), 1);
    if (v && TclValue_type_object_cmp(v, "label") == 0) {
        TclValueObject *obj = (TclValueObject*)TCL_VALUE_TAG_REMOVE(v->container->value);
        longjmp(*(jmp_buf*)obj->ptr, GOTO_IN);
    }
    return TCL_EXCEPTION;
}

TclReturn TclStd_leave(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
    if (argc != 2) {
        return TCL_BADCMD;
    }

    /* lookup name and see if it is a label */
    TclValue *v = Tcl_getVariableUp(vm, TclValue_str(argv[1]), 1);
    if (v && TclValue_type_object_cmp(v, "label") == 0) {
        TclValueObject *obj = (TclValueObject*)TCL_VALUE_TAG_REMOVE(v->container->value);
        longjmp(*(jmp_buf*)obj->ptr, GOTO_OUT);
    }
    return TCL_EXCEPTION;
}

TclReturn TclStd_noop(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
    return TCL_OK;
}

TclReturn TclStd_null(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
    if (argc != 2) {
        return TCL_BADCMD;
    }
    TclValue_set_null(ret);
    return TCL_OK;
}

TclReturn TclStd_typeof(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
    if (argc != 2) {
        return TCL_BADCMD;
    }

    TclValue *value = Tcl_getVariable(vm, TclValue_str(argv[1]));
    if (value)
        TclValue_set(ret, TclValue_type_str(value));

    return TCL_OK;
}

TclReturn TclStd_take(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
    if (argc != 3) {
        return TCL_BADCMD;
    }
    int to_take = TclValue_int(argv[1]);
    TclValue *list = Tcl_getVariable(vm, TclValue_str(argv[2]));

    TclValue_coerce(list, TCL_VALUE_LIST);

    if (to_take > TclValue_list_size(list))
        to_take = TclValue_list_size(list);

    TclValue *result;
    TclValue_new_list(&result);

    for (unsigned int i = 0; i < to_take; i++) {
        TclValue *v = TclValue_list_elt(list, i);
        TclValue_list_push(result, v);
    }

    TclValue_replace(ret, result);
    TclValue_delete(result);

    return TCL_OK;
}

TclReturn TclStd_drop(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
    if (argc != 3) {
        return TCL_BADCMD;
    }
    int to_drop = TclValue_int(argv[1]);
    TclValue *list = Tcl_getVariable(vm, TclValue_str(argv[2]));

    TclValue_coerce(list, TCL_VALUE_LIST);

    if (to_drop > TclValue_list_size(list))
        to_drop = TclValue_list_size(list);

    TclValue *result;
    TclValue_new_list(&result);

    for (unsigned int i = 0; i < to_drop; i++) {
        TclValue *elt = TclValue_list_shift(list);
        TclValue_list_push(result, elt);
        TclValue_delete(elt);
    }

    TclValue_replace(ret, result);
    TclValue_delete(result);

    return TCL_OK;
}

TclReturn TclStd_length(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
    if (argc != 2) {
        return TCL_BADCMD;
    }
    TclValue *list = Tcl_getVariable(vm, TclValue_str(argv[1]));

    if (TclValue_type(list) != TCL_VALUE_LIST) {
        return TCL_EXCEPTION;
    }

    TclValue_set_int(ret, TclValue_list_size(list));

    return TCL_OK;
}

TclReturn TclExt_expand(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
    if (argc < 2) {
        return TCL_BADCMD;
    }

    TclReturn status;
    TclValue *r = NULL;
    TclValue *list;
    TclValue_new_list(&list);

    for (int i = 1; i < argc; i++) {
        TclValue_new(&r, NULL);
        status = Tcl_expand(vm, TclValue_str(argv[i]), r);
        if (status != TCL_OK)
            break;
        TclValue_list_push(list, r);
        TclValue_delete(r);
        r = NULL;
    }

    TclValue_replace(ret, list);
    TclValue_delete(list);
    if (r) TclValue_delete(r);

    return status;
}

TclReturn TclExt_escape(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
    if (argc < 2) {
        return TCL_BADCMD;
    }

    TclValue *list;
    TclValue_new_list(&list);

    for (unsigned int i = 1; i < argc; i++) {
        TclValue *r;
        TclValue_new(&r, TclValue_str_esc(argv[i]));
        TclValue_list_push(list, r);
        TclValue_delete(r);
    }
    TclValue_replace(ret, list);
    TclValue_delete(list);
    return TCL_OK;
}

typedef void (*TclLibraryRegister)(Tcl *vm);

/*tcl: use library
 * Load a Tentcl shared library
 */
TclReturn TclStd_use(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
    if (argc != 2) {
        printf("wrong number of arguments\n");
        return TCL_EXCEPTION;
    }
#ifdef WITH_LIBRARIES
    void *dll = dlopen(TclValue_str(argv[1]), RTLD_NOW);
    if (!dll) {
        printf("%s\n", dlerror());
        return TCL_EXCEPTION;
    }
    ((TclLibraryRegister)dlsym(dll, "Tcl_registerLibrary"))(vm);
    List_push(&dlls, dll);
#else
    puts("use not supported\n");
#endif
    TclValue_set(ret, "loaded");
    return TCL_OK;
}

static void dllsDealloc(ListNode *node) {
#ifdef WITH_LIBRARIES
    dlclose(node->data);
#endif
}

/* static void lambdasDealloc(ListNode *node) { */
/*     TclUserFunction *f = (TclUserFunction*)node->data; */
/*     List_delete(f->args); */
/* } */

void TclExt_cleanup(void) {
    List_delete(&dlls);
    /* List_delete(&lambdas); */
}

void TclExt_register(Tcl *vm) {
    Tcl_register(vm, "info", TclStd_info);
    /* Tcl_register(vm, "lambda", TclStd_lambda); */
    Tcl_register(vm, "add", TclStd_add);
    Tcl_register(vm, "sub", TclStd_sub);
    Tcl_register(vm, "div", TclStd_div);
    Tcl_register(vm, "mul", TclStd_mul);
    Tcl_register(vm, "eql", TclStd_eql);
    Tcl_register(vm, "gt", TclStd_gt);
    Tcl_register(vm, "lt", TclStd_lt);
    Tcl_register(vm, "or", TclStd_or);
    Tcl_register(vm, "and", TclStd_and);
    Tcl_register(vm, "not", TclStd_not);
    Tcl_register(vm, "range", TclStd_range);
    Tcl_register(vm, "use", TclStd_use);

    Tcl_register(vm, "repl", TclStd_repl);
    Tcl_register(vm, "bindings", TclStd_bindings);
    Tcl_register(vm, "label", TclStd_label);
    Tcl_register(vm, "goto", TclStd_goto);
    Tcl_register(vm, "leave", TclStd_leave);
    Tcl_register(vm, "noop", TclStd_noop);
    Tcl_register(vm, "null", TclStd_null);
    Tcl_register(vm, "take", TclStd_take);
    Tcl_register(vm, "drop", TclStd_drop);
    Tcl_register(vm, "length", TclStd_length);
    Tcl_register(vm, "typeof", TclStd_typeof);

    /* For debugging */
    Tcl_register(vm, "expand", TclExt_expand);
    Tcl_register(vm, "escape", TclExt_escape);

    List_new(&dlls);
    dlls.dealloc = dllsDealloc;

    /* List_new(&lambdas); */
    /* lambdas.dealloc = lambdasDealloc; */

    atexit(TclExt_cleanup);
}
