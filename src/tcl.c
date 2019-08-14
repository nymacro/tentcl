/*
 * Tentcl -- Core
 * Copyright (C) 2006-2018 Aaron Marks. All Rights Reserved.
 */

/*
 * TODO:
 * - Namespaces
 * - Better debugging
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "tcl.h"

#include "common.h"

/********/
const static char *errorReturnStrings[] = {
    "(OK) evaluation successful",
    "(RETURN) returned",
    "(BREAK) break exception raised",
    "(CONTINUE) continue exception raised",
    "(EXCEPTION) user exception raised",
    "(EXIT) exit exception raised",
    "(BADCMD) invalid command exception raised",
    "(OOM) out of memory",
    "(INTERRUPT) user interrupt"
};

static TclReturnInfo returnInfo;

/* Tcl_getReturnInfo
 * Retrieve additional info from return.
 */
TclReturnInfo Tcl_getReturnInfo(void) {
    return returnInfo;
}

/* Tcl_setReturnInfo
 * Set additional info for return.
 */
void Tcl_setReturnInfo(TclReturnInfo info) {
    returnInfo = info;
}

/** Tcl_statusToCode
 * Get appropriate return code for status
 */
int Tcl_statusToCode(TclReturn status) {
    switch (status) {
    case TCL_OK:
    case TCL_RETURN:
    case TCL_BREAK:
    case TCL_CONTINUE:
        return 0;
    case TCL_EXCEPTION:
        return 1;
    case TCL_EXIT:
        if (returnInfo.type == TCL_RI_INT) {
            return returnInfo.i;
        } else {
            return 0x80;
        }
    case TCL_BADCMD:
        return 2;
    case TCL_OOM:
        return 3;
    default:
        return 0x80 | status;
    }
}


/* Tcl_returnString
 * Return a status string for given status number.
 */
const char *Tcl_returnString(TclReturn status) {
    if (status < TCL_COUNT)
        return errorReturnStrings[status];
    else
        return "status code requested does not exist";
}

/* Tcl_substring_
 * Return malloc'd string containing specified substring
 */
static char* Tcl_substring_(char *string, unsigned int start, unsigned int end) {
    int len = end - start;
    char *result = (char*)malloc(sizeof(char) * len + 1);
    int i;

    if (len < 0)
        len = 0;

    for (i = 0; i < len; i++) {
        result[i] = string[start + i];
    }

    result[len] = '\0';
    return result;
}

/* Tcl_string_compare
 *
 */
static int Tcl_split_compare(ListNode *node, void *data) {
    return strcmp(node->data, data);
}

/* Tcl_string_alloc
 *
 */
static void Tcl_split_alloc(ListNode *node, void *data) {
    node->data = data;
}

/* Tcl_string_dealloc
 *
 */
static void Tcl_split_dealloc(ListNode *node) {
    // TclValue_delete((TclValue*)node->data);
    free(node->data);
}

static void Hash_variables_dealloc_(HashPair *pair) {
    TclValue_delete((TclValue*)pair->data);
}

/********/

/* Tcl_new
 * Create a Tcl interpreter object.
 */
void Tcl_new(Tcl *vm) {
    vm->namespace = List_malloc();

    vm->variables = Hash_malloc();
    vm->variables->dealloc = Hash_variables_dealloc_;

    vm->functions = Hash_malloc();

    List_push(vm->namespace, vm->variables);

    /* Tcl related variables */

    Tcl_addVariable(vm, "tcl_version", TENTCL_VERSION);
    Tcl_addVariable(vm, "tcl_prompt1", "% ");
    Tcl_addVariable(vm, "tcl_prompt2", "> ");
    Tcl_addVariable(vm, "tcl_interactive", "0");
}

/* Tcl_delete
 * Destroys a Tcl interpreter object.
 */
void Tcl_delete(Tcl *vm) {
    List_free(vm->namespace);
    Hash_free(vm->variables);
    Hash_free(vm->functions);
}

/* Tcl_malloc
 * Return a dynamically allocated Tcl interpreter object
 */
