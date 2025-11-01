#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define MAX_SIZE (1024UL * 1024UL) // 1 MiB

// Allocates two chunks, copies the first into the second, returns the second.
// On error (invalid input, oversized, allocation failure), returns NULL.
// Caller must free the returned pointer.
char* copy_chunk(const char* input, size_t len) {
    if (len > MAX_SIZE) {
        return NULL;
    }
    if (len > 0 && input == NULL) {
        return NULL;
    }
    if (len > SIZE_MAX - 1) { // ensure space for NUL terminator
        return NULL;
    }

    // Allocate first chunk and copy input
    char* first = (char*)malloc(len + 1);
    if (!first) {
        return NULL;
    }
    if (len > 0) {
        memcpy(first, input, len);
    }
    first[len] = '\0';

    // Allocate second chunk and copy first
    char* second = (char*)malloc(len + 1);
    if (!second) {
        free(first);
        return NULL;
    }
    if (len > 0) {
        memcpy(second, first, len);
    }
    second[len] = '\0';

    free(first);
    return second;
}

static void run_test(const char* input, size_t len, const char* label) {
    printf("Test: %s\n", label);
    char* out = copy_chunk(input, len);
    if (!out) {
        printf("[ERROR] Copy failed or input too large.\n");
    } else {
        printf("%s\n", out);
        free(out);
    }
    printf("----\n");
}

int main(void) {
    // Test 1: Empty
    const char* s1 = "";
    run_test(s1, strlen(s1), "Empty");

    // Test 2: Simple text
    const char* s2 = "Hello, world!";
    run_test(s2, strlen(s2), "Simple text");

    // Test 3: Unicode text (UTF-8)
    const char* s3 = "Unicode test: 你好, мир, مرحبا";
    run_test(s3, strlen(s3), "Unicode text");

    // Test 4: Large but within limit
    size_t len4 = 100000;
    char* buf4 = (char*)malloc(len4 + 1);
    if (!buf4) {
        fprintf(stderr, "Allocation failure in test 4.\n");
        return 1;
    }
    memset(buf4, 'A', len4);
    buf4[len4] = '\0';
    run_test(buf4, len4, "Large within limit");
    free(buf4);

    // Test 5: Oversized (exceeds MAX_SIZE)
    size_t len5 = MAX_SIZE + 1;
    char* buf5 = (char*)malloc(len5);
    if (!buf5) {
        // Even if allocation fails, the test intent is to show rejection by size.
        // Call with NULL and len5 to trigger size rejection path safely.
        run_test(NULL, len5, "Oversized");
    } else {
        memset(buf5, 'B', len5);
        run_test(buf5, len5, "Oversized");
        free(buf5);
    }

    return 0;
}