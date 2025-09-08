/* Unity Test Framework - Minimal Version */
#ifndef UNITY_H
#define UNITY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Test counters */
extern int unity_tests_run;
extern int unity_tests_failed;

/* Test result macros */
#define TEST_PASS 0
#define TEST_FAIL 1

/* Test assertion macros */
#define TEST_ASSERT(condition) \
    do { \
        unity_tests_run++; \
        if (!(condition)) { \
            printf("FAIL: %s:%d - %s\n", __FILE__, __LINE__, #condition); \
            unity_tests_failed++; \
        } else { \
            printf("PASS: %s\n", #condition); \
        } \
    } while(0)

#define TEST_ASSERT_EQUAL_INT(expected, actual) \
    do { \
        unity_tests_run++; \
        if ((expected) != (actual)) { \
            printf("FAIL: %s:%d - Expected %ld, got %ld\n", __FILE__, __LINE__, (long)(expected), (long)(actual)); \
            unity_tests_failed++; \
        } else { \
            printf("PASS: Expected %ld == %ld\n", (long)(expected), (long)(actual)); \
        } \
    } while(0)

#define TEST_ASSERT_EQUAL_PTR(expected, actual) \
    do { \
        unity_tests_run++; \
        if ((expected) != (actual)) { \
            printf("FAIL: %s:%d - Expected %p, got %p\n", __FILE__, __LINE__, (expected), (actual)); \
            unity_tests_failed++; \
        } else { \
            printf("PASS: Expected %p == %p\n", (expected), (actual)); \
        } \
    } while(0)

#define TEST_ASSERT_NULL(ptr) TEST_ASSERT_EQUAL_PTR(NULL, ptr)
#define TEST_ASSERT_NOT_NULL(ptr) TEST_ASSERT((ptr) != NULL)

#define TEST_ASSERT_EQUAL_MEMORY(expected, actual, size) \
    do { \
        unity_tests_run++; \
        if (memcmp((expected), (actual), (size)) != 0) { \
            printf("FAIL: %s:%d - Memory contents differ\n", __FILE__, __LINE__); \
            unity_tests_failed++; \
        } else { \
            printf("PASS: Memory contents match\n"); \
        } \
    } while(0)

/* Test runner */
#define RUN_TEST(test_func) \
    do { \
        printf("\n--- Running %s ---\n", #test_func); \
        test_func(); \
    } while(0)

/* Unity initialization and teardown */
void unity_begin(void);
int unity_end(void);

#endif /* UNITY_H */