#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "vector.h"
#include <stdio.h>

/* Exact copy of test that's failing */
void test_vector_append_debug(void)
{
    printf("=== STARTING APPEND TEST DEBUG ===\n");
    
    vector* v = vector_create(int, 0);
    printf("After vector_create(int, 0): v=%p, length=%zu, capacity=%zu\n", 
           (void*)v, v ? vector_length(v) : 999, v ? vector_capacity(v) : 999);
    
    if (!v) {
        printf("ERROR: vector creation failed\n");
        return;
    }
    
    printf("About to call vector_append(v, int, 42)\n");
    int result = vector_append(v, int, 42);
    printf("vector_append returned: %d\n", result);
    printf("After append: length=%zu, capacity=%zu\n", vector_length(v), vector_capacity(v));
    
    if (vector_length(v) > 0) {
        int* val = vector_at(int, v, 0);
        printf("First element: %s = %d\n", val ? "valid" : "NULL", val ? *val : 0);
    }
    
    printf("About to call vector_append(v, int, 100, 200)\n");
    result = vector_append(v, int, 100, 200);
    printf("vector_append returned: %d\n", result);
    printf("After second append: length=%zu, capacity=%zu\n", vector_length(v), vector_capacity(v));
    
    printf("All elements:\n");
    for (size_t i = 0; i < vector_length(v); i++) {
        int* val = vector_at(int, v, i);
        printf("  [%zu]: %s = %d\n", i, val ? "valid" : "NULL", val ? *val : 0);
    }
    
    vector_free(v);
    printf("=== APPEND TEST DEBUG COMPLETE ===\n");
}

int main() {
    test_vector_append_debug();
    return 0;
}