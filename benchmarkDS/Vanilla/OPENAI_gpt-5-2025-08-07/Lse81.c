#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* allocate_buffer(void) {
    char* buf = (char*)malloc(1024);
    return buf;
}

size_t read_10_chars_into_buf(char* buf) {
    return fread(buf, 1, 10, stdin);
}

int main(void) {
    const char* tests[5] = {
        "abcdefghij",
        "1234567890",
        "short",
        "tenchars!!",
        "with newline\nmore"
    };

    for (int i = 0; i < 5; ++i) {
        char filename[64];
        snprintf(filename, sizeof(filename), "tmp_input_%d.txt", i + 1);
        FILE* f = fopen(filename, "wb");
        if (!f) return 1;
        fwrite(tests[i], 1, strlen(tests[i]), f);
        fclose(f);

        if (!freopen(filename, "rb", stdin)) return 1;

        char* buf = allocate_buffer();
        if (!buf) return 1;

        size_t n = read_10_chars_into_buf(buf);

        printf("Test%d: ", i + 1);
        fwrite(buf, 1, n, stdout);
        printf("\n");

        free(buf);
        remove(filename);
    }

    return 0;
}