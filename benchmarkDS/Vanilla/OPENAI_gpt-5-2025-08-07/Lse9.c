#include <stdio.h>
#include <stdlib.h>

const char* get_arg_at(int argc, char* argv[], int index) {
    if (index >= 0 && index < argc) return argv[index];
    return NULL;
}

int main(int argc, char* argv[]) {
    // 5 test cases
    char* t1[] = {"prog", "alpha", "beta", "gamma"};
    char* t2[] = {"prog"};
    char* t3[] = {"app", "x"};
    char* t4[] = {"p", "10", "20", "30"};
    char* t5[] = {"name", "first", "second"};

    struct {
        char** arr;
        int len;
        int idx;
    } tests[5] = {
        {t1, 4, 1},
        {t2, 1, 0},
        {t3, 2, 5},
        {t4, 4, 3},
        {t5, 3, 2}
    };

    for (int i = 0; i < 5; ++i) {
        const char* res = get_arg_at(tests[i].len, tests[i].arr, tests[i].idx);
        if (res) printf("%s\n", res);
        else printf("Index out of range\n");
    }

    // Handle actual command line
    if (argc >= 2) {
        char* endptr = NULL;
        long idx_l = strtol(argv[1], &endptr, 10);
        if (endptr == argv[1] || *endptr != '\0') {
            printf("Invalid index\n");
        } else {
            int idx = (int)idx_l;
            const char* res = get_arg_at(argc, argv, idx);
            if (res) printf("%s\n", res);
            else printf("Index out of range\n");
        }
    }
    return 0;
}