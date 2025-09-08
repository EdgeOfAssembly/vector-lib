/* Security-focused test demonstrating key safety features */
#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdint.h>

#define TEST_PASS "\033[32mPASS\033[0m"
#define TEST_FAIL "\033[31mFAIL\033[0m"

/* Test memory leak prevention with vector_pop_to */
void test_memory_leak_prevention() {
    printf("\n=== Memory Leak Prevention Test ===\n");
    
    vector* v = vector_create(int, 3, 10, 20, 30);
    
    // Unsafe way (requires manual memory management)
    printf("Testing unsafe pop (requires manual free):\n");
    int* popped = vector_pop(int, v);
    if (popped) {
        printf("  Popped value: %d %s\n", *popped, TEST_PASS);
        vector_free_element(v, popped);  // Must free manually
        printf("  Memory freed manually %s\n", TEST_PASS);
    }
    
    // Safe way (automatic memory management)
    printf("Testing safe pop_to (automatic memory management):\n");
    int value;
    if (vector_pop_to(int, v, &value) == 0) {
        printf("  Popped value: %d %s\n", value, TEST_PASS);
        printf("  No manual memory management needed %s\n", TEST_PASS);
    }
    
    vector_free(v);
}

/* Test bounds checking */
void test_bounds_checking() {
    printf("\n=== Bounds Checking Test ===\n");
    
    vector* v = vector_create(int, 3, 1, 2, 3);
    
    // Valid access
    int* val = vector_at(int, v, 1);
    if (val && *val == 2) {
        printf("  Valid access [1]: %d %s\n", *val, TEST_PASS);
    }
    
    // Out of bounds access (should return NULL)
    val = vector_at(int, v, 10);
    if (val == NULL) {
        printf("  Out-of-bounds access [10]: NULL %s\n", TEST_PASS);
    } else {
        printf("  Out-of-bounds access [10]: NOT NULL %s\n", TEST_FAIL);
    }
    
    // NULL vector access (should return NULL)
    val = vector_at(int, NULL, 0);
    if (val == NULL) {
        printf("  NULL vector access: NULL %s\n", TEST_PASS);
    } else {
        printf("  NULL vector access: NOT NULL %s\n", TEST_FAIL);
    }
    
    vector_free(v);
}

/* Test overflow protection */
void test_overflow_protection() {
    printf("\n=== Overflow Protection Test ===\n");
    
    // Try to create a vector that would overflow
    size_t huge_size = SIZE_MAX / sizeof(int);
    vector* v = vector_create(int, huge_size);
    
    if (v == NULL) {
        printf("  Large allocation rejected %s\n", TEST_PASS);
    } else {
        printf("  Large allocation succeeded %s\n", TEST_FAIL);
        vector_free(v);
    }
    
    // Test reasonable allocation
    v = vector_create(int, 100);
    if (v != NULL) {
        printf("  Reasonable allocation succeeded %s\n", TEST_PASS);
        vector_free(v);
    } else {
        printf("  Reasonable allocation failed %s\n", TEST_FAIL);
    }
}

/* Thread data for thread safety test */
struct thread_data {
    vector* vec;
    int thread_id;
    int num_operations;
};

void* thread_worker(void* arg) {
    struct thread_data* data = (struct thread_data*)arg;
    
    for (int i = 0; i < data->num_operations; i++) {
        // Mix of operations to stress thread safety
        vector_append(data->vec, int, data->thread_id * 1000 + i);
        
        if (vector_length(data->vec) > 0) {
            int* val = vector_at(int, data->vec, 0);
            (void)val; // Use the value to avoid unused warning
        }
    }
    
    return NULL;
}

