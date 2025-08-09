/*
 * vector.h - Dynamic Array Library
 * Copyright (C) 2025 Stefan Froberg <stefan.froberg@protonmail.com>
 *
 * Overview:
 * A thread-safe, dynamic array implementation with support for custom allocators,
 * serialization, and sorting. Dual-licensed under GPLv3 (open-source) and BSD 3-Clause
 * (commercial, contact author).
 *
 * Key Features:
 * - O(1) access, O(1) amortized append, O(n) insert/remove.
 * - Thread-safe with read-write locks (Windows SRWLOCK, Linux pthread_rwlock_t).
 * - Customizable error handling via vector_set_error_callback.
 *
 * Usage Example:
 *   vector* v = vector_create(int, 3, 1, 2, 3); // Creates [1, 2, 3]
 *   vector_append(v, int, 4);                  // Appends 4
 *   int* val = vector_at(int, v, 0);           // Accesses first element
 *   vector_free(v);                            // Frees vector
 *
 * Thread Safety:
 *   Safe for concurrent reads and exclusive writes. Ensure proper locking in custom code.
 *
 * Notes:
 * - Uninitialized memory is allocated; initialize elements before use.
 * - Requires C99 or later.
 */
#ifndef __VECTOR_H__
#define __VECTOR_H__

/* Includes */
#include <stddef.h>  /* size_t, max_align_t */
#include <stdio.h>   /* fprintf, FILE */
#include <stdlib.h>  /* malloc, free, realloc, qsort, aligned_alloc */
#include <string.h>  /* memcpy, memmove, memset */
#include <stdarg.h>  /* va_list, vsnprintf */
#include <stdint.h>  /* SIZE_MAX, uint64_t, ssize_t */
#include <stdbool.h>
#include "align.h"   /* alignof, alignas */

#if defined(_WIN32)
#include <windows.h> /* SRWLOCK */
#elif defined(__linux__)
#include <pthread.h> /* pthread_rwlock_t */
#endif

/* Enforce C99 or later */
#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 199901L
#error "This library requires C99 or later."
#endif

/* Default alignment */
#define VECTOR_DEFAULT_ALIGNMENT 16

/* Vector struct definition */
typedef struct {
    void* data alignas(VECTOR_DEFAULT_ALIGNMENT); /* Pointer to data array */
    size_t length;       /* Current number of elements */
    size_t capacity;     /* Total allocated capacity */
    size_t element_size; /* Size of each element in bytes */
    struct {
        void* (*alloc)(size_t);      /* Allocator function */
        void* (*realloc)(void*, size_t); /* Reallocator function */
        void (*free)(void*);         /* Deallocator function */
    } allocator;         /* Custom allocator functions */
#if defined(_WIN32)
    SRWLOCK rwlock;      /* Windows read-write lock */
#elif defined(__linux__)
    pthread_rwlock_t rwlock; /* POSIX read-write lock */
#endif
} vector;

/* Thread-local storage for sorting */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
    _Thread_local static vector* _sort_context;
    _Thread_local static int (*_sort_compar)(const void*, const void*, void*);
#elif defined(__GNUC__) || defined(__clang__)
    __thread static vector* _sort_context;
    __thread static int (*_sort_compar)(const void*, const void*, void*);
#else
    static vector* _sort_context;
    static int (*_sort_compar)(const void*, const void*, void*);
    #warning "vector_sort not thread-safe in pre-C11 without GCC/Clang"
#endif

/* Forward declarations */
static void _vector_error(const char* format, ...);
static void* _vector_at(vector* vec, size_t index);
static vector* _vector_create_base(size_t element_size, size_t num_elements);
static int _vector_append_internal(vector* vec, size_t num_values,
                                   const void* values);
static int _vector_insert_internal(vector* vec, size_t index, size_t num_values,
                                   const void* values);
static int _vector_prepend_internal(vector* vec, size_t num_values,
                                    const void* values);
static void* _vector_pop_internal(vector* vec);
static int _vector_remove_internal(vector* vec, size_t index,
                                   size_t num_elements);
