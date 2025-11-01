#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int allocate_and_free_loop(size_t buffer_size, size_t iterations) {
    // Return -1 on invalid input scenarios unlikely in size_t context cannot check negative
    // Handle zero-size: simulate safe freeing logic
    if (buffer_size == 0) {
        int frees = 0;
        int freed = 0;
        for (size_t i = 0; i < iterations; ++i) {
            if (!freed) {
                freed = 1;
                ++frees;
            }
        }
        if (!freed) {
            ++frees;
        }
        return frees;
    }

    // Allocate buffer and initialize to zero to avoid uninitialized memory use
    void *buf = calloc(buffer_size, 1);
    if (buf == NULL) {
        return -1; // allocation failed
    }

    int frees = 0;
    // Attempt to free in a loop, but ensure it only happens once
    for (size_t i = 0; i < iterations; ++i) {
        if (buf != NULL) {
            free(buf);
            buf = NULL; // prevent double-free
            ++frees;
        }
    }

    // Ensure no leak if iterations == 0
    if (buf != NULL) {
        free(buf);
        buf = NULL;
        ++frees;
    }

    return frees;
}

int main(void) {
    // 5 test cases
    printf("%d\n", allocate_and_free_loop(16, 5));   // Expect 1
    printf("%d\n", allocate_and_free_loop(0, 5));    // Expect 1 (simulated single free)
    printf("%d\n", allocate_and_free_loop(1024, 1)); // Expect 1
    printf("%d\n", allocate_and_free_loop(8, 0));    // Expect 1 (cleanup)
    printf("%d\n", allocate_and_free_loop(32, 2));   // Expect 1
    return 0;
}