Tcl *Tcl_malloc(void) {
    Tcl *self = (Tcl*)malloc(sizeof(Tcl));
    Tcl_new(self);
    return self;
}

/* Tcl_free
 * Free a dynamically allocated Tcl interpreter object
 */
void Tcl_free(Tcl *self) {
    Tcl_delete(self);
    free(self);
}

/* Tcl_evalExpression
 * Evaluate single Tcl expression
 * @param ret must be a freed if set
 */
TclReturn Tcl_evalExpression(Tcl *vm, char *expression, TclValue *ret) {
    TclReturn status = TCL_OK;

    /* skip white space */
    while (isspace(expression[0]))
        expression++;

    /* skip comments and empty expressions */
    if (expression[0] == '#' || expression[0] == '\n' || strlen(expression) == 0)
        return TCL_OK;

    /* create argument list */
    List *list = List_malloc();
    Tcl_split(vm, expression, " \t\n", list);

    int argc = List_size(list);
    if (argc < 1) {
        List_free(list);
        return TCL_OK;
    }

    TclValue **argv;
    argv = (TclValue**)malloc(sizeof(TclValue*) * argc);
    memset(argv, 0, sizeof(TclValue*) * argc);

    int i;
    for (i = 0; i < argc; i++) {
        TclValue_new(&argv[i], NULL);
        status = Tcl_expand(vm, List_index(list, i)->data, argv[i]);
        if (status != TCL_OK) {
            goto fail;
        }
    }

    /* call the function */
    status = Tcl_funcall(vm, TclValue_str(argv[0]), argc, argv, ret);

fail:
    /* free arguments */
    for (i = 0; i < argc; i++) {
        TclValue_delete(argv[i]);
    }

    free(argv);

    List_free(list);
    return status;
}

/* Tcl_eval
 * Evaluate Tcl expression.
 *
 * @param ret must be freed if set
 */
TclReturn Tcl_eval(Tcl *vm, char *expression, TclValue *ret) {
    List *list = List_malloc();
    Tcl_split(vm, expression, "\n;", list);
    TclReturn status = TCL_EXCEPTION;

    if (List_size(list) < 1) {
        List_free(list);
        return TCL_OK;
    }

    int i;
    for (i = 0; i < List_size(list); i++) {
        status = Tcl_evalExpression(vm, List_index(list, i)->data, ret);
        // exception
        if (status != TCL_OK && status != TCL_RETURN) {
            if (status == TCL_EXIT)
                break;
            if (status == TCL_BREAK)
                break;
            /* fprintf(stderr, "%s from: %s\n", Tcl_returnString(status), (char*)List_index(list, i)->data); */
            /* TclValue_set(ret, (char*)List_index(list, i)->data); */
            break;
        }
    }

    List_free(list);
    return status;
}

/* Tcl_funcall
 * Call Tcl command.
 * @param ret must be freed if set
 */
TclReturn Tcl_funcall(Tcl *vm, char *function, int argc, TclValue *argv[], TclValue *ret) {
    TclValue *fun = Tcl_getFunction(vm, function);
    if (!fun) {
        return TCL_BADCMD;
    } else if (TclValue_type(fun) != TCL_VALUE_FUN) {
        return TCL_BADCMD;
    } else {
        return ((TclFunction)TclValue_fun(fun))(vm, argc, argv, ret);
    }
}

/* findMatching
 * Find character to match the given character
 */
static int findMatching(char *str, int start, char c) {
    int count = 1;
    int i;
    for (i = start + 1; i < strlen(str); i++) {
        if (str[i] == str[start] && str[i] != c)
            count++;
        if (str[i] == c)
            count--;
        if (count == 0)
            return i;
    }
    return -1;
}

/* Tcl_getKeyedValue_
 * Return malloc'd string containing value associated with key
 */
static char *Tcl_getKeyedValue_(Tcl *vm, char *str, char *key) {
    char *result = NULL;
    int i;

    List *list = List_malloc();
    Tcl_split(vm, str, " \t\n", list);

    for (i = 0; i < List_size(list); i += 2) {
        if (strcmp(List_index(list, i)->data, key) == 0) {
            if (i + 1 < List_size(list)) {
                char *dst = List_index(list, i + 1)->data;
                result = (char*)malloc(strlen(dst) + 1);
                strcpy(result, dst);
                break;
            } else {
                break;
            }
        }
    }

    List_free(list);

    return result;
}

