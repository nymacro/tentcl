/*
 * dstructs -- Name/Value Hash Table
 * Copyright (C) 2006-2015 Aaron Marks
 */
#ifndef DSTRUCTS_HASH_H
#define DSTRUCTS_HASH_H

#include "BTree.h"

/*
 * NOTE:
 * this data structure is not implemented as a proper hash table! It is just
 * an abstraction on top of BTree to provide a key/value pair mapping.
 */

/* key/value pair for hash */
struct HashPair {
    char *name;
    void *data;
    /*Hash*/void *self;
};
typedef struct HashPair HashPair;

typedef int (*HashPairCompareFunc)(HashPair*, void*);
typedef void (*HashPairAllocFunc)(HashPair*, void*);
typedef void (*HashPairDeallocFunc)(HashPair*);

/* hash data type */
struct Hash {
    /*
    HashPairCompareFunc compare;
    HashPairAllocFunc alloc;
    */
    HashPairDeallocFunc dealloc;
    
    BTree tree;
};
typedef struct Hash Hash;

void Hash_new(Hash*);
void Hash_delete(Hash*);
Hash *Hash_malloc(void);
void Hash_free(Hash*);
HashPair* Hash_get(Hash*, char*);
void Hash_remove(Hash*, HashPair*);
void Hash_map(Hash*, BTreeNodeMapFunc, void *);
int Hash_exists(Hash *hash, char *key);

#endif
