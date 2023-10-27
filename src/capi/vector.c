#include "vector.h"

#include <stdio.h>
#include <stdlib.h>

#define INITIAL_SIZE 10

void vector_init(struct vector *vector) {
    vector->data = NULL;
    vector->size = 0;
    vector->capacity = 0;
}

void vector_reserve(struct vector *vector, int capacity) {
    if (capacity <= vector->capacity) {
        return;
    }

    void **newData = (void **) realloc(vector->data, capacity * sizeof(void *));
    if (newData == NULL) {
        fprintf(stderr, "Failed to allocate memory.\n");
        return;
    }

    vector->data = newData;
    vector->capacity = capacity;
}

void vector_resize(struct vector *vector, int size) {
    if (size < vector->size) {
        vector->size = size;
    } else if (size > vector->size) {
        vector_reserve(vector, size);
        if (vector->data == NULL) {
            fprintf(stderr, "Failed to allocate memory.\n");
            return;
        }
        vector->size = size;
    }
}

void vector_push_back(struct vector *vector, void *element) {
    if (vector->size == vector->capacity) {
        int new_capacity = vector->capacity == 0 ? INITIAL_SIZE : vector->capacity * 2;
        vector_reserve(vector, new_capacity);
    }
    vector->data[vector->size] = element;
    vector->size++;
}

void vector_clear(struct vector *vector) {
    vector_resize(vector, 0);
}

void vector_fini(struct vector *vector) {
    vector_clear(vector);
    free(vector->data);
}