static int _vector_reserve_internal(vector* vec, size_t new_capacity);
static int _vector_resize_internal(vector* vec, size_t new_length);
static int _vector_serialize_internal(const vector* vec, FILE* fp);
static vector* _vector_deserialize_internal(FILE* fp, size_t element_size);
static int _vector_shrink_to_fit_internal(vector* vec);
static int _vector_swap_internal(vector* vec, size_t idx1, size_t idx2);
static void vector_rdlock(vector* vec);
static void vector_wrlock(vector* vec);
static void vector_unlock(vector* vec);
static int _safe_add(size_t a, size_t b, size_t* result);
static int _safe_mul(size_t a, size_t b, size_t* result);
static void* default_alloc(size_t size);
static void* default_realloc(void* ptr, size_t size);
static void default_free(void* ptr);

/* Public API Macros and Functions */

/* Macro to append values to the vector */
/* Args: vec - vector pointer, type - element type, ... - values to append */
/* Returns: 0 on success, -1 on failure */
#define vector_append(vec, type, ...) \
    ({ \
        int _ret; \
        if (!vec) { \
            _vector_error("NULL vector"); \
            _ret = -1; \
        } else { \
            vector_wrlock(vec); \
            _ret = _vector_append_internal(vec, ARG_COUNT(__VA_ARGS__), \
                                           (const type[]){__VA_ARGS__}); \
            if (_ret == -1) _vector_error("Failed to append to vector"); \
            vector_unlock(vec); \
        } \
        _ret; \
    })

/* Macro to access an element at an index */
/* Args: type - element type, vec - vector pointer, index - element index */
/* Returns: pointer to element, NULL if invalid */
#define vector_at(type, vec, index) \
    ({ \
        vector_rdlock(vec); \
        type* _ptr = (type*)_vector_at(vec, index); \
        if (!_ptr) _vector_error("Invalid vector or index %zu out of bounds " \
                                 "(length: %zu)", index, vec ? vec->length : 0); \
        vector_unlock(vec); \
        _ptr; \
    })

/* Macro to get pointer to element at index */
/* Args: type - element type, vec - vector pointer, index - element index */
/* Returns: pointer to element, NULL if invalid */
#define vector_at_ptr(type, vec, index) ((type*)_vector_at(vec, index))

/* Macro to get vector capacity */
/* Args: vec - vector pointer */
/* Returns: capacity of vector, 0 if NULL */
#define vector_capacity(vec) ((vec) ? (vec)->capacity : 0)

/* Clears vector by setting length to 0 */
/* Args: vec - vector pointer */
/* Returns: 0 on success, -1 if NULL */
static int vector_clear(vector* vec)
{
    if (!vec)
    {
        _vector_error("NULL vector");
        return -1;
    }
    vector_wrlock(vec);
    vec->length = 0;
    vector_unlock(vec);
    return 0;
}

/* Creates a deep copy of the vector */
/* Args: src - source vector pointer (read-only) */
/* Returns: new vector pointer, NULL on failure */
static vector* vector_copy(const vector* src)
{
    if (!src)
    {
        _vector_error("NULL source vector");
        return NULL;
    }
    vector_rdlock((vector*)src);
    vector* dst = _vector_create_base(src->element_size, src->length);
    if (dst)
    {
        memcpy(dst->data, src->data, src->length * src->element_size);
        dst->length = src->length;
    }
    vector_unlock((vector*)src);
    return dst;
}

/* Macro to create vector with type and elements */
/* Args: type - element type, ... - num_elements and optional values */
/* Returns: new vector pointer, NULL on failure */
#define vector_create(type, ...) \
    _vector_create_dispatch(type, __VA_ARGS__)
#define _vector_create_dispatch(type, num_elements_or_first, ...) \
    _vector_create_with_values(sizeof(type), num_elements_or_first, \
                               ARG_COUNT(__VA_ARGS__), (const type[]){__VA_ARGS__})

