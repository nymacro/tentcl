/*
 * tentcl -- record type
 * copyright (c) 2006-2018 aaron marks. all rights reserved.
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "value.h"
#include "record.h"

#define RECORD_TYPE_STR "record-type"
#define RECORD_STR "record"

static int BTree_strcmp_(BTreeNode *node, void *data) {
    return strcmp(node->data, data);
}

static void BTree_strfree_(BTreeNode *node) {
    free(node->data);
}

int TclValue_is_record_type(TclValue *value) {
    return TclValue_type_object_cmp(value, RECORD_TYPE_STR) == 0;
}

void TclRecordType_delete(TclRecordType *type) {
    BTree_free(type->attributes);
    free(type->name);
    free(type);
}

static void free_type(TclValueObject *obj) {
    TclRecordType_delete(obj->ptr);
    free(obj->ptr);
}

void TclRecordType_new(TclValue **type, char *name) {
    TclRecordType *t = malloc(sizeof(TclRecordType));
    t->name = strdup(name);
    t->attributes = BTree_malloc();
    t->attributes->compare = BTree_strcmp_;
    t->attributes->dealloc = BTree_strfree_;

    TclValue_new_object(type, RECORD_TYPE_STR, t, (void (*)(void*))free_type);
}

static int TclRecordType_has_field(TclRecordType *type, char*name) {
    return BTree_get(type->attributes, name) != NULL;
}

void TclRecordType_add_field(TclValue *value, char *name) {
    if (!TclValue_is_record_type(value))
        abort();

    TclRecordType *type = TclValue_object_ptr(value);

    if (!TclRecordType_has_field(type, name))
        BTree_add(type->attributes, strdup(name));
}

static void Hash_variables_dealloc_(HashPair *pair) {
    TclValue_delete((TclValue*)pair->data);
}

void TclRecord_delete(TclRecord *record) {
    Hash_free(record->values);
    free(record);
}

static void free_value(TclValueObject *obj) {
    TclRecord_delete(obj->ptr);
    free(obj->ptr);
}

void TclRecord_new(TclValue **record, TclRecordType *type) {
    TclRecord *r = malloc(sizeof(TclRecord));
    r->type = type;
    r->values = Hash_malloc();
    r->values->dealloc = Hash_variables_dealloc_;

    TclValue_new_object(record, RECORD_STR, r, (void (*)(void*))free_value);
}

TclValue *TclRecord_set_field(TclRecord *record, char *name, TclValue *value) {
    if (!TclRecordType_has_field(record->type, name)) {
        abort();
    }

    HashPair *pair = Hash_get(record->values, name);
    if (pair->data)
        TclValue_delete(pair->data);
    TclValue_ref(value);
    pair->data = value;

    return value;
}

TclValue *TclRecord_get_field(TclRecord *record, char *name) {
    HashPair *pair = Hash_get(record->values, name);
    return pair->data;
}

int TclValue_is_record(TclValue *value) {
    return TclValue_type_object_cmp(value, RECORD_STR) == 0;
}
