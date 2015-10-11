/*
 * Tentcl -- Built-in Types
 * Copyright (C) 2006-2015 Aaron Marks. All Rights Reserved.
 */
#include "value.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "List.h"

#include "common.h"

void TclValue_new(TclValue *value, char *init) {
#ifdef DEBUG
    printf("TclValue_new: Creating new variable (initial value provided=%i)\n", init?1:0);
#endif

    if (init) {
        *value = (TclValue)malloc(sizeof(char)*strlen(init) + 1);
        strcpy(*value, init);
    } else {
        *value = (TclValue)malloc(sizeof(char));
        (*value)[0] = 0;
    }
}

void TclValue_delete(TclValue *value) {
    if (value && *value) {
        free(*value);
        *value = NULL;
    } else {
        fprintf(stderr, "TclValue_delete called on NULL\n");
    }
}

void TclValue_set(TclValue *value, char *data) {
    TclValue_delete(value);
    TclValue_new(value, data);
}

void TclValue_set_(TclValue *value, char *data) {
    TclValue_delete(value);
    *value = data;
}

void TclValue_replace(TclValue *value, TclValue *value2) {
    TclValue_delete(value);
    *value = *value2;
}

void TclValue_append(TclValue *value, char *data) {
    *value = (TclValue)realloc(*value, strlen(*value) + strlen(data) + 1);
    strcat(*value, data);
}

void TclValue_prepend(TclValue *value, char *data) {
    char *newstr = (char*)malloc(strlen(*value) + strlen(data) + 1);
    strcpy(newstr, data);
    strcpy(&newstr[strlen(data)], *value);

    TclValue_delete(value);

    TclValue_set_(value, data);
}
