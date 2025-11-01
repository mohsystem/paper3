#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FIXED_SIZE 1024

// Allocates a buffer of exactly 1024 bytes; returns NULL on failure
char* allocate_buffer(size_t requested_size) {
    (void)requested_size; // enforce fixed size
    char* buf = (char*)calloc(FIXED_SIZE, 1);
    return buf;
}

// Reads up to 10 characters from the given stream into buf[0..n-1].
// Ensures a null terminator at buf[n]. Returns number of bytes read (0..10),
// or -1 on error (e.g., invalid arguments).
int read_ten_into(FILE* in, char* buf, size_t bufsize) {
    if (in == NULL || buf == NULL || bufsize < 11) {
        return -1;
    }
    size_t i = 0;
    for (; i < 10; ) {
        int ch = fgetc(in);
        if (ch == EOF) {
            break;
        }
        buf[i++] = (char)ch;
    }
    buf[i] = '\0';
    return (int)i;
}

int main(void) {
    for (int t = 1; t <= 5; ++t) {
        printf("Test %d: Please type at least 10 characters and press Enter:\n", t);
        fflush(stdout);

        char* buffer = allocate_buffer(FIXED_SIZE);
        if (buffer == NULL) {
            fprintf(stderr, "Allocation failed.\n");
            return 1;
        }

        int n = read_ten_into(stdin, buffer, FIXED_SIZE);
        if (n < 0) {
            fprintf(stderr, "Read error.\n");
            free(buffer);
            return 1;
        }

        printf("Read %d byte(s): ", n);
        fwrite(buffer, 1, (size_t)n, stdout);
        putchar('\n');

        free(buffer);
    }
    return 0;
}