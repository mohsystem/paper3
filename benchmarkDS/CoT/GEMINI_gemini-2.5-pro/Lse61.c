#include <stdio.h>
#include <stdlib.h>

/**
 * WARNING: This function contains a deliberate double-free vulnerability.
 * It allocates a buffer and then frees it multiple times inside a loop.
 * This leads to undefined behavior and can crash the program or be exploited.
 * DO NOT use this pattern in production code.
 *
 * @param buffer_size The size of the buffer to allocate.
 * @param loop_count The number of times to free the buffer. Should be > 1 to cause a double-free.
 */
void vulnerable_double_free(size_t buffer_size, int loop_count) {
    if (buffer_size == 0 || loop_count <= 0) {
        printf("Invalid size or loop count.\n");
        return;
    }

    printf("Allocating buffer of size: %zu\n", buffer_size);
    char* buffer = (char*)malloc(buffer_size);

    if (buffer == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        return;
    }

    printf("Buffer allocated at: %p\n", (void*)buffer);

    for (int i = 0; i < loop_count; ++i) {
        printf("Loop %d: Freeing buffer.\n", i + 1);
        // The first call to free() is valid.
        // Any subsequent call with the same pointer value is a double-free,
        // which is undefined behavior.
        free(buffer);
    }
    printf("Completed vulnerable function for buffer size %zu.\n", buffer_size);
    printf("------------------------------------\n");
}

int main() {
    // Test Cases
    // Note: The program will likely crash on the first test case with loop_count > 1.
    // The behavior is undefined and may vary between systems and compilers.
    vulnerable_double_free(1024, 2);
    vulnerable_double_free(256, 3);
    vulnerable_double_free(1, 5);
    vulnerable_double_free(4096, 1); // This is safe, single free
    vulnerable_double_free(128, 10);

    return 0;
}