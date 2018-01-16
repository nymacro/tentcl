/*
 * Tentcl -- Shell
 * Copyright (C) 2006-2018 Aaron Marks. All Rights Reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#include "repl.h"

struct FilterData {
    List *matches;
    char *search;
};
typedef struct FilterData FilterData;

static void Hash_BTree_filter_(BTreeNode *node, void *data) {
    FilterData *fd = (FilterData*)data;
    char *fname = ((HashPair*)node->data)->name; /* function name */
    if (strncmp(fname, fd->search, strlen(fd->search)) == 0) {
        List_add(fd->matches, fname);
    }
}

#define KEY_LEFT   (char)37
#define KEY_UP     (char)38
#define KEY_RIGHT  (char)39
#define KEY_DOWN   (char)40

/* LineRead */
static int keyHandler(LineRead *self) {
    Tcl *vm = (Tcl*)self->data;

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
        List *matches = List_malloc();
        FilterData fd = { matches, autoStr };
        Hash_map(vm->functions, Hash_BTree_filter_, &fd);

        if (List_size(matches) == 1) {
            /* copy string to buffer and put characters to screen */
            char *name = List_first(matches)->data;
            int completeLen = strlen(name) - autoLen;
            for (i = strlen(name) - completeLen; i < strlen(name); i++) {
                self->buf[len - autoLen + i] = name[i];
                fputc(name[i], stdout);
            }
            /* move bufp on and add space/null terminate string */
            self->bufp += completeLen;
            self->buf[self->bufp++] = ' ';
            fputc(' ', stdout);
            self->buf[self->bufp] = '\0';
        } else {
            /* print matches */
            printf("\r\f");
            for (i = 0; i < List_size(matches); i++) {
                printf("%s ", (char*)List_index(matches, i)->data);
            }
            printf("\r\f");
            printf("> %s", autoStr);
        }
        List_free(matches);

    } else {
        /* add normal character */
        if (self->lastChar == '\b' || self->lastChar == 0x7f) {
            /* erase last character */
            if (len > 0) {
                self->bufp--;
                self->buf[len - 1] = '\0';
                printf("\b \b");
            }
        } else if (self->lastChar == LR_ENDOFLINE && Tcl_isComplete(vm, self->buf)) {
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
static int isComplete(LineRead *self) {
    Tcl *vm = (Tcl*)self->data;

    if ((self->lastChar == LR_ENDOFLINE || self->lastChar == EOF) &&
        Tcl_isComplete(vm, self->buf))
        return 1;
    return 0;
}

LineRead *TclRepl_readerMalloc(Tcl *vm) {
    LineRead *lr = LineRead_malloc();
    lr->isComplete = (LineReadIsComplete)isComplete;
    lr->keyHandler = (LineReadKeyHandler)keyHandler;
    lr->data       = (void*)vm;
    return lr;
}

/* Read-Eval-Print-Loop */
TclReturn TclRepl_repl_(Tcl *vm, FILE *input, LineRead *lr) {
    TclValue *promptVar = Tcl_getVariable(vm, "tcl_prompt1");
    char *prompt = "% ";
    if (promptVar && promptVar->container)
        prompt = promptVar->container->value;

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
    TclValue *ret = NULL;
    TclValue_new(&ret, NULL);
    status = Tcl_eval(vm, line, ret);
    // shell command execution if command doesn't exist
    if (status == TCL_BADCMD) {
        TclValue *interactive = Tcl_getVariable(vm, "tcl_interactive");
        if (interactive && TclValue_int(interactive)) {
            printf("trying to execute external command\n");
            system(line);
        }
    } else if (status == TCL_EXIT || status == TCL_INTERRUPT) {
        TclValue_delete(ret);
        return status;
    }
    printf("-> %s\n", (ret->container == NULL) ? "NULL" : ret->container->value);
    TclValue_delete(ret);

    return TclRepl_repl_(vm, input, lr); /* hope your compiler does TCO */
}

TclReturn TclRepl_repl(Tcl *vm, FILE *input) {
    LineRead *reader = TclRepl_readerMalloc(vm);
    TclReturn status = TclRepl_repl_(vm, input, reader);
    LineRead_free(reader);
    return status;
}
