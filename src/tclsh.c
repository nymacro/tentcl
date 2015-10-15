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

#include "LineRead.h"
#include "tcl.h"
#include "List.h"
#include "Hash.h"
#include "std.h"
#include "ext.h"

#include "common.h"

static Tcl tcl;
static LineRead *lr;

/* Hacky compare function to replace Hash/BTree's standard compare. This is
 * used to find auto-complete */
int Hash_BTree_compareN_(BTreeNode *node, void *data) {
    return strncmp(((HashPair*)node->data)->name, data, strlen(data));
}

#define KEY_LEFT   (char)37
#define KEY_UP     (char)38
#define KEY_RIGHT  (char)39
#define KEY_DOWN   (char)40

/* LineRead */
int keyHandler(LineRead *self) {
    int len = strlen(self->buf);
    if (self->lastChar == '\t') {
        /* auto complete function names */
        char autoStr[128];
        
        /* get the string to auto complete */
        int i, autoLen = 0;
        for (i = len - 1; i > 0; i--) {
            if (isspace(self->buf[i])) {
                break;
            }
        }
        autoLen = len - i;
        if (autoLen <= 1 || autoLen >= 128) {
            return LR_KEYREMOVE;
        }
                
        /* copy out string to auto complete */
        strncpy(autoStr, &self->buf[i], autoLen);
        autoStr[autoLen] = '\0';
        
        /* search for a match */
        BTreeNodeCompareFunc tmp = tcl.functions->tree.compare;
        tcl.functions->tree.compare = Hash_BTree_compareN_;
        HashPair *pair = Hash_get(tcl.functions, autoStr);
        tcl.functions->tree.compare = tmp;
        
        /* check if we have a match */
        if (pair->data) {
            /* copy string to buffer and put characters to screen */
            int completeLen = strlen(pair->name) - autoLen;
            for (i = strlen(pair->name) - completeLen; i < strlen(pair->name); i++) {
                self->buf[len - autoLen + i] = pair->name[i];
                fputc(pair->name[i], stdout);
            }
            /* move bufp on and add space/null terminate string */
            self->bufp += completeLen;
            self->buf[self->bufp++] = ' ';
            fputc(' ', stdout);
            self->buf[self->bufp] = '\0';
        }
    } else {
        /* add normal character */
        if (self->lastChar == '\b' || self->lastChar == 0x7f) {
            /* erase last character */
            if (len > 0) {
                self->bufp--;
                self->buf[len - 1] = '\0';
                printf("\b \b");
            }
        } else if (self->lastChar == LR_ENDOFLINE && Tcl_isComplete(&tcl, self->buf)) {
            /* go to new line */
            printf("\r\f");
        } else {
            /* add character to buffer */
            self->buf[len] = self->lastChar;
            self->buf[len + 1] = '\0';
            self->bufp++;

            if (self->lastChar == '\n') {
                printf("\r\f");
            } else {
                fputc(self->lastChar, stdout);
            }
        }
    }
    /* flush output */
    fflush(stdout);
    /* remove all keys since we're handling it */
    return LR_KEYREMOVE;
}


/* LineRead */
int isComplete(LineRead *self) {
    if ((self->lastChar == LR_ENDOFLINE || self->lastChar == EOF) &&
        Tcl_isComplete(&tcl, self->buf))
        return 1;
    return 0;
}

/* Read-Eval-Print-Loop */
TclReturn repl(Tcl *vm, FILE *input) {
    char *prompt = Tcl_getVariable(vm, "tcl_prompt1");
    if (!prompt)
        prompt = "% ";
    
#ifdef NO_LINEREAD
    char line[1024];
    printf("%s", prompt);
    fgets(line, 1024, input);
#else
    printf("%s", prompt);
    char *line = NULL;
    system("stty raw -echo");
    line = LineRead_readLine(lr);
    system("stty -raw echo");
    if (line)
        LineRead_addHistory(lr, line);
#endif
    
    if (feof(stdin) || !line) {
        return TCL_OK;
    }
    TclReturn status = TCL_OK;
    TclValue ret = NULL;
    status = Tcl_eval(vm, line, &ret);
    // shell command execution if command doesn't exist
    if (status == TCL_BADCMD) {
        char *interactive = Tcl_getVariable(vm, "tcl_interactive");
        if (interactive && atoi(interactive)) {
            printf("trying to execute external command\n");
            system(line);
        }
    } else if (status == TCL_EXIT) {
	TclValue_delete(&ret);
        return status;
    }
    printf("-> %s\n", (ret == NULL) ? "NULL" : ret);
    TclValue_delete(&ret);

    return repl(vm, input); /* hope your compiler does TCO */
}

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
    
    /* Initialize GC */
#ifdef LEAK_CHECK
    GC_INIT();
#endif

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

    lr = LineRead_malloc();
    lr->isComplete = (LineReadIsComplete)isComplete;
    lr->keyHandler = (LineReadKeyHandler)keyHandler;
    status = repl(&tcl, stdin);
    LineRead_free(lr);

    printf("\n");
#ifdef LEAK_CHECK
    CHECK_LEAKS();
#endif
    printf("\n");

    return Tcl_statusToCode(status);
}
