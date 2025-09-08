/* Simple test to find memory corruption */
#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "vector.h"
#include <stdio.h>

void test_simple_operations() {
    printf("=== Simple Operations Test ===\n");
    
    // Create vector exactly like the failing test  
    vector* v = vector_create(int, 0);
    printf("1. Created empty vector: length=%zu capacity=%zu\n", 
           vector_length(v), vector_capacity(v));
    
    // First append - this should result in length=1 but test shows length=2
    int result = vector_append(v, int, 42);
    printf("2. Append 42: result=%d length=%zu capacity=%zu\n", 
           result, vector_length(v), vector_capacity(v));
    
    // Print actual contents
    printf("3. Contents: ");
    for (size_t i = 0; i < vector_length(v); i++) {
        int* val = vector_at(int, v, i);
        printf("[%zu]=%d ", i, val ? *val : -999);
    }
    printf("\n");
    
    // Second append
    result = vector_append(v, int, 100, 200);
    printf("4. Append 100,200: result=%d length=%zu capacity=%zu\n", 
           result, vector_length(v), vector_capacity(v));
    
    // Print actual contents
    printf("5. Contents: ");
    for (size_t i = 0; i < vector_length(v); i++) {
        int* val = vector_at(int, v, i);
        printf("[%zu]=%d ", i, val ? *val : -999);
    }
    printf("\n");
    
    vector_free(v);
    printf("=== Test Complete ===\n");
}

// Also test with same exact flow as failing test suite
void test_exact_failing_sequence() {
    printf("\n=== Exact Failing Sequence ===\n");
    
    // Test 1: Create and free
    {
        vector* v = vector_create(int, 3, 1, 2, 3);
        printf("Test1: Created vector(1,2,3): length=%zu\n", vector_length(v));
        vector_free(v);
        printf("Test1: Freed vector\n");
    }
    
    // Test 2: Create empty
    {
        vector* v = vector_create(int, 0);
        printf("Test2: Created empty vector: length=%zu\n", vector_length(v));
        vector_free(v);
        printf("Test2: Freed empty vector\n");
    }
    
    // Test 3: Append (this is where it fails)
    {
        vector* v = vector_create(int, 0);
        printf("Test3: Created empty vector: length=%zu\n", vector_length(v));
        
        int result = vector_append(v, int, 42);
        printf("Test3: After append(42): result=%d length=%zu (expected 1)\n", 
               result, vector_length(v));
               
        if (vector_length(v) != 1) {
            printf("ERROR: Length mismatch! Expected 1, got %zu\n", vector_length(v));
        }
        
        vector_free(v);
        printf("Test3: Freed vector\n");
    }
    
    printf("=== Sequence Complete ===\n");
}

int main() {
    test_simple_operations();
    test_exact_failing_sequence();
    return 0;
}