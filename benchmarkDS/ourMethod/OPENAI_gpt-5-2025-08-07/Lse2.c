#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

// Function: copies a number of bytes from src to dest.
// First argument: destination pointer
// Second argument: source pointer
// Third argument: number of bytes to copy
// Additional arguments: destination capacity and source length for bounds checking.
// Returns: number of bytes actually copied.
size_t safe_copy_bytes(uint8_t *dest, const uint8_t *src, size_t n, size_t dest_capacity, size_t src_length) {
    if (dest == NULL || src == NULL || n == 0) {
        return 0;
    }
    size_t to_copy = n;
    if (to_copy > dest_capacity) to_copy = dest_capacity;
    if (to_copy > src_length) to_copy = src_length;
    if (to_copy == 0) {
        return 0;
    }
    memmove(dest, src, to_copy);
    return to_copy;
}

static void print_result(const char *label, const uint8_t *buf, size_t copied) {
    printf("%s bytes_copied=%zu, content='%.*s'\n", label, copied, (int)copied, (const char*)buf);
}

int main(void) {
    // Test 1: Copy full message to sufficiently large buffer
    const char *msg1 = "Hello, Secure World!";
    uint8_t buffer1[64];
    memset(buffer1, 0, sizeof(buffer1));
    size_t c1 = safe_copy_bytes(buffer1, (const uint8_t*)msg1, strlen(msg1), sizeof(buffer1), strlen(msg1));
    if (c1 < sizeof(buffer1)) buffer1[c1] = '\0';
    print_result("Test 1", buffer1, c1);

    // Test 2: Partial copy (n smaller than source and dest)
    const char *msg2 = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    uint8_t buffer2[32];
    memset(buffer2, 0, sizeof(buffer2));
    size_t c2 = safe_copy_bytes(buffer2, (const uint8_t*)msg2, 5, sizeof(buffer2), strlen(msg2));
    if (c2 < sizeof(buffer2)) buffer2[c2] = '\0';
    print_result("Test 2", buffer2, c2);

    // Test 3: n larger than destination capacity (should be bounded by dest)
    const char *msg3 = "1234567890abcdefghijklmnop";
    uint8_t buffer3[10];
    memset(buffer3, 0, sizeof(buffer3));
    size_t c3 = safe_copy_bytes(buffer3, (const uint8_t*)msg3, 50, sizeof(buffer3), strlen(msg3));
    // Can't always null-terminate if copied equals capacity; use precision print
    print_result("Test 3", buffer3, c3);

    // Test 4: Zero-length copy (should copy nothing)
    const char *msg4 = "NoCopy";
    uint8_t buffer4[10];
    memset(buffer4, 0, sizeof(buffer4));
    size_t c4 = safe_copy_bytes(buffer4, (const uint8_t*)msg4, 0, sizeof(buffer4), strlen(msg4));
    if (c4 < sizeof(buffer4)) buffer4[c4] = '\0';
    print_result("Test 4", buffer4, c4);

    // Test 5: Empty source (should copy zero)
    const char *msg5 = "";
    uint8_t buffer5[10];
    memset(buffer5, 0, sizeof(buffer5));
    size_t c5 = safe_copy_bytes(buffer5, (const uint8_t*)msg5, 5, sizeof(buffer5), 0);
    if (c5 < sizeof(buffer5)) buffer5[c5] = '\0';
    print_result("Test 5", buffer5, c5);

    return 0;
}