#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define MAX_SIZE (1u << 20) /* 1 MiB cap */

static void secure_zero(void *v, size_t n) {
    volatile unsigned char *p = (volatile unsigned char *)v;
    while (n--) {
        *p++ = 0;
    }
}

/*
 Allocates two chunks, copies first to second, prints second, frees first,
 prints second again, then frees second.
 Returns a newly allocated copy of the second chunk content captured before freeing it.
 The caller must free the returned pointer.
*/
char* demo_copy(const char* input) {
    if (input == NULL) {
        printf("Invalid input\n");
        return NULL;
    }

    size_t len = strnlen(input, MAX_SIZE + 1);
    if (len > MAX_SIZE) {
        printf("Input too large\n");
        return NULL;
    }

    /* Allocate two chunks (+1 for null terminator) */
    char *chunk1 = (char*)malloc(len + 1);
    char *chunk2 = (char*)malloc(len + 1);
    if (chunk1 == NULL || chunk2 == NULL) {
        printf("Allocation error\n");
        free(chunk1);
        free(chunk2);
        return NULL;
    }

    /* Copy input -> chunk1 safely */
    if (len > 0) {
        memcpy(chunk1, input, len);
    }
    chunk1[len] = '\0';

    /* Copy chunk1 -> chunk2 safely */
    if (len > 0) {
        memcpy(chunk2, chunk1, len);
    }
    chunk2[len] = '\0';

    /* Print second chunk */
    printf("%s\n", chunk2);

    /* Free first chunk: zeroize then free */
    secure_zero(chunk1, len + 1);
    free(chunk1);
    chunk1 = NULL;

    /* Print second chunk again (should remain unchanged) */
    printf("%s\n", chunk2);

    /* Capture return value before freeing second */
    char *ret = (char*)malloc(len + 1);
    if (ret == NULL) {
        printf("Allocation error\n");
        secure_zero(chunk2, len + 1);
        free(chunk2);
        return NULL;
    }
    if (len > 0) {
        memcpy(ret, chunk2, len);
    }
    ret[len] = '\0';

    /* Free second chunk: zeroize then free */
    secure_zero(chunk2, len + 1);
    free(chunk2);
    chunk2 = NULL;

    return ret;
}

int main(void) {
    const char* tests[5] = {
        "Hello, world!",
        "",
        "A longer string with multiple words.",
        "Unicode: 😊🚀✨",
        "Line1\nLine2\tTabbed"
    };

    for (int i = 0; i < 5; ++i) {
        char *result = demo_copy(tests[i]);
        if (result != NULL) {
            printf("Returned: %s\n", result);
            secure_zero(result, strlen(result) + 1);
            free(result);
        } else {
            printf("Returned: (null)\n");
        }
        printf("---\n");
    }
    return 0;
}