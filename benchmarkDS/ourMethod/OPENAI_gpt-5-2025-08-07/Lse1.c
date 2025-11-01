#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>

typedef struct {
    unsigned char* data;
    size_t len;
} Buffer;

// Copies 'n' bytes from src->data to dest->data. Returns number of bytes copied on success, -1 on error.
int mem_copy(Buffer* dest, const Buffer* src, size_t n) {
    if (dest == NULL || src == NULL || dest->data == NULL || src->data == NULL) {
        return -1;
    }
    if (n > dest->len || n > src->len) {
        return -1;
    }
    if (n > (size_t)INT_MAX) {
        return -1;
    }
    if (n == 0) {
        return 0;
    }
    memmove(dest->data, src->data, n);
    return (int)n;
}

static void print_hex(const unsigned char* data, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        printf("%02X", data[i]);
        if (i + 1 < len) printf(" ");
    }
}

int main(void) {
    // Test 1: normal copy of 5 bytes
    Buffer src1 = {0}, dest1 = {0};
    src1.len = 10; dest1.len = 10;
    src1.data = (unsigned char*)calloc(src1.len, 1);
    dest1.data = (unsigned char*)calloc(dest1.len, 1);
    if (!src1.data || !dest1.data) { fprintf(stderr, "alloc fail\n"); return 1; }
    for (size_t i = 0; i < src1.len; ++i) src1.data[i] = (unsigned char)i;
    int r1 = mem_copy(&dest1, &src1, 5);
    printf("Test1 result=%d dest1=", r1); print_hex(dest1.data, dest1.len); printf("\n");

    // Test 2: copy zero bytes
    Buffer src2 = {0}, dest2 = {0};
    src2.len = 5; dest2.len = 5;
    unsigned char s2[] = {1,2,3,4,5};
    unsigned char d2[] = {9,9,9,9,9};
    src2.data = s2; dest2.data = d2;
    int r2 = mem_copy(&dest2, &src2, 0);
    printf("Test2 result=%d dest2=", r2); print_hex(dest2.data, dest2.len); printf("\n");

    // Test 3: copy full size exact
    Buffer src3 = {0}, dest3 = {0};
    unsigned char s3[] = {'A','B','C','D','E','F','G','H'};
    src3.data = s3; src3.len = sizeof(s3);
    dest3.data = (unsigned char*)calloc(8, 1); dest3.len = 8;
    if (!dest3.data) { fprintf(stderr, "alloc fail\n"); return 1; }
    int r3 = mem_copy(&dest3, &src3, 8);
    printf("Test3 result=%d dest3=", r3); print_hex(dest3.data, dest3.len); printf("\n");

    // Test 4: attempt to copy too many bytes (should fail)
    Buffer src4 = {0}, dest4 = {0};
    unsigned char s4[] = {10,20,30,40,50};
    unsigned char d4[] = {0,0,0};
    src4.data = s4; src4.len = sizeof(s4);
    dest4.data = d4; dest4.len = sizeof(d4);
    int r4 = mem_copy(&dest4, &src4, 4);
    printf("Test4 result=%d dest4=", r4); print_hex(dest4.data, dest4.len); printf("\n");

    // Test 5: same buffer as both src and dest
    unsigned char sameArr[] = {7,7,7,7,7,7};
    Buffer same = { sameArr, sizeof(sameArr) };
    int r5 = mem_copy(&same, &same, 6);
    printf("Test5 result=%d same=", r5); print_hex(same.data, same.len); printf("\n");

    free(src1.data);
    free(dest1.data);
    free(dest3.data);
    return 0;
}