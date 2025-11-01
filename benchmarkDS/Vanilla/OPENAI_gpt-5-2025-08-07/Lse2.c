#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void* copy_bytes(void* dest, const void* src, size_t n) {
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    while (n--) {
        *d++ = *s++;
    }
    return dest;
}

int main(void) {
    struct TestCase { const char* src; size_t n; size_t dest_size; } tests[] = {
        {"Hello, World!", 13, 20},
        {"Hello, World!", 5, 5},
        {"Data", 4, 10},
        {"ByteCopy", 0, 10},
        {"Sample", 6, 6},
    };

    for (int i = 0; i < 5; ++i) {
        const char* src = tests[i].src;
        size_t n = tests[i].n;
        size_t dest_size = tests[i].dest_size;

        unsigned char* dest = (unsigned char*)calloc(dest_size, 1);
        if (!dest) {
            fprintf(stderr, "Allocation failed\n");
            return 1;
        }

        copy_bytes(dest, src, n);
        printf("Test %d: \"", i + 1);
        printf("%.*s", (int)n, dest);
        printf("\"\n");

        free(dest);
    }

    return 0;
}