#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef struct {
    unsigned char data[10];
    int ok; /* 1 on success, 0 on failure */
} CopyResult;

/* Secure zero function to prevent optimization removal */
static void secure_zero(void *ptr, size_t len) {
    volatile unsigned char *p = (volatile unsigned char *)ptr;
    while (len--) {
        *p++ = 0;
    }
}

/* Allocates two chunks of size 10, copies first to second, frees both, returns a detached copy */
CopyResult allocate_copy_free(const unsigned char *input, size_t input_len) {
    enum { SIZE = 10 };
    CopyResult result;
    memset(result.data, 0, SIZE);
    result.ok = 0;

    unsigned char *chunk1 = NULL;
    unsigned char *chunk2 = NULL;

    /* Allocate chunk1 (zero-initialized) */
    chunk1 = (unsigned char *)calloc(SIZE, 1);
    if (!chunk1) {
        goto cleanup;
    }

    /* Bounded copy of input into chunk1 */
    if (input && input_len > 0) {
        size_t copy_len = input_len < SIZE ? input_len : SIZE;
        if (copy_len > 0) {
            memcpy(chunk1, input, copy_len);
        }
    }

    /* Allocate chunk2 (zero-initialized) */
    chunk2 = (unsigned char *)calloc(SIZE, 1);
    if (!chunk2) {
        goto cleanup;
    }

    /* Copy chunk1 -> chunk2 safely within bounds */
    memcpy(chunk2, chunk1, SIZE);

    /* Detach result copy before freeing both chunks */
    memcpy(result.data, chunk2, SIZE);
    result.ok = 1;

cleanup:
    if (chunk1) {
        secure_zero(chunk1, SIZE);
        free(chunk1);
        chunk1 = NULL;
    }
    if (chunk2) {
        secure_zero(chunk2, SIZE);
        free(chunk2);
        chunk2 = NULL;
    }
    return result;
}

static void print_result(const CopyResult *r) {
    printf("ok=%d data=[", r->ok);
    for (size_t i = 0; i < 10; ++i) {
        if (i) printf(", ");
        printf("%u", (unsigned)r->data[i]);
    }
    printf("]\n");
}

int main(void) {
    /* 5 test cases */
    unsigned char in1[] = {1, 2, 3};
    unsigned char in2[] = {0};
    unsigned char in3[] = {0,1,2,3,4,5,6,7,8,9};
    unsigned char in4[] = {10,11,12,13,14,15,16,17,18,19,20}; /* length 11 */
    unsigned char in5[] = {255, 0, 128};

    CopyResult r1 = allocate_copy_free(in1, sizeof(in1));
    CopyResult r2 = allocate_copy_free(NULL, 0);
    CopyResult r3 = allocate_copy_free(in3, sizeof(in3));
    CopyResult r4 = allocate_copy_free(in4, sizeof(in4));
    CopyResult r5 = allocate_copy_free(in5, sizeof(in5));

    print_result(&r1);
    print_result(&r2);
    print_result(&r3);
    print_result(&r4);
    print_result(&r5);

    return 0;
}