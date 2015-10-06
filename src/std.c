/*
 * Tentcl -- Standard Function Library
 * Copyright (C) 2006-2008  Aaron Marks. All Rights Reserved.
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
        TclValue_delete(&fn->code);
        free(node->data);
    }
}

TclReturn TclStd_userFuncall(Tcl *vm, int argc, char *argv[], TclValue *ret) {
    TclReturn status;
    TclUserFunction *f = Hash_get(functions, argv[0])->data;
    int i;

    if (List_size(f->args) != argc - 1) {
        printf("Invalid argument list (takes %i, given %i)\n", List_size(f->args), argc - 1);
        for (i = 0; i < List_size(f->args); i++) {
            printf("%i: '%s' %i\n", i, (char*)List_index(f->args, i)->data, (int)strlen((char*)List_index(f->args, i)->data));
        }
        return TCL_EXCEPTION;
    }
    
    Tcl_pushNamespace(vm);
    
    for (i = 0; i < List_size(f->args); i++) {
        HashPair *p = Hash_get(vm->variables, List_index(f->args, i)->data);
        p->data = argv[i + 1];
        //printf("%i '%s' '%s'\n", i, List_index(&f->args, i)->data, argv[i + 1]);
    }
    status = Tcl_eval(vm, f->code, ret);
    
    Tcl_popNamespace(vm);
    
    return status;
}

TclValue TclStd_expression(Tcl *vm, TclValue val) {
    val = Tcl_expand(vm, val);
    int ret = Math_eval(val);
    TclValue_delete(&val);
    char tmp[128];
    snprintf(tmp, 128, "%i", ret);
    TclValue retval;
    TclValue_new(&retval, tmp);
    return retval;
}


/**********************************************/

/*tcl: puts ?-nonewline? ?fileid? ?output?
 * Outputs output to the standard output stream. If -nonewline is specified
 * the cursor will stay on the same line. If fileid is specified it will
 * output to the appropriate file.
 */
TclReturn TclStd_puts(Tcl *vm, int argc, TclValue argv[], TclValue *ret) {
    int nonewline = 0;
    int i = 1;
    if (argc > 4) {
        return TCL_EXCEPTION;
    }

    if (argc > 1 && strcmp(argv[1], "-nonewline") == 0) {
        nonewline = 1;
        i++;
    }

    if (argc - nonewline > 2) {
        HashPair *p = Hash_get(files, argv[i]);
        if (!p->data) {
            fprintf(stderr, "Stream '%s' does not exist\n", argv[i]);
            return TCL_EXCEPTION;
        }
        fputs(argv[++i], p->data);
        if (!nonewline)
            fputc('\n', p->data);
    } else if (argc - nonewline == 2) {
        fputs(argv[i], stdout);
        if (!nonewline)
            fputc('\n', stdout);
    }
    return TCL_OK;
}

/*tcl: set varName ?value?
 * Set the variable named varName to value. If value is not provided the
 * existing value of varName is returned.
 */
TclReturn TclStd_set(Tcl *vm, int argc, TclValue argv[], TclValue *ret) {
    /*
    printf("GETTING HASH PAIR FOR '%s' from %x\n", argv[1], vm->variables);
    printf("EXISTS: %i\n", Hash_exists(vm->variables, argv[1]));
    */
    if (argc < 2) {
        return TCL_EXCEPTION;
    } else if (argc < 3) {
        HashPair *p = Hash_get(vm->variables, argv[1]);
        if (p->data)
            *ret = p->data;
        else
            printf("can't read \"%s\": no such variable\n", argv[1]);
    } else {
        HashPair *p = Hash_get(vm->variables, argv[1]);
        TclValue value;
        TclValue_new(&value, argv[2]);
        p->data = value;
        *ret = p->data;
    }
    return TCL_OK;
}

/*tcl: unset varName
 * Removes the variable varName and its associated value from the program.
 */