/* Macro to iterate over vector elements */
/* Args: type - element type, vec - vector pointer, ptr - iterator variable */
#define vector_foreach(type, vec, ptr) \
    do { \
        if (vec) { \
            vector_rdlock(vec); \
            for (type* ptr = (type*)(vec)->data; \
                 ptr < (type*)((char*)(vec)->data + (vec)->length * (vec)->element_size); \
                 ++ptr) { \
                /* User code here */ \
            } \
            vector_unlock(vec); \
        } \
    } while (0)

/* Macro to find element in vector */
/* Args: type - element type, vec - vector pointer, value - value to find, */
/*       compar - comparison function */
/* Returns: index of element, -1 if not found */
#define vector_find(type, vec, value, compar) \
    _vector_find_internal((vec), (const void*)&(type){(value)}, sizeof(type), (compar))

/* Frees vector and its data */
/* Args: vec - vector pointer to free */
static void vector_free(vector* vec)
{
    if (vec)
    {
        vector_wrlock(vec);
        if (vec->data)
            vec->allocator.free(vec->data);
        vector_unlock(vec);
#if defined(_WIN32)
        /* SRWLOCK does not require destruction */
#elif defined(__linux__)
        pthread_rwlock_destroy(&vec->rwlock);
#endif
        free(vec);
    }
}

/* Macro to insert values at index */
/* Args: vec - vector pointer, type - element type, index - insertion point, */
/*       ... - values to insert */
/* Returns: 0 on success, -1 on failure */
#define vector_insert(vec, type, index, ...) \
    ({ \
        int _ret; \
        vector_wrlock(vec); \
        _ret = _vector_insert_internal(vec, index, ARG_COUNT(__VA_ARGS__), \
                                       (const type[]){__VA_ARGS__}); \
        if (_ret == -1) _vector_error("Failed to insert into vector"); \
        vector_unlock(vec); \
        _ret; \
    })

/* Checks if vector is empty */
/* Args: vec - vector pointer */
/* Returns: true if empty or NULL, false otherwise */
#define vector_is_empty(vec) ((vec) ? (vec)->length == 0 : true)

/* Returns current length of vector */
/* Args: vec - vector pointer */
/* Returns: length of vector, 0 if NULL */
#define vector_length(vec) ((vec) ? (vec)->length : 0)

/* Removes and returns last element */
/* Args: type - element type, vec - vector pointer */
/* Returns: pointer to popped element, NULL on failure */
#define vector_pop(type, vec) ((type*)_vector_pop_internal(vec))

/* Macro to prepend values to vector */
/* Args: vec - vector pointer, type - element type, ... - values to prepend */
/* Returns: 0 on success, -1 on failure */
#define vector_prepend(vec, type, ...) \
    ({ \
        int _ret; \
        vector_wrlock(vec); \
        _ret = _vector_prepend_internal(vec, ARG_COUNT(__VA_ARGS__), \
                                        (const type[]){__VA_ARGS__}); \
        if (_ret == -1) _vector_error("Failed to prepend to vector"); \
        vector_unlock(vec); \
        _ret; \
    })

#define vector_push vector_append /* Alias for append */

/* Removes elements from index */
/* Args: vec - vector pointer, index - start index, num_elements - count */
/* Returns: 0 on success, -1 on failure */
static int vector_remove(vector* vec, size_t index, size_t num_elements)
{
    if (!vec)
    {
        _vector_error("NULL vector");
        return -1;
    }
    vector_wrlock(vec);
    int result = _vector_remove_internal(vec, index, num_elements);
    vector_unlock(vec);
    return result;
}

/* Reserves capacity for vector */
/* Args: vec - vector pointer, new_capacity - desired capacity */
/* Returns: 0 on success, -1 on failure */
static int vector_reserve(vector* vec, size_t new_capacity)
{
    if (!vec)
    {
        _vector_error("NULL vector");
        return -1;
    }
    vector_wrlock(vec);
    int result = _vector_reserve_internal(vec, new_capacity);
    vector_unlock(vec);
    return result;
}

/* Resizes vector to new length */
/* Args: vec - vector pointer, new_length - desired length */
/* Returns: 0 on success, -1 on failure */
static int vector_resize(vector* vec, size_t new_length)
{
    if (!vec)
    {
        _vector_error("NULL vector");
        return -1;
    }
    vector_wrlock(vec);
    int result = _vector_resize_internal(vec, new_length);
    vector_unlock(vec);
    return result;
}

