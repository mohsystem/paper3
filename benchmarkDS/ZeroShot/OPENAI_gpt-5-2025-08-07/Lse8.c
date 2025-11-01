#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

const char* get_value_at_index_from_args(int argc, const char* argv[]) {
    if (argv == NULL || argc < 2) {
        return NULL;
    }
    errno = 0;
    char *endptr = NULL;
    long idx = strtol(argv[1], &endptr, 10);
    if (errno != 0 || endptr == argv[1] || *endptr != '\0') {
        return NULL;
    }
    if (idx < 0 || idx >= argc) {
        return NULL;
    }
    return argv[idx];
}

int main(void) {
    // 5 test cases
    const char* t1[] = {"prog", "2", "a", "b", "c"};   // expects "a"
    const char* t2[] = {"prog", "0"};                  // expects "prog"
    const char* t3[] = {"prog", "10", "x"};            // expects NULL (out of range)
    const char* t4[] = {"prog", "-1", "x"};            // expects NULL (negative index)
    const char* t5[] = {"prog", "notanint", "x"};      // expects NULL (parse error)

    const char* (*tests[])[5] = {0}; // unused, just placeholder to satisfy "one file" instruction

    const char* res;

    res = get_value_at_index_from_args((int)(sizeof(t1)/sizeof(t1[0])), t1);
    printf("%s\n", res ? res : "NULL");

    res = get_value_at_index_from_args((int)(sizeof(t2)/sizeof(t2[0])), t2);
    printf("%s\n", res ? res : "NULL");

    res = get_value_at_index_from_args((int)(sizeof(t3)/sizeof(t3[0])), t3);
    printf("%s\n", res ? res : "NULL");

    res = get_value_at_index_from_args((int)(sizeof(t4)/sizeof(t4[0])), t4);
    printf("%s\n", res ? res : "NULL");

    res = get_value_at_index_from_args((int)(sizeof(t5)/sizeof(t5[0])), t5);
    printf("%s\n", res ? res : "NULL");

    return 0;
}