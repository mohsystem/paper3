#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STACK_BUFFER (64 * 1024) /* 64 KiB safety limit */

char* create_filled_stack_buffer(size_t buffer_size) {
    if (buffer_size > MAX_STACK_BUFFER) {
        return NULL; /* Exceeds safe stack limit */
    }
    if (buffer_size == 0) {
        char* out = (char*)malloc(1);
        if (out != NULL) {
            out[0] = '\0';
        }
        return out;
    }

    /* C99 VLA on stack */
    char stack_buf[buffer_size];
    memset(stack_buf, 'A', buffer_size);

    char* out = (char*)malloc(buffer_size + 1);
    if (out == NULL) {
        return NULL;
    }
    memcpy(out, stack_buf, buffer_size);
    out[buffer_size] = '\0';
    return out;
}

static void print_sample(const char* s) {
    if (s == NULL) {
        printf("NULL\n");
        return;
    }
    size_t len = strlen(s);
    printf("len=%zu sample=\"", len);
    size_t show = len < 20 ? len : 20;
    for (size_t i = 0; i < show; ++i) {
        putchar(s[i]);
    }
    if (len > show) {
        printf("...");
    }
    printf("\"\n");
}

int main(void) {
    /* 5 test cases */
    size_t tests[5] = {0, 1, 5, 64, 1024};
    for (size_t i = 0; i < 5; ++i) {
        char* result = create_filled_stack_buffer(tests[i]);
        if (tests[i] > MAX_STACK_BUFFER) {
            printf("Requested size %zu exceeds safe stack limit\n", tests[i]);
        }
        if (result == NULL && tests[i] <= MAX_STACK_BUFFER) {
            printf("Allocation failed for size %zu\n", tests[i]);
        } else if (result != NULL) {
            print_sample(result);
            free(result);
        }
    }
    return 0;
}