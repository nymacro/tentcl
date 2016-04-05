/*
 * Tentcl -- Shell
 * Copyright (C) 2006-2015 Aaron Marks. All Rights Reserved.
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

#include "common.h"

static Tcl tcl;

void destroy(void) {
    Tcl_delete(&tcl);
}

TclReturn evalFile(char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "Error opening file '%s'\n", filename);
        return TCL_EXCEPTION;
    }
    int start = ftell(fp);
    fseek(fp, 0, SEEK_END);
    int end = ftell(fp);
    int size = end - start;
    fseek(fp, 0, SEEK_SET);
    
    char *buf = (char*)malloc(sizeof(char) * (size + 1));
    fread(buf, size, sizeof(char), fp);
    buf[size] = '\0';
    fclose(fp);
    
    TclReturn status;
    TclValue ret = NULL;
    status = Tcl_eval(&tcl, buf, &ret);
    TclValue_delete(&ret);
    free(buf);
    
    return status;
}

void about(void) {
    printf("tentcl interactive shell " TENTCL_VERSION "\n\tCopyright (C) 2006-2015 Aaron Marks. All Rights Reserved.\n"
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
           "\t-I<source>     --include=<source>     include source before executing script or shell\n"
           "                                        \n");
}

int main(int argc, char *argv[]) {
    TclReturn status = TCL_OK;
    
    /* Initialize tentcl */
    Tcl_new(&tcl);
    atexit(destroy);
    TclStd_register(&tcl);
    TclExt_register(&tcl);
    
    /* Parse command line params */
    int c = 0;
    int option_index = 0;
    while (1) {
        static struct option long_options[] = {
            {"include", required_argument, 0, 0},
            {"help", no_argument, 0, 0},
            {0, 0, 0, 0}
        };
        c = getopt_long(argc, argv, "I:h", long_options, &option_index);
        if (c == -1) {
            break;
        }
        switch (c) {
        case 0: /* long options */
            if (strcmp(long_options[option_index].name, "include") == 0) {
                printf("including %s\n", optarg);
                status = evalFile(optarg);
            } if (strcmp(long_options[option_index].name, "help") == 0) {
                usage();
                exit(0);
            }
            break;
        case 'I':
            status = evalFile(optarg);
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
    
    /* Script mode */
    if (optind < argc) {
        while (optind < argc) {
            status = evalFile(argv[optind++]);
            if (status != TCL_OK) {
                return Tcl_statusToCode(status);
            }
        }
        return Tcl_statusToCode(status);
    }
    
    /* Interactive mode */
    about();
    
    status = TclRepl_repl(&tcl, stdin);
    
    printf("\n");
    
    return Tcl_statusToCode(status);
}
