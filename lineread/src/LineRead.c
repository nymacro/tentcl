/*
 * lineread
 * Copyright (C) 2008-2018 Aaron Marks. All Rights Reserved.
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

    self->data = NULL;
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
    List_unshift(self->history, string);
}

/* LineRead_readLine:
 * Read a line from stdin
 */
char *LineRead_readLine(LineRead *self) {
    self->bufp = 0;
    self->buf[0] = '\0';
    self->lastHistory = -1;
    do {
        int len = strlen(self->buf);
        int historyLength = List_size(self->history);
        int special = 0;

        fflush(stdout);

        /* get the character */
        self->lastChar = getchar();

        /* check for special keys */
        if (self->lastChar == '\033') {
            getchar(); /* skip the [ */
            self->lastChar = getchar();
            special = 1;
        }

        /* printf("\n%i%s=", self->lastChar, special ? "s" : ""); */

        if (special || self->lastChar == 14 || self->lastChar == 16) {
            switch (self->lastChar) {
            case 16:
            case 'A': /* up arrow */
                if (self->lastHistory <= historyLength - 1)
                    self->lastHistory++;
                break;

            case 14:
            case 'B': /* down arrow */
                if (self->lastHistory > 0)
                    self->lastHistory--;
                break;

            default:
                continue;
            }

            if (historyLength > 0 && self->lastHistory < historyLength) {
                char *item = (char*)List_index(self->history, self->lastHistory)->data;
                strcpy(self->buf, item);

                /* redisplay */
                printf("\r%*s\r> %s", len, "", self->buf);
            }
            continue;
        }

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

