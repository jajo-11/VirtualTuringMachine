#ifndef _DArray_h_
#define _DArray_h_

#include <stdlib.h>
#include <assert.h>
#include "dbg.h"

typedef struct Darray {
    size_t size;
    size_t capacity;
    size_t element_size;
    size_t expand_rate;
    void **data;
} Darray;

Darray *Darray_create(size_t element_size, size_t capacity);
void Darray_destroy(Darray *array);
void Darray_clear(Darray *array);
int Darray_expand(Darray *array, size_t elements);
int Darray_contract(Darray *array);
int Darray_push(Darray *array, void *elem);
void *Darray_pop(Darray *array);
void Darray_clear_destroy(Darray *array);

#define DEFAULT_EXPAND_RATE 300
#define DEFAULT_CAPACITY 100

static inline int Darray_resize(Darray *array, size_t capacity)
{
    CHECK(array != NULL, "Can't resize NULL.");
    CHECK(capacity > 0, "Capacity must be at least 1.");

    array->data = (void **) realloc(array->data, sizeof(void *) * capacity);
    CHECK_MEM(array->data);

    if (capacity > array->capacity)
    {
        memset(array->data+array->capacity, 0, capacity - array->capacity);
    }

    array->capacity = capacity;

    return 0;
    error:
        return -1;
}

static inline void Darray_set(Darray *array, size_t index, void *elem)
{
    if (array != NULL)
    {
        if (array->capacity <= index) Darray_resize(array, index + array->expand_rate);
        if (array->size <= index) array->size = index + 1;
        array->data[index] = elem;
    }
}

static inline void *Darray_get(Darray *array, size_t index)
{
    if (array == NULL || array->size <= index) return NULL;
    else return array->data[index];
}

static inline void *Darray_remove(Darray *array, size_t index)
{
    if (array == NULL || array->size <= index) return NULL;
    else {
        void *data = array->data[index];
        array->data[index] = NULL;
        return data;
    }
}
static inline void *Darray_alloc(Darray *array)
{
    if (array == NULL) return NULL;
    else return calloc(1, array->element_size);
}

#endif