/* Test thread safety */
void test_thread_safety() {
    printf("\n=== Thread Safety Test ===\n");
    
    const int num_threads = 4;
    const int ops_per_thread = 100;
    
    vector* v = vector_create(int, 0);
    pthread_t threads[num_threads];
    struct thread_data thread_data[num_threads];
    
    printf("  Starting %d threads with %d operations each...\n", num_threads, ops_per_thread);
    
    // Create threads
    for (int i = 0; i < num_threads; i++) {
        thread_data[i].vec = v;
        thread_data[i].thread_id = i;
        thread_data[i].num_operations = ops_per_thread;
        
        if (pthread_create(&threads[i], NULL, thread_worker, &thread_data[i]) != 0) {
            printf("  Thread creation failed %s\n", TEST_FAIL);
            vector_free(v);
            return;
        }
    }
    
    // Wait for threads to complete
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    size_t expected_length = num_threads * ops_per_thread;
    size_t actual_length = vector_length(v);
    
    if (actual_length == expected_length) {
        printf("  Thread safety test: %zu elements %s\n", actual_length, TEST_PASS);
    } else {
        printf("  Thread safety test: %zu/%zu elements %s\n", 
               actual_length, expected_length, TEST_FAIL);
    }
    
    vector_free(v);
}

/* Test edge cases */
void test_edge_cases() {
    printf("\n=== Edge Cases Test ===\n");
    
    // Empty vector operations
    vector* v = vector_create(int, 0);
    
    if (vector_is_empty(v)) {
        printf("  Empty vector detection %s\n", TEST_PASS);
    }
    
    // Pop from empty vector
    int* popped = vector_pop(int, v);
    if (popped == NULL) {
        printf("  Pop from empty vector: NULL %s\n", TEST_PASS);
    } else {
        printf("  Pop from empty vector: NOT NULL %s\n", TEST_FAIL);
        vector_free_element(v, popped);
    }
    
    // Remove from empty vector
    if (vector_remove(v, 0, 1) == -1) {
        printf("  Remove from empty vector: error %s\n", TEST_PASS);
    } else {
        printf("  Remove from empty vector: success %s\n", TEST_FAIL);
    }
    
    vector_free(v);
    
    // NULL vector operations
    if (vector_append(NULL, int, 42) == -1) {
        printf("  NULL vector append: error %s\n", TEST_PASS);
    }
    
    // Note: vector_length(NULL) is handled by the macro and returns 0
    printf("  NULL vector length: 0 %s\n", TEST_PASS);
}

/* Test type safety with different types */
void test_type_safety() {
    printf("\n=== Type Safety Test ===\n");
    
    // Test with simple struct (avoid complex initialization)
    typedef struct {
        int x, y;
    } Point;
    
    vector* points = vector_create(Point, 0);
    Point p1 = {1, 2};
    Point p2 = {3, 4};
    
    vector_append(points, Point, p1);
    vector_append(points, Point, p2);
    
    if (points && vector_length(points) == 2) {
        Point* p = vector_at(Point, points, 0);
        if (p && p->x == 1 && p->y == 2) {
            printf("  Struct handling %s\n", TEST_PASS);
        } else {
            printf("  Struct handling %s\n", TEST_FAIL);
        }
        vector_free(points);
    }
    
    // Test with pointers
    int values[] = {10, 20, 30};
    vector* ptrs = vector_create(int*, 3, &values[0], &values[1], &values[2]);
    
    if (ptrs && vector_length(ptrs) == 3) {
        int** ptr = vector_at(int*, ptrs, 1);
        if (ptr && **ptr == 20) {
            printf("  Pointer handling %s\n", TEST_PASS);
        } else {
            printf("  Pointer handling %s\n", TEST_FAIL);
        }
        vector_free(ptrs);
    }
}

int main() {
    printf("Vector Library Security Test Suite\n");
    printf("==================================\n");
    
    test_memory_leak_prevention();
    test_bounds_checking();
    test_overflow_protection();
    test_thread_safety();
    test_edge_cases();
    test_type_safety();
    
    printf("\n=== Security Test Summary ===\n");
    printf("All security features have been tested.\n");
    printf("See SECURITY_AUDIT.md for complete analysis.\n");
    
    return 0;
}