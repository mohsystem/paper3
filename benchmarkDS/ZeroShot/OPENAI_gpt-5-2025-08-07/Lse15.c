#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

static const int ID_SEQUENCE[] = {101, 203, 307, 409, 512, 618, 723, 834, 945, 1056};
static const size_t ID_SEQUENCE_LEN = sizeof(ID_SEQUENCE) / sizeof(ID_SEQUENCE[0]);

typedef struct {
    int has_value; /* 1 if value is valid, 0 otherwise */
    int value;
} Result;

/* Function: takes index and returns the value at that index via a Result struct */
Result get_value_at_index(long long index) {
    Result r;
    if (index < 0 || (unsigned long long)index >= ID_SEQUENCE_LEN) {
        r.has_value = 0;
        r.value = 0;
        return r;
    }
    r.has_value = 1;
    r.value = ID_SEQUENCE[(size_t)index];
    return r;
}

static void prompt_once(void) {
    char buf[128];
    printf("Enter an index (0-%zu): ", ID_SEQUENCE_LEN - 1);
    if (!fgets(buf, sizeof(buf), stdin)) {
        printf("No input provided.\n");
        return;
    }
    /* Trim newline */
    buf[strcspn(buf, "\r\n")] = '\0';
    /* Skip leading spaces */
    char *p = buf;
    while (*p == ' ' || *p == '\t') p++;
    if (*p == '\0') {
        printf("Empty input.\n");
        return;
    }
    char *end = NULL;
    /* Use strtoll for robust parsing */
    long long val = strtoll(p, &end, 10);
    if (p == end) {
        printf("Invalid number format.\n");
        return;
    }
    /* Check for trailing non-space characters */
    while (*end == ' ' || *end == '\t') end++;
    if (*end != '\0') {
        printf("Invalid number format.\n");
        return;
    }
    Result res = get_value_at_index(val);
    if (res.has_value) {
        printf("Value at index %lld is: %d\n", val, res.value);
    } else {
        printf("Error: Index out of bounds.\n");
    }
}

int main(void) {
    /* Prompt the user for an index */
    prompt_once();

    /* Five test cases */
    long long tests[5] = {0, 3, 9, 10, 100};
    printf("Running 5 test cases:\n");
    for (size_t i = 0; i < 5; i++) {
        Result r = get_value_at_index(tests[i]);
        if (r.has_value) {
            printf("Index %lld -> %d\n", tests[i], r.value);
        } else {
            printf("Index %lld -> Error: Index out of bounds.\n", tests[i]);
        }
    }
    return 0;
}