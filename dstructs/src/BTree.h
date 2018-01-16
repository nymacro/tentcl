/*
 * dstructs -- Binary Tree
 * Copyright (C) 2006-2018 Aaron Marks
 */
#ifndef DSTRUCTS_BTREE_H
#define DSTRUCTS_BTREE_H

/* unbalanced binary tree node */
struct BTreeNode {
    struct BTreeNode *left;
    struct BTreeNode *right;
    void *data;
};
typedef struct BTreeNode BTreeNode;

/* prototypes for callback functions */
typedef int (*BTreeNodeCompareFunc)(BTreeNode*, void*);
typedef void (*BTreeNodeAllocFunc)(BTreeNode*, void*);
typedef void (*BTreeNodeDeallocFunc)(BTreeNode*);
typedef void (*BTreeNodeMapFunc)(BTreeNode*, void*);

/* binary tree data type */
struct BTree {
    BTreeNode *head;
    
    BTreeNodeCompareFunc compare;
    BTreeNodeAllocFunc alloc;
    BTreeNodeDeallocFunc dealloc;
};
typedef struct BTree BTree;

void BTree_new(BTree*);
void BTree_delete(BTree*);
BTree *BTree_malloc(void);
void BTree_free(BTree *);
void BTree_add(BTree*, void*);
void BTree_remove(BTree*, BTreeNode*);
BTreeNode* BTree_get(BTree*, void*);
void BTree_map(BTree*, BTreeNodeMapFunc, void *);
int BTree_size(BTree *);

#endif
