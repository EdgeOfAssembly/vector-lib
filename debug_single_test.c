/* Single test runner for append test */
#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "unity.h"
#include "vector.h"

/* Test vector append operations */
void test_vector_append(void)
{
    printf("DEBUG: Starting test_vector_append\n");
    vector* v = vector_create(int, 0);
    printf("DEBUG: Created vector, v=%p\n", (void*)v);
    TEST_ASSERT_NOT_NULL(v);
    
    printf("DEBUG: About to append single element\n");
    /* Append single element */
    int append_result = vector_append(v, int, 42);
    printf("DEBUG: Append result=%d, length=%zu\n", append_result, vector_length(v));
    TEST_ASSERT_EQUAL_INT(0, append_result);
    TEST_ASSERT_EQUAL_INT(1, vector_length(v));
    
    int* first_val = vector_at(int, v, 0);
    printf("DEBUG: First element pointer=%p, value=%d\n", (void*)first_val, first_val ? *first_val : -999);
    TEST_ASSERT_EQUAL_INT(42, *first_val);
    
    printf("DEBUG: About to append two elements\n");
    /* Append multiple elements */
    append_result = vector_append(v, int, 100, 200);
    printf("DEBUG: Second append result=%d, length=%zu\n", append_result, vector_length(v));
    TEST_ASSERT_EQUAL_INT(0, append_result);
    TEST_ASSERT_EQUAL_INT(3, vector_length(v));
    
    int* second_val = vector_at(int, v, 1);
    int* third_val = vector_at(int, v, 2);
    printf("DEBUG: Second element pointer=%p, value=%d\n", (void*)second_val, second_val ? *second_val : -999);
    printf("DEBUG: Third element pointer=%p, value=%d\n", (void*)third_val, third_val ? *third_val : -999);
    TEST_ASSERT_EQUAL_INT(100, *second_val);
    TEST_ASSERT_EQUAL_INT(200, *third_val);
    
    vector_free(v);
    printf("DEBUG: test_vector_append completed\n");
}

int main(void)
{
    unity_begin();
    RUN_TEST(test_vector_append);
    return unity_end();
}