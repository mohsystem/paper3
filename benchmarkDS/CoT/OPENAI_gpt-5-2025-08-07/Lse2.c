/* 
1) Problem understanding: copy N bytes from src to dest.
2) Security requirements: handle NULL and overlap safely.
3) Secure coding generation: memmove-like safe logic.
4) Code review: callers must ensure capacities; tests use adequate buffers.
5) Secure code output: final function below.
*/

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

void* copy_bytes(void* destination, const void* source, size_t n) {
    if (!destination || !source) return destination;
    if (n == 0 || destination == source) return destination;

    unsigned char* dest = (unsigned char*)destination;
    const unsigned char* src = (const unsigned char*)source;

    if (dest < src || dest >= src + n) {
        for (size_t i = 0; i < n; ++i) dest[i] = src[i];
    } else {
        for (size_t i = n; i > 0; --i) dest[i - 1] = src[i - 1];
    }
    return destination;
}

static void print_result(const char* label, const unsigned char* buf, size_t copied) {
    printf("%s | copied=%zu | result=\"%.*s\"\n", label, copied, (int)copied, (const char*)buf);
}

int main(void) {
    const char* msg = "Hello, World!";
    size_t msg_len = strlen(msg);
    const unsigned char* src = (const unsigned char*)msg;

    // Test 1: exact-size buffer
    unsigned char* dest1 = (unsigned char*)calloc(msg_len + 1, 1);
    copy_bytes(dest1, src, msg_len);
    print_result("Test1 (exact copy)", dest1, msg_len);

    // Test 2: larger buffer
    unsigned char* dest2 = (unsigned char*)calloc(64, 1);
    copy_bytes(dest2, src, msg_len);
    print_result("Test2 (into larger buffer)", dest2, msg_len);

    // Test 3: partial copy (first 5 bytes)
    unsigned char* dest3 = (unsigned char*)calloc(5 + 1, 1);
    copy_bytes(dest3, src, 5);
    print_result("Test3 (partial 5 bytes)", dest3, 5);

    // Test 4: zero-length copy
    unsigned char* dest4 = (unsigned char*)calloc(10, 1);
    copy_bytes(dest4, src, 0);
    print_result("Test4 (zero bytes)", dest4, 0);

    // Test 5: overlapping copy within same buffer (safe)
    unsigned char* buf = (unsigned char*)calloc(msg_len + 1, 1);
    copy_bytes(buf, src, msg_len);
    // Shift first 5 bytes by 2 positions to the right
    copy_bytes(buf + 2, buf, 5);
    print_result("Test5 (overlap shift right by 2 of first 5 bytes)", buf, msg_len);

    free(dest1);
    free(dest2);
    free(dest3);
    free(dest4);
    free(buf);
    return 0;
}