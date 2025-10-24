/* Chain-of-Through process in code generation applied with secure checks.
   Language: C */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>

typedef struct {
    int success;           /* 1 on success, 0 on failure */
    unsigned char value;   /* valid only if success == 1 */
} ReadResult;

#define MAX_ALLOC 10000000U /* Prevent excessive allocation */

unsigned char* allocate_buffer(size_t size) {
    if (size > MAX_ALLOC) {
        return NULL;
    }
    /* Check for overflow not necessary for single element size, but keep pattern */
    unsigned char* buf = (unsigned char*)malloc(size * sizeof(unsigned char));
    if (buf == NULL) {
        return NULL;
    }
    for (size_t i = 0; i < size; ++i) {
        buf[i] = (unsigned char)(i & 0xFFu);
    }
    return buf;
}

ReadResult read_from_buffer(const unsigned char* buffer, size_t size, long long index) {
    ReadResult r;
    if (buffer == NULL) {
        r.success = 0;
        r.value = 0;
        return r;
    }
    if (index < 0) {
        r.success = 0;
        r.value = 0;
        return r;
    }
    size_t idx = (size_t)index;
    if (idx >= size) {
        r.success = 0;
        r.value = 0;
        return r;
    }
    r.success = 1;
    r.value = buffer[idx];
    return r;
}

int main(void) {
    size_t size = 10;
    unsigned char* buffer = allocate_buffer(size);
    if (buffer == NULL) {
        fprintf(stderr, "Failed to allocate buffer\n");
        return 1;
    }

    long long test_indices[5] = {0, 5, 9, -1, 10};
    for (size_t i = 0; i < 5; ++i) {
        long long idx = test_indices[i];
        ReadResult res = read_from_buffer(buffer, size, idx);
        if (res.success) {
            printf("Index %lld -> %u\n", idx, (unsigned int)res.value);
        } else {
            printf("Index %lld -> invalid index\n", idx);
        }
    }

    /* Clean up */
    free(buffer);
    return 0;
}