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
   ```

2. Copy vector.h and align.h to your project’s include directory.

3. Include vector.h in your code:
    ```bash
    #include "vector.h"
   ```
## Usage

See example.c for a complete example. Basic usage:

    ```c
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
   ```

To run the example, compile it with a C compiler (e.g., gcc):
    ```bash
gcc -std=c99 -pthread example.c -o example
./example
    ```

## Licensing

This library is dual-licensed:

- GNU General Public License v3.0 (GPLv3): For open-source use. See LICENSE.GPL for details. Suitable for projects that comply with GPLv3’s terms, requiring derivative works to be open-source.

- BSD 3-Clause License: For commercial use. Contact Stefan Fröberg at haxbox2000@gmail.com (mailto:haxbox2000@gmail.com) to obtain a commercial license. A fee may apply, and upon agreement, you will receive the BSD 3-Clause license, allowing proprietary use and distribution.

Note: By default, the library is distributed under GPLv3 (LICENSE.GPL). The BSD 3-Clause license is not included in the repository and must be obtained directly from the author for commercial use.

## Contributing

Contributions are welcome! Please:

1. Fork the repository.
2. Create a feature branch (git checkout -b feature/your-feature).
3. Commit changes (git commit -m "Add your feature").
4. Push to the branch (git push origin feature/your-feature).
5. Open a pull request.

Report bugs or suggest features via GitHub Issues.


## Author

- Name: Stefan Fröberg
- Email: haxbox2000@gmail.com (mailto:haxbox2000@gmail.com)

## Acknowledgments

- Inspired by C++'s std::vector and other C dynamic array libraries.
- Uses align.h for cross-compiler alignment support.

## Contact

For questions, bug reports, or commercial licensing inquiries, email Stefan Fröberg at haxbox2000@gmail.com (mailto:haxbox2000@gmail.com).
