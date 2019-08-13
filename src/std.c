/*
 * Tentcl -- Standard Function Library
 * Copyright (C) 2006-2018 Aaron Marks. All Rights Reserved.
 */
/*
 * TODO:
 * - Allow set to use array syntax
 * - Better error messages/debugging
 * - Investigate seg fault on args delete (args is getting messed up)
 * - Implement a replacement for atexit for shared libs -- atexit only
 *   supports 32 callbacks.
 */
#include "std.h"
#include "value.h"
#include "mathexpr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <glob.h>

#include "common.h"

static Hash *files;
Hash *functions;

extern void string_dealloc(ListNode *node);

void TclStd_files_dealloc(HashPair *pair) {
    fclose(pair->data);
}

void TclStd_functions_dealloc(BTreeNode *node) {
    /* FIXME: for some reason -- args is getting screwed... */
    if (node->data) {
        TclUserFunction *fn = (TclUserFunction*)node->data;
        /*List_free(fn->args);*/
        TclValue_delete(fn->code);
        free(node->data);
    }
}

TclReturn TclStd_userFuncall(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
    TclReturn status;
    TclUserFunction *f = Hash_get(functions, TclValue_str(argv[0]))->data;
    int i;
    int var_args = 0;

    /* check of variable args */
    ListNode *last_arg = List_last(f->args);
    if (last_arg && strcmp(last_arg->data, "args") == 0)
      var_args = 1;

    if (!var_args && List_size(f->args) != argc - 1) {
        printf("Invalid argument list (takes %i, given %i)\n", List_size(f->args), argc - 1);
        for (i = 0; i < List_size(f->args); i++) {
            printf("%i: '%s' %i\n", i, (char*)List_index(f->args, i)->data, (int)strlen((char*)List_index(f->args, i)->data));
        }
        return TCL_BADCMD;
    }

    Tcl_pushNamespace(vm);

    for (i = 0; i < List_size(f->args) - var_args; i++) {
        HashPair *p = Hash_get(vm->variables, List_index(f->args, i)->data);
        TclValue_new((TclValue**)&p->data, TclValue_str(argv[i + 1]));
    }

    /* combine rest of arguments */
    if (var_args && i < List_size(f->args)) {
        HashPair *p = Hash_get(vm->variables, List_index(f->args, i)->data);
        TclValue **v = (TclValue**)&p->data;
        TclValue_new(v, NULL);

        for (; i < argc - 1; i++) {
            TclValue_append(*v, TclValue_str(argv[i + 1]));
            TclValue_append(*v, " ");
        }
    }

    status = Tcl_eval(vm, TclValue_str(f->code), ret);

    Tcl_popNamespace(vm);

    return status;
}

TclValue *TclStd_expression(Tcl *vm, TclValue *expression) {
    TclValue *val;
    TclValue_new(&val, NULL);
    TclReturn status = Tcl_expand(vm, TclValue_str(expression), val);
    if (status != TCL_OK)
        return val;
    int ret = Math_eval(TclValue_str(val));
    TclValue_delete(val);

    TclValue *retval;
    TclValue_new_int(&retval, ret);
    return retval;
}

/**********************************************/

TclReturn TclStd_apply(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
    if (argc != 3) {
        return TCL_BADCMD;
    }

    TclReturn status;
    char *function = TclValue_str(argv[1]);
    List *args = List_malloc();
    Tcl_split(vm, TclValue_str(argv[2]), " \t\n", args); /* dirty */
    int nargs = List_size(args) + 1;

    TclValue **vargs = (TclValue**)malloc(sizeof(TclValue*) * nargs);
    memset(vargs, 0, sizeof(TclValue*) * nargs);

    TclValue_new_ref(&vargs[0], argv[1]);
    for (int i = 0; i < nargs-1; i++) {
        TclValue_new(&vargs[i+1], NULL);
        status = Tcl_expand(vm, List_index(args, i)->data, vargs[i+1]);
        if (status != TCL_OK)
            goto fail;
    }

    status = Tcl_funcall(vm, function, nargs, vargs, ret);

fail:
    TclValue_delete(vargs[0]);
    free(vargs);
    List_free(args);
    return status;
}

