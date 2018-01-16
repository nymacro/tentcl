/*
 * dstructs -- Doubly Linked List
 * Copyright (C) 2006-2018 Aaron Marks
 */
#include "List.h"
#include <stdio.h>
#include <stdlib.h>

/*
 * List_compare_: default node/data compare. Returns 0 on equality, 1 if
 * if the node's data is greater than the specified data, -1 otherwise.
 */
static int List_compare_(ListNode *node, void *data) {
    if (*(int*)node->data == *(int*)data)
        return 0;
    else if (*(int*)node->data > *(int*)data)
        return 1;
    else
        return -1;
}

/*
 * List_alloc_: default allocator, used to allocate data to a node
 */
static void List_alloc_(ListNode *node, void *data) {
    node->data = data;
}

/*
 * List_dealloc_: default deallocator, used to deallocate a node
 */
static void List_dealloc_(ListNode *node) {
    // blank
}

/*
 * List_new: create a new doubly-linked list
 */
void List_new(List *list) {
    list->head = NULL;
    list->tail = NULL;

    list->compare = List_compare_;
    list->alloc = List_alloc_;
    list->dealloc = List_dealloc_;
}

/*
 * List_delete: delete a doubly-linked list created with List_create
 */
void List_delete(List *list) {
    ListNode *p = list->head;
    while (p) {
        ListNode *cur = p;
        p = p->next;

        list->dealloc(cur);
        free(cur);
    }
}

/*
 * List_malloc: allocate memory and create a new doubly-linked list
 */
List *List_malloc(void) {
    List *self = (List *) malloc(sizeof(List));
    List_new(self);
    return self;
}

/*
 * List_free: delete doubly-linked list and free memory assigned by List_malloc
 */
void List_free(List *list) {
    List_delete(list);
    free(list);
}

/*
 * List_insertAfter: insert data after a specified node
 */
void List_insertAfter(List *list, ListNode *prev, void *data) {
    if (!prev || !list->head || !list->tail) {
        List_push(list, data);
        return;
    }
    ListNode *node = (ListNode*)malloc(sizeof(ListNode));
    node->next = prev->next;
    node->prev = prev;
    if (prev == list->tail) {
        list->tail = node;
    }
    prev->next = node;
    list->alloc(node, data);
}

/*
 * List_insertBefore: insert data before a specified node
 */
void List_insertBefore(List *list, ListNode *next, void *data) {
    if (!next || !list->head || !list->tail) {
        List_push(list, data);
        return;
    }
    ListNode *node = (ListNode*)malloc(sizeof(ListNode));
    node->next = next;
    if (!next->prev) {
        list->head = node;
        node->prev = NULL;
    } else {
        node->prev = next->prev;
    }
    next->prev = node;
    list->alloc(node, data);
}

/*
 * List_add: ordered insert into the list
 */
void List_add(List *list, void *data) {
    ListNode *p = list->head;
    ListNode *prev = NULL;
    while (p) {
        if (list->compare(p, data) < 0)
            break;
        prev = p;
        p = p->next;
    }
    if (p)
        List_insertBefore(list, prev, data);
    else
        List_insertAfter(list, prev, data);
}

/*
 * List_push: push data onto back of list
 */
void List_push(List *list, void *data) {
    if (!list->head) {
        list->head = (ListNode*)malloc(sizeof(ListNode));
        list->head->prev = NULL;
        list->head->next = NULL;
        list->alloc(list->head, data);
        list->tail = list->head;
    } else if (list->tail) {
        list->tail->next = (ListNode*)malloc(sizeof(ListNode));
        list->tail->next->prev = list->tail;
        list->tail->next->next = NULL;
        list->alloc(list->tail->next, data);
        list->tail = list->tail->next;
    } else {
        fprintf(stderr, "fatal: list with no head or tail");
        exit(1);
    }
}

/*
 * List_pop: pop node off the back off list
 */
void List_pop(List *list) {
    List_remove(list, List_last(list));
}

/*
 * List_shift: pop node off the front of list
 */
void List_shift(List *list) {
    List_remove(list, List_first(list));
}

/*
 * List_size: return the size of the list
 */
unsigned int List_size(List *list) {
    ListNode *p = list->head;
    unsigned int count = 0;
    while (p) {
        p = p->next;
        count++;
    }
    return count;
}

/*
 * List_first: get the first node in the list
 */
ListNode* List_first(List *list) {
    return list->head;
}

/*
 * List_last: get the last node in the list
 */
ListNode* List_last(List *list) {
    return list->tail;
}

/*
 * List_index: get the node at the specified index of the list
 */
ListNode* List_index(List *list, unsigned int index) {
    ListNode *p = list->head;
    int i = 0;
    while (p && i != index) {
        p = p->next;
        ++i;;
    }
    return (index == i) ? p : NULL;
}

/*
 * List_find: find a specified node in the list
 */
ListNode* List_find(List *list, void *data) {
    ListNode *p = list->head;
    while (p) {
        if (list->compare(p, data) == 0) {
            return p;
        }
        p = p->next;
    }
    return NULL;
}

/*
 * List_remove: remove specified node from the list
 */
void List_remove(List *list, ListNode *node) {
    ListNode *p = list->head;
    while (p) {
        if (p == node) {
            ListNode *prev = p->prev;
            ListNode *next = p->next;
            list->dealloc(p);
            free(p);
 
            if (!prev)
                list->head = next;
            else
                prev->next = next;

            if (!next)
                list->tail = prev;
            else
                next->prev = prev;
            return;
        }
        p = p->next;
    }
}
