/* Chain-of-Through Step 1: Problem understanding
   Purpose: Concatenate multiple C strings provided as an array.
   Inputs: const char* parts[] with count elements; elements may be NULL.
   Output: Newly allocated concatenated string (caller must free), or NULL on error. */

/* Chain-of-Through Step 2: Security requirements
   - Handle NULL array or NULL elements safely (treat NULL elements as empty).
   - Check for size_t overflow during length computation.
   - Allocate exact required memory and ensure NUL-termination.
   - Return NULL on allocation failure. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

static int safe_add_size_t(size_t a, size_t b, size_t* out) {
    if (SIZE_MAX - a < b) return 0;
    *out = a + b;
    return 1;
}

/* Chain-of-Through Step 3: Secure coding generation */
char* concatenate_strings(const char* parts[], size_t count) {
    if (parts == NULL || count == 0) {
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty; /* Return empty string on no input */
    }

    size_t total = 0;
    for (size_t i = 0; i < count; ++i) {
        size_t len = 0;
        if (parts[i] != NULL) {
            len = strlen(parts[i]);
        }
        if (!safe_add_size_t(total, len, &total)) {
            return NULL; /* overflow */
        }
    }

    /* Add 1 for NUL terminator */
    size_t alloc_size = 0;
    if (!safe_add_size_t(total, 1, &alloc_size)) {
        return NULL; /* overflow */
    }

    char* out = (char*)malloc(alloc_size);
    if (!out) return NULL;

    char* p = out;
    for (size_t i = 0; i < count; ++i) {
        if (parts[i] != NULL) {
            size_t len = strlen(parts[i]);
            if (len > 0) {
                memcpy(p, parts[i], len);
                p += len;
            }
        }
    }
    *p = '\0';
    return out;
}

/* Chain-of-Through Step 4: Code review (comments)
   - Overflow-checked size calculations.
   - NULL-safe handling of elements.
   - Proper allocation and NUL-termination.
   - Caller frees returned buffer. */

/* Chain-of-Through Step 5: Secure code output (final) */
int main(void) {
    /* 5 test cases */
    const char* t1[] = {"Hello", " ", "World", "!"};
    const char* t2[] = {NULL, "Secure", NULL, "Concat"};
    const char* t3[] = {};
    const char* t4[] = {"", "", "non-empty", "", "end"};
    const char* t5[] = {"Unicode: ", "😀", " ", "文字", " ", "✓"};

    const char** tests[] = {t1, t2, t3, t4, t5};
    size_t counts[] = {4, 4, 0, 5, 5};

    for (size_t i = 0; i < 5; ++i) {
        char* r = concatenate_strings(tests[i], counts[i]);
        if (r) {
            printf("Test %zu: %s\n", i + 1, r);
            free(r);
        } else {
            printf("Test %zu: <allocation or overflow error>\n", i + 1);
        }
    }
    return 0;
}