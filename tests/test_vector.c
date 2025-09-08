/* Vector Library Test Suite */
#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "unity.h"
#include "../vector.h"
#include <pthread.h>
#include <time.h>
#include <sys/wait.h>
#include <unistd.h>

/* Test basic vector creation and destruction */
void test_vector_create_and_free(void)
{
    vector* v = vector_create(int, 3, 1, 2, 3);
    TEST_ASSERT_NOT_NULL(v);
    TEST_ASSERT_EQUAL_INT(3, vector_length(v));
    TEST_ASSERT_EQUAL_INT(3, vector_capacity(v));
    
    int* val1 = vector_at(int, v, 0);
    int* val2 = vector_at(int, v, 1);
    int* val3 = vector_at(int, v, 2);
    
    TEST_ASSERT_NOT_NULL(val1);
    TEST_ASSERT_NOT_NULL(val2);
    TEST_ASSERT_NOT_NULL(val3);
    TEST_ASSERT_EQUAL_INT(1, *val1);
    TEST_ASSERT_EQUAL_INT(2, *val2);
    TEST_ASSERT_EQUAL_INT(3, *val3);
    
    vector_free(v);
}

/* Test empty vector creation */
void test_vector_create_empty(void)
{
    vector* v = vector_create(int, 0);
    TEST_ASSERT_NOT_NULL(v);
    TEST_ASSERT_EQUAL_INT(0, vector_length(v));
    TEST_ASSERT_EQUAL_INT(0, vector_capacity(v));
    TEST_ASSERT(vector_is_empty(v));
    
    vector_free(v);
}

/* Test vector append operations */
void test_vector_append(void)
{
    vector* v = vector_create(int, 0);
    TEST_ASSERT_NOT_NULL(v);
    
    /* Append single element */
    TEST_ASSERT_EQUAL_INT(0, vector_append(v, int, 42));
    TEST_ASSERT_EQUAL_INT(1, vector_length(v));
    TEST_ASSERT_EQUAL_INT(42, *vector_at(int, v, 0));
    
    /* Append multiple elements */
    TEST_ASSERT_EQUAL_INT(0, vector_append(v, int, 100, 200));
    TEST_ASSERT_EQUAL_INT(3, vector_length(v));
    TEST_ASSERT_EQUAL_INT(100, *vector_at(int, v, 1));
    TEST_ASSERT_EQUAL_INT(200, *vector_at(int, v, 2));
    
    vector_free(v);
}

/* Test vector bounds checking */
void test_vector_bounds_checking(void)
{
    vector* v = vector_create(int, 3, 1, 2, 3);
    TEST_ASSERT_NOT_NULL(v);
    
    /* Valid access */
    TEST_ASSERT_NOT_NULL(vector_at(int, v, 0));
    TEST_ASSERT_NOT_NULL(vector_at(int, v, 2));
    
    /* Out of bounds access */
    TEST_ASSERT_NULL(vector_at(int, v, 3));
    TEST_ASSERT_NULL(vector_at(int, v, 100));
    
    /* NULL vector access */
    TEST_ASSERT_NULL(vector_at(int, NULL, 0));
    
    vector_free(v);
}

/* Test vector pop operations */
void test_vector_pop(void)
{
    vector* v = vector_create(int, 3, 10, 20, 30);
    TEST_ASSERT_NOT_NULL(v);
    
    /* Pop last element */
    int* popped = vector_pop(int, v);
    TEST_ASSERT_NOT_NULL(popped);
    TEST_ASSERT_EQUAL_INT(30, *popped);
    TEST_ASSERT_EQUAL_INT(2, vector_length(v));
    vector_free_element(v, popped);
    
    /* Pop another element */
    popped = vector_pop(int, v);
    TEST_ASSERT_NOT_NULL(popped);
    TEST_ASSERT_EQUAL_INT(20, *popped);
    TEST_ASSERT_EQUAL_INT(1, vector_length(v));
    vector_free_element(v, popped);
    
    /* Test pop_to (safer alternative) */
    int dest;
    TEST_ASSERT_EQUAL_INT(0, vector_pop_to(int, v, &dest));
    TEST_ASSERT_EQUAL_INT(10, dest);
    TEST_ASSERT_EQUAL_INT(0, vector_length(v));
    
    /* Pop from empty vector should fail */
    TEST_ASSERT_NULL(vector_pop(int, v));
    TEST_ASSERT_EQUAL_INT(-1, vector_pop_to(int, v, &dest));
    
    vector_free(v);
}

