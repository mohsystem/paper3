#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>

#define MAX_STACK (1024u * 1024u) /* 1 MiB cap to avoid stack exhaustion */

/*
 * Allocates a buffer of size buffer_size on the stack and fills it with 'A'.
 * Returns a heap-allocated NUL-terminated copy of the buffer (caller must free),
 * or NULL on invalid input or allocation failure.
 */
char* fill_buffer_on_stack(size_t buffer_size) {
    if (buffer_size > MAX_STACK) {
        return NULL;
    }
    /* Handle zero-size explicitly; return an empty string */
    if (buffer_size == 0) {
        char *out = (char*)malloc(1);
        if (!out) return NULL;
        out[0] = '\0';
        return out;
    }
    /* Use a VLA for stack allocation within a controlled cap */
    size_t n = buffer_size;
    char buf[/* VLA size */  (size_t)1]; /* placeholder to satisfy some compilers */
    /* To ensure standard-compliant VLA with variable size, use a compound statement */
    {
        char vla_buf[n];
        memset(vla_buf, 'A', n);

        /* Allocate heap copy to return */
        if (n > SIZE_MAX - 1) {
            return NULL; /* overflow guard for n+1 */
        }
        char *out = (char*)malloc(n + 1);
        if (!out) {
            return NULL;
        }
        memcpy(out, vla_buf, n);
        out[n] = '\0';
        return out;
    }
}

int main(void) {
    size_t tests[5] = {0, 1, 5, 16, 1024};
    for (size_t i = 0; i < 5; ++i) {
        size_t sz = tests[i];
        char *result = fill_buffer_on_stack(sz);
        if (!result) {
            printf("size=%zu error\n", sz);
            continue;
        }
        if (sz <= 32) {
            printf("size=%zu len=%zu sample=%s\n", sz, strlen(result), result);
        } else {
            /* Print only a prefix to avoid huge output */
            char sample[33];
            memcpy(sample, result, 32);
            sample[32] = '\0';
            printf("size=%zu len=%zu sample=%s...\n", sz, strlen(result), sample);
        }
        free(result);
    }
    return 0;
}