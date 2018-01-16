/*
 * dstructs -- Skip List
 * Copyright (C) 2006-2018 Aaron Marks
 */

/*
 * TODO:
 * - make it work ;o
 */

#include "SkipList.h"
#include "BTree.h"

#include <stdio.h>
#include <stdlib.h>

/*
 * List compare for levels member
 */
int SkipList_List_compare_(ListNode *node, void *data) {
    return 1;
}

/*
 * List allocator for levels member (must be BTree_malloc'ed data)
 */
void SkipList_List_alloc_(ListNode *node, void *data) {
    node->data = data;
}

/*
 * List deallocator for levels member
 */
void SkipList_List_dealloc_(ListNode *node) {
    BTree_free(node->data);
}

/*
 * Levels btree comparator (calls back to SkipList_compare_)
 */
int SkipList_BTree_compare_(BTreeNode *node, void *data) {
    return ((SkipListNode*)node->data)->parent->compare(node->data, data);
}

/*
 * Levels BTree allocator (calls back to SkipList_alloc_)
 */
void SkipList_BTree_alloc_(BTreeNode *node, void *data) {
    return ((SkipListNode*)node->data)->parent->alloc(node->data, data);
}

/*
 * Levels BTree deallocator (calls back to SkipList_dealloc_)
 */
void SkipList_BTree_dealloc_(BTreeNode *node) {
    ((SkipListNode*)node->data)->parent->dealloc(node->data);
    free(node->data);
}

/*
 * SkipList_compare_: SkipList comparator
 */
int SkipList_compare_(SkipListNode *node, void *data) {
    if (node->data == data)
        return 0;
    else
        return 1;
}

/*
 * SkipList_alloc_: SkipList allocator
 */
void SkipList_alloc_(SkipListNode *node, void *data) {
    node->data = data;
}

/*
 * SkipList_dealloc_: SkipList deallocator
 */
void SkipList_dealloc_(SkipListNode *node) {
    /* do nothing */
}

/*
 * Add level to skip list
 */
void SkipList_addLevel_(SkipList *self) {
    BTree *b = BTree_malloc();
    b->compare = SkipList_BTree_compare_;
    b->alloc = SkipList_BTree_alloc_;
    b->dealloc = SkipList_BTree_dealloc_;
    
    List_push(self->levels, b);
}

/*
 * Create a new Skip List
 */
void SkipList_new(SkipList *self) {
    self->levels = List_malloc();
    
    self->compare = SkipList_compare_;
    self->alloc = SkipList_alloc_;
    self->dealloc = SkipList_dealloc_;
    
    SkipList_addLevel_(self);
    self->topLevel = List_first(self->levels)->data;
}

/*
 * Delete SkipList made with SkipList_new
 */
void SkipList_delete(SkipList *self) {
    List_free(self->levels);
}

/*
 * Assign memory and create new SkipList
 */
SkipList *SkipList_malloc(void) {
    SkipList *self = (SkipList*)malloc(sizeof(SkipList));
    SkipList_new(self);
    return self;
}

/*
 * Destroys Skip List and free's memory assigned by SkipList_malloc
 */
void SkipList_free(SkipList *self) {
    SkipList_delete(self);
    free(self);
}

/*
 * Return skip list size
 */
int SkipList_size(SkipList *self) {
    return BTree_size(List_first(self->levels)->data);
}

/*
 * Add data to skip list
 */
void SkipList_add(SkipList *self, void *data) {
    /* add to lowest level */
    SkipListNode *node = (SkipListNode*)malloc(sizeof(SkipListNode));
    node->parent = self;
    self->alloc(node, data);
    
    
}

