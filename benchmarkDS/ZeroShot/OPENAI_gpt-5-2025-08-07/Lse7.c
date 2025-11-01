#include <stdio.h>
#include <limits.h>
#include <ctype.h>
#include <string.h>

// Securely tries to parse a non-negative integer from a string. Returns 1 on success, 0 otherwise.
// On success, writes parsed value to *out.
int try_parse_non_negative_int(const char* s, int* out) {
    if (out == NULL || s == NULL) return 0;

    // Trim leading whitespace
    const char* p = s;
    while (*p && isspace((unsigned char)*p)) p++;

    if (*p == '\0') return 0;

    if (*p == '+') {
        p++;
        if (*p == '\0') return 0;
    } else if (*p == '-') {
        return 0;
    }

    long long val = 0;
    int has_digit = 0;
    while (*p && isdigit((unsigned char)*p)) {
        has_digit = 1;
        val = val * 10 + (*p - '0');
        if (val > INT_MAX) return 0;
        p++;
    }
    if (!has_digit) return 0;

    // Allow trailing whitespace only
    while (*p) {
        if (!isspace((unsigned char)*p)) return 0;
        p++;
    }

    *out = (int)val;
    return 1;
}

// Retrieves argument at the index specified by index_str. Returns 1 on success and sets *out_str.
// Returns 0 on failure.
int get_arg_at_index(int argc, const char* argv[], const char* index_str, const char** out_str) {
    if (argv == NULL || out_str == NULL || index_str == NULL) return 0;
    int idx = -1;
    if (!try_parse_non_negative_int(index_str, &idx)) return 0;
    if (idx < 0 || idx >= argc) return 0;
    *out_str = argv[idx];
    return 1;
}

// Prints the argument at the index specified by index_str if valid.
void print_arg_at_index(int argc, const char* argv[], const char* index_str) {
    const char* v = NULL;
    if (get_arg_at_index(argc, argv, index_str, &v)) {
        puts(v);
    }
}

int main(int argc, const char* argv[]) {
    // If run with CLI arguments, use the second argument as index, and print the argument at that index.
    if (argc > 1) {
        print_arg_at_index(argc, argv, argv[1]);
    }

    // 5 test cases
    {
        const char* t1[] = {"a0", "2", "x0", "x1", "x2"};       // prints x0
        print_arg_at_index(5, t1, t1[1]);
    }
    {
        const char* t2[] = {"first", "0", "z"};                 // prints first
        print_arg_at_index(3, t2, t2[1]);
    }
    {
        const char* t3[] = {"foo", "3", "aa", "bb", "cc", "dd"}; // prints dd
        print_arg_at_index(6, t3, t3[1]);
    }
    {
        const char* t4[] = {"unused", "-1", "val"};             // prints nothing (invalid)
        print_arg_at_index(3, t4, t4[1]);
    }
    {
        const char* t5[] = {"hello", "abc", "p", "q"};          // prints nothing (invalid)
        print_arg_at_index(4, t5, t5[1]);
    }

    return 0;
}