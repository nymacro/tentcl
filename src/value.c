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

void TclValue_new(TclValue **value, char *init) {
#ifdef DEBUG
    printf("TclValue_new: Creating new variable (initial value provided=%i)\n", init?1:0);
#endif
    TclValue *v = (TclValue*)malloc(sizeof(TclValue));

    if (init) {
        v->container = (struct TclValueRef*)malloc(sizeof(struct TclValueRef));
        v->container->value = (char*)malloc(sizeof(char)*strlen(init) + 1);
        v->container->ref = 1;
        strcpy(v->container->value, init);
    } else {
        v->container = NULL;
    }

    *value = v;
}

void TclValue_delete(TclValue *value) {
    if (!value)
        return;
    if (value->container)
    {
        --value->container->ref;
        if (value->container->ref == 0) {
            if (value->container->value) {
                free(value->container->value);
                free(value->container);
            }
            free(value);
        }
    }
}

void TclValue_new_ref(TclValue **value, TclValue *existing)
{
    TclValue_new(value, NULL);
    (*value)->container = existing->container;
    TclValue_ref(existing);
}

void TclValue_ref(TclValue *value) {
    if (value->container)
        ++value->container->ref;
}

void TclValue_set(TclValue *value, char *data) {
    if (value->container)
        free(value->container->value);
    else
    {
        value->container = (struct TclValueRef*)malloc(sizeof(struct TclValueRef));
        value->container->ref = 1;
    }

    value->container->value = strdup(data);
}

void TclValue_set_(TclValue *value, char *data) {
    if (value->container->value)
        free(value->container->value);
    value->container->value = data;
}

void TclValue_replace(TclValue *value, TclValue *value2) {
    if (value->container)
        free(value->container->value);
    TclValue_ref(value2);
    value->container = value2->container;
}

void TclValue_append(TclValue *value, char *data) {
    if (data == NULL)
        return;

    if (value->container) {
        value->container->value = (char*)realloc(value->container->value,
                                                 strlen(value->container->value) + strlen(data) + 1);
        strcat(value->container->value, data);
    } else {
        value->container = (struct TclValueRef*)malloc(sizeof(struct TclValueRef));
        value->container->value = strdup(data);
        value->container->ref = 1;
    }
}

void TclValue_prepend(TclValue *value, char *data) {
    if (data == NULL)
        return;

    char *newstr = (char*)malloc(strlen(value->container->value) + strlen(data) + 1);
    strcpy(newstr, data);
    strcpy(&newstr[strlen(data)], value->container->value);

    TclValue_set_(value, data);
}

/* TclValue TclValue_const(char *value) { */
/*     TclValue v = { value, 0 }; */
/*     return v; */
/* } */

char *TclValue_str(TclValue *v) {
    static char empty[] = "";
    if (v->container)
        return v->container->value;
    else
        return empty;
}

int TclValue_int(TclValue *v) {
    if (v->container->value)
        return atoi(v->container->value);
    else
        return 0;
}