/* Serializes vector to file */
/* Args: vec - vector pointer (read-only), fp - file pointer */
/* Returns: 0 on success, -1 on failure */
static int vector_serialize(const vector* vec, FILE* fp)
{
    if (!vec || !fp)
    {
        _vector_error("NULL vector or file pointer");
        return -1;
    }
    vector_rdlock((vector*)vec);
    int result = _vector_serialize_internal(vec, fp);
    vector_unlock((vector*)vec);
    return result;
}

/* Deserializes vector from file */
/* Args: fp - file pointer, element_size - size of each element */
/* Returns: new vector pointer, NULL on failure */
static vector* vector_deserialize(FILE* fp, size_t element_size)
{
    if (!fp)
    {
        _vector_error("NULL file pointer");
        return NULL;
    }
    return _vector_deserialize_internal(fp, element_size);
}

/* Macro to set value at index */
/* Args: type - element type, vec - vector pointer, index - position, */
/*       value - value to set */
#define vector_set(type, vec, index, value) do { \
    vector_wrlock(vec); \
    type* _ptr = (type*)_vector_at(vec, index); \
    if (_ptr) *_ptr = (value); \
    vector_unlock(vec); \
} while (0)

/* Shrinks vector capacity to length */
/* Args: vec - vector pointer */
/* Returns: 0 on success, -1 on failure */
static int vector_shrink_to_fit(vector* vec)
{
    if (!vec)
    {
        _vector_error("NULL vector");
        return -1;
    }
    vector_wrlock(vec);
    int result = _vector_shrink_to_fit_internal(vec);
    vector_unlock(vec);
    return result;
}

/* Macro to sort vector */
/* Args: vec - vector pointer, type - element type, compar - comparison fn */
/* Note: See vector.h header for thread safety and comparison details */
#define vector_sort(vec, type, compar) \
    do { \
        vector_wrlock(vec); \
        _vector_sort_internal(vec, compar); \
        vector_unlock(vec); \
    } while (0)

/* Swaps two elements in vector */
/* Args: vec - vector pointer, idx1 - first index, idx2 - second index */
/* Returns: 0 on success, -1 on failure */
static int vector_swap(vector* vec, size_t idx1, size_t idx2)
{
    if (!vec)
    {
        _vector_error("NULL vector");
        return -1;
    }
    vector_wrlock(vec);
    int result = _vector_swap_internal(vec, idx1, idx2);
    vector_unlock(vec);
    return result;
}

/* Comparison macros for sorting */
#define compare_asc   _vector_compare_asc
#define compare_desc  _vector_compare_desc
#define compare_eq    _vector_compare_eq

/* Error callback type */
typedef void (*vector_error_callback)(const char* message);

/* Sets error callback function */
/* Args: callback - function to handle errors */
void vector_set_error_callback(vector_error_callback callback);

/* Internal Macros and Functions */

/* Counts variable arguments */
#define ARG_COUNT_N(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, N, ...) N
#define ARG_COUNT(...) ARG_COUNT_N(__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

/* Gets pointer to element at index */
/* Args: vec - vector pointer, index - element index */
/* Returns: pointer to element, NULL if invalid */
static void* _vector_at(vector* vec, size_t index)
{
    if (!vec || index >= vec->length)
        return NULL;
    return (char*)vec->data + index * vec->element_size;
}

/* Appends values to vector */
/* Args: vec - vector pointer, num_values - count, values - data to append */
/* Returns: 0 on success, -1 on failure */
static int _vector_append_internal(vector* vec, size_t num_values,
                                   const void* values)
{
    if (!vec)
        return -1;
    if (num_values == 0)
        return 0;

    size_t total_elements;
    if (_safe_add(vec->length, num_values, &total_elements) == -1)
        return -1;

    if (total_elements > vec->capacity)
    {
        size_t new_capacity = vec->capacity == 0 ? num_values :
                              vec->capacity + vec->capacity / 2;
        if (new_capacity < total_elements)
            new_capacity = total_elements;
        size_t new_size;
        if (_safe_mul(new_capacity, vec->element_size, &new_size) == -1)
            return -1;
        void* new_data = vec->allocator.realloc(vec->data, new_size);
        if (!new_data)
            return -1;
        vec->data = new_data;
        vec->capacity = new_capacity;
    }
    memcpy((char*)vec->data + vec->length * vec->element_size, values,
           num_values * vec->element_size);
    vec->length = total_elements;
    return 0;
}

