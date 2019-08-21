/*
 * Tentcl -- Shell
 * Copyright (C) 2006-2018 Aaron Marks. All Rights Reserved.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <getopt.h>

#include "tcl.h"
#include "repl.h"
#include "List.h"
#include "Hash.h"
#include "std.h"
#include "ext.h"
#include "io.h"
#include "regexp.h"

#include "common.h"

static Tcl tcl;

void destroy(void) {
    Tcl_delete(&tcl);
}

TclReturn evalFile(char *filename) {
    TclReturn status = TCL_OK;
    char *buf = NULL;
    TclValue *ret = NULL;
    FILE *fp = fopen(filename, "r");

    TclValue_new(&ret, NULL);

    if (!fp) {
        fprintf(stderr, "Error opening file '%s'\n", filename);
        status = TCL_EXCEPTION;
        goto end;
    }
    int start = ftell(fp);
    fseek(fp, 0, SEEK_END);
    int end = ftell(fp);
    int size = end - start;
    fseek(fp, 0, SEEK_SET);

    buf = (char*)malloc(sizeof(char) * (size + 1));
    if (!buf) {
        status = TCL_OOM;
        goto end;
    }
    fread(buf, size, sizeof(char), fp);
    buf[size] = '\0';

    status = Tcl_eval(&tcl, buf, ret);

end:
    if (fp)
        fclose(fp);
    if (ret)
        TclValue_delete(ret);
    if (buf)
        free(buf);

    return status;
}

void about(void) {
    printf("tentcl interactive shell " TENTCL_VERSION "\n\tCopyright (C) 2006-2018 Aaron Marks. All Rights Reserved.\n"
           "Features: "
#ifndef NO_LINEREAD
           "lineread "
#endif
#ifdef WITH_LIBRARIES
           "sharelibs "
#endif
           "\n");
}

void usage(void) {
    printf("Usage:\n"
           "\ttclsh [options] [script name]\n"
           "\n"
           "Options:\n"
           "\t-e<source-code>                          execute source before running script or shell\n"
           "\t-I<source-file> 	--include=<source>     include source file before executing script or shell\n"
           "\t-h             	--help                 print this message\n"
           "                                        \n");
}

int main(int argc, char *argv[]) {
    TclReturn status = TCL_OK;
    TclValue *ret;

    /* Initialize tentcl */
    Tcl_new(&tcl);
    atexit(destroy);
    TclStd_register(&tcl);
    TclExt_register(&tcl);
    TclRegexp_register(&tcl);
    TclIO_register(&tcl);

    TclValue_new(&ret, NULL);

    /* Parse command line params */
    int c = 0;
    int option_index = 0;
    while (1) {
        static struct option long_options[] = {
            {"include", required_argument, 0, 0},
            {"e", required_argument, 0, 0},
            {"help", no_argument, 0, 0},
            {0, 0, 0, 0}
        };
        c = getopt_long(argc, argv, "I:e:h", long_options, &option_index);
        if (c == -1) {
            break;
        }
        switch (c) {
        case 0: /* long options */
            if (strcmp(long_options[option_index].name, "include") == 0) {
                printf("including %s\n", optarg);
                status = evalFile(optarg);
                if (status != TCL_OK) {
                    fprintf(stderr, "Failed to include file: %s\n", Tcl_returnString(status));
                    exit(1);
                }
            } if (strcmp(long_options[option_index].name, "help") == 0) {
                usage();
                exit(0);
            }
            break;
        case 'e':
            status = Tcl_eval(&tcl, optarg, ret);
            break;
        case 'I':
            status = evalFile(optarg);
            if (status != TCL_OK) {
                fprintf(stderr, "Failed to include file: %s\n", Tcl_returnString(status));
                exit(1);
            }
            break;
        case 'h':
            usage();
            exit(0);
            break;
        default:
            usage();
            exit(1);
            break;
        }
    }

    if (optind < argc) {
        /* Script mode */
        while (optind < argc) {
            if (strcmp(argv[optind], "-") == 0) {
                status = TclRepl_repl(&tcl, stdin);
            } else {
                status = evalFile(argv[optind]);
            }
            optind++;
            if (status != TCL_OK)
                break;
        }
    } else {
        /* Interactive mode */
        about();
        status = TclRepl_repl(&tcl, stdin);
        printf("\n");
    }

    if (status != TCL_OK && status != TCL_EXIT)
        fprintf(stderr, "Error: %s", Tcl_returnString(status));
    return Tcl_statusToCode(status);
}
