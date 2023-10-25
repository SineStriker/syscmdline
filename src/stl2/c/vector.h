#ifndef VECTOR_H
#define VECTOR_H

#ifdef __cplusplus
extern "C" {
#endif

struct vector {
    void **data;
    int size;
    int capacity;
};

typedef struct vector vector_t;

vector *vector_init();

void vector_reserve(vector_t *vector, int capacity);

void vector_resize(vector_t *vector, int size);

void vector_push_back(vector_t *vector, void *element);

void vector_clear(vector_t *vector);

void vector_free(vector_t *vector);

#ifdef __cplusplus
}
#endif

#endif // VECTOR_H
