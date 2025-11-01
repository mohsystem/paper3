#include <iostream>
#include <cstdlib>
#include <utility>

// Allocates a buffer of size buffer_size, then frees it in a loop.
// Returns the number of times the buffer was actually freed (non-null to null transition).
int allocate_and_free_in_loop(std::size_t buffer_size, int loops) {
    void* buffer = std::malloc(buffer_size);
    int frees = 0;
    for (int i = 0; i < loops; ++i) {
        if (buffer != nullptr) {
            std::free(buffer);
            buffer = nullptr;
            frees++;
        } else {
            // free(nullptr) is a no-op; still "freeing in a loop"
            std::free(buffer);
        }
    }
    return frees;
}

int main() {
    std::pair<std::size_t, int> tests[5] = {
        {0, 1},
        {1, 3},
        {16, 5},
        {1024, 2},
        {65536, 10}
    };
    for (int i = 0; i < 5; ++i) {
        int res = allocate_and_free_in_loop(tests[i].first, tests[i].second);
        std::cout << "Test " << (i + 1)
                  << ": size=" << tests[i].first
                  << ", loops=" << tests[i].second
                  << ", frees=" << res << std::endl;
    }
    return 0;
}