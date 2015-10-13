#include "BTree.h"

typedef void (*TestFunc)(BTree*);

#define RUN_TEST(test_func) {			\
	printf("starting %s\n", (#test_func));	\
	run_test((test_func));			\
	printf("finished %s\n", (#test_func));	\
    }
void run_test(TestFunc func) {
    BTree *tree = BTree_malloc();
    func(tree);
    BTree_free(tree);
}

int string_compare(BTreeNode *node, void *data) {
    return strcmp(data, node->data);
}

void string_alloc(BTreeNode *node, void *data) {
    node->data = data;
}

void string_dealloc(BTreeNode *node) {
}

void bad_test(BTree *tree) {
    BTree_add(tree, "a");
    BTree_add(tree, "b");
    BTree_add(tree, "c");
    BTree_add(tree, "d");
    BTree_add(tree, "a");
    BTree_add(tree, "d");
    BTree_add(tree, "b");
    BTree_add(tree, "z");
}

int main(int argc, char *argv[]) {
    RUN_TEST(bad_test);

    return 0;
}

