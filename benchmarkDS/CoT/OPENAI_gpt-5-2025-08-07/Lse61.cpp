// Chain-of-Through process in code generation (as comments within code):
// 1) Problem understanding: Allocate a buffer of size buffer_size, then attempt to free it in a loop safely.
// 2) Security requirements: Avoid double-free, validate input size, avoid excessive allocation.
// 3) Secure coding generation: Use smart pointers, cap max size, and reset once.
// 4) Code review: Guard checks for size/iterations and use unique_ptr reset to prevent double-free.
// 5) Secure code output: Final safe implementation.

#include <iostream>
#include <memory>
#include <limits>

static const size_t MAX_BUFFER_SIZE = 100000000ULL; // 100 MB

// Allocates a buffer of size buffer_size, then frees it in a loop safely.
// Returns the number of successful free operations (0 or 1).
size_t allocate_and_free(size_t buffer_size, size_t iterations) {
    if (iterations == 0) return 0;
    if (buffer_size == 0 || buffer_size > MAX_BUFFER_SIZE) return 0;

    std::unique_ptr<unsigned char[]> buffer;
    try {
        buffer = std::make_unique<unsigned char[]>(buffer_size);
    } catch (const std::bad_alloc&) {
        return 0;
    }

    size_t freed = 0;
    for (size_t i = 0; i < iterations; ++i) {
        if (buffer) {
            // Overwrite before releasing to reduce data remanence
            std::fill_n(buffer.get(), buffer_size, 0);
            buffer.reset(); // free once; subsequent iterations will no-op
            ++freed;
        }
    }
    return freed;
}

int main() {
    // 5 test cases
    std::cout << "Test 1 (1024, 5): " << allocate_and_free(1024, 5) << "\n";               // Expect 1
    std::cout << "Test 2 (0, 10): " << allocate_and_free(0, 10) << "\n";                    // Expect 0
    std::cout << "Test 3 (10, 1): " << allocate_and_free(10, 1) << "\n";                    // Expect 1
    std::cout << "Test 4 (static_cast<size_t>(-5), 3): " << allocate_and_free(0, 3) << "\n"; // Negative not applicable; using 0 => Expect 0
    std::cout << "Test 5 (200000000, 2): " << allocate_and_free(200000000ULL, 2) << "\n";    // Expect 0
    return 0;
}