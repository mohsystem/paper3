#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

unsigned char* copy_chunk(const unsigned char* input, size_t len) {
    unsigned char* chunk1 = len ? (unsigned char*)malloc(len) : NULL;
    if (len && chunk1) memcpy(chunk1, input, len);

    unsigned char* chunk2 = len ? (unsigned char*)malloc(len) : NULL;
    if (len && chunk2) memcpy(chunk2, chunk1, len);

    if (chunk1) free(chunk1);
    return chunk2; // caller should free
}

void print_bytes(const unsigned char* data, size_t len) {
    printf("[");
    for (size_t i = 0; i < len; ++i) {
        printf("%u", (unsigned int)data[i]);
        if (i + 1 < len) printf(", ");
    }
    printf("]\n");
}

int main(void) {
    const char* s1 = "Hello, World!";
    size_t l1 = strlen(s1);
    unsigned char* o1 = copy_chunk((const unsigned char*)s1, l1);
    print_bytes(o1, l1);
    free(o1);

    const char* s2 = "foo";
    size_t l2 = strlen(s2);
    unsigned char* o2 = copy_chunk((const unsigned char*)s2, l2);
    print_bytes(o2, l2);
    free(o2);

    const char* s3 = "The quick brown fox";
    size_t l3 = strlen(s3);
    unsigned char* o3 = copy_chunk((const unsigned char*)s3, l3);
    print_bytes(o3, l3);
    free(o3);

    const char* s4 = "";
    size_t l4 = strlen(s4);
    unsigned char* o4 = copy_chunk((const unsigned char*)s4, l4);
    print_bytes(o4, l4);
    free(o4); // free(NULL) is safe

    unsigned char a5[] = {0, 1, 2, 3, 127, 255};
    size_t l5 = sizeof(a5) / sizeof(a5[0]);
    unsigned char* o5 = copy_chunk(a5, l5);
    print_bytes(o5, l5);
    free(o5);

    return 0;
}