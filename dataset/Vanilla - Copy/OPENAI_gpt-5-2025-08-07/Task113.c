#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* concat_strings(const char* parts[], size_t n) {
    size_t total = 0;
    for (size_t i = 0; i < n; ++i) {
        if (parts[i]) total += strlen(parts[i]);
    }
    char* result = (char*)malloc(total + 1);
    if (!result) return NULL;
    char* p = result;
    for (size_t i = 0; i < n; ++i) {
        if (parts[i]) {
            size_t len = strlen(parts[i]);
            memcpy(p, parts[i], len);
            p += len;
        }
    }
    *p = '\0';
    return result;
}

int main() {
    const char* t1[] = {"Hello", " ", "World"};
    const char* t2[] = {"", "abc", "", "123"};
    const char* t3[] = {"Join", "-", "these", "-", "together"};
    const char* t4[] = {"multi", "", "ple", " ", "strings"};
    const char* t5[] = {"Unicode:", " ", "😊", " ", "测试"};

    char* r1 = concat_strings(t1, sizeof(t1)/sizeof(t1[0]));
    char* r2 = concat_strings(t2, sizeof(t2)/sizeof(t2[0]));
    char* r3 = concat_strings(t3, sizeof(t3)/sizeof(t3[0]));
    char* r4 = concat_strings(t4, sizeof(t4)/sizeof(t4[0]));
    char* r5 = concat_strings(t5, sizeof(t5)/sizeof(t5[0]));

    if (r1) { printf("%s\n", r1); free(r1); }
    if (r2) { printf("%s\n", r2); free(r2); }
    if (r3) { printf("%s\n", r3); free(r3); }
    if (r4) { printf("%s\n", r4); free(r4); }
    if (r5) { printf("%s\n", r5); free(r5); }

    return 0;
}