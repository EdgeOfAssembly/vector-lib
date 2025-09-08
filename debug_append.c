#include "vector.h"
#include <stdio.h>

int main() {
    vector* v = vector_create(int, 0);
    printf("Created empty vector, length: %zu\n", vector_length(v));
    
    printf("Appending single element (42)...\n");
    int result = vector_append(v, int, 42);
    printf("Append result: %d, new length: %zu\n", result, vector_length(v));
    
    if (vector_length(v) > 0) {
        int* val = vector_at(int, v, 0);
        if (val) {
            printf("First element: %d\n", *val);
        }
    }
    
    printf("Appending two elements (100, 200)...\n");
    result = vector_append(v, int, 100, 200);
    printf("Append result: %d, new length: %zu\n", result, vector_length(v));
    
    printf("All elements:\n");
    for (size_t i = 0; i < vector_length(v); i++) {
        int* val = vector_at(int, v, i);
        if (val) {
            printf("  [%zu]: %d\n", i, *val);
        }
    }
    
    vector_free(v);
    return 0;
}