# Vector Library - Usage Examples and Best Practices

## Secure Usage Examples

### 1. Safe Memory Management

```c
#include "vector.h"

void safe_vector_usage() {
    vector* v = vector_create(int, 3, 10, 20, 30);
    
    // RECOMMENDED: Use vector_pop_to for automatic memory management
    int value;
    if (vector_pop_to(int, v, &value) == 0) {
        printf("Popped: %d\n", value);  // No memory leak
    }
    
    // AVOID: vector_pop requires manual memory management
    // int* popped = vector_pop(int, v);
    // if (popped) {
    //     printf("Popped: %d\n", *popped);
    //     vector_free_element(v, popped);  // Must remember to free!
    // }
    
    vector_free(v);
}
```

### 2. Thread-Safe Operations

```c
#include "vector.h"
#include <pthread.h>

vector* shared_vector;

void* worker_thread(void* arg) {
    int thread_id = *(int*)arg;
    
    // All vector operations are thread-safe
    for (int i = 0; i < 100; i++) {
        vector_append(shared_vector, int, thread_id * 1000 + i);
        
        // Safe concurrent reads
        if (vector_length(shared_vector) > 0) {
            int* val = vector_at(int, shared_vector, 0);
            // Use val...
        }
    }
    return NULL;
}

void thread_safe_example() {
    shared_vector = vector_create(int, 0);
    
    pthread_t threads[4];
    int thread_ids[4] = {0, 1, 2, 3};
    
    for (int i = 0; i < 4; i++) {
        pthread_create(&threads[i], NULL, worker_thread, &thread_ids[i]);
    }
    
    for (int i = 0; i < 4; i++) {
        pthread_join(threads[i], NULL);
    }
    
    printf("Final length: %zu\n", vector_length(shared_vector));
    vector_free(shared_vector);
}
```

### 3. Type-Safe Operations

```c
typedef struct {
    int id;
    double value;
} DataPoint;

void type_safe_example() {
    // Create vector of custom structs
    vector* data = vector_create(DataPoint, 0);
    
    DataPoint point = {1, 3.14};
    vector_append(data, DataPoint, point);
    
    // Type-safe access
    DataPoint* retrieved = vector_at(DataPoint, data, 0);
    if (retrieved) {
        printf("ID: %d, Value: %f\n", retrieved->id, retrieved->value);
    }
    
    vector_free(data);
}
```

### 4. Error Handling Best Practices

```c
void robust_error_handling() {
    vector* v = vector_create(int, 0);
    if (!v) {
        fprintf(stderr, "Failed to create vector\n");
        return;
    }
    
    // Always check return codes
    if (vector_append(v, int, 42) != 0) {
        fprintf(stderr, "Failed to append element\n");
        vector_free(v);
        return;
    }
    
    // Safe bounds checking
    int* val = vector_at(int, v, 0);
    if (val) {
        printf("Value: %d\n", *val);
    } else {
        printf("Index out of bounds\n");
    }
    
    vector_free(v);
}
```

## Performance Optimization Tips

### 1. Pre-allocate Capacity

```c
void performance_optimized() {
    // If you know the approximate size, reserve capacity
    vector* v = vector_create(int, 0);
    vector_reserve(v, 1000);  // Avoids multiple reallocations
    
    for (int i = 0; i < 1000; i++) {
        vector_append(v, int, i);  // O(1) operations
    }
    
    vector_free(v);
}
```

### 2. Batch Operations

```c
void batch_operations() {
    vector* v = vector_create(int, 0);
    
    // More efficient than multiple single appends
    vector_append(v, int, 1, 2, 3, 4, 5);
    
    vector_free(v);
}
```

### 3. Memory Reclamation

```c
void memory_efficient() {
    vector* v = vector_create(int, 1000);
    
    // Fill vector...
    for (int i = 0; i < 1000; i++) {
        vector_append(v, int, i);
    }
    
    // Remove many elements...
    for (int i = 0; i < 900; i++) {
        vector_remove(v, 0, 1);
    }
    
    // Reclaim unused memory
    vector_shrink_to_fit(v);
    
    vector_free(v);
}
```

## Security Considerations

### 1. Input Validation

```c
int safe_vector_operation(vector* v, size_t index) {
    if (!v) {
        return -1;  // Handle NULL vector
    }
    
    if (index >= vector_length(v)) {
        return -1;  // Handle out-of-bounds
    }
    
    // Safe to proceed
    int* val = vector_at(int, v, index);
    return val ? *val : -1;
}
```

### 2. Resource Management

```c
void safe_resource_management() {
    vector* v = vector_create(int, 100);
    if (!v) return;
    
    // Use RAII-style pattern or ensure cleanup
    // ... operations ...
    
    // Always clean up
    vector_free(v);
    v = NULL;  // Prevent double-free
}
```

## Integration Guidelines

### 1. In Existing Codebases

- Replace manual dynamic arrays with vector
- Use vector_pop_to() instead of vector_pop() for safety
- Add error checking for all vector operations
- Consider thread safety requirements

### 2. Performance-Critical Code

- Pre-allocate capacity with vector_reserve()
- Use vector_at_ptr() for hot paths (but ensure bounds safety)
- Consider vector_shrink_to_fit() for long-lived vectors
- Profile memory usage patterns

### 3. Multi-threaded Applications

- All operations are thread-safe by default
- Consider read-heavy vs write-heavy workloads
- Be aware of lock contention in high-concurrency scenarios
- Use appropriate error handling for concurrent failures

## Compilation and Linking

```bash
# Basic compilation
gcc -std=c99 -pthread your_code.c -o your_program

# With security flags
gcc -std=c99 -pthread -Wall -Wextra -O2 -D_FORTIFY_SOURCE=2 your_code.c -o your_program

# Debug build
gcc -std=c99 -pthread -Wall -Wextra -g -O0 your_code.c -o your_program

# With sanitizers
gcc -std=c99 -pthread -fsanitize=address -fsanitize=thread your_code.c -o your_program
```

Remember to include both `vector.h` and `align.h` in your project.