/*
 * dstructs -- Skip List
 * Copyright (C) 2006-2008  Aaron Marks
 */
#ifndef DSTRUCTS_SKIPLIST_H
#define DSTRUCTS_SKIPLIST_H

#include "List.h"
#include "BTree.h"

struct SkipListNode {
    struct SkipList *parent;
    void *data;
};
typedef struct SkipListNode SkipListNode;

/* prototypes for skip list callback functions */
typedef int (*SkipListCompareFunc)(SkipListNode*, void*);
typedef void (*SkipListAllocFunc)(SkipListNode*, void*);
typedef void (*SkipListDeallocFunc)(SkipListNode*);

struct SkipList {
    BTree *topLevel;
    List *levels; /* List of BTree */
    SkipListCompareFunc compare;
    SkipListAllocFunc alloc;
    SkipListDeallocFunc dealloc;
};
typedef struct SkipList SkipList;

#endif
