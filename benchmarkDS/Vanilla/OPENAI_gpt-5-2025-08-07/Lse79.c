#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* alloc_and_copy_100(const char* input) {
    char* buf = (char*)malloc(1024);
    if (!buf) return NULL;
    size_t inputLen = input ? strlen(input) : 0;
    size_t len = inputLen;
    if (len > 100) len = 100;
    if (len > 1023) len = 1023;
    if (len > 0 && input) memcpy(buf, input, len);
    buf[len] = '\0';
    char* out = (char*)malloc(len + 1);
    if (!out) {
        free(buf);
        return NULL;
    }
    memcpy(out, buf, len + 1);
    free(buf);
    return out;
}

char* alloc_and_copy_100_second(const char* input) {
    char* buf1 = (char*)malloc(1024);
    if (!buf1) return NULL;
    size_t inputLen = input ? strlen(input) : 0;
    size_t len = inputLen;
    if (len > 100) len = 100;
    if (len > 1023) len = 1023;
    if (len > 0 && input) memcpy(buf1, input, len);
    buf1[len] = '\0';
    char* out = (char*)malloc(len + 1);
    if (!out) {
        free(buf1);
        return NULL;
    }
    memcpy(out, buf1, len + 1);
    free(buf1);
    return out;
}

char* read_up_to_1024(const char* input) {
    char* buf = (char*)malloc(1024);
    if (!buf) return NULL;
    size_t inputLen = input ? strlen(input) : 0;
    size_t len = inputLen > 1024 ? 1024 : inputLen;
    if (len > 0 && input) memcpy(buf, input, len);
    size_t safeLen = len > 1023 ? 1023 : len;
    buf[safeLen] = '\0';
    char* out = (char*)malloc(len + 1);
    if (!out) {
        free(buf);
        return NULL;
    }
    memcpy(out, buf, safeLen + 1);
    if (len > safeLen) {
        // copy remaining bytes if any (without extra nulls)
        memcpy(out + safeLen + 1, buf + safeLen + 1, len - safeLen - 1);
        out[len] = '\0';
    }
    free(buf);
    return out;
}

int main() {
    const char* s1 = "Hello World";
    char s2[101]; memset(s2, 'A', 100); s2[100] = '\0';
    char s3[151]; memset(s3, 'B', 150); s3[150] = '\0';
    const char* s4 = "";
    // Build s5: "Unicode 😃🚀 " + 1100 'C'
    char *s5_prefix = "Unicode 😃🚀 ";
    size_t prefix_len = strlen(s5_prefix);
    char *s5 = (char*)malloc(prefix_len + 1100 + 1);
    memcpy(s5, s5_prefix, prefix_len);
    memset(s5 + prefix_len, 'C', 1100);
    s5[prefix_len + 1100] = '\0';

    const char* tests[5] = { s1, s2, s3, s4, s5 };

    for (int i = 0; i < 5; ++i) {
        char* r1 = alloc_and_copy_100(tests[i]);
        char* r2 = alloc_and_copy_100_second(tests[i]);
        char* r3 = read_up_to_1024(tests[i]);

        printf("Test %d\n", i + 1);
        printf("alloc_and_copy_100: %s\n", r1 ? r1 : "(null)");
        printf("alloc_and_copy_100_second: %s\n", r2 ? r2 : "(null)");
        if (r3) {
            if (strlen(r3) > 80) {
                char tmp = r3[80];
                r3[80] = '\0';
                printf("read_up_to_1024: %s...(truncated print)\n", r3);
                r3[80] = tmp;
            } else {
                printf("read_up_to_1024: %s\n", r3);
            }
        } else {
            printf("read_up_to_1024: (null)\n");
        }
        printf("---\n");

        free(r1);
        free(r2);
        free(r3);
    }

    free(s5);
    return 0;
}