/* Chain-of-Through process in code generation (as comments within code):
 * 1) Problem understanding: Allocate a buffer of size buffer_size, then attempt to free it in a loop safely.
 * 2) Security requirements: Avoid double-free, validate inputs, avoid excessive allocation, scrub before free.
 * 3) Secure coding generation: Use guards, cap max size, set pointer to NULL after free, and zero memory securely.
 * 4) Code review: Ensure safe release in loop, handle allocation failures.
 * 5) Secure code output: Final safe implementation with tests.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define MAX_BUFFER_SIZE ((size_t)100000000ULL) /* 100 MB */

/* Securely zero memory to avoid compiler optimizing out memset */
static void secure_memzero(void *ptr, size_t len) {
    volatile unsigned char *p = (volatile unsigned char *)ptr;
    while (len--) {
        *p++ = 0;
    }
}

/* Allocates a buffer of size buffer_size, then frees it in a loop safely.
 * Returns the number of successful free operations (0 or 1).
 */
size_t allocate_and_free(size_t buffer_size, size_t iterations) {
    if (iterations == 0) return 0;
    if (buffer_size == 0 || buffer_size > MAX_BUFFER_SIZE) return 0;

    unsigned char *buffer = (unsigned char *)malloc(buffer_size);
    if (buffer == NULL) {
        return 0;
    }

    size_t freed = 0;
    for (size_t i = 0; i < iterations; ++i) {
        if (buffer != NULL) {
            /* Overwrite before free to reduce data remanence */
            secure_memzero(buffer, buffer_size);
            free(buffer);
            buffer = NULL; /* prevent double-free */
            ++freed;
        }
    }

    return freed;
}

int main(void) {
    /* 5 test cases */
    printf("Test 1 (1024, 5): %zu\n", allocate_and_free((size_t)1024, (size_t)5));               /* Expect 1 */
    printf("Test 2 (0, 10): %zu\n", allocate_and_free((size_t)0, (size_t)10));                    /* Expect 0 */
    printf("Test 3 (10, 1): %zu\n", allocate_and_free((size_t)10, (size_t)1));                    /* Expect 1 */
    printf("Test 4 (0, 3): %zu\n", allocate_and_free((size_t)0, (size_t)3));                      /* Negative not applicable; using 0 => Expect 0 */
    printf("Test 5 (200000000, 2): %zu\n", allocate_and_free((size_t)200000000ULL, (size_t)2));    /* Expect 0 */
    return 0;
}