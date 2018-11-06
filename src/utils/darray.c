#include "darray.h"
#include <assert.h>

Darray *Darray_create(size_t element_size, const size_t capacity)
{
    Darray *array = NULL;
    CHECK(element_size > 0, "Can't create darray with element size 0.");

    size_t cap = (capacity == 0) ? DEFAULT_CAPACITY : capacity;

    array = malloc(sizeof(Darray));
    CHECK_MEM(array);

    array->data = calloc(cap, sizeof(void *));
    CHECK_MEM(array->data);

    array->capacity = cap;
    array->size = 0;
    array->element_size = element_size;
    array->expand_rate = DEFAULT_EXPAND_RATE;

    return array;

    error:
        if (array != NULL) free(array);
        return NULL;  
}

void Darray_destroy(Darray *array)
{
    CHECK(array != NULL, "Can't destroy NULL.");
    if (array->data != NULL)
    {
        free(array->data);
    }
    free(array);
    error: return;
}

void Darray_clear(Darray *array)
{
    CHECK(array != NULL, "Tried to clear empty darray");
    
    size_t i = 0;
    for (i = 0; i < array->size; i++)
    {
        if (array->data[i] != NULL) free(array->data[i]);
    }

    array->size = 0;
    error:
        return;
}

int Darray_expand(Darray *array, const size_t elements)
{
    CHECK(array != NULL, "Can't expand NULL.");
    size_t elem = (elements == 0) ? array->expand_rate : elements;
    return Darray_resize(array, array->capacity + elem);
    error:
        return -1;
}

//will not reduce beyond expand rate
int Darray_contract(Darray *array)
{
    CHECK(array != NULL, "Can't contract NULL.");
    if (array->size > array->expand_rate)
    {
        return Darray_resize(array, array->size);
    }
    else if (array->capacity > array->expand_rate)
    {
        return Darray_resize(array, array->expand_rate);
    }
    else
    {
        return 0;
    }

    error:
        return -1;
}

int Darray_push(Darray *array, void *elem)
{
    CHECK(array != NULL, "Can't push  to NULL.");

    if (array->size >= array->capacity)
    {
        if(Darray_resize(array, array->capacity + array->expand_rate) != 0) return -1;
    }

    array->data[array->size] = elem;
    array->size++;

    return 0;

    error:
        return -1;
}

void *Darray_pop(Darray *array)
{
    CHECK(array != NULL, "Can't pop NULL.");
    
    if (array->size > 0)
    {
        array->size--;
        return array->data[array->size];
    }
    
    error:
        return NULL;
}

void Darray_clear_destroy(Darray *array)
{
    Darray_clear(array);
    Darray_destroy(array);
}