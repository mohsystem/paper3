#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdbool.h>

/*
Chain-of-Through:
1) Understand: Safely place user input into a fixed-size buffer.
2) Security: Prevent buffer overflows, handle NULL inputs, zero-size buffers, and allocation failures.
3) Implementation: Allocate buffer_size + 1 to ensure room for NUL; copy min(length, capacity) bytes.
4) Review: No unsafe functions like gets/strcpy; validated sizes; proper NUL-termination; free after use.
5) Output: Final safe function.
*/

char* handle_input_to_fixed_buffer(const char* input, size_t buffer_size) {
    // Treat buffer_size as capacity (number of storable characters).
    // Allocate buffer_size + 1 to ensure space for NUL terminator.
    size_t alloc_size = buffer_size + 1;
    char* buffer = (char*)malloc(alloc_size);
    if (buffer == NULL) {
        return NULL; // Allocation failure
    }

    if (buffer_size == 0) {
        buffer[0] = '\0';
        return buffer;
    }

    // Treat NULL input as empty string.
    if (input == NULL) {
        buffer[0] = '\0';
        return buffer;
    }

    // Compute input length safely.
    size_t input_len = 0;
    while (input[input_len] != '\0') {
        input_len++;
    }

    size_t to_copy = (input_len < buffer_size) ? input_len : buffer_size;
    if (to_copy > 0) {
        memcpy(buffer, input, to_copy);
    }
    buffer[to_copy] = '\0';
    return buffer;
}

int main(void) {
    const char* inputs[5] = {"Hello", "HelloWorld", "", "SafeBuffer123", "EdgeCase"};
    size_t sizes[5] = {10, 5, 8, 12, 0};

    for (int i = 0; i < 5; ++i) {
        char* result = handle_input_to_fixed_buffer(inputs[i], sizes[i]);
        if (result == NULL) {
            printf("Test %d: Allocation failed\n", i + 1);
            continue;
        }
        printf("Test %d: input=\"%s\", size=%zu -> \"%s\"\n", i + 1, inputs[i], sizes[i], result);
        free(result);
    }
    return 0;
}