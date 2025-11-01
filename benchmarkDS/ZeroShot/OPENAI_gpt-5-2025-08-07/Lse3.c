#include <stdio.h>
#include <string.h>
#include <stddef.h>

// Copy n bytes from src to dest. Returns dest on success, NULL on error.
void* copy_bytes(void* dest, const void* src, size_t n) {
    if (n == 0) return dest;
    if (dest == NULL || src == NULL) return NULL;
    // Use memmove to safely handle potential overlaps
    return memmove(dest, src, n);
}

int main(void) {
    // Test 1: Normal copy of "Hello"
    const char src1[] = "Hello";
    char dest1[10] = {0};
    void* r1p = copy_bytes(dest1, src1, 5);
    dest1[5] = '\0';
    printf("Test1 result=%d dest='%s'\n", r1p ? 5 : -1, dest1);

    // Test 2: Copy 0 bytes (no-op)
    char dest2[3] = {0};
    void* r2p = copy_bytes(dest2, src1, 0);
    printf("Test2 result=%d\n", r2p ? 0 : -1);

    // Test 3: Exact size buffer for "Hello"
    char dest3[6] = {0};
    void* r3p = copy_bytes(dest3, src1, 5);
    dest3[5] = '\0';
    printf("Test3 result=%d dest='%s'\n", r3p ? 5 : -1, dest3);

    // Test 4: Overlap scenario within the same buffer
    char buf[16] = "HelloWorld";
    void* r4p = copy_bytes(buf + 5, buf, 5); // copy "Hello" to position 5-9
    buf[10] = '\0';
    printf("Test4 result=%d buf='%s'\n", r4p ? 5 : -1, buf);

    // Test 5: Null source (should fail, return NULL)
    char dest5[10] = {0};
    void* r5p = copy_bytes(dest5, NULL, 5);
    printf("Test5 result=%d\n", r5p ? 5 : -1);

    return 0;
}