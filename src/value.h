/*
 * Tentcl -- Built-in Types
 * Copyright (C) 2006-2018 Aaron Marks. All Rights Reserved.
 */
#ifndef TENTCL_VALUE_H
#define TENTCL_VALUE_H

#include <string.h>

typedef struct TclValueRef {
    char *value;      /* Tagged pointer */
    unsigned int ref; /* Reference count */
} TclValueRef;
struct TclValue {
    struct TclValueRef *container;
};
typedef struct TclValue TclValue;

typedef struct {
    char *type_str;
    void *ptr;
    void (*free)(void *);
} TclValueObject;

/* forward declare circular */
typedef int (*TclFunction_)(void*, int, TclValue*[], TclValue*);

/* Tcl Value Primitive Functions */
void TclValue_new(TclValue**, char*); /* Create Tcl value */
void TclValue_new_ref(TclValue **, TclValue *); /* New value referencing existing value */
void TclValue_detach(TclValue *); /* Copy value and remove shared references */
void TclValue_new_object(TclValue **value, char *type_str, void *obj, void (*free)(void *));
void TclValue_new_function(TclValue **value, TclFunction_ function);
void TclValue_new_int(TclValue **value, int i);
void TclValue_delete(TclValue*); /* Destroy Tcl value */
void TclValue_ref(TclValue *); /* Increment reference count on value */
void TclValue_set(TclValue*, char*); /* Set Tcl value */
void TclValue_set_raw(TclValue *value, char *data, size_t len);
void TclValue_set_null(TclValue *value);
void TclValue_set_int(TclValue *value, int i);
void TclValue_replace(TclValue*, TclValue*); /* Replace existing value */
void TclValue_swap(TclValue *value, TclValue *value2); /* Swap existing values */
void TclValue_append(TclValue *, char*); /* Append string to end of value */
void TclValue_prepend(TclValue *, char*); /* Prepend string to start of value */

void TclValue_new_list(TclValue **value);
void TclValue_list_push(TclValue *value, TclValue *v);
void TclValue_list_push_str(TclValue *value, char *str);
TclValue *TclValue_list_pop(TclValue *value);
TclValue *TclValue_list_shift(TclValue *value);
int TclValue_list_size(TclValue *value);
TclValue *TclValue_list_elt(TclValue *value, int idx);

#define TCL_VALUE_TAG_MASK 0x07
#define TCL_VALUE_TAG_BITS 3
#define TCL_VALUE_TAG_REMOVE(value) ((unsigned long)(value) & ~TCL_VALUE_TAG_MASK)
/* #define TCL_VALUE_TAG_SHIFT(value) ((long)(value) >> TCL_VALUE_TAG_BITS) */
#define TCL_VALUE_TAG(value, type) ((unsigned long)(value) | (type))

typedef enum {
    TCL_VALUE_STR = 0x00,
    TCL_VALUE_INT = 0x01,
    TCL_VALUE_OBJ = 0x02,
    TCL_VALUE_FUN = 0x03,
    TCL_VALUE_LIST = 0x04,
    TCL_VALUE_NULL = 255
} TclValueType;

TclValueType TclValue_type(TclValue *v);
TclValue *TclValue_coerce(TclValue *v, TclValueType new_type);

void *TclValue_ptr(TclValue *v);
void *TclValue_object_ptr(TclValue *v);
char *TclValue_str(TclValue *v);
char *TclValue_str_(TclValue *v);
char *TclValue_str_esc(TclValue *v);
int TclValue_int(TclValue *v);
int TclValue_null(TclValue *v);
TclFunction_ TclValue_fun(TclValue *v);

int TclValue_str_cmp(TclValue *v, char *str);
int TclValue_type_object_cmp(TclValue *v, char *type_str);

char *TclValue_type_str(TclValue *type);

#endif
