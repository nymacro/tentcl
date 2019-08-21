/*
 * tentcl -- record type
 * copyright (c) 2006-2018 aaron marks. all rights reserved.
 */

#ifndef TENTCL_RECORD_H
#define TENTCL_RECORD_H

#include "BTree.h"
#include "Hash.h"

typedef struct TclRecordType {
    char *name;
    BTree *attributes;
} TclRecordType;

typedef struct TclRecord {
    TclRecordType *type;
    Hash *values;
} TclRecord;

void TclRecordType_new(TclValue **type, char *name);
void TclRecordType_add_field(TclValue *type, char *name);
int TclValue_is_record_type(TclValue *type);

void TclRecord_new(TclValue **, TclRecordType *type);
int TclValue_is_record(TclValue *value);

#endif