/*tcl: puts ?-nonewline? ?fileid? ?output?
 * Outputs output to the standard output stream. If -nonewline is specified
 * the cursor will stay on the same line. If fileid is specified it will
 * output to the appropriate file.
 */
TclReturn TclStd_puts(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
    int nonewline = 0;
    int i = 1;
    FILE *fp = stdout;

    if (argc > 4) {
        return TCL_BADCMD;
    }

    if (argc > 1 && strcmp(TclValue_str(argv[1]), "-nonewline") == 0) {
        nonewline = 1;
        i++;
    }

    if (argc - nonewline > 2) {
        HashPair *p = Hash_get(files, TclValue_str(argv[i]));
        if (!p->data) {
            fprintf(stderr, "Stream '%s' does not exist\n", TclValue_str(argv[i]));
            return TCL_EXCEPTION;
        }
        fp = (FILE*)p->data;
        fputs(TclValue_str(argv[++i]), fp);
    } else if (argc - nonewline == 2) {
        fputs(TclValue_str(argv[i]), fp);
    }

    if (!nonewline)
        fputc('\n', fp);

    return TCL_OK;
}

/*tcl: set varName ?value?
 * Set the variable named varName to value. If value is not provided the
 * existing value of varName is returned.
 */
TclReturn TclStd_set(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
    if (argc < 2) {
        return TCL_BADCMD;
    } else if (argc < 3) {
        TclValue *value = Tcl_getVariable(vm, TclValue_str(argv[1]));
        if (value) {
            /* TclValue_set(ret, TclValue_str(value)); */
            TclValue_replace(ret, value);
        } else {
            printf("can't read \"%s\": no such variable\n", TclValue_str(argv[1]));
            return TCL_EXCEPTION;
        }
    } else {
        TclValue *value = Tcl_getVariable(vm, TclValue_str(argv[1]));
        if (value) {
            TclValue_set(value, TclValue_str(argv[2]));
            /* TclValue_replace(value, argv[2]); */
        } else {
            TclValue_new(&value, NULL);
            TclValue_replace(value, argv[2]);
            Tcl_addVariable_(vm, TclValue_str(argv[1]), value);
        }

        TclValue_replace(ret, value); /* set return value */
    }
    return TCL_OK;
}

/*tcl: unset varName
 * Removes the variable varName and its associated value from the program.
 */
TclReturn TclStd_unset(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
    // TODO: extend Hash to remove the key to value (which is NULL after unset)
    if (argc != 2) {
        return TCL_BADCMD;
    }
    HashPair *p = Hash_get(vm->variables, TclValue_str(argv[1]));
    if (p->data) {
        TclValue_delete((TclValue*)&p->data);
        p->data = NULL;
    }
    return TCL_OK;
}

/*tcl: exit ?returncode?
 * TODO: implement return code
 *
 * Exits the program with the status returncode. If returncode is not specified
 * a default value of 0 (sucess) will be used.
 */
TclReturn TclStd_exit(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
    if (argc == 1)
        return TCL_EXIT;
    if (argc == 2) {
        TclReturnInfo info;
        info.type = TCL_RI_INT;
        info.i    = TclValue_int(argv[1]);
        Tcl_setReturnInfo(info);

        /* set return value to exit code */
        char buf[8];
        snprintf(buf, 8, "%i", info.i);
        TclValue_set(ret, buf);

        return TCL_EXIT;
    }
    return TCL_BADCMD;
}

/*tcl: proc name varlist body
 *
 * Defines a procedure called name as a Tcl command. When this command is
 * executed each argument specified will be bound to each element of the list
 * varlist before the body is evaluated.
 */
