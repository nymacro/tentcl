/*
 * dstructs -- Binary Tree
 * Copyright (C) 2006-2018 Aaron Marks
 */
#include "BTree.h"
#include <stdio.h>
#include <stdlib.h>

/*
 * BTree_compare_: default comparator, like List_compare_
 */
static int BTree_compare_(BTreeNode *node, void *data) {
    if (node->data < data)
        return -1;
    else if (node->data > data)
        return 1;
    else
        return 0;
}

/*
 * BTree_alloc_: default allocator, like List_alloc_
 */
static void BTree_alloc_(BTreeNode *node, void *data) {
    node->data = data;
}

/*
 * BTree_dealloc_: default deallocator, like List_dealloc_
 */
static void BTree_dealloc_(BTreeNode *node) {
    // blank
}

/*
 * BTree_new: create a new unbalanced binary tree
 */
void BTree_new(BTree *tree) {
    tree->head = NULL;

    tree->compare = BTree_compare_;
    tree->alloc = BTree_alloc_;
    tree->dealloc = BTree_dealloc_;
}

/*
 * BTree_delete_: do not use directly! used by BTree_delete
 */
static void BTree_delete_(BTree *tree, BTreeNode *node) {
    if (!node)
        return;
    BTree_delete_(tree, node->left);
    BTree_delete_(tree, node->right);
    tree->dealloc(node);
    free(node);
}

/*
 * BTree_delete: delete a binary tree created with BTree_new
 */
void BTree_delete(BTree *tree) {
    BTree_delete_(tree, tree->head);
}

/*
 * BTree_malloc: allocate memory for a binary tree and create with BTree_create
 */
BTree *BTree_malloc(void) {
    BTree *self = (BTree *)malloc(sizeof(BTree));
    BTree_new(self);
    return self;
}

/*
 * BTree_free: delete binary tree and free the memory assigned by BTree_malloc
 */
void BTree_free(BTree *tree) {
    BTree_delete(tree);
    free(tree);
}

/*
 * BTree_add_: do not use directly! worker function for BTree_add
 */
static BTreeNode **BTree_add_(BTree *tree, BTreeNode **node, void *data) {
    if (!*node) {
        return node;
    } else if (tree->compare(*node, data) < 0) {
        return BTree_add_(tree, &(*node)->left, data);
    } else {
        return BTree_add_(tree, &(*node)->right, data);
    }
}

/*
 * BTree_add: add data to binary tree
 */
void BTree_add(BTree *tree, void *data) {
    BTreeNode **pos = BTree_add_(tree, &tree->head, data);
    *pos = (BTreeNode*)malloc(sizeof(BTreeNode));
    (*pos)->left = NULL;
    (*pos)->right = NULL;
    tree->alloc(*pos, data);
}

/*
 * BTree_findParent_: do not use directly! helper function for BTree_findParent
 */
static BTreeNode *BTree_findParent_(BTree *tree, BTreeNode *node,
                                     BTreeNode *child) {
    BTreeNode *parent;
    
    if (node == NULL)
        return NULL;
    else if (node->right == child)
        return node;
    else if (node->left == child)
        return node;
    else if (node == tree->head)
        return NULL;
    
    parent = BTree_findParent_(tree, node->left, child);
    if (parent)
        return parent;
    parent = BTree_findParent_(tree, node->right, child);
    if (parent)
        return parent;
    
    return NULL;
}

/*
 * BTree_findParent: find the parent node of specified node
 */
BTreeNode *BTree_findParent(BTree *tree, BTreeNode *node) {
    return BTree_findParent_(tree, tree->head, node);
}

/*
 * BTree_remove: remove node from binary tree, adjusting parent/child nodes.
 */
void BTree_remove(BTree *tree, BTreeNode *node) {
    BTreeNode *left, *right, *parent;
    
    if (!node)
        return;
    
    parent = BTree_findParent_(tree, tree->head, node);
    left = node->left;
    right = node->right;
    
    if (node == tree->head) {
        if (right) {
            tree->head = right;
            if (left) {
                /* relocate left */
                BTreeNode **pos = BTree_add_(tree, &tree->head, left->data);
                *pos = left;
            }
        } else if (left) {
            tree->head = left;
        } else {
            tree->head = NULL;
        }
    } else {
        if (right) {
            if (parent->right == node)
                parent->right = right;
            else
                parent->left = right;
            if (left) {
                /* relocate left */
                BTreeNode **pos = BTree_add_(tree, &tree->head, left->data);
                *pos = left;
            }
        } else if (left) {
            if (parent->right == node)
                parent->right = left;
            else
                parent->left = left;
        } else {
            if (parent->right == node)
                parent->right = NULL;
            else
                parent->left = NULL;
        }
    }
    
    tree->dealloc(node);
    free(node);
}

/*
 * BTree_get_: do not use directly! worker function for BTree_get
 */
static BTreeNode *BTree_get_(BTree *tree, BTreeNode *node, void *data) {
    if (!node)
        return NULL;
    
    int cmp = tree->compare(node, data);
    if (cmp == 0) {
        return node;
    } else if (cmp < 0) {
        return BTree_get_(tree, node->left, data);
    } else {
        return BTree_get_(tree, node->right, data);
    }
}

/*
 * BTree_get: return node with matching data, otherwise NULL
 */
BTreeNode *BTree_get(BTree *tree, void *data) {
    return BTree_get_(tree, tree->head, data);
}

/*
 * BTree_map_: do not use directly! worker function for BTree_map
 */
void BTree_map_(BTreeNode *node, BTreeNodeMapFunc map, void *data) {
    if (node) {
        map(node, data);
        BTree_map_(node->left, map, data);
        BTree_map_(node->right, map, data);
    }
}

/*
 * BTree_map: map function for every node in tree, passing data to callback
 */
void BTree_map(BTree *tree, BTreeNodeMapFunc map, void *data) {
    BTree_map_(tree->head, map, data);
}

/*
 * BTree_size_: do not use directly! worker function for BTree_size
 */
int BTree_size_(BTree *tree, BTreeNode *node) {
    if (!node)
        return 0;
    return BTree_size_(tree, node->left) + BTree_size_(tree, node->right) + 1;
}

/*
 * BTree_size: Return size of BTree
 */
int BTree_size(BTree *tree) {
    return BTree_size_(tree, tree->head);
}
