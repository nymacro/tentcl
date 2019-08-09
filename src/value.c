/*
 * Tentcl -- Built-in Types
 * Copyright (C) 2006-2018 Aaron Marks. All Rights Reserved.
 */
#include "value.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "List.h"

#include "common.h"

static void TclValue_free_value_(TclValue *value) {
    TclValueObject *obj;
    switch (TclValue_type(value)) {
    case TCL_VALUE_STR:
        free(value->container->value);
        break;
    case TCL_VALUE_OBJ:
        obj = (TclValueObject*)TCL_VALUE_TAG_REMOVE(value->container->value);
        if (obj->free)
            obj->free(obj->ptr);
        free(obj);
        break;
    default:
        break;
    }
    value->container->value = NULL;
}

static void TclValue_unref_(TclValue *value) {
    if (value->container)
    {
        --value->container->ref;
        if (value->container->ref == 0) {
            TclValue_free_value_(value);
            free(value->container);
            value->container = NULL;
        }
    }
}

void TclValue_new_(TclValue **value) {
    TclValue *v = (TclValue*)malloc(sizeof(TclValue));

    v->container = (struct TclValueRef*)malloc(sizeof(struct TclValueRef));
    v->container->value = NULL;
    v->container->ref = 1;

    *value = v;
}

void TclValue_new(TclValue **value, char *init) {
    TclValue *v = (TclValue*)malloc(sizeof(TclValue));
    TclValue_new_(&v);

    if (init) {
        v->container->value = (char*)malloc(sizeof(char)*strlen(init) + 1);
        strcpy(v->container->value, init);
    }

    *value = v;
}

TclValueObject *TclValueObject_new(char *type_str, void *obj, void (*free)(void *)) {
    TclValueObject *v = (TclValueObject*)malloc(sizeof(TclValueObject));
    v->type_str = type_str;
    v->ptr = obj;
    v->free = free;
    return (TclValueObject*)TCL_VALUE_TAG(v, TCL_VALUE_OBJ);
}

void TclValue_new_object(TclValue **value, char *type_str, void *obj, void (*free)(void *)) {
    TclValue_new_(value);
    (*value)->container->value = (char*)TclValueObject_new(type_str, obj, free);
}

void TclValue_delete(TclValue *value) {
    if (!value)
        return;
    if (value->container)
    {
        TclValue_unref_(value);
    }
    free(value);
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
    if (!value->container) {
        value->container = (struct TclValueRef*)malloc(sizeof(struct TclValueRef));
        value->container->value = NULL;
        value->container->ref = 1;
    }
    TclValue_free_value_(value);
    if (data) {
        value->container->value = strdup(data);
    }
}

void TclValue_set_raw(TclValue *value, char *data, size_t len) {
    TclValue_set(value, NULL);
    value->container->value = (char*)malloc(sizeof(char)*len+1);
    memcpy(value->container->value, data, len);
    value->container->value[len] = 0;
}

void TclValue_set_(TclValue *value, char *data) {
    TclValue_free_value_(value);
    value->container->value = data;
}

void TclValue_set_null(TclValue *value) {
    TclValue_free_value_(value);
    value->container->value = NULL;
}

void TclValue_set_object(TclValue *value, char *type_str, void *f, void (*free)(void *)) {
    TclValue *obj;
    TclValue_new_object(&obj, type_str, f, free);
    TclValue_replace(value, obj);
}

void TclValue_replace(TclValue *value, TclValue *value2) {
    TclValue_unref_(value);
    TclValue_ref(value2);
    value->container = value2->container;
}

void TclValue_append(TclValue *value, char *data) {
    if (data == NULL)
        return;

    TclValue_coerce(value, TCL_VALUE_STR);

    if (!TclValue_null(value) && value->container) {
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

    TclValue_coerce(value, TCL_VALUE_STR);

    if (!TclValue_null(value)) {
        char *newstr = (char*)malloc(strlen(value->container->value) + strlen(data) + 1);
        strcpy(newstr, data);
        strcpy(&newstr[strlen(data)], value->container->value);
        TclValue_set_(value, newstr);
    } else {
        TclValue_set_(value, strdup(data));
    }
}

/* TclValue TclValue_const(char *value) { */
/*     TclValue v = { value, 0 }; */
/*     return v; */
/* } */

TclValueType TclValue_type(TclValue *v) {
    if (TclValue_null(v))
        return TCL_VALUE_NULL;
    return TCL_VALUE_TAG_MASK & (unsigned int)v->container->value;
}

TclValue *TclValue_coerce(TclValue *v, TclValueType new_type) {
    int int_val;
    char *str_val;
    TclValueType old_type = TclValue_type(v);

    /* str -> int */
    if (old_type == TCL_VALUE_STR && new_type == TCL_VALUE_INT) {
        int_val = (int)strtol(v->container->value, NULL, 10);
        TclValue_free_value_(v);
        v->container->value = (char *)(((long)int_val << TCL_VALUE_TAG_BITS) | TCL_VALUE_INT);
    }
    /* int -> str */
    if (old_type == TCL_VALUE_INT && new_type == TCL_VALUE_STR) {
        str_val = (char*)malloc(sizeof(char) * 20);
        snprintf(str_val, 20, "%i", TclValue_int(v));
        v->container->value = str_val;
    }
    return v;
}

char *TclValue_str(TclValue *v) {
    static char empty[] = "NULL";
    static char int_buf[256]; /* not thread safe */

    TclValue_coerce(v, TCL_VALUE_STR);

    switch (TclValue_type(v)) {
    case TCL_VALUE_INT:
        snprintf(int_buf, sizeof(int_buf) - 1, "%i", TclValue_int(v));
        return int_buf;
    case TCL_VALUE_STR:
        return v->container->value;
    case TCL_VALUE_OBJ:
        return ((TclValueObject*)TCL_VALUE_TAG_REMOVE(v->container->value))->type_str;
    default:
        return empty;
    }
}

int TclValue_int(TclValue *v) {
    TclValue_coerce(v, TCL_VALUE_INT);

    switch (TclValue_type(v)) {
    case TCL_VALUE_INT:
        return (int)((long)v->container->value >> TCL_VALUE_TAG_BITS);
    case TCL_VALUE_STR:
        return atoi(v->container->value);
    default:
        return 0;
    }
}

int TclValue_null(TclValue *v) {
    if (v->container == NULL || (v->container && v->container->value == NULL))
        return 1;
    return 0;
}

int TclValue_str_cmp(TclValue *v, char *str) {
  return strcmp(TclValue_str(v), str);
}

int TclValue_type_object_cmp(TclValue *v, char *type_str) {
    if (TclValue_type(v) == TCL_VALUE_OBJ) {
        TclValueObject *obj = (TclValueObject*)TCL_VALUE_TAG_REMOVE(v->container->value);
        return strcmp(obj->type_str, type_str);
    } else {
        return -1;
    }
}
