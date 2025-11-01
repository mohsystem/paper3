#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#define MAX_BUFFER_SIZE (16u * 1024u * 1024u) /* 16 MiB */

static void secure_zero(void *ptr, size_t len) {
    volatile unsigned char *p = (volatile unsigned char *)ptr;
    while (len--) {
        *p++ = 0;
    }
}

/* Allocates a buffer of size buffer_size, then frees it in a loop safely.
   Returns the number of times the buffer was freed (including final cleanup if needed). */
size_t free_buffer_in_loop(size_t buffer_size, size_t loops) {
    if (buffer_size > MAX_BUFFER_SIZE) {
        return 0;
    }

    unsigned char *buf = NULL;
    if (buffer_size > 0) {
        buf = (unsigned char *)malloc(buffer_size);
        if (buf == NULL) {
            return 0;
        }
    }

    size_t frees = 0;
    for (size_t i = 0; i < loops; i++) {
        if (buf != NULL) {
            secure_zero(buf, buffer_size);
            free(buf);
            buf = NULL;
            frees++;
        }
    }

    /* Ensure no leak if not freed in the loop */
    if (buf != NULL) {
        secure_zero(buf, buffer_size);
        free(buf);
        buf = NULL;
        frees++;
    }

    return frees;
}

int main(void) {
    /* 5 test cases */
    printf("%zu\n", free_buffer_in_loop(1024, 3));              /* Expect 1 */
    printf("%zu\n", free_buffer_in_loop(2048, 1));              /* Expect 1 */
    printf("%zu\n", free_buffer_in_loop(4096, 0));              /* Expect 1 (cleanup) */
    printf("%zu\n", free_buffer_in_loop(8192, 5));              /* Expect 1 */
    printf("%zu\n", free_buffer_in_loop(16u * 1024u * 1024u, 2)); /* Expect 1 */
    return 0;
}