TclReturn TclStd_proc(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
    if (argc != 4) {
        return TCL_BADCMD;
    }
    TclUserFunction *f = (TclUserFunction*)malloc(sizeof(TclUserFunction));
    f->args = List_malloc();
    Tcl_split(vm, TclValue_str(argv[2]), " \t\n", f->args);

    TclValue_new(&f->code, TclValue_str(argv[3]));
    Hash_get(functions, TclValue_str(argv[1]))->data = f;

    Tcl_register(vm, TclValue_str(argv[1]), TclStd_userFuncall);

    return TCL_OK;
}

/*tcl: expr expression ?expression ...?
 * Concatenates all arguments into a single string then evaluates them
 * mathematically.
 *
 * Also supports special mathematical functions which are used like their C
 * counterparts. eg. sin(1)
 *
 * Supported Math Functions:
 * <ul>
 * <li>sin()</li>
 * <li>seed()</li>
 * <li>rand()</li>
 * <li>... finish this</li>
 * </ul>
 */
TclReturn TclStd_expr(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
    int i;
    TclValue *combined = NULL;
    TclValue_new(&combined, NULL);
    for (i = 1; i < argc; i++) {
        TclValue_append(combined, TclValue_str(argv[i]));
    }
    TclValue *value = TclStd_expression(vm, combined);
    TclValue_replace(ret, value);
    TclValue_delete(value);
    return TCL_OK;
}

/*tcl: if expression body ?elseif expression body ...? ?else body?
 * The if conditional expression evaluates body if expression evalutes to true.
 * Otherwise if an elseif clause is provided it's associated expression is
 * evaluated, evaluating it's body if true. Otherwise the body of the else
 * clause is evaluated if it is provided.
 */
TclReturn TclStd_if(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
    TclReturn status = TCL_OK;
    if (argc < 3) {
        return TCL_BADCMD;
    }
    TclValue *result = TclStd_expression(vm, argv[1]);
    if (TclValue_int(result)) {
        status = Tcl_eval(vm, TclValue_str(argv[2]), ret);
    } else {
        /* derp */
        if (argc == 4) {
            status = Tcl_eval(vm, TclValue_str(argv[3]), ret);
            goto done;
        }

        for (unsigned int i = 3; i < argc; ++i) {
            if (strcmp(TclValue_str(argv[i]), "elseif") == 0) {
                if (i + 2 >= argc) {
                    status = TCL_BADCMD;
                    goto done;
                }
                TclValue_delete(result);
                result = TclStd_expression(vm, argv[i+1]);
                if (TclValue_int(result)) {
                    status = Tcl_eval(vm, TclValue_str(argv[i+2]), ret);
                    goto done;
                }
                i += 2;
            } else if (strcmp(TclValue_str(argv[i]), "else") == 0) {
                if (i >= argc) {
                    status = TCL_BADCMD;
                    goto done;
                }
                status = Tcl_eval(vm, TclValue_str(argv[i+1]), ret);
                goto done;
            }
        }
        TclValue_set_null(ret);
    }
done:
    TclValue_delete(result);
    return status;
}

/*tcl: incr value ?amount?
 * TODO: Fix this function. Should be 'incr varname ?amount?'
 * Increments the value by amount and returns it. If amount is not specified
 * the default value of 1 will be used.
 */
TclReturn TclStd_incr(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
    if (argc < 2) {
        return TCL_BADCMD;
    }
    int amount = 1;
    if (argc > 2) {
        amount = TclValue_int(argv[2]);
    }
    int val = TclValue_int(argv[1]) + amount;
    char tmp[64];
    snprintf(tmp, 64, "%i", val);
    TclValue_set(ret, tmp);

    return TCL_OK;
}

/*tcl: source file
 * Evaluates the contents of file in the context of the current program.
 */
TclReturn TclStd_source(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
    TclReturn status;
    if (argc != 2) {
        return TCL_BADCMD;
    }

    FILE *fp = fopen(TclValue_str(argv[1]), "r");
    if (!fp) {
        fprintf(stderr, "File not found (%s)\n", TclValue_str(argv[1]));
        return TCL_EXCEPTION;
    }
    int start = ftell(fp);
    fseek(fp, 0, SEEK_END);
    int end = ftell(fp);
    int size = end - start;
    fseek(fp, 0, SEEK_SET);

    char *code = (char*)malloc(sizeof(char) * size + 1);
    fread(code, size, sizeof(char), fp);
    code[size] = '\0';

    fclose(fp);

    status = Tcl_eval(vm, code, ret);
    free(code);

    return status;
}

