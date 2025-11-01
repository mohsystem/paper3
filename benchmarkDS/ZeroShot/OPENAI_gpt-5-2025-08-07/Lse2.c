#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>

size_t copy_bytes(unsigned char *dest, const unsigned char *src, size_t n) {
    if (dest == NULL || src == NULL || n == 0) {
        return 0;
    }
    if (dest == src) {
        return n;
    }
    if (dest < src || dest >= src + n) {
        for (size_t i = 0; i < n; ++i) {
            dest[i] = src[i];
        }
    } else {
        for (size_t i = n; i > 0; --i) {
            dest[i - 1] = src[i - 1];
        }
    }
    return n;
}

int main(void) {
    // Test 1: Normal copy
    const char *msg1 = "Secure copy test";
    unsigned char dest1[64] = {0};
    size_t n1 = strlen(msg1);
    size_t c1 = copy_bytes(dest1, (const unsigned char*)msg1, n1);
    fwrite(dest1, 1, c1, stdout);
    fputc('\n', stdout);

    // Test 2: n larger than destination length
    const char *msg2 = "ShortMessage";
    unsigned char dest2[5] = {0};
    size_t msg2_len = strlen(msg2);
    size_t n2 = 100;
    if (n2 > sizeof(dest2)) n2 = sizeof(dest2);
    if (n2 > msg2_len) n2 = msg2_len;
    size_t c2 = copy_bytes(dest2, (const unsigned char*)msg2, n2);
    fwrite(dest2, 1, c2, stdout);
    fputc('\n', stdout);

    // Test 3: n is zero
    const char *msg3 = "ZeroCopy";
    unsigned char dest3[16] = {0};
    size_t c3 = copy_bytes(dest3, (const unsigned char*)msg3, 0);
    printf("Bytes copied: %zu\n", c3);

    // Test 4: n larger than source length
    const char *msg4 = "SrcSmall";
    unsigned char dest4[100] = {0};
    size_t msg4_len = strlen(msg4);
    size_t n4 = 1000;
    if (n4 > msg4_len) n4 = msg4_len;
    size_t c4 = copy_bytes(dest4, (const unsigned char*)msg4, n4);
    fwrite(dest4, 1, c4, stdout);
    fputc('\n', stdout);

    // Test 5: Overlapping copy within same buffer
    unsigned char buf[64] = {0};
    const char *s = "OverlapExample";
    size_t len = strlen(s);
    memcpy(buf, s, len);
    size_t safe_n = (len > 2) ? (len - 2) : 0;
    size_t c5 = copy_bytes(buf + 2, buf, safe_n);
    (void)c5;
    fwrite(buf, 1, safe_n + 2, stdout);
    fputc('\n', stdout);

    return 0;
}