/*
 * Linked list test
 */
#include "List.h"
#include <stdio.h>
#include <assert.h>

typedef void (*TestFunc)(List*);

#define RUN_TEST(test_func) {			\
	printf("starting %s\n", (#test_func));	\
	run_test((test_func));			\
	printf("finished %s\n", (#test_func));	\
    }
void run_test(TestFunc func) {
    List *list = List_malloc();
    func(list);
    List_free(list);
}

int string_compare(ListNode *node, void* val) { return strcmp(node->data, val); }
void string_alloc(ListNode *node, void *val)  { node->data = strdup(val); }
void string_dealloc(ListNode *node)           { free(node->data); }
#define SET_TYPE(list, type) {			\
	(list)->compare = type ## _compare;	\
	(list)->alloc   = type ## _alloc;	\
	(list)->dealloc = type ## _dealloc;	\
    }

void push_pop(List *list) {
    List_push(list, 12);
    List_push(list, 7);
    List_push(list, 2);
    assert(List_index(list, 0)->data == 12);
    assert(List_index(list, 1)->data == 7);
    assert(List_first(list)->data == 12);
    assert(List_last(list)->data == 2);
    List_pop(list);
    assert(List_first(list)->data == 12);
    List_pop(list);
    assert(List_first(list)->data == 12);
    List_pop(list);
    assert(List_first(list) == NULL);
}

void push_shift(List *list) {
    List_push(list, 12);
    List_push(list, 7);
    List_push(list, 2);
    assert(List_index(list, 0)->data == 12);
    assert(List_index(list, 1)->data == 7);
    assert(List_first(list)->data == 12);
    assert(List_last(list)->data == 2);
    List_shift(list);
    assert(List_first(list)->data == 7);
    List_shift(list);
    assert(List_first(list)->data == 2);
    List_shift(list);
    assert(List_first(list) == NULL);
}

void remove_head(List *list) {
    List_push(list, 12);
    List_push(list, 7);
    List_push(list, 8);
    List_remove(list, List_index(list, 0));
    assert(List_size(list) == 2);
    assert(List_first(list)->data == 7);
    assert(List_last(list)->data == 8);
}

void remove_tail(List *list) {
    List_push(list, 12);
    List_push(list, 7);
    List_push(list, 8);
    List_remove(list, List_index(list, 2));
    assert(List_size(list) == 2);
    assert(List_first(list)->data == 12);
    assert(List_last(list)->data == 7);
}

void remove_mid(List *list) {
    List_push(list, 12);
    List_push(list, 7);
    List_push(list, 8);
    List_remove(list, List_index(list, 1));
    assert(List_size(list) == 2);
    assert(List_first(list)->data == 12);
    assert(List_last(list)->data == 8);
}

void str_push_pop(List *list) {
    SET_TYPE(list, string);
    List_push(list, "hello world");
    List_push(list, "bad boy");
    assert(strcmp(List_index(list, 0)->data, "hello world") == 0);
    assert(strcmp(List_index(list, 1)->data, "bad boy") == 0);
    assert(strcmp(List_first(list)->data, "hello world") == 0);
    assert(strcmp(List_last(list)->data, "bad boy") == 0);
    List_pop(list);
    List_pop(list);
}

int main(int argc, char *argv[]) {
    RUN_TEST(push_pop);
    RUN_TEST(push_shift);
    RUN_TEST(remove_head);
    RUN_TEST(remove_tail);
    RUN_TEST(remove_mid);

    RUN_TEST(str_push_pop);

    return 0;
}

