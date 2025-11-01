#include <stdio.h>
#include <stddef.h>

void* copy_bytes(void* dest, const void* src, size_t n) {
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    for (size_t i = 0; i < n; ++i) {
        d[i] = s[i];
    }
    return dest;
}

int main() {
    const char src[] = "Hello";

    char dest1[10] = {0};
    copy_bytes(dest1, src, 5);
    dest1[5] = '\0';
    printf("Test1: %s\n", dest1);

    char dest2[5] = {'x','x','x','x','x'};
    copy_bytes(dest2, src, 5);
    char out2[6] = {0};
    copy_bytes(out2, dest2, 5);
    out2[5] = '\0';
    printf("Test2: %s\n", out2);

    char dest3[6] = {0};
    copy_bytes(dest3, src, 5);
    dest3[5] = '\0';
    printf("Test3: %s\n", dest3);

    char dest4[100] = {0};
    copy_bytes(dest4, src, 5);
    dest4[5] = '\0';
    printf("Test4: %s\n", dest4);

    char dest5[5];
    copy_bytes(dest5, src, 5);
    char out5[6] = {0};
    copy_bytes(out5, dest5, 5);
    out5[5] = '\0';
    printf("Test5: %s\n", out5);

    return 0;
}