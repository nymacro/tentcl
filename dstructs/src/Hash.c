/*
 * dstructs -- Name/Value HashKey (Hash)
 * Copyright (C) 2006-2018 Aaron Marks
 */
#include "Hash.h"
#include <string.h>
#include <stdlib.h>

/*
 * Hash_compare_: default compare function for hash elements
 */
int Hash_BTree_compare_(BTreeNode *node, void *data) {
    return strcmp(((HashPair*)node->data)->name, data);
}

/*
 * Hash_add_compare_: default compare function for adding hash elements
 */
int Hash_BTree_add_compare_(BTreeNode *node, void *data) {
    return strcmp(((HashPair*)node->data)->name, ((HashPair*)data)->name);
}

/*
 * Hash_alloc_: default allocator for tree
 */
void Hash_BTree_alloc(BTreeNode *node, void *data) {
    node->data = data;
}

/*
 * Hash_dealloc_: default deallocator for node's, free's node data
 */
void Hash_BTree_dealloc_(BTreeNode *node) {
    HashPair *pair = node->data;
    Hash *self = pair->self;
    self->dealloc(pair);
    free(pair->name);
    free(pair);
}

void Hash_dealloc_(HashPair *pair) {
    /* do nothing */
}

/*
 * Hash_new: create a new hash table
 */
void Hash_new(Hash *hash) {
    BTree_new(&hash->tree);
    hash->tree.compare = Hash_BTree_compare_;
    hash->tree.alloc = Hash_BTree_alloc;
    hash->tree.dealloc = Hash_BTree_dealloc_;
    
    hash->dealloc = Hash_dealloc_;
}

/*
 * Hash_delete: delete hash table created with Hash_new
 */
void Hash_delete(Hash *hash) {
    BTree_delete(&hash->tree);
}

/*
 * Hash_malloc: allocate memory and create a hash
 */
Hash *Hash_malloc(void) {
    Hash *self = (Hash*)malloc(sizeof(Hash));
    Hash_new(self);
    return self;
}

/*
 * Hash_free: delete hash then free memory assigned by Hash_malloc
 */
void Hash_free(Hash *hash) {
    Hash_delete(hash);
    free(hash);
}

/*
 * Hash_get: return HashPair* for key name. If it doesn't exist in the table
 *           it will be created, making HashPair->data NULL
 */
HashPair* Hash_get(Hash *hash, char *name) {
    BTreeNode *p = BTree_get(&hash->tree, name);
    if (p) {
        return p->data;
    } else {
        HashPair *pair = (HashPair*)malloc(sizeof(HashPair));
        pair->self = hash;
        pair->name = strdup(name);
        pair->data = NULL;
        
        hash->tree.compare = Hash_BTree_add_compare_;
        BTree_add(&hash->tree, pair);
        hash->tree.compare = Hash_BTree_compare_;
        
        return pair;
    }
}

/*
 * Hash_remove: remove key/value pair from hash
 */
void Hash_remove(Hash *hash, HashPair *pair) {
    BTreeNode *node = BTree_get(&hash->tree, pair->name);
    if (node) {
        BTree_remove(&hash->tree, node);
    }
}

/*
 * Hash_map: map function to every key/value pair in hash
 */
void Hash_map(Hash *hash, BTreeNodeMapFunc map, void *data) {
    BTree_map(&hash->tree, map, data);
}

/*
 * Hash_exists: check for a key in the hash
 */
int Hash_exists(Hash *hash, char *key) {
    BTreeNode *p = BTree_get(&hash->tree, (void*)key);
    if (p)
        return 1;
    return 0;
}
