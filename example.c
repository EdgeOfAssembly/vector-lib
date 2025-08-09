#include "vector.h"
#include <stdio.h>

int main() {
    // Create a vector of integers
    vector* v = vector_create(int, 3, 1, 2, 3);
    
    // Append elements
    vector_append(v, int, 4);
    
    // Access and print elements
    for (size_t i = 0; i < vector_length(v); i++) {
        printf("%d ", *vector_at(int, v, i));
    }
    printf("\n"); // Output: 1 2 3 4
    
    // Free the vector
    vector_free(v);
    return 0;
}