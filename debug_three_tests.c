/* Test runner for first three tests */
#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "unity.h"
#include "vector.h"

/* Test basic vector creation and destruction */
void test_vector_create_and_free(void)
{
    printf("DEBUG: test_vector_create_and_free starting\n");
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
    printf("DEBUG: test_vector_create_and_free completed\n");
}

/* Test empty vector creation */
void test_vector_create_empty(void)
{
    printf("DEBUG: test_vector_create_empty starting\n");
    vector* v = vector_create(int, 0);
    TEST_ASSERT_NOT_NULL(v);
    TEST_ASSERT_EQUAL_INT(0, vector_length(v));
    TEST_ASSERT_EQUAL_INT(0, vector_capacity(v));
    TEST_ASSERT(vector_is_empty(v));
    
    vector_free(v);
    printf("DEBUG: test_vector_create_empty completed\n");
}

/* Test vector append operations */
void test_vector_append(void)
{
    printf("DEBUG: test_vector_append starting\n");
    vector* v = vector_create(int, 0);
    printf("DEBUG: Created vector for append test, v=%p, length=%zu\n", (void*)v, vector_length(v));
    TEST_ASSERT_NOT_NULL(v);
    
    /* Append single element */
    printf("DEBUG: About to append 42\n");
    int result = vector_append(v, int, 42);
    printf("DEBUG: Append 42 result=%d, length=%zu\n", result, vector_length(v));
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_INT(1, vector_length(v));
    TEST_ASSERT_EQUAL_INT(42, *vector_at(int, v, 0));
    
    /* Append multiple elements */
    printf("DEBUG: About to append 100, 200\n");
    result = vector_append(v, int, 100, 200);
    printf("DEBUG: Append 100,200 result=%d, length=%zu\n", result, vector_length(v));
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_INT(3, vector_length(v));
    TEST_ASSERT_EQUAL_INT(100, *vector_at(int, v, 1));
    TEST_ASSERT_EQUAL_INT(200, *vector_at(int, v, 2));
    
    vector_free(v);
    printf("DEBUG: test_vector_append completed\n");
}

int main(void)
{
    unity_begin();
    RUN_TEST(test_vector_create_and_free);
    RUN_TEST(test_vector_create_empty);
    RUN_TEST(test_vector_append);
    return unity_end();
}