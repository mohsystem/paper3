#include <iostream>
#include <memory>
#include <vector>
#include <cstdint>

int allocate_and_free_loop(std::size_t buffer_size, std::size_t iterations) {
    // Validate inputs
    if (iterations > static_cast<std::size_t>(-1) / 2) {
        return -1; // overly large iteration count
    }

    // Handle zero-size: simulate safe free once if loop runs or via cleanup
    if (buffer_size == 0) {
        bool freed = false;
        int frees = 0;
        for (std::size_t i = 0; i < iterations; ++i) {
            if (!freed) {
                freed = true;
                ++frees;
            }
        }
        if (!freed) {
            ++frees;
        }
        return frees;
    }

    class SafeBuffer {
    public:
        explicit SafeBuffer(std::size_t n)
            : data_(nullptr), freed_(false) {
            try {
                std::unique_ptr<uint8_t[]> tmp(new uint8_t[n]());
                data_ = std::move(tmp);
            } catch (...) {
                // Allocation failed
                data_.reset();
                freed_ = true; // nothing to free
                throw;
            }
        }

        int free() {
            if (!freed_) {
                data_.reset();
                freed_ = true;
                return 1;
            }
            return 0;
        }

        bool isFreed() const {
            return freed_;
        }

    private:
        std::unique_ptr<uint8_t[]> data_;
        bool freed_;
    };

    int frees = 0;
    try {
        SafeBuffer buf(buffer_size);
        for (std::size_t i = 0; i < iterations; ++i) {
            frees += buf.free();
        }
        if (!buf.isFreed()) {
            frees += buf.free();
        }
    } catch (...) {
        // Allocation failed; nothing to free
        return -1;
    }
    return frees;
}

int main() {
    // 5 test cases
    std::cout << allocate_and_free_loop(16, 5) << "\n";    // Expect 1
    std::cout << allocate_and_free_loop(0, 5) << "\n";     // Expect 1 (simulated single free)
    std::cout << allocate_and_free_loop(1024, 1) << "\n";  // Expect 1
    std::cout << allocate_and_free_loop(8, 0) << "\n";     // Expect 1 (cleanup)
    std::cout << allocate_and_free_loop(32, (std::size_t)2) << "\n"; // Expect 1
    return 0;
}