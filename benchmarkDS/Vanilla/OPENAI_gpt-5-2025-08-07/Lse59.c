#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Allocates a buffer of size buffer_size on the stack (via VLA), fills with 'A',
// then returns a heap-allocated copy for the caller to use/freeze outside the function.
// Caller must free the returned pointer.
char* fill_buffer(size_t buffer_size) {
    size_t n = buffer_size;
    // VLA allocates on the stack (C99+)
    char buf[n + 1];
    if (n > 0) {
        memset(buf, 'A', n);
    }
    buf[n] = '\0';

    char* out = (char*)malloc(n + 1);
    if (!out) return NULL;
    memcpy(out, buf, n + 1);
    return out;
}

int main(void) {
    size_t tests[5] = {0, 1, 5, 10, 100};
    for (int i = 0; i < 5; ++i) {
        char* s = fill_buffer(tests[i]);
        if (!s) {
            printf("Allocation failed for size=%zu\n", tests[i]);
        } else {
            printf("size=%zu -> \"%s\"\n", tests[i], s);
            free(s);
        }
    }
    return 0;
}