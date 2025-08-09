# Vector Library for C

A thread-safe, type-agnostic dynamic array library for C, inspired by C++'s `std::vector`. This single-header library supports custom allocators, serialization, sorting, and thread-safe operations using read-write locks. It is dual-licensed under GPLv3 for open-source use and BSD 3-Clause for commercial use (contact the author for commercial licensing).

## Features
- **Type-Agnostic**: Works with any data type using macros (e.g., `vector_create(int, ...)`).
- **Thread-Safe**: Uses Windows SRWLOCK or POSIX `pthread_rwlock_t` for concurrent reads and exclusive writes.
- **Custom Allocators**: Supports user-defined memory allocation functions.
- **Dynamic Resizing**: Amortized O(1) appends, O(n) inserts/removals.
- **Serialization**: Save and load vectors to/from files.
- **Alignment Support**: Uses `align.h` for proper memory alignment (e.g., for SIMD).
- **Comprehensive API**: Includes append, prepend, insert, remove, pop, sort, swap, and more.

## Requirements
- C99 or later.
- POSIX threads (`pthread`) on Linux or Windows API on Windows.
- Compiler support for alignment (`alignas`, `alignof`) or `align.h` fallbacks.

## Installation
1. Clone the repository:
   ```bash
   git clone git@github.com:EdgeOfAssembly/vector-lib.git
   cd vector-lib