/*tcl: while guard body
 * Evaluates body repeatedly until guard is evaluated to false.
 */
TclReturn TclStd_while(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
    if (argc != 3) {
        return TCL_BADCMD;
    }

    TclReturn status = TCL_EXCEPTION;

    while (1) {
        TclValue *val = TclStd_expression(vm, argv[1]);
        int numval = TclValue_int(val);
        TclValue_delete(val);

        if (!numval) {
            break;
        } else {
            //TclValue_delete(&ret);
        }

        status = Tcl_eval(vm, TclValue_str(argv[2]), ret);

        if (status == TCL_BREAK) {
            status = TCL_OK;
            break;
        } else if (status == TCL_CONTINUE) {
            status = TCL_OK;
            continue;
        } else if (status == TCL_EXCEPTION) {
            break;
        } else if (status == TCL_RETURN) {
            break;
        }
    }

    return status;
}

/*tcl: for init guard step body
 * Initializes a loop by evaluating init, then evaluates guard. If guard
 * evaluates to true the body is then evaluated, once finished step is
 * evaluated. This is repeated (except for init) until the guard is evaluated
 * to false.
 */
TclReturn TclStd_for(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
    TclReturn status;
    if (argc != 5) {
        return TCL_BADCMD;
    }
    status = Tcl_eval(vm, TclValue_str(argv[1]), ret);

    TclValue *value = TclStd_expression(vm, argv[2]);
    while (TclValue_int(value)) {
        status = Tcl_eval(vm, TclValue_str(argv[4]), ret);
        TclValue *tmp = NULL;
        TclValue_new(&tmp, NULL);
        Tcl_eval(vm, TclValue_str(argv[3]), tmp);
        TclValue_delete(tmp);
        if (status == TCL_BREAK) {
            status = TCL_OK;
            break;
        } else if (status == TCL_CONTINUE) {
            status = TCL_OK;
            continue;
        } else if (status == TCL_EXCEPTION) {
            break;
        } else if (status == TCL_RETURN) {
            status = TCL_OK;
            break;
        }
    }
    TclValue_delete(value);

    return status;
}

/*tcl: foreach varname list body
 * Loops over each element of list sequentially, setting varname to the list
 * element's value and evaluating body.
 */
TclReturn TclStd_foreach(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
    TclReturn status = TCL_EXCEPTION;
    if (argc != 4) {
        return TCL_BADCMD;
    }
    List list;
    List_new(&list);
    Tcl_split(vm, TclValue_str(argv[2]), " \t", &list);

    int i;
    for (i = 0; i < List_size(&list); i++) {
        HashPair *p = Hash_get(vm->variables, TclValue_str(argv[1]));
        if (!p->data)
            TclValue_new((TclValue**)&p->data, List_index(&list, i)->data);
        else
            TclValue_set((TclValue*)p->data, List_index(&list, i)->data);
        status = Tcl_eval(vm, TclValue_str(argv[3]), ret);
        if (status == TCL_BREAK) {
            status = TCL_OK;
            break;
        } else if (status == TCL_CONTINUE) {
            status = TCL_OK;
            continue;
        } else if (status == TCL_EXCEPTION) {
            break;
        } else if (status == TCL_RETURN) {
            status = TCL_OK;
            break;
        }
    }

    List_delete(&list);
    return status;
}

/*tcl: break
 * Break out of a control flow statement immediately.
 */
TclReturn TclStd_break(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
    return TCL_BREAK;
}

/*tcl: continue
 * Move control to the top of a control flow statement immediately.
 */
TclReturn TclStd_continue(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
    return TCL_CONTINUE;
}

