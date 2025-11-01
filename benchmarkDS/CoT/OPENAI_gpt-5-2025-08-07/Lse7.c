/* Chain-of-Through process:
 * 1) Problem understanding: Convert string to integer safely; print argv[INDEX] where INDEX is argv[1].
 * 2) Security requirements: Validate input, avoid overflow, bounds check, no undefined behavior.
 * 3) Secure coding generation: Careful parsing using long long and ctype checks.
 * 4) Code review: Ensure all pointer uses and indices are validated.
 * 5) Secure code output: Print "ERROR" on any invalid input or out-of-bounds.
 */

#include <stdio.h>
#include <ctype.h>
#include <limits.h>
#include <string.h>

static int safe_parse_index(const char* s, int* out_index) {
    if (s == NULL || out_index == NULL) return 0;
    const unsigned char* p = (const unsigned char*)s;

    // skip leading spaces
    while (*p && isspace(*p)) p++;

    if (*p == '\0') return 0;

    if (*p == '-') return 0; // negative not allowed
    if (*p == '+') {
        p++;
        if (*p == '\0') return 0;
    }

    long long val = 0;
    int digits = 0;
    while (*p) {
        if (isspace(*p)) break;
        if (*p < '0' || *p > '9') return 0;
        val = val * 10 + (*p - '0');
        if (val > INT_MAX) return 0;
        digits++;
        p++;
    }
    if (digits == 0) return 0;

    // skip trailing spaces
    while (*p) {
        if (!isspace(*p)) return 0;
        p++;
    }

    *out_index = (int)val;
    return 1;
}

static const char* get_arg_at_index(const char* const* argv, int argc, const char* index_str) {
    int idx = -1;
    if (!safe_parse_index(index_str, &idx)) return NULL;
    if (idx < 0 || idx >= argc) return NULL;
    return argv[idx];
}

static const char* print_at_index(const char* const* argv, int argc, const char* index_str) {
    const char* val = get_arg_at_index(argv, argc, index_str);
    if (val) {
        printf("%s\n", val);
        return val;
    } else {
        printf("ERROR\n");
        return NULL;
    }
}

int main(void) {
    const char* case1[] = {"a", "2", "alpha", "beta", "gamma"}; // prints alpha
    const char* case2[] = {"first", "0", "hello"};              // prints first
    const char* case3[] = {"cat", "3", "a", "b", "c", "d"};     // prints d
    const char* case4[] = {"oops", "notnum", "foo"};            // ERROR
    const char* case5[] = {"few", "9", "only", "two"};          // ERROR

    print_at_index(case1, 5, case1[1]);
    print_at_index(case2, 3, case2[1]);
    print_at_index(case3, 6, case3[1]);
    print_at_index(case4, 3, case4[1]);
    print_at_index(case5, 4, case5[1]);

    return 0;
}