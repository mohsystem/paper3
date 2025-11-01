/* Chain-of-Through process in code generation:
 * 1) Problem understanding: Function returns value at index from id_sequence; plus a prompt-based variant.
 * 2) Security requirements: Validate indices and inputs; avoid buffer overflows and undefined behavior.
 * 3) Secure coding generation: Bounds checks, use fgets + strtol for safe parsing.
 * 4) Code review: Ensure no out-of-bounds access; check all return values and ranges.
 * 5) Secure code output: Return -1 on invalid or error conditions.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

int getValueAtIndex(const int* id_sequence, size_t n, int index) {
    if (id_sequence == NULL) {
        return -1;
    }
    if (index < 0 || (size_t)index >= n) {
        return -1;
    }
    return id_sequence[(size_t)index];
}

int promptAndGetValue(const int* id_sequence, size_t n) {
    char buf[128];
    fputs("Enter index: ", stdout);
    fflush(stdout);
    if (fgets(buf, sizeof(buf), stdin) == NULL) {
        return -1;
    }
    // Trim leading/trailing whitespace
    size_t len = strlen(buf);
    while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r' || buf[len - 1] == ' ' || buf[len - 1] == '\t')) {
        buf[--len] = '\0';
    }
    char* p = buf;
    while (*p == ' ' || *p == '\t') p++;
    if (*p == '\0') {
        return -1;
    }
    char* endptr = NULL;
    long val = strtol(p, &endptr, 10);
    if (endptr == p || *endptr != '\0') {
        return -1;
    }
    if (val < INT_MIN || val > INT_MAX) {
        return -1;
    }
    return getValueAtIndex(id_sequence, n, (int)val);
}

int main(void) {
    int id_sequence[] = {10, 20, 30, 40, 50};
    size_t n = sizeof(id_sequence) / sizeof(id_sequence[0]);

    int testIndices[5] = {0, 2, 4, 1, 3};
    for (int i = 0; i < 5; ++i) {
        int idx = testIndices[i];
        int result = getValueAtIndex(id_sequence, n, idx);
        printf("Index %d -> %d\n", idx, result);
    }

    // Uncomment to test interactive prompt:
    // int userResult = promptAndGetValue(id_sequence, n);
    // printf("User-selected value -> %d\n", userResult);

    return 0;
}