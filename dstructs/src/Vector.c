/*
 * dstructs -- "Vector"
 * Copyright (C) 2006-2015 Aaron Marks
 */
#include "Vector.h"

#include <stdio.h>
#include <stdlib.h>

int Vector_compare_(VectorNode *vecdata, void *data) {
    if (vecdata->data == data)
        return 0;
    else
        return 1;
}

void Vector_alloc_(VectorNode *vecdata, void *data) {
    vecdata->data = data;
}

void Vector_dealloc_(VectorNode *vecdata) {
    /* nothing */
}

/*
 * Create a new Vector
 */
void Vector_new(Vector *self) {
    self->p = 0;
    self->size = 0;
    self->chunk = 4;
    self->nodes = NULL;
    
    self->compare = Vector_compare_;
    self->alloc = Vector_alloc_;
    self->dealloc = Vector_dealloc_;
}

/*
 * Delete a vector created with Vector_new
 */
void Vector_delete(Vector *self) {
    int i;
    for (i = 0; i < self->p; i++) {
        self->dealloc(&self->nodes[i]);
    }
    free(self->nodes);
}

/*
 * Assign memory and create a vector
 */
Vector *Vector_malloc(void) {
    Vector *self = (Vector*)malloc(sizeof(Vector));
    Vector_new(self);
    return self;
}

/*
 * Destroy a vector and free memory allocated from Vector_malloc
 */
void Vector_free(Vector *self) {
    Vector_delete(self);
    free(self);
}

/*
 * Adjuse size of vector
 */
void Vector_adjust(Vector *self, int size) {
    int i;
    if (size < 1)
        return;
    if (self->size != size) {
        /* dealloc any data which will be chopped off in adjust */
        for (i = size; i < self->p; i++) {
            self->dealloc(&self->nodes[i]);
            self->p--;
        }
        if (self->nodes)
            self->nodes = (VectorNode*)realloc(self->nodes, sizeof(VectorNode) * size);
        else
            self->nodes = (VectorNode*)malloc(sizeof(VectorNode) * size);
        self->size = size;
    }
}

/*
 * Push data onto the end of the vector
 */
void Vector_push(Vector *self, void *data) {
    /* expand if needed */
    if (self->p >= self->size) {
        Vector_adjust(self, self->size + self->chunk);
    }
    self->alloc(&self->nodes[self->p++], data);
}

/*
 * Pop data off the end of the vector
 */
void Vector_pop(Vector *self) {
    self->dealloc(&self->nodes[self->p--]);
    /* shrink if needed */
    if (self->size > self->p + self->chunk)
        Vector_adjust(self, self->size - self->chunk);
}

/*
 * Return last element
 */
VectorNode *Vector_last(Vector *self) {
    if (self->p - 1 < 0)
        return NULL;
    return &self->nodes[self->p - 1];
}

/*
 * Return first element
 */
VectorNode *Vector_first(Vector *self) {
    if (self->size < 1)
        return NULL;
    return &self->nodes[0];
}

/*
 * Get element at specified index
 */
VectorNode *Vector_index(Vector *self, int index) {
    if (index >= self->size)
        return NULL;
    return &self->nodes[index];
}

/*
 * Return size of vector
 */
int Vector_size(Vector *self) {
    return self->size;
}

/*
 * Set the extra blocks to add when resizing the vector
 */
void Vector_setChunk(Vector *self, int size) {
    if (size > 0)
        self->chunk = size;
}
