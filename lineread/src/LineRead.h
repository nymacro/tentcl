/*
 * lineread
 * Copyright (C) 2008-2015 Aaron Marks. All Rights Reserved.
 */
#ifndef LINEREAD_H
#define LINEREAD_H

/* TODO:
 * - Look into how to do user data nicely;
 */

#include "List.h"

#define LR_ENDOFLINE    '\n'
#define LR_KEYREMOVE    0
#define LR_KEYKEEP      1

typedef int (*LineReadIsComplete)(void *self);
typedef int (*LineReadKeyHandler)(void *self);

/* lineread state structure */
struct LineRead {
    LineReadIsComplete isComplete;
    LineReadKeyHandler keyHandler;

    /* user data; does not get freed */
    void *data;

    /* command history */
    List *history;

    /* internal string buffer */
    int lastChar; /* last character processed */
    int bufp; /* index to current character in buffer */
    char buf[1024];
};
typedef struct LineRead LineRead;

void LineRead_new(LineRead *self);
LineRead *LineRead_malloc(void);
void LineRead_delete(LineRead *self);
void LineRead_free(LineRead *self);
void LineRead_setData(LineRead *self, void *data);

void LineRead_addHistory(LineRead *self, char *string);
char *LineRead_readLine(LineRead *self);

#endif

