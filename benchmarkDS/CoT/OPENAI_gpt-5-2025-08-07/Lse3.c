#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Step 1-5: Secure byte copy with validation and overlap-safety using a temporary buffer.
// Note: Caller must ensure 'n' does not exceed available sizes for dest and src.
size_t copy_bytes(void *dest, const void *src, size_t n) {
    if (dest == NULL || src == NULL || n == 0) return 0;
    void *temp = malloc(n);
    if (temp == NULL) {
        return 0;
    }
    memcpy(temp, src, n);
    memcpy(dest, temp, n);
    free(temp);
    return n;
}
// End of secure copy

static void print_as_string(const char *label, const unsigned char *buf, size_t count) {
    printf("%s", label);
    for (size_t i = 0; i < count; ++i) {
        putchar((char)buf[i]);
    }
    putchar('"');
    putchar('\n');
}

int main(void) {
    // Test 1: Copy "Hello" (5 bytes)
    unsigned char dest1[16] = {0};
    const unsigned char src1[] = "Hello"; // 5 bytes + null terminator
    size_t c1 = copy_bytes(dest1, src1, 5);
    printf("Test1 bytesCopied=%zu content=\"", c1);
    print_as_string("", dest1, c1);

    // Test 2: Destination smaller (3) - copy only 3 bytes safely by caller choice
    unsigned char dest2[3] = {0};
    size_t c2 = copy_bytes(dest2, src1, 3);
    printf("Test2 bytesCopied=%zu content=\"", c2);
    print_as_string("", dest2, c2);

    // Test 3: Zero length copy
    unsigned char dest3[10] = {0};
    size_t c3 = copy_bytes(dest3, src1, 0);
    printf("Test3 bytesCopied=%zu content=\"", c3);
    print_as_string("", dest3, c3);

    // Test 4: Overlapping regions (memmove-like) - copy "Hello" into same buffer offset
    unsigned char buf4[16] = "HelloWorld";
    size_t c4 = copy_bytes(buf4 + 5, buf4, 5); // result should be "HelloHello"
    printf("Test4 bytesCopied=%zu content=\"", c4);
    print_as_string("", buf4, 10);

    // Test 5: Null source - should safely return 0
    unsigned char dest5[10] = {0};
    size_t c5 = copy_bytes(dest5, NULL, 5);
    printf("Test5 bytesCopied=%zu content=\"", c5);
    print_as_string("", dest5, c5);

    return 0;
}