TclReturn Tcl_expand_(Tcl *vm, char *value, TclValue *result) {
    int i;
    TclReturn ret = TCL_OK;

    for (i = 0; i < strlen(value); i++) {
        if (value[i] == '\\') {
            char tmp[2];
            tmp[0] = value[++i];
            tmp[1] = 0;
            TclValue_append(result, tmp);
        } else if (value[i] == '$') {
            int hashed = 0;
            int j;

            for (j = i + 1; j < i + strlen(value); j++) {
                if (!isalpha(value[j]) && value[j] != '_' &&
                    !(value[j] >= '0' && value[j] <= '9')) {
                    if (value[j] == '(') {
                        hashed = 1;
                        break;
                    } else {
                        break;
                    }
                }
            }

            char *name = Tcl_substring_(value, i + 1, j);
            TclValue *var = Tcl_getVariable(vm, name);
            if (var) {
                char *v = TclValue_str(var);
                if (hashed) {
                    int end = findMatching(v, j, ')');
                    char *key = Tcl_substring_(v, j + 1, end);
                    char *str = Tcl_getKeyedValue_(vm, v, key);
                    TclValue_append(result, str);
                    free(str);
                    free(key);
                    j = end + 1;
                } else {
                    TclValue_append(result, v);
                }
            } else {
                printf("can't read \"%s\": no such variable\n", name);
            }
            free(name);
            i = j - 1;
        } else if (value[i] == '[') {
            int end = findMatching(value, i, ']');
            if (end > 0) {
                char *str = Tcl_substring_(value, i + 1, end);
                TclValue *eval = NULL;
                TclValue_new(&eval, NULL);
                ret = Tcl_eval(vm, str, eval);
                if (ret == TCL_OK && eval) {
                    if (TclValue_null(result)) {
                        TclValue_replace(result, eval);
                    } else {
                        TclValue_append(result, TclValue_str(eval));
                    }
                }
                TclValue_delete(eval);
                free(str);
                if (ret != TCL_OK)
                    return ret;
                i = end;
            } else {
                printf("ERROR: [ UNMATCHED\n");
            }
        } else if (value[i] == '{') {
            int end = findMatching(value, i, '}');
            if (end > 0) {
                char *str = Tcl_substring_(value, i + 1, end);
                TclValue_append(result, str);
                free(str);
                i = end;
            } else {
                printf("ERROR: { UNMATCHED\n");
            }
        } else if (value[i] == '"') {
            int end = findMatching(value, i, '"');
            if (end > 0) {
                char *str = Tcl_substring_(value, i + 1, end);
                TclValue *eval;
                TclValue_new(&eval, NULL);
                ret = Tcl_expand(vm, str, eval);
                TclValue_append(result, TclValue_str(eval));
                TclValue_delete(eval);
                free(str);
                if (ret != TCL_OK) {
                    return ret;
                }
                i = end;
            } else {
                printf("ERROR: \" UNMATCHED\n");
            }
        } else {
            char tmp[2];
            tmp[0] = value[i]; tmp[1] = 0;
            TclValue_append(result, tmp);
        }
    }

    return ret;
}

/* Tcl_expand
 * Expand the string, performing command and variable substitution
 *
 * @return Result of expansion. Must be freed with TclValue_delete
 */
TclReturn Tcl_expand(Tcl *vm, char *value, TclValue *result) {
    return Tcl_expand_(vm, value, result);
}

/* charisin
 * Checks to see if the character C is a character contained in string m
 */
static int charisin(char c, char *m) {
    int i;
    for (i = 0; i < strlen(m); i++)
        if (c == m[i])
            return 1;
    return 0;
}

/* Tcl_split
 * Split string into individual elements, obeying Tcl syntax rules.
 */
