/*
 * dstructs -- Doubly Linked List
 * Copyright (C) 2006-2015 Aaron Marks
 */
#ifndef DSTRUCTS_LIST_H
#define DSTRUCTS_LIST_H

/* doubly linked list node */
struct ListNode {
    struct ListNode *prev;
    struct ListNode *next;
    void *data;
};
typedef struct ListNode ListNode;

/* prototypes for list callback functions */
typedef int (*ListNodeCompareFunc)(ListNode*, void*);
typedef void (*ListNodeAllocFunc)(ListNode*, void*);
typedef void (*ListNodeDeallocFunc)(ListNode*);

/* linked list structure */
struct List {
    ListNode *head;
    ListNode *tail;

    ListNodeCompareFunc compare;
    ListNodeAllocFunc alloc;
    ListNodeDeallocFunc dealloc;
};
typedef struct List List;

void List_new(List*);
void List_delete(List*);
List *List_malloc(void);
void List_free(List *);
void List_insertAfter(List*, ListNode*, void*);
void List_insertBefore(List*, ListNode*, void*);
void List_add(List*, void*); /* sorted add */
void List_push(List*, void*);
void List_pop(List*);
void List_shift(List*);
void List_remove(List*, ListNode*);
unsigned int List_size(List*);
ListNode* List_first(List*);
ListNode* List_last(List*);
ListNode* List_index(List*, unsigned int);
ListNode* List_find(List*, void*);

#endif
