#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

void* memmove_secure(void* dest, const void* src, size_t count) {
    if (dest == NULL || src == NULL) {
        return NULL;
    }
    if (count == 0 || dest == src) {
        return dest;
    }
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;

    if (d < s) {
        for (size_t i = 0; i < count; ++i) {
            d[i] = s[i];
        }
    } else {
        for (size_t i = count; i > 0; --i) {
            d[i - 1] = s[i - 1];
        }
    }
    return dest;
}

static void print_hex(const uint8_t* buf, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        printf("%02X%s", buf[i], (i + 1 < len) ? " " : "");
    }
}

int main(void) {
    // Test 1: Copy 10 bytes "HelloWorld"
    const char src1[] = "HelloWorld";
    char dest1[11] = {0};
    memmove_secure(dest1, src1, 10);
    printf("Test1: %s\n", dest1);

    // Test 2: Copy 3 bytes from "abcdef"
    const char src2[] = "abcdef";
    uint8_t dest2[6] = {0};
    memmove_secure(dest2, src2, 3);
    printf("Test2: ");
    fwrite(dest2, 1, 6, stdout);
    printf(" | Hex: ");
    print_hex(dest2, 6);
    printf("\n");

    // Test 3: Zero bytes copy (no change)
    const char src3[] = "xyz";
    uint8_t dest3[3] = {0};
    memmove_secure(dest3, src3, 0);
    printf("Test3: ");
    print_hex(dest3, 3);
    printf("\n");

    // Test 4: Null arguments (should return NULL)
    void* res4 = memmove_secure(NULL, src1, 5);
    printf("Test4: %s\n", (res4 == NULL) ? "NULL" : "not null");

    // Test 5: Same buffer as both src and dest
    char both[] = "12345";
    memmove_secure(both, both, 4);
    printf("Test5: %s\n", both);

    return 0;
}