#include "vector.h"
#include <stdio.h>

/* Debug version of _vector_append_internal */
static int debug_vector_append_internal(vector* vec, size_t num_values, const void* values)
{
    printf("DEBUG: _vector_append_internal called with num_values=%zu\n", num_values);
    return _vector_append_internal(vec, num_values, values);
}

/* Debug version of vector_append */
#define debug_vector_append(vec, type, ...) \
    ({ \
        int _ret; \
        if (!vec) { \
            _vector_error("NULL vector"); \
            _ret = -1; \
        } else { \
            size_t arg_count = ARG_COUNT(__VA_ARGS__); \
            printf("DEBUG: ARG_COUNT for args [%s] = %zu\n", #__VA_ARGS__, arg_count); \
            vector_wrlock(vec); \
            _ret = debug_vector_append_internal(vec, arg_count, \
                                               (const type[]){__VA_ARGS__}); \
            vector_unlock(vec); \
        } \
        _ret; \
    })

int main() {
    vector* v = vector_create(int, 0);
    printf("Testing debug_vector_append with single arg...\n");
    debug_vector_append(v, int, 42);
    printf("Length after append: %zu\n", vector_length(v));
    
    printf("\nTesting debug_vector_append with two args...\n");
    debug_vector_append(v, int, 100, 200);
    printf("Length after append: %zu\n", vector_length(v));
    
    vector_free(v);
    return 0;
}