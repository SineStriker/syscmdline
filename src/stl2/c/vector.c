#include "vector.h"

#include <stdio.h>
#include <stdlib.h>

#define INITIAL_SIZE 10

void vector_init(vector_t *vector) {
    vector->data = NULL;
    vector->size = 0;
    vector->capacity = 0;
}

void vector_reserve(vector_t *vector, int capacity) {
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

void vector_resize(vector_t *vector, int size) {
    if (size < vector->size) {
        vector->size = size;
    } else if (size > vector->size) {
        vector_reserve(vector, size);
        if (vector->data == NULL) {
            fprintf(stderr, "Failed to allocate memory.\n");
            return;
        }
        for (int i = vector->size; i < size; i++) {
            vector->data[i] = NULL;
        }
        vector->size = size;
    }
}

void vector_push_back(vector_t *vector, void *element) {
    if (vector->size == vector->capacity) {
        int new_capacity = vector->capacity == 0 ? INITIAL_SIZE : vector->capacity * 2;
        vector_reserve(vector, new_capacity);
    }
    vector->data[vector->size] = element;
    vector->size++;
}

void vector_clear(vector_t *vector) {
    vector->size = 0;
}

void vector_free(vector_t *vector) {
    free(vector->data);
    vector->data = NULL;
    vector->size = 0;
    vector->capacity = 0;
}