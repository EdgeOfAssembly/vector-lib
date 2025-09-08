CC = gcc
CFLAGS = -std=c99 -pthread -Wall -Wextra -g -O0
INCLUDES = -I. -I./tests
LDFLAGS = -pthread

# Source files
VECTOR_HEADER = vector.h
TEST_SOURCES = tests/test_vector.c tests/unity.c
TEST_BINARY = tests/test_vector

# Default target
all: test

# Build and run tests
test: $(TEST_BINARY)
	@echo "Running vector library tests..."
	./$(TEST_BINARY)

# Build test binary
$(TEST_BINARY): $(TEST_SOURCES) $(VECTOR_HEADER)
	$(CC) $(CFLAGS) $(INCLUDES) $(TEST_SOURCES) -o $(TEST_BINARY) $(LDFLAGS)

# Build example
example: example.c $(VECTOR_HEADER)
	$(CC) $(CFLAGS) example.c -o example $(LDFLAGS)

# Memory leak checking with valgrind
memcheck: $(TEST_BINARY)
	@echo "Running memory leak detection..."
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(TEST_BINARY)

# Static analysis with cppcheck (if available)
static-analysis:
	@if command -v cppcheck >/dev/null 2>&1; then \
		echo "Running static analysis..."; \
		cppcheck --enable=all --std=c99 --suppress=missingIncludeSystem vector.h tests/test_vector.c; \
	else \
		echo "cppcheck not available, skipping static analysis"; \
	fi

# Address sanitizer build
asan: $(TEST_SOURCES) $(VECTOR_HEADER)
	$(CC) $(CFLAGS) -fsanitize=address -fno-omit-frame-pointer $(INCLUDES) $(TEST_SOURCES) -o $(TEST_BINARY)_asan $(LDFLAGS)
	./$(TEST_BINARY)_asan

# Thread sanitizer build
tsan: $(TEST_SOURCES) $(VECTOR_HEADER)
	$(CC) $(CFLAGS) -fsanitize=thread $(INCLUDES) $(TEST_SOURCES) -o $(TEST_BINARY)_tsan $(LDFLAGS)
	./$(TEST_BINARY)_tsan

# Clean build artifacts
clean:
	rm -f $(TEST_BINARY) $(TEST_BINARY)_asan $(TEST_BINARY)_tsan example

# Help target
help:
	@echo "Available targets:"
	@echo "  test          - Build and run tests"
	@echo "  example       - Build example program"
	@echo "  memcheck      - Run valgrind memory leak detection"
	@echo "  static-analysis - Run cppcheck static analysis"
	@echo "  asan          - Build and run with AddressSanitizer"
	@echo "  tsan          - Build and run with ThreadSanitizer"
	@echo "  clean         - Remove build artifacts"
	@echo "  help          - Show this help message"

.PHONY: all test example memcheck static-analysis asan tsan clean help