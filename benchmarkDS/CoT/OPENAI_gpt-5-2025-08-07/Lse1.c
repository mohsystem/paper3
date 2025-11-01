#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

void* safe_memcpy(void* dest, const void* src, size_t n) {
    if (n == 0) return dest;
    if (dest == NULL || src == NULL) return NULL;
    if (dest == src) return dest;

    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;

    // Handle overlap safely (memmove semantics)
    if (d < s || d >= s + n) {
        // No harmful overlap or dest is before src: copy forward
        for (size_t i = 0; i < n; ++i) d[i] = s[i];
    } else {
        // Overlap with dest after src: copy backward
        for (size_t i = n; i > 0; --i) d[i - 1] = s[i - 1];
    }
    return dest;
}

static void print_bytes(const char* label, const unsigned char* data, size_t n) {
    printf("%s: [", label);
    for (size_t i = 0; i < n; ++i) {
        printf("%d", (int)data[i]);
        if (i + 1 < n) printf(", ");
    }
    printf("]\n");
}

int main() {
    // Test 1: Basic copy
    unsigned char src1[] = {1, 2, 3, 4, 5};
    unsigned char dst1[5] = {0};
    safe_memcpy(dst1, src1, 5);
    print_bytes("Test1 dest", dst1, 5);

    // Test 2: Zero-length copy
    unsigned char src2[] = {7, 8, 9};
    unsigned char dst2[] = {9, 9, 9};
    safe_memcpy(dst2, src2, 0);
    print_bytes("Test2 dest", dst2, 3);

    // Test 3: Overlapping forward copy
    unsigned char buf3[] = {'a','b','c','d','e','f','g','\0'};
    safe_memcpy(buf3 + 2, buf3, 5); // copy "abcde" to start at index 2
    printf("Test3 dest: %s\n", buf3);

    // Test 4: Overlapping backward copy
    unsigned char buf4[] = {'1','2','3','4','5','6','7','\0'};
    safe_memcpy(buf4, buf4 + 2, 5); // copy "34567" to start at index 0
    printf("Test4 dest: %s\n", buf4);

    // Test 5: Copy with buffers of integers cast to bytes
    uint32_t isrc[3] = {0x11223344u, 0x55667788u, 0x99AABBCCu};
    uint32_t idst[3] = {0};
    safe_memcpy(idst, isrc, sizeof(isrc));
    print_bytes("Test5 dest bytes", (unsigned char*)idst, sizeof(idst));

    return 0;
}