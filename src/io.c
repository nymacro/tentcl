/*
 * Tentcl -- IO Types & Functions
 * Copyright (C) 2006-2018 Aaron Marks. All Rights Reserved.
 */

#include <stdio.h>

#include "io.h"

static Hash *files;

/*tcl: close fileid
 * Close the file associated with fileid which was created by the appropriate
 * open call.
 */
TclReturn TclIO_close(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
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
TclReturn TclIO_open(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
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
TclReturn TclIO_gets(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
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
TclReturn TclIO_flush(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
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

/*tcl: puts ?-nonewline? ?fileid? ?output?
 * Outputs output to the standard output stream. If -nonewline is specified
 * the cursor will stay on the same line. If fileid is specified it will
 * output to the appropriate file.
 */
TclReturn TclIO_puts(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
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


void TclIO_cleanup(void) {
    Hash_free(files);
}

void TclIO_register(Tcl *vm) {
    Tcl_register(vm, "open", TclIO_open);
    Tcl_register(vm, "flush", TclIO_flush);
    Tcl_register(vm, "close", TclIO_close);
    Tcl_register(vm, "gets", TclIO_gets);
    Tcl_register(vm, "puts", TclIO_puts);

    files = Hash_malloc();
    HashPair *p = Hash_get(files, "stdout");
    p->data = stdout;
    p = Hash_get(files, "stdin");
    p->data = stdin;
    p = Hash_get(files, "stderr");
    p->data = stderr;

    atexit(TclIO_cleanup);
}
