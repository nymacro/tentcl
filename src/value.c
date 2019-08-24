/*
 * tentcl -- built-in types
 * copyright (c) 2006-2018 aaron marks. all rights reserved.
 */
#include "value.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "List.h"

#include "common.h"

char *TclValue_str_unesc(TclValue *v);

static void TclValue_free_value_(TclValue *value) {
    List *list;
    TclValueObject *obj;
    switch (TclValue_type(value)) {
    case TCL_VALUE_STR:
        free(value->container->value);
        break;
    case TCL_VALUE_OBJ:
        obj = (TclValueObject*)TclValue_ptr(value);
        if (obj->free)
            obj->free(obj);
        free(obj);
        break;
    case TCL_VALUE_LIST:
        list = (List*)TclValue_ptr(value);
        List_free(list);
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
    TclValue *v;
    TclValue_new_(&v);

    if (init) {
        v->container->value = (char*)malloc(sizeof(char)*strlen(init) + 1);
        strcpy(v->container->value, init);
    }

    *value = v;
}

void TclValue_new_int(TclValue **value, int i) {
    TclValue_new(value, NULL);
    TclValue_set_int(*value, i);
}

TclValueObject *TclValueObject_new(char *type_str, void *obj, void (*free)(void *)) {
    TclValueObject *v = (TclValueObject*)malloc(sizeof(TclValueObject));
    v->type_str = type_str;
    v->ptr = obj;
    v->free = free;
    return v;
}

void TclValue_new_object(TclValue **value, char *type_str, void *obj, void (*free)(void *)) {
    TclValue_new_(value);
    (*value)->container->value = (char*)TCL_VALUE_TAG(TclValueObject_new(type_str, obj, free), TCL_VALUE_OBJ);
}

void TclValue_new_function(TclValue **value, TclFunction_ function) {
    TclValue_new_(value);
    (*value)->container->value = (char*)TCL_VALUE_TAG(function, TCL_VALUE_FUN);
}

void TclValue_detach(TclValue *value) {
    if (TclValue_null(value))
        return;
    if (value->container->ref == 1)
        return;

    TclValue *tmp;
    TclValue_new(&tmp, TclValue_str(value));
    TclValue_replace(value, tmp);
    TclValue_delete(tmp);
}

static void List_TclValue_delete(ListNode *node) {
    /*TclValue_delete((TclValue*)node->data);*/
}
static void List_TclValue_alloc(ListNode *node, void *data) {
    node->data = data;
}
void TclValue_new_list(TclValue **value) {
    TclValue_new_(value);
    List *list = List_malloc();
    list->alloc = List_TclValue_alloc;
    list->dealloc = List_TclValue_delete;
    (*value)->container->value = (char*)TCL_VALUE_TAG(list, TCL_VALUE_LIST);
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

void TclValue_set_int(TclValue *value, int i) {
    TclValue_free_value_(value);
    value->container->value = (char*)TCL_VALUE_TAG((unsigned long)i << 3, TCL_VALUE_INT);
}

static void object_free(TclValueObject *obj) {
    free(obj->ptr);
}

void TclValue_set_object(TclValue *value, char *type_str, void *f, void (*free)(void *)) {
    TclValue *obj;
    TclValue_new_object(&obj, type_str, f, (void (*)(void*))object_free);
    TclValue_replace(value, obj);
    TclValue_delete(obj);
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

void TclValue_list_push(TclValue *value, TclValue *v) {
    if (v == NULL)
        return;

    TclValue_coerce(value, TCL_VALUE_LIST);

    List *list = (List*)TclValue_ptr(value);
    TclValue *n;
    TclValue_new_ref(&n, v);
    List_push(list, n);
}

void TclValue_list_push_str(TclValue *value, char *str) {
    TclValue *v;
    TclValue_new(&v, str);
    TclValue_list_push(value, v);
    TclValue_delete(v);
}

int TclValue_list_size(TclValue *value) {
    TclValue_coerce(value, TCL_VALUE_LIST);
    /* if (TclValue_type(value) != TCL_VALUE_LIST) */
    /*     return -1; /\* oops? *\/ */

    List *list = (List*)TclValue_ptr(value);
    return List_size(list);
}

TclValue *TclValue_list_elt(TclValue *value, int idx) {
    TclValue_coerce(value, TCL_VALUE_LIST);
    /* if (TclValue_type(value) != TCL_VALUE_LIST) */
    /*     return NULL; /\* oops? *\/ */

    List *list = (List*)TclValue_ptr(value);
    unsigned int size = List_size(list);
    if (idx < 0) {
        idx = size - idx;
    }

    if (idx >= size) {
        return NULL; /* oops? */
    }

    return List_index(list, idx)->data;
}

TclValue *TclValue_list_pop(TclValue *value) {
    TclValue_coerce(value, TCL_VALUE_LIST);

    List *list = (List*)TclValue_ptr(value);
    TclValue *ret = NULL;
    if (List_size(list) > 0) {
        ret = List_last(list)->data;
        TclValue_ref(ret);
        List_pop(list);
    } else {
        TclValue_new(&ret, NULL);
    }
    return ret;
}

TclValue *TclValue_list_shift(TclValue *value) {
    TclValue_coerce(value, TCL_VALUE_LIST);

    List *list = (List*)TclValue_ptr(value);
    TclValue *ret = NULL;
    if (List_size(list) > 0) {
        ret = List_first(list)->data;
        TclValue_ref(ret);
        List_shift(list);
    } else {
        TclValue_new(&ret, NULL);
    }
    return ret;
}

int TclValue_list_join_(TclValue *value, char *buf, unsigned int buf_len) {
    if (TclValue_type(value) != TCL_VALUE_LIST) {
        return snprintf(buf, buf_len, "WRONG TYPE");
    }
    unsigned int used = 0;

    List *list = (List*)TclValue_ptr(value);
    unsigned int size = List_size(list);

    for (unsigned int i = 0; i < size; i++) {
        if (buf_len == 0)
            return -1;
        char *s = (i == size-1) ? "" : " ";
        unsigned int t = snprintf(buf, buf_len, "%s%s",
                                  TclValue_str_esc(List_index(list, i)->data),
                                  s);
        used += t;
        buf_len -= t;
        buf += t;
    }

    buf[used] = '\0';

    return used;
}

#define TCL_VALUE_GET_TAG(value) ((unsigned long)(value) & TCL_VALUE_TAG_MASK)
TclValueType TclValue_type(TclValue *v) {
    if (TclValue_null(v))
        return TCL_VALUE_NULL;
    return TCL_VALUE_GET_TAG(v->container->value);
}

int Tcl_split(void*, char*, char*, List*);

TclValue *TclValue_coerce(TclValue *v, TclValueType new_type) {
    int int_val;
    char *str_val;
    TclValueType old_type = TclValue_type(v);

    if (old_type == new_type)
        return v;

    /* str -> int */
    if (old_type == TCL_VALUE_STR && new_type == TCL_VALUE_INT) {
        int_val = (int)strtol(v->container->value, NULL, 10);
        TclValue_free_value_(v);
        v->container->value = (char *)((((unsigned long)int_val) << TCL_VALUE_TAG_BITS) | TCL_VALUE_INT);
    } else
    /* int -> str */
    if (old_type == TCL_VALUE_INT && new_type == TCL_VALUE_STR) {
        str_val = (char*)malloc(sizeof(char) * 20);
        snprintf(str_val, 20, "%i", TclValue_int(v));
        v->container->value = str_val;
    } else
    /* any -> list */
    if (new_type == TCL_VALUE_LIST) {
        TclValue *list;
        TclValue_new_list(&list);

        List *elms = List_malloc();
        Tcl_split(NULL, TclValue_str_unesc(v), " \t", elms);

        for (unsigned int i = 0; i < List_size(elms); i++) {
            TclValue_list_push_str(list, List_index(elms, i)->data);
        }

        TclValue_replace(v, list);
        TclValue_delete(list);
    } else
    /* null -> any */
    if (old_type == TCL_VALUE_NULL) {
        /* don't do anything */
    } else
    /* any -> str */
    if (new_type == TCL_VALUE_STR) {
        str_val = strdup(TclValue_str(v));
        TclValue_free_value_(v);
        v->container->value = str_val;
    }
    return v;
}

void *TclValue_ptr(TclValue *v) {
    if (TclValue_type(v) != TCL_VALUE_NULL)
        return (void*)TCL_VALUE_TAG_REMOVE(v->container->value);
    else
        return NULL;
}

char *TclValue_str_(TclValue *v) {
    static char empty[] = "";
    static unsigned char int_buf_idx = 0;
    static char int_buf_ary[32][1024]; /* not thread safe */
    int_buf_idx = (int_buf_idx + 1) % 32;
    char *int_buf = &int_buf_ary[int_buf_idx];

    switch (TclValue_type(v)) {
    case TCL_VALUE_INT:
        snprintf(int_buf, sizeof(int_buf) - 1, "%i", TclValue_int(v));
        return int_buf;
    case TCL_VALUE_STR:
        return v->container->value;
    case TCL_VALUE_OBJ:
        snprintf(int_buf, sizeof(int_buf) - 1, "<%s>",
                 ((TclValueObject*)TclValue_ptr(v))->type_str);
        return int_buf;
    case TCL_VALUE_FUN:
        snprintf(int_buf, sizeof(int_buf) - 1, "<FUNCTION %p>", TclValue_fun(v));
        return int_buf;
    case TCL_VALUE_LIST:
        if (TclValue_list_join_(v, int_buf, sizeof(int_buf_ary[0])) < 0) {
            abort();
        }
        return int_buf;
    default:
        return empty;
    }
}

char *TclValue_str(TclValue *v) {
    /* TclValue_coerce(v, TCL_VALUE_STR); */
    return TclValue_str_(v);
}

char *TclValue_str_unesc(TclValue *v) {
    static char buf[1024]; /* having a real GC would be wonderful */
    size_t buf_len = sizeof(buf) - 1;
    char *str = TclValue_str_(v);
    size_t str_len = strlen(str);

    /* unescape any space characters */
    size_t i = 0, b = 0;
    for (; i < str_len; i++) {
        if (i > buf_len) { abort(); }

        if (str[i] == '\\') {
            continue;
        }

        if (i > buf_len) { abort(); }

        buf[b++] = str[i];
    }

    buf[b] = '\0';

    return buf;
}

char *TclValue_str_esc(TclValue *v) {
    static char buf[1024]; /* having a real GC would be wonderful */
    size_t buf_len = sizeof(buf) - 1;
    char *str = TclValue_str_(v);
    size_t str_len = strlen(str);

    /* escape any space characters */
    size_t i = 0, b = 0;
    for (; i < str_len; i++) {
        if (i > buf_len) { abort(); }

        if (isspace(str[i]) || str[i] == '\\') {
            buf[b++] = '\\';
        }

        if (i > buf_len) { abort(); }

        buf[b++] = str[i];
    }

    buf[b] = '\0';

    return buf;
}

int TclValue_int(TclValue *v) {
    TclValue_coerce(v, TCL_VALUE_INT);

    switch (TclValue_type(v)) {
    case TCL_VALUE_INT:
        return (int)((unsigned long)v->container->value >> TCL_VALUE_TAG_BITS);
    case TCL_VALUE_STR:
        return atoi(v->container->value);
    default:
        return 0;
    }
}

static int oops(void *vm, int argc, TclValue *argv[], TclValue *ret) {
    return 0;
}

TclFunction_ TclValue_fun(TclValue *v) {
    switch (TclValue_type(v)) {
    case TCL_VALUE_FUN:
        return (TclFunction_)TclValue_ptr(v);
    default:
        return oops;
    }
}

int TclValue_null(TclValue *v) {
    if (v == NULL || v->container == NULL || v->container->value == NULL)
        return 1;
    return 0;
}

int TclValue_str_cmp(TclValue *v, char *str) {
  return strcmp(TclValue_str(v), str);
}

int TclValue_type_object_cmp(TclValue *v, char *type_str) {
    if (TclValue_type(v) == TCL_VALUE_OBJ) {
        TclValueObject *obj = TclValue_ptr(v);
        return strcmp(obj->type_str, type_str);
    } else {
        return -1;
    }
}

void *TclValue_object_ptr(TclValue *v) {
    if (TclValue_type(v) != TCL_VALUE_OBJ)
        return NULL;
    TclValueObject *obj = TclValue_ptr(v);
    return obj->ptr;
}

char *TclValue_type_str(TclValue *type) {
    switch (TclValue_type(type)) {
    case TCL_VALUE_STR:
        return "STRING";
    case TCL_VALUE_INT:
        return "INT";
    case TCL_VALUE_OBJ:
        return "OBJECT";
    case TCL_VALUE_FUN:
        return "FUNCTION";
    case TCL_VALUE_NULL:
        return "NULL";
    case TCL_VALUE_LIST:
        return "LIST";
    default:
        return "UNKNOWN";
    }
}
