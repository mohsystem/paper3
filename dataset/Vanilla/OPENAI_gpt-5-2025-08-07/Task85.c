#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

static const char ALPHABET[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

char* random_ascii_letters(size_t length) {
    char* out = (char*)malloc(length + 1);
    if (!out) return NULL;

    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) {
        free(out);
        return NULL;
    }

    size_t i = 0;
    while (i < length) {
        unsigned char byte;
        ssize_t r = read(fd, &byte, 1);
        if (r != 1) {
            close(fd);
            free(out);
            return NULL;
        }
        if (byte < 208) { // 208 = 52 * 4 to avoid modulo bias
            out[i++] = ALPHABET[byte % 52];
        }
    }

    out[length] = '\0';
    close(fd);
    return out;
}

int main(void) {
    size_t tests[5] = {0, 1, 8, 16, 32};
    for (int t = 0; t < 5; t++) {
        char* s = random_ascii_letters(tests[t]);
        if (!s) {
            fprintf(stderr, "Failed to generate string for length %zu\n", tests[t]);
            continue;
        }
        printf("%zu: %s\n", tests[t], s);
        free(s);
    }
    return 0;
}