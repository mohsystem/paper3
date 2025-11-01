#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

void mem_copy(unsigned char* dest, const unsigned char* src, size_t n) {
    if (!dest || !src) return;
    for (size_t i = 0; i < n; ++i) {
        dest[i] = src[i];
    }
}

void print_hex(const unsigned char* b, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        printf("%02X", b[i]);
        if (i + 1 < n) printf(" ");
    }
    printf("\n");
}

int main(void) {
    // Test 1: Copy full ASCII buffer
    unsigned char src1[] = "HelloWorld"; // 10 bytes + null terminator in storage
    unsigned char dest1[10] = {0};
    mem_copy(dest1, src1, 10);
    printf("T1 HEX: "); print_hex(dest1, 10);
    printf("T1 STR: %.*s\n", 10, dest1);

    // Test 2: Partial copy
    unsigned char src2[] = "OpenAI";
    unsigned char dest2[4] = {0};
    mem_copy(dest2, src2, 4); // "Open"
    printf("T2 HEX: "); print_hex(dest2, 4);
    printf("T2 STR: %.*s\n", 4, dest2);

    // Test 3: Zero-length copy (dest remains unchanged)
    unsigned char src3[] = "XYZ";
    unsigned char dest3[3] = {0xEE, 0xEE, 0xEE};
    mem_copy(dest3, src3, 0);
    printf("T3 HEX: "); print_hex(dest3, 3);

    // Test 4: Binary data copy
    unsigned char src4[5] = {0x00, 0x01, 0xAB, 0xFF, 0x10};
    unsigned char dest4[5] = {0};
    mem_copy(dest4, src4, 5);
    printf("T4 HEX: "); print_hex(dest4, 5);

    // Test 5: Copy first 7 bytes
    unsigned char src5[] = "1234567890";
    unsigned char dest5[7] = {0};
    mem_copy(dest5, src5, 7); // "1234567"
    printf("T5 HEX: "); print_hex(dest5, 7);
    printf("T5 STR: %.*s\n", 7, dest5);

    return 0;
}