/* Compares elements ascending */
/* Args: a - first element, b - second element, context - vector pointer */
/* Returns: -1 if a < b, 1 if a > b, 0 if equal */
static int _vector_compare_asc(const void* a, const void* b, void* context)
{
    vector* vec = (vector*)context;
    const char* ptr_a = (const char*)a;
    const char* ptr_b = (const char*)b;
    for (size_t i = 0; i < vec->element_size; ++i)
    {
        if (ptr_a[i] < ptr_b[i])
            return -1;
        if (ptr_a[i] > ptr_b[i])
            return 1;
    }
    return 0;
}

/* Compares elements descending */
/* Args: a - first element, b - second element, context - vector pointer */
/* Returns: -1 if b < a, 1 if b > a, 0 if equal */
static int _vector_compare_desc(const void* a, const void* b, void* context)
{
    vector* vec = (vector*)context;
    const char* ptr_a = (const char*)a;
    const char* ptr_b = (const char*)b;
    for (size_t i = 0; i < vec->element_size; ++i)
    {
        if (ptr_b[i] < ptr_a[i])
            return -1;
        if (ptr_b[i] > ptr_a[i])
            return 1;
    }
    return 0;
}

/* Checks element equality */
/* Args: a - first element, b - second element, context - vector pointer */
/* Returns: 0 if equal, 1 if not equal */
static int _vector_compare_eq(const void* a, const void* b, void* context)
{
    vector* vec = (vector*)context;
    const char* ptr_a = (const char*)a;
    const char* ptr_b = (const char*)b;
    for (size_t i = 0; i < vec->element_size; ++i)
    {
        if (ptr_a[i] != ptr_b[i])
            return 1;
    }
    return 0;
}

/* Creates vector with base settings */
/* Args: element_size - size of each element, num_elements - initial count */
/* Returns: new vector pointer, NULL on failure */
static vector* _vector_create_base(size_t element_size, size_t num_elements)
{
    size_t alloc_size;
    if (_safe_mul(element_size, num_elements, &alloc_size) == -1)
    {
        _vector_error("Overflow in allocation: element_size %zu * num_elements %zu",
                      element_size, num_elements);
        return NULL;
    }

    vector* vec = malloc(sizeof(vector));
    if (!vec)
    {
        _vector_error("Failed to allocate vector structure");
        return NULL;
    }

    vec->allocator.alloc = default_alloc;
    vec->allocator.realloc = default_realloc;
    vec->allocator.free = default_free;
    vec->data = alloc_size ? calloc(num_elements, element_size) : NULL;
    if (!vec->data && alloc_size > 0)
    {
        _vector_error("Failed to allocate vector data for %zu bytes", alloc_size);
        free(vec);
        return NULL;
    }

    vec->length = num_elements;
    vec->capacity = num_elements;
    vec->element_size = element_size;
#if defined(_WIN32)
    InitializeSRWLock(&vec->rwlock);
#elif defined(__linux__)
    if (pthread_rwlock_init(&vec->rwlock, NULL) != 0)
    {
        vec->allocator.free(vec->data);
        free(vec);
        _vector_error("Failed to initialize rwlock");
        return NULL;
    }
#endif
    return vec;
}

