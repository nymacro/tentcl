/*
 * dstructs -- Expandable String
 * Copyright (C) 2006-2008  Aaron Marks
 */
#ifndef DSTRUCTS_ESTRING_H
#define DSTRUCTS_ESTRING_H

struct EString {
    char *string;
    int size;
};
typedef struct EString EString;

void EString_new(EString *self);
void EString_delete(EString *self);
EString *EString_malloc(void);
void EString_free(EString *self);
void EString_set(EString *self, char *value);
void EString_append(EString *self, char *value);
void EString_prepend(EString *self, char *value);
void EString_adjust(EString *self, int size);

#endif
