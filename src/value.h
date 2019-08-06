/*
 * Tentcl -- Built-in Types
 * Copyright (C) 2006-2018 Aaron Marks. All Rights Reserved.
 */
#ifndef TENTCL_VALUE_H
#define TENTCL_VALUE_H

#include <string.h>

typedef struct TclValueRef {
    char *value;      /* Stringly typed */
    unsigned int ref; /* Reference count */
} TclValueRef;
struct TclValue {
    struct TclValueRef *container;
};
typedef struct TclValue TclValue;

/* Tcl Value Primitive Functions */
void TclValue_new(TclValue**, char*); /* Create Tcl value */
void TclValue_new_ref(TclValue **, TclValue *); /* New value referencing existing value */
void TclValue_delete(TclValue*); /* Destroy Tcl value */
void TclValue_ref(TclValue *); /* Increment reference count on value */
void TclValue_set(TclValue*, char*); /* Set Tcl value */
void TclValue_set_raw(TclValue *value, char *data, size_t len);
void TclValue_replace(TclValue*, TclValue*); /* Replace existing value */
void TclValue_append(TclValue *, char*); /* Append string to end of value */
void TclValue_prepend(TclValue *, char*); /* Prepend string to start of value */

/* TclValue TclValue_const(char *value); */

#define TCL_VALUE_TAG_MASK 0x07
#define TCL_VALUE_TAG_BITS 3
#define TCL_VALUE_TAG_REMOVE(value) ((int)((value) >> TCL_VALUE_TAG_BITS))

typedef enum {
    TCL_VALUE_STR = 0x00,
    TCL_VALUE_INT = 0x01,
    TCL_VALUE_NULL = 255
} TclValueType;

TclValueType TclValue_type(TclValue *v);
TclValue *TclValue_coerce(TclValue *v, TclValueType new_type);

char *TclValue_str(TclValue *v);
int TclValue_int(TclValue *v);
int TclValue_null(TclValue *v);

int TclValue_str_cmp(TclValue *v, char *str);

#endif
