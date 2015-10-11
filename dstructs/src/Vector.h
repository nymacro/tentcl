/*
 * dstructs -- "Vector"
 * Copyright (C) 2006-2015 Aaron Marks
 */
#ifndef DSTRUCTS_VECTOR_H
#define DSTRUCTS_VECTOR_H

struct VectorNode {
    void *data;
};
typedef struct VectorNode VectorNode;

/* prototypes for vector callback functions */
typedef int (*VectorCompareFunc)(VectorNode*, void*);
typedef void (*VectorAllocFunc)(VectorNode*, void*);
typedef void (*VectorDeallocFunc)(VectorNode*);

/* vector */
struct Vector {
    int p;
    int size;
    int chunk;
    VectorNode *nodes;
    VectorCompareFunc compare;
    VectorAllocFunc alloc;
    VectorDeallocFunc dealloc;
};
typedef struct Vector Vector;

void Vector_new(Vector *self);
void Vector_delete(Vector *self);
Vector *Vector_malloc(void);
void Vector_free(Vector *self);
void Vector_adjust(Vector *self, int size);
void Vector_push(Vector *self, void *data);
void Vector_pop(Vector *self);
VectorNode *Vector_last(Vector *self);
VectorNode *Vector_first(Vector *self);
VectorNode *Vector_index(Vector *self, int index);
int Vector_size(Vector *self);
void Vector_setChunk(Vector *self, int size);

#endif
