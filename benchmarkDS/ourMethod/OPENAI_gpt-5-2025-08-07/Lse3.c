#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stddef.h>

size_t copy_bytes(void *dest, const void *src, size_t n, size_t dest_capacity, size_t src_size) {
    if ((dest == NULL || src == NULL) && n > 0) {
        return 0;
    }
    size_t m = n;
    if (m > dest_capacity) m = dest_capacity;
    if (m > src_size) m = src_size;
    if (m > 0) {
        memmove(dest, src, m);
    }
    return m;
}

int main(void) {
    const char *src = "Hello";
    const size_t src_len = 5;

    // Test 1: Exact fit
    char dest1[5] = {0};
    size_t copied1 = copy_bytes(dest1, src, 5, sizeof(dest1), src_len);
    printf("Test1 copied: %zu data: %.*s\n", copied1, (int)copied1, dest1);

    // Test 2: Destination smaller than requested
    char dest2[3] = {0};
    size_t copied2 = copy_bytes(dest2, src, 5, sizeof(dest2), src_len);
    printf("Test2 copied: %zu data: %.*s\n", copied2, (int)copied2, dest2);

    // Test 3: Destination larger than requested (exact request)
    char dest3[10] = {0};
    size_t copied3 = copy_bytes(dest3, src, 5, sizeof(dest3), src_len);
    printf("Test3 copied: %zu data: %.*s\n", copied3, (int)copied3, dest3);

    // Test 4: Copy fewer bytes than available
    char dest4[10] = {0};
    size_t copied4 = copy_bytes(dest4, src, 3, sizeof(dest4), src_len);
    printf("Test4 copied: %zu data: %.*s\n", copied4, (int)copied4, dest4);

    // Test 5: Copy zero bytes
    char dest5[5] = {0};
    size_t copied5 = copy_bytes(dest5, src, 0, sizeof(dest5), src_len);
    printf("Test5 copied: %zu data: %.*s\n", copied5, (int)copied5, dest5);

    return 0;
}