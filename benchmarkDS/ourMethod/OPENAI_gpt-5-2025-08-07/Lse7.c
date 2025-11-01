#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

static int string_to_int(const char *s, int *out_value) {
    if (s == NULL || out_value == NULL) return -1;

    // Trim leading/trailing whitespace
    const char *start = s;
    while (*start && isspace((unsigned char)*start)) start++;
    size_t len = strlen(start);
    while (len > 0 && isspace((unsigned char)start[len - 1])) len--;

    if (len == 0) return -1;

    size_t i = 0;
    if (start[0] == '-') return -1; // negative not allowed
    if (start[0] == '+') {
        i = 1;
        if (i >= len) return -1;
    }

    int result = 0;
    for (; i < len; i++) {
        char ch = start[i];
        if (ch < '0' || ch > '9') return -1;
        int digit = ch - '0';
        if (result > (INT_MAX - digit) / 10) return -1; // overflow
        result = result * 10 + digit;
    }
    *out_value = result;
    return 0;
}

// Prints the argument at the INDEX specified by index_str within argv[0..argc-1].
// Returns 1 on success, 0 on failure.
static int print_arg_at_index(const char *argv[], int argc, const char *index_str) {
    if (argv == NULL || argc <= 0 || index_str == NULL) {
        fprintf(stderr, "ERROR\n");
        return 0;
    }
    int idx = 0;
    if (string_to_int(index_str, &idx) != 0) {
        fprintf(stderr, "ERROR\n");
        return 0;
    }
    if (idx < 0 || idx >= argc) {
        fprintf(stderr, "ERROR\n");
        return 0;
    }
    // Use puts to avoid format string issues
    if (argv[idx] == NULL) {
        fprintf(stderr, "ERROR\n");
        return 0;
    }
    puts(argv[idx]);
    return 1;
}

static void run_tests(void) {
    // 5 test cases
    const char *t1[] = {"x", "2", "a", "b", "c", "d"};
    const char *t2[] = {"index", "0", "hello"};
    const char *t3[] = {"index", "3", "p", "q", "r", "s"};
    const char *t4[] = {"cmd", "100", "a", "b"};
    const char *t5[] = {"cmd", "abc", "z"};

    print_arg_at_index(t1, (int)(sizeof(t1) / sizeof(t1[0])), t1[1]); // prints "a"
    print_arg_at_index(t2, (int)(sizeof(t2) / sizeof(t2[0])), t2[1]); // prints "index"
    print_arg_at_index(t3, (int)(sizeof(t3) / sizeof(t3[0])), t3[1]); // prints "s"
    print_arg_at_index(t4, (int)(sizeof(t4) / sizeof(t4[0])), t4[1]); // ERROR
    print_arg_at_index(t5, (int)(sizeof(t5) / sizeof(t5[0])), t5[1]); // ERROR
}

int main(int argc, char *argv[]) {
    // Run 5 test cases
    run_tests();

    // Also process real command line input (excluding program name)
    if (argc > 2) {
        // Construct view excluding program name
        const char **args_only = (const char **)(argv + 1);
        int args_count = argc - 1;
        // index string is the second argument => original argv[2], now args_only[1]
        print_arg_at_index(args_only, args_count, args_only[1]);
    }
    return 0;
}