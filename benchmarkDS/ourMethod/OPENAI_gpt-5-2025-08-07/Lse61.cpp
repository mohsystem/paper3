#include <cstddef>
#include <iostream>
#include <new>

static const std::size_t MAX_BUFFER_SIZE = 16u * 1024u * 1024u; // 16 MiB

static void secure_zero(void* ptr, std::size_t len) {
    volatile unsigned char* p = static_cast<volatile unsigned char*>(ptr);
    while (len--) {
        *p++ = 0;
    }
}

// Allocates a buffer of size buffer_size, then frees it in a loop safely.
// Returns the number of times the buffer was freed (including final cleanup if needed).
std::size_t free_buffer_in_loop(std::size_t buffer_size, std::size_t loops) {
    if (buffer_size > MAX_BUFFER_SIZE) {
        return 0;
    }

    unsigned char* buf = nullptr;
    if (buffer_size > 0) {
        buf = new (std::nothrow) unsigned char[buffer_size];
        if (!buf) {
            return 0;
        }
    }

    std::size_t frees = 0;
    for (std::size_t i = 0; i < loops; ++i) {
        if (buf != nullptr) {
            secure_zero(buf, buffer_size);
            delete[] buf;
            buf = nullptr;
            ++frees;
        }
    }

    // Ensure no leak if not freed in the loop
    if (buf != nullptr) {
        secure_zero(buf, buffer_size);
        delete[] buf;
        buf = nullptr;
        ++frees;
    }

    return frees;
}

int main() {
    // 5 test cases
    std::cout << free_buffer_in_loop(1024, 3) << "\n";             // Expect 1
    std::cout << free_buffer_in_loop(2048, 1) << "\n";             // Expect 1
    std::cout << free_buffer_in_loop(4096, 0) << "\n";             // Expect 1 (cleanup)
    std::cout << free_buffer_in_loop(8192, 5) << "\n";             // Expect 1
    std::cout << free_buffer_in_loop(16u * 1024u * 1024u, 2) << "\n"; // Expect 1
    return 0;
}