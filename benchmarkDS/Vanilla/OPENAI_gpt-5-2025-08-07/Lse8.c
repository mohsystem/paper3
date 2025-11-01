#include <stdio.h>
#include <stdlib.h>

const char* valueAtIndexFromArg1(int argc, const char* argv[]) {
    if (argc < 2) return NULL;
    char* endptr = NULL;
    long idx = strtol(argv[1], &endptr, 10);
    if (endptr == argv[1] || *endptr != '\0') return NULL;
    if (idx < 0 || idx >= argc) return NULL;
    return argv[idx];
}

int main(void) {
    const char* t1[] = {"prog", "2", "a", "b", "c"};
    const char* t2[] = {"prog", "0", "X"};
    const char* t3[] = {"prog", "4", "w", "x", "y", "z"};
    const char* t4[] = {"prog", "1", "hello"};
    const char* t5[] = {"prog", "3", "p", "q", "r", "s"};

    const char** tests[] = {t1, t2, t3, t4, t5};
    int sizes[] = {5, 3, 6, 3, 6};

    for (int i = 0; i < 5; ++i) {
        const char* res = valueAtIndexFromArg1(sizes[i], tests[i]);
        if (res) {
            printf("%s\n", res);
        } else {
            printf("\n");
        }
    }
    return 0;
}