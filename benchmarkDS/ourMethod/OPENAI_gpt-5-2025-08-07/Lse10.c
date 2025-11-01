#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <ctype.h>

int getValueFromArray(const int *arr, size_t size, int index) {
    if (arr == NULL) {
        errno = EINVAL;
        return 0;
    }
    if (size > 0 && (index >= 0) && ((size_t)index) < size) {
        return arr[index];
    }
    errno = EINVAL;
    return 0;
}

static void run_tests(void) {
    const int id_sequence[] = {3, 14, 15, 92, 65, 35, 89, 79};
    const size_t n = sizeof(id_sequence) / sizeof(id_sequence[0]);

    int all_ok = 1;
    int v;

    v = getValueFromArray(id_sequence, n, 0);
    all_ok &= (errno == 0 || errno == EINVAL ? (v == 3) : 0);
    errno = 0;

    v = getValueFromArray(id_sequence, n, 3);
    all_ok &= (errno == 0 || errno == EINVAL ? (v == 92) : 0);
    errno = 0;

    v = getValueFromArray(id_sequence, n, 7);
    all_ok &= (errno == 0 || errno == EINVAL ? (v == 79) : 0);
    errno = 0;

    v = getValueFromArray(id_sequence, n, -1);
    all_ok &= (errno == EINVAL);
    errno = 0;

    v = getValueFromArray(id_sequence, n, 100);
    all_ok &= (errno == EINVAL);
    errno = 0;

    if (!all_ok) {
        fprintf(stderr, "Tests failed\n");
    } else {
        fprintf(stderr, "All tests passed\n");
    }
}

static int safe_parse_int(const char *s, int *out) {
    char *end = NULL;
    long val;

    if (s == NULL || out == NULL) return 0;

    errno = 0;
    val = strtol(s, &end, 10);
    if (errno != 0) return 0;
    if (end == s) return 0;

    while (*end != '\0' && isspace((unsigned char)*end)) end++;
    if (*end != '\0') return 0;

    if (val < INT_MIN || val > INT_MAX) return 0;

    *out = (int)val;
    return 1;
}

int main(int argc, char *argv[]) {
    const int id_sequence[] = {3, 14, 15, 92, 65, 35, 89, 79};
    const size_t n = sizeof(id_sequence) / sizeof(id_sequence[0]);

    if (argc > 1 && strcmp(argv[1], "test") == 0) {
        run_tests();
        return 0;
    }

    char buf[128];
    if (fgets(buf, sizeof(buf), stdin) == NULL) {
        return 1;
    }

    // Trim leading/trailing whitespace
    size_t len = strnlen(buf, sizeof(buf));
    while (len > 0 && isspace((unsigned char)buf[len - 1])) {
        buf[len - 1] = '\0';
        len--;
    }
    char *start = buf;
    while (*start && isspace((unsigned char)*start)) start++;

    if (*start == '\0' || strlen(start) > 100) {
        return 1;
    }

    int idx = 0;
    if (!safe_parse_int(start, &idx)) {
        return 1;
    }

    if (idx < 0 || (size_t)idx >= n) {
        return 1;
    }

    int value = getValueFromArray(id_sequence, n, idx);
    if (errno != 0) {
        return 1;
    }

    return value & 0xFF; // ensure portable exit code
}