/* Test vector insertion */
void test_vector_insert(void)
{
    vector* v = vector_create(int, 3, 1, 3, 5);
    TEST_ASSERT_NOT_NULL(v);
    
    /* Insert at beginning */
    TEST_ASSERT_EQUAL_INT(0, vector_insert(v, int, 0, 0));
    TEST_ASSERT_EQUAL_INT(4, vector_length(v));
    TEST_ASSERT_EQUAL_INT(0, *vector_at(int, v, 0));
    TEST_ASSERT_EQUAL_INT(1, *vector_at(int, v, 1));
    
    /* Insert in middle */
    TEST_ASSERT_EQUAL_INT(0, vector_insert(v, int, 2, 2));
    TEST_ASSERT_EQUAL_INT(5, vector_length(v));
    TEST_ASSERT_EQUAL_INT(2, *vector_at(int, v, 2));
    TEST_ASSERT_EQUAL_INT(3, *vector_at(int, v, 3));
    
    /* Insert at end */
    TEST_ASSERT_EQUAL_INT(0, vector_insert(v, int, 5, 6));
    TEST_ASSERT_EQUAL_INT(6, vector_length(v));
    TEST_ASSERT_EQUAL_INT(6, *vector_at(int, v, 5));
    
    /* Verify final sequence: [0, 1, 2, 3, 5, 6] */
    for (int i = 0; i < 6; i++) {
        if (i == 4) {
            TEST_ASSERT_EQUAL_INT(5, *vector_at(int, v, i));
        } else {
            TEST_ASSERT_EQUAL_INT(i, *vector_at(int, v, i));
        }
    }
    
    vector_free(v);
}

/* Test vector removal */
void test_vector_remove(void)
{
    vector* v = vector_create(int, 5, 1, 2, 3, 4, 5);
    TEST_ASSERT_NOT_NULL(v);
    
    /* Remove from middle */
    TEST_ASSERT_EQUAL_INT(0, vector_remove(v, 2, 1));
    TEST_ASSERT_EQUAL_INT(4, vector_length(v));
    TEST_ASSERT_EQUAL_INT(1, *vector_at(int, v, 0));
    TEST_ASSERT_EQUAL_INT(2, *vector_at(int, v, 1));
    TEST_ASSERT_EQUAL_INT(4, *vector_at(int, v, 2));
    TEST_ASSERT_EQUAL_INT(5, *vector_at(int, v, 3));
    
    /* Remove multiple elements */
    TEST_ASSERT_EQUAL_INT(0, vector_remove(v, 1, 2));
    TEST_ASSERT_EQUAL_INT(2, vector_length(v));
    TEST_ASSERT_EQUAL_INT(1, *vector_at(int, v, 0));
    TEST_ASSERT_EQUAL_INT(5, *vector_at(int, v, 1));
    
    /* Remove out of bounds should fail */
    TEST_ASSERT_EQUAL_INT(-1, vector_remove(v, 5, 1));
    TEST_ASSERT_EQUAL_INT(-1, vector_remove(v, 1, 10));
    
    vector_free(v);
}

/* Test vector resize operations */
void test_vector_resize(void)
{
    vector* v = vector_create(int, 3, 1, 2, 3);
    TEST_ASSERT_NOT_NULL(v);
    
    /* Resize to larger size */
    TEST_ASSERT_EQUAL_INT(0, vector_resize(v, 5));
    TEST_ASSERT_EQUAL_INT(5, vector_length(v));
    TEST_ASSERT_EQUAL_INT(1, *vector_at(int, v, 0));
    TEST_ASSERT_EQUAL_INT(2, *vector_at(int, v, 1));
    TEST_ASSERT_EQUAL_INT(3, *vector_at(int, v, 2));
    /* New elements should be zero-initialized */
    TEST_ASSERT_EQUAL_INT(0, *vector_at(int, v, 3));
    TEST_ASSERT_EQUAL_INT(0, *vector_at(int, v, 4));
    
    /* Resize to smaller size */
    TEST_ASSERT_EQUAL_INT(0, vector_resize(v, 2));
    TEST_ASSERT_EQUAL_INT(2, vector_length(v));
    TEST_ASSERT_EQUAL_INT(1, *vector_at(int, v, 0));
    TEST_ASSERT_EQUAL_INT(2, *vector_at(int, v, 1));
    
    vector_free(v);
}