/* Creates vector with values */
/* Args: element_size - size of each element, num_elements - count, */
/*       arg_count - number of values, values - initial values */
/* Returns: new vector pointer, NULL on failure */
static vector* _vector_create_with_values(size_t element_size, size_t num_elements,
                                          size_t arg_count, const void* values)
{
    vector* vec = _vector_create_base(element_size, num_elements);
    if (!vec)
        return NULL;

    if (arg_count == 0)
    {
        /* Already zeroed by calloc */
    }
    else if (arg_count == 1)
    {
        for (size_t i = 0; i < num_elements; ++i)
            memcpy((char*)vec->data + i * element_size, values, element_size);
    }
    else if (arg_count <= num_elements)
    {
        memcpy(vec->data, values, arg_count * element_size);
        /* Remaining space already zeroed by calloc */
    }
    else
    {
        _vector_error("Argument count %zu exceeds num_elements %zu",
                      arg_count, num_elements);
        vector_free(vec);
        return NULL;
    }
    return vec;
}

/* Finds element in vector */
/* Args: vec - vector pointer, value - value to find, element_size - size, */
/*       compar - comparison function */
/* Returns: index of element, -1 if not found */
static ssize_t _vector_find_internal(vector* vec, const void* value,
                                     size_t element_size,
                                     int (*compar)(const void*, const void*, void*))
{
    if (!vec)
    {
        _vector_error("NULL vector");
        return -1;
    }
    vector_rdlock(vec);
    for (size_t i = 0; i < vec->length; ++i)
    {
        void* elem = (char*)vec->data + i * element_size;
        if (compar(elem, value, vec) == 0)
        {
            vector_unlock(vec);
            return (ssize_t)i;
        }
    }
    vector_unlock(vec);
    return -1;
}

/* Inserts values at index */
/* Args: vec - vector pointer, index - insertion point, num_values - count, */
/*       values - data to insert */
/* Returns: 0 on success, -1 on failure */
static int _vector_insert_internal(vector* vec, size_t index, size_t num_values,
                                   const void* values)
{
    if (!vec)
        return -1;
    if (num_values == 0)
        return 0;
    if (index > vec->length)
        return -1;

    size_t total_elements;
    if (_safe_add(vec->length, num_values, &total_elements) == -1)
        return -1;

    if (total_elements > vec->capacity)
    {
        size_t new_capacity = vec->capacity == 0 ? num_values :
                              vec->capacity + vec->capacity / 2;
        if (new_capacity < total_elements)
            new_capacity = total_elements;
        if (_vector_reserve_internal(vec, new_capacity) == -1)
            return -1;
    }
    if (index < vec->length)
    {
        memmove((char*)vec->data + (index + num_values) * vec->element_size,
                (char*)vec->data + index * vec->element_size,
                (vec->length - index) * vec->element_size);
    }
    memcpy((char*)vec->data + index * vec->element_size, values,
           num_values * vec->element_size);
    vec->length = total_elements;
    return 0;
}

/* Pops last element */
/* Args: vec - vector pointer */
/* Returns: pointer to popped element, NULL on failure */
static void* _vector_pop_internal(vector* vec)
{
    if (!vec || vec->length == 0)
    {
        _vector_error("NULL or empty vector");
        return NULL;
    }
    vector_wrlock(vec);
    void* popped_data = vec->allocator.alloc(vec->element_size);
    if (!popped_data)
    {
        vector_unlock(vec);
        return NULL;
    }
    void* last_element = (char*)vec->data + (vec->length - 1) * vec->element_size;
    memcpy(popped_data, last_element, vec->element_size);
    vec->length--;
    vector_unlock(vec);
    return popped_data;
}

/* Prepends values to vector */
/* Args: vec - vector pointer, num_values - count, values - data to prepend */
/* Returns: 0 on success, -1 on failure */
static int _vector_prepend_internal(vector* vec, size_t num_values,
                                    const void* values)
{
    return _vector_insert_internal(vec, 0, num_values, values);
}

/* Wrapper for qsort */
/* Args: a - first element, b - second element */
/* Returns: comparison result */
static int _vector_qsort_wrapper(const void* a, const void* b)
{
    return _sort_compar(a, b, _sort_context);
}

/* Sorts vector with comparison function */
/* Args: vec - vector pointer, compar - comparison function */
static void _vector_sort_internal(vector* vec,
                                  int (*compar)(const void*, const void*, void*))
{
    if (!vec || vec->length <= 1)
        return;
    _sort_context = vec;
    _sort_compar = compar;
    qsort(vec->data, vec->length, vec->element_size, _vector_qsort_wrapper);
    _sort_context = NULL;
    _sort_compar = NULL;
}