/*tcl: return ?value?
 * Returns from procedure/control structure.
 */
TclReturn TclStd_return(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
    if (argc == 1) {
        return TCL_RETURN;
    } else if (argc == 2) {
        TclValue_set(ret, TclValue_str(argv[1]));
        return TCL_RETURN;
    } else {
        return TCL_EXCEPTION;
    }
}

/*tcl: exec command
 * Execute the program named command.
 */
TclReturn TclStd_exec(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
    // TODO: return stdout of command
    if (argc < 2)
        return TCL_BADCMD;
    system(TclValue_str(argv[1]));
    return TCL_OK;
}

/*tcl: eval arg ?arg ...?
 * Concatenates arguments together then evaluates the resulting string as a
 * Tcl expression.
 */
TclReturn TclStd_eval(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
    if (argc < 2) {
        return TCL_BADCMD;
    }
    TclValue *command = NULL;
    TclValue_new(&command, NULL);
    int i;
    int status;
    for (i = 1; i < argc; i++) {
        TclValue_append(command, TclValue_str(argv[i]));
        TclValue_append(command, " ");
    }
    status = Tcl_eval(vm, TclValue_str(command), ret);
    TclValue_delete(command);
    return status;
}

/*tcl: list ?value ...?
 * Return a list filled with each argument provided to the command (after
 * substitution).
 */
TclReturn TclStd_list(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
    if (argc < 2)
        return TCL_BADCMD;
    int i;
    TclValue *list;
    TclValue_new_list(&list);

    for (i = 1; i < argc; i++) {
        TclValue_list_push(list, argv[i]);
    }
    TclValue_replace(ret, list);

    /* TclValue_set(ret, ""); */
    /* for (i = 1; i < argc; i++) { */
    /*     TclValue_append(ret, TclValue_str(argv[i])); */
    /*     TclValue_append(ret, " "); */
    /* } */
    return TCL_OK;
}

/*tcl: llength list
 * Return the number of elements in list.
 */
TclReturn TclStd_llength(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
    if (argc != 2) {
        return TCL_BADCMD;
    }
    List elms;
    List_new(&elms);
    Tcl_split(vm, TclValue_str(argv[1]), " \t", &elms);

    char tmp[32];
    snprintf(tmp, 32, "%i", List_size(&elms));
    TclValue_set(ret, tmp);

    List_delete(&elms);

    return TCL_OK;
}

/*tcl: lindex list index
 * Return the value of the element in list at index.
 */
TclReturn TclStd_lindex(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
    if (argc != 3) {
        return TCL_BADCMD;
    }
    List elms;
    List_new(&elms);
    Tcl_split(vm, TclValue_str(argv[1]), " \t", &elms);

    if (TclValue_int(argv[2]) < List_size(&elms))
        TclValue_set(ret, List_index(&elms, TclValue_int(argv[2]))->data);

    List_delete(&elms);

    return TCL_OK;
}

/*tcl: cd ?dirname?
 * Change the current working directory.
 */
TclReturn TclStd_cd(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
    if (argc > 2) {
        return TCL_BADCMD;
    }
    if (argc == 2) {
        chdir(TclValue_str(argv[1]));
    } else {
        char *home = getenv("HOME");
        if (home)
            chdir(home);
        else
            return TCL_EXCEPTION;
    }
    return TCL_OK;
}

/*tcl: pwd
 * Return the current working directory.
 */
TclReturn TclStd_pwd(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
    if (argc != 1) {
        return TCL_BADCMD;
    }
    char cwd[1024];
    if (getcwd(cwd, 1024) != NULL)
        TclValue_set(ret, cwd);
    else
        return TCL_EXCEPTION;
    return TCL_OK;
}

/*tcl: close fileid
 * Close the file associated with fileid which was created by the appropriate
 * open call.
 */