/* Test vector with different data types */
void test_vector_different_types(void)
{
    /* Test with struct */
    typedef struct {
        int x, y;
    } Point;
    
    vector* points = vector_create(Point, 2, {1, 2}, {3, 4});
    TEST_ASSERT_NOT_NULL(points);
    TEST_ASSERT_EQUAL_INT(2, vector_length(points));
    
    Point* p1 = vector_at(Point, points, 0);
    Point* p2 = vector_at(Point, points, 1);
    TEST_ASSERT_EQUAL_INT(1, p1->x);
    TEST_ASSERT_EQUAL_INT(2, p1->y);
    TEST_ASSERT_EQUAL_INT(3, p2->x);
    TEST_ASSERT_EQUAL_INT(4, p2->y);
    
    vector_free(points);
    
    /* Test with pointers */
    int values[] = {10, 20, 30};
    vector* ptrs = vector_create(int*, 3, &values[0], &values[1], &values[2]);
    TEST_ASSERT_NOT_NULL(ptrs);
    
    int** ptr1 = vector_at(int*, ptrs, 0);
    int** ptr2 = vector_at(int*, ptrs, 1);
    TEST_ASSERT_EQUAL_INT(10, **ptr1);
    TEST_ASSERT_EQUAL_INT(20, **ptr2);
    
    vector_free(ptrs);
}

/* Test memory leak scenarios */
void test_memory_management(void)
{
    /* Test with many allocations and frees */
    for (int i = 0; i < 100; i++) {
        vector* v = vector_create(int, 10);
        TEST_ASSERT_NOT_NULL(v);
        
        /* Fill vector */
        for (int j = 0; j < 50; j++) {
            vector_append(v, int, j);
        }
        
        /* Pop some elements and free them properly */
        for (int j = 0; j < 10; j++) {
            int* popped = vector_pop(int, v);
            if (popped) {
                vector_free_element(v, popped);
            }
        }
        
        vector_free(v);
    }
}

/* Test edge cases */
void test_edge_cases(void)
{
    /* Test with zero-sized elements (should fail gracefully) */
    // Note: This might not be a valid test case as sizeof() is never 0
    
    /* Test with very large sizes (should fail gracefully) */
    vector* v = vector_create(int, SIZE_MAX / sizeof(int));
    /* This should fail due to overflow protection */
    TEST_ASSERT_NULL(v);
    
    /* Test NULL operations */
    TEST_ASSERT_EQUAL_INT(-1, vector_append(NULL, int, 1));
    TEST_ASSERT_EQUAL_INT(-1, vector_clear(NULL));
    TEST_ASSERT_EQUAL_INT(-1, vector_remove(NULL, 0, 1));
    TEST_ASSERT_EQUAL_INT(-1, vector_resize(NULL, 10));
    TEST_ASSERT_NULL(vector_copy(NULL));
    
    /* Test operations on empty vector */
    v = vector_create(int, 0);
    TEST_ASSERT_NOT_NULL(v);
    TEST_ASSERT_NULL(vector_pop(int, v));
    TEST_ASSERT_EQUAL_INT(-1, vector_remove(v, 0, 1));
    vector_free(v);
}

/* Thread-safety test data */
struct thread_test_data {
    vector* vec;
    int thread_id;
    int operations;
};

/* Thread function for concurrent access */
void* thread_append_worker(void* arg)
{
    struct thread_test_data* data = (struct thread_test_data*)arg;
    
    for (int i = 0; i < data->operations; i++) {
        int value = data->thread_id * 1000 + i;
        vector_append(data->vec, int, value);
        
        /* Small delay to increase chance of race conditions */
        struct timespec ts = {0, 1000}; /* 1 microsecond */
        nanosleep(&ts, NULL);
    }
    
    return NULL;
}

