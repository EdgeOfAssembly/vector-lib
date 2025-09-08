#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>

#define ARG_COUNT_N(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, N, ...) N
#define ARG_COUNT(...) ARG_COUNT_N(__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

/* Test compound literal behavior */
void test_compound_literal(void)
{
    printf("Testing compound literal behavior...\n");
    
    /* Test 1: Single element */
    {
        int count = ARG_COUNT(42);
        const int arr[] = {42};
        printf("Single element: ARG_COUNT(42) = %d, array size = %zu\n", count, sizeof(arr)/sizeof(arr[0]));
    }
    
    /* Test 2: Two elements */
    {
        int count = ARG_COUNT(100, 200);
        const int arr[] = {100, 200};
        printf("Two elements: ARG_COUNT(100, 200) = %d, array size = %zu\n", count, sizeof(arr)/sizeof(arr[0]));
    }
    
    /* Test 3: Check if compound literal has correct size */
    {
        const int* arr = (const int[]){42};
        printf("Compound literal single: first element = %d\n", arr[0]);
    }
    
    {
        const int* arr = (const int[]){100, 200};
        printf("Compound literal double: first = %d, second = %d\n", arr[0], arr[1]);
    }
}

int main() {
    test_compound_literal();
    return 0;
}