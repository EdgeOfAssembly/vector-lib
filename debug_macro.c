/* Minimal test to debug the ARG_COUNT issue */
#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "vector.h"
#include <stdio.h>

/* Add debug to the ARG_COUNT macro */
#undef ARG_COUNT
#define ARG_COUNT(...) ({ \
    size_t count = ARG_COUNT_N(__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0); \
    printf("DEBUG ARG_COUNT: args=[%s] count=%zu\n", #__VA_ARGS__, count); \
    count; \
})

/* Add debug to vector_append */
#undef vector_append
#define vector_append(vec, type, ...) \
    ({ \
        int _ret; \
        if (!vec) { \
            _vector_error("NULL vector"); \
            _ret = -1; \
        } else { \
            printf("DEBUG: vector_append called with vec=%p type=%s args=[%s]\n", \
                   (void*)vec, #type, #__VA_ARGS__); \
            size_t arg_count = ARG_COUNT(__VA_ARGS__); \
            printf("DEBUG: ARG_COUNT returned %zu\n", arg_count); \
            printf("DEBUG: Creating compound literal (const %s[]){%s}\n", #type, #__VA_ARGS__); \
            vector_wrlock(vec); \
            printf("DEBUG: About to call _vector_append_internal with num_values=%zu\n", arg_count); \
            _ret = _vector_append_internal(vec, arg_count, \
                                           (const type[]){__VA_ARGS__}); \
            printf("DEBUG: _vector_append_internal returned %d, new length=%zu\n", _ret, vec->length); \
            if (_ret == -1) _vector_error("Failed to append to vector"); \
            vector_unlock(vec); \
        } \
        _ret; \
    })

int main() {
    printf("=== DEBUG TEST START ===\n");
    
    vector* v = vector_create(int, 0);
    printf("Created vector: %p, length=%zu\n", (void*)v, vector_length(v));
    
    printf("\n--- Test 1: Append single element ---\n");
    vector_append(v, int, 42);
    printf("After append: length=%zu\n", vector_length(v));
    
    printf("\n--- Test 2: Append two elements ---\n");
    vector_append(v, int, 100, 200);
    printf("After append: length=%zu\n", vector_length(v));
    
    vector_free(v);
    printf("=== DEBUG TEST END ===\n");
    return 0;
}