/* Test thread safety with concurrent appends */
void test_thread_safety_append(void)
{
    const int num_threads = 4;
    const int ops_per_thread = 50;
    
    vector* v = vector_create(int, 0);
    TEST_ASSERT_NOT_NULL(v);
    
    pthread_t threads[num_threads];
    struct thread_test_data thread_data[num_threads];
    
    /* Create threads */
    for (int i = 0; i < num_threads; i++) {
        thread_data[i].vec = v;
        thread_data[i].thread_id = i;
        thread_data[i].operations = ops_per_thread;
        
        int result = pthread_create(&threads[i], NULL, thread_append_worker, &thread_data[i]);
        TEST_ASSERT_EQUAL_INT(0, result);
    }
    
    /* Wait for all threads to complete */
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    /* Verify total number of elements */
    TEST_ASSERT_EQUAL_INT(num_threads * ops_per_thread, vector_length(v));
    
    /* Verify all elements are present (though order may vary due to threading) */
    int found_counts[4] = {0}; /* Fixed size for the test */
    for (size_t i = 0; i < vector_length(v); i++) {
        int* val = vector_at(int, v, i);
        TEST_ASSERT_NOT_NULL(val);
        
        int thread_id = *val / 1000;
        if (thread_id >= 0 && thread_id < num_threads) {
            found_counts[thread_id]++;
        }
    }
    
    /* Each thread should have contributed exactly ops_per_thread elements */
    for (int i = 0; i < num_threads; i++) {
        TEST_ASSERT_EQUAL_INT(ops_per_thread, found_counts[i]);
    }
    
    vector_free(v);
}

/* Custom allocator test */
static size_t custom_alloc_count = 0;
static size_t custom_free_count = 0;

void* custom_alloc(size_t size) {
    custom_alloc_count++;
    return malloc(size);
}

void* custom_realloc(void* ptr, size_t size) {
    return realloc(ptr, size);
}

void custom_free(void* ptr) {
    custom_free_count++;
    free(ptr);
}

void test_custom_allocators(void)
{
    /* Reset counters */
    custom_alloc_count = 0;
    custom_free_count = 0;
    
    /* Note: The current vector implementation doesn't expose custom allocator setting
     * This test would need the API to be extended to test custom allocators properly
     * For now, we'll test the default allocator behavior */
    
    vector* v = vector_create(int, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    TEST_ASSERT_NOT_NULL(v);
    
    /* Test that memory operations work */
    for (int i = 0; i < 100; i++) {
        vector_append(v, int, i);
    }
    
    TEST_ASSERT_EQUAL_INT(110, vector_length(v));
    
    vector_free(v);
}

/* Performance stress test */
void test_performance_stress(void)
{
    const int large_size = 10000;
    
    vector* v = vector_create(int, 0);
    TEST_ASSERT_NOT_NULL(v);
    
    /* Time the append operations */
    clock_t start = clock();
    
    for (int i = 0; i < large_size; i++) {
        vector_append(v, int, i);
    }
    
    clock_t end = clock();
    double cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    printf("Appended %d elements in %f seconds\n", large_size, cpu_time_used);
    
    TEST_ASSERT_EQUAL_INT(large_size, vector_length(v));
    
    /* Verify some elements */
    TEST_ASSERT_EQUAL_INT(0, *vector_at(int, v, 0));
    TEST_ASSERT_EQUAL_INT(large_size - 1, *vector_at(int, v, large_size - 1));
    
    vector_free(v);
}

/* Memory corruption detection */
void test_memory_corruption_detection(void)
{
    /* Test buffer overrun protection */
    vector* v = vector_create(int, 5, 1, 2, 3, 4, 5);
    TEST_ASSERT_NOT_NULL(v);
    
    /* Try to access beyond bounds - should return NULL */
    TEST_ASSERT_NULL(vector_at(int, v, 10));
    TEST_ASSERT_NULL(vector_at(int, v, SIZE_MAX));
    
    /* Try invalid removes */
    TEST_ASSERT_EQUAL_INT(-1, vector_remove(v, 10, 1));
    TEST_ASSERT_EQUAL_INT(-1, vector_remove(v, 0, SIZE_MAX));
    
    vector_free(v);
}

int main(void)
{
    unity_begin();
    
    /* Basic functionality tests */
    RUN_TEST(test_vector_create_and_free);
    RUN_TEST(test_vector_create_empty);
    RUN_TEST(test_vector_append);
    RUN_TEST(test_vector_bounds_checking);
    RUN_TEST(test_vector_pop);
    RUN_TEST(test_vector_insert);
    RUN_TEST(test_vector_remove);
    RUN_TEST(test_vector_resize);
    
    /* Type system tests */
    RUN_TEST(test_vector_different_types);
    
    /* Memory management tests */
    RUN_TEST(test_memory_management);
    
    /* Edge case tests */
    RUN_TEST(test_edge_cases);
    
    /* Thread safety tests */
    RUN_TEST(test_thread_safety_append);
    
    /* Allocator tests */
    RUN_TEST(test_custom_allocators);
    
    /* Performance tests */
    RUN_TEST(test_performance_stress);
    
    /* Security tests */
    RUN_TEST(test_memory_corruption_detection);
    
    return unity_end();
}