#include "BTree.h"

int string_compare(BTreeNode *node, void *data) {
    return strcmp(data, node->data);
}

void string_alloc(BTreeNode *node, void *data) {
    node->data = data;
}

void string_dealloc(BTreeNode *node) {
}

int main(int argc, char *argv[]) {
    BTree *tree = BTree_malloc();
    tree->compare = string_compare;
    tree->alloc = string_alloc;
    tree->dealloc = string_dealloc;

    printf("Adding c\n");
    BTree_add(tree, "c");
    printf("Adding a\n");
    BTree_add(tree, "a");
    printf("Adding b\n");
    BTree_add(tree, "b");

    BTree_free(tree);
    return 0;
}

