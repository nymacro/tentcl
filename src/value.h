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

char *TclValue_str(TclValue *v);
int TclValue_int(TclValue *v);
int TclValue_null(TclValue *v);

#endif
