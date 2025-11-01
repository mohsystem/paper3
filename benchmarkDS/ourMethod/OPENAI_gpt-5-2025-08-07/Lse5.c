#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

static void create_elements(const char* arr[], size_t* len) {
    static const char* local[] = { "alpha", "bravo", "charlie", "delta", "echo", "foxtrot" };
    for (size_t i = 0; i < sizeof(local)/sizeof(local[0]); ++i) {
        arr[i] = local[i];
    }
    *len = sizeof(local)/sizeof(local[0]);
}

/*
 * Returns 1 on success and sets *out to point to the string in arr.
 * Returns 0 on invalid index.
 */
static int get_element_at(const char* const arr[], size_t len, long long idx, const char** out) {
    if (out == NULL) return 0;
    *out = NULL;
    if (idx < 0) return 0;
    if ((unsigned long long)idx >= len) return 0;
    *out = arr[(size_t)idx];
    return 1;
}

static int safe_parse_index(const char* s, long long* out) {
    if (s == NULL || out == NULL) return 0;
    // Trim leading/trailing whitespace
    const char* start = s;
    while (*start == ' ' || *start == '\t' || *start == '\n' || *start == '\r' || *start == '\v' || *start == '\f') {
        start++;
    }
    size_t len = strlen(start);
    while (len > 0 && (start[len - 1] == ' ' || start[len - 1] == '\t' || start[len - 1] == '\n' || start[len - 1] == '\r' || start[len - 1] == '\v' || start[len - 1] == '\f')) {
        len--;
    }
    if (len == 0 || len > 32) return 0;

    char buf[64];
    if (len >= sizeof(buf)) return 0;
    memcpy(buf, start, len);
    buf[len] = '\0';

    char* endptr = NULL;
    errno = 0;
    long long val = strtoll(buf, &endptr, 10);
    if (errno != 0 || endptr == buf || *endptr != '\0') {
        return 0;
    }
    *out = val;
    return 1;
}

int main(int argc, char* argv[]) {
    const char* elements[16] = {0};
    size_t len = 0U;
    create_elements(elements, &len);

    if (argc >= 2) {
        long long idx = 0;
        if (!safe_parse_index(argv[1], &idx)) {
            puts("Invalid input");
            return 0;
        }
        const char* value = NULL;
        if (get_element_at(elements, len, idx, &value)) {
            puts(value);
        } else {
            puts("Invalid index");
        }
        return 0;
    }

    // 5 test cases
    long long tests[5] = {0, 2, 5, -1, 6};
    for (size_t i = 0; i < 5; ++i) {
        const char* value = NULL;
        if (get_element_at(elements, len, tests[i], &value)) {
            printf("Index %lld -> %s\n", tests[i], value);
        } else {
            printf("Index %lld -> Invalid index\n", tests[i]);
        }
    }
    return 0;
}