#include <stdio.h>
#include <stdlib.h>

// Allocates a buffer of size buffer_size, then frees it in a loop.
// Returns the number of times the buffer was actually freed (non-NULL to NULL transition).
int allocate_and_free_in_loop(size_t buffer_size, int loops) {
    void* buffer = malloc(buffer_size);
    int frees = 0;
    for (int i = 0; i < loops; ++i) {
        if (buffer != NULL) {
            free(buffer);
            buffer = NULL;
            frees++;
        } else {
            // free(NULL) is a no-op; still "freeing in a loop"
            free(buffer);
        }
    }
    return frees;
}

int main(void) {
    size_t sizes[5] = {0, 1, 16, 1024, 65536};
    int loops[5] = {1, 3, 5, 2, 10};
    for (int i = 0; i < 5; ++i) {
        int res = allocate_and_free_in_loop(sizes[i], loops[i]);
        printf("Test %d: size=%zu, loops=%d, frees=%d\n", i + 1, sizes[i], loops[i], res);
    }
    return 0;
}