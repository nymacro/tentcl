#include <stdio.h>
#include <string.h>
#include "LineRead.h"

int keyHandler(LineRead *self) {
    int len = strlen(self->buf);
    printf("0x%x", self->lastChar);
    if (len > 0 && self->lastChar == '\t') {
        /* auto complete */
        if (self->buf[len - 1] == 'a') {
            self->buf[len] = 'b';
            self->buf[len + 1] = 'c';
            self->buf[len + 2] = '\0';
            printf("bc");
            self->bufp+=2;
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
        } else if (self->lastChar == LR_ENDOFLINE) {
            /* go to new line */
            printf("\r\f");
        } else {
            /* add character to buffer */
            self->buf[len] = self->lastChar;
            self->buf[len + 1] = '\0';
            self->bufp++;

            fputc(self->lastChar, stdout);
        }
    }
    /* flush output */
    fflush(stdout);
    /* remove all keys since we're handling it */
    return LR_KEYREMOVE;
}

int main(int argc, char *argv[]) {
    LineRead *lr;
    lr = LineRead_malloc();
    lr->keyHandler = (LineReadKeyHandler)keyHandler;

    system("stty raw -echo");
    printf(">> ");
    printf("%s\n", LineRead_readLine(lr));
    system("stty -raw echo");

    LineRead_free(lr);
    return 0;
}

