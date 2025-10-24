#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stddef.h>

#define MAX_SIZE (16u * 1024u * 1024u)

static void secure_memzero(void* p, size_t n) {
    volatile unsigned char* vp = (volatile unsigned char*)p;
    while (n--) {
        *vp++ = 0;
    }
}

// Dynamically allocates a memory buffer, uses it, securely clears it, and deallocates it.
// Returns checksum on success; ULLONG_MAX on error (invalid size or OOM).
unsigned long long allocate_and_use(size_t size) {
    if (size == 0 || size > MAX_SIZE) {
        return ULLONG_MAX;
    }

    unsigned char* buf = (unsigned char*)malloc(size);
    if (buf == NULL) {
        return ULLONG_MAX;
    }

    for (size_t i = 0; i < size; ++i) {
        buf[i] = (unsigned char)(i & 0xFFu);
    }

    unsigned long long checksum = 0ULL;
    for (size_t i = 0; i < size; ++i) {
        checksum += (unsigned long long)buf[i];
    }

    secure_memzero(buf, size);
    free(buf);
    buf = NULL;

    return checksum;
}

int main(void) {
    size_t tests[5] = {1u, 64u, 1024u, 4096u, 1000000u};
    for (int i = 0; i < 5; ++i) {
        unsigned long long res = allocate_and_use(tests[i]);
        printf("size=%zu checksum=%llu\n", tests[i], (unsigned long long)res);
    }
    return 0;
}