#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef struct {
    unsigned char* buffer;
    size_t bytes_copied;
} CopyResult;

static size_t utf8_prefix_len_for_n_chars(const unsigned char* s, size_t len, size_t max_chars) {
    size_t i = 0;
    size_t chars = 0;
    while (i < len && chars < max_chars) {
        unsigned char c = s[i];
        size_t clen = 0;
        if ((c & 0x80u) == 0x00u) {
            clen = 1;
        } else if ((c & 0xE0u) == 0xC0u) {
            if (i + 1 >= len) break;
            if ((s[i+1] & 0xC0u) != 0x80u) break;
            clen = 2;
        } else if ((c & 0xF0u) == 0xE0u) {
            if (i + 2 >= len) break;
            if ((s[i+1] & 0xC0u) != 0x80u || (s[i+2] & 0xC0u) != 0x80u) break;
            clen = 3;
        } else if ((c & 0xF8u) == 0xF0u) {
            if (i + 3 >= len) break;
            if ((s[i+1] & 0xC0u) != 0x80u || (s[i+2] & 0xC0u) != 0x80u || (s[i+3] & 0xC0u) != 0x80u) break;
            clen = 4;
        } else {
            break;
        }
        if (i + clen > len) break;
        i += clen;
        chars++;
    }
    return i;
}

static CopyResult alloc_buf(const char* input) {
    CopyResult res;
    res.buffer = (unsigned char*)calloc(1024, 1);
    res.bytes_copied = 0;
    if (!res.buffer) {
        return res;
    }
    if (!input) input = "";
    size_t inlen = strlen(input);
    size_t byteCount = utf8_prefix_len_for_n_chars((const unsigned char*)input, inlen, 100);
    if (byteCount > 1024) byteCount = 1024;
    if (byteCount > 0) {
        memmove(res.buffer, input, byteCount);
    }
    res.bytes_copied = byteCount;
    return res;
}

static CopyResult alloc_buf1(const char* input) {
    CopyResult res;
    res.buffer = (unsigned char*)calloc(1024, 1);
    res.bytes_copied = 0;
    if (!res.buffer) {
        return res;
    }
    if (!input) input = "";
    size_t inlen = strlen(input);
    size_t byteCount = utf8_prefix_len_for_n_chars((const unsigned char*)input, inlen, 100);
    if (byteCount > 1024) byteCount = 1024;
    if (byteCount > 0) {
        memmove(res.buffer, input, byteCount);
    }
    res.bytes_copied = byteCount;
    return res;
}

static char* repeat_char(char c, size_t n) {
    char* s = (char*)malloc(n + 1);
    if (!s) return NULL;
    for (size_t i = 0; i < n; ++i) s[i] = c;
    s[n] = '\0';
    return s;
}

static char* repeat_string(const char* s, size_t times) {
    size_t len = strlen(s);
    size_t total = len * times;
    char* out = (char*)malloc(total + 1);
    if (!out) return NULL;
    char* p = out;
    for (size_t i = 0; i < times; ++i) {
        memcpy(p, s, len);
        p += len;
    }
    out[total] = '\0';
    return out;
}

static void print_preview(const unsigned char* buf, size_t maxn) {
    size_t i = 0;
    for (; i < maxn && i < 1024 && buf[i] != 0; ++i) {
        unsigned char c = buf[i];
        if (c >= 32 && c <= 126) {
            putchar((int)c);
        } else {
            putchar('.');
        }
    }
    putchar('\n');
}

int main(void) {
    const char* t1 = "Hello, world!";
    char* t2 = repeat_char('A', 100);
    char* t3 = repeat_char('B', 150);
    const char* t4 = "";
    char* t5 = repeat_string("The quick brown fox jumps over the lazy dog 0123456789 Sphinx of black quartz, judge my vow. ", 3);

    const char* tests[5];
    tests[0] = t1;
    tests[1] = t2 ? t2 : "";
    tests[2] = t3 ? t3 : "";
    tests[3] = t4;
    tests[4] = t5 ? t5 : "";

    for (int i = 0; i < 5; ++i) {
        CopyResult r1 = alloc_buf(tests[i]);
        CopyResult r2 = alloc_buf1(tests[i]);
        printf("Test %d: bytes1=%zu, bytes2=%zu\n", i + 1, r1.bytes_copied, r2.bytes_copied);
        printf("Preview1: ");
        print_preview(r1.buffer, 60);
        printf("Preview2: ");
        print_preview(r2.buffer, 60);
        free(r1.buffer);
        free(r2.buffer);
    }

    free(t2);
    free(t3);
    free(t5);
    return 0;
}