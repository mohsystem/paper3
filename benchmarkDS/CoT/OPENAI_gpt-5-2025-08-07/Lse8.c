/* Chain-of-Through process in code generation:
 * 1) Problem understanding: Return argv[index] where index is parsed from argv[1].
 * 2) Security requirements: Validate argc/argv, parse safely, check bounds.
 * 3) Secure coding generation: Use strtoll with error checks; avoid undefined behavior.
 * 4) Code review: Ensure no unchecked access or NULL dereference.
 * 5) Secure code output: Robust function with tests.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

const char* get_value_at_index(int argc, const char* argv[]) {
    if (argv == NULL || argc < 2) {
        return NULL;
    }
    errno = 0;
    char *end = NULL;
    long long idx = strtoll(argv[1], &end, 10);
    if (errno != 0 || end == argv[1] || (end && *end != '\0')) {
        return NULL;
    }
    if (idx < 0 || idx >= argc) {
        return NULL;
    }
    return argv[idx];
}

int main(void) {
    // 5 test cases
    const char* t1[] = {"prog", "2", "alpha", "beta", "gamma"}; // expect "alpha"
    const char* t2[] = {"prog", "0", "alpha"};                  // expect "prog"
    const char* t3[] = {"prog", "5", "a", "b"};                 // invalid -> NULL
    const char* t4[] = {"prog", "-1", "x", "y"};                // invalid -> NULL
    const char* t5[] = {"prog", "notint", "x", "y"};            // invalid -> NULL

    const char* results[5];
    results[0] = get_value_at_index(5, t1);
    results[1] = get_value_at_index(3, t2);
    results[2] = get_value_at_index(4, t3);
    results[3] = get_value_at_index(4, t4);
    results[4] = get_value_at_index(4, t5);

    for (int i = 0; i < 5; ++i) {
        if (results[i] == NULL) {
            printf("Test %d: (null)\n", i + 1);
        } else {
            printf("Test %d: %s\n", i + 1, results[i]);
        }
    }
    return 0;
}