TclReturn TclStd_close(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
    if (argc != 2) {
        return TCL_BADCMD;
    }
    FILE *fp;
    fp = Hash_get(files, TclValue_str(argv[1]))->data;
    if (!fp) {
        return TCL_EXCEPTION;
    } else {
        fclose(fp);
        Hash_get(files, TclValue_str(argv[1]))->data = NULL;
    }
    Hash_remove(files, Hash_get(files, TclValue_str(argv[1])));
    return TCL_OK;
}

/*tcl: open file ?mode?
 * Returns a fileid associated with the file in read-only mode.
 *
 * The following predefined fileid's are available:
 * <ul>
 * <li>stdin - Standard input</li>
 * <li>stdout - Standard output</li>
 * <li>stderr - Standard error</li>
 * </ul>
 */
TclReturn TclStd_open(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
    if (argc < 2 || argc > 3) {
        return TCL_BADCMD;
    }
    FILE *fp = Hash_get(files, TclValue_str(argv[1]))->data;
    if (fp) {
        return TCL_EXCEPTION;
    }
    fp = fopen(TclValue_str(argv[1]), "r");
    Hash_get(files, TclValue_str(argv[1]))->data = fp;
    TclValue_set(ret, TclValue_str(argv[1]));
    return TCL_OK;
}

/*tcl: gets fileid ?varname?
 * Read and return a line of content from file associated with fileid. If
 * varname is specified the line is put into the variable varname.
 */
TclReturn TclStd_gets(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
    if (argc < 2 || argc > 3) {
        return TCL_BADCMD;
    }
    FILE *fp = Hash_get(files, TclValue_str(argv[1]))->data;
    if (!fp) {
        return TCL_EXCEPTION;
    }
    char line[1024];
    fgets(line, 1023, fp);
    line[1023] = '\0';
    if (argc == 3) {
        TclValue *value = NULL;
        HashPair *p = Hash_get(vm->variables, TclValue_str(argv[2]));
        if (!p->data) {
            TclValue_new(&value, NULL);
            p->data = value;
        }
        TclValue_set(value, line);
    } else {
        TclValue_set(ret, line);
    }
    return TCL_OK;
}

/*tcl: flush fileid
 * Flushes output from stream
 */
TclReturn TclStd_flush(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
    if (argc < 2) {
        return TCL_BADCMD;
    }
    FILE *fp = Hash_get(files, TclValue_str(argv[1]))->data;
    if (!fp) {
        return TCL_EXCEPTION;
    }
    fflush(fp);
    return TCL_OK;
}

/*tcl: string option arg
 * Perform option on string arg.
 *
 * Options:
 * <ul>
 * <li>length</li>
 * </ul>
 */
TclReturn TclStd_string(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
    if (argc < 3) {
        return TCL_BADCMD;
    }
    if (strcmp(TclValue_str(argv[1]), "length") == 0) {
        char tmp[32];
        snprintf(tmp, 32, "%i", (int)strlen(TclValue_str(argv[2])));
        TclValue_set(ret, tmp);
    } else {
        printf("'%s' option not supported\n", TclValue_str(argv[1]));
        return TCL_EXCEPTION;
    }

    return TCL_OK;
}

/*tcl: glob pattern
 * Glob pattern
 */
TclReturn TclStd_glob(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
    if (argc != 2) {
        return TCL_BADCMD;
    }
    int i;
    glob_t g;
    g.gl_offs = 0;
    if (glob(TclValue_str(argv[1]), 0, NULL, &g) != 0)
        return TCL_EXCEPTION;

    TclValue_set(ret, "");
    for (i = 0; i < g.gl_pathc; i++) {
        TclValue_append(ret, g.gl_pathv[i]);
        TclValue_append(ret, " ");
    }

    globfree(&g);
    return TCL_OK;
}

/*tcl: catch script ?varName?
 * Catch all exceptional return codes
 */
TclReturn TclStd_catch(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
    /* basically eval with an additional argument */
    if (argc < 2 || argc > 3) {
        return TCL_BADCMD;
    }
    TclValue *evalRet = NULL;
    TclValue_new(&evalRet, NULL);

    TclReturn status;
    status = Tcl_eval(vm, TclValue_str(argv[1]), evalRet);

    /* change value of varName */
    if (argc == 3) {
        /* try to set variable */
        TclValue *value = NULL;
        value = Tcl_getVariable(vm, TclValue_str(argv[2]));
        if (value) {
            TclValue_set(value, TclValue_str(evalRet));
        } else {
            TclValue_new_ref(&value, evalRet);
            Tcl_addVariable_(vm, TclValue_str(argv[2]), value);
        }
    }

    TclValue_set_int(ret, status);
    TclValue_delete(evalRet);

    return TCL_OK;
}

TclReturn TclStd_throw(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
  if (argc < 2) {
    return TCL_BADCMD;
  }

  if (argc == 2) {
    TclValue_replace(ret, argv[1]);
  }

  return TCL_EXCEPTION;
}

TclReturn TclStd_upvar(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
    if (argc < 3)
        return TCL_BADCMD;

    int level = 0;
    char *name = NULL;
    char *newName = NULL;

    if (argc >= 4) {
        level = TclValue_int(argv[1]);
        if (level < 0)
            return TCL_EXCEPTION;

        name = TclValue_str(argv[2]);
        newName = TclValue_str(argv[3]);
    } else {
        name = TclValue_str(argv[1]);
        newName = TclValue_str(argv[2]);
    }

    TclValue *v = Tcl_getVariableUp(vm, name, level);
    if (!v)
        return TCL_EXCEPTION;

    TclValue *new_v = NULL;
    TclValue_new_ref(&new_v, v);
    Tcl_addVariable_(vm, newName, new_v);

    return TCL_OK;
}


/**********************************************/

void TclStd_cleanup(void) {
    Hash_free(files);
    Hash_free(functions);
}

void TclStd_register(Tcl *vm) {
    Tcl_register(vm, "apply", TclStd_apply);
    Tcl_register(vm, "puts", TclStd_puts);
    Tcl_register(vm, "set", TclStd_set);
    Tcl_register(vm, "unset", TclStd_unset);
    Tcl_register(vm, "exit", TclStd_exit);
    Tcl_register(vm, "proc", TclStd_proc);
    Tcl_register(vm, "expr", TclStd_expr);
    Tcl_register(vm, "if", TclStd_if);
    Tcl_register(vm, "incr", TclStd_incr);
    Tcl_register(vm, "inc", TclStd_incr);
    Tcl_register(vm, "source", TclStd_source);
    Tcl_register(vm, "while", TclStd_while);
    Tcl_register(vm, "for", TclStd_for);
    Tcl_register(vm, "foreach", TclStd_foreach);
    Tcl_register(vm, "break", TclStd_break);
    Tcl_register(vm, "continue", TclStd_continue);
    Tcl_register(vm, "return", TclStd_return);
    Tcl_register(vm, "exec", TclStd_exec);
    Tcl_register(vm, "eval", TclStd_eval);
    Tcl_register(vm, "list", TclStd_list);
    Tcl_register(vm, "llength", TclStd_llength);
    Tcl_register(vm, "lindex", TclStd_lindex);
    Tcl_register(vm, "cd", TclStd_cd);
    Tcl_register(vm, "pwd", TclStd_pwd);
    Tcl_register(vm, "open", TclStd_open);
    Tcl_register(vm, "flush", TclStd_flush);
    Tcl_register(vm, "close", TclStd_close);
    Tcl_register(vm, "gets", TclStd_gets);
    Tcl_register(vm, "string", TclStd_string);
    Tcl_register(vm, "glob", TclStd_glob);
    Tcl_register(vm, "catch", TclStd_catch);
    Tcl_register(vm, "upvar", TclStd_upvar);

    files = Hash_malloc();
    HashPair *p = Hash_get(files, "stdout");
    p->data = stdout;
    p = Hash_get(files, "stdin");
    p->data = stdin;
    p = Hash_get(files, "stderr");
    p->data = stderr;

    functions = Hash_malloc();
    functions->tree.dealloc = TclStd_functions_dealloc;

    atexit(TclStd_cleanup);
}