/* Removes elements from index */
/* Args: vec - vector pointer, index - start index, num_elements - count */
/* Returns: 0 on success, -1 on failure */
static int _vector_remove_internal(vector* vec, size_t index,
                                   size_t num_elements)
{
    if (index >= vec->length || index + num_elements > vec->length)
    {
        _vector_error("Index out of bounds: index %zu, num_elements %zu, length %zu",
                      index, num_elements, vec->length);
        return -1;
    }
    if (num_elements == 0)
        return 0;

    size_t elements_to_move = vec->length - index - num_elements;
    if (elements_to_move > 0)
    {
        size_t bytes_to_move = elements_to_move * vec->element_size;
        memmove((char*)vec->data + index * vec->element_size,
                (char*)vec->data + (index + num_elements) * vec->element_size,
                bytes_to_move);
    }
    vec->length -= num_elements;
    return 0;
}

/* Reserves capacity for vector */
/* Args: vec - vector pointer, new_capacity - desired capacity */
/* Returns: 0 on success, -1 on failure */
static int _vector_reserve_internal(vector* vec, size_t new_capacity)
{
    if (new_capacity <= vec->capacity)
        return 0;
    size_t new_size;
    if (_safe_mul(new_capacity, vec->element_size, &new_size) == -1)
        return -1;
    void* new_data = vec->allocator.realloc(vec->data, new_size);
    if (!new_data && new_size > 0)
        return -1;
    vec->data = new_data;
    vec->capacity = new_capacity;
    return 0;
}

/* Resizes vector to new length */
/* Args: vec - vector pointer, new_length - desired length */
/* Returns: 0 on success, -1 on failure */
static int _vector_resize_internal(vector* vec, size_t new_length)
{
    if (new_length > vec->capacity)
    {
        size_t new_capacity = vec->capacity ? vec->capacity * 2 : new_length;
        if (new_capacity < new_length)
            new_capacity = new_length;
        if (_vector_reserve_internal(vec, new_capacity) == -1)
            return -1;
    }
    if (new_length > vec->length)
    {
        memset((char*)vec->data + vec->length * vec->element_size, 0,
               (new_length - vec->length) * vec->element_size);
    }
    vec->length = new_length;
    return 0;
}

/* Serializes vector to file */
/* Args: vec - vector pointer (read-only), fp - file pointer */
/* Returns: 0 on success, -1 on failure */
static int _vector_serialize_internal(const vector* vec, FILE* fp)
{
    if (fwrite(&vec->length, sizeof(size_t), 1, fp) != 1 ||
        fwrite(&vec->element_size, sizeof(size_t), 1, fp) != 1 ||
        fwrite(vec->data, vec->element_size, vec->length, fp) != vec->length)
        return -1;
    return 0;
}

/* Deserializes vector from file */
/* Args: fp - file pointer, element_size - size of each element */
/* Returns: new vector pointer, NULL on failure */
static vector* _vector_deserialize_internal(FILE* fp, size_t element_size)
{
    size_t length, read_element_size;
    if (fread(&length, sizeof(size_t), 1, fp) != 1 ||
        fread(&read_element_size, sizeof(size_t), 1, fp) != 1 ||
        read_element_size != element_size)
        return NULL;
    vector* vec = _vector_create_base(element_size, length);
    if (!vec || fread(vec->data, element_size, length, fp) != length)
    {
        vector_free(vec);
        return NULL;
    }
    return vec;
}

/* Shrinks capacity to fit length */
/* Args: vec - vector pointer */
/* Returns: 0 on success, -1 on failure */
static int _vector_shrink_to_fit_internal(vector* vec)
{
    if (vec->capacity == vec->length)
        return 0;
    size_t new_size;
    if (_safe_mul(vec->length, vec->element_size, &new_size) == -1)
        return -1;
    void* new_data = vec->length ? vec->allocator.realloc(vec->data, new_size) : NULL;
    if (vec->length && !new_data)
        return -1;
    if (!vec->length && vec->data)
        vec->allocator.free(vec->data);
    vec->data = new_data;
    vec->capacity = vec->length;
    return 0;
}

