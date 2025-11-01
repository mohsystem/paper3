#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_MSC_VER)
  #include <malloc.h>
#else
  #include <alloca.h>
#endif

// Step 1-4: Implement secure overwrite with bounds checking
// Returns a heap-allocated null-terminated string containing 'A's of length buffer_size.
// The internal buffer used for overwrite is on the stack. Caller must free the returned pointer.
// On invalid size, returns NULL.
char* overwrite_with_A(size_t buffer_size) {
    const size_t MAX_STACK_BUFFER = 65536; // Security: prevent excessive stack usage
    if (buffer_size > MAX_STACK_BUFFER) {
        return NULL;
    }
    if (buffer_size == 0) {
        char* out = (char*)malloc(1);
        if (out) out[0] = '\0';
        return out;
    }

    // Allocate on stack
    char* buf = (char*)alloca(buffer_size);
    if (!buf) {
        return NULL;
    }
    memset(buf, 'A', buffer_size);

    // Return a safe copy to the caller
    char* out = (char*)malloc(buffer_size + 1);
    if (!out) {
        return NULL;
    }
    memcpy(out, buf, buffer_size);
    out[buffer_size] = '\0';
    return out;
}

int main(void) {
    size_t tests[5] = {0, 1, 5, 16, 70000};
    for (int i = 0; i < 5; ++i) {
        size_t t = tests[i];
        char* result = overwrite_with_A(t);
        if (result) {
            printf("bufferSize=%zu, len=%zu, preview=%.*s\n",
                   t, t, (int)(t < 16 ? t : 16), result);
            free(result);
        } else {
            printf("bufferSize=%zu, result=NULL\n", t);
        }
    }
    return 0;
}