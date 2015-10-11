/*
 * dstructs -- Expandable String
 * Copyright (C) 2006-2015 Aaron Marks
 */

#include "EString.h"

#include <stdlib.h>
#include <string.h>

void EString_new(EString *self) {
    self->string = NULL;
}

void EString_delete(EString *self) {
    if (self->string)
        free(self->string);
}

EString *EString_malloc(void) {
    EString *self = (EString*)malloc(sizeof(EString));
    EString_new(self);
    return self;
}

void EString_free(EString *self) {
    EString_delete(self);
    free(self);
}

/*
 * Set the value of the string
 */
void EString_set(EString *self, char *value) {
    if (self->string)
        free(self->string);
    self->string = strdup(value);
}

/*
 * Append to the end of the string
 */
void EString_append(EString *self, char *value) {
    if (self->string) {
        self->string = realloc(self->string, strlen(self->string) + strlen(value) + 1);
        strcat(self->string, value);
    } else {
        EString_set(self, value);
    }
}

/*
 * Prepend a value onto the front of a string
 */
void EString_prepend(EString *self, char *value) {
    if (self->string) {
        char *newstr = (char*)malloc(strlen(self->string) + strlen(value) + 1);
        strcpy(newstr, value);
        strcpy(&newstr[strlen(value)], self->string);
        self->string = newstr;
    } else {
        EString_set(self, value);
    }
}