/* Swaps two elements in vector */
/* Args: vec - vector pointer, idx1 - first index, idx2 - second index */
/* Returns: 0 on success, -1 on failure */
static int _vector_swap_internal(vector* vec, size_t idx1, size_t idx2)
{
    if (idx1 >= vec->length || idx2 >= vec->length)
        return -1;
    if (idx1 == idx2)
        return 0;
    char temp[vec->element_size];
    void* ptr1 = (char*)vec->data + idx1 * vec->element_size;
    void* ptr2 = (char*)vec->data + idx2 * vec->element_size;
    memcpy(temp, ptr1, vec->element_size);
    memcpy(ptr1, ptr2, vec->element_size);
    memcpy(ptr2, temp, vec->element_size);
    return 0;
}

/* Default allocator: allocates aligned memory */
/* Args: size - bytes to allocate */
/* Returns: pointer to allocated memory, NULL on failure */
static void* default_alloc(size_t size)
{
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
    return aligned_alloc(VECTOR_DEFAULT_ALIGNMENT, size); /* C11 */
#else
    return malloc(size); /* C99 */
#endif
}

/* Default realloc: resizes memory */
/* Args: ptr - pointer to resize, size - new size */
/* Returns: pointer to reallocated memory, NULL on failure */
static void* default_realloc(void* ptr, size_t size)
{
    return realloc(ptr, size);
}

/* Default free: deallocates memory */
/* Args: ptr - pointer to free */
static void default_free(void* ptr)
{
    free(ptr);
}

/* Default error callback: prints to stderr */
/* Args: message - error message to print */
static void default_error_callback(const char* message)
{
    fprintf(stderr, "%s\n", message);
}

/* Static error callback variable */
static vector_error_callback error_callback = default_error_callback;

/* Sets error callback function */
/* Args: callback - function to handle errors */
void vector_set_error_callback(vector_error_callback callback)
{
    error_callback = callback ? callback : default_error_callback;
}

/* Formats and dispatches error messages */
/* Args: format - format string, ... - variable arguments */
static void _vector_error(const char* format, ...)
{
    if (error_callback)
    {
        char msg[256];
        va_list args;
        va_start(args, format);
        vsnprintf(msg, sizeof(msg), format, args);
        va_end(args);
        error_callback(msg);
    }
}

/* Locks vector for reading */
/* Args: vec - vector pointer */
static void vector_rdlock(vector* vec)
{
    if (!vec)
        return;
#if defined(_WIN32)
    AcquireSRWLockShared(&vec->rwlock);
#elif defined(__linux__)
    pthread_rwlock_rdlock(&vec->rwlock);
#endif
}

/* Locks vector for writing */
/* Args: vec - vector pointer */
static void vector_wrlock(vector* vec)
{
    if (!vec)
        return;
#if defined(_WIN32)
    AcquireSRWLockExclusive(&vec->rwlock);
#elif defined(__linux__)
    pthread_rwlock_wrlock(&vec->rwlock);
#endif
}

/* Unlocks vector */
/* Args: vec - vector pointer */
static void vector_unlock(vector* vec)
{
    if (!vec)
        return;
#if defined(_WIN32)
    ReleaseSRWLockExclusive(&vec->rwlock); /* Assumes write lock; adjust if needed */
#elif defined(__linux__)
    pthread_rwlock_unlock(&vec->rwlock);
#endif
}

/* Safe addition */
/* Args: a - first number, b - second number, result - sum */
/* Returns: 0 on success, -1 on overflow */
static int _safe_add(size_t a, size_t b, size_t* result)
{
    if (b > SIZE_MAX - a)
        return -1;
    *result = a + b;
    return 0;
}

/* Safe multiplication */
/* Args: a - first number, b - second number, result - product */
/* Returns: 0 on success, -1 on overflow */
static int _safe_mul(size_t a, size_t b, size_t* result)
{
    if (a && b > SIZE_MAX / a)
        return -1;
    *result = a * b;
    return 0;
}

#endif /* __VECTOR_H__ */

