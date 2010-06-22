/*
 * Tentcl -- Shell
 * Copyright (C) 2006-2008  Aaron Marks. All Rights Reserved.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
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

/* LineRead */
int keyHandler(LineRead *self) {
    int len = strlen(self->buf);
    if (len > 0 && self->lastChar == '\t') {
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
void repl(Tcl *vm, FILE *input) {
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
        return;
    }
    TclReturn status;
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
		return;
	}
    printf("-> %s\n", (ret == NULL || strlen(ret) == 0) ? "NULL" : ret);
    
    repl(vm, input);
}

void destroy(void) {
    Tcl_delete(&tcl);
}

int main(int argc, char *argv[]) {
    /* Initialize GC */
#ifdef LEAK_CHECK
    GC_INIT();
#endif

    /* Initialize tentcl */
    Tcl_new(&tcl);
    atexit(destroy);
    TclStd_register(&tcl);
    TclExt_register(&tcl);

    /* Script mode */
    if (argc > 1) {
        FILE *fp = fopen(argv[1], "r");
        if (!fp) {
            fprintf(stderr, "Error opening file '%s'\n", argv[1]);
            return 1;
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
        TclValue ret;
        status = Tcl_eval(&tcl, buf, &ret);
        free(buf);
        return 0;
    }

    /* Interactive mode */
    printf("tentcl interactive shell " TENTCL_VERSION "\n\tCopyright (C) 2006-2008 Aaron Marks. All Rights Reserved.\n");
    printf("Features: ");
#ifndef NO_LINEREAD
    printf("lineread ");
#endif
#ifdef WITH_LIBRARIES
    printf("sharelibs ");
#endif
    printf("\n");

    lr = LineRead_malloc();
    lr->isComplete = (LineReadIsComplete)isComplete;
    lr->keyHandler = (LineReadKeyHandler)keyHandler;
    repl(&tcl, stdin);
    LineRead_free(lr);

    printf("\n");
#ifdef LEAK_CHECK
    CHECK_LEAKS();
#endif
    printf("\n");
    return 0;
}