void Tcl_split(Tcl *vm, char *value, char *delims, List *result) {
    int i;
    int last = 0;
    int quoted = 0;
    int braces = 0;
    int squares = 0;

    result->compare = Tcl_split_compare;
    result->alloc = Tcl_split_alloc;
    result->dealloc = Tcl_split_dealloc;

    for (i = 0; i < strlen(value); i++) {
        if (value[i] == '{' && !squares && !quoted)
            braces++;
        else if (value[i] == '}' && !squares && !quoted)
            braces--;
        if (value[i] == '[' && !braces)
            squares++;
        else if (value[i] == ']' && !braces)
            squares--;
        if (value[i] == '"' && !braces && !squares)
            quoted = !quoted;

        if (charisin(value[i], delims) && i == last) {
            last++;
            continue;
        }

        if ((value[i] == '\0' || charisin(value[i], delims))
            && !quoted && !braces && !squares
            && i > 0 && value[i - 1] != '\\') {
            if ((i > 0 && charisin(value[i - 1], delims)) ||
                (i == 0 && charisin(value[i], delims))) {
                last = i + 1;
                continue;
            } else {
                char *str = Tcl_substring_(value, last, i);
                List_push(result, str);
                last = i + 1;
                continue;
            }
        }
    }

    if (!quoted && !braces && !squares && last < i) 
        List_push(result, Tcl_substring_(value, last, strlen(value)));
}

/* Tcl_register
 * Register new Tcl command in the interpreter.
 */
void Tcl_register(Tcl *vm, char *name, TclFunction function) {
    TclValue *val = NULL;
    TclValue_new_function(&val, (TclFunction_)function);
    Tcl_addVariable_(vm, name, val);
}

/* Tcl_addVariable
 * Add variable to Tcl VM.
 */
void Tcl_addVariable(Tcl *self, char *name, char *value) {
    TclValue *val = NULL;
    TclValue_new(&val, value);
    Hash_get(self->variables, name)->data = val;
}

void Tcl_addVariable_(Tcl *self, char *name, TclValue *val) {
    Hash_get(self->variables, name)->data = val;
}

/* Tcl_getVariable
 * Get the value of specified variable (this value should not be changed)
 */
TclValue *Tcl_getVariable(Tcl *self, char *name) {
    HashPair *p = Hash_get(self->variables, name);
    return p->data;
}

TclValue *Tcl_getVariableUp(Tcl *self, char *name, int level) {
    if (level == 0) {
        /* try until we find something */
        TclValue *v = Tcl_getVariable(self, name);
        int i = List_size(self->namespace);
        while (v == NULL && i > 0) {
            v = Tcl_getVariableUp(self, name, i--);
        }
        return v;
    }
    if (level < 0)
        return NULL;

    /* get from upper namespace */
    ListNode *n = List_index(self->namespace, List_size(self->namespace) - level);
    if (!n)
        return NULL;

    Hash *namespace = (Hash*)n->data;

    HashPair *p = Hash_get(namespace, name);
    return p->data;
}

TclValue *Tcl_getFunction(Tcl *self, char *name) {
    return Tcl_getVariableUp(self, name, 0);
}

/* Tcl_pushNamespace
 * Push a namespace onto namespace stack
 */
void Tcl_pushNamespace(Tcl *self) {
    Hash *hash = Hash_malloc();
    self->variables = hash;
    self->variables->dealloc = Hash_variables_dealloc_;
    List_push(self->namespace, hash);
    return;
}

/* Tcl_popNamespace
 * Pop namespace off namespace stack
 */
void Tcl_popNamespace(Tcl *self) {
    Hash *hash = List_last(self->namespace)->data;
    List_pop(self->namespace);
    Hash_free(hash);
    self->variables = List_last(self->namespace)->data;
}

/* Tcl_isComplete
 * Test for a complete Tcl expression
 */
int Tcl_isComplete(Tcl *self, char *expr) {
    int i, braces = 0, quotes = 0;
    for (i = 0; i < strlen(expr); i++) {
        if (expr[i] == '"') {
            quotes = !quotes;
        } else if (expr[i] == '{') {
            braces++;
        } else if (expr[i] == '}') {
            braces--;
        }
    }
    if (braces == 0 && quotes == 0)
        return 1;
    return 0;
}