TclReturn TclStd_unset(Tcl *vm, int argc, TclValue argv[], TclValue *ret) {
    // TODO: extend Hash to remove the key to value (which is NULL after unset)
    if (argc != 2) {
        return TCL_EXCEPTION;
    }
    HashPair *p = Hash_get(vm->variables, argv[1]);
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
TclReturn TclStd_exit(Tcl *vm, int argc, TclValue argv[], TclValue *ret) {
    return TCL_EXIT;
}

/*tcl: proc name varlist body
 * TODO: variable length arguments
 *
 * Defines a procedure called name as a Tcl command. When this command is
 * executed each argument specified will be bound to each element of the list
 * varlist before the body is evaluated.
 */
TclReturn TclStd_proc(Tcl *vm, int argc, TclValue argv[], TclValue *ret) {
    if (argc != 4) {
        return TCL_EXCEPTION;
    }
    TclUserFunction *f = (TclUserFunction*)malloc(sizeof(TclUserFunction));
    f->args = List_malloc();
    Tcl_split(vm, argv[2], " \t\n", f->args);
    
    TclValue_new(&f->code, argv[3]);
    Hash_get(functions, argv[1])->data = f;

    Tcl_register(vm, argv[1], TclStd_userFuncall);

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
TclReturn TclStd_expr(Tcl *vm, int argc, TclValue argv[], TclValue *ret) {
    int i;
    TclValue combined;
    TclValue_new(&combined, NULL);
    for (i = 1; i < argc; i++) {
        TclValue_append(&combined, argv[i]);
    }
    *ret = TclStd_expression(vm, combined);
    return TCL_OK;
}

/*tcl: if expression body ?elseif expression body ...? ?else body?
 * The if conditional expression evaluates body if expression evalutes to true.
 * Otherwise if an elseif clause is provided it's associated expression is
 * evaluated, evaluating it's body if true. Otherwise the body of the else
 * clause is evaluated if it is provided.
 */
TclReturn TclStd_if(Tcl *vm, int argc, TclValue argv[], TclValue *ret) {
    TclReturn status;
    if (argc < 3) {
        return TCL_EXCEPTION;
    }
    TclValue result = TclStd_expression(vm, argv[1]);
    if (atoi(result)) {
        status = Tcl_eval(vm, argv[2], ret);
    } else {
        if (argc == 5 && strcmp(argv[3], "else") == 0) {
            status = Tcl_eval(vm, argv[4], ret);
        } else if (argc == 4) {
            status = Tcl_eval(vm, argv[3], ret);
        } else {
            status = TCL_EXCEPTION;
        }
    }
    return status;
}

/*tcl: incr value ?amount?
 * TODO: Fix this function. Should be 'incr varname ?amount?'
 * Increments the value by amount and returns it. If amount is not specified
 * the default value of 1 will be used.
 */
TclReturn TclStd_incr(Tcl *vm, int argc, TclValue argv[], TclValue *ret) {
    if (argc < 2) {
        return TCL_EXCEPTION;
    }
    int amount = 1;
    if (argc > 2) {
        amount = atoi(argv[2]);
    }
    int val = atoi(argv[1]) + amount;
    char tmp[64];
    snprintf(tmp, 64, "%i", val);
    TclValue_set(ret, tmp);

    return TCL_OK;
}

/*tcl: source file
 * Evaluates the contents of file in the context of the current program.
 */
TclReturn TclStd_source(Tcl *vm, int argc, TclValue argv[], TclValue *ret) {
    TclReturn status;
    if (argc != 2) {
        return TCL_EXCEPTION;
    }

    FILE *fp = fopen(argv[1], "r");
    if (!fp) {
        fprintf(stderr, "File not found (%s)\n", argv[1]);
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

    status = Tcl_eval(vm, code, ret);
    free(code);

    return status;
}

/*tcl: while guard body
 * Evaluates body repeatedly until guard is evaluated to false.
 */
TclReturn TclStd_while(Tcl *vm, int argc, TclValue argv[], TclValue *ret) {
    if (argc != 3) {
        return TCL_EXCEPTION;
    }
    
    TclReturn status;
    
    while (1) {
        TclValue val = TclStd_expression(vm, argv[1]);
        int numval = atoi(val);
        TclValue_delete(&val);
        
        if (!numval) {
            break;
        } else {
            //TclValue_delete(&ret);
        }
        
        status = Tcl_eval(vm, argv[2], ret);
        
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
TclReturn TclStd_for(Tcl *vm, int argc, TclValue argv[], TclValue *ret) {
    TclReturn status;
    if (argc != 5) {
        return TCL_EXCEPTION;
    }
    status = Tcl_eval(vm, argv[1], ret);
    while (atoi(TclStd_expression(vm, argv[2]))) {
        status = Tcl_eval(vm, argv[4], ret);
        TclValue tmp;
        TclValue_new(&tmp, NULL);
        Tcl_eval(vm, argv[3], &tmp);
        TclValue_delete(&tmp);
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
    return status;
}

/*tcl: foreach varname list body
 * Loops over each element of list sequentially, setting varname to the list
 * element's value and evaluating body.
 */
TclReturn TclStd_foreach(Tcl *vm, int argc, TclValue argv[], TclValue *ret) {
    TclReturn status;
    if (argc != 4) {
        return TCL_EXCEPTION;
    }
    List list;
    List_new(&list);
    Tcl_split(vm, argv[2], " \t", &list);

    int i;
    for (i = 0; i < List_size(&list); i++) {
        HashPair *p = Hash_get(vm->variables, argv[1]);
        if (!p->data)
            TclValue_new((TclValue*)&p->data, List_index(&list, i)->data);
        else
            TclValue_set((TclValue*)&p->data, List_index(&list, i)->data);
        status = Tcl_eval(vm, argv[3], ret);
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
TclReturn TclStd_break(Tcl *vm, int argc, TclValue argv[], TclValue *ret) {
    return TCL_BREAK;
}

/*tcl: continue
 * Move control to the top of a control flow statement immediately.
 */
TclReturn TclStd_continue(Tcl *vm, int argc, TclValue argv[], TclValue *ret) {
    return TCL_CONTINUE;
}

/*tcl: return ?value?
 * Returns from procedure/control structure.
 */
TclReturn TclStd_return(Tcl *vm, int argc, TclValue argv[], TclValue *ret) {
    if (argc == 1) {
        return TCL_RETURN;
    } else if (argc == 2) {
        TclValue_set(ret, argv[1]);
        return TCL_RETURN;
    } else {
        return TCL_EXCEPTION;
    }
}

/*tcl: exec command
 * Execute the program named command.
 */
TclReturn TclStd_exec(Tcl *vm, int argc, TclValue argv[], TclValue *ret) {
    // TODO: return stdout of command
    if (argc < 2)
        return TCL_EXCEPTION;
    system(argv[1]);
    return TCL_OK;
}

/*tcl: eval arg ?arg ...?
 * Concatenates arguments together then evaluates the resulting string as a
 * Tcl expression.
 */
TclReturn TclStd_eval(Tcl *vm, int argc, TclValue argv[], TclValue *ret) {
    if (argc < 2) {
        return TCL_EXCEPTION;
    }
    TclValue command;
    TclValue_new(&command, NULL);
    int i;
    int status;
    for (i = 1; i < argc; i++) {
        TclValue_append(&command, argv[i]);
        TclValue_append(&command, " ");
    }
    status = Tcl_eval(vm, command, ret);
    TclValue_delete(&command);
    return status;
}

/*tcl: list ?value ...?
 * Return a list filled with each argument provided to the command (after
 * substitution).
 */
TclReturn TclStd_list(Tcl *vm, int argc, TclValue argv[], TclValue *ret) {
    if (argc < 2)
        return TCL_EXCEPTION;
    int i;
    for (i = 1; i < argc; i++) {
        TclValue_append(ret, argv[i]);
        TclValue_append(ret, " ");
    }
    return TCL_OK;
}

/*tcl: llength list
 * Return the number of elements in list.
 */
TclReturn TclStd_llength(Tcl *vm, int argc, TclValue argv[], TclValue *ret) {
    if (argc != 2) {
        return TCL_EXCEPTION;
    }
    List elms;
    List_new(&elms);
    Tcl_split(vm, argv[1], " \t", &elms);

    char tmp[32];
    snprintf(tmp, 32, "%i", List_size(&elms));
    TclValue_set(ret, tmp);

    List_delete(&elms);

    return TCL_OK;
}

/*tcl: lindex list index
 * Return the value of the element in list at index.
 */
TclReturn TclStd_lindex(Tcl *vm, int argc, TclValue argv[], TclValue *ret) {
    if (argc != 3) {
        return TCL_EXCEPTION;
    }
    List elms;
    List_new(&elms);
    Tcl_split(vm, argv[1], " \t", &elms);

    if (atoi(argv[2]) < List_size(&elms))
        TclValue_set(ret, List_index(&elms, atoi(argv[2]))->data);

    List_delete(&elms);

    return TCL_OK;
}

/*tcl: cd ?dirname?
 * Change the current working directory.
 */
TclReturn TclStd_cd(Tcl *vm, int argc, TclValue argv[], TclValue *ret) {
    if (argc > 2) {
        return TCL_EXCEPTION;
    }
    if (argc == 2) {
        chdir(argv[1]);
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
TclReturn TclStd_pwd(Tcl *vm, int argc, TclValue argv[], TclValue *ret) {
    if (argc != 1) {
        return TCL_EXCEPTION;
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
TclReturn TclStd_close(Tcl *vm, int argc, TclValue argv[], TclValue *ret) {
    if (argc != 2) {
        return TCL_EXCEPTION;
    }
    FILE *fp;
    fp = Hash_get(files, argv[1])->data;
    if (!fp) {
        return TCL_EXCEPTION;
    } else {
        fclose(fp);
        Hash_get(files, argv[1])->data = NULL;
    }
    Hash_remove(files, Hash_get(files, argv[1]));
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
TclReturn TclStd_open(Tcl *vm, int argc, TclValue argv[], TclValue *ret) {
    if (argc < 2 || argc > 3) {
        return TCL_EXCEPTION;
    }
    FILE *fp = Hash_get(files, argv[1])->data;
    if (fp) {
        return TCL_EXCEPTION;
    }
    fp = fopen(argv[1], "r");
    Hash_get(files, argv[1])->data = fp;
    TclValue_set(ret, argv[1]);
    return TCL_OK;
}

/*tcl: gets fileid ?varname?
 * Read and return a line of content from file associated with fileid. If
 * varname is specified the line is put into the variable varname.
 */
TclReturn TclStd_gets(Tcl *vm, int argc, TclValue argv[], TclValue *ret) {
    if (argc < 2 || argc > 3) {
        return TCL_EXCEPTION;
    }
    FILE *fp = Hash_get(files, argv[1])->data;
    if (!fp) {
        return TCL_EXCEPTION;
    }
    char line[1024];
    fgets(line, 1023, fp);
    line[1023] = '\0';
    if (argc == 3) {
        TclValue value;
        HashPair *p = Hash_get(vm->variables, argv[2]);
        if (!p->data) {
            TclValue_new(&value, NULL);
            p->data = value;
        }
        TclValue_set((TclValue*)&p->data, line);
    } else {
        TclValue_set(ret, line);
    }
    return TCL_OK;
}

/*tcl: flush fileid
 * Flushes output from stream
 */
TclReturn TclStd_flush(Tcl *vm, int argc, TclValue argv[], TclValue *ret) {
    if (argc < 2) {
        return TCL_EXCEPTION;
    }
    FILE *fp = Hash_get(files, argv[1])->data;
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
TclReturn TclStd_string(Tcl *vm, int argc, TclValue argv[], TclValue *ret) {
    if (argc < 3) {
        return TCL_EXCEPTION;
    }
    if (strcmp(argv[1], "length") == 0) {
        char tmp[32];
        snprintf(tmp, 32, "%i", (int)strlen(argv[2]));
        TclValue_set(ret, tmp);
    } else {
        printf("'%s' option not supported\n", argv[1]);
        return TCL_EXCEPTION;
    }
    
    return TCL_OK;
}

/*tcl: glob pattern
 * Glob pattern
 */
TclReturn TclStd_glob(Tcl *vm, int argc, TclValue argv[], TclValue *ret) {
    if (argc != 2) {
        return TCL_EXCEPTION;
    }
    int i;
    glob_t g;
    g.gl_offs = 0;
    if (glob(argv[1], 0, NULL, &g) != 0)
        return TCL_EXCEPTION;

    TclValue_set(ret, "");
    for (i = 0; i < g.gl_pathc; i++) {
        TclValue_append(ret, g.gl_pathv[i]);
        TclValue_append(ret, " ");
    }
    
    globfree(&g);
    return TCL_OK;
}

/**********************************************/

void TclStd_cleanup(void) {
    Hash_free(files);
    Hash_free(functions);
}

void TclStd_register(Tcl *vm) {
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

