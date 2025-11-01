/*
Chain-of-Through process:
1) Problem understanding:
   - Create an array of elements.
   - Read a command-line argument, convert to integer, print element at that index.
   - Provide a function accepting inputs as parameters and returning the output (if valid).
2) Security requirements:
   - Validate argument presence, safe integer parsing with strtoll, and bounds checks.
3) Secure coding generation:
   - Use strtoll with endptr and errno checks, ensure index within [0, len).
4) Code review:
   - No unchecked array access, errors to stderr, no undefined behavior.
5) Secure code output:
   - Final code applies safe parsing and strict bounds checking.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

static const char* ELEMENTS[] = { "alpha", "beta", "gamma", "delta", "epsilon" };
static const size_t ELEMENTS_LEN = sizeof(ELEMENTS) / sizeof(ELEMENTS[0]);

// Function: accepts inputs as parameters and returns the output if any.
// Returns NULL if invalid index.
const char* element_at_index(const char* elements[], size_t len, long long index) {
    if (elements == NULL) {
        return NULL;
    }
    if (index < 0 || (size_t)index >= len) {
        return NULL;
    }
    return elements[(size_t)index];
}

static void process_argument(const char* arg) {
    if (arg == NULL) {
        fprintf(stderr, "Error: No index argument provided.\n");
        return;
    }
    errno = 0;
    char* endptr = NULL;
    long long idx = strtoll(arg, &endptr, 10);
    if (errno != 0 || endptr == arg || *endptr != '\0') {
        fprintf(stderr, "Error: Index must be a valid integer.\n");
        return;
    }

    const char* res = element_at_index(ELEMENTS, ELEMENTS_LEN, idx);
    if (res == NULL) {
        fprintf(stderr, "Error: Index out of bounds. Valid range is 0 to %zu.\n", ELEMENTS_LEN - 1);
    } else {
        printf("%s\n", res);
    }
}

int main(int argc, char* argv[]) {
    if (argc >= 2) {
        process_argument(argv[1]);
    } else {
        // 5 test cases
        long long tests[5] = { 0, 2, 4, -1, 5 };
        for (int i = 0; i < 5; ++i) {
            const char* res = element_at_index(ELEMENTS, ELEMENTS_LEN, tests[i]);
            if (res == NULL) {
                fprintf(stderr, "Test index %lld: Error - Index out of bounds. Valid range is 0 to %zu.\n", tests[i], ELEMENTS_LEN - 1);
            } else {
                printf("Test index %lld: %s\n", tests[i], res);
            }
        }
    }
    return 0;
}