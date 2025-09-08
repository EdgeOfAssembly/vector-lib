/* Unity Test Framework Implementation */
#include "unity.h"

/* Global test counters */
int unity_tests_run = 0;
int unity_tests_failed = 0;

void unity_begin(void)
{
    unity_tests_run = 0;
    unity_tests_failed = 0;
    printf("Unity Test Framework\n");
    printf("===================\n");
}

int unity_end(void)
{
    printf("\n===================\n");
    printf("Tests run: %d\n", unity_tests_run);
    printf("Failures:  %d\n", unity_tests_failed);
    
    if (unity_tests_failed == 0) {
        printf("ALL TESTS PASSED\n");
        return 0;
    } else {
        printf("SOME TESTS FAILED\n");
        return 1;
    }
}