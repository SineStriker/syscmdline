#ifndef VECTOR_H
#define VECTOR_H

#ifdef __cplusplus
extern "C" {
#endif

struct vector_handler {
    void *(*construct)(const void *);
    void (*destroy)(void *);
};

struct vector {
    void **data;
    int size;
    int capacity;
};

typedef struct vector vector_t;

void vector_init(struct vector *vector);

void vector_reserve(struct vector *vector, int capacity);

void vector_resize(struct vector *vector, int size);

void vector_push_back(struct vector *vector, void *element);

void vector_clear(struct vector *vector);

void vector_fini(struct vector *vector);

#ifdef __cplusplus
}
#endif

#endif // VECTOR_H
