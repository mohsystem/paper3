#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <ctype.h>

const char* value_at_index(int argc, const char* argv[], int* errcode) {
    if (errcode != NULL) {
        *errcode = 0;
    }
    if (argv == NULL || argc < 2) {
        if (errcode) *errcode = 1; /* insufficient arguments */
        return NULL;
    }

    const char* idxStr = argv[1];
    if (idxStr == NULL) {
        if (errcode) *errcode = 2; /* invalid index */
        return NULL;
    }

    size_t len = strlen(idxStr);
    if (len == 0 || len > 10) {
        if (errcode) *errcode = 2; /* invalid index */
        return NULL;
    }
    for (size_t i = 0; i < len; ++i) {
        if (!isdigit((unsigned char)idxStr[i])) {
            if (errcode) *errcode = 2; /* invalid index */
            return NULL;
        }
    }

    errno = 0;
    char* endptr = NULL;
    unsigned long idx = strtoul(idxStr, &endptr, 10);
    if (errno != 0 || endptr == NULL || *endptr != '\0' || idx > (unsigned long)INT_MAX) {
        if (errcode) *errcode = 2; /* invalid index */
        return NULL;
    }

    int i = (int)idx;
    if (i < 0 || i >= argc) {
        if (errcode) *errcode = 3; /* index out of range */
        return NULL;
    }

    return argv[i];
}

static void run_test(const char* label, int argc, const char* argv[]) {
    int err = 0;
    const char* res = value_at_index(argc, argv, &err);
    if (res != NULL) {
        printf("%s -> %s\n", label, res);
    } else {
        if (err == 1) {
            printf("%s -> error: insufficient arguments\n", label);
        } else if (err == 2) {
            printf("%s -> error: invalid index\n", label);
        } else if (err == 3) {
            printf("%s -> error: index out of range\n", label);
        } else {
            printf("%s -> error: unknown\n", label);
        }
    }
}

int main(void) {
    const char* t1[] = {"prog", "0"};
    const char* t2[] = {"app", "2", "x", "y", "z"};
    const char* t3[] = {"cmd", "4", "A", "B", "C", "D", "E"};
    const char* t4[] = {"tool", "-1", "val"};
    const char* t5[] = {"t", "10", "a", "b"};

    run_test("Test 1", 2, t1);
    run_test("Test 2", 5, t2);
    run_test("Test 3", 7, t3);
    run_test("Test 4", 3, t4);
    run_test("Test 5", 4, t5);

    return 0;
}