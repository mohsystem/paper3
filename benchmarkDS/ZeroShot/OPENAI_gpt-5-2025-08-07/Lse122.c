#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct Buffer10 {
    uint8_t data[10];
};

static void secure_bzero(void* ptr, size_t len) {
    volatile unsigned char* p = (volatile unsigned char*)ptr;
    while (len--) {
        *p++ = 0;
    }
}

struct Buffer10 copy_two_chunks(const uint8_t src[10]) {
    struct Buffer10 out = {{0}};
    if (src == NULL) {
        return out;
    }

    size_t n = 10;

    uint8_t* chunk1 = (uint8_t*)malloc(n);
    uint8_t* chunk2 = (uint8_t*)malloc(n);
    if (chunk1 == NULL || chunk2 == NULL) {
        free(chunk1);
        free(chunk2);
        return out;
    }

    memcpy(chunk1, src, n);
    memcpy(chunk2, chunk1, n);

    memcpy(out.data, chunk2, n);

    secure_bzero(chunk1, n);
    secure_bzero(chunk2, n);
    free(chunk1);
    free(chunk2);

    return out;
}

static void print_buf(const struct Buffer10* b) {
    printf("[");
    for (int i = 0; i < 10; ++i) {
        if (i) printf(", ");
        printf("%u", (unsigned)b->data[i]);
    }
    printf("]\n");
}

int main(void) {
    uint8_t tests[5][10] = {
        {0,1,2,3,4,5,6,7,8,9},
        {10,20,30,40,50,60,70,80,90,100},
        {255,254,253,252,251,250,249,248,247,246},
        {42,42,42,42,42,42,42,42,42,42},
        {9,8,7,6,5,4,3,2,1,0}
    };

    for (int i = 0; i < 5; ++i) {
        struct Buffer10 res = copy_two_chunks(tests[i]);
        print_buf(&res);
    }

    return 0;
}