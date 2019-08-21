/*
 * Tentcl -- IO Types & Functions
 * Copyright (C) 2006-2018 Aaron Marks. All Rights Reserved.
 */

#include <stdio.h>

#include "io.h"
#include "record.h"

static void BTree_list_add_(BTreeNode *node, TclValue *list) {
    TclValue_list_push_str(list, node->data);
}

TclReturn TclExt_attributes(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
    if (argc != 2) {
        return TCL_BADCMD;
    }

    TclValue *rec = Tcl_getVariable(vm, TclValue_str(argv[1]));
    if (rec && TclValue_is_record(rec)) {
        TclRecord *record = TclValue_object_ptr(rec);
        TclValue *attrs;
        TclValue_new_list(&attrs);
        BTree_map(record->type->attributes, (BTreeNodeMapFunc)BTree_list_add_, attrs);
        TclValue_replace(ret, attrs);
    }

    return TCL_OK;
}

TclReturn TclExt_make_record_type(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
    if (argc < 3) {
        return TCL_BADCMD;
    }

    TclValue *record_type;
    TclRecordType_new(&record_type, TclValue_str(argv[1]));
    for (int i = 2; i < argc; i++) {
        TclRecordType_add_field(record_type, TclValue_str(argv[i]));
    }

    TclValue_replace(ret, record_type);

    return TCL_OK;
}

TclReturn TclExt_make_record(Tcl *vm, int argc, TclValue *argv[], TclValue *ret) {
    if (argc != 2) {
        return TCL_BADCMD;
    }

    TclValue *record_type_value = Tcl_getVariable(vm, TclValue_str(argv[1]));

    if (!TclValue_is_record_type(record_type_value)) {
        return TCL_EXCEPTION;
    }

    TclRecordType *record_type = TclValue_object_ptr(record_type_value);

    TclValue *record;
    TclRecord_new(&record, record_type);
    TclValue_replace(ret, record);

    return TCL_OK;
}

void TclIO_register(Tcl *vm) {
    Tcl_register(vm, "attributes", TclExt_attributes);
    Tcl_register(vm, "make-record-type", TclExt_make_record_type);
    Tcl_register(vm, "make-record", TclExt_make_record);
}
