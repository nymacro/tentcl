/*
 * lineread
 * Copyright (C) 2008-2015 Aaron Marks. All Rights Reserved.
 */
#include "LineRead.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int LineRead_List_history_compare_(ListNode *node, void *data) {
    return 1;
}

void LineRead_List_history_alloc_(ListNode *node, void *data) {
    node->data = strdup(data);
}

void LineRead_List_history_dealloc_(ListNode *node) {
    free(node->data);
}

/* LineRead_isComplete_:
 * Default "complete" function.
 */
int LineRead_isComplete_(LineRead *self) {
    if (self->lastChar == LR_ENDOFLINE || self->lastChar == '\r' ||
        self->lastChar == EOF)
        return 1;
    return 0;
}

/* LineRead_keyHandler_:
 * Default key handler function.
 */
int LineRead_keyHandler_(LineRead *self) {
    return LR_KEYKEEP;
}

/* LineRead_new:
 * Initialise a static LineRead
 */
void LineRead_new(LineRead *self) {
    self->history = List_malloc();
    self->history->alloc = LineRead_List_history_alloc_;
    self->history->dealloc = LineRead_List_history_dealloc_;
    self->history->compare = LineRead_List_history_compare_;

    self->isComplete = (LineReadIsComplete)LineRead_isComplete_;
    self->keyHandler = (LineReadKeyHandler)LineRead_keyHandler_;
}

/* LineRead_malloc:
 * Allocate and initialise a LineRead object
 */
LineRead *LineRead_malloc(void) {
    LineRead *self = (LineRead*)malloc(sizeof(LineRead));
    LineRead_new(self);
    return self;
}

/* LineRead_delete:
 * Delete a statically allocated LineRead
 */
void LineRead_delete(LineRead *self) {
    List_free(self->history);
}

/* LineRead_free:
 * Free a dynamically allocated LineRead
 */
void LineRead_free(LineRead *self) {
    LineRead_delete(self);
    free(self);
}

/* LineRead_addHistory:
 * Add string to history
 */
void LineRead_addHistory(LineRead *self, char *string) {
    List_add(self->history, string);
}

/* LineRead_readLine:
 * Read a line from stdin
 */
char *LineRead_readLine(LineRead *self) {
    self->bufp = 0;
    self->buf[0] = '\0';
    do {
        /* get the character */
        self->lastChar = getchar();
        
        /* filter/substitute characters */
        if (self->lastChar == '\r')
            self->lastChar = '\n';
        else if (self->lastChar == 0x04)
            self->lastChar = EOF;

        /* check */
        if (self->lastChar == EOF && self->bufp == 0) {
            return NULL;
        }
        
        /* call key handler */
        if (self->keyHandler(self) != LR_KEYREMOVE) {
            /* default action with key (add to buffer) */
            self->buf[self->bufp++] = self->lastChar;
            self->buf[self->bufp] = '\0';
        }
    } while (!self->isComplete(self));
